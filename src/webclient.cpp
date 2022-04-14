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

#include <errno.h>
#include <unistd.h> // close
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h> // strtok_r
#include <sstream>

#include "utils.h"
#include "webclient.h"

#ifndef NO_SSL
//#include <openssl/bio.h>
//#include <openssl/ssl.h>
#include <openssl/err.h>
static SSL_CTX *ctxClientSSL;
#endif //NO_SSL

//
//
//

#include <netdb.h> // struct addrinf


WebClient::WebClient(const bool useSSL) : secure(useSSL),sock(NULL),ssl(NULL),ws(NULL),recvLen(0) {
	LOGT("constructor");
	memset(recvBuf,0,sizeof(recvBuf));

#ifndef NO_SSL
	sslInit();
#endif //NO_SSL
}
WebClient::~WebClient() {
	LOGT("destructor");
	close();

#ifndef NO_SSL
	// if (NULL != ctxClientSSL) {
	// 	SSL_CTX_free(ctxClientSSL);
	// 	ctxClientSSL = NULL;
	// }
#endif //NO_SSL
}

/*
unsigned WebClient::getResultCode() {
	for (unsigned i=0;i<recvLen;i++) {
		if ('\r' == recvBuf[i]) { break; }
		if ('\n' == recvBuf[i]) { break; }
		if (' ' == recvBuf[i]) {
			return (unsigned)atoi(&recvBuf[1+i]);
		}
	}
	return 0;
}

char* WebClient::getResultData() {
	if (0 == recvLen) {
		LOGD("recvLen is 0");
		return NULL;
	}

	unsigned concurrentLF = 0;
	for (unsigned i=0;i<recvLen;i++) {
		if ('\r' == recvBuf[i]) { continue; }
		if ('\n' != recvBuf[i]) { concurrentLF = 0; continue; }
		concurrentLF++;
		if (2 > concurrentLF) continue;

		// avoid overflow
		if (i +1 >= recvLen) break;
		return &recvBuf[i+1];
	}

	return NULL;
}
*/

extern void sslInit();

#ifndef NO_SSL
bool WebClient::sslInit() {

	if (NULL != ctxClientSSL) return true;

	::sslInit();
// 	SSL_library_init();
//	SSL_load_error_strings();
//	SSLeay_add_ssl_algorithms();

#ifdef OPENSSL_101
	SSL_CTX *const ctx = SSL_CTX_new(TLSv1_2_client_method());
#else //OPENSSL_101
	SSL_CTX *const ctx = SSL_CTX_new(TLS_client_method());
#endif //OPENSSL_101

	if (NULL == ctx) {
		LOGE("SSL_CTX_new failed");
		ERR_print_errors_fp(stderr);
		return false;
	}

	ctxClientSSL = ctx;
	return true;
}
#endif //NO_SSL

bool WebClient::send(const char *const host,const unsigned port,const char *rqst,const char *const data) {

	bool r = open(host,port);

	if (r) {
		std::ostringstream ss;

		ss << rqst <<" HTTP/1.1\r\n";
		ss << "Host: " <<host <<"\r\n";
		ss << "Connection: close\r\n";

		if (!data) {
			ss <<"\r\n";
		} else {
			ss << "Content-Type: application/json\r\n";
			ss << "Content-Length: " << strlen(data) <<"\r\n";
			ss <<"\r\n";
			ss << data;
		}

		r = rqstSend(ss.str().c_str());

		rspRead();
	}

	if (sock) { delete sock; sock = NULL; }
	return r;
}

