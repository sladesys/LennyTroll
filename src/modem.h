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

//
enum MODEM_DLE {
	CAS_TONE       = '@',
	ANSWER_TONE    = 'a',
	BUSY_TONE      = 'b',
	FAX_TONE       = 'c',
	DATA_TONE      = 'e',
	LOCAL_ON_HOOK  = 'h',
	LOCAL_OFF_HOOK = 'H',
	RING_TONE      = 'R',
	SILENCE        = 's'
}; 

//
class Modem {
public:
	char modemDevice[PATH_MAX];
	int modemFd;

private:
	struct pollfd pollFDs[1];

public:
	volatile bool offHook,dtmfActive;
	unsigned lastCallRings;
	uint8_t lastDCEcode,lastDTMFcode;

	volatile bool lastCallerIdLoaded;
	char lastCallerIdBuf[512],lastCallerId[512];
	char lastCallerDate[16+1],lastCallerTime[4+1],lastCallerNumber[128],lastCallerName[128];

	bool parseBufferLstIsDLE;

	uint64_t totalBytesOut,totalBytesIn;
	unsigned totalCalls,totalCallRings,totalCallsAnswered,totalCallSeconds;
	uint64_t tickOffHook;

public:
	Modem();
	~Modem();

	bool open(const char *serialPort);
	void close();

	void reset();
	void callReset();
	void dtmfReset();

	int waitForRing();
	void waitAndRead(unsigned ms);

	int write(unsigned len,const uint8_t *buf);
	int readBuffer(bool keepReading,unsigned ms,unsigned len,uint8_t *buf);
	bool parseBuffer(unsigned inlen,const uint8_t *inbuf,unsigned &outIdx,uint8_t *buf);

	bool answer(unsigned len,const char *const *cmds);
	void hangup();

	bool cmd(const char *command,const char *response = "OK");
	bool cmd(unsigned timeout,const char *command,const char *response = "OK");

private:
	bool cmd(unsigned len,const char *const *cmds);
	bool cmdRsp(unsigned timeout,const char *command,const char *response);

	void parseCallerId();
};

//
class Modem_VRX {
private:
	Modem &modem;
public:
	Modem_VRX(Modem &m);
	bool start() const;
	void stop() const;
};

class Modem_VTX {
private:
	Modem &modem;
public:
	Modem_VTX(Modem &m);
	bool start() const;
	void stop() const;
};

/*
class Modem_VTR {
private:
	Modem &modem;
public:
	Modem_VTR(Modem &m);
	bool start() const;
	void stop() const;
};
//*/
