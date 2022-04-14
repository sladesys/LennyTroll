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

#include "webserver.h"
#include <map>

class WebClient {
public:
	bool secure;
	Socket *sock;
	SSL *ssl;
	WebSocket *ws;

	unsigned resultCode;
	std::map<std::string,std::string> mapHeaders;

	unsigned recvLen;
	char recvBuf[4096];

public:
	WebClient(bool secure =true);
	~WebClient();
	bool sslInit();

	bool send(const char *host,unsigned port,const char *rqst,const char *data = NULL);
	bool sendFile(const char *host,unsigned port,const char *rqst,const char *file);

	bool wsStart(const char *host,unsigned port,const char *uri,const char *protocol,WebSocket_Callbacks *cbs);

	bool rqstSend(const char *rqst);
	void rspRead();
	char* rspParse(const char *buf);
	void rspParseChunks(unsigned len,const char *buf);

	bool open(const char *host,unsigned port);
	void close();

	unsigned getResultCode() const { return resultCode; }
	const char* getResultData() const { return recvBuf; }
};


