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

#include "data.h"
#include "stats.h"
#include "json.h"
#include "webserver.h"

#include <algorithm> // std::find

#include "lenny.h"
extern void sendStatus(void *sock =NULL);


//
extern Config config;
extern Stats stats;


//
//
//
Lenny::Lenny(const unsigned id) :
	idx(id),
	configLine(NULL),
	outPCM(*this),
	state(CLOSED),mode(READY),
	tickRingLast(0),
	answerModeActive(false),sentCallerId(false),
	dtmfInterrupted(false),dtmfCode(0),dtmfCodeTickStart(0),

	queue("lenny"),
  	waitForCall(*this),waitForCallCheck(*this),
  	offHook(*this),onHook(*this),
	answerWithLenny(*this),
	answerWithDisconnected(*this),
	answerWithDTMF(*this),
	answerWithMessage(*this),
	answerWithRecord(*this)
{
	LOGC("constructor");
}

Lenny::~Lenny() {
	LOGC("destructor +");
	state = CLOSED;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	LOGC("destructor -");
}

//
// Start our thread
//
bool Lenny::start(ConfigLine &cfg) {
	configLine = &cfg;

	if (!modem.open(configLine->device)) {
		LOGV("modem open failed device:%s",configLine->device);
		return false;
	}

	state = ONHOOK;

	queue.start();
	queue.post(&waitForCall);

	return true;
}

//
//
//
void Lenny::handleWaitForCall() {

	while (queue.empty()) {

		const int r = modem.waitForRing();
		if (0 > r) {
			LOGC("modem error, close instance");

			state = CLOSED;
			sendStatus();
			return;
		}

		if (1 == r) {

			//
			// Heard new RING
			//

			LOGC("waitForRing RING:%d",modem.lastCallRings);

			if (RING != state) {
				LOGC("state = RING");
				state = RING;
			}

			tickRingLast = getTickMs();
			sendStatus();

		} else {

			//
			// No new RING yet
			//

			if (RING == state) {
				//LOGV("waitForRing NO RING timeout:%d",(unsigned)(getTickMs() - tickRingLast));

				#define RING_TIME_BETWEEN_RINGS_TIMEOUT 10000 // assume: six seconds between rings

				if (RING_TIME_BETWEEN_RINGS_TIMEOUT < getTickMs() - tickRingLast) {
					LOGC("missing ring timeout, resetting")
					hangUp();

					continue;
				}
			}
		}


		//
		//
		//
		if (modem.lastCallerIdLoaded) {

			//
			// Send caller id
			//
			if (!sentCallerId) {
				sentCallerId = true;

				//
				// YYYYMMDDHHMMSS_N.wav
				//
				snprintf(outFile,sizeof(outFile),"%s/%s%s00_%d.wav",PATH_CALLS,modem.lastCallerDate,modem.lastCallerTime,configLine->line);
				LOGC("CALL id:%s outFile:%s",modem.lastCallerId,outFile);

				addCall(configLine->name,modem.lastCallerId);
				sendStatus();

			} else {

				if (0 == strlen(outFile)) {
					createDefaultFileName();
				}
			}
		}


		//
		// Auto-Answer mode
		//
		if (sentCallerId) {

		//	LOGC("RING:%d messages:%d (rings:%d)",modem.lastCallRings,configLine->tadEnabled,configLine->tadRings);
		//	LOGC("RING:%d config line:%d lenny:%d disconnected:%d messages:%d (rings:%d)",modem.lastCallRings,configLine->line,configLine->lennyEnabled,configLine->disconnectedEnabled,configLine->tadEnabled,configLine->tadRings);

			//
			// Biz-Logic: in skiplist has call go to TAD after number of rings
			//
			if (config.skiplistEnabled && inSkiplist(modem.lastCallerNumber,modem.lastCallerName)) {
			//	LOGV("inSkiplist %s %s",modem.lastCallerNumber,modem.lastCallerName);

				if (configLine->tadRings <= modem.lastCallRings) {
					queue.post(&answerWithMessage);
					return;
				}
			} else

			if (configLine->lennyEnabled) {

				LOGC("LENNY RING:%d",modem.lastCallRings);

				if (1 < modem.lastCallRings) {
					queue.post(&answerWithLenny);
					return;
				}
			} else

			if (configLine->disconnectedEnabled) {
				if (0 < modem.lastCallRings) {
					queue.post(&answerWithDisconnected);
					return;
				}
			} else

			if (configLine->tadEnabled) {
				if (configLine->tadRings <= modem.lastCallRings) {
					queue.post(&answerWithMessage);
					return;
				}
			}
		}
	}
}

