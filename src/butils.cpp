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

#include "butils.h"
#include <sstream>
#include <errno.h>
#include <unistd.h> // unlink,ftruncate,write,close,getpid


//
// sudo lsusb -D /dev/bus/usb/001/004
//

extern std::string getSystemInfo();
extern bool signinValidate(const char *name,const char *pwd,std::ostringstream &ss);



//
//
//

void SystemInfo::get() {
	appStartTick = getTickMs();

#if defined(LINUX)
	osType = "linux";
	osLogo = getSystemLogo();
	osIssue = getSystemPrettyName();
//	osIssue = getSystemIssue();
	osUname = trimRight(exec("uname -srm"));

	hwMachineId = trimRight(exec("cat /etc/machine-id"));
	hwSerial = trimRight(exec("cat /sys/firmware/devicetree/base/serial-number"));
	hwName = trimRight(exec("cat /sys/firmware/devicetree/base/model"));

	cpuBits = atoi(exec("getconf LONG_BIT").c_str());
#elif defined(OSX)
	osType = "mac_osx";
	osLogo = "apple";
	osIssue = "MacOSX";
	osUname = trimRight(exec("uname -srm"));

	hwMachineId = "";
	hwSerial = "";
	hwName = "";

	cpuBits = (unsigned)atoi(exec("getconf LONG_BIT").c_str());
#else
	osType = "unknown";
	osLogo = "linux";
	osIssue = "";
	osUname = "";

	hwMachineId = "";
	hwSerial = "";
	hwName = "";

	cpuBits = 32;
#endif

};

std::string getRootDir() {
	char cwd[PATH_MAX]; getcwd(cwd,sizeof(cwd));
/*
	char *const s = strrchr(cwd,'/');
	if (NULL == s) {
		strcpy(cwd,"/");
		LOGV("strrchr failed %s",cwd);
	} else {
		*s = '\0';
	}
*/
	return cwd;
}

std::string getHostName() { return ::getHostname(); }
std::string getPublicIpAddress() { return getExternalIpAddress(); }
std::string getIpAddress() { char ip[32] = {0}; ::getIpAddress(ip); return ip; }
std::string getSSID() { return ""; }

std::string getOsUptime() { return trimRight(exec("uptime -p")); }


std::string getAppVersion() {
	std::ostringstream v; v <<APP_VERSION_MAJOR <<"." <<APP_VERSION_MINOR <<":" <<LICENSE_APP_VERSION;
	return v.str();
}

//
#include "config.h"
//
//
ServerVersion::ServerVersion() : mjr(0),mnr(0),lic(0) {}
ServerVersion::ServerVersion(const char *str) { parse(str); }
std::string ServerVersion::toString() const { std::ostringstream ss; ss <<mjr <<"." <<mnr <<":" <<lic; return ss.str(); }
void ServerVersion::parse(const char *str) {
	char mj[5],mn[5],lv[5];

	{
		char *tvv,*vv;
		vv = strtok_r((char*)str,":",&tvv);
		if (NULL == vv) { LOGV("strtok_r failed : %s",str); return; }

		char av[10];
		strncpy(av,vv,sizeof(av)-1);

		vv = strtok_r(NULL,":",&tvv);
		if (NULL == vv) { LOGV("strtok_r failed : %s",str); return; }
		strncpy(lv,vv,sizeof(lv)-1);

		vv = strtok_r(av,".",&tvv);
		if (NULL == vv) { LOGV("strtok_r failed : %s",str); return; }
		strncpy(mj,vv,sizeof(mj)-1);

		vv = strtok_r(NULL,":",&tvv);
		if (NULL == vv) { LOGV("strtok_r failed : %s",str); return; }
		strncpy(mn,vv,sizeof(mn)-1);
	}

	mjr = (unsigned)atoi(mj);
	mnr = (unsigned)atoi(mn);
	lic = (unsigned)atoi(lv);
}

bool ServerVersion::hasUpdate() const { return LICENSE_APP_VERSION < lic || APP_VERSION_MAJOR < mjr || APP_VERSION_MINOR < mnr; }
bool ServerVersion::requiresUpdate() const {
	if (LICENSE_APP_VERSION < lic) {
		LOGW("Retired: requires upgrade from v%s < latest v%s",getAppVersion().c_str(),toString().c_str());
		return true;
	}

	if (APP_VERSION_MAJOR < mjr) {
		LOGW("Deprecated: requires upgrade from v%s < latest v%s",getAppVersion().c_str(),toString().c_str());
		return true;
	}

	if (APP_VERSION_MINOR < mnr) {
		LOGW("Updated: should upgrade from v%s < latest v%s",getAppVersion().c_str(),toString().c_str());
	}

	return false;
}

DenialOfService::DenialOfService(const unsigned nMinSecs,const unsigned nMaxTries) : minSecs(nMinSecs),maxTriesInADay(nMaxTries),ticks(0),secs(0),trys(0) {}
bool DenialOfService::check() {
	const uint64_t now = getTickMs();
	{
		const uint64_t last = ticks;
		ticks = now;

		if (0 < last && minSecs *1000 > getTickMs() - last) {
			LOGW("less than %d seconds",minSecs);
			return false;
		}
	}
	{
		if (0 == secs) {
			secs = ((now / SECONDS_IN_DAY) % SECONDS_IN_DAY);
		} else
		if (secs < ((secs + SECONDS_IN_DAY) % SECONDS_IN_DAY)) {
			secs = ((now / SECONDS_IN_DAY) % SECONDS_IN_DAY);
			trys = 0;
		} else
		if (maxTriesInADay < ++trys) {
			LOGW("more than %d in a day count:%d",maxTriesInADay,trys);
			return false;
		}
	}
	return true;
}



//
//
//
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <algorithm>


std::map<std::string,std::string> getNetworkIpV4Addresses() {

	std::map<std::string,std::string> map;

	{
		struct ifaddrs *ifAddrStruct = NULL;
		getifaddrs(&ifAddrStruct);

		struct ifaddrs *ifa;
		for (ifa = ifAddrStruct;NULL != ifa;ifa = ifa->ifa_next) {
			if ('l' == ifa->ifa_name[0] && 'o' == ifa->ifa_name[1]) continue;
			if (AF_INET == ifa->ifa_addr->sa_family) {
				char ip[INET_ADDRSTRLEN];
				void *const p = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
				inet_ntop(AF_INET,p,ip,INET_ADDRSTRLEN);
				map.insert(std::pair<std::string,std::string>(ifa->ifa_name,ip));

				continue;
			}
		}

		if (NULL != ifAddrStruct) {
			freeifaddrs(ifAddrStruct);
		}
	}

	return map;
}


