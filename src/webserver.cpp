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

#include <stdlib.h>
#include <string.h> //strtok_r
#include <limits.h> //PATH_MAX

#include <unistd.h> // unlink
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include <thread>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>

#ifdef NO_OPENSSL
#include <sha.h>
#endif //NO_OPENSSL

#include "webserver.h"
#include "utils.h"


//
//
//
extern bool wsSendText(void *sock,const char *text);
extern bool wsSendBinary(void *sock,unsigned len,const uint8_t *binary);
extern void sslInit();


//
// Internal
//

class WebServer {
public:
	WebSocket_Callbacks &cbs;
	bool useSSL;
	int serverSocket;
	
	WebServer(WebSocket_Callbacks &cb,bool ssl);
	~WebServer();

#ifndef NO_SSL
	bool sslInit();
#endif //NO_SSL

	bool open(unsigned port);
	void close();

	static void *threadListen(void*);
	void listen();

	static void *threadRequest(void*);
};



//
//
//

static WebServer *server = NULL,*sslServer = NULL;


bool startWebServer(WebSocket_Callbacks *const cbs,const unsigned port,const bool secure) {

	if (!secure) {
		if (NULL == server) {
			server = new WebServer(*cbs,false);

			if (!server->open(port)) {
				LOGC("HTTP server open failed port:%d",port);

				delete server;
				server = NULL;
				return false;
			}
		}

	} else {

#ifdef NO_SSL
		return false;
#else //NO_SSL

		if (NULL == sslServer) {
			sslServer = new WebServer(*cbs,true);

			if (!sslServer->open(port)) {
				LOGC("HTTPS server open failed port:%d",port);

				delete sslServer;
				sslServer = NULL;
				return false;
			}
		}
#endif //NO_SSL
	}

	LOGC("WebServer started");
	return true;
}



//
// Internal
//

class WebServerRequest {
public:
	WebServer &server;
	Socket &sock;
	struct sockaddr_in addr;

	WebServerRequest(WebServer &svr,Socket &s,struct sockaddr_in *addr);
	~WebServerRequest();

	static void *thread(void*);

	void handle();
	char* readHeader();

	void reply101(const char *protocol,const char *key);
	void reply200(const char *path);
	void reply404();
};

class WebSocketServer {
public:
	WebServerRequest &rqst;
	WebSocket ws;

	WebSocketServer(WebServerRequest &rqst,WebSocket_Callbacks &cbs);
	~WebSocketServer();

	void run();
	void close();

	inline bool sendText(const char *txt) { return ws.sendText(txt); }
	inline bool sendBinary(unsigned len,const uint8_t *buf) { return ws.sendBinary(len,buf); }
};


//
//
//
bool wsSendText(void *const sock,const char *const text) {
	LOGD(text);
	return ((WebSocket*)sock)->sendText(text);
}

bool wsSendBinary(void *const sock,const unsigned len,const uint8_t *const binary) {
	//LOGD("send binary:%d",len);;
	return ((WebSocket*)sock)->sendBinary(len,binary);
}







//
//
//
#ifndef NO_SSL

static SSL_CTX *ctxServerSSL = NULL;

/*

	https://wiki.mozilla.org/Security/Server_Side_TLS
	https://www.openssl.org/docs/man1.0.2/man1/ciphers.html
	https://www.openssl.org/docs/man1.1.1/man1/ciphers.html

*/

//#define CIPHER_LIST "RC4-SHA:AES256-SHA"
//#define CIPHER_LIST "AES256-SHA"
//#define CIPHER_LIST "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256"
//#define CIPHER_LIST "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA256:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DSS:!DES:!RC4:!3DES:!MD5:!PSK"
//#define CIPHER_LIST "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384"
//#define CIPHER_LIST "ALL:!EXPORT:!EXPORT40:!EXPORT56:!aNULL:!LOW:!RC4:@STRENGTH"

//TLS1.2
//#define CIPHER_LIST "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:DHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES256-SHA256:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:DES-CBC3-SHA"

//TLS1.3
//#define CIPHER_LIST "TLS_AES_128_GCM_SHA256:TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256"
#define CIPHER_LIST "TLS_AES_128_GCM_SHA256:TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256" \
					":" \
					"ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:DHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES256-SHA256:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:DES-CBC3-SHA"

#include "webcert.h"
#include <openssl/err.h>

