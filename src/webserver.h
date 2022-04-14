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
#include <stdint.h> // uint8_t



class WebSocket_Callbacks {
public:
	virtual ~WebSocket_Callbacks();
	virtual void onOpen(void* sock) =0;
	virtual void onText(void* sock,unsigned length,const char *data) =0;
	virtual void onBinary(void* sock,unsigned length,const uint8_t *data) =0;
	virtual void onClose(void* sock) =0;
};


extern bool startWebServer(WebSocket_Callbacks *cbs,unsigned port,bool secure=false);


class Socket {
public:
	int fd;
	Socket(int sock);
	virtual ~Socket();

	virtual bool accept();
	virtual bool connect();
private:
	virtual bool open();
public:
	virtual void close();
	virtual int poll(unsigned ms,unsigned pollValue); // 
	virtual int read(unsigned len,uint8_t *buf);
	virtual int write(unsigned len,const uint8_t *buf);
};


class WebSocket {
public:
	Socket &sock;
	WebSocket_Callbacks &cbs;
	bool active;

	WebSocket(Socket &sock,WebSocket_Callbacks &cbs);
	~WebSocket();

	void run();
	void close();

	bool sendText(const char *txt);
	bool sendBinary(unsigned len,const uint8_t *buf);
	bool sendPong(unsigned len,const uint8_t *buf);
	bool sendPing();

	uint8_t *encode(unsigned opcode,bool mask,unsigned *length,const uint8_t *data);
	bool decode();
};


#ifndef NO_SSL
#include <openssl/ssl.h>
#include <openssl/bio.h>

class SSLSocket : public Socket {
public:
	SSL_CTX *ctx;
	BIO *bio;
	SSL *ssl;

	SSLSocket(SSL_CTX *ctx,int sock);
	virtual ~SSLSocket();

	bool accept();
	bool connect();
	bool open();
	void close();
	int poll(unsigned ms,unsigned pollValue); // POLLIN | POLLOUT
	int read(unsigned len,uint8_t *buf);
	int write(unsigned len,const uint8_t *buf);
};
#endif //NO_SSL
