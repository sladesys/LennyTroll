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

#include <stdlib.h> // malloc,realpath
#include <stdarg.h>
#include <pthread.h>
#include <thread>

#include <time.h>
#include <sys/time.h> // gettimeofday
#include <sys/types.h>

#include "utils.h"



//
//
//

void logMessage(const char *const prefix,const char *const typ,const char *const file,const char *const function,const unsigned line,const unsigned long threadId,const char *const f, ...) {
	char tb[20];
	{
		const time_t now = time(NULL);
		struct tm *const tm = localtime(&now);
		strftime(tb,sizeof(tb),"%F %T",tm);
	}
	char ms[20];
	{
		struct timeval tv; gettimeofday(&tv,NULL);
		sprintf(ms,".%03u",(unsigned)(tv.tv_usec /1000));
	}
	char fn[32];
	{
		strcpy(fn,file);
		char *const ext = strrchr(fn,'.');
		if (ext) *ext = '\0';
	}

	char s[4096];
	strcpy(s,tb); strcat(s,ms);
	sprintf(s+strlen(s),prefix,typ,fn,function,line,threadId);
	{
		va_list ap;
		va_start(ap,f);
		vsnprintf(s+strlen(s),sizeof(s) -strlen(s) -1,f,ap);
		va_end(ap);
	}
	strcat(s,"\n");

	fwrite(s,strlen(s),1,stderr);
	fflush(stderr);
}


//
//
//

unsigned threadGetId() {
//	return std::thread::native_handle();
//	return std::this_thread::native_handle();

#ifdef OSX //__MACH__
	uint64_t _tid; pthread_threadid_np(NULL,&_tid);
	return (unsigned)_tid;
#endif //OSX
#ifdef LINUX
	const pthread_t _tid = pthread_self();
	return (unsigned)_tid;
#endif //LINUX

}

void threadSetName(const char *name) {
#ifdef OSX
	pthread_setname_np(name);
#endif //OSX
#ifdef LINUX	
	pthread_setname_np(pthread_self(),name);
#endif //LINUX
}


//
//
//

uint64_t getTickMs() {
	struct timeval tv; gettimeofday(&tv,NULL);
	return ((uint64_t)tv.tv_sec * 1000) + (uint64_t)(tv.tv_usec /1000);
}
uint64_t getTickUs() {
	struct timeval tv; gettimeofday(&tv,NULL);
	return (1000000LL * (uint64_t)tv.tv_sec) + (uint64_t)tv.tv_usec;
}


//
//
//
void dump(const unsigned len,const uint8_t *const data) {
	fprintf(stderr,"\ndump hex len:%d\n",len);

	unsigned i=0,j;
	while (i < len) {
		char buf[128]; buf[0] = 0;
		int si = (int)i;
		for (j=0;j<16 && i<len;i++,j++) {
			char b[32];
			sprintf(b,"%02X ",data[i]&0xFF);
			strcat(buf,b);
		}

		while (j < 16) { strcat(buf,"   "); j++; }
		strcat(buf,"  ");

		i = (unsigned)si;
		for (j=0;j<16 && i<len;i++,j++) {
			char b[32];
			char d = (char)data[i];
			if ('\n' == d) d = '_'; else
			if ('\r' == d) d = '_';
			sprintf(b,"%c ",d);
			strcat(buf,b);
		}
		strcat(buf,"\n");
		fwrite(buf,strlen(buf),1,stderr);
	}

	fprintf(stderr,"\n\n");
	fflush(stderr);
}


//
//
//

static char* _trim(int len,char *buffer);
char* trim(const char *const buffer) { return _trim((int)strlen(buffer),(char*)buffer); }
char* _trim(const int len,char *const buffer) {
	if (0 >= len) return buffer;

	//
	// Remove leading CR/LF/Space
	// 
	int i=0,j=0;
	for (i=0;i<len;i++) {
		if ('\0' == buffer[i]) break;
		if (' ' == buffer[i] && 0 == j) continue;
		if ('\r' == buffer[i]) continue;
	//	if ('\n' == buffer[i] && 0 == j) continue;
		if ('\n' == buffer[i]) continue;
		buffer[j++] = buffer[i];
	}
	//
	// Remove trailing CR/LF/Space
	//
	do {
		buffer[j--] = '\0';
	} while (0 < j && ' ' == buffer[j]);

	return buffer;
}