void sslInit() {
	static bool init = false;
	if (init) return;
	init = true;

	LOGC("sslInit");

 	SSL_library_init();
 	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
}

bool WebServer::sslInit() {
	if (NULL != ctxServerSSL) return true;

	::sslInit();

 	SSL_CTX *const ctx = SSL_CTX_new(SSLv23_server_method());

	if (NULL == ctx) {
		LOGC("SSL_CTX_new failed");
		if (LOGGING_CONTROL) ERR_print_errors_fp(stderr);
		return false;
	}

	if (1 != SSL_CTX_set_cipher_list(ctx,CIPHER_LIST)) {
		LOGC("SSL_CTX_set_cipher_list failed");
		if (LOGGING_CONTROL) ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
	    return false;
  	}

#ifdef SSL_CTRL_SET_ECDH_AUTO
	SSL_CTX_set_ecdh_auto(ctx,1);
#else //SSL_CTRL_SET_ECDH_AUTO
	#define SSL_CTRL_SET_ECDH_AUTO 94
	SSL_CTX_ctrl(ctx,SSL_CTRL_SET_ECDH_AUTO,1,NULL);
#endif //SSL_CTRL_SET_ECDH_AUTO


	//
	//
	//
	{
		WebCert c;
		if (!c.load(ctx)) {
			LOGC("WebCert load failed");
			SSL_CTX_free(ctx);
			return false;
		}
	}

	//
	ctxServerSSL = ctx;
	return true;
}
#endif //NO_SSL



//
//
//

WebServer::WebServer(WebSocket_Callbacks &cb,bool ssl) : cbs(cb),useSSL(ssl),serverSocket(0) {
	LOGT("constructor");
}

WebServer::~WebServer() {
	LOGT("destructor");

#ifndef NO_SSL
	if (NULL != ctxServerSSL) {
		SSL_CTX_free(ctxServerSSL);
		ctxServerSSL = NULL;
	}
#endif //NO_SSL
}


//
//
//

bool WebServer::open(const unsigned port) {
	LOGT("open port:%u",port);

#ifndef NO_SSL
	if (useSSL && !sslInit()) {
		return false;
	}
#endif //NO_SSL

	const int fd = socket(PF_INET,SOCK_STREAM,0);

	if (0 > fd) {
		perror("socket");
		return false;
	}

	LOGT("socket socket:%i",fd);

	// Reuse address
	{
		unsigned yes = 1;
		if (0 > setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes))) {
			perror("setsockopt");
			::close(fd);
			return false;
		}
	}

	// Blocking
	{
		const int flags = fcntl(fd,F_GETFL);
		if (0 > fcntl(fd,F_SETFL,flags & ~O_NONBLOCK)) {
 			perror("fcntl BLOCKING");
			::close(fd);
			return false;
		}
	}

	{
		struct sockaddr_in a; memset(&a,'\0',sizeof(a));
		a.sin_family = AF_INET;
		a.sin_port = htons((uint16_t)port);
		a.sin_addr.s_addr = INADDR_ANY;
	
		if (0 > bind(fd,(struct sockaddr *)&a,sizeof(struct sockaddr))) {
			perror("bind");
			return false;
		}
	}


#define BACKLOG 2

	//LOGT("openServer start listen with backlog:%i",BACKLOG);

	if (0 > ::listen(fd,BACKLOG)) {
		perror("listen");
		return false;
	}

	serverSocket = fd;
    std::thread t(threadListen,this);
    t.detach();

	return true;	
}

void WebServer::close() {
	LOGT("close");
	if (0 <  serverSocket) {
		::close(serverSocket);
		serverSocket = 0;
	}
}

void* WebServer::threadListen(void *const arg) {
	LOGT("thread server +");
	((WebServer*)arg)->listen();
	LOGT("thread server -");
	return NULL;
}

