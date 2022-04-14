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

#include "biz.h"
#include "butils.h"
#include "data.h"
#include "stats.h"
#include "license.h"
#include "lenny.h"

//
#define LICENSE_APP_ID "com.slade.lenny"
//std::string LICENSE_MAC_ADDRESS = getMacAddress(); // eth0,wlan0


//
SystemInfo sysInfo;
char ip[32];

Config config;
Stats stats;

WSCallback cbs;
unsigned httpPort = 0,httpsPort = 0;

//
//
//
bool appStart() {
	LOGT(" ");

#ifdef LINUX
	if (!appFileLock()) {
		fprintf(stderr,"Is another instance of Lenny already running?\n");
		return false;
	}
#endif //LINUX

	appConfigLoad();

	//
	// Start webserver
	//
	{
		// HTTP
		getIpAddress(ip);
		unsigned port = config.httpPort;
		if (!startWebServer(&cbs,port,false)) {
			// what to do
			LOGE("startWebServer failed port:%d",port);
			return false;
		} else {
			httpPort = port;

			port++;
		}

		// HTTPS
		if (!startWebServer(&cbs,port,true)) {
			// what to do
			LOGE("startWebServer secure failed port:%d",port);
		} else {
			httpsPort = port;
		}
	}

	return true;
}

void appStop() {
	LOGT(" ");

	stopLenny();

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//
	// write status file
	//

	appFileUnLock();
}

void appConfigLoad() {
	sysInfo.get();
	config.load(PATH_CONFIG);
	stats.load(PATH_STATS_FILE);
}

void appUpdate() {
	LOGC(" ");

	appCheckLicense();
}

void appWebAppStart() {
	//LOGC(" ");

	appConfigLoad();

//#ifdef LINUX
//#endif //LINUX
#define LAUNCH_CMD "open "  // "xdg-open "

	std::ostringstream ss; ss <<LAUNCH_CMD << "http://" <<ip <<":" <<config.httpPort;

	LOGC("%s",ss.str().c_str());

	system(ss.str().c_str());
}


#include "webcert.h"

//
//
//
std::string appInfo() {

#if defined(__i386__)
	const char *const ARCH = "i386";
#elif defined(__x86_64__)
	const char *const ARCH = "x86_64";
#elif defined(__arm__)
	#if defined(__ARM_ARCH_5T__)
		const char *const ARCH = "arm";
	#elif defined(__ARM_ARCH_7A__)
		const char *const ARCH = "arm7a";
	#else
		const char *const ARCH = "arm";
	#endif
#elif defined(__powerpc64__)
	const char *const ARCH = "PowerPC64";
#elif defined(__aarch64__)
	const char *const ARCH = "aarch64";
#else
	const char *const ARCH = "unkArch";
#endif


	std::ostringstream ss;

	ss << "Lenny v" << getAppVersion() << " (cpu:" << ARCH <<" build:" << (8*sizeof(char*)) <<"bit)" << std::endl;

	if (config.sv && config.sv->hasUpdate()) {
		ss << "  available update to v" << config.sv->toString() << std::endl;
	}

	ss << " root: " << getRootDir() << std::endl;

    if (false) {
		std::map<std::string,std::string> m = getNetworkIpV4Addresses();
		for (std::map<std::string,std::string>::iterator it = m.begin(); it != m.end(); it++) {
			ss << " ip: " << it->first << ":" << it->second << std::endl;
		}
	}

	if (false) {
		extern char PATH_LOG_FILE[];
		char l[PATH_MAX];
		char *const p = realpath(PATH_LOG_FILE,l);
		ss << " log: " << p << std::endl;
	}

	ss << std::endl;


	//
	//
	//
#ifdef NO_LICENSE

	ss << "Evaluation Beta License" << std::endl;
	ss << std::endl;

#else //NO_LICENSE

	if (0 == config.license.length()) {
		ss << "UNLICENSED";
	} else {
		time_t expiration;
		LicenseFlags flags;

		const int valid = appGetLicense(config.license.c_str(),expiration,flags);

		if (LICENSE_INVALID == valid) {
			ss << "INVALID License " << config.license;
		} else {
			char expiry[20] = {0}; // "2020-01-31"
			formatDays((uint16_t)(expiration / SECONDS_IN_DAY),expiry,sizeof(expiry));

			if (LICENSE_EXPIRED == valid) {
				ss << (flags.eval?"EXPIRED Evaluation License " :"EXPIRED License ") << config.license <<" v:" <<flags.ver <<" EXPIRED:" << expiry << std::endl;
			} else {
				ss <<( flags.eval?"ACTIVE Evaluation License " :"ACTIVE License ") << config.license <<" v:" <<flags.ver <<" expires:" <<expiry << std::endl;
			}
			ss <<" Registered Email: " << config.email << std::endl;
		}
	}

	ss << std::endl;

#endif //NO_LICENSE

	//
	ss << "WebApp Access:" <<std::endl;
    {
		std::ostringstream http,https;
		{
			std::map<std::string,std::string> m = getNetworkIpV4Addresses();
			unsigned c = 0;
			for (std::map<std::string,std::string>::iterator it = m.begin(); it != m.end(); it++) {
				if (0 < httpPort ) { if (0 < c) { http  << ", "; } http  << "http://"  << it->second <<":" << httpPort; }
				if (0 < httpsPort) { if (0 < c) { https << ", "; } https << "https://" << it->second <<":" << httpsPort; }
				c++;
			}
		}

		ss << " HTTP : " << http.str() << std::endl;
		ss << " HTTPS: " << https.str() << std::endl;
		ss <<webCertInfo() << std::endl;
	}

	//
#ifdef LINUX
	ss << "Host Info:" << std::endl;
	ss << " os: " << sysInfo.osIssue <<" / " << sysInfo.osUname << std::endl;
	ss << " hw: " << sysInfo.hwName <<" id:" << sysInfo.hwMachineId <<" serial:"<< sysInfo.hwSerial << std::endl;
	ss << " cpu: " << sysInfo.cpuBits <<"bit\n";
//	ss << " mac: " << getMacAddress() <<std::endl;
#else //LINUX
	ss << "Host Info:";
	ss << " os: " <<sysInfo.osIssue << " / " << sysInfo.osUname << std::endl;
//	ss << " mac: " <<getMacAddress() <<std::endl;
#endif //LINUX

    {
		std::ostringstream mac;
		{
			std::map<std::string,std::string> m = getMacAddresses();
			for (std::map<std::string,std::string>::iterator it = m.begin(); it != m.end(); it++) {
				mac <<" mac: " << it->second << std::endl;
			}
		}
		ss << mac.str();
	}
	ss << std::endl;

#ifdef LINUX
	ss << "Modem Info:" <<std::endl;;
	{
		std::vector<std::string> m = enumUSBModemDevices();

//
// TODO what if none found
//

		unsigned c = 0;
		for (std::vector<std::string>::iterator it = m.begin();it != m.end();it++) {
			if (0 == c++) {
				ss << " line1: " <<(*it) <<" " <<config.line1.device <<std::endl;
			} else {
				ss << " line2: " <<(*it) <<" " <<config.line2.device <<std::endl;
			}
		}
	}
	ss << std::endl;
#endif //LINUX

	return ss.str();
}