/*

//	std::vector<std::string> getNetworkIpV4Addresses() const;
//	std::vector<std::string> getNetworkInterfaceNames() const;

std::vector<std::string> getNetworkIpV4Addresses() {

	std::vector<std::string> vec;

	{
		struct ifaddrs *ifAddrStruct = NULL;
		getifaddrs(&ifAddrStruct);

		struct ifaddrs *ifa;
		for (ifa = ifAddrStruct;NULL != ifa;ifa = ifa->ifa_next) {
			if ('l' == ifa->ifa_name[0] && 'o' == ifa->ifa_name[1]) continue;
			if (AF_INET == ifa->ifa_addr->sa_family) {
				char ip[INET_ADDRSTRLEN];
				void *const p = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
				inet_ntop(AF_INET,p,ip,INET_ADDRSTRLEN);
				vec.push_back(ip);
				continue;
			}

		//	if (AF_INET6 == ifa->ifa_addr->sa_family) {
		//		char ip[INET_ADDRSTRLEN];
		//		void *const p = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
		//		inet_ntop(AF_INET6,p,ip,INET6_ADDRSTRLEN);
		//		vec.push_back(ip);
		//		continue;
		//	}

		}

		if (NULL != ifAddrStruct) {
			freeifaddrs(ifAddrStruct);
		}
	}

	sort( vec.begin(), vec.end() );
	vec.erase( unique( vec.begin(), vec.end() ), vec.end() );

	return vec;
}
*/

/*
std::vector<std::string> getNetworkInterfaceNames() {

	std::vector<std::string> vec;

	{
		struct ifaddrs *ifAddrStruct = NULL;
		getifaddrs(&ifAddrStruct);

		struct ifaddrs *ifa;
		for (ifa = ifAddrStruct; NULL != ifa; ifa = ifa->ifa_next) {
			if ('l' == ifa->ifa_name[0] && 'o' == ifa->ifa_name[1]) continue;
			if (AF_INET == ifa->ifa_addr->sa_family) {
				vec.push_back(ifa->ifa_name);
				continue;
			}
			if (AF_INET6 == ifa->ifa_addr->sa_family) {
				vec.push_back(ifa->ifa_name);
				continue;
			}
		}

		if (NULL != ifAddrStruct) {
			freeifaddrs(ifAddrStruct);
		}
	}

	sort( vec.begin(), vec.end() );
	vec.erase( unique( vec.begin(), vec.end() ), vec.end() );

	return vec;
}
*/



//
//
//

extern SystemInfo sysInfo;



//
//
//

//#include <pwd.h> // struct passwd
//#include <sys/types.h>
#include <sys/statvfs.h> //statvfs
//#include <sys/sysctl.h>

#ifdef OSX
#include <sys/vmmeter.h>
#endif //OSX


std::string getSystemInfo() {

#ifdef NEW_STUFF

	// struct passwd *const p = getpwnam(const char *name);
	static const int uid = getuid();
	static const int euid = geteuid()
	const bool rootAccess = 0 == uid && 0 == euid;

	struct passwd *const p = pgetpwuid(euid);
   	if (NULL == p) {
   		LOGV("");
   	} else {
		static std::string user_name = p->pw_name;
   	}

/*
	<pwd.h>
	struct passwd {
		char   *pw_name;       // username
		char   *pw_passwd;     // user password
		uid_t   pw_uid;        // user ID
		gid_t   pw_gid;        // group ID
		char   *pw_gecos;      // user information
		char   *pw_dir;        // home directory
		char   *pw_shell;      // shell program
	};
*/

#endif //NEW_STUFF





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


	//
	//
	//
	std::ostringstream ss;

	ss << "\"app\":{";
		ss << "\"version\":\"" <<APP_VERSION_MAJOR <<"." <<APP_VERSION_MINOR <<"\"";
		ss << ",";
		ss << "\"arch\":\"" <<ARCH <<"\"";
		ss << ",";
		ss << "\"build\":\"" <<(8*sizeof(char*)) <<"bit\"";
		ss << ",";
		ss << "\"uptime\":" <<(sysInfo.getAppTime()/1000);
		ss << ",";
		ss << "\"pwd\":\"" <<getRootDir() <<"\"";
	ss << "},";

	ss << "\"hw\":{";
		ss << "\"machineId\":\"" <<sysInfo.hwMachineId <<"\"";
		ss << ",";
		ss << "\"serial\":\"" <<sysInfo.hwSerial <<"\"";
		ss << ",";
		ss << "\"name\":\"" <<sysInfo.hwName <<"\"";
	ss << "},";

	ss << "\"os\":{";
		ss << "\"type\":\"" <<sysInfo.osType <<"\"";
		ss << ",";
		ss << "\"logo\":\"" <<sysInfo.osLogo <<"\"";
		ss << ",";
		ss << "\"issue\":\"" <<sysInfo.osIssue <<"\"";
		ss << ",";
		ss << "\"uname\":\"" <<sysInfo.osUname <<"\"";
		ss << ",";
		ss << "\"uptime\":\"" <<getOsUptime() <<"\"";
	ss << "},";

	//
	// Static
	//
	const long cpus = sysconf(_SC_NPROCESSORS_ONLN);
	ss << "\"cpu\":{\"cores\":" << cpus <<",\"bits\":" <<sysInfo.cpuBits <<"}";

#ifdef LINUX
#ifdef DEBUG
	const long cpus_conf = sysconf(_SC_NPROCESSORS_CONF);
	if (cpus != cpus_conf) LOGV("cpus:%d conf:%d",cpus,cpus_conf);
#endif //DEBUG
#endif //LINUX

	ss << ",";

	//
	// changes
	//
	ss << "\"space\":{";
	{
		struct statvfs s;
		if (0 > (statvfs("/",&s))) {
			LOGV("statvfs failed errno:%d",errno);
		} else {
			ss << "\"disk\":{";
			ss << "\"block_size\":" << s.f_bsize <<",";
			ss << "\"total\":"<< s.f_blocks <<",";
			ss << "\"free\":"<< s.f_bfree;
			ss << "}";
		}
	}

#ifdef OSX
/*
struct vmtotal {
	short	t_rq;		// length of the run queue
	short	t_dw;		// jobs in ``disk wait'' (neg priority)
	short	t_pw;		// jobs in page wait
	short	t_sl;		// jobs sleeping in core
	short	t_sw;		// swapped out runnable/short block jobs
	int 	t_vm;		// total virtual memory
	int 	t_avm;		// active virtual memory
	short	t_rm;		// total real memory in use
	short	t_arm;		// active real memory
	int 	t_vmtxt;	// virtual memory used by text
	int 	t_avmtxt;	// active virtual memory used by text
	short	t_rmtxt;	// real memory used by text
	short	t_armtxt;	// active real memory used by text
	short	t_free;		// free memory pages
};
*/
	{
	    struct vmtotal vmt;
	    size_t vmt_size = sizeof(vmt);
		int rc = sysctlbyname("vm.vmtotal",&vmt,&vmt_size,NULL,0);

		if (0 > rc) {
			perror("sysctlbyname");
		} else {

		    u_int page_size;
			size_t uint_size = sizeof(page_size);
		    rc = sysctlbyname("vm.stats.vm.v_page_size",&page_size,&uint_size,NULL,0);

			if (0 > rc) {
				perror("sysctlbyname");
			} else {

				ss << ",";

				ss << "\"memory\":{";
				ss << "\"page_size\":"<< page_size <<",";
				ss << "\"total\":"<< ((uint64_t)vmt.t_avm * (uint64_t)page_size) <<",";
				ss << "\"free\":"<< ((uint64_t)vmt.t_free * (uint64_t)page_size);
				ss << "}";
				ss << ",";

//				LOGV("memory free:%llu available:%llu",vmt.t_free * (uint64_t)page_size,vmt.t_avm * (uint64_t)page_size);
			}
		}
	}
#endif //OSX

#ifdef LINUX
	{
	    const long pages = sysconf(_SC_PHYS_PAGES);
	    const long free = sysconf(_SC_AVPHYS_PAGES);
	    const long size = sysconf(_SC_PAGE_SIZE);

		ss << ",";

		ss << "\"memory\":{";
		ss << "\"page_size\":"<< size <<",";
		ss << "\"free\":"<< free <<",";
		ss << "\"total\":"<< pages;
		ss << "}";
	}
	ss << "}";
#endif //LINUX

	ss << ",";

	ss << "\"network\":{";
		ss << "\"hostname\":\"" <<getHostName() <<"\"";
		ss << ",";
		ss << "\"local\":[";

		{
			std::map<std::string,std::string> macs = getMacAddresses();
			std::map<std::string,std::string> adrs = getNetworkIpV4Addresses();

			unsigned c = 0;	
			for (std::map<std::string,std::string>::iterator it = macs.begin(); it != macs.end(); it++) {
				std::map<std::string,std::string>::iterator addr = adrs.find(it->first);
				if (adrs.end() == addr) { LOGV("ip not found if:%s",it->first.c_str()); continue; }

				if (0 < c++) ss <<",";
				ss <<"{ \"type\":\"" <<it->first <<"\",\"ip\":\"" <<addr->second <<"\",\"mac\":\"" <<it->second << "\" } ";
			}
		}

		ss <<"]";

		ss << ",";
		ss << "\"ip_wifi\":{ \"ssid\":\"" <<getSSID() <<"\" }";

//		ss << ",";
//		ss << "\"public\":{ \"ip\":\"" <<eip <<"\" }";

	ss << "}";

	return ss.str();
}





// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include "threader.h"
class ThreadService : public ThreadRunnable {
public:
	void run();
};
static ThreadService threadService;
void threadServiceSchudule() {
	extern ThreadQueue queue;
	queue.cancel(&threadService);
	queue.post(&threadService,60*60*1000);
}
void ThreadService::run() {
	sendLennyComments();
	sendLennyBugs();
}


//
// Local external / public address
//
#include "webclient.h"

const char *const WS_HOST               = "lennytroll.com";
const char *const WS_GET_PUBLIC_IP      =  "GET /js/?ip";
const char *const WS_GET_VERSION_INFO   =  "GET /js/?version";
const char *const WS_POST_LICENSE_EMAIL = "POST /js/?license";
const char *const WS_POST_BUG_REPORT    = "POST /js/?bug";
const char *const WS_POST_COMMENT       = "POST /js/?comment";
const char *const WS_POST_LOG_FILE      = "POST /js/?log";
const char *const WS_POST_STATS_FILE    = "POST /js/?stat";

#ifdef NO_SSL
#define WS_PORT 80
#else //NO_SSL
#define WS_PORT 443
#endif //NO_SSL


std::string getExternalIpAddress() {
	WebClient c;
	c.send(WS_HOST,WS_PORT,WS_GET_PUBLIC_IP);

	//LOGV("result code:%d",c.getResultCode());
	//LOGV("result data:%s",c.recvBuf);

	if (2 != c.getResultCode() /100) {
		LOGV("error code:%d",c.getResultCode());
		LOGV("result data:%s",c.recvBuf);
	}

	return std::string(c.getResultData());
}

bool sendLennyLicenseToEmail(const char *const email,const char *const mac,const char *const license,const unsigned days) {

	std::ostringstream ss;
	ss << "{";
		ss << "\"v\":\"" <<APP_VERSION_MAJOR <<"." <<APP_VERSION_MINOR <<":" <<LICENSE_APP_VERSION <<"\"";
		ss << ",";
		ss << "\"e\":\"" <<email <<"\"";
		ss << ",";
		ss << "\"m\":\"" <<mac <<"\"";
		ss << ",";
		ss << "\"l\":\"" <<license <<"\"";
		ss << ",";
		ss << "\"d\":" <<days;
	ss << "}";

	WebClient c;
	c.send(WS_HOST,WS_PORT,WS_POST_LICENSE_EMAIL,ss.str().c_str());

	if (2 != c.getResultCode() /100) {
		LOGV("failed result code:%d",c.getResultCode());
		return false;
	}

	LOGV("result code:%d",c.getResultCode());
	//LOGV("result data:%s",c.recvBuf);
	LOGV("result data:%s",c.getResultData());
//	return std::string(c.getResultData());
	return true;
}

#include "json.h"
#include <string.h> //strtok_r

bool getLennyVersionInfo() {
	WebClient c;
	c.send(WS_HOST,WS_PORT,WS_GET_VERSION_INFO);

	if (2 != c.getResultCode() /100) {
		LOGV("failed result code:%d",c.getResultCode());
		return false;;
	}

	bool result = false;
	JSONItem *const json = JSONItem::parse((char *)c.getResultData());

	if (NULL == json) {
		LOGV("json_parse failed");
	} else {
    	JSONItem *const v = json->get("v");
    	if (NULL == v || JSON_TYPE_TEXT != v->type) {
    		LOGV("v is null");
    	} else
    	if (10 < strlen(v->text_value)) {
    		LOGV("v 10 < len : %s",v->text_value);
    	} else {

			extern Config config;
    		config.loadServerVersion(v->text_value);
			result = true;    		
    	}

		JSONItem::free(json);
    }

	return result;
}

