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
#include <map>
#include <vector>
#include <string>

//
//
//
extern std::string LICENSE_MAC_ADDRESS;

extern std::string getAppVersion();

//
//
//
class SystemInfo {
public:
	std::string osType,osLogo,osIssue,osUname;
	std::string hwMachineId,hwSerial,hwName;
	unsigned cpuBits;

	uint64_t appStartTick;

	void parse(const char *str);
	void get();
	inline unsigned getAppTime() const { return (unsigned)(getTickMs() - appStartTick); }
};

//
//
//
class DenialOfService {
private:
	unsigned minSecs,maxTriesInADay;
	uint64_t ticks,secs;
	unsigned trys;
public:
	DenialOfService(const unsigned minSecsBetweenRetries = 10,const unsigned maxTriesInADay = 10);
	bool check();
};



//
extern bool appFileLock();
extern void appFileUnLock();

//
extern bool sendLennyLicenseToEmail(const char *email,const char *mac,const char *license,unsigned days);
extern bool getLennyVersionInfo();
extern bool sendLennyBugs();
extern bool sendLennyComments();
extern bool sendLennyLog();
extern bool sendLennyStats();


//
extern char *formatDays(unsigned epochInDays,char *resultBuffer,size_t bufferSize);

typedef union {
	uint16_t flags;
	struct { uint16_t ver:8,eval:1,flag9:1,flagA:1,flagB:1,flagC:1,flagD:1,flagE:1,flagF:1; };
} LicenseFlags;
extern std::string createAppLicense(unsigned days,LicenseFlags flags,const char *appId);
extern int getAppLicenseInfo(const char *const license,const char *appId,time_t &dateMs,LicenseFlags &flags);


//
extern std::string getRootDir();
extern std::string getHostName();
extern std::string getPublicIpAddress();
extern std::string getIpAddress();
extern std::string getExternalIpAddress();
extern std::string getSSID();
extern std::map<std::string,std::string> getNetworkIpV4Addresses();
extern std::map<std::string,std::string> getMacAddresses();
extern std::string getMacAddress(const char *iface);
extern std::string getFirstMacAddress();
extern std::string getHostname();

std::string getOsUptime();

extern std::string getCPUStat();

extern std::string getLinuxInfo();
extern std::string getSystemLogo();
extern std::string getSystemPrettyName();
//extern std::string getSystemIssue();

extern std::vector<std::string> enumUSBModemDevices(bool json = false);

extern void dnsReverseLookup(const char *ip);