//
// YYYYMMDDHHMMSS_N.wav
//  assume seconds (SS) are zero (0)
//
void Lenny::createDefaultFileName() {
	const time_t now = time(0);
	tm *const ltm = localtime(&now);
//	snprintf(outFile,sizeof(outFile),"%s/%d%02d%02d%02d%02d%02d_%d.wav",PATH_CALLS,1900+ltm->tm_year,1+ltm->tm_mon,ltm->tm_mday,ltm->tm_hour,ltm->tm_min,ltm->tm_sec,configLine->line);
	snprintf(outFile,sizeof(outFile),"%s/%d%02d%02d%02d%02d00_%d.wav",PATH_CALLS,1900+ltm->tm_year,1+ltm->tm_mon,ltm->tm_mday,ltm->tm_hour,ltm->tm_min,configLine->line);
	LOGC("DEFAULT: outFile:%s",outFile);
}



//
//
//

void Lenny::answer() {
	LOGC("answer");

	static const char *const cmds[] = {
		 "AT+FCLASS=8"     // voice mode
		,"AT+VSM=128,8000" // 8bit pcm / 8k

		,"AT+VSD=255,5"    // 5=500ms & 255 largest value to treat noisier line conditions as silence
//		,"AT+VSD=0,0"    // off ?

		,"AT+VGR=121"      // rx - gain { 121 - 134 } = lowest for noisy environments

		//
		// TBD are these needed?
		//     do they have an effect?
		//
	//	,"AT+VGT=255"      // speaker volume = output at full possible
	//	,"AT+VGS=128"      // speaker gain
	//	,"AT+VGM=128"      // mic gain


		//
		// off-hook
		//

	//	,"AT+VLS=1"        // speaker-off & mic-off
		,"AT+VLS=5"        // speaker-on & mic-off
	//	,"AT+VLS=7"        // speaker-on & mic-on
	};

	if (!modem.answer(COUNT_OF(cmds),cmds)) {
		LOGV("answer failed");
		return;
	}

	answerModeActive = true;

	LOGC("state = OFF HOOK");
	state = OFFHOOK;
}

void Lenny::hangUp() {
	LOGC("hangUp");

	LOGC("answerModeActive = false");
	answerModeActive = false;

	LOGC("state = ON HOOK");
	state = ONHOOK;

	mode = READY;
	sendStatus();
	sendCalls();

	modem.hangup();

	memset(outFile,0,sizeof(outFile));
	sentCallerId = false;
	tickRingLast = 0;
}




//
//
//

void Lenny::handleCallWithDTMF() {
	LOGC("handleCallWithDTMF");

	if (dtmfInterrupted) {
		dtmfInterrupted = false;

		if ('#' == dtmfCode) {
			// mailbox
		} else
		if ('*' == dtmfCode) {
			// control
		} else
		if (('0' <= dtmfCode && dtmfCode <= '9') ||
			('A' <= dtmfCode && dtmfCode <= 'D')) {
			// code
		}
	}

	//
	// TODO: check if not required because DTMF is a tone played already
	//

	// Tone
	//modem.cmd("AT+VLS=1"); // speaker-off
	//modem.cmd(3000,"AT+VTS=[933,,50]");
	//modem.cmd("AT+VLS=5"); // speaker-on

	recordToFile();
}