bool WebClient::sendFile(const char *const host,const unsigned port,const char *rqst,const char *const file) {

//	const char *const boundary = "----boundary";

	FILE *const fd = fopen(file,"rb");

	if (NULL == fd) {
		LOGV("fopen failed: %s",file);
		return 0;
	}

	fseek(fd,0,SEEK_END);
	const long len = ftell(fd);

	if (0 == len) {
		fclose(fd);
		LOGV("ftell zero length: %s",file);
		return 0;
	}

	fseek(fd,0,SEEK_SET);

	bool r = open(host,port);

	if (r) {
		std::ostringstream ss;

		ss << rqst <<" HTTP/1.1\r\n";
		ss << "Host: " <<host <<"\r\n";
		ss << "Connection: close\r\n";
		ss << "Content-Length: " <<len <<"\r\n";
		ss << "Content-Type: application/x-www-form-urlencoded\r\n";

/*
		ss << "Content-Type: multipart/form-data; boundary=" <<boundary <<"\r\n";
		ss << "\r\n";
		ss << boundary <<"\r\n";
		ss << "Content-Disposition: form-data; name=\"uploadedfile\"; filename=\"upload.txt\"" <<"\r\n";
		ss << "Content-Type: application/x-object\r\n";
*/

		ss << "\r\n";

		r = rqstSend(ss.str().c_str());

		if (!r) {
			LOGC("reqstSend failed");
		} else {

			//
			// Send file
			//
			{
				uint8_t buf[4096];
				while (true) {
					// read chunk
					const size_t rr = fread(buf,1,sizeof(buf),fd);
					if (0 >= rr) { break; }

					const int wr = sock->write((unsigned)rr,buf);
					if (wr != (int)rr) {
						LOGC("write failed");
						r = false;
						break;
					}
				}
			}

/*
			//
			// Send boundary close
			//
			{
				ss.clear()
				ss << "--" <<boundary <<"--";

				const int wr = sock->write((unsigned)rr,buf);
				if (wr != (int)rr) {
					LOGC("write failed");
					r = false;
				}
			}
*/

		}
		if (r) rspRead();
	}

	if (sock) { delete sock; sock = NULL; }
	fclose(fd);

	return r;
}

bool WebClient::wsStart(const char *const host,const unsigned port,const char *uri,const char *const protocol,WebSocket_Callbacks *const cbs) {

	bool r = open(host,port);

	if (r) {
		std::ostringstream ss;
		ss << "GET " <<uri <<" HTTP/1.1\r\n";
		ss << "Host: " <<host <<"\r\n";
		ss << "Upgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\n";
		ss << "Sec-WebSocket-Protocol: " <<protocol <<"\r\n";
	//	ss << "Sec-WebSocket-Key: KEY\r\n";
	//	ss << "Sec-WebSocket-Origin: origin\r\n";
		ss << "\r\n";

		r = rqstSend(ss.str().c_str());
	}

	ws = new WebSocket(*sock,*cbs);

	return r;
}

void WebClient::close() {
	LOGT(" ");

	if (ws) {
		delete ws;
		ws = NULL;
	}

	if (NULL != ssl) {
		SSL_shutdown(ssl);
		SSL_free(ssl);
		ssl = NULL;
	}

	if (sock) {
		delete sock;
		sock = NULL;
	}
}