std::string trimRight(std::string s) {
	for (int i=(int)(s.size() -1);i>=0;i--) {
		const uint8_t c = (uint8_t)s.at((unsigned)i);
		if (0x20 <= c && c < 0x7F) { break; }
		s.erase((std::string::size_type)i);
	}
    return s;
}





//
//
//
#include <sys/stat.h>

bool fileExists(const char *const file) {
	struct stat st;
	if (0 != stat(file,&st)) return false;
    return 0 != (st.st_mode & S_IFREG);
}

std::string readFile(const char *const path) {

    std::string s;

    FILE *const f = fopen(path,"r");
    if (NULL == f) return s;
    
    while (true) {
        char buf[1024];
        const size_t r = fread(buf,1,sizeof(buf),f);
        if (0 >= r) break;
        buf[r] = '\0';
        s.append(buf);
    }
    fclose(f);
    return s;
}



//
// Exec
//

#include <array>
#include <memory>

std::string exec(const char *const cmd) {
	//const uint64_t start = getTickUs();

    std::array<char,128> buffer;
    std::string result;

    std::shared_ptr<FILE> pipe(popen(cmd,"r"),pclose);
    if (!pipe) {
    	//LOGV("popen failed cmd:%s",cmd);
    	return result;
    }

    while (!feof(pipe.get())) {
        if (nullptr != fgets(buffer.data(),128,pipe.get())) {
            result += buffer.data();
        }
    }

    //LOGV("exec %dus \"%s\" \"%s\"",(unsigned)(getTickUs() - start),cmd,result.c_str());
    return result;
}



//
//
//
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// min size INET_ADDRSTRLEN or INET6_ADDRSTRLEN
char *getIpAddress(char *const buffer) {
	struct ifaddrs *ifAddrStruct = NULL;
	getifaddrs(&ifAddrStruct);

	struct ifaddrs *ifa;
	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		//LOGV("check %s",ifa->ifa_name);

		// Looking for ethernet
		if ('l' == ifa->ifa_name[0] && 'o' == ifa->ifa_name[1]) continue;

	/*
		if (0 != strcasecmp((char*)"en0",ifa->ifa_name) && 0 != strcasecmp((char*)"en1",ifa->ifa_name)
			&&
			0 != strcasecmp((char*)"eth0",ifa->ifa_name) && 0 != strcasecmp((char*)"eth1",ifa->ifa_name)) continue;
			&&
			0 != strcasecmp((char*)"wlan0",ifa->ifa_name) && 0 != strcasecmp((char*)"wlan1",ifa->ifa_name)) continue;
	*/

		// is IP4
		if (AF_INET == ifa->ifa_addr->sa_family) {
			//LOGV("ip4 %s",ifa->ifa_name);
			void *const p = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			inet_ntop(AF_INET,p,(char*)buffer,INET_ADDRSTRLEN);

			//LOGV("AF_INET  %s IP Address %s",ifa->ifa_name, buffer);
			break;
		}

		// is IP6
		if (AF_INET6 == ifa->ifa_addr->sa_family) {
			//LOGV("ip6 %s",ifa->ifa_name);
			void *const p = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
			inet_ntop(AF_INET6,p,(char*)buffer,INET6_ADDRSTRLEN);

			//LOGV("AF_INET6 %s IP Address %s",ifa->ifa_name, buffer);
			continue;
		}
	}

	if (NULL != ifAddrStruct) {
		freeifaddrs(ifAddrStruct);
	}

	return buffer;
}




//
// http://fileformats.archiveteam.org/wiki/Mime.types
// https://pki-tutorial.readthedocs.io/en/latest/mime.html

