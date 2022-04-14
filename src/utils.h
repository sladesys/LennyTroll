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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif //_GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h> // uint8_t
#include <limits.h> // PATH_MAX
#include <poll.h>
#include <time.h> // strftime
#include <sys/time.h> //gettimeofday
#include <stdbool.h> //true,false

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

#define COUNTOF( ar) (sizeof(ar)/sizeof(ar[0]))
#define COUNT_OF(ar) (sizeof(ar)/sizeof(ar[0]))

#define SECONDS_IN_DAY 86400

extern void threadSetName(const char *name);
extern unsigned threadGetId();

extern uint64_t getTickMs();
extern uint64_t getTickUs();
extern void dump(unsigned len,const uint8_t *data);
extern char* trim(const char *buffer);

#include <string>
extern std::string trimRight(std::string s);

extern std::string rexec(const char *cmd);
extern std::string exec(const char *cmd);

extern void serveWebSocket(int port);

extern char *getIpAddress(char *buffer);
extern const char *getMimeType(const char *file);

extern bool fileExists(const char *file);
extern std::string readFile(const char *f);

extern char *base64_encode(const uint8_t *data,size_t input_length,size_t *output_length);
//extern unsigned char *base64_decode(const uint8_t *data,size_t input_length,size_t *output_length);



//
//
//

extern FILE *LOG_FD;
#include <pthread.h>
extern unsigned verbosity;
extern void logMessage(const char *prefix,const char *typ,const char *file,const char *function,unsigned line,unsigned long threadId,const char *const f, ...);

#define LOG_MSG(str,typ,...) { const unsigned _tid = threadGetId(); logMessage(" %s %s:%s(%d):%X - ",typ,__FILE__,__FUNCTION__,__LINE__,_tid,str,##__VA_ARGS__); }


enum {
	LOG_NONE       =0,
	LOG_ERROR      =1,
	LOG_WARN       =2,
	LOG_3          =3,
	LOG_LIFE_CYCLE =4,
	LOG_CONTROL    =5,
	LOG_VERBOSE    =6,
	LOG_DATA       =7
};

#define LOGGING_LIFE_CYCLE (LOG_LIFE_CYCLE <= verbosity)
#define LOGGING_CONTROL    (LOG_CONTROL    <= verbosity)
#define LOGGING_DATA       (LOG_DATA       <= verbosity)
#define LOGGING_VERBOSE    (LOG_VERBOSE    <= verbosity)

#define LOGE(str,...) { if (LOG_ERROR      <= verbosity) { LOG_MSG(str,"E",##__VA_ARGS__); } }
#define LOGW(str,...) { if (LOG_WARN       <= verbosity) { LOG_MSG(str,"W",##__VA_ARGS__); } }
#define LOGV(str,...) { if (LOG_VERBOSE    <= verbosity) { LOG_MSG(str,"V",##__VA_ARGS__); } }

#define LOGT(str,...) { if (LOG_LIFE_CYCLE <= verbosity) { LOG_MSG(str,"L",##__VA_ARGS__); } }
#define LOGC(str,...) { if (LOG_CONTROL    <= verbosity) { LOG_MSG(str,"C",##__VA_ARGS__); } }
#define LOGD(str,...) { if (LOG_DATA       <= verbosity) { LOG_MSG(str,"D",##__VA_ARGS__); } }

