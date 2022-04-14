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

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <map>
#include <sys/stat.h>
#include "json.h"
#include "utils.h"
#include "config.h"


#define DEFAULT_MESSAGE_RINGS 4

static void load(JSONItem *json,ConfigLine &l);
static void jsonRead(JSONItem *json,const char *key,char *tgt,unsigned len);
static int jsonReadInt(JSONItem *json,const char *key,int defaultVal =0);



//
//
//

Config::Config()
 :	sv(NULL),svCheckLast(0),

	//
	// Defaults
	//
 	httpPort(5150),
 	userSecurity(false),skiplistEnabled(true),

	silenceVolumeThreshold(20),
 	silenceSwitchTime(1500),
	endCallSilenceTime(5000),
	endCallBusyCount(3),endCallBusyTime(13000)
{
	profiles.push_back("Lenny");
}
ConfigLine::ConfigLine()
 :	line(0),
 	lennyEnabled(true),
 	disconnectedEnabled(true),
 	tadEnabled(true),
 	tadRings(DEFAULT_MESSAGE_RINGS)
 {}

void Config::load(const char *const CONFIG_FILE) {
	LOGC(" ");

	srcFile = CONFIG_FILE;

	std::string buf = readFile(CONFIG_FILE);
	if (0 == buf.length()) {

		//
		// Defaults
		//
		line1.line = 1;
		strcpy(line1.name,"Line 1");
		strcpy(line1.number,"");
		strcpy(line1.device,"/dev/ttyACM0");
		line1.lennyEnabled = true;
		line1.disconnectedEnabled = false;
		line1.tadEnabled = true;
		line1.lennyProfile = "Lenny";
		line1.tadRings = 4;

		save();
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

			JSONItem *const config = json->get("config");
			if (NULL == config) {
				LOGV("config not found");
				return;
			}



			{
				JSONItem *const v = config->get("log_verbosity");
				if (NULL != v && JSON_TYPE_INTEGER == v->type) {
					verbosity = (unsigned)MIN(10,MAX(0,v->int_value));
				}
			}

			{
				JSONItem *const v = config->get("server_check_last");
				if (v) {
					struct tm tm;
					strptime(v->text_value,"%F %T",&tm);
					svCheckLast = mktime(&tm);
				}
			}
			{
				JSONItem *const v = config->get("server_version");
				if (sv) { delete sv; sv = NULL; }
				if (v) {
					sv = new ServerVersion(v->text_value);
				}
			}

			{
				JSONItem *const p = config->get("http_port");
				httpPort = (unsigned)(NULL == p || JSON_TYPE_INTEGER != p->type ?0 :p->int_value);
				if (1000 > httpPort || httpPort > 0xFFFF) {
					LOGW("http_port %d invalid (range: 1000 ~ 65535), using default: 5150",httpPort);
					httpPort = 5150;
				}
			}

		#ifndef NO_LICENSE
			{
				JSONItem *const em = config->get("email");
				email = NULL == em ?"" :em->text_value;
			}

			{
				JSONItem *const lic = config->get("license");
				license = NULL == lic ?"" :lic->text_value;
			}
		#endif //NO_LICENSE

			{
				JSONItem *const s = config->get("user_security");
				userSecurity = NULL == s ?false :0 != s->int_value;
				LOGD("userSecurity %s",userSecurity ?"true" :"false");
			}

			{
				JSONItem *const s = config->get("skiplist_enabled");
				skiplistEnabled = NULL == s ?false :0 != s->int_value;
			}

			//
			//
			//
			{
				JSONItem *const prs = config->get("profiles");
				if (NULL == prs || JSON_TYPE_ARRAY != prs->type) {
					LOGV("json_parse profiles failed");
				} else {
					std::map<std::string,bool> onlyOne;
	            	for (JSONItem *a = prs->child;NULL != a;a = a->next) {
	            		if (onlyOne.end() != onlyOne.find(a->text_value)) {
	            			LOGC("already contains %s",a->text_value);
	            			continue;
	            		}
	            		onlyOne[a->text_value] = true;

	            		LOGC("add profile %s",a->text_value);
	            		profiles.push_back(a->text_value);
	            	}
				}
			}

			{
				JSONItem *const p = config->get("silence_volume_threshold");
				silenceVolumeThreshold = (unsigned)(NULL == p || JSON_TYPE_INTEGER != p->type ?0 :p->int_value);
				if (10 > silenceVolumeThreshold || silenceVolumeThreshold > 64) {
					LOGW("silence_volume_threshold %d invalid (range: 10 ~ 64 of 255), using default: 10",silenceVolumeThreshold);
					silenceVolumeThreshold = 10;
				}
			}

			{
				JSONItem *const p = config->get("silence_switch_time");
				silenceSwitchTime = (unsigned)(NULL == p || JSON_TYPE_INTEGER != p->type ?0 :p->int_value);
				if (500 > silenceSwitchTime || silenceSwitchTime > 5000) {
					LOGW("silence_volume_threshold %d invalid (range: 500ms ~ 5000ms), using default: 1500ms",silenceVolumeThreshold);
					silenceSwitchTime = 1500;
				}
			}

			{
				JSONItem *const p = config->get("end_call_silence_time");
				endCallSilenceTime = (unsigned)(NULL == p || JSON_TYPE_INTEGER != p->type ?0 :p->int_value);
				if (10 > endCallSilenceTime || endCallSilenceTime > 10000) {
					LOGW("end_call_silence_time %d invalid (range: 10ms ~ 10000ms), using default: 5000ms",endCallSilenceTime);
					endCallSilenceTime = 5000;
				}
			}

			{
				JSONItem *const p = config->get("end_call_busy_signal_count");
				endCallBusyCount = (unsigned)(NULL == p || JSON_TYPE_INTEGER != p->type ?0 :p->int_value);
				if (1 > endCallBusyCount || endCallBusyCount > 10) {
					LOGW("end_call_busy_signal_count %d invalid (range: 10 ~ 64), using default: 10",endCallBusyCount);
					endCallBusyCount = 3;
				}
			}

			{
				JSONItem *const p = config->get("end_call_busy_signal_time");
				endCallBusyTime = (unsigned)(NULL == p || JSON_TYPE_INTEGER != p->type ?0 :p->int_value);
				if (1000 > endCallBusyTime || endCallBusyTime > 20000) {
					LOGW("end_call_busy_signal_time %d invalid (range: 1000ms ~ 20000ms), using default: 13000",endCallBusyTime);
					endCallBusyTime = 13000;
				}
			}

			//
			//
			//
			{
				JSONItem *const lines = json->get("lines");
				if (NULL == lines) {
					LOGV("json_parse lines failed");
				} else {

					{
						JSONItem *const l1 = lines->get("1");
						if (NULL == l1) {
						} else {
							line1.line = 1;
							::load(l1,line1);
						}
					}
					{
						JSONItem *const l2 = lines->get("2");
						if (NULL == l2) {
						} else {
							line2.line = 2;
							::load(l2,line2);
						}
					}
				}
			}

			JSONItem::free(json);
		}
	}
}