bool WebClient::open(const char *const host,const unsigned port) {
	struct addrinfo *address = NULL;

	//
	// Get the address
	//
	{
		struct addrinfo hints; memset(&hints,0,sizeof(hints));
		hints.ai_family   = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		
		struct in6_addr addr;
		if (1 == inet_pton(AF_INET,host,&addr)) {
		  //LOGD("valid IPv4 text address");
			hints.ai_family = AF_INET;
		} else

		if (1 == inet_pton(AF_INET6,host,&addr)) {
		  //LOGD("valid IPv6 text address");
			hints.ai_family = AF_INET6;
			hints.ai_flags |= AI_V4MAPPED;
		} else {
			// no hints
		}

		char sport[10]; sprintf(sport,"%d",port);

		const int rc = getaddrinfo(host,(char*)sport,&hints,&address);

		if (0 != rc) {
			LOGC("host not found --> %s",gai_strerror(rc));

			if (EAI_SYSTEM == rc) {
				LOGC("getaddrinfo failed");
			}
			
			return false;
		}

		if (LOGGING_DATA) {
			char buf[INET_ADDRSTRLEN] = {0};
			LOGD("host:%s ip:%s",host,inet_ntop(AF_INET,&((sockaddr_in const *)address->ai_addr)->sin_addr,buf,sizeof(buf)));
		}
	}



	//
	// Get Socket
	//
	const int fd = socket(address->ai_family,address->ai_socktype,address->ai_protocol);

	if (0 >= fd) {
		LOGC("Could not open socket");

		freeaddrinfo(address);
		return false;
	}
	
/*
	{
		struct timeval t = {8,0};
		setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&t,sizeof(t));
		setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&t,sizeof(t));
	}

	// Non-blocking
	fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,NULL) | O_NONBLOCK);
*/

	//
	// Connect socket
	//
	int r = ::connect(fd,address->ai_addr,address->ai_addrlen);

	freeaddrinfo(address);


	//
	// Handle results
	//
	if (0 > r) {

		if (EINPROGRESS != errno) {
			LOGC("Socket connect failed errno:%d %s",errno,strerror(errno));
			return false;
		}

		struct pollfd fds[1] = { {fd,POLLOUT,-1} };
		r = poll(fds,1,8*1000);

		if (0 >= r || (1 ==r && POLLOUT != fds[0].revents)) {
			LOGC("Socket poll res:%d revents:0x%02X errno:%d %s",r,fds[0].revents,errno,strerror(errno));

			::close(fd);
			return false;
		}

		{
			int valopt; 
			socklen_t lon = sizeof(int);

			r = getsockopt(fd,SOL_SOCKET,SO_ERROR,&valopt,&lon); 

			if (0 > r) { 
				LOGC("getsockopt failed errno:%d %s",errno,strerror(errno));

				::close(fd);
				return false;
			}

			if (valopt) { 
				LOGC("Socket getsockopt valopt:%d %s\n",valopt,strerror(valopt));

				::close(fd);
				return false;
			}
		}
	}



#ifdef NO_SSL
	sock = new Socket(fd);

#else // NO_SSL
	if (443 != port) {
		sock = new Socket(fd);
	} else {
		sock = new SSLSocket(ctxClientSSL,fd);
	}

	if (!sock->connect()) {
		LOGW("connect failed");

		delete sock;
		return false;		
	}

	// if (ssl) {
	// 	LOGE("ssl not null, should never happen");
	// 	return false;
	// }

	// ssl = SSL_new(ctxClientSSL);
	// SSL_set_fd(ssl,fd);

	// const int err = SSL_connect(ssl);

	// if (0 >= err) {
	// 	ERR_print_errors_fp(stderr);
	// 	LOGC("Error creating SSL connection err:%x",err);

	// 	SSL_free(ssl);
	// 	ssl = NULL;

	// 	::close(fd);
	// 	fd = 0;
	// 	return false;
	// }

#endif // NO_SSL

	return true;
}

bool WebClient::rqstSend(const char *const str) {
	LOGD("%s",str)

	const unsigned len = (unsigned)strlen(str);
	const int r = sock->write(len,(uint8_t *)str);

	if (r != (signed)len) {
		LOGD("rqst failed");
		return false;
	}

	return true;
}


void WebClient::rspRead() {

	char buf[4096];
	const int r = sock->read(sizeof(buf),(uint8_t*)buf);

	if (0 >= r) {
		LOGD("read failed r:%d",r);
		return;
	}

    buf[r] = 0;
	const char *const d = rspParse(buf);
	if (NULL == d) {
		LOGC("rspParse returned NULL");
		return;
	}

/*

< HTTP/1.1 200 OK
< Date: Fri, 24 Apr 2020 18:37:22 GMT
< Server: Apache
< X-Powered-By: PHP/7.2.30
< Strict-Transport-Security: max-age=63072000; includeSubDomains
< X-Frame-Options: SAMEORIGIN
< X-Content-Type-Options: nosniff
< Upgrade: h2,h2c
< Connection: Upgrade, close
< Vary: Accept-Encoding
< Transfer-Encoding: chunked
< Content-Type: application/json

< HTTP/2 200 
< date: Sat, 22 Feb 2020 18:25:48 GMT
< server: Apache
< x-powered-by: PHP/7.2.27
< strict-transport-security: max-age=63072000; includeSubDomains
< x-frame-options: SAMEORIGIN
< x-content-type-options: nosniff
< content-length: 11
< content-type: text/html; charset=UTF-8
*/

	std::string cl = mapHeaders["content-length"];
	std::string te = mapHeaders["transfer-encoding"];

	unsigned contentLen = (unsigned)atoi(cl.c_str());
	bool     chunked    = 0 == strncasecmp("chunked",te.c_str(),7);

//LOGV("contentLen:%s chunked:%s",cl.c_str(),te.c_str());
//LOGV("contentLen:%d chunked:%d",contentLen,chunked);

	//const unsigned contentLen = 0;
	//const bool     chunked    = false;
	//const unsigned contentLen = mapHeaders.end() != mapHeaders["content-length"] ?(unsigned)atoi(mapHeaders["content-length"].c_str()) :0;
	//const bool     chunked    = mapHeaders["transfer-encoding"] ?0 == strcasecmp("chunked",mapHeaders["transfer-encoding"]) :false;


/*
	for (std::map<std::string,std::string>::iterator it = mapHeaders.begin(); it != mapHeaders.end(); it++) {
		const char *const key = it->first.c_str();
		LOGV("%s = %s",key,it->second.c_str());

		if (0 == strcasecmp("content-length"   ,key)) { contentLen = (unsigned)atoi(it->second.c_str()); continue; }
		if (0 == strcasecmp("transfer-encoding",key)) {
			if (0 == strcasecmp("chunked",it->second.c_str())) { chunked = true; }
		}
	}

LOGV("contentLen:%s chunked:%s",cl.c_str(),te.c_str());
LOGV("contentLen:%d chunked:%d",contentLen,chunked);
*/

	const unsigned remaining = (unsigned)r - (unsigned)(d - buf);

//LOGV("r:%d off:%d remaining:%d",r,(unsigned)(d - buf),remaining);

	if (!chunked) {
		if (remaining != contentLen) LOGV("remaining:%d contentLen:%d",remaining,contentLen);

		memcpy(recvBuf,d,(size_t)remaining);
	    
	    recvBuf[remaining] = 0;
		recvLen = remaining;
	} else {
		rspParseChunks(remaining,d);
	}

	//LOGD("t:%d\n%s",recvLen,recvBuf);
	//if (LOGGING_DATA) dump(recvLen,(uint8_t*)recvBuf);
}