void WebServer::listen() {
	extern void threadSetName(const char *name);

#ifdef NO_SSL
	threadSetName("http");
#else //NO_SSL
	threadSetName(!useSSL ?"http" :"https");
#endif //NO_SSL

	while (true) {

		int fd;
		struct sockaddr_in addr;
		socklen_t sin_size = sizeof(addr);

		if (0 > (fd = accept(serverSocket,(struct sockaddr *)&addr,&sin_size))) {
			perror("accept");
			continue;
		}

		LOGT("accept socket:%u addr:%s",fd,inet_ntoa(addr.sin_addr));


		//
		//
		//

#ifdef NO_SSL
		Socket *const s = new Socket(fd);
#else //NO_SSL
		Socket *s;
		if (!useSSL) {
			s = new Socket(fd);
		} else {
			s = new SSLSocket(ctxServerSSL,fd);
		}
#endif //NO_SSL


		//
		//
		//

		if (!s->accept()) {
			LOGC("Socket accept failed");
			s->close();
		} else {
		    std::thread t(threadRequest,new WebServerRequest(*this,*s,&addr));
		    t.detach();
		}
	}

	close();
}

void* WebServer::threadRequest(void *const arg) {
	LOGC("thread request +");
	((WebServerRequest*)arg)->handle();
	LOGC("thread request -");
	return NULL;
}



//
//
//
WebServerRequest::WebServerRequest(WebServer &sv,Socket &s,struct sockaddr_in *const inaddr) : server(sv),sock(s) {
	LOGT("constructor");
	memcpy(&addr,inaddr,sizeof(addr));
}
WebServerRequest::~WebServerRequest() {
	LOGT("destructor");
	delete(&sock);
}
void* WebServerRequest::thread(void *const arg) {
	LOGT("thread +");
	((WebServerRequest*)arg)->handle();
	delete (WebServerRequest*)arg;
	LOGT("thread -");
	return NULL;
}

