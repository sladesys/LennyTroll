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

#include <stdarg.h>
#include <signal.h>

#include <stdlib.h> //atoi
#include <unistd.h> // getcwd,close

#include "utils.h"
#include "modem.h"
#include "biz.h"
#include "webcert.h"

//
extern const char USAGE_OPTS[];
extern void drawTitle();
extern void drawUsage();
static void daemonize();
static void start();
//static void runMainThread();

#ifdef LINUX
//extern "C" void enumDevices();
#endif //LINUX

//
unsigned verbosity = 2;
char rootDir[PATH_MAX];

//
int main(const int argc,char **argv) {

	//
	// Change to root directory
	//
	{
		char scwd[PATH_MAX]; getcwd(scwd,sizeof(scwd));

		char path[PATH_MAX];
		char *const p = realpath(argv[0],path);
		strcpy(rootDir,p);
		char *s;

		s = strrchr(rootDir,'/');
		if (NULL == s) {
			fprintf(stderr,"strrchr failed %s\n",rootDir);
			exit(1);
		}
		*s = '\0';

		s = strrchr(rootDir,'/');
		if (NULL == s) {
			fprintf(stderr,"strrchr failed %s\n",rootDir);
			exit(1);
		}
		*s = '\0';

		if (0 != chdir(rootDir)) {
			fprintf(stderr,"chdir failed %s\n",rootDir);
			exit(1);
		}

	//	char cwd[PATH_MAX]; getcwd(cwd,sizeof(cwd));
	//	fprintf(stderr,"start cwd:%s\nargv:%s\npath:%s\nrootDir:%s\ncwd:%s\n\n",scwd,argv[0],p,rootDir,cwd);
	}

	drawTitle();

	//
	// Parameters parsed
	//
	if (1 < argc) {
		bool daemonStart = false,infoDisplay = false,certCreate = false,webappStart = false;
	//	bool deviceList = false;
		while (true) {
			const int c = getopt(argc,argv,USAGE_OPTS);
			if (EOF == c) break;
			switch (c) {
				case 'd': daemonStart = true; break;
				case 'i': infoDisplay = true; break;
				case 'c': certCreate = true; break;
				case 'l': webappStart = true; break;

				case 'v': { const int v = atoi(optarg); if (0 <= v) verbosity = (unsigned)v; break; }

			//	case 'l': deviceList = true; break;

				case 'h':
				case '?':
				default: drawUsage(); _exit(-1);
			}
		}

/*
		if (deviceList) {
#ifdef LINUX
//			enumDevices();
#endif //LINUX
			//exit(0);
			return 0;
		}
*/

		if (infoDisplay) {
			appConfigLoad();
			fprintf(stdout,"%s",appInfo().c_str());
			fflush(stdout);
			return 0;
		}

		if (certCreate) {
			webCertCreate();
			return 0;
		}

		if (daemonStart) {
			daemonize();
			return 0;
		}

		if (webappStart) {
			appWebAppStart();
			return 0;
		}

	}

	//
	//
	//
	start();

	return 0;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
static void onSignal(const int sig);

//#include <sys/stat.h> //umask
#ifdef LINUX
#include <syslog.h>
#endif //LINUX
//
//
//
void daemonize() {

	const pid_t pid = fork();

	if (0 > pid) {
		perror("fork failed");
		exit(EXIT_FAILURE);
	}

	if (0 < pid) {
		//LOGV("fork success pid:%d",(int)getpid());
		return;
	}

	//
	// Child process starts
	//

	// New session - CTTY detached
	const pid_t sid = setsid();

	if (0 > sid) {
		perror("setsid");
		exit(EXIT_FAILURE);
	}

/*
	{
		char dcwd[PATH_MAX]; getcwd(dcwd,sizeof(dcwd));

		if (0 != chdir(rootDir)) {
			LOGV("daemon chdir failed %s",rootDir);
			exit(1);
		}

		char cwd[PATH_MAX]; getcwd(cwd,sizeof(cwd));
		LOGV("daemon dcwd:%s cwd:%s",dcwd,cwd);
	}
*/

	// Reset file mode mask
	// umask(0);

	// Close descriptors
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

#ifdef LINUX

	//
	// the daemon process, write the daemon PID (as returned by getpid()) to a PID file,
	//	for example /run/foobar.pid 
	//	(for a hypothetical daemon "foobar") to ensure that the daemon cannot be started more than once.
	//

	openlog("LennyTroll",LOG_CONS | LOG_PERROR | LOG_PID,LOG_DAEMON);
	syslog(LOG_NOTICE,"%s","Lenny daemon started");
	closelog();

#endif //LINUX

	//
	//
	//
	{
		extern char PATH_LOG_FILE[];
		freopen(PATH_LOG_FILE,"a",stderr);
		stdout = stderr;
	}

	//LOGV("forked child pid:%d",(unsigned)getpid());

	drawTitle();

	start();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "threader.h"
ThreadQueue queue("main");


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class ThreadOnceADay : public ThreadRunnable {
public:
	void run() {
		LOGC("ThreadOnceADay +");
		queue.cancel(this);

		//
		//
		//
		appCheckLicense();

		//
		// schedule 12:01am
		//
		const time_t now = time(NULL);
		struct tm *const tm = localtime(&now);
		const unsigned secs = (unsigned)(((60*60)*(24 - tm->tm_hour -1)) + ((60)*(60 - tm->tm_min -1)) + (60 - tm->tm_sec));
		LOGC("time:%02d:%02d:%02d til midnight:%02d:%02d:%02d",tm->tm_hour,tm->tm_min,tm->tm_sec,secs/60/60,(secs/60)%60,secs%60);

		queue.post(this,(60+secs)*1000);


		//
		// Monday at 12:01am
		//
		if (1 == tm->tm_wday && 0 == tm->tm_hour) {
			extern bool sendLennyStats();
			sendLennyStats();
		}

		LOGC("ThreadOnceADay -");
	}
};

static ThreadOnceADay threadOnceADay;

class ThreadStart : public ThreadRunnable {
public:
	void run() {
		if (!appStart()) {
			LOGV("startApp failed");
			queue.stop();
			return;
		}

		fprintf(stdout,"%s",appInfo().c_str());
		fflush(stdout);

		queue.post(&threadOnceADay);
	}
};

static ThreadStart threadStart;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
// SIGINT (Ctrl-C) and SIGKILL signal handler
//  SIGHUP  1   Hangup detected - cause daemon to reload its config file
//	SIGINT	2	Interrupt (ANSI) Program interrupt (ctrl-c)
//	SIGILL	4	Illegal Instruction (ANSI) Generally indicates that the executable file is corrupted or use of data where a pointer to a function was expected
//
void onSignal(const int sig) {
	fprintf(stderr,"Lenny signal:%d\n",sig);
	fflush(stderr);

	appStop();

	if (SIGHUP == sig) {
		queue.post(&threadStart);
		return;
	}

	//queue.stop();
	_exit(0);
}


#include <map>
#include <string>
std::map<std::string,std::string> map;


//
void start() {
	//LOGV("start");

	//
	// Handle : Ctrl-C, Reload and kill terminator
	// Ignore : pipe / socket
	//
	signal(SIGINT ,onSignal);
	signal(SIGHUP ,onSignal);
	signal(SIGKILL,onSignal);
	signal(SIGPIPE,SIG_IGN);

	//
	//
	//
	queue.post(&threadStart);

	LOGC("main thread +");
	queue.run();
	LOGC("main thread -");

	appStop();
}