bool sendLennyComments() {
	extern std::string getLatestComments();

	std::string str = getLatestComments();
	if (0 == str.length()) return true;

	WebClient c;
	c.send(WS_HOST,WS_PORT,WS_POST_COMMENT,str.c_str());

	if (2 != c.getResultCode() /100) {
		LOGV("failed result code:%d",c.getResultCode());
		return false;
	}

	return true;
}

bool sendLennyBugs() {
	extern std::string getLatestBugReports();

	std::string str = getLatestBugReports();
	if (0 == str.length()) return true;

	WebClient c;
	c.send(WS_HOST,WS_PORT,WS_POST_BUG_REPORT,str.c_str());

	if (2 != c.getResultCode() /100) {
		LOGV("failed result code:%d",c.getResultCode());
		return false;
	}

	return true;
}

bool sendLennyLog() {
	//
	// DOS protection
	//
	static DenialOfService dos(60,10);
	if (!dos.check()) {
		LOGW("DenialOfService check failed");
		return false;
	}

	extern char PATH_LOG_FILE[];
	LOGV("sendLennyLog %s",PATH_LOG_FILE);

	WebClient c;
	c.sendFile(WS_HOST,WS_PORT,WS_POST_LOG_FILE,PATH_LOG_FILE);

	if (2 != c.getResultCode() /100) {
		LOGV("failed result code:%d",c.getResultCode());
		return false;
	}

	return true;
}