void Lenny::handleCallWithMessage() {
	LOGC("handleCallWithMessage");

	//
	// outgoing message (ogm)
	//
	{
		char f[PATH_MAX]; sprintf(f,PATH_TAD_OGM_FMT,configLine->line);
		FILE *const fd = fopen(f,"rb");
		if (NULL != fd) {
			LOGC("ogm found:%s",f);
			fclose(fd);
			playFile(f);
		} else {
			LOGV("ogm not found:%s",f);
			playFile(PATH_TAD_DEFAULT);
		}
	}

	modem.cmd("AT+VLS=1"); // speaker-off
	modem.cmd(3000,"AT+VTS=[933,,200]");
	modem.cmd("AT+VLS=5"); // speaker-on

	recordToFile();
}

void Lenny::handleCallWithRecord() {
	LOGC("handleCallWithRecord");
	recordToFile();

/*

	%d%02d%02d%02d%02d%02d_%d.wav
	       YYYYMMDDHHMMSS_N.wav
           98765432109876543210
../var/log/20200312122423_2.wav

*/

	char d[9],t[7-2];
	{
		const unsigned l = (unsigned)strlen(outFile);
		strncpy(d,outFile +l -20,sizeof(d)-1);
		strncpy(t,outFile +l -12,sizeof(t)-1);

		d[sizeof(d)-1] = '\0';
		t[sizeof(t)-1] = '\0';
	}

	std::ostringstream ss;
	ss << "\"date\":\"" <<d <<"\",";
	ss << "\"time\":\"" <<t <<"\",";
	ss << "\"number\":\"local\",";
	ss << "\"name\":\"recording\"";

	addCall(configLine->name,modem.lastCallerIdLoaded ?modem.lastCallerId :ss.str().c_str());
}

void Lenny::handleCallWithDisconnected() {
	LOGC("handleCallWithDisconnected");
	playFile(PATH_DISCONNECTED);

	stats.disconnectedCalls++;
}


struct VolumeCalc {
	unsigned max = 0,min = 255,avg = 0;
	VolumeCalc(const unsigned len,const uint8_t *const buf) { calc(len,buf); }
	void calc(const unsigned len,const uint8_t *const buf) {
		uint64_t t = 0;
		for (unsigned i=0;i<len;i++) {
			const int s = buf[i]; // 0 - 0x7F/0x80 - 0xFF
			const unsigned v = (unsigned)abs(s - 0x7F);
			if (0 < v) min = MIN(min,v);
			max = MAX(max,v);
			t += v;
		}
		avg = (unsigned)(t /(uint64_t)len);
		LOGV("volume: [max:%d avg:%d min:%d t:%d / len:%d ]",max,avg,min,(unsigned)t,len);		
	}
};




//
//
//