void WebClient::rspParseChunks(const unsigned len,const char *const buf) {
	//LOGD("len:%d",len);

	//if (LOGGING_DATA) dump(len,(uint8_t*)buf);

	unsigned off = 0,finalLen = 0;
	while (len > off) {
		char lbuf[10];

		{
			unsigned l = 0;
			while (len > (off+l+1) && 0x0D != buf[off+l+0] && 0x0A != buf[off+l+1]) l++;

//LOGV("l:%d len:%d off:%d",l,len,off);

			if (0 < l) {
				strncpy(lbuf,buf+off,l); lbuf[l] = '\0';
			}

//LOGV("lbuf:%s",lbuf);

			off += l+2;
		}

		const unsigned chunkLen = (unsigned)strtol(lbuf,NULL,16);

//LOGV("chunkLen:%d off:%d",chunkLen,off);

		if (0x0D != buf[off+chunkLen+0] || 0x0A != buf[off+chunkLen+1]) {
			LOGC("parse error off:%d chunkLen:%d",off,chunkLen);
			return;
		}

		if (0 < chunkLen) {
			memmove(recvBuf+finalLen,buf+off,chunkLen);
			finalLen += chunkLen;
			off += chunkLen;
		}

		off += 2;
	}

    recvBuf[finalLen] = 0;
	recvLen = finalLen;
}

char* WebClient::rspParse(const char *const buf) {

	char *outer;
	char *lines = strtok_r((char*)buf,"\n",&outer);

	if (NULL == lines) {
		LOGD("null request hdr:%s",buf);
		return NULL;
	}

	if (0 == strlen(lines)) {
		LOGD("null request hdr:%s",buf);
		return NULL;
	}

	{
		char *first;

		{
			char *const p = strtok_r(lines," ",&first);	
			if (NULL == p) {
				LOGD("parse failed hdr:%s",buf);
				return NULL;
			}
			// HTTP/2
		}
		{
			char *const p = strtok_r(NULL," ",&first);	
			if (NULL == p) {
				LOGD("parse failed hdr:%s",buf);
				return NULL;
			}
			resultCode = (unsigned)atoi(p);
		}

		LOGD("resultCode:%d",resultCode);
	}
	{
		lines = strtok_r(NULL,"\n",&outer);
		while (NULL != lines) {
			if (2 >= strlen(lines)) break;

			//LOGD("parse header line:%s",lines);

			char *inner;
			char *const key = strtok_r(lines,":",&inner);
			if (NULL == key) break;

			char *value = strtok_r(NULL,":",&inner);
			lines = strtok_r(NULL,"\n",&outer);

			if (value) value++;

			//LOGD("header key:%s value:%s",key,value);

			for (unsigned i=0;key[i];i++) key[i] = (char)tolower(key[i]);
			mapHeaders[key] = value;
		}
	}

	return strtok_r(NULL,"\n",&outer);
}