bool sendLennyStats() {

	extern char PATH_STATS_FILE[];
	LOGV("sendLennyStats %s",PATH_STATS_FILE);

	WebClient c;
	c.sendFile(WS_HOST,WS_PORT,WS_POST_STATS_FILE,PATH_STATS_FILE);

	if (2 != c.getResultCode() /100) {
		LOGV("failed result code:%d",c.getResultCode());
		return false;
	}

	return true;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - -

//
//
//
bool signinValidate(const char *const name,const char *const pwd,std::ostringstream &ss) {

#ifndef LINUX
	(void)name;(void)pwd;

	ss << "\"err\":0";
	return true;

#else //LINUX

	char f[128]; sprintf(f,"sudo cat /etc/shadow | grep %s",name);
	std::string s = exec(f);

	if (0 == s.size()) {
		ss << "\"err\":-12,\"err_msg\":\"user not found\"";
		return false;
	}

	//
	// hash size: MD5 (22 bytes),SHA-256 (43 bytes),SHA-512 (86 bytes)
	//
	char acct[32],type[8],salt[32],hash[100];

	{
		char *sttptr;
		char *tt = strtok_r((char*)s.c_str(),":",&sttptr);

		strncpy(acct,tt,sizeof(acct)-1);

		tt = strtok_r(NULL,":",&sttptr);
		if ('$' != *tt) {
			ss << "\"err\":-13,\"err_msg\":\"user invalid\"";
			return false;
		}
		{
			char *stptr;
			char *t;

			t = strtok_r(tt,"$",&stptr);
			strncpy(type,t,sizeof(type)-1);

			t = strtok_r(NULL,"$",&stptr);
			strncpy(salt,t,sizeof(salt)-1);

			t = strtok_r(NULL,"$",&stptr);
			strncpy(hash,t,sizeof(hash)-1);
		}

		//LOGV("acct:%s type:%s salt:%s hash:%s",acct,type,salt,hash);

		char salted[128]; sprintf(salted,"$%s$%s$",type,salt);
		char *const hk = crypt(pwd,salted);

		//LOGV("crypt salted:%s hash:%s",salted,hk);
		//LOGV("compare hk:'%s' hash:'%s' cmp:%d",hk,hash,strcmp(hash,hk+strlen(salted)));

		if (0 != strcmp(hash,hk+strlen(salted))) {
			ss << "\"err\":-22,\"err_msg\":\"password invalid\"";
			return false;
		}
	}

	ss << "\"err\":0";
	return true;

#endif //LINUX
}





// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


//
//
//

#include <fcntl.h>
//#include <sys/file.h>
extern char PATH_LOCK_FILE[];
static int lockFd = 0;

bool appFileLock() {

	if (0 != lockFd) { LOGV("already locked"); return true; }

    const int fd = open(PATH_LOCK_FILE,O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (-1 == fd) {
		LOGV("open failed");
		return false;
	}

    {
        int flags = fcntl(fd,F_GETFD);

        if (-1 == flags) {
			LOGV("fcntl failed");
			close(fd);
			return false;
		}

        flags |= FD_CLOEXEC;

        if (-1 == fcntl(fd,F_SETFD,flags)) {
			LOGV("fcntl failed");
			close(fd);
			return false;
		}
    }

    {
		struct flock fl;
		fl.l_type = F_WRLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start = 0;
		fl.l_len = 0;

		if (-1 == fcntl(fd,F_SETLK,&fl)) {
	        if (EAGAIN == errno || EACCES == errno) {
	            LOGV("LOCKED");
				close(fd);
	            return false;
	        }

		    LOGV("lockRegion failed");
			close(fd);
		    return false;
	    }
    }

    if (-1 == ftruncate(fd,0)) {
		LOGV("ftruncate failed");
		close(fd);
		return false;
    }

    char buf[32];
    snprintf(buf,sizeof(buf),"%ld\n",(long)getpid());

    if (strlen(buf) != (unsigned)write(fd,buf,strlen(buf))) {
		LOGV("write failed");
    }

    lockFd = fd;
    return true;
}
void appFileUnLock() {

	if (0 == lockFd) return;
	close(lockFd);
	lockFd = 0;

	unlink(PATH_LOCK_FILE);
}









// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
// Host Name
//

#include <netdb.h> // getaddrinfo

std::string getHostname() {

	char hostname[1024] = {0};
	gethostname(hostname,sizeof(hostname) -1);

#ifdef XDEBUG
	struct addrinfo hints; memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC; //e ither IPV4 or IPV6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;

	struct addrinfo *info;
	const int r = getaddrinfo(hostname,"http",&hints,&info);

	if (0 != r) {
	    LOGV("getaddrinfo failed: %s",gai_strerror(r));
	} else {

		for (struct addrinfo *p=info;p!=NULL;p=p->ai_next) {
			LOGV("hostname: %s",p->ai_canonname);
		}

		freeaddrinfo(info);
	}
#endif //DEBUG

	return std::string(hostname);
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
// MAC address
//

/*
std::string getNetworkInterfaceNames() {

	std::vector<std::string> vec;
#ifdef LINUX
	std::string interface = exec("route | grep '^default' | grep -o '[^ ]*$'");
#endif //LINUX
#ifdef OSX
	std::string interface = exec("route -n get default | grep 'interface:' | grep -o '[^ ]*$'");
#endif //OSX

	char *tptr;
	char *toks = strtok_r((char*)s.c_str(),"\n",&tptr);

	while (toks) {
		vec.push_back(toks);
		toks = strtok_r(NULL,"\n",&tptr);
	}

	return vec;
}
*/


std::string getFirstMacAddress() {
	std::map<std::string,std::string> m = getMacAddresses();
	std::map<std::string,std::string>::iterator it = m.begin();
	if (m.end() == it) return "";
	return it->second;
}

std::map<std::string,std::string> getMacAddresses() {

	std::map<std::string,std::string> map;
	{
		std::map<std::string,std::string> addrs = getNetworkIpV4Addresses();

		for (std::map<std::string,std::string>::iterator it = addrs.begin(); it != addrs.end(); it++) {
			map.insert(std::pair<std::string,std::string>(it->first,getMacAddress(it->first.c_str())));
		}
	}
	return map;
}



//#include <stdlib.h>
//#include <string.h>
#include <sys/socket.h>
#include <net/if.h>

#ifdef LINUX
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

std::string getMacAddress(const char *const iface) {
	//const char *const iface = "eth0";// eth0,wlan0

	struct ifreq ifr;

	{
		int fd = socket(AF_INET,SOCK_DGRAM,0);

		ifr.ifr_addr.sa_family = AF_INET;
		strncpy((char *)ifr.ifr_name,(const char *)iface,IFNAMSIZ-1);

		ioctl(fd, SIOCGIFHWADDR, &ifr);

		close(fd);
	}

	char *mac = (char *)ifr.ifr_hwaddr.sa_data;

	char str[32] = {0};
	sprintf(str,(const char *)"%s:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",iface,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	return std::string(str);
}
#endif //LINUX

#ifdef OSX
#include <sys/sysctl.h>
#include <net/if_dl.h>

std::string getMacAddress(const char *const iface) {
	//const char *const iface = "en0";// en0

	int mib[6];
	mib[0] = CTL_NET;
	mib[1] = AF_ROUTE;
	mib[2] = 0;
	mib[3] = AF_LINK;
	mib[4] = NET_RT_IFLIST;

	if (0 == (mib[5] = (int)if_nametoindex("en0"))) {
		perror("if_nametoindex error");
		return "";
	}

	size_t len;
	if (0 > sysctl(mib,6,NULL,&len,NULL,0)) {
		perror("sysctl 1 error");
		return "";
	}

	char *const buf = (char*)malloc(len);
	if (NULL == buf) {
		perror("malloc error");
		return "";
	}

	if (0 > sysctl(mib,6,buf,&len,NULL,0)) {
		perror("sysctl 2 error");
		return "";
	}

	struct if_msghdr *const ifm = (struct if_msghdr *)buf;
	struct sockaddr_dl *const sdl = (struct sockaddr_dl *)(ifm + 1);
	uint8_t *const ptr = (uint8_t *)LLADDR(sdl);

	char str[32] = {0};
	sprintf(str,"%s:%02x:%02x:%02x:%02x:%02x:%02x",iface,*ptr,*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5));

	return std::string(str);
}
#endif //OSX


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
// Wifi SSID
//

#ifdef LINUX


/*
$ cat /etc/wpa_supplicant/wpa_supplicant.conf
 ...
network={
   ssid="Test Wifi Network"
   psk="SecretPassWord"
}


$ ifconfig wlan0
wlan0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether b8:27:eb:15:65:79  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0


$ ifconfig eth0
eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.1.132  netmask 255.255.255.0  broadcast 192.168.1.255
        inet6 fe80::fff6:db67:eee3:5660  prefixlen 64  scopeid 0x20<link>
        ether b8:27:eb:40:30:2c  txqueuelen 1000  (Ethernet)
        RX packets 196836  bytes 33592106 (32.0 MiB)
        RX errors 0  dropped 193  overruns 0  frame 0
        TX packets 5084  bytes 374236 (365.4 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0


*/



//#include <net/if.h>
//#include <linux/if.h>
//#include <linux/wireless.h>

//bool getSSID() {

/*
	const int fd = socket(AF_INET,SOCK_DGRAM,0);

    if (-1 == fd) {
        LOGV("socket failed errno:%d : %s",errno,strerror(errno));
        return false;
    }

    struct iwreq wreq; memset(wreq,0,sizeof(wreq));
    wreq.u.essid.length = IW_ESSID_MAX_SIZE+1;
    sprintf(wreq.ifr_name,IW_INTERFACE);

    char id[IW_ESSID_MAX_SIZE+1];
    wreq.u.essid.pointer = id;

    if (-1 == ioctl(fd,SIOCGIWESSID,&wreq)) {
        LOGV("ioctl ESSID failed errno:%d : %s",errno,strerror(errno));
	    close(fd);
        return false;
    }
    close(fd);

    LOGV("ESSID: %s\n",(char *)wreq.u.essid.pointer);
*/
//    return true;
//}
#else // LINUX
//bool getSSID() { return true; }

#endif //LINUX






// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
// OS Info
//



/*




OS Name:        Mac OS X
OS Version:     10.14.6
Architecture:   x86_64



$ cat /proc/version 
Linux version 4.19.66-v7+ (dom@buildbot) (gcc version 4.9.3 (crosstool-NG crosstool-ng-1.22.0-88-g8460611)) #1253 SMP Thu Aug 15 11:49:46 BST 2019




$ cat /proc/cpuinfo
...
Hardware	: BCM2835
Revision	: a22082
Serial		: 000000008802b4f8

$ cat /proc/cpuinfo | grep -i serial | cut -d ' ' -f 2
000000008802b4f8

$ cat /proc/cpuinfo | grep - hardware | cut -d ' ' -f 2
BCM2835

$ cat /proc/cpuinfo | grep R-i revision | cut -d ' ' -f 2
a22082






std::string getLinuxInfo() {
	std::ostringstream ss;


$ cat /etc/ *-release
PRETTY_NAME="Raspbian GNU/Linux 8 (jessie)"
NAME="Raspbian GNU/Linux"
VERSION_ID="8"
VERSION="8 (jessie)"
ID=raspbian
ID_LIKE=debian
HOME_URL="http://www.raspbian.org/"
SUPPORT_URL="http://www.raspbian.org/RaspbianForums"
BUG_REPORT_URL="http://www.raspbian.org/RaspbianBugs"

$ lsb_release -a
No LSB modules are available.
Distributor ID:	Raspbian
Description:	Raspbian GNU/Linux 8.0 (jessie)
Release:	8.0
Codename:	jessie

$ hostnamectl
   Static hostname: pi1
         Icon name: computer
           Chassis: n/a
        Machine ID: cbd9c21f96824fc0a90fac89e9463af5
           Boot ID: 65ac5e5b36f3450f96ba1e08b7da3767
  Operating System: Raspbian GNU/Linux 8 (jessie)
            Kernel: Linux 4.14.93-v7+
      Architecture: arm

$ uname -mrs
Linux 4.14.93-v7+ armv7l

$ cat /proc/version
Linux version 4.14.93-v7+ (dom@dom-XPS-13-9370) (gcc version 4.9.3 (crosstool-NG crosstool-ng-1.22.0-88-g8460611)) #1189 SMP Mon Jan 14 16:52:29 GMT 2019



$ cat /etc/issue
Debian GNU/Linux 9

$ cat /etc/os-release

The output should be like:
PRETTY_NAME="Debian GNU/Linux 9 (stretch)"
NAME="Debian GNU/Linux"
VERSION_ID="9"
VERSION="9 (stretch)"
ID=debian
HOME_URL="https://www.debian.org/"
SUPPORT_URL="https://www.debian.org/support"
BUG_REPORT_URL="https://bugs.debian.org/"



$ cat /etc/issue
Linux Mint 18.2 Sonya \n \l

$ cat /etc/issue
Red Hat Enterprise Linux Server release 6.10 (Santiago)
Kernel \r on an \m

$ cat /etc/issue
Amazon Linux AMI release 2010.11.2 (beta)
Kernel \r on an \m


http://linuxmafia.com/faq/Admin/release-files.html
https://superuser.com/questions/344837/determine-linux-distribution

Raspbian       /etc/os-release
Amazon Linux   /etc/system-release

Ubuntu         /etc/lsb-release
Debian         /etc/debian_release, /etc/debian_version

Red Hat,CentOS /etc/redhat-release, /etc/redhat_version
Fedora         /etc/fedora-release, /etc/redhat-release, /etc/os-release

Novell SuSE    /etc/SuSE-release
OpenSuSE       /etc/SuSE-release, /etc/os-release

Slackware      /etc/slackware-release, /etc/slackware-version
PLD Linux      /etc/pld-release, /etc/os-release
ArchLinux      /etc/arch-release, /etc/os-release

Gentoo         /etc/gentoo-release
Mandrake       /etc/mandrake-release
Yellow dog     /etc/yellowdog-release
Sun JDS        /etc/sun-release 
Solaris/Sparc  /etc/release 




	return ss.str();
}
*/



//
//
//

#include <map>
static std::map<std::string,std::string> getSystemReleaseInfo();

//
//
//
std::string getSystemLogo() {
	std::map<std::string,std::string> map = getSystemReleaseInfo();

	for (std::map<std::string,std::string>::iterator it = map.begin(); it != map.end(); it++) {
		if (0 != strcasecmp("id",it->first.c_str())) continue;
		return it->second;
	}
	return "";
}
std::string getSystemPrettyName() {
	std::map<std::string,std::string> map = getSystemReleaseInfo();

	for (std::map<std::string,std::string>::iterator it = map.begin(); it != map.end(); it++) {
		if (0 != strcasecmp("pretty_name",it->first.c_str())) continue;
		return it->second;
	}
	return "";
}

std::map<std::string,std::string> getSystemReleaseInfo() {
	std::string s = exec("cat /etc/os-release");

	//LOGV("info:%s",s.c_str());

	std::map<std::string,std::string> map;
	{
		char *tptr;
		char *toks = strtok_r((char*)s.c_str(),"\n",&tptr);

		while (toks) {

			{
				char *sttptr;

				char *tt = strtok_r(toks,"=",&sttptr);
				char name[32] ={0}; strncpy(name,tt,sizeof(name)-1);

				// lower-case key
				for (unsigned i=0;name[i];i++) name[i] = (char)tolower(name[i]);

				tt = strtok_r(NULL,"=",&sttptr);
				char value[32] ={0}; strncpy(value,tt,sizeof(value)-1);

				// Quote character to space
				while (char *ss = strchr(value,'"')) *ss = ' ';

				map[name] = trim(value);
			}

			toks = strtok_r(NULL,"\n",&tptr);
		}
	}
	return map;
}

/*
std::string getSystemIssue() {
	std::string r = trimRight(exec("cat /etc/issue"));
	{
		for (unsigned i=0;i<r.size();) {
			const uint8_t c = (uint8_t)r.at(i);
			if ('\\' != c) { i++; continue; }
			r = r.substr(0,i);
			break;
		}

		// right trim
	//	while (' ' == r.at(r.size() -1)) r.erase(r.size() -1);
	}
	return r;
}
*/


#ifdef LINUX

#include <usb.h>
#include <libudev.h>

class USBLib {
public:
	struct udev *udev;
	struct udev_hwdb *hwdb;

	USBLib();
	~USBLib();

	std::vector<std::string> enumDevices(bool json) const;

	int get_vendor_string(char *const buf,const size_t size,const uint16_t vid) const;
	int get_product_string(char *const buf,const size_t size,const uint16_t vid,const uint16_t pid) const;

#ifdef XDEBUG
	int get_class_string(char *const buf,const size_t size,const uint8_t cls) const;
#endif //XDEBUG

	const char *names_vendor(const uint16_t vendorid) const;
	const char *names_product(const uint16_t vendorid,const uint16_t productid) const;
	const char *names_class(const uint8_t classid) const;

	const char *hwdb_get(const char *const modalias, const char *const key) const;
};

#endif //LINUX


std::vector<std::string> enumUSBModemDevices(const bool json) {
#ifdef LINUX
	USBLib usb;
	return usb.enumDevices(json);
#else //LINUX
	(void)json;
	std::vector<std::string> r;
	return r;
#endif //LINUX
}



#ifdef LINUX

USBLib::USBLib() {
	usb_init();
	usb_find_busses();
	usb_find_devices();

	udev = udev_new();
	if (udev) {
		hwdb = udev_hwdb_new(udev);
		if (!hwdb) {
			udev = udev_unref(udev);
		}
	}
}

USBLib::~USBLib() {
	if (hwdb) { hwdb = udev_hwdb_unref(hwdb); }
	if (udev) { udev = udev_unref(udev); }
}


static bool sortUSB(std::string l,std::string r) { return 0 > l.compare(r); }

#define USB_COMMUNICATION_DEVICE_CLASS 2

std::vector<std::string> USBLib::enumDevices(const bool json) const {

#ifdef XDEBUG
	{
		char cls[256];
		get_class_string(cls,sizeof(cls),USB_COMMUNICATION_DEVICE_CLASS);
		LOGV("CLASS:%s",cls)
	}
#endif //XDEBUG

	std::vector<std::string> v;

	for (struct usb_bus *bus = usb_busses; bus; bus = bus->next) {

		for (struct usb_device *dev = bus->devices; dev; dev = dev->next) {
			if (USB_COMMUNICATION_DEVICE_CLASS != dev->descriptor.bDeviceClass) continue;

			usb_dev_handle *const h = usb_open(dev);

			if (!h) { LOGV("usb_open failed"); continue; }

			char ven[256]; get_vendor_string(ven,sizeof(ven),dev->descriptor.idVendor);
			char prd[256]; get_product_string(prd,sizeof(prd),dev->descriptor.idVendor,dev->descriptor.idProduct);
			//char mfg[256]; get_vendor_string(mfg,sizeof(mfg),dev->descriptor.iManufacturer);
			//char serial[256]; usb_get_string_simple(h,dev->descriptor.iSerialNumber,serial,sizeof(serial)-1);

			std::ostringstream ss;
			if (json) {
				ss <<"{ \"id\":\"" <<dev->filename <<"\",\"product\":\"" <<prd <<"\",\"vendor\":\"" <<ven <<"\" }";
			} else {
				ss <<dev->filename <<" " <<prd <<" " <<ven;
			}

			v.push_back(ss.str());

			if (h) usb_close(h);
		}
	}

	std::sort(v.begin(),v.end(),sortUSB);

	return v;
}


int USBLib::get_vendor_string(char *const buf,const size_t size,const uint16_t vid) const {
    if (size < 1) return 0;
    *buf = 0;

    const char *cp;
    if (!(cp = names_vendor(vid))) return 0;
    return snprintf(buf,size,"%s",cp);
}

int USBLib::get_product_string(char *const buf,const size_t size,const uint16_t vid,const uint16_t pid) const {
    if (size < 1) return 0;
    *buf = 0;

    const char *cp;
    if (!(cp = names_product(vid, pid))) return 0;
    return snprintf(buf,size,"%s",cp);
}

#ifdef XDEBUG
int USBLib::get_class_string(char *const buf,const size_t size,const uint8_t cls) const {
	if (size < 1) return 0;
	*buf = 0;

	const char *cp;
	if (!(cp = names_class(cls))) return 0;
    return snprintf(buf,size,"%s",cp);
}
#endif //XDEBUG


const char* USBLib::names_vendor(const uint16_t vendorid) const {
	char modalias[64];
	sprintf(modalias,"usb:v%04X*",vendorid);
	return hwdb_get(modalias,"ID_VENDOR_FROM_DATABASE");
}

const char* USBLib::names_product(const uint16_t vendorid,const uint16_t productid) const {
	char modalias[64];
	sprintf(modalias,"usb:v%04Xp%04X*",vendorid,productid);
	return hwdb_get(modalias, "ID_MODEL_FROM_DATABASE");
}


#ifdef XDEBUG
const char* USBLib::names_class(const uint8_t classid) const {
	char modalias[64];
	sprintf(modalias,"usb:v*p*d*dc%02X*",classid);
	return hwdb_get(modalias,"ID_USB_CLASS_FROM_DATABASE");
}
#endif //XDEBUG

const char* USBLib::hwdb_get(const char *const modalias, const char *const key) const {
	struct udev_list_entry *entry;

	udev_list_entry_foreach(entry, udev_hwdb_get_properties_list_entry(hwdb, modalias, 0))
		if (0 == strcmp(udev_list_entry_get_name(entry),key))
			return udev_list_entry_get_value(entry);

	return NULL;
}



#endif //LINUX

#ifdef NOT_INCLUDED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
// Reverse DNS lookup
//


#include <arpa/inet.h>
#include <netdb.h> // getnameinfo

//
// dig +noall +answer -x lennytroll.com
// dig +noall +answer -x 85.187.150.130
// nslookup lennytroll.com
// nslookup 85.187.150.130
//
void dnsReverseLookup(const char *const ip) {
	const uint64_t start = getTickUs();

	// sockaddr_in or sockaddr_in6
	struct sockaddr_in addr; addr.sin_family = AF_INET;

	if (1 != inet_pton(AF_INET,ip,&addr.sin_addr)) {
		LOGV("inet_pton failed");
		return;
	}

	char host[NI_MAXHOST],svc[NI_MAXSERV];
	struct sockaddr *found = NULL;

	// getnameinfo is better than gethostbyaddr
	const int r = getnameinfo((struct sockaddr*)&addr,sizeof(addr),host,sizeof(host),svc,sizeof(svc),NI_NAMEREQD);

	if (0 != r) {
		LOGV("getnameinfo failed r:%d",r);

		const int err = getnameinfo(found,sizeof(struct sockaddr),host,sizeof(host),NULL,0,NI_NUMERICHOST);		
		LOGV("getnameinfo NI_NUMERICHOST err:%d",err);

		if (0 == r) {
			char b[INET_ADDRSTRLEN];
			inet_ntop(AF_INET,found,b,sizeof(b));
			LOGV("inet_pton found %s",b);
		}
		return;
	}

	{
		struct addrinfo hints; memset(&hints,0,sizeof(hints));
		hints.ai_socktype = SOCK_DGRAM; /*dummy*/
		hints.ai_flags = AI_NUMERICHOST;

		struct addrinfo *res = NULL;
		if (0 == getaddrinfo(host,"0",&hints,&res)) {

			{
				char b[INET_ADDRSTRLEN];
				inet_ntop(AF_INET,&((sockaddr_in const *)res->ai_addr)->sin_addr,b,sizeof(b));
				LOGV("malicious PTR record %s",b);
			}


			freeaddrinfo(res);
			LOGV("getaddrinfo failed : bogus PTR record");
			return;
		}
	}

	const unsigned t = (unsigned)(getTickUs() - start);
	LOGV("PTR lookup is FQDN in %dus host:%s svc:%s",t,host,svc);

}

#endif// NOT_INCLUDED


#ifdef NOT_INCLUDED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string getCPUStat() {

	std::string s = exec("/proc/stat");

	char *tptr;
	char *toks = strtok_r((char*)s.c_str(),"\n",&tptr);

	if (NULL == toks) {
		LOGV("strtok_r failed");
	} else {

/*

name,user,nice,system,idle,iowait,irq,softirq,steal,gues,gues_nice

$ cat /proc/stat
cpu  243216 0 49254 100635371 24623 0 4524 0 0 0
cpu0 31605 0 13645 24915653 16193 0 3820 0 0 0
cpu1 60901 0 12640 25248802 4000 0 409 0 0 0
cpu2 85082 0 13776 25221603 2347 0 134 0 0 0
cpu3 65628 0 9193 25249311 2082 0 161 0 0 0
intr 92384942 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 11075959 0 0 0 0 0 123347 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 94207 0 0 87882 0 0 0 0 0 0 0 74580870 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 80133 4787 0 0 0 0 1956334 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
ctxt 27922291
btime 1579543843
processes 19492
procs_running 1
procs_blocked 0
softirq 12373728 2048099 3614044 13 213996 0 0 2061258 3414108 0 1022210

*/

		while (toks) {
			LOGV("cpu line :%s",toks);

			if (0 != strncmp("cpu ",toks,4)) {
				strtok_r(NULL,"\n",&tptr);
				continue;
			}

			char *ttptr;
			char *ttoks = strtok_r(toks +4," ",&ttptr);
			if (NULL == ttoks) { LOGV("strtok_r failed"); break; }

			ttoks = strtok_r(NULL," ",&ttptr);
			if (NULL == ttoks) { LOGV("strtok_r failed"); break; }
			const unsigned long user = std::stol(ttoks); 

			ttoks = strtok_r(NULL," ",&ttptr);
			if (NULL == ttoks) { LOGV("strtok_r failed"); break; }
			const unsigned long nice = std::stol(ttoks); 

			ttoks = strtok_r(NULL," ",&ttptr);
			if (NULL == ttoks) { LOGV("strtok_r failed"); break; }
			const unsigned long system = std::stol(ttoks); 

			ttoks = strtok_r(NULL," ",&ttptr);
			if (NULL == ttoks) { LOGV("strtok_r failed"); break; }
			const unsigned long idle = std::stol(ttoks); 

			ttoks = strtok_r(NULL," ",&ttptr);
			if (NULL == ttoks) { LOGV("strtok_r failed"); break; }
			const unsigned long iowait = std::stol(ttoks); 

LOGV("cpu user:%lu nice:%lu system:%lu idle:%lu iowait:%lu",user,nice,system,idle,iowait);

			break;
		}
	}

/*
 currentUsage.open("/proc/stat",ios::in);
    string line;    
    if(currentUsage.is_open()){

        while(getline(currentUsage,line)){
            if(line.at(0) == 'c' && line.at(1) == 'p' && line.at(2) == 'u'){
                if(line.at(3) == convertIntToString(i).at(0)){
                    int total_ticks = getTotalTicks(line);
                    int work_ticks = getWorkTicks(line);                    
                    int total_over_period =  total_ticks - (prev_total_ticks[i-1]);
                    int work_over_period =   work_ticks - (prev_work_ticks[-1]);
                    cout << prev_total_ticks[i-1] << "\n";
                    cout << prev_work_ticks[i-1] << "\n";
                    prev_work_ticks[i-1] = work_ticks;
                    prev_total_ticks[i-1] = total_ticks;

                    return ((double)work_over_period / (double)total_over_period) * 100.0;
                }
            }
        }	
*/

	return std::string("");
}

#endif //NOT_INCLUDED








// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
// License
//

//#include <string>
//#include <sstream>
//#include <time.h>
#include "license.h"

extern void addDashes(char *dest,const char *src);
extern void removeDashes(char *dest,const char *src);

//
//  appId + '_' + mac_addr + '_' + email
//  'com.slade.lenny_b8:27:eb:02:b4:f8_test@lennytroll.com'
//
//  secret = 'OU812'
static inline void getSecret(char *const buf) { buf[0] = 'O'; buf[1] = 'U'; buf[2] = '8'; buf[3] = '1'; buf[4] = '2'; buf[5] = '\0'; }
//

std::string createAppLicense(const unsigned days,LicenseFlags f,const char *const appId) {

	License lic;
	lic.data = f.flags;

	char expiry[20] = {0}; // "2020-01-31"

	{
		time_t today; time(&today);
		today += (days * SECONDS_IN_DAY);
		today -= (today % SECONDS_IN_DAY); // round to day boundary

		const time_t exp = (uint16_t)(today / SECONDS_IN_DAY); // save days
		formatDays((unsigned)exp,expiry,sizeof(expiry));

		LOGV("expires:%s",expiry);
		lic.date = (uint16_t)exp;
	}

	char secret[32];
	getSecret(secret);

	char l[LICENSE_SIZE +1];
	lic.generate(secret,appId,l);
	LOGV("secret:%s appId:%s = %s",secret,appId,l);

	char dl[LICENSE_SIZE +4 +1] = {0};
	addDashes(dl,l);

	LOGV("GENERATE appId:%s exp:%s flags:0x%X lic:%s",appId,expiry,lic.data,dl);
	return std::string(dl);
}

int getAppLicenseInfo(const char *const license,const char *const appId,time_t &dateMs,LicenseFlags &flags) {

	char secret[32];
	getSecret(secret);

	char l[LICENSE_SIZE +1];
	removeDashes(l,license);

	//LOGV("secret:%s appId:%s = %s",secret,appId,l);

	License lic;
	const Result r = lic.verify(secret,appId,l);

	if (LICENSE_INVALID == r) {
		return LICENSE_INVALID;
	}

	flags = *(LicenseFlags*)&lic.data;

	dateMs = lic.date * SECONDS_IN_DAY;

	if (LICENSE_EXPIRED == r) {
		return LICENSE_EXPIRED;
	}

	return LICENSE_VERIFIED;
}

//
//
//
char *formatDays(const unsigned days,char *const buf,const size_t bufSize) {
	time_t s = days * SECONDS_IN_DAY;
	struct tm *const d = gmtime(&s);
	strftime(buf,bufSize,"%Y-%m-%d",d);
	return buf;
}

void addDashes(char *const dest,const char *const src) {
	const unsigned x = (unsigned)strlen(src);
	unsigned i,j;
	for (j=0,i=0;i<x;i++) {
		if (i>0 && i %4 == 0) dest[j++] = '-';
		dest[j++] = src[i];
	}
	dest[j] = '\0';
}

void removeDashes(char *const dest,const char *const src) {
	const unsigned x = (unsigned)strlen(src);
	char *buf_ptr = dest;
	for (unsigned i=0;i<x;i++) {
		if (src[i] == '-') continue;
		*buf_ptr++ = src[i];
	}
	*buf_ptr = '\0';
}