//
// Check license
//

void appCheckLicense() {
	LOGC(" ");


	//
	//
	//
	LOGC("sendLennyComments");
	sendLennyComments();

	LOGC("sendLennyBugs");
	sendLennyBugs();



#ifndef USE_LICENSE

	startLenny();

#else //USE_LICENSE
	{
		time_t expiration;
		LicenseFlags flags;

		const int valid = appGetLicense(config.license.c_str(),expiration,flags);

		if (LICENSE_INVALID == valid) {
			LOGE("LICENSE INVALID stopping Lenny");
			stopLenny();
		} else {

			char expiry[20] = {0}; // "2020-01-31"
			formatDays((uint16_t)(expiration / SECONDS_IN_DAY),expiry,sizeof(expiry));

			if (LICENSE_EXPIRED == valid) {
				LOGE("LICENSE EXPIRED:%s ver:%d eval:%s",expiry,flags.ver,flags.eval?"true":"false");
				stopLenny();

			} else {
				LOGC("LICENSE EXPIRES:%s ver:%d eval:%s",expiry,flags.ver,flags.eval?"true":"false");
				startLenny();
			}
		}
	}
#endif //USE_LICENSE

}

//
//
//

bool appGetServerVersion() {
	LOGC(" ");

/*
	//
	// DOS protection
	//
	static DenialOfService dos;
	if (!dos.check()) {
		LOGV("DenialOfService check failed");
		return false;
	}
*/

	//
	//
	//
	if (!getLennyVersionInfo()) {
		LOGV("getLennyVersionInfo failed");
		return false;
	}

	LOGD("latest v%s",config.sv->toString().c_str());

	//
	//
	//
	LOGC("sendLennyComments");
	sendLennyComments();

	LOGC("sendLennyBugs");
	sendLennyBugs();

	return true;
}




//
//
//  appId + '_' + mac_addr + '_' + email
//  'com.slade.lenny_b8:27:eb:02:b4:f8_test@lennytroll.com'
//
//

int appGetLicense(const char *const license,time_t &dateMs,LicenseFlags &flags) {

	std::map<std::string,std::string> m = getMacAddresses();

	for (std::map<std::string,std::string>::iterator it = m.begin();it != m.end(); it++) {
		std::string mac = it->second;

		std::string appId;
		appId.append(LICENSE_APP_ID).append("_").append(mac).append("_").append(config.email);

		const int r = getAppLicenseInfo(license,appId.c_str(),dateMs,flags);

		if (LICENSE_INVALID != r) return r;

		LOGE("license invalid : MAC:%s",mac.c_str());
	}

	return LICENSE_INVALID;
}

bool appCreateLicense(const unsigned days,const bool evalFlag) {
	LOGC("days:%d eval:%d",days,evalFlag);

	//
	// DOS protection
	//
	static DenialOfService dos;
	if (!dos.check()) {
		LOGV("DenialOfService check failed");
		return false;
	}

	//
	//
	//
	LicenseFlags f = {0};
	f.ver  = LICENSE_APP_VERSION;
	f.eval = evalFlag;

	std::string appId = LICENSE_APP_ID;
	std::string mac = getFirstMacAddress();
	appId.append("_");
	appId.append(mac);
	appId.append("_");
	appId.append(config.email);

	std::string l = createAppLicense(days,f,appId.c_str());

	return sendLennyLicenseToEmail(config.email.c_str(),mac.c_str(),l.c_str(),days);
}