/*
application/pkcs8                   .p8  .key
application/pkcs10                  .p10 .csr
application/pkix-cert               .cer
application/pkix-crl                .crl
application/pkcs7-mime              .p7c

application/x-x509-ca-cert          .crt .der
application/x-x509-user-cert        .crt
application/x-pkcs7-crl             .crl

application/x-pem-file              .pem
application/x-pkcs12                .p12 .pfx

application/x-pkcs7-certificates    .p7b .spc
application/x-pkcs7-certreqresp     .p7r
*/

const char *getMimeType(const char *const file) {

	char f[512] = {0}; strncpy(f,file,sizeof(f)-1);
	char *ext = NULL;

	{	// Find last period in file
		char *e = strtok(f,".");
		while (NULL != e) {
			ext = e;
			e = strtok(NULL,".");
		}
	}

	LOGC("getMimeType file:%s ext:%s",file,ext);

	if (NULL != ext) {

		//
		// Media
		//
		//if (NULL == ext) return "video/mp4";
		if (0 == strcmp(ext,"mp4" )) return "video/mp4";
		if (0 == strcmp(ext,"m4a" )) return "video/mp4";
		if (0 == strcmp(ext,"mov" )) return "video/mov";

		if (0 == strcmp(ext,"wav" )) return "audio/wav";

		//
		// Images
		//
		if (0 == strcmp(ext,"jpg" )) return "image/jpeg";
		if (0 == strcmp(ext,"gif" )) return "image/gif";
		if (0 == strcmp(ext,"png" )) return "image/png";
		if (0 == strcmp(ext,"ico" )) return "image/x-icon";
		if (0 == strcmp(ext,"svg" )) return "image/svg+xml";

		//
		// Web app
		//
		if (0 == strcmp(ext,"html"    )) return "text/html";
		if (0 == strcmp(ext,"css"     )) return "text/css";
		if (0 == strcmp(ext,"txt"     )) return "text/plain";

		if (0 == strcmp(ext,"js"      )) return "text/javascript";
		if (0 == strcmp(ext,"json"    )) return "application/json";
		if (0 == strcmp(ext,"xml"     )) return "text/xml";

		if (0 == strcmp(ext,"webmanifest")) return "application/manifest+json";
		if (0 == strcmp(ext,"manifest")) return "application/x-web-app-manifest+json";

//		if (0 == strcmp(ext,"pem"     )) return "application/x-x509-ca-cert";
		if (0 == strcmp(ext,"pem"     )) return "application/x-pem-file";
//		if (0 == strcmp(ext,"der"     )) return "application/x-x509-ca-cert";
//		if (0 == strcmp(ext,"crt"     )) return "application/x-x509-ca-cert";
		if (0 == strcmp(ext,"crt"     )) return "application/x-x509-user-cert";
		if (0 == strcmp(ext,"key"     )) return "application/pkcs8";
		if (0 == strcmp(ext,"csr"     )) return "application/pkcs10";

		if (0 == strcmp(ext,"pfx"     )) return "application/x-pkcs12";
		if (0 == strcmp(ext,"p12"     )) return "application/x-pkcs12";

		LOGV("getMimeType unhandled file:%s ext:%s",file,ext);
	}

	return "application/octet-stream";
}





//
// Base84 encode / decode
//

static char encoding_table[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
static unsigned mod_table[] = { 0,2,1 };

char *base64_encode(const uint8_t *data,const size_t input_length,size_t *output_length) {

    *output_length = 4 * ((input_length + 2) /3);

    char *const encoded_data = (char*)malloc(*output_length +1);
    if (encoded_data == NULL) return NULL;
    
    encoded_data[*output_length] = 0;

    unsigned i,j;
    for (i=0,j=0; i<input_length;) {

        const uint32_t octet_a = i < input_length ? data[i++] : 0;
        const uint32_t octet_b = i < input_length ? data[i++] : 0;
        const uint32_t octet_c = i < input_length ? data[i++] : 0;

        const uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (i=0; i<mod_table[input_length % 3]; i++) {
        encoded_data[*output_length -1 -i] = '=';
	}

    return encoded_data;
}