void Lenny::handleCallWithLenny() {
	LOGC(" ");

	std::vector<std::string> listFiles = getLennyFiles(configLine->lennyProfile.c_str());

	if (0 == listFiles.size()) {
		if (configLine->disconnectedEnabled) {
			LOGV("getLennyFiles returned nothing, run disconnected");
			handleCallWithDisconnected();
		} else
		if (configLine->tadEnabled) {
			LOGV("getLennyFiles returned nothing, run messages");
			handleCallWithMessage();
		} else {
			LOGV("getLennyFiles returned nothing, hangup");
		}

		hangUp();
		return;		
	}

	// Record audio file
	outPCM.open();

	//
	// Audio receive mode
	//
	Modem_VRX vrx(modem);



	unsigned index = 1;


	// - - - - - - - - - - - - - - - - - - - - - -

	//
	// Hello ...
	//
	if (!playFile(listFiles[0].c_str())) {
		LOGV("sendAudio failed");
		hangUp();
	} else


	//
	// Handle DTMF tone
	//
	if (dtmfInterrupted) {		
		LOGC("DTMF tone interrupted : '%c'",modem.lastDTMFcode);
		queue.post(&answerWithDTMF);
	} else

	// - - - - - - - - - - - - - - - - - - - - - -

	//
	// Rcv start
	//

	if (!vrx.start()) {
		LOGV("voice mode failed");
		hangUp();

		outPCM.close();

	} else {

		//
		// Lenny silence detection logic
		//

		bool nextLenny = false;
		uint64_t nonSilenceTickLast = 0,waitingTickLast = getTickMs();
		unsigned busyCount = 0;
		uint64_t busyFirst = 0,busyLast = 0;

		while (answerModeActive) {

			//
			// Check for input
			//
			uint8_t buf[4000+1]; // 500ms (8000 /2)
			int r = modem.readBuffer(true,1000,sizeof(buf) -1,buf);

			if (0 > r) { LOGC("modem.read returned error"); break; }

		#ifdef DEBUG
			if (0 == r && 0 == modem.lastDCEcode && 0 == modem.lastDTMFcode) {
				LOGV("modem.read returned zero with no DCE code, should never happen!");
				break;
			}

			//	LOGV("modem.read r:%d dce:%d (%c) dtmf:%c",r,modem.lastDCEcode,modem.lastDCEcode,lastDTMFcode);
		#endif //DEBUG


			//
			// Check DTMF
			//
			if (dtmfInterrupted || checkInterruptionByDTMF()) {
				LOGC("DTMF tone interrupted : '%c'",modem.lastDTMFcode);
				queue.post(&answerWithDTMF);
				break;
			}


			//
			//
			//
			if (0 < r) {
				struct VolumeCalc vol((unsigned)r,buf);


				//
				//
				//

				if (config.silenceVolumeThreshold < vol.max) {
					LOGC("talking continue: volume [threshold:%d max:%d avg:%d min:%d]",config.silenceVolumeThreshold,vol.max,vol.avg,vol.min);
					nonSilenceTickLast = getTickMs();

				} else {

					if (0 < nonSilenceTickLast) {
						const unsigned t = (unsigned)(getTickMs() - nonSilenceTickLast);

						LOGC("silence after talk: volume [max:%d avg:%d min:%d] silence:%d",vol.max,vol.avg,vol.min,t);

						if (config.silenceSwitchTime < t) {
							LOGC("nextLenny = true [max:%d avg:%d min:%d] silence:%d",vol.max,vol.avg,vol.min,t);
							nextLenny = true;
						}

					} else {
						const unsigned t = (unsigned)(getTickMs() - waitingTickLast);

						LOGC("silence continuing: volume [max:%d avg:%d min:%d] waiting:%dms",vol.max,vol.avg,vol.min,t);




					/* Testing continuous lenny
						if (config.endCallSilenceTime < t) {
							LOGC("SILENCE next lenny t:%dms",t);
							nextLenny =true;;
						} else
					//*/

					//* Testing busy hangup only
						// Hangup after silence time expired
						if (config.endCallSilenceTime < t) {
							LOGC("SILENCE HANG-UP t:%dms",t);
							answerModeActive = false;
							break;
						}
					//*/

					}
				}


				//
				// Save received audio
				//
				savePCMAudio((unsigned)r,buf);
			}


			//
			//
			//

			if (0 == modem.lastDCEcode) {
				// No codes
			} else
			if (SILENCE == modem.lastDCEcode) {
				// NA
			} else

			//
			// BUSY_TONE = Hang up
			//
			if (BUSY_TONE == modem.lastDCEcode) {
				LOGC("BUSY_TONE r:%d count:%d",r,busyCount);

				//
				// debounce is at least three busy signals
				//
				busyCount++;
				busyLast = getTickMs();

				if (0 == busyFirst) {
					busyFirst = busyLast;
				} else {
					LOGC("BUSY cnt:%d in %dms",busyCount,(unsigned)(busyLast - busyFirst));

					// 3 in more than 10 seconds, resets
					// 3 in 10 seconds ends call
					if (config.endCallBusyCount < busyCount) {
						if (config.endCallBusyTime <= busyLast - busyFirst) {
							LOGC("BUSY reset");
							busyCount = 0;
							busyFirst = 0;
						} else {
							LOGC("BUSY detected ends call, answerModeActive = FALSE");
							answerModeActive = false;
							break;
						}
					}
				}
			}

			//
			//
			//
			if (!nextLenny) continue;
			nextLenny = false;

			//
			// Rcv stop
			//
			vrx.stop();

			//
			// Play audio
			//
			{

				std::string f = listFiles[index];
				if (++index >= listFiles.size()) index = 3;

				if (!playFile(f.c_str())) {
					LOGV("sendAudio failed");
					answerModeActive = false;
					break;
				}
			}

			//
			// Handle DTMF
			//
			if (checkInterruptionByDTMF()) break;

			//
			// Rcv start
			//
			if (!vrx.start()) {
				LOGV("vrx start failed");
				answerModeActive = false;
				break;
			}

			//
			//
			//
			nonSilenceTickLast = 0;
			waitingTickLast = getTickMs();
		}
	}

	modem.dtmfReset();
	vrx.stop();

	//
	//
	//
	outPCM.close();
}

