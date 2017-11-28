#include "MFJSON.h"
#ifndef WIN32
#include <sys/time.h>
#else
#include <windows.h>
#endif

#include "rapidjson/document.h"


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
	len = fread(&s[0], 1, len ,fp);
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
struct TestValue
{
	float crit;
};

MFJSON_ACCESSOR_VALUE_BEGIN(TestValue)
MFJSON_ACCESSOR_VALUE(crit);
MFJSON_ACCESSOR_VALUE_END();

int main()
{
	
#ifdef WIN32
	//::Sleep(2000);
#endif
	printf("test MFJSON  begin!\n");
	auto jsonstr = read_file_bytes("monster_attr.json");

	{
		using namespace rapidjson;
		TimeDuration dt;
		Document doc;
		doc.Parse(jsonstr.c_str());
		printf("rapidjson load ok! used:%lldus\n", dt.duration());
	}

	{
		using namespace MFJSON;
		TimeDuration dt;
		Doc doc;
		//doc.load(jsonstr.c_str());
		doc.load(jsonstr.c_str());


		printf("MFJSON load ok!    used:%lldus\n", dt.duration());

	}

	{
		using namespace MFJSON;
		TimeDuration dt;
		Doc doc;
		//doc.load(jsonstr.c_str());
		std::map<std::string, std::vector<TestValue>> datas;
		doc.load(jsonstr.c_str(), datas);


		printf("MFJSON load ok!    used:%lldus load:%d\n", dt.duration(), (int)datas.size());

	}

	{
		std::unordered_map<int,int> hashmap;
		for(int i= 0; i< 100000; i++)
		{
			hashmap.insert(std::make_pair(i,i));
		}

		TimeDuration dt;

		for (int i = 0; i < 100000; ++i)
		{
			hashmap.find(i);
		}

		printf("hashmap find used:%lldus\n", dt.duration());
	}
	{

	}


	return 0;
}