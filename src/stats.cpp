/*

	This file is part of Lenny Troll project
	Copyright 2020 Slade Systems

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

    	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.

*/

#include "stats.h"
#include "utils.h"
#include "json.h"
#include <sstream>


//static std::string readFile(const char *const file);
extern std::string readFile(const char *const file);
static void saveFile(std::string str,const char *file);

//static void jsonRead(JSONItem *json,const char *key,char *tgt,unsigned len);
static int jsonReadInt(JSONItem *json,const char *key,int defaultVal =0);


//
//
//

Stats::Stats() : lennyCalls(0),lennyTime(0),disconnectedCalls(0),tadCalls(0) {}

void Stats::save() { saveFile(toString(),srcFile.c_str()); }

void Stats::load(const char *const STATS_FILE) {
	srcFile = STATS_FILE;

	std::string buf = readFile(STATS_FILE);
	if (0 == buf.length()) {

		//
		// Defaults
		//

		saveFile(toString(),STATS_FILE);
		return;
	}

//LOGV("%s",buf);

	//
	// Parse json
	//
	{
		JSONItem *const json = JSONItem::parse((char *)buf.c_str());
		if (NULL == json) {
			LOGV("json_parse failed");
		} else {
			JSONItem *const stats = json->get("stats");

			if (NULL == stats) {
				saveFile(toString(),STATS_FILE);
				return;
			}

			{
				JSONItem *const l = stats->get("lenny");
				if (NULL == l) {
					LOGV("missing lenny");
				} else {
					lennyCalls = (unsigned)jsonReadInt(l,"calls");
					lennyTime = (unsigned)jsonReadInt(l,"time");
				}
			}
			{
				JSONItem *const l = stats->get("disconnected");
				if (NULL == l) {
					LOGV("missing disconnected");
				} else {
					disconnectedCalls = (unsigned)jsonReadInt(l,"calls");
				}
			}
			{
				JSONItem *const l = stats->get("tad");
				if (NULL == l) {
					LOGV("missing tad");
				} else {
					tadCalls = (unsigned)jsonReadInt(l,"calls");
				}
			}
		}
	}	
}

std::string Stats::toString() {
	std::ostringstream ss;
	ss << "{\"stats\":{";

		ss << "\"lenny\":{";
			ss << "\"calls\":" << lennyCalls <<",";
			ss << "\"mins\":" << lennyTime;
		ss << "},";

		ss << "\"disconnected\":{";
			ss << "\"calls\":" << disconnectedCalls;
		ss << "},";

		ss << "\"tad\":{";
			ss << "\"calls\":" << tadCalls;
		ss << "}";

	ss << "}}";
	ss << std::endl;

	return ss.str();
}



//
//
//

int jsonReadInt(JSONItem *const json,const char *const key,const int defaultVal) {
	JSONItem *const v = json->get(key);
	if (NULL == v || (JSON_TYPE_INTEGER != v->type && JSON_TYPE_BOOL != v->type)) return defaultVal;
	return (int)v->int_value;
}


/*
std::string readFile(const char *const path) {
    std::string s;

    FILE *const f = fopen(path,"r");
    if (NULL == f) return s;
    
    while (true) {
        char buf[1024];
        const size_t r = fread(buf,1,sizeof(buf),f);
        if (0 >= r) break;
        buf[r] = '\0';
        s.append(buf);
    }
    fclose(f);
    return s;
}
*/

void saveFile(std::string str,const char *const STATS_FILE) {

	{
		char f[PATH_MAX]; sprintf(f,"%s.old",STATS_FILE);
		rename(STATS_FILE,f);
	}

	FILE *const file = fopen(STATS_FILE,"wb");
	if (NULL == file) {
		LOGV("fopen failed");
	} else {
		const size_t len = str.size();
		const size_t r = fwrite(str.c_str(),len,1,file);

		if (1 != r) {
			LOGV("fwrite failed");
		}

		fclose(file);
	}
}