bool Lenny::checkInterruptionByDTMF() {
	//LOGV("checkInterruptionByDTMF '%c'",!modem.lastDTMFcode ?' ' :modem.lastDTMFcode);

	if (0 < modem.lastDTMFcode) {

		if (0 == dtmfCodeTickStart) {
			dtmfCodeTickStart = getTickMs();
			dtmfCode = modem.lastDTMFcode;

			LOGC("start dtmf:%c",modem.lastDTMFcode);

		} else {

			if (dtmfCode != modem.lastDTMFcode) {
				LOGC("change dtmf fr:%c to:%c",dtmfCode,modem.lastDTMFcode);
				dtmfCode = modem.lastDTMFcode;

				//
				// TODO what to do? testing required
				//
			} else {

				LOGC("still dtmf:%c",modem.lastDTMFcode);

			}

			if (1000 < getTickMs() - dtmfCodeTickStart) {
				LOGC("interrupt dtmf:%c after:%dms",dtmfCode,(unsigned)(getTickMs() - dtmfCodeTickStart));
				dtmfInterrupted = true;
				return true;
			}

		}
	} else

	if (0 < dtmfCodeTickStart) {
		LOGC("reset dtmf:%c after:%dms",dtmfCode,(unsigned)(getTickMs() - dtmfCodeTickStart));
		dtmfCodeTickStart = 0;
		dtmfCode = 0;
	}

	return false;
}




//
//
//
bool Lenny::playFile(const char *const file) {
	LOGC("playFile start f:%s",file);

	Modem_VTX vtx(modem);

	if (!vtx.start()) {
		LOGV("voice mode failed");
		return false;
	}

	const int r = sendPCMAudio(file);

	if (-1 == r) {
		LOGV("sendAudio failed");
	}

	vtx.stop();

	LOGC("playFile finish f:%s",file);
	return true;
}


//
//
//
int Lenny::sendPCMAudio(const char *const file) {
	LOGC("sendPCMAudio file:%s",file);

	FILE *const fd = wavOpen(file);

	if (NULL == fd) {
		LOGV("fopen failed: %s",file);
		return -1;
	}

#define CLIP_LEVEL 20

	int total = 0;

	uint8_t buf[8000 /10]; // 100ms chunk size

	bool active = true;
	while (active) {
		const uint64_t t = getTickMs();

		//
		// Send file
		//
		{
			// read chunk
			const size_t r = fread(buf,1,sizeof(buf),fd);
			if (0 >= r) { break; }

			//
			// Clip volume
			//

			for (unsigned i=0;i<(unsigned)r;i++) {
				buf[i] = CLIP_LEVEL > buf[i] ?0 :buf[i];
			}

			savePCMAudio((unsigned)r,buf);

			// write chunk
			const int w = modem.write((unsigned)r,buf);
			if (w != (int)r) {
				LOGV("sendPCMAudio write failed r:%d w::%d",r,w);

				LOGC("BUSY detected, call answerModeActive = FALSE");
				answerModeActive = false;

				total = -1;
				break;
			}

			total += r;
		}

		//
		// Read modem
		//
		while (true) {
			const uint64_t now = getTickMs();
			if (50 < now - t) break;

			const int r = modem.readBuffer(true,(unsigned)(50-(now - t)),sizeof(buf),buf);
	
			if (0 > r) {
				LOGV("modem.read returned error, should never happen");

				LOGC("BUSY detected, call answerModeActive = FALSE");
				answerModeActive = false;

				total = -1;
				break;
			}

			//
			if (checkInterruptionByDTMF()) {
				LOGC("checkInterruptionByDTMF");
				active = false;
				break;
			}

			//
			if (0 == modem.lastDCEcode) {
			} else
			if (BUSY_TONE == modem.lastDCEcode) {
				LOGC("BUSY detected, call answerModeActive = FALSE");
				active = false;
				answerModeActive = false;
				break;
			}

		}
	}

	fclose(fd);

	LOGC("finish file:%s total:%d",file,total);

	return total;
}


