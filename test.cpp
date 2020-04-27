#include "MFJSON.h"
#ifndef _MSC_VER
#include <sys/time.h>
#else
#include <windows.h>
#endif

#include "rapidjson/document.h"
#include "simdjson.h"

std::string read_file_bytes(const char* path)
{
	FILE* fp = fopen(path,"rb");
	if(!fp)
	{
		printf("open file%s failed!\n", path);
		return std::string();
	}	
	fseek(fp, 0, SEEK_END);
	auto len = ftell(fp);
	std::string s;
	s.resize(len);
	fseek(fp,0, SEEK_SET);
	len = (int)fread(&s[0], 1, len ,fp);
	s.resize(len);
	fclose(fp);
	return s;
}
#ifndef WIN32

unsigned long long get_micro_sec()
{
 	timespec tv;
    clock_gettime (CLOCK_MONOTONIC, &tv);
    return tv.tv_sec * 1000000 + tv.tv_nsec/1000;
}
#else
unsigned long long get_micro_sec()
{
	static long long freq=0;
	if (freq == 0)
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		freq /= 1000;
	}
	long long counter;
	QueryPerformanceCounter((LARGE_INTEGER*)&counter);
	return counter * 1000 / freq;
}

#endif

struct TimeDuration
{
	TimeDuration()
	{
		t0 = get_micro_sec();
	}
	~TimeDuration()
	{

	}

	unsigned long long duration()
	{
		return get_micro_sec()-t0;
	}

	unsigned long long t0;
};


enum EntityType
{
	NPC,
	MONSTER,
	PLAYER,
};


struct EntityBase
{
	EntityType type;
	int id;
	int level;
	std::string name;

};

struct MinMax
{
	int min;
	int max;
};
struct MonsterAttr: public EntityBase
{
	float crit;
	float critdam;
	float dodge;
	float gather_speed;
	float hit;
	int maxhp;
	MinMax mdam;
	int mdef;
	MinMax pdam;
	int pdef;
	int run_away;
	int speed;
};

MFJSON_ACCESSOR_ENUM_BEGIN(EntityType)
MFJSON_ACCESSOR_ENUM(NPC)
MFJSON_ACCESSOR_ENUM(MONSTER)
MFJSON_ACCESSOR_ENUM(PLAYER)
MFJSON_ACCESSOR_ENUM_END()

MFJSON_ACCESSOR_VALUE_BEGIN(EntityBase)
MFJSON_ACCESSOR_VALUE(type);
MFJSON_ACCESSOR_VALUE(id);
MFJSON_ACCESSOR_VALUE(level);
MFJSON_ACCESSOR_VALUE(name);
MFJSON_ACCESSOR_VALUE_END()

MFJSON_ACCESSOR_VALUE_BEGIN(MinMax)
MFJSON_ACCESSOR_VALUE(min);
MFJSON_ACCESSOR_VALUE(max);
MFJSON_ACCESSOR_VALUE_END()


MFJSON_ACCESSOR_VALUE_BEGIN(MonsterAttr, EntityBase)
MFJSON_ACCESSOR_VALUE(crit);
MFJSON_ACCESSOR_VALUE(critdam);
MFJSON_ACCESSOR_VALUE(dodge);
MFJSON_ACCESSOR_VALUE(gather_speed);
MFJSON_ACCESSOR_VALUE(hit);
MFJSON_ACCESSOR_VALUE(maxhp);
MFJSON_ACCESSOR_VALUE(mdam);
MFJSON_ACCESSOR_VALUE(mdef);
MFJSON_ACCESSOR_VALUE(pdam);
MFJSON_ACCESSOR_VALUE(pdef);
MFJSON_ACCESSOR_VALUE(run_away);
MFJSON_ACCESSOR_VALUE(speed);
MFJSON_ACCESSOR_VALUE_END()



int main()
{
	bool b = std::is_enum<EntityType>();
#ifdef WIN32
	//::Sleep(2000);
#endif
	printf("test MFJSON  begin!\n");
	auto jsonstr = read_file_bytes("monster_attr.json");
	//speed test
	int accesscount = 1000000;
	{
		using namespace rapidjson;
		TimeDuration dt;
		Document doc;
		doc.Parse(jsonstr.c_str());
		//doc.ParseInsitu((char*)jsonstr.c_str());

		if (doc.GetParseError() == RAPIDJSON_NAMESPACE::kParseErrorNone)
		{
			printf("rapidjson load ok! used:%lldus\n", dt.duration());

			{
				int maxhp;
				TimeDuration dt;
				for (int i = 0; i < accesscount; i++)
				{
					maxhp = doc["6100"][6]["maxhp"].GetInt();
				}
				printf("rapidjson read value %d used:%lldus\n", accesscount, dt.duration());

			}
		}
		else
		{
			
		}

		



	}
	{
		TimeDuration dt;
		simdjson::dom::parser parser;
		simdjson::dom::element doc = parser.parse(jsonstr.c_str(), jsonstr.length());
		printf("simdjson load ok! used:%lldus\n", dt.duration());


	/*	{
			TimeDuration dt;

			int maxhp;
			TimeDuration dt;
			for (int i = 0; i < accesscount; i++)
			{
				maxhp = doc.at("6100").at(6) ["maxhp"];
			}
			printf("simdjson read value %d used:%lldus\n", accesscount, dt.duration());

		}*/

	}
	{
		using namespace MFJSON;
		Doc doc;
		{
			TimeDuration dt;

			if (!doc.load(jsonstr.c_str()))
			{
				printf("%s\n", doc.errorDesc(jsonstr.c_str()).c_str());
			}

			printf("MFJSON load ok!    used:%lldus\n", dt.duration());

		}
		//doc.load(jsonstr.c_str());

		{
			int maxhp;
			TimeDuration dt;

			for (int i = 0; i < accesscount; i++)
			{
				maxhp = doc.root()["6100"][6]["maxhp"].getInt();
			}
			printf("MFJSON read value %d used:%lldus\n", accesscount, dt.duration());

		}



	}

	//load c++ class
	MonsterAttr testv;

	{
		using namespace MFJSON;
		TimeDuration dt;
		Doc doc;
		//doc.load(jsonstr.c_str());
		std::map<std::string, std::vector<MonsterAttr>> datas;
		doc.loadTo(jsonstr.c_str(), datas);


		printf("MFJSON obj load ok!    used:%lldus load:%d\n", dt.duration(), (int)datas.size());

		testv = datas["6100"][6];
	}
	//save c++ class
	{

		using namespace MFJSON;
		Doc doc;

		std::vector<MonsterAttr> values;
		values.push_back(testv);
		testv.level = 999;
		values.push_back(testv);

		doc.root().set(values);
		values.clear();
		doc.root().get(values);
		std::string s = doc.dump(true);
		printf("%s\n", s.c_str());
	}

	//save doc
	{
		using namespace MFJSON;
		Doc doc;

		doc.root().arrayBegin();

		for (int i = 0; i < 2; i++)
		{
			 auto element = doc.root().arrayPush();
			 element.mapBegin();
			 element.mapPush("id").set(i);
			 element.mapPush("level").set(2);
			 element.mapPush("name").set("haha\nadsd");
			 element.mapEnd();
		}

		doc.root().arrayEnd();
	
		std::string s = doc.dump(true);
		printf("%s\n", s.c_str());

	}



	return 0;
}