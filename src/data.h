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

#pragma once

extern char PATH_ETC[];
extern char PATH_OPT[];
extern char PATH_WEB[];
extern char PATH_VAR[];
extern char PATH_LOG[];
extern char PATH_CALLS[];

extern char PATH_LOG_FILE[];
extern char PATH_STATS_FILE[];

extern char PATH_CONFIG[];
extern char PATH_SSL_PEM[];
extern char PATH_SSL_KEY[];

extern char PATH_OPT_LENNY[];
extern char PATH_VAR_LENNY[];

extern char PATH_SKIPLIST[];
extern char PATH_NEW_CALLS[];
extern char PATH_ANSWER_CALLS[];
extern char PATH_HISTORY_CALLS[];

extern char PATH_DISCONNECTED[];
extern char PATH_TAD_DEFAULT[];
extern char PATH_TAD_OGM_FMT[];





// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include "json.h"


//
std::string toString(const JSONItem *j);
extern bool fileExists(const char *const file);

//
//
//
extern std::string getSystemInfo();
extern bool signinValidate(const char *name,const char *pwd,std::ostringstream &ss);

extern std::string createAppLicense(unsigned days);
extern bool getAppLicenseInfo(const char *license,std::ostringstream &ss);


//
extern int shareWAVFile(void *sock,unsigned line,const char *file);

//
//
//
extern bool wsSendText(void *sock,const char *text);
extern bool wsSendBinary(void *sock,unsigned length,const uint8_t *data);
extern void wsSendAll(const char *const data);

//
//
//
extern FILE *wavOpen(const char *file);
extern FILE *wavCreate(const char *file);
extern void wavWriteHeader(FILE *const fd,const unsigned len);



//
//
//
extern void addCall(const char *line,const char *callerId);
extern void addHistory(JSONItem *it,const char *line,const char *date,const char *time);

extern void pruneCalls(const char *line,const char *date,const char *time);
extern void pruneNewCalls(const char *line,const char *date,const char *time);
extern void pruneHistory(const char *line,const char *date,const char *time);
extern void pruneMessage(const char *file);

extern void sendCalls(void *sock = NULL);


//
//
//
extern bool inSkiplist(const char *number,const char *name);
extern void cacheSkiplist();
extern void addSkiplist(const char *name,const char *number,const char *note);
extern std::string readSkiplist();
extern void sendSkiplist(void *sock);
extern void pruneSkiplist(const char *number);


//
//
//
extern bool addLennyProfile(std::vector<std::string> &profiles,const char *name);
extern void removeLennyProfile(std::vector<std::string> &profiles,const char *name);
extern void sendLennyProfiles(const std::vector<std::string> &profiles,void *sock);
extern std::string getLennyProfile(const char *const profile);
extern std::vector<std::string> getLennyFiles(const char *profile,bool includePath =true);


//
//
//
extern void cacheComment(unsigned rating,const char *comment);
extern void cacheBugReport(const char *summary,const char *comment);

extern std::string getLatestComments();
extern std::string getLatestBugReports();


