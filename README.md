# MFJSON more fast json parser

### how to load json:

```c++
#include "MFJSON.h"

using namespace MFJSON;
Doc doc;

if (!doc.load(jsonstr.c_str()))
{
	printf("%s\n", doc.errorDesc(jsonstr.c_str()).c_str());				
}

		

```
#### how to read json value:

example json:

```json
{
	"1405":[
		{
			"crit":0.001,
			"critdam":0.5,
			"dodge":0,
			"gather_speed":0,
			"hit":1,
			"id":1405,
			"level":1,
			"maxhp":488,
			"mdam":{
				"max":135,
				"min":110
			},
			"mdef":21,
			"pdam":{
				"max":0,
				"min":0
			},
			"pdef":36,
			"run_away":0,
			"speed":4
		}
        ]
}
```

read maxhp:

```c++
auto maxhp = doc.root()["1045"][0]["maxhp"].getInt();

```



also support read as c++ object:

```c++
struct EntityBase
{
	int id;
	int level;
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
//define accessor for mfjson
MFJSON_ACCESSOR_VALUE_BEGIN(EntityBase)
MFJSON_ACCESSOR_VALUE(id);
MFJSON_ACCESSOR_VALUE(level);
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
    
void read()
{
		using namespace MFJSON;
		TimeDuration dt;
		Doc doc;
		std::map<std::string, std::vector<MonsterAttr>> datas;
		doc.load(jsonstr.c_str(), datas);

		testv = datas["1045"][0];
		printf("MFJSON obj load ok!    used:%lldus load:%d\n", dt.duration(), (int)datas.size());

}
```



#### how to serialize json text:

direct serialize c++ object:

```c++
void ObjSerialize()
{
		using namespace MFJSON;
		Doc doc;
    
		std::vector<MonsterAttr> values;
		values.push_back(testv);
		testv.level = 999;
		values.push_back(testv);

		doc.root().set(values);
		std::string s = doc.dump(true);
		printf("%s\n", s.c_str());
	
}
```



serialize with manual

```C++
void ManualSerialize()	
{
		using namespace MFJSON;
		Doc doc;

		doc.root().arrayBegin();

		for (int i = 0; i < 2; i++)
		{
			 auto& element = doc.root().arrayPush();
			 element.mapBegin();
			 element.mapPush("id").set(i);
			 element.mapPush("level").set(2);
			 element.mapPush("name").set("haha");
			 element.mapEnd();
		}

		doc.root().arrayEnd();
	
		std::string s = doc.dump(true);
		printf("%s\n", s.c_str());

	}
```

#### Performance:

compare with rapidjson:

load 3.6MB json , vs2017 compile with /Ox

access  json value 1000000 times

rapidjson load ok! used:25379us

rapidjson read value 1000000 used:1061692us

MFJSON load ok!    used:11508us

MFJSON read value 1000000 used:499141us