//
//
//
void Lenny::savePCMAudio(const unsigned len,const uint8_t *const buf) {
	sharePCMAudioBuffer(len,buf);

	outPCM.write(len,buf);
}



//
//
//
bool Lenny::recordToFile() {
	LOGC("recordToFile f:%s",outFile);

	if (!outPCM.open()) {
		LOGV("outPCM open failed f:%s",outFile);
		return false;
	}

	// Receive mode
	Modem_VRX vrx(modem);
	vrx.start();

	uint8_t buf[2000];

	unsigned silenceCount = 0;
	const uint64_t tstart = getTickMs();

	const unsigned busyInterval = (unsigned)(config.endCallBusyTime / config.endCallBusyCount);
	unsigned busyCount = 0;
	uint64_t busyFirst = 0,busyLast = 0;

	while (answerModeActive) {

		const int r = modem.readBuffer(true,1000,sizeof(buf),buf);
		//LOGV("readBuffer r:%d",r);

		if (0 > r) { LOGV("modem.read returned error, should never happen"); break; }

		if (0 < r) { savePCMAudio((unsigned)r,buf); }

		switch (modem.lastDCEcode) {
			case 0: break;
			default: LOGC("DCE r:%d code:%c (0x%X)",r,modem.lastDCEcode,modem.lastDCEcode); break;

			case BUSY_TONE:	
				busyCount++;
				busyLast = getTickMs();

				LOGC("BUSY detected, count:%d t:%d",busyCount,busyLast - busyFirst);

				if (0 == busyFirst) busyFirst = busyLast;

				if (busyInterval < busyLast - busyFirst) {
					busyCount = 0;
					busyFirst = 0;
				} else {
					LOGC("BUSY detected, call answerModeActive = FALSE");
					answerModeActive = false;
				}

				break;

			case SILENCE:
				silenceCount++;

				LOGC("SILENCE %d t:%d",silenceCount,(int)(getTickMs() - tstart));

				if (60 < silenceCount) {
					LOGC("30s of silences ends call");
					answerModeActive = false;
					break;
				}

				break;
		}
	}

	outPCM.close();

	vrx.stop();

	return true;
}


//
//
//
void Lenny::listenAdd(void *const sock) {
	std::vector<void*>::iterator it;
	for (it = listListeningSockets.begin();it != listListeningSockets.end();it++) {
		if (sock != (*it)) continue;
		break;
	}
	if (listListeningSockets.end() == it) {
		LOGD("listen add sock:%d",sock);
		listListeningSockets.push_back(sock);
	} else {
		LOGD("listen found sock:%d",sock);
	}

}

void Lenny::listenRemove(void *const sock) {
	if (NULL == sock) {
		LOGD("listen remove all");
		listListeningSockets.clear();
		return;
	}
	std::vector<void*>::iterator it;
	for (it = listListeningSockets.begin();it != listListeningSockets.end();it++) {
		if (sock != (*it)) continue;
		break;
	}
	if (listListeningSockets.end() == it) {
		LOGC("listen not found sock:%d",sock);
	} else {
		LOGD("listen removed sock:%d",sock);
		listListeningSockets.erase(it);
	}
}