void WebServerRequest::handle() {
	LOGT("handle socket:%u",sock.fd);

	extern void threadSetName(const char *name);
	threadSetName(inet_ntoa(addr.sin_addr));
	
	char method[16] = {0},uri[512] = {0},protocol[16] = {0};
	long contentLength = 0;
	int code = 200;

	int websocket = false;
	char websocketKey[32] = {0},websocketProtocol[32] = {0};

	//
	// Read and parse header
	//
	{
		char *const hdr = readHeader();
		if (NULL == hdr || 0 == strlen(hdr)) {
			LOGC("httpReadHeader returned null method:%s uri:%s",method,uri);
			return;
		}

		{
			char *outer;
			char *lines = strtok_r(hdr,"\n",&outer);
			if (NULL == lines) {
				LOGC("null request hdr:%s",hdr);
				//reply404();
				return;
			}

			char *first;
			LOGD("request first parse:%s",lines);

			if (0 == strlen(lines)) {
				LOGD("null request hdr:%s",hdr);
				//reply404();
				return;
			}

			// Save request
			{
				char *const p = strtok_r(lines," ",&first);	
				if (NULL == p) {
					LOGD("parse failed hdr:%s",hdr);
					return;
				}
				strncpy(method,p,sizeof(method)-1);
			}
			{
				char *const p = strtok_r(NULL," ",&first);	
				if (NULL == p) {
					LOGD("parse failed hdr:%s",hdr);
					return;
				}
				strncpy(uri,p,sizeof(uri)-1);
			}
			{
				char *const p = strtok_r(NULL," ",&first);	
				if (NULL == p) {
					LOGD("parse failed hdr:%s",hdr);
					return;
				}
				strncpy(protocol,p,sizeof(protocol)-1);
			}

			LOGC("request method:%s uri:%s protocol:%s",method,uri,protocol);

			lines = strtok_r(NULL,"\n",&outer);
			while (NULL != lines) {
			//	LOGD("parse line:%s",lines);

				char *inner;
				char *const key = strtok_r(lines,":",&inner);
				if (NULL == key) break;

				char *value = strtok_r(NULL,":",&inner);
				lines = strtok_r(NULL,"\n",&outer);

				if (value) value++;

				LOGD("header key:%s value:%s",key,value);
				
				//if (0 == strcasecmp("Host-name"	,key)) { continue; }
				//if (0 == strcasecmp("Content-type",key)) { continue; }
				//if (0 == strcasecmp("Connection"	,key)) { continue; }

				if (0 == strcasecmp("Content-length" ,key)) { contentLength = atol(value); continue; }

				//if (0 == strcasecmp("Origin"	 ,key)) { continue; }

				if (0 == strcasecmp("Connection",key) && 0 == strcasecmp("Upgrade",value)) { continue; }
				if (0 == strcasecmp("Upgrade"   ,key) && 0 == strcasecmp("WebSocket",value)) { websocket = true; continue; }

				if (0 == strcasecmp("Sec-WebSocket-Version" ,key) && 0 == strcasecmp("13",value)) { continue; }
				if (0 == strcasecmp("Sec-WebSocket-Protocol",key)) { strcpy(websocketProtocol,value); continue; }
				if (0 == strcasecmp("Sec-WebSocket-Key"     ,key)) { strcpy(websocketKey,value); continue; }
			}
		}
		
		free(hdr);
	}

	//
	// Read the data portion
	//
	{
		if (0 == contentLength) {
		//	LOGD("contentLength:0");

			{
				const int flags = fcntl(sock.fd,F_GETFL);
				if (0 > fcntl(sock.fd,F_SETFL,flags | O_NONBLOCK)) {
					LOGC("fcntl failed setting to NONBLOCKing");
					perror("setsockopt");
					reply404();
					return;
				}
			}

			{
				contentLength = 4096;
				unsigned total = 0;
				uint8_t *const data = (uint8_t*)malloc((size_t)contentLength +1);
				while (true) {
					//LOGC("read +");
					const int r = sock.read((unsigned)contentLength,data);
					//LOGC("read r:%d -",r);
					if (-1 == r) { /*LOGC("read returned -1");*/ break; }
					if ( 0 == r) { /*LOGC("read returned 0, exiting loop total:%d",total);*/ break; }
					LOGD("data:\n%s",data);
					total += (unsigned)r;
				}
				LOGD("read total:%d",total);
				free(data);
			}

			{
				const int flags = fcntl(sock.fd,F_GETFL);
				if (0 > fcntl(sock.fd,F_SETFL,flags & !O_NONBLOCK)) {
					LOGC("fcntl failed setting to BLOCKing");
					perror("setsockopt");
					reply404();
					return;
				}
			}

		} else {
			LOGD("contentLength:%d",(unsigned)contentLength);

			uint8_t *const data = (uint8_t*)malloc((size_t)contentLength+1);
			uint8_t *p = data;
			while (0 < contentLength) {
				const int r = sock.read((unsigned)contentLength,p);
				if (-1 == r) { LOGC("read returned -1"); break; }
				if ( 0 == r) { LOGC("read returned 0, exiting loop"); break; }
				p += r;
			}
			LOGD("data:\n%s",data);
			free(data);
		}
	}


	//LOGD("uri file:%s",uri);	
	//LOGD("response code:%d",code);	

//	if (0 == strcasecmp("POST",method)) {
//		reply404();
//		return;
//	}


	if (0 != strcasecmp("GET",method)) {
		LOGC("not GET method:%s",method);
		reply404();
		return;
	}





	//
	// Connect using websocket or http get resource?
	//


	//
	// Virtual directory '/ws' with lenny protocol
	//
	if (websocket) {

		//
		// Lenny WebSocket 
		//
		// TODO can this be a callback?  true / false
		//

		if (0 == strcmp(uri,"/ws")) {
			if (0 != strcmp("lenny",websocketProtocol)) {
				LOGC("invalid protocol:%s",websocketProtocol);
				reply404();
				return;
			}

			//
			// WebSocket ...
			//
			LOGC("websocket");
			reply101(websocketProtocol,websocketKey);

			new WebSocketServer(*this,server.cbs);
			return;
		}

		LOGC("no WebSockets uri:%s",uri);
		reply404();
		return;
	}

 








	//
	// Check the uri resource and try to return it
	//

/*

	physical

	├── etc
	│    ├── lenny.cfg
	│    ├── lenny.pem
	│    └── lenny.key
	│
	├── opt
	│  └── web
	│    ├── index.html
	│    ├── favicon.ico
	│    ├── l.webmanifest
	│    ├── css
	│    │  ├── l.css
	│    │  ├── ld.css
	│    │  └── ll.css
	│    ├── js
	│    │  ├── l.js
	│    │  └── sw.js
	│    └── img
	│      └── { *.ico,*.png,*.jpgs }
	│
	└── var
	  ├── disconnected.wav
	  ├── tad_default.wav
	  ├── calls
	  └── lenny


	virtual

	├── css
	├── js
	├── img
	│
	├── etc
	│ └── lenny.pem
	│
	├── opt
	│ ├── disconnected.wav
	│ ├── tad_default.wav
	│ └── lenny
	│
	└── var
	  ├── tad_1.wav
	  ├── calls
	  └── lenny

*/

	{
		char file[PATH_MAX];
		{
			strcpy(file,uri);

			char *const p = strchr(file,'?');
			if (NULL != p) *p = '\0';
		}

		//
		// Root index default?
		//
		if (0 == strlen(file) || 0 == strcmp(file,"/")) {
			extern char PATH_WEB[];
			strcpy(file,PATH_WEB);
			strcat(file,"/index.html");
		} else {

// uri:      /opt/aud/lenny/lenny_08.wav
// file:      opt/web/opt/aud/lenny/lenny_08.wav
// resource: /home/pi/project/lenny/opt/web/opt


			//
			// WAV file access using path redirection
			//
			if (0 == strncmp(uri,"/etc/",5)) {
				strcpy(file,uri+1);
			} else

			if (0 == strncmp(uri,"/opt/",5)) {
				strcpy(file,uri+1);
			} else

			if (0 == strncmp(uri,"/var/",5)) {
				strcpy(file,uri+1);
			} else {

				extern char PATH_WEB[];
				strcpy(file,PATH_WEB);

				if (*uri != '/') strcat(file,"/");
				strcat(file,uri);
			}
		}


		//
		// Trim query parameters
		//
		{
			LOGC("file:'%s'",file);

			char *const p = strchr(file,'?');
			if (NULL != p) {
				//LOGD("parm:'%s'",p);
				*p = '\0';
			}
		}


		//
		// Validate path
		//
		char path[PATH_MAX];
		char *const p = realpath(file,path);

		if (NULL == p) {
			perror("realpath");
			LOGC("resource error uri:'%s' file:%s resource:%s",uri,file,path);
			reply404();
			return;
		}

		//
		// Security check path
		//
		{

			//
			// Ensure realpath is not out of scope of cwd
			//

			char cwd[PATH_MAX]; getcwd(cwd,sizeof(cwd));

			LOGC("\nfile:\n uri     :%s\n realpath:%s\n cwd     :%s\n\n",uri,p,cwd);

			if (0 != strncmp(cwd,p,strlen(cwd))) {
				LOGC("resource security error:\n uri:'%s'\n file:%s\n resource:%s\n realpath:%s\n",uri,file,path,p);
				reply404();
				return;
			}

		}



		//
		// Validate file
		//
		struct stat s;
		if (0 > stat(path,&s)) {
			if (0 > stat(path+1,&s)) {
				perror("stat");
				code = 404;
			} else {
				strcpy(path,uri+1);
			}
		} else
		if (S_IFDIR == (s.st_mode &S_IFDIR)) {
			LOGC("is a directory path:%s",path);
			code = 404;
		} else
		if (S_IFREG != (s.st_mode &S_IFREG)) {
			LOGC("not a regular file path:%s",path);
			code = 404;
		}

		switch (code) {
			default:  reply404(); return;
			case 200: reply200(path); sock.close(); return;
		}
	}
}

