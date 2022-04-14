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

#include "utils.h"
#include "config.h"
#include "modem.h"
#include "threader.h"
#include "webserver.h"


//
//
//
extern void startLenny();
extern void stopLenny();


//
//
//
class WSCallback : public WebSocket_Callbacks {
public:
    void onOpen(void* sock);
    void onText(void* sock,unsigned length,const char *data);
    void onBinary(void* sock,unsigned length,const uint8_t *data);
    void onClose(void* sock);
};


//
//
//
class Lenny;

class ThreadWaitForCall : public ThreadRunnable {
public:
	Lenny &lenny;
	ThreadWaitForCall(Lenny &l);
	void run();
};
class ThreadWaitForCallCheck : public ThreadRunnable {
public:
	Lenny &lenny;
	ThreadWaitForCallCheck(Lenny &l);
	void run();
};
class ThreadOffHook : public ThreadRunnable {
public:
	Lenny &lenny;
	ThreadOffHook(Lenny &l);
	void run();
};
class ThreadOnHook : public ThreadRunnable {
public:
	Lenny &lenny;
	ThreadOnHook(Lenny &l);
	void run();
};

class ThreadAnswerWithLenny : public ThreadRunnable {
public:
	Lenny &lenny;
	ThreadAnswerWithLenny(Lenny &l);
	void run();
};
class ThreadAnswerWithDisconnected : public ThreadRunnable {
public:
	Lenny &lenny;
	ThreadAnswerWithDisconnected(Lenny &l);
	void run();
};
class ThreadAnswerWithDTMF : public ThreadRunnable {
public:
	Lenny &lenny;
	ThreadAnswerWithDTMF(Lenny &l);
	void run();
};
class ThreadAnswerWithMessage : public ThreadRunnable {
public:
	Lenny &lenny;
	ThreadAnswerWithMessage(Lenny &l);
	void run();
};
class ThreadAnswerWithRecord : public ThreadRunnable {
public:
	Lenny &lenny;
	ThreadAnswerWithRecord(Lenny &l);
	void run();
};

class PCMFile {
public:
	Lenny &lenny;
	uint64_t tickStart,tickChunkStart;
	FILE *outFd;
	unsigned outTotal;

	PCMFile(Lenny &line);
	~PCMFile();

	bool open();
	void close();
	void write(unsigned len,const uint8_t *buf);
};


//
//
//
class Lenny {
public:
	unsigned idx;
	ConfigLine *configLine;
	Modem modem;

	PCMFile outPCM;
	char outFile[PATH_MAX];

	//
	//
	//
	enum { CLOSED=0,ONHOOK,RING,OFFHOOK };
	unsigned state;

	enum { READY=0,LENNY,DISCONNECTED,MESSAGES,RECORD };
	unsigned mode;

	uint64_t tickRingLast;

	volatile bool answerModeActive,sentCallerId;

	//
	bool dtmfInterrupted;
	unsigned dtmfCode;
	uint64_t dtmfCodeTickStart;


	//
	//
	//
	ThreadQueue queue;

	ThreadWaitForCall            waitForCall;
	ThreadWaitForCallCheck       waitForCallCheck;
	ThreadOffHook                offHook;
	ThreadOnHook                 onHook;

	ThreadAnswerWithLenny        answerWithLenny;
	ThreadAnswerWithDisconnected answerWithDisconnected;
	ThreadAnswerWithDTMF         answerWithDTMF;
	ThreadAnswerWithMessage      answerWithMessage;
	ThreadAnswerWithRecord       answerWithRecord;

	std::vector<void*> listListeningSockets;

	void listenAdd(void *sock);
	void listenRemove(void *sock);

public:
	Lenny(unsigned idx);
	~Lenny();

	bool start(ConfigLine &config);
	void createDefaultFileName();

	void answer();
	void hangUp();

	void startQueueThread();
	void handleWaitForCall();
	void handleCallWithLenny();
	void handleCallWithDisconnected();
	void handleCallWithDTMF();
	void handleCallWithMessage();
	void handleCallWithRecord();

	bool checkInterruptionByDTMF();

	bool playFile(const char *file);
	bool recordToFile();

	int sendPCMAudio(const char *file);
	void savePCMAudio(unsigned len,const uint8_t *buf);
	void sharePCMAudioBuffer(unsigned len,const uint8_t *buf);
};