void Config::save() {
	LOGC(" ");

	std::string str = toString();

	{
		char f[PATH_MAX]; sprintf(f,"%s.old",srcFile.c_str());
		rename(srcFile.c_str(),f);
	}

	//
	// Write file
	//
	FILE *const file = fopen(srcFile.c_str(),"wb");
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

void Config::loadServerVersion(const char *const str) {
	LOGD("%s",str);
	if (sv) delete sv;
	sv = new ServerVersion(str);
}

std::string Config::toString() {
	std::ostringstream ss;
	ss << "{";

		ss << "\"config\":{";
			ss <<"\"log_verbosity\":" <<verbosity;
			ss <<",";

			if (0 < svCheckLast) {
				char t[32]; strftime(t,sizeof(t),"%F %T",localtime(&svCheckLast));
				ss << "\"server_check_last\":\"" <<t <<"\",";
			}

			if (sv) ss << "\"server_version\":\"" <<sv->toString() <<"\",";

			ss << "\"http_port\":" <<httpPort;
			ss <<",";
			ss << "\"user_security\":" <<(userSecurity ?"true" :"false");
			ss <<",";
			ss << "\"skiplist_enabled\":" <<(skiplistEnabled ?"true" :"false");
			ss <<",";

			ss <<"\"profiles\":[";
		    for (unsigned i=0,c=0; i<profiles.size();i++) {
		    	if (0 == strcasecmp("Lenny",profiles[i].c_str())) continue;
		    	if (0 < c++) ss <<",";
				ss <<"\"" <<profiles[i] <<"\"";
			}
			ss <<"]";

			ss <<",";
			ss << "\"silence_switch_time\":" <<silenceSwitchTime;
			ss <<",";
			ss << "\"silence_volume_threshold\":" <<silenceVolumeThreshold;
			ss <<",";
			ss << "\"end_call_silence_time\":" <<endCallSilenceTime;
			ss <<",";
			ss << "\"end_call_busy_signal_count\":" <<endCallBusyCount;
			ss <<",";
			ss << "\"end_call_busy_signal_time\":" <<endCallBusyTime;

			ss <<",";
			ss << "\"email\":\"" <<email <<"\"";
			ss <<",";
			ss << "\"license\":\"" <<license <<"\"";

		ss << "}";

		ss << ",";
		ss << "\"lines\":{";

			if (0 < strlen(line1.device)) {
				ss << "\"1\":" <<line1.toString();
				if (0 < strlen(line2.device)) {
					ss << ",\"2\":" <<line2.toString();
				}
			} else
			if (0 < strlen(line2.device)) {
				ss << ",\"1\":" <<line2.toString();
			}

		ss << "}";

	ss << "}";

	return ss.str();
}

std::string ConfigLine::toString() {
	std::ostringstream ss;
	ss << "{";
		ss << "\"name\":\""               << name   <<"\",";
		ss << "\"number\":\""             << number <<"\",";
		ss << "\"device\":\""             << device <<"\",";

		ss << "\"lenny_enabled\":"        << (lennyEnabled        ?"true":"false") <<",";
		ss << "\"disconnected_enabled\":" << (disconnectedEnabled ?"true":"false") <<",";
		ss << "\"tad_enabled\":"          << (tadEnabled          ?"true":"false") <<",";

		ss << "\"lenny_profile\":\""      << lennyProfile <<"\",";
		ss << "\"tad_rings\":"            << tadRings;
	ss << "}";

	return ss.str();
}

//
// Helpers
//
void load(JSONItem *const json,ConfigLine &l) {
	jsonRead(json,"name"  ,l.name  ,sizeof(l.name));
	jsonRead(json,"number",l.number,sizeof(l.number));
	jsonRead(json,"device",l.device,sizeof(l.device));
	l.lennyEnabled        = jsonReadInt(json,"lenny_enabled",0);
	l.disconnectedEnabled = jsonReadInt(json,"disconnected_enabled",0);
	l.tadEnabled          = jsonReadInt(json,"tad_enabled",0);

	{
		JSONItem *const pr = json->get("lenny_profile");
		l.lennyProfile = NULL == pr ?"Lenny" :pr->text_value;
	}

	l.tadRings = (unsigned)jsonReadInt(json,"tad_rings");
}

void jsonRead(JSONItem *const json,const char *const key,char *const tgt,const unsigned len) {
	memset(tgt,'\0',len);
	JSONItem *const v = json->get(key);
	if (NULL == v || JSON_TYPE_TEXT != v->type) return;
	strncpy(tgt,v->text_value,len-1);
}

int jsonReadInt(JSONItem *const json,const char *const key,const int defaultVal) {
	JSONItem *const v = json->get(key);
	if (NULL == v || (JSON_TYPE_INTEGER != v->type && JSON_TYPE_BOOL != v->type)) return defaultVal;
	return (int)v->int_value;
}