char* WebServerRequest::readHeader() {
	int max = 4096;
	char *header = (char*)malloc((size_t)max);
	header[0] = 0;

	char *ptr = header;

	int count = 0;
	char last = '\0';

	char line[1024]; line[0] = 0;
	char *l = line;
	
	while (true) {
		uint8_t c;

		int r = sock.read(sizeof(c),&c);
		//LOGD("read r:%d c:0x%X (%c)",r,c,c);

		if (0 > r) {
			LOGC("read failed r:%d",r);
			perror("read");
			sock.close();
			return NULL;
		}
		if (0 == r) {
			//LOGC("read end-of-stream");
			break;
		}

		//LOGC("read success r:%d",r);

		if ('\n' == last && '\r' == c) continue;
		if ('\n' == last && '\n' == c) break;
		if ('\r' == c) continue;

		last = (char)c;

		if ('\n' == c) {
			*l = '\0';
			l = line;
			//LOGD("header:%s",l);
		}

		*l++ = (char)c;
		*ptr++ = (char)c;
		*ptr = '\0';

		if (++count > max) {
			LOGC("buffer overflow");
			break;
		}
	}
	//LOGD("final header:\n%s",header);

	return header;
}

void WebServerRequest::reply101(const char *protocol,const char *key) {
	LOGT("reply101 protocol:'%s' key:'%s'",protocol,key);

	char buf[1024];

#ifndef OPENSSL_NO_SHA1

	uint8_t digest[SHA_DIGEST_LENGTH];

	{
		strcpy(buf,key);
		strcat(buf,"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

		SHA_CTX ctx;
		SHA1_Init(&ctx);
		SHA1_Update(&ctx,(uint8_t*)buf,strlen(buf));
		SHA1_Final(digest,&ctx);
	}

#else //NO_OPENSSL

	uint8_t digest[SHA1HashSize];

	{
		strcpy(buf,key);
		strcat(buf,"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

		SHA1Context ctx;
		SHA1Reset(&ctx);
		SHA1Input(&ctx,(uint8_t*)buf,strlen(buf));
		SHA1Result(&ctx,digest);
	}

#endif //NO_OPENSSL

	{
		size_t len = 0;
		char *const b64 = base64_encode(digest,sizeof(digest),&len);

		strcpy(buf,"HTTP/1.1 101 Switching Protocols\r\n");
		strcat(buf,"Upgrade: websocket\r\n");
		strcat(buf,"Connection: Upgrade\r\n");

		strcat(buf,"Sec-WebSocket-Accept: ");
		strcat(buf,b64); free(b64);
		strcat(buf,"\r\n");

		strcat(buf,"Sec-WebSocket-Protocol: ");
		strcat(buf,protocol);
		strcat(buf,"\r\n");

		strcat(buf,"\r\n");
		sock.write((unsigned)strlen(buf),(uint8_t*)buf);

		LOGC("response:\n%s",buf);
	}
}

void WebServerRequest::reply200(const char *const path) {
	LOGT("reply200 path:%s",path);

	long filesize = 0;
	FILE *file = NULL;

	if (NULL != path) {
		if (NULL == (file = fopen(path,"rb"))) {
			LOGC("fopen failed : %s",path)
			return reply404();
		}
	
		fseek(file,0,SEEK_END);
		filesize = ftell(file);
		rewind(file);
		//LOGD("filesize = %ld",filesize);
	}

	char sent[500];
	{
		extern char ip[32];
		char b[32];
		strcpy(sent,"HTTP/1.1 200 OK\r\n");
		//strcat(sent,"Server: Lenny 0.9.1\r\n");
		//strcat(sent,"Host: 127.0.0.1\r\n");
		strcat(sent,"Host: "); strcat(sent,ip); strcat(sent,"\r\n");
		
		strcat(sent,"Content-length: ");
	//static char* itoa(char *b,int v) { sprintf(b,"%d",v); return b; }
	//static char* ltoa(char *b,long v) { sprintf(b,"%ld",v); return b; }
		//strcat(sent,ltoa(b,filesize));
		sprintf(b,"%ld\r\n",filesize); strcat(sent,b);
	}

	if (NULL != path) {
		strcat(sent,"Content-Type: ");
		strcat(sent,getMimeType(path));
		strcat(sent,"; charset=UTF-8\r\n");
	}

	strcat(sent,"Connection: close\r\n");

	strcat(sent,"\r\n");
	sock.write((unsigned)strlen(sent),(uint8_t*)sent);

	if (NULL != file) {
		uint8_t b[4096];
		while (true) {
			size_t r = fread(b,sizeof(char),sizeof(b),file);

			if (0 == r) break;

			const int w = sock.write((unsigned)r,b);

			if (0 > w) {
				//LOGC("write <0, socket closed during write");
				break;
			}
			if (0 == w) {
				LOGC("write = 0, socket write failed");
				break;
			}
			if ((int)r > w) {
				LOGC("write = %d, not %d, socket write failed",w,r);
				break;
			}
		}
		fclose(file);
	}
}

void WebServerRequest::reply404() {
	LOGT("reply404");

	extern char ip[32];
	char buf[500];
	strcpy(buf,"HTTP/1.1 404 Not Found\r\n");
	//strcat(buf,"Server: Lenny 0.9.1\r\n");
	//strcat(buf,"Host: 127.0.0.1\r\n");
	strcat(buf,"Host: "); strcat(buf,ip); strcat(buf,"\r\n");

	strcat(buf,"Connection: close\r\n");
	strcat(buf,"\r\n");
	sock.write((unsigned)strlen(buf),(uint8_t*)buf);
	sock.close();
}



//
//
//


WebSocketServer::WebSocketServer(WebServerRequest &r,WebSocket_Callbacks &cbs) : rqst(r),ws(r.sock,cbs) {
	LOGT("constructor");
	run();
}
WebSocketServer::~WebSocketServer() {
	LOGT("destructor");
}

void WebSocketServer::run() {
	LOGT("run +");
	ws.run();
	delete this;
	LOGT("run -");
}

void WebSocketServer::close() {
	LOGT("close");
	ws.close();
}




//
//
//
Socket::Socket(const int sock) :
	fd(sock)
{
	LOGT("Socket constructor");
}
Socket::~Socket() {
	LOGT("Socket destructor");
}

bool Socket::accept() {
	LOGT("Socket accept");
	return true;
}

bool Socket::connect() {
	LOGT("Socket connect");
	return true;
}

bool Socket::open() {
	LOGT("Socket open");
	return true;
}

void Socket::close() {
	LOGT("Socket close");
	if (0 < fd) {
		::close(fd);
		fd = 0;
	}
}

int Socket::poll(const unsigned ms=1000,const unsigned pv=POLLIN) {
	struct pollfd fds[1] = { {(int)fd,(short)pv,-1} };
	const int r = ::poll(fds,1,(int)ms);

	//LOGC("read poll timeout %dms r:%d",ms,r);

	if (0 == r) return 0;

	if (0 > r) {
		if (EAGAIN == errno) return 0;

		LOGC("poll error r:%d errno:%d",r,errno);
		return r;
	}

	return 1;
}

int Socket::read(const unsigned len,uint8_t *buf) {
//	const ssize_t r = ::read(fd,(char*)buf,len);
	const ssize_t r = ::recv(fd,(char*)buf,len,0 /*MSG_DONTWAIT*/);
	//LOGC("read len:%d r:%d",len,r);
	return (int)r;
}

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0 //
#endif //
int Socket::write(const unsigned len,const uint8_t *buf) {
	int t = 0;
	while ((unsigned)t < len) {
	//	const ssize_t w = ::write(fd,(char*)buf+t,len - (int)t);
		const ssize_t w = ::send(fd,(char*)buf+t,len - (unsigned)t,MSG_NOSIGNAL);
		LOGD("send r:%d",w);
		if (0 > w) {
			LOGC("socket error");
			return -1;
		}
		if (0 == w) {
			LOGD("write returned zero");
			continue;
		}
		t += (int)w;
	}
	return t;
}


//
//
//

#ifndef NO_SSL

SSLSocket::SSLSocket(SSL_CTX *const sslCtx,int sock) : Socket(sock),ctx(sslCtx),bio(NULL),ssl(NULL) {
	LOGT("SSLSocket constructor");
}
SSLSocket::~SSLSocket() {
	LOGT("SSLSocket destructor");
}

bool SSLSocket::accept() {

	if (!open()) {
		LOGC("open failed");
		return false;
	}

	ssl = SSL_new(ctx);

	if (NULL == ssl) {
		LOGC("SSL_new failed");
	    return false;
  	}

	bio = BIO_new_socket(fd,BIO_NOCLOSE);
	SSL_set_bio(ssl,bio,bio);

	LOGC("ssl accept +");

	if (1 != SSL_accept(ssl)) {
		LOGC("SSL_accept failed");
		if (LOGGING_CONTROL) ERR_print_errors_fp(stderr);
	    return false;
	}

	LOGC("ssl accept -");

	return true;
}

bool SSLSocket::connect() {
	LOGT("SSLSocket connect");

	if (!open()) {
		LOGV("open failed");
		return false;
	}

	ssl = SSL_new(ctx);

	if (NULL == ssl) {
		LOGV("SSL_new failed");
	    return false;
  	}

	SSL_set_fd(ssl,fd);

	const int err = SSL_connect(ssl);

	if (0 >= err) {
		LOGC("Error creating SSL connection err:%x",err);
		if (LOGGING_CONTROL) ERR_print_errors_fp(stderr);

		SSL_free(ssl);
		ssl = NULL;

		::close(fd);
		fd = 0;
		return false;
	}

	return true;
}

bool SSLSocket::open() {
	LOGT("SSLSocket open");

	if (ssl) {
		LOGV("ssl not null, should never happen");
		return false;
	}

	return true;
}

void SSLSocket::close() {
	LOGT("SSLSocket close");

	if (NULL != ssl) {
		SSL_shutdown(ssl);
		SSL_free(ssl);
		ssl = NULL;
	}

	Socket::close();
}

int SSLSocket::poll(const unsigned ms,const unsigned pollValue) {
	return Socket::poll(ms,pollValue);
}

int SSLSocket::read(unsigned len,uint8_t *buf) {
	const ssize_t r = SSL_read(ssl,buf,(int)len);
	//LOGD("SSL_read len:%d r:%d",len,r);

	if (0 < r) return (int)r;

	{
		const int rr = SSL_get_error(ssl,1);
		switch (rr) {
			default: LOGC("SSL_UNKNOWN:%d",rr); break;
			case SSL_ERROR_NONE       : /*LOGC("SSL_ERROR_NONE");*/ return 0;
			case SSL_ERROR_ZERO_RETURN: LOGC("SSL_ERROR_ZERO_RETURN"); break;
			case SSL_ERROR_WANT_READ  : LOGC("SSL_ERROR_WANT_READ"); break;
			case SSL_ERROR_WANT_WRITE : LOGC("SSL_ERROR_WANT_WRITE"); break;

			case SSL_ERROR_WANT_CONNECT        : LOGC("SSL_ERROR_WANT_CONNECT"); break;
			case SSL_ERROR_WANT_ACCEPT         : LOGC("SSL_ERROR_WANT_ACCEPT"); break;
			case SSL_ERROR_WANT_X509_LOOKUP    : LOGC("SSL_ERROR_WANT_X509_LOOKUP"); break;
		//	case SSL_ERROR_WANT_ASYNC          : LOGC("SSL_ERROR_WANT_ASYNC"); break;
		//	case SSL_ERROR_WANT_ASYNC_JOB      : LOGC("SSL_ERROR_WANT_ASYNC_JOB"); break;
		//	case SSL_ERROR_WANT_CLIENT_HELLO_CB: LOGC("SSL_ERROR_WANT_CLIENT_HELLO_CB"); break;
			case SSL_ERROR_SYSCALL: LOGC("SSL_ERROR_SYSCALL"); break;
			case SSL_ERROR_SSL    : LOGC("SSL_ERROR_SSL"); break;
		}
	}

	return (int)r;
}

int SSLSocket::write(unsigned len,const uint8_t *buf) {
	int t = 0;
	while ((unsigned)t < len) {

		const ssize_t r = SSL_write(ssl,buf+t,(int)len - t);
		//LOGD("SSL_write r:%d",r);

		if (0 >= r) {
			LOGC("send return -1, socket closed errno:%d",errno);

			{
				const int rr = SSL_get_error(ssl,1);
				switch (rr) {
					default: LOGC("SSL_UNKNOWN:%d",rr); break;
					case SSL_ERROR_NONE       : /*LOGC("SSL_ERROR_NONE");*/ break;
					case SSL_ERROR_ZERO_RETURN: LOGC("SSL_ERROR_ZERO_RETURN"); break;
					case SSL_ERROR_WANT_READ  : LOGC("SSL_ERROR_WANT_READ"); break;
					case SSL_ERROR_WANT_WRITE : LOGC("SSL_ERROR_WANT_WRITE"); break;

					case SSL_ERROR_WANT_CONNECT        : LOGC("SSL_ERROR_WANT_CONNECT"); break;
					case SSL_ERROR_WANT_ACCEPT         : LOGC("SSL_ERROR_WANT_ACCEPT"); break;
					case SSL_ERROR_WANT_X509_LOOKUP    : LOGC("SSL_ERROR_WANT_X509_LOOKUP"); break;
				//	case SSL_ERROR_WANT_ASYNC          : LOGC("SSL_ERROR_WANT_ASYNC"); break;
				//	case SSL_ERROR_WANT_ASYNC_JOB      : LOGC("SSL_ERROR_WANT_ASYNC_JOB"); break;
				//	case SSL_ERROR_WANT_CLIENT_HELLO_CB: LOGC("SSL_ERROR_WANT_CLIENT_HELLO_CB"); break;
					case SSL_ERROR_SYSCALL: LOGC("SSL_ERROR_SYSCALL"); break;
					case SSL_ERROR_SSL    : LOGC("SSL_ERROR_SSL"); break;
				}
			}

			return -1;
		}

		t += r;
	}

	return t;
}
#endif //NO_SSL

