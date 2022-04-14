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

// AT-commands (v.25ter)

#include "utils.h"
#include "modem.h"

#include <unistd.h> // write,read
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <errno.h>

static const bool MODEM_DEBUG = false;

static char* mtrim(char *buffer);


//
//
//
static const unsigned RING_TIMEOUT  = 1000;
static const unsigned READ_TIMEOUT  = 1000;
static const unsigned POLL_TIMEOUT  = 3000;
static const unsigned VOICE_TIMEOUT = 15000;


//
//
//

Modem::Modem() :
	modemFd(-1),offHook(false),dtmfActive(false),
	lastCallRings(0),lastDCEcode(0),lastDTMFcode(0),lastCallerIdLoaded(false),
	parseBufferLstIsDLE(false),
	totalBytesOut(0),totalBytesIn(0),
	totalCalls(0),totalCallRings(0),totalCallsAnswered(0),totalCallSeconds(0)
{
	if (MODEM_DEBUG) LOGT("construct");
}

Modem::~Modem() {
	if (MODEM_DEBUG) LOGT("destruct");
	close();
}


//
//
//

bool Modem::open(const char *const serialDevice) {
	LOGT("%s",serialDevice);

 	strcpy(modemDevice,serialDevice);

	modemFd = ::open(serialDevice,O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (0 > modemFd) {
		perror(serialDevice);
		modemFd = -1;
		return false;
	}

	LOGC("fd:%d dev:%s",modemFd,serialDevice);

	//
	pollFDs[0].fd     = modemFd;
	pollFDs[0].events = POLLIN;

	//
	//
	// setup terminal
	//  https://www.gnu.org/software/libc/manual/html_node/Control-Modes.html
	//  https://en.wikibooks.org/wiki/Serial_Programming/termios
	//  https://www.mkssoftware.com/docs/man5/struct_termios.5.asp
	//
	//
	{
		// set raw input, 0 second timeout
		struct termios a;
		tcgetattr(modemFd,&a);

		//
		// non-blocking
		//
		a.c_cc[VMIN]  = 0;
		a.c_cc[VTIME] = 0;

		//
		// Control Modes
		//
		// CLOCAL	Ignore modem status lines
		// CREAD	Enable receiver
		// CSIZE	Number of bits per byte CS5 CS6 CS7 CS8
		// CSTOPB	Send two stop bits else one
		// HUPCL	Hang up on last close
		// PARENB	Parity enable
		// PARODD	Odd parity else even
		//
		// CRTSCTS  flow control
		//
		a.c_cflag |= (unsigned long) (CLOCAL | CREAD | CS8 | CRTSCTS);

		//
		// Local Mode
		//
		// ECHO 	Enable echo
		// ECHOE	Echo ERASE as an error-correcting backspace
		// ECHOK	Echo KILL
		// ECHONL	Echo \n
		// ICANON	Canonical input (erase and kill processing)
		// IEXTEN	Enable extended functions
		// ISIG 	Enable signals
		// NOFLSH	Disable flush after interrupt, quit or suspend
		// TOSTOP	Send SIGTTOU for background output
		//
		a.c_lflag &= ~(unsigned long)(ICANON | ECHO | ECHOE | ISIG);

		//
		// Input modes
		//
		// BRKINT	Signal interrupt on break
		// ICRNL	Map CR to NL on input
		// IGNBRK	Ignore break condition
		// IGNCR	Ignore CR
		// IGNPAR	Ignore characters with parity errors
		// INLCR	Map NL to CR on input
		// INPCK	Enable input parity check
		// ISTRIP	Strip character
		// IXOFF	Enable start/stop input control
		// IXON 	Enable start/stop output control
		// PARMRK	Mark parity errors
		//
		a.c_iflag = IGNPAR;

		//
		// Output Modes
		//
		// OPOST	Perform output processing
		// OLCUC	Map lower case to upper on output
		// ONLCR	Map NL to CR-NL on output
		// OCRNL	Map CR to NL on output
		// ONOCR	No CR output at column 0
		// ONLRET	NL performs CR function
		// OFILL	Use fill characters for delay
		// OFDEL	Fill is DEL else NUL
		// NLDLY	Select new-line delays: NL0 NL1
		// CRDLY	Select carriage-return delays: CR0 CR1 CR2 CR3
		// TABDLY	Select horizontal-tab delays: TAB0 TAB1 TAB2
		// XTABS	Expand tabs to spaces
		// BSDLY	Select backspace delays: BS0 BS1
		// VTDLY	Select vertical tab delays: VT0 VT1
		// FFDLY	Select form-feed delays: FF0 FF1
		//
		a.c_oflag &= ~(unsigned long)(OPOST);

		tcflush(modemFd,TCIFLUSH);
		tcsetattr(modemFd,TCSANOW,&a);
	}

	callReset();
	reset();

	return true;
}

void Modem::close() {
	LOGT(" ");

	if (-1 == modemFd) return;

	LOGC("h:%d",modemFd);

	const int fd = modemFd;
	modemFd = -1;

	::close(fd);
}



void Modem::reset() {
	LOGC(" ");

	cmd("\x10""!""\x10""\x03","OK");

	static const char *const cfgs[] = {
		 "ATZ"       // reset
	//	,"ATZ3"      // factory reset

	//	,"ATE0"      // echo disable

		,"ATV1"      // verbose errors in text
		,"AT+VCID=1" // caller id: 0=off,1=formatted,2=formatted

	//	,"AT&C1"     // carrier detect normal (modem data ?)
	//	,"ATX4"      // extended result code with dial & busy tone detect
	};

	cmd(COUNT_OF(cfgs),cfgs);		
}




//
//
//
bool Modem::answer(unsigned len,const char *const *cmds) {
	if (!cmd(len,cmds)) {
		LOGC("answer failed");
		return false;
	}

	tickOffHook = getTickMs();

	offHook = true;
	totalCallsAnswered++;

	return true;
}

void Modem::hangup() {
	offHook = false;

	if (!cmd(5000,"ATH0")) {
		LOGC("hangup failed");

		//
		// TODO reset device?
		//
		//return;
	}

	if (0 < tickOffHook) {
		totalCallSeconds += (unsigned)(getTickMs() - tickOffHook) /1000;
		tickOffHook = 0;
	}

	callReset();
}

//
//
//
void Modem::callReset() {
	LOGC(" ");

	offHook = false;
	dtmfActive = false;
	lastDCEcode = 0;
	lastDTMFcode = 0;

	lastCallRings = 0;
	lastCallerIdLoaded = false;

	memset(lastCallerIdBuf ,'\0',sizeof(lastCallerIdBuf));
	memset(lastCallerId    ,'\0',sizeof(lastCallerId));

	memset(lastCallerDate  ,'\0',sizeof(lastCallerDate));
	memset(lastCallerTime  ,'\0',sizeof(lastCallerTime));
	memset(lastCallerNumber,'\0',sizeof(lastCallerNumber));
	memset(lastCallerName  ,'\0',sizeof(lastCallerName));	
}

void Modem::dtmfReset() {
	LOGC(" ");
	dtmfActive = false;
	lastDTMFcode = 0;
}

//
//
//
void Modem::parseCallerId() {
	memset(lastCallerDate  ,'\0',sizeof(lastCallerDate));
	memset(lastCallerTime  ,'\0',sizeof(lastCallerTime));
	memset(lastCallerNumber,'\0',sizeof(lastCallerNumber));
	memset(lastCallerName  ,'\0',sizeof(lastCallerName));	

	char *tptr;
	char *toks = strtok_r(lastCallerIdBuf,"\n",&tptr);

	while (toks) {

		char *stptr;
		char *stoks = strtok_r(toks,"=",&stptr);

		char t[128]; strncpy(t,stoks,sizeof(t)-1);
		mtrim(t);

		//LOGD("parse key:'%s'",t);
		//LOGD("stptr: %s\n",stptr);

		if (0 == strcmp("DATE",t)) {
			time_t tm; time(&tm);
			struct tm *const tmi = localtime(&tm);
			char *const st = strtok_r(NULL,"=",&stptr);
			if (NULL == st) {
				LOGC("strtok_r returned NULL");
			} else {
				snprintf(lastCallerDate,sizeof(lastCallerDate)-1,"%d%s",1900+tmi->tm_year,mtrim(st));
			}
		} else
		if (0 == strcmp("TIME",t)) {
			strncpy(lastCallerTime,mtrim(strtok_r(NULL,"=",&stptr)),sizeof(lastCallerTime)-1);
		} else
		if (0 == strcmp("NMBR",t)) {
			strncpy(lastCallerNumber,mtrim(strtok_r(NULL,"=",&stptr)),sizeof(lastCallerNumber)-1);
		} else
		if (0 == strcmp("NAME",t)) {
			strncpy(lastCallerName,mtrim(strtok_r(NULL,"=",&stptr)),sizeof(lastCallerName)-1);
		} else {
			LOGC("parse error '%s'",t);
		}

		toks = strtok_r(NULL,"\n",&tptr);
	}

	snprintf(lastCallerId,sizeof(lastCallerId)-1,"\"date\":\"%s\",\"time\":\"%s\",\"number\":\"%s\",\"name\":\"%s\"",lastCallerDate,lastCallerTime,lastCallerNumber,lastCallerName);

	LOGD("CallerId: %s",lastCallerId);
}

//
//
//
int Modem::waitForRing() {
	if (0 > modemFd) { LOGC("modemFd is zero"); return -1; }

	uint8_t buf[256+1] = {0}; //buf[0] = '\0';

	const int r = readBuffer(false,RING_TIMEOUT,sizeof(buf) -1,buf);

	if (0 > r) { LOGC("readBuffer returned -1"); return -1; }

	if (0 == r) {

		if (0 != lastDCEcode) {
			LOGC("DCE:%d (%c)",lastDCEcode,(char)lastDCEcode);

			const int c = lastDCEcode;
			lastDCEcode = 0;

			if ('R' == c) {
				totalCallRings++;
				if (0 == lastCallRings) totalCalls++;
				lastCallRings++;
				return 1;
			}
		}

		return 0;
	}

	LOGC("waitForRing read:%d",r);

	//
	// Filter NUL character avoiding premature string termination
	//
	for (int i=0;i<r;i++) {
		if ('\0' == buf[i]) { buf[i] = ' '; }
	}

	// String termination
	buf[r] = '\0';

	if (LOGGING_DATA) dump((unsigned)r,buf);

	// Remove leading CR/LF and trailing LF
	mtrim((char*)buf);

	LOGC("buf:%s",buf);
	if (MODEM_DEBUG) LOGC("DCE:%d (%c) buf:%s",lastDCEcode,(char)lastDCEcode,buf);

	//
	// A string was received
	//
	if ('R' == lastDCEcode || NULL != strstr((char*)buf,"RING")) {
		totalCallRings++;
		if (0 == lastCallRings) totalCalls++;
		lastCallRings++;
		return 1;
	}

	if (NULL != strstr((char*)buf,"DATE")) {
		strncpy(lastCallerIdBuf,(char*)buf,sizeof(lastCallerIdBuf)-1);

		parseCallerId();
		lastCallerIdLoaded = true;
	}

	return 0;
}




//
//
//
int Modem::write(const unsigned len,const uint8_t *const buf) {
	if (0 > modemFd) { LOGC("modemFd is zero"); return -1; }

	int offset = 0;

	while (len > (unsigned)offset) {

		const ssize_t w = ::write(modemFd,buf+offset,len-(unsigned)offset);

		if (0 < w) {
			offset += w;
			continue;
		}

		if (0 > w) {
			if (EAGAIN != errno) {
				LOGC("write failed -1");
				return -1;
			}
			if (MODEM_DEBUG) LOGC("write failed EAGAIN");
		}

		totalBytesOut += (uint64_t)w;

		while (true) {
			struct pollfd pfd[1];
			pfd[0].fd     = modemFd;
			pfd[0].events = POLLOUT;

			const int pr = ::poll(pfd,1,100);

			if (MODEM_DEBUG) LOGC("write poll r:%d revents:0x%02X",pr,pfd[0].revents);

			if (0 == pr) continue;

			if (0 > pr) {
				LOGC("write poll r:%d",pr);
				return false;
			}

			if (1 == pr && POLLOUT != pfd[0].revents) {
				LOGC("write poll error r:%d revents:0x%02X",pr,pfd[0].revents);
				return false;
			}
			break;
		}

	//	if (w != (int)i) { LOGC("write failed"); break; }
	}

	return offset;
}



//
//
//
bool Modem::cmd(const unsigned len,const char *const *const cmds) {
	bool r = true;
	unsigned i;
	for (i=0;i<len;i++) {
		if (!cmd(POLL_TIMEOUT,cmds[i],"OK")) {
			LOGC("command '%s' failed",cmds[i]);
			r = false;
		}
	}
	return r;
}

bool Modem::cmd(const char *const atCmd,const char *const response) {
	return cmd(READ_TIMEOUT,atCmd,response);
}

bool Modem::cmd(const unsigned timeout,const char *const atCmd,const char *const response) {
	if (0 > modemFd) { LOGC("modemFd is zero"); return -1; }

	LOGC("%s send timeout:%d",atCmd,timeout);

#ifdef DEBUG
	if (LOGGING_DATA) dump((unsigned)strlen(atCmd),(uint8_t*)atCmd);
#endif //DEBUG

	const uint64_t start = getTickUs();

	//tcflush(modemFd,TCOFLUSH);
	//tcflush(modemFd,TCIFLUSH);
	//tcflush(modemFd,TCIOFLUSH);

	waitAndRead(30);

	const int w = dprintf(modemFd,"%s\r",atCmd);
	totalBytesOut += (uint64_t)w;

	const bool rr = cmdRsp(timeout,atCmd,response);

	LOGC("%s %s for:%s wait:%d after:%dms",NULL==atCmd?"":atCmd,rr?"SUCCESS":"FAILED",response,timeout,(unsigned)((getTickUs() - start)/1000));

	return rr;
}

bool Modem::cmdRsp(const unsigned timeout,const char *const atCmd,const char *const response) {
	LOGC("cmd:%s t:%d",atCmd,timeout);

	const uint64_t start = getTickUs();

	{
		const int pr = ::poll(pollFDs,1,(int)timeout);

		if (0 >= pr) {
			LOGC("poll r:%d timeout:%dms after:%dms",pr,timeout,(unsigned)((getTickUs() - start) /1000));
			return false;
		}

		if (1 == pr && POLLIN != pollFDs[0].revents) {
			LOGC("poll error r:%d revents:0x%02X",pr,pollFDs[0].revents);
			return false;
		}
	}

	uint8_t buf[512] = {0};
	unsigned offset = 0;

	while (offset < sizeof(buf)) {
		const unsigned elapsed = (unsigned)((getTickUs() - start) /1000);
		const unsigned wait = timeout <= elapsed ?0 :timeout - elapsed;

		const unsigned chunk = sizeof(buf) - offset;
		ssize_t r = ::read(modemFd,&buf[offset],chunk);

		LOGD("read r:%d chunk:%d offset:%d",(int)r,chunk,offset);

		if (0 > r) {
			if (EAGAIN != errno) {
				LOGC("errno not EAGAIN errno:%d after:%dms",errno,(unsigned)((getTickUs() - start) /1000));
				break;
			}

			if (MODEM_DEBUG) LOGC("read EAGAIN, set to zero to read again");
			r = 0;
		}

		if (0 == wait) {
			LOGC("wait:%d offset:%d r:%d wait:%dms after:%dms",wait,offset,r,wait,(unsigned)((getTickUs() - start) /1000));
			break;
		}

		//
		// Keep checking
		//
		if (0 == r) {
			const int pr = ::poll(pollFDs,1,(int)wait);

			if (0 > pr) {
				LOGC("poll r:%d",pr);
				return false;
			}
			if (1 == pr && POLLIN != pollFDs[0].revents) {
				LOGC("poll error revents:0x%02X",pollFDs[0].revents);
				return false;
			}

			LOGC("poll r:%d revents:0x%02X t:%dms after:%dms",pr,pollFDs[0].revents,wait,(unsigned)((getTickUs() - start) /1000));

			if (0 < pr) continue;
		}

		//
		// More data to check
		//
		if (0 < r) {
			totalBytesIn += (uint64_t)r;
		}

		LOGC("r:%d offset:%d",r,offset);
	#ifdef DEBUG
		if (LOGGING_DATA) dump(offset+(unsigned)r,buf);
	#endif //DEBUG

		if (0 == offset + r) {
			LOGC("(offset +r) is zero, keep listening");
			continue;
		}

		//
		//
		//
		parseBuffer(offset +(unsigned)r,buf,offset,NULL);
		LOGC("after parseBuffer offset:%d",offset);

		if (0 == offset) {
			LOGC("offset is zero, keep listening");
			continue;
		}

		//
		// Quick find first two characters of response
		//
		bool found = false;
		unsigned i;
		for (i=0;i<offset-1;i++) {
			if (response[0] == buf[i] && response[1] == buf[i+1]) {
				found = true;
				break;
			}
		}

		if (MODEM_DEBUG) LOGC("%s %s i:%d r:%d '%s' = '%s'",found?"FOUND":"NOT_FOUND",response,i,r,buf+i,response);

		//
		// Find all of response
		//
		if (found) {
			//LOGC("strstr '%s' == '%s'",buf+i,response);
			if (NULL == strstr((char*)buf+i,response)) {
				LOGC("strstr failed '%s' != '%s'",buf+i,response);
				if (LOGGING_DATA) dump(offset,buf);
				found = false;
			} else {

				//
				// Success
				//
				LOGC("match %s for:%s r:%d wait:%d after:%dms offset:%d",NULL==atCmd?"":atCmd,response,r,wait,(unsigned)((getTickUs() - start)/1000),offset);
			//	if (LOGGING_DATA) dump(offset,buf);
				return true;
			}
		}


		//
		// Quick find ERROR response
		//
		if (4 <= offset) {
			for (i=0;i<offset-4;i++) {
				if ('E' != buf[i]) continue;
				if ('R' != buf[i+1]) continue;
				if ('R' != buf[i+2]) continue;
				if ('O' != buf[i+3]) continue;
				if ('R' != buf[i+4]) continue;
				found = true;
				break;
			}
			if (found) {
				LOGC("found ERROR");
				if (LOGGING_DATA) dump(offset,buf);
				break;
			}
		}

		//
		// Check timeout
		//
		if (timeout < (unsigned)((getTickUs() - start) /1000)) {
			LOGC("timeout t:%d ~ %d offset:%d",timeout,(unsigned)((getTickUs() - start) /1000),offset);
			break;
		}

		//
		// Buffer overflow recycles buffer
		//
		if (sizeof(buf) <= offset) {
			LOGC("flush buffer : %d <= %d",sizeof(buf),offset);
			offset = 0;
		}
	}

	//
	// Failed
	//
#ifdef DEBUG
	LOGC("no match %s for:%s offset:%d timeout:%d after:%dms",atCmd,response,offset,timeout,(unsigned)((getTickUs() - start) /1000));
	if (0 < offset && LOGGING_DATA) dump(offset,buf);
#endif //DEBUG

	return false;
}



//
//
//

void Modem::waitAndRead(const unsigned ms) {
	LOGC("ms:%d +",ms);

	const uint64_t st = getTickMs();
	const uint64_t end = ms + st;

	unsigned t = 0,d = 0;

	while (true) {
		uint8_t buf[512] = {0};

		const int r = readBuffer(true,(unsigned)(ms - d),sizeof(buf),buf);
		if (0 >= r) break;
		t += (unsigned)r;

		LOGC("readBuffer r:%d dce:%c (0x%02X)",r,lastDCEcode,lastDCEcode);

	#ifdef DEBUG
		//if (LOGGING_DATA) dump(r,buf);
	#endif //DEBUG

		{
			unsigned off = 0; // offset not important
			parseBuffer((unsigned)r,buf,off,NULL);
		}

	//	if (LOGGING_DATA) { LOGV("waitAndRead"); dump(r,buf); }

		const uint64_t now = getTickMs();
		if (end > now) { d = (unsigned)(now - st); continue; }

		break;
	}

	LOGC("ms:%d after:%d total:%d -",ms,(unsigned)(getTickMs()-st),t);
}



//
//
//
static bool isDCECode(uint8_t c);

#define DCE_DTMF_START '/' //0x2F
#define DCE_DTMF_END   '~' //0x7E


/*

#define POLLIN          0x0001
#define POLLPRI         0x0002
#define POLLOUT         0x0004
#define POLLERR         0x0008
#define POLLHUP         0x0010
#define POLLNVAL        0x0020

*/

int Modem::readBuffer(const bool keepReading,const unsigned ms,const unsigned len,uint8_t *const buf) {
	if (0 > modemFd) { LOGC("modemFd is zero"); return -1; }

	if (MODEM_DEBUG) LOGD("readBuffer len:%d ms:%d +",len,ms);

	lastDCEcode = 0;

	const uint64_t st = getTickMs();

	{
		const int pr = ::poll(pollFDs,1,(int)ms);

		if (0 >= pr) {
			//LOGC("poll r:%d",pr);
			return 0;
		}
		if (1 == pr && POLLIN != pollFDs[0].revents) {
			LOGC("poll error revents:0x%02X",pollFDs[0].revents);
			return -1;
		}

		LOGC("readBuffer poll ms:%d r:%d revents:0x%02X",ms,pr,pollFDs[0].revents);
	}


	//
	//
	//
	bool interrupted = false;
	unsigned offset = 0;


	//
	// fill buffer
	//
	while (!interrupted && offset < len) {
		uint8_t tmp[1000];// = {0};

		// whats left?
		const unsigned chunk = sizeof(tmp) < len - offset ?sizeof(tmp) :len - offset;
		ssize_t r = ::read(modemFd,tmp,chunk);

		if (MODEM_DEBUG) LOGD("read r:%d chunk:%d offset:%d",(int)r,chunk,offset);

		if (0 > r) {
			if (EAGAIN != errno) { LOGC("read failed, should never happen!") return -1; }
			LOGC("read EAGAIN %s",keepReading?"keepReading":"");
			r = 0;
		}

		if (0 == r) {
			if (!keepReading) break;

			const unsigned el = (unsigned)(getTickMs() - st);
			const unsigned t = ms > el ?ms - el :10;

			const int pr = ::poll(pollFDs,1,(int)t);

			if (0 >= pr) {
				LOGC("poll r:%d",pr);
				break;
			}
			if (1 == pr && POLLIN != pollFDs[0].revents) {
				LOGC("poll error revents:0x%02X",pollFDs[0].revents);
				return -1;
			}

			if (MODEM_DEBUG) LOGC("poll offset:%d t:%d",offset,t);

			continue;
		}

		totalBytesIn += (uint64_t)r;

	#ifdef DEBUG
	//	if (LOGGING_DATA) dump(r,tmp);
	#endif //DEBUG

		if (parseBuffer((unsigned)r,tmp,offset,buf)) {
			interrupted = true;
		}
	}

	if (MODEM_DEBUG) LOGD("readBuffer len:%d ms:%d -",offset,(unsigned)(getTickMs() - st));

	return (int)offset;
}

bool Modem::parseBuffer(const unsigned inlen,const uint8_t *const inbuf,unsigned &outIdx,uint8_t *const buf) {
	if (MODEM_DEBUG) LOGC("parseBuffer off:%d inlen:%d +",outIdx,inlen);

	bool interrupted = false;

	//
	// move in -> out filtering DCE codes
	//

	for (unsigned i=0;i<inlen;i++) {
		uint8_t ch = inbuf[i];


		//
		// Overflow catch flow ...
		//

		if (parseBufferLstIsDLE) {
			LOGC("parseBufferLstIsDLE = FALSE i:%d off:%d ch:0x%02X",i,outIdx,ch);
			parseBufferLstIsDLE = false;

			if (dtmfActive) {
				LOGC("dtmfActive i:%d off:%d ch:0x%02X",i,outIdx,ch);

				// faster than isDCECode

				if (('0' <= ch && ch <= '9') ||
					('A' <= ch && ch <= 'D') ||
					 '#' == ch || '*' == ch) {

					if (0 == lastDTMFcode) {
						LOGC("dtmf start to:%c",lastDTMFcode,ch);
						lastDTMFcode = ch;
					} else 
					if (ch != lastDTMFcode) {
						LOGC("dtmf change fr:%c to:%c",lastDTMFcode,ch);
						lastDTMFcode = ch;
					}

				} else {
					LOGC("invalid dtmf:%c",ch);
				}

			 } else {

				if (0x10 == ch) {
					//LOGC("0x10 0x10 = TRUE, skipping one at i:%d inlen:%d off:%d",i,inlen,outIdx);
					if (buf) buf[outIdx++] = 0x10;
					continue;
				}


				// faster than isDCECode
			//	if (('0' <= ch && ch <= '9') ||
			//		('A' <= ch && ch <= 'D') ||
			//		 '#' == ch || '*' == ch) {

			//		LOGC("dtmfCode:%c",ch);
			//		lastDTMFcode = ch;
			//	} else {
			//		if (!isDCECode(ch)) { LOGC("not DCECode i:%d inlen:%d off:%d",i,inlen,outIdx); continue; }


					LOGC("dceCode:%c",ch);
					lastDCEcode = ch;
//				}
			}


		} else



		{


			//
			// Normal flow ...
			//
			if (0x10 != ch) {
				if (buf) buf[outIdx] = ch;
				outIdx++;
				continue;
			}




			//
			// Overflow set flow ...
			//
			if (i >= inlen -1) {
				LOGC("parseBufferLstIsDLE = TRUE");
				parseBufferLstIsDLE = true;

				LOGC("wait for matching DLE +");
				while (true) {

					const int pr = ::poll(pollFDs,1,(int)READ_TIMEOUT);

					if (0 > pr) {
						LOGC("poll",pr);
						return false;
					}

					if (1 == pr && POLLIN != pollFDs[0].revents) {
						LOGC("poll error revents:0x%02X",pollFDs[0].revents);
						return false;
					}

					if (0 < pr) break;
					LOGC("poll timeout");
				}
				LOGC("wait for matching DLE -");

				LOGC("read poll revents:0x%02X",pollFDs[0].revents);

				continue;
			}




			//
			// found 0x10, check next character
			//
			ch = inbuf[++i];

			if (0x10 == ch) {
				//LOGC("0x10 0x10 = TRUE, skipping one at i:%d inlen:%d off:%d",i,inlen,outIdx);
				if (buf) buf[outIdx] = 0x10;
				outIdx++;
				continue;
			}

			//
			// DTMF life-cycle
			//
			if (DCE_DTMF_START == ch) {
				LOGC("dtmfActive TRUE");
				dtmfActive = true;

				continue;
			}

			if (DCE_DTMF_END == ch) {
				LOGC("dtmfActive FALSE");
				dtmfActive = false;

				lastDTMFcode = 0;

				interrupted = true;
				continue;
			}

			//
			// DTMF life-cycle
			//
			if (dtmfActive) {
				//LOGC("dtmfCode:%c",ch);

				if (('0' <= ch && ch <= '9') ||
					('A' <= ch && ch <= 'D') ||
					 '#' == ch || '*' == ch) {

					if (0 == lastDTMFcode) {
						LOGC("dtmf start to:%c",ch);
						lastDTMFcode = ch;

						interrupted = true;
						continue;
					} else 
					if (ch != lastDTMFcode) {
						LOGC("dtmf change fr:%c (0x%02X) to:%c (0x%02X)",lastDTMFcode,lastDTMFcode,0==ch?' ':ch,ch);
						lastDTMFcode = ch;

						interrupted = true;
						continue;
					}

					continue;

				} else {
					LOGC("invalid dtmf:%c",ch);
				}
			}

			//
			// DCE codes
			//
			if (!isDCECode(ch)) { LOGC("not DCECode ch:%c (0x%02X) i:%d inlen:%d off:%d",0==ch?' ':ch,ch,i,inlen,outIdx); continue; }

			LOGC("dceCode:%c (0x%02X)",0==ch?' ':ch,ch);
			lastDCEcode = ch;

			interrupted = true;
			continue;
		}
	}

	if (MODEM_DEBUG) LOGC("outIdx:%d interrupted:%d -",outIdx,interrupted);

	return interrupted;
}

//
//
//
char* mtrim(char *const buffer) {
	const int len = (int)strlen(buffer);
	if (0 >= len) return buffer;

	//
	// Remove leading CR/LF/Space
	// 
	int i=0,j=0;
	for (i=0;i<len;i++) {
		if ('\0' == buffer[i]) break;
		if (' ' == buffer[i] && 0 == j) continue;
		if ('\r' == buffer[i]) continue;
		if ('\n' == buffer[i] && 0 == j) continue;
	//	if ('\n' == buffer[i]) continue;
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


//
//
//
bool isDCECode(const uint8_t c) {
	const bool dbg = LOGGING_CONTROL;
//	if (0x10 == c) { if (dbg) LOGV("DCE Code: <DLE><DLE>"); return true; }
	if (0x03 == c) { if (dbg) LOGV("DCE Code: <DLE><ETX>"); return true; } // end of voice transmission

	if ('d'  == c) { if (dbg) LOGV("DCE Code: <DLE>d DIAL tone"); return true; }
	if ('R'  == c) { if (dbg) LOGV("DCE Code: <DLE>R RING tone"); return true; }
	if ('a'  == c) { if (dbg) LOGV("DCE Code: <DLE>a ANSWER tone"); return true; }
	if ('c'  == c) { if (dbg) LOGV("DCE Code: <DLE>c FAX tone"); return true; }
	if ('e'  == c) { if (dbg) LOGV("DCE Code: <DLE>e DATA tone"); return true; }

	if ('h'  == c) { if (dbg) LOGV("DCE Code: <DLE>h Local phone ON-HOOK"); return true; }
	if ('H'  == c) { if (dbg) LOGV("DCE Code: <DLE>H Local phone OFF-HOOK"); return true; }

	if ('s'  == c) { if (dbg) LOGV("DCE Code: <DLE>s SILENCE"); return true; }
	if ('b'  == c) { if (dbg) LOGV("DCE Code: <DLE>b BUSY tone"); return true; }

	if ('@'  == c) { if (dbg) LOGV("DCE Code: <DLE>@ CAS tone"); return true; }

	//
	if ('/'  == c) { if (dbg) LOGV("DCE Code: <DLE>/ start DTMF tone"); return true; }
	if ('~'  == c) { if (dbg) LOGV("DCE Code: <DLE>~ end   DTMF tone"); return true; }

	if ('0' <= c && c <= '9') { if (dbg) LOGV("DCE Code: <DLE>%c DTMF '%c'",c,c); return true; }
	if ('A' <= c && c <= 'D') { if (dbg) LOGV("DCE Code: <DLE>%c DTMF '%c'",c,c); return true; }

	if ('#' == c || '*' == c) { if (dbg) LOGV("DCE Code: <DLE>%c DTMF '%c'",c,c); return true; }

	if (dbg) LOGV("DCE Code: %c (0x%02X)",c,c);

	return false;
}


//
//
//

Modem_VRX::Modem_VRX(Modem &m) : modem(m) {}
bool Modem_VRX::start() const {
	LOGC("VRX start");
	if (!modem.cmd(VOICE_TIMEOUT,"AT+VRX","CONNECT")) {
		LOGC("VRX start failed");
		return false;
	}
	return true;
}
void Modem_VRX::stop() const {
	LOGC("VRX stop : <DLE>!<DLE><ETX>");
	if (!modem.cmd(VOICE_TIMEOUT,"\x10""!""\x10""\x03","OK")) {
		LOGC("VRX stop failed");
	}
	modem.waitAndRead(100);
}

Modem_VTX::Modem_VTX(Modem &m) : modem(m) {}
bool Modem_VTX::start() const {
	LOGC("VTX start");
	if (!modem.cmd(VOICE_TIMEOUT,"AT+VTX","CONNECT")) {
		LOGC("VTX start failed");
		return false;
	}
	return true;
}
void Modem_VTX::stop() const {
	LOGC("VTX stop : <DLE>!<DLE><ETX>");
	if (!modem.cmd(VOICE_TIMEOUT,"\x10""!""\x10""\x03","OK")) {
		LOGC("VTX stop failed");
	}
	modem.waitAndRead(30);
}

/*
Modem_VTR::Modem_VTR(Modem &m) : modem(m) {}
bool Modem_VTR::start() const {
	LOGC("VTR start");
	if (!modem.cmd(VOICE_TIMEOUT,"AT+VTR","CONNECT")) {
		LOGC("VTR start failed");
		return false;
	}
	return true;
}
void Modem_VTR::stop() const {
	LOGC("VTR stop : <DLE>^<DLE><ETX>");
	if (!modem.cmd(VOICE_TIMEOUT,"\x10""^""\x10""\x03","OK")) {
		LOGC("VTR stop failed");
	}
	modem.waitAndRead(100);
}
//*/