void Lenny::sharePCMAudioBuffer(const unsigned len,const uint8_t *const buf) {
	//LOGV("len:%d count:%d",len,listListeningSockets.size());
	uint8_t pkt[8192];

	if (sizeof(pkt) < 4+len) {
		LOGV("overflow len:%d",len);
		return;
	}

	bool created = false;

	for (std::vector<void*>::iterator it = listListeningSockets.begin();it != listListeningSockets.end();it++) {
		if (!created) {
			created = true;
			const unsigned line = NULL == configLine ?1 :configLine->line;
			pkt[0] = (uint8_t)(0x40 | (line &0x0F));
			pkt[1] = (uint8_t)(len >> 16);
			pkt[2] = (uint8_t)(len >> 8);
			pkt[3] = (uint8_t)(len >> 0);
			memcpy(pkt+4,buf,len);
		}

		if (!wsSendBinary(*it,4+len,pkt)) {
			LOGV("wsSendBinary failed");
		}
	}
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
// 30min max
#define PCM_30_MINS ( 8000 *60 *30 )
//
PCMFile::PCMFile(Lenny &line) : lenny(line),outFd(NULL),outTotal(0) {}
PCMFile::~PCMFile() { close(); }
bool PCMFile::open() {
	if (outFd) return true;
	outTotal = 0;
	outFd = wavCreate(lenny.outFile);
	if (!outFd) {
		LOGV("PCMFile open failed f:%s",lenny.outFile);
		return false;
	}
	return true;	
}
void PCMFile::close() {
	if (!outFd) return;

	FILE *const fd = outFd;
	outFd = NULL;

	LOGC("wavWriteHeader t:%d",outTotal);
	wavWriteHeader(fd,outTotal);
	fclose(fd);
}
void PCMFile::write(unsigned len,const uint8_t *buf) {
	if (NULL != outFd) {
		const size_t wr = fwrite(buf,len,1,outFd);
		if (1 != wr) {
			LOGE("fwrite failed len:%d",len);
			close();
			return;
		}
		outTotal += len;
	}

	if (outTotal >= PCM_30_MINS) {
		LOGC("PCMFile overflow close f:%s",lenny.outFile);
		close();
		lenny.createDefaultFileName();
		LOGC("PCMFile overflow open f:%s",lenny.outFile);
		open();
	}
}



//
//
//
ThreadWaitForCall::ThreadWaitForCall(Lenny &l) :lenny(l) {}
void ThreadWaitForCall::run() {
	//LOGV("ThreadWaitForCall +");

	if (lenny.modem.offHook) {
		LOGC("OFFHOOK");
		return;
	}

	lenny.handleWaitForCall();
	//LOGV("ThreadWaitForCall -");
}

ThreadWaitForCallCheck::ThreadWaitForCallCheck(Lenny &l) :lenny(l) {}
void ThreadWaitForCallCheck::run() {
	LOGV("ThreadWaitForCallCheck");

	if (lenny.modem.offHook) {
		LOGC("not ONHOOK");
		return;
	}

	LOGV("schedule waitForCall 1 seconds");
	lenny.queue.post(&lenny.waitForCall,1000);
}

ThreadOffHook::ThreadOffHook(Lenny &l) :lenny(l) {}
void ThreadOffHook::run() {
	LOGC("ThreadOffHook");

	LOGC("cancel scheduled waitForCall");
	lenny.queue.cancel(&lenny.waitForCall);

	if (!lenny.modem.offHook) {
		lenny.answer();
	}

	sendStatus();
}

ThreadOnHook::ThreadOnHook(Lenny &l) :lenny(l) {}
void ThreadOnHook::run() {
	LOGC("ThreadOnHook");

	LOGC("cancel scheduled waitForCall");
	lenny.queue.cancel(&lenny.waitForCall);

	if (lenny.modem.offHook) {
		lenny.hangUp();
	}

	LOGC("schedule waitForCall");
	lenny.queue.post(&lenny.waitForCall,500);
}

ThreadAnswerWithLenny::ThreadAnswerWithLenny(Lenny &l) :lenny(l) {}
void ThreadAnswerWithLenny::run() {
	LOGC("ThreadAnswerWithLenny");

	LOGC("cancel scheduled waitForCall");
	lenny.queue.cancel(&lenny.waitForCall);

	if (!lenny.modem.offHook) {
		lenny.answer();
	}

	lenny.mode = Lenny::LENNY;
	sendStatus();

	{
		const uint64_t tstart = getTickMs();

		lenny.handleCallWithLenny();

		const uint64_t tfinish = getTickMs();
		const unsigned tdur = (unsigned)(tfinish - tstart);

		LOGV("duration:%d",tdur);
		stats.lennyCalls++;
		stats.lennyTime += tdur;
		stats.save();
	}

	if (lenny.dtmfInterrupted && 0 < lenny.queue.size()) {
		LOGC("dtmfInterrupted");
		return;
	}

	if (lenny.modem.offHook) {
		lenny.hangUp();
	
		LOGC("schedule waitForCall");
		lenny.queue.post(&lenny.waitForCall,500);
	}

	lenny.mode = Lenny::READY;
	sendStatus();
}

ThreadAnswerWithDisconnected::ThreadAnswerWithDisconnected(Lenny &l) :lenny(l) {}
void ThreadAnswerWithDisconnected::run() {
	LOGC("ThreadAnswerWithDisconnected");

	LOGC("cancel scheduled waitForCall");
	lenny.queue.cancel(&lenny.waitForCall);

	if (!lenny.modem.offHook) {
		lenny.answer();
	}

	lenny.mode = Lenny::DISCONNECTED;
	sendStatus();

	lenny.handleCallWithDisconnected();

	stats.disconnectedCalls++;
	stats.save();

	if (lenny.modem.offHook) {
		lenny.hangUp();

		LOGC("schedule waitForCall");
		lenny.queue.post(&lenny.waitForCall,500);
	}

	lenny.mode = Lenny::READY;
	sendStatus();
}

ThreadAnswerWithDTMF::ThreadAnswerWithDTMF(Lenny &l) :lenny(l) {}
void ThreadAnswerWithDTMF::run() {
	LOGC("ThreadAnswerWithDTMF");

	LOGC("cancel scheduled waitForCall");
	lenny.queue.cancel(&lenny.waitForCall);

	if (!lenny.modem.offHook) {
		lenny.answer();
	}

	//
	// New outFile after DTMF interruption
	//
	{
		const time_t now = time(0);
		tm *const ltm = localtime(&now);
		snprintf(lenny.outFile,sizeof(lenny.outFile),"%s/%d%02d%02d%02d%02d%02d_%d.wav",PATH_CALLS,1900+ltm->tm_year,1+ltm->tm_mon,ltm->tm_mday,ltm->tm_hour,ltm->tm_min,ltm->tm_sec,lenny.configLine->line);
		LOGC("DTMF: outFile:%s",lenny.outFile);
	}

	lenny.mode = Lenny::MESSAGES;
	sendStatus();

	//
	lenny.handleCallWithDTMF();

	if (lenny.modem.offHook) {
		lenny.hangUp();

		LOGC("schedule waitForCall");
		lenny.queue.post(&lenny.waitForCall,500);
	}

	lenny.mode = Lenny::READY;
	sendStatus();
}

ThreadAnswerWithMessage::ThreadAnswerWithMessage(Lenny &l) :lenny(l) {}
void ThreadAnswerWithMessage::run() {
	LOGC("ThreadAnswerWithMessage");

	LOGC("cancel scheduled waitForCall");
	lenny.queue.cancel(&lenny.waitForCall);

	if (!lenny.modem.offHook) {
		lenny.answer();
	}

	lenny.mode = Lenny::MESSAGES;
	sendStatus();

	lenny.handleCallWithMessage();

	stats.tadCalls++;
	stats.save();

	if (lenny.modem.offHook) {
		lenny.hangUp();

		LOGC("schedule waitForCall");
		lenny.queue.post(&lenny.waitForCall,500);
	}

	lenny.mode = Lenny::READY;
	sendStatus();
}

ThreadAnswerWithRecord::ThreadAnswerWithRecord(Lenny &l) :lenny(l) {}
void ThreadAnswerWithRecord::run() {
	LOGC("ThreadAnswerWithRecord");

	LOGC("cancel scheduled waitForCall");
	lenny.queue.cancel(&lenny.waitForCall);

	if (!lenny.modem.offHook) {
		lenny.answer();
	}

	lenny.mode = Lenny::RECORD;
	sendStatus();

	lenny.handleCallWithRecord();

	if (lenny.modem.offHook) {
		lenny.hangUp();

		LOGC("schedule waitForCall");
		lenny.queue.post(&lenny.waitForCall,500);
	}

	lenny.mode = Lenny::READY;
	sendStatus();
}

