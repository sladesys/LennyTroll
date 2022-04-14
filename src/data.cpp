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


//
//
//
#define PATH_ROOT ""

char PATH_ETC[]           = PATH_ROOT "etc";
char PATH_OPT[]           = PATH_ROOT "opt";
char PATH_WEB[]           = PATH_ROOT "opt/web";
char PATH_VAR[]           = PATH_ROOT "var";
char PATH_LOG[]           = PATH_ROOT "var/log";
char PATH_CALLS[]         = PATH_ROOT "var/call";

char PATH_LOG_FILE[]      = PATH_ROOT "var/log/lenny.log";
char PATH_STATS_FILE[]    = PATH_ROOT "var/log/stats.log";
char PATH_LOCK_FILE[]     = PATH_ROOT "var/lenny.pid";

char PATH_CONFIG[]        = PATH_ROOT "etc/lenny.cfg";
char PATH_SSL_PEM[]       = PATH_ROOT "etc/lenny.pem";
char PATH_SSL_KEY[]       = PATH_ROOT "etc/lenny.key";

char PATH_OPT_LENNY[]     = PATH_ROOT "opt/aud/lenny";
char PATH_VAR_LENNY[]     = PATH_ROOT "var/lenny";

char PATH_SKIPLIST[]      = PATH_ROOT "var/skiplist.txt";
char PATH_NEW_CALLS[]     = PATH_ROOT "var/call/new.log";
char PATH_ANSWER_CALLS[]  = PATH_ROOT "var/call/answer.log";
char PATH_HISTORY_CALLS[] = PATH_ROOT "var/call/history.log";

char PATH_DISCONNECTED[]  = PATH_ROOT "opt/aud/disconnected.wav";
char PATH_TAD_DEFAULT[]   = PATH_ROOT "opt/aud/tad_default.wav";
char PATH_TAD_OGM_FMT[]   = PATH_ROOT "var/tad_%d.wav";

char PATH_BUGS_CACHE[]    = PATH_ROOT "var/log/bugs.log";
char PATH_COMMENT_CACHE[] = PATH_ROOT "var/log/comment.log";

//char PATH_TAD_MBOX[]     = PATH_ROOT "var/mbox";
//char PATH_TAD_MBOX_FMT[] = PATH_ROOT "var/mbox/%02d";




//
//
//

#include "data.h"
#include "json.h"
#include "utils.h"

#include <unistd.h> // unlink
#include <dirent.h> //opendir,readdir,closedir
#include <fcntl.h>
#include <sys/stat.h>

#include <vector>
#include <sstream>





/*


std::string string_format(const char *const fmt, ...) {

    std::string str;
    int size = 100;

    while (1) {
        str.resize(size);

	    va_list ap;
        va_start(ap,fmt);
        const int n = vsnprintf(&str[0], size, fmt, ap);
        va_end(ap);

        if (-1 < n && size < n) return str;
//        if (n > -1 && n < size) return str;
        
        size = -1 < n ?n+1 : n+2;
//        if (n > -1)
//            size = n + 1;
//        else
//            size *= 2;
//    }

}

*/


/*
void dump(const JSONItem *const json) {
	LOGV("dump len:%d",json->length);

    JSONItem *j = (JSONItem *)json;
    while (NULL != j) {
        switch (j->type) {
			case JSON_TYPE_NULL   : LOGV("%s : NULL"       ,j->key); break;
			case JSON_TYPE_INTEGER: LOGV("%s : INT %d"     ,j->key,(int)j->int_value); break;
			case JSON_TYPE_DOUBLE : LOGV("%s : DOUBLE %f"  ,j->key,j->dbl_value); break;
			case JSON_TYPE_BOOL   : LOGV("%s : BOOL %s"    ,j->key,0 == j->int_value ?"false":"true"); break;
			case JSON_TYPE_TEXT   : LOGV("%s : STRING '%s'",j->key,j->text_value); break;
            case JSON_TYPE_OBJECT : LOGV("%s : OBJECT [%d]",j->key,j->length); dump(j->child); break;
            case JSON_TYPE_ARRAY  : LOGV("%s : ARRAY [%d]" ,j->key,j->length); for (JSONItem *a = j->child;NULL != a;a = a->next) dump(a); break;
        }
        j = j->next;
    }
}
*/


static std::string _toString(const JSONItem *const json) {
	std::ostringstream ss;

    JSONItem *j = (JSONItem *)json;
    while (NULL != j) {
    	if (0 < ss.str().size()) ss << ",";

        switch (j->type) {
			case JSON_TYPE_NULL   : break;
			case JSON_TYPE_INTEGER: ss << "\"" <<j->key <<"\":" <<j->int_value; break;
			case JSON_TYPE_DOUBLE : ss << "\"" <<j->key <<"\":" <<j->dbl_value; ; break;
			case JSON_TYPE_BOOL   : ss << "\"" <<j->key <<"\":" <<(0 == j->int_value ?"false":"true"); break;
			case JSON_TYPE_TEXT   : ss << "\"" <<j->key <<"\":\"" <<j->text_value <<"\""; break;
            case JSON_TYPE_OBJECT : if (NULL != j->key) { ss << "\"" <<j->key <<"\":"; } ss << "{" <<_toString(j->child) <<"}"; break;
            case JSON_TYPE_ARRAY  : if (NULL != j->key) { ss << "\"" <<j->key <<"\":"; }
            {
	            ss <<"[";
            	unsigned c=0;
            	for (JSONItem *a = j->child;NULL != a;a = a->next) {
    				if (0 < c++) ss << ",";
            		ss << _toString(a);
            	}
            	ss << "]";
            	break;
            }
        }
        j = j->next;
    }
    return ss.str();
}

std::string toString(const JSONItem *const j) {
	std::ostringstream ss;

    switch (j->type) {
    	default: ss << _toString(j); break;
		case JSON_TYPE_NULL   : break;
        case JSON_TYPE_OBJECT : if (NULL != j->key) { ss << "\"" <<j->key <<"\":"; } ss << "{" <<_toString(j->child) <<"}"; break;
        case JSON_TYPE_ARRAY  : if (NULL != j->key) { ss << "\"" <<j->key <<"\":"; }
        {
	        ss << "[";
        	unsigned c=0;
        	for (JSONItem *a = j->child;NULL != a;a = a->next) {
				if (0 < c++) ss << ",";
        		ss << _toString(a);
        	}
        	ss << "]";
        	break;
        }
    }
    return ss.str();
}





//
//
//

int shareWAVFile(void *const sock,const unsigned line,const char *const file) {
	LOGD("shareWAVFile line:%d file:%s",line,file);

	FILE *const fd = fopen(file,"rb");

	if (NULL == fd) {
		LOGV("fopen failed: %s",file);
		return 0;
	}

	{
		fseek(fd,0,SEEK_END);
		const long len = ftell(fd);
		fseek(fd,0,SEEK_SET);

		uint8_t pkt[4];
		pkt[0] = (uint8_t)(0x40 | (line &0x0F));
		pkt[1] = (uint8_t)(len >> 16);
		pkt[2] = (uint8_t)(len >> 8);
		pkt[3] = (uint8_t)(len >> 0);

		const bool ok = wsSendBinary(sock,4,pkt);

		if (!ok) {
			LOGV("wsSendBinary failed");

			fclose(fd);
			return -1;
		}
	}

	int total = 0;
	{
		uint8_t buf[8000];

		while (true) {
			// read chunk
			const size_t r = fread(buf,1,sizeof(buf),fd);
			if (0 >= r) { break; }

			// write chunk
			const bool ok = wsSendBinary(sock,(unsigned)r,buf);

			if (!ok) {
				LOGV("wsSendBinary failed");
				break;
			}

			total += (unsigned)r;
		}

		LOGD("finish file:%s total:%d",file,total);
	}

	fclose(fd);

	return (int)total;
}




// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


static int dirFilterWAV(const struct dirent *e);
static int dirCompare(const struct dirent **l,const struct dirent **r);

//
//
//
int dirFilterWAV(const struct dirent *const e) {
	//LOGV("dirFilterWAV %s",e->d_name);
	const unsigned l = (unsigned)strlen(e->d_name);
	if (4 >= l) return false;

	const char *const ext = e->d_name +(l-4); 
	return 0 == strncmp(".wav",ext,4);
}
int dirCompare(const struct dirent **const l,const struct dirent **const r) {
    return strcmp((*r)->d_name,(*l)->d_name);
}




//
//
//

/*
DIR *getMailBoxDir(const unsigned box) {
	char dir[PATH_MAX];
	sprintf(dir,PATH_TAD_MBOX_FMT,box);

	DIR *d = opendir(dir);

	if (NULL == d) {
		struct stat st;
		if (0 != stat(PATH_VAR,&st)) {
			mkdir(PATH_VAR,S_IRWXU);
			mkdir(PATH_TAD_MBOX,S_IRWXU);
			mkdir(dir,S_IRWXU);
			d = opendir(dir);
		} else
	    if (0 == (st.st_mode & S_IFDIR)) {
			unlink(PATH_VAR);
			mkdir(PATH_VAR,S_IRWXU);
			mkdir(PATH_TAD_MBOX_FMT,S_IRWXU);
			mkdir(dir,S_IRWXU);
			d = opendir(dir);
	    } else
		if (0 != stat(PATH_TAD_MBOX_FMT,&st)) {
			mkdir(PATH_TAD_MBOX_FMT,S_IRWXU);
			d = opendir(dir);
		} else
	    if (0 == (st.st_mode & S_IFDIR)) {
			unlink(PATH_TAD_MBOX_FMT);
			mkdir(PATH_TAD_MBOX_FMT,S_IRWXU);
			mkdir(dir,S_IRWXU);
			d = opendir(dir);
	    } else
		if (0 != stat(dir,&st)) {
			mkdir(dir,S_IRWXU);
		} else
	    if (0 == (st.st_mode & S_IFDIR)) {
			unlink(PATH_TAD_MBOX_FMT);
			mkdir(dir,S_IRWXU);
			d = opendir(dir);
	    }
	}

	return d;
}
*/




//
//
//
bool addLennyProfile(std::vector<std::string> &profiles,const char *const name) {
	if (0 == strcasecmp("Lenny",name)) {
		LOGV("addLennyProfile not adding 'Lenny'");
		return false;
	}

	LOGD("addLennyProfile :%s",name);

	for (std::vector<std::string>::iterator p = profiles.begin();p<profiles.end();p++) {
		if (0 != strcasecmp((*p).c_str(),name)) continue;
		LOGV("already present :%s",name);
		return false;
	}

	profiles.push_back(name);
	return true;
}

void removeLennyProfile(std::vector<std::string> &profiles,const char *const name) {
	if (0 == strcasecmp("Lenny",name)) {
		LOGV("removeLennyProfile not removing 'Lenny'");
		return;
	}

	LOGD("removeLennyProfile :%s",name);

	for (std::vector<std::string>::iterator p = profiles.begin();p<profiles.end();p++) {
		if (0 != strcasecmp((*p).c_str(),name)) continue;
		profiles.erase(p);
		break;
	}

	std::vector<std::string> fs = getLennyFiles(name,true);
	for (std::vector<std::string>::iterator p = fs.begin();p<fs.end();p++) {
		LOGV("unlink profile file:%s",(*p).c_str());
		if (0 != unlink((*p).c_str())) {
			LOGV("unlink failed '%s' errno:%d",(*p).c_str(),errno);
		}
	}
}

void sendLennyProfiles(const std::vector<std::string> &profiles,void *const sock) {
	//LOGV("sendLennyProfile :%s",profile);

	std::ostringstream ss;

	ss << "{\"profiles\":{";
    for (unsigned i=0; i<profiles.size();i++) {
    	if (0 < i) ss <<",";
    	ss <<"\"" <<profiles[i] <<"\":[";
//		ss <<getLennyProfile(profiles[i].c_str());
    	{
			std::vector<std::string> f = getLennyFiles(profiles[i].c_str(),false);
			unsigned c = 0;
			for (std::vector<std::string>::iterator p = f.begin();p<f.end();p++) {
				if (0 < c++) ss << ",";
				ss << "\"" << *p <<"\"";
			}		
    	}
		ss <<"]";
	}
	ss << "}}";

	if (NULL == sock) {
		wsSendAll(ss.str().c_str());
	} else {
	    wsSendText(sock,ss.str().c_str());
	}
}

std::string getLennyProfile(const char *const profile) {
	LOGD("getLennyProfile :%s",profile);
	std::ostringstream ss;

	ss << "{\"profile\":\"" <<profile <<"\"";
	ss << ",";
	ss << "\"files\":[";
	{
		std::vector<std::string> f = getLennyFiles(profile,false);
		unsigned c = 0;
		for (std::vector<std::string>::iterator p = f.begin();p<f.end();p++) {
			if (0 < c++) ss << ",";
			ss << "\"" << *p <<"\"";
		}
	}
	ss << "]";

	ss << "}";
	return ss.str();
}

std::vector<std::string> getLennyFiles(const char *const profile,const bool includePath) {

	char lwr[32] ={0};

	// to lowercase with underscore
	const unsigned len = MIN(sizeof(lwr),(unsigned)strlen(profile));
	{
		for (unsigned i=0;i<len;i++) {
			lwr[i] = (char)tolower(profile[i]);
		}
		strcat(lwr,"_");
	}	

	std::vector<std::string> l;

	const char *const dir = (0 == strcasecmp("lenny",profile) ?PATH_OPT_LENNY :PATH_VAR_LENNY);
	LOGV("profile:%s dir:%s",profile,dir);

	{
		struct dirent **nl;
		int i = scandir(dir,&nl,0,dirCompare);

		if (0 > i) {
			LOGV("audio directory not found");
			perror("scandir");
			return l;
		}

	//	LOGV("getLennyFiles compare prefix %d :%s",1+len,lwr);

		while (i--) {

		//	LOGV("getLennyFiles :%s",nl[i]->d_name);

			if (0 != strncmp(nl[i]->d_name,lwr,1+len)) {
				// not included
			} else {

			//	LOGV("adding %s",nl[i]->d_name);

				std::ostringstream ss;
				if (includePath) ss <<dir <<"/";
				ss << nl[i]->d_name;

				l.push_back(ss.str());
			}

			free(nl[i]);
       }
       free(nl);
	}

	return l;
}




//
//
//
static std::string parseIntoJsonArray(std::string buf);
static std::string findLog(const char *file,const char *line,const char *date,const char *time);
static void pruneLog(const char *file,const char *line,const char *date,const char *time);


void addCall(const char *const line,const char *const callerId) {
    //LOGV("addCall");
	{
		FILE *const fd = fopen(PATH_ANSWER_CALLS,"a");
		fprintf(fd,"{\"line\":\"%s\",%s}\n",line,callerId);
		fclose(fd);
	}
	{
		FILE *const fd = fopen(PATH_NEW_CALLS,"a");
		fprintf(fd,"{\"line\":\"%s\",%s}\n",line,callerId);
		fclose(fd);
	}
}

void addHistory(JSONItem *const it,const char *const line,const char *const date,const char *const time) {
    //LOGV("addHistory");

	//pruneLog(PATH_NEW_CALLS,line,date,time);

    std::string str = findLog(PATH_ANSWER_CALLS,line,date,time);
    pruneLog(PATH_ANSWER_CALLS,line,date,time);

	if (0 == str.size()) {
	    LOGV("findLog failed line:%s date:%s time:%s",line,date,time);
	}
	{
		FILE *const fd = fopen(PATH_HISTORY_CALLS,"a");
		fprintf(fd,"%s\n",toString(it).c_str());
		fclose(fd);
	}
}

void pruneCalls(const char *const line,const char *const date,const char *const time) {
    //LOGV("pruneCalls");
    pruneLog(PATH_ANSWER_CALLS,line,date,time);
}

void pruneNewCalls(const char *const line,const char *const date,const char *const time) {
    //LOGV("pruneNewCalls");
    pruneLog(PATH_NEW_CALLS,line,date,time);
}

void pruneHistory(const char *const line,const char *const date,const char *const time) {
    //LOGV("pruneHistory");
    pruneLog(PATH_HISTORY_CALLS,line,date,time);
}
void pruneMessage(const char *const file) {
    //LOGV("pruneMessage");
	char f[PATH_MAX]; sprintf(f,"%s/%s",PATH_CALLS,file);
	if (0 != unlink(f)) {
		LOGV("unlink failed '%s' errno:%d",f,errno);
	}
}





//
//
//
static std::string getCallAudioMessages();

void sendCalls(void *const sock) {
	LOGD("sendCalls");

	std::ostringstream ss; ss << "{\"calls\":{";

		ss << "\"answered\":" << parseIntoJsonArray(readFile(PATH_ANSWER_CALLS));
		ss << ",";
		ss << "\"history\":" << parseIntoJsonArray(readFile(PATH_HISTORY_CALLS));
		ss << ",";
		ss << "\"new\":" << parseIntoJsonArray(readFile(PATH_NEW_CALLS));
		ss << ",";
		ss << "\"messages\":" << getCallAudioMessages();

	ss << "}}";

	if (NULL == sock) {
		wsSendAll(ss.str().c_str());
	} else {
	    wsSendText(sock,ss.str().c_str());
	}
}

std::string getCallAudioMessages() {
	std::ostringstream ss;
	ss << "[";
	{
		struct dirent **nl;
		int i = scandir(PATH_CALLS,&nl,dirFilterWAV,dirCompare);

		if (0 > i) {
			LOGV("directory not found %s",PATH_CALLS);
			perror("scandir");
		} else {

			unsigned c = 0;
			while (i--) {

			//	std::ostringstream ss; ss <<PATH_CALLS << nl[i]->d_name;
			//	l.push_back(ss.str());

	            //
	            // Calculate time in millseconds
	            //
	            unsigned ms = 0;
	            {
	            	char f[PATH_MAX]; sprintf(f,"%s/%s",PATH_CALLS,nl[i]->d_name);
					struct stat st;
					if (0 != stat(f,&st)) {
						LOGV("stat failed f:%s",f);
						continue;
					} else
					if (44 >= st.st_size) {
						LOGC("unlink small file:%s",f);
						if (0 != unlink(f)) {
							LOGV("unlink failed '%s' errno:%d",f,errno);
						}
						continue;
					} else {
						ms = (unsigned)((st.st_size - 44) /8);
					}
				}

	            if (0 < c++) ss <<",";
				ss <<"{" <<"\"file\":\"" << nl[i]->d_name <<"\",\"ms\":" <<ms <<"}";

				free(nl[i]);
	       }
	       free(nl);
		}
	}
	ss <<"]";

	return ss.str();
}





//
//
//

std::map<std::string,std::string> mapNumberToSkiplist;

//
//
//
bool inSkiplist(const char *const number,const char *const name) {

	const std::map<std::string,std::string>::iterator it = mapNumberToSkiplist.find(number);

	if (mapNumberToSkiplist.end() == it) {
		return false;
	}

	//LOGV("match %s : %s",it->first.c_str(),it->second.c_str());

	char v[1024]; strcpy(v,it->second.c_str());
	JSONItem *const json = JSONItem::parse(v);

	if (NULL == json) {
		LOGV("json_parse failed : %s",v);
		return false;
	}

	bool result = true;

	{
		JSONItem *const nam = json->get("name");

		if (NULL == nam) {
			LOGV("name is null, wildcard match");
		} else
		if (JSON_TYPE_TEXT != nam->type) {
			LOGV("name is not a TEXT, wildcard match");
		} else
		if (0 == strlen(nam->text_value)) {
			LOGV("name is empty, wildcard match");
		} else {
			result = (0 == strcmp(name,nam->text_value));
		}
	}

	JSONItem::free(json);
	return result;
}

void cacheSkiplist() {
	LOGC("cacheSkiplist");

	std::string s = readSkiplist();
	JSONItem *const json = JSONItem::parse(s.c_str());

	if (NULL == json) {
		LOGV("json_parse failed");
		return;
	}

	JSONItem *const wl = json->get("skiplist");
	if (NULL == wl) {
		LOGV("json_parse skiplist failed");
	} else {

		mapNumberToSkiplist.clear();

		for (int i=0;i<wl->length;i++) {
			JSONItem *const it = wl->item(i);
			JSONItem *const name = it->get("name");
			JSONItem *const num  = it->get("number");
			if (NULL == name) {
				LOGV("name is null");
			} else
			if (NULL == num) {
				LOGV("num is null");
			} else {
				std::string str = toString(it);

				LOGD("mapNumberToSkiplist insert %s : %s",num->text_value,str.c_str());

				mapNumberToSkiplist.insert(std::pair<std::string,std::string>(num->text_value,str));
			}
		}
	}

	JSONItem::free(json);
}

void addSkiplist(const char *name,const char *number,const char *note) {
	LOGD("addSkiplist name:%s num:%s note:%s",name,number,note);

	pruneSkiplist(number);

	FILE *const fd = fopen(PATH_SKIPLIST,"a");
	fprintf(fd,"{ \"name\":\"%s\",\"number\":\"%s\",\"note\":\"%s\" }\n",name,number,note);
	fclose(fd);

	cacheSkiplist();
}
std::string readSkiplist() {
	std::ostringstream ss;
	ss << "{\"skiplist\":" << parseIntoJsonArray(readFile(PATH_SKIPLIST)) << "}";
	return ss.str();
}
void sendSkiplist(void *sock) {
    LOGD("sendSkiplist");
	if (NULL == sock) {
		wsSendAll(readSkiplist().c_str());
	} else {
	    wsSendText(sock,readSkiplist().c_str());
	}
}
void pruneSkiplist(const char *number) {
	LOGD("pruneSkiplist num:%s",number);

	std::string str = parseIntoJsonArray(readFile(PATH_SKIPLIST));

    if (0 == str.size()) {
        LOGV("readLog failed");
        return;
    }

	std::ostringstream ss;

    {
		JSONItem *const l = JSONItem::parse(str.c_str());

		if (NULL == l) {
			LOGV("json_parse failed");
			return;
		}

		for (int i=0;i<l->length;i++) {
			JSONItem *const it = l->item(i);

			JSONItem *const num = it->get("number");
			if (NULL == num) {
				LOGV("invalid");
				continue;
			}
			if (0 == strcmp(number,num->text_value)) {
				LOGV("pruning %s",number);
				continue;
			}

			ss << toString(it) << "\n";
		}

		JSONItem::free(l);
	}
	{
		std::string r = ss.str();

		{
			char f[PATH_MAX]; sprintf(f,"%s.old",PATH_SKIPLIST);
			rename(PATH_SKIPLIST,f);
		}
		{
			FILE *const fd = fopen(PATH_SKIPLIST,"w");
			const size_t wr = 0 == r.size() ?1 :fwrite(r.c_str(),r.size(),1,fd);
			fclose(fd);
			if (1 != wr) {
				LOGD("fwrite failed len:%d",(unsigned)r.size());
			}
		}
	}

	cacheSkiplist();
}


//
//
//

std::string findLog(const char *const file,const char *const line,const char *const date,const char *const time) {
    //LOGV("findLog %s",file);

    std::string buf = readFile(file);

	std::string r;

    if (0 == buf.length()) {
        LOGV("readFile failed");
        return r;
    }

	{
	    std::string str = parseIntoJsonArray(readFile(file));

	#ifdef DEBUG
	//	if (LOGGING_DATA) dump(str.length(),(uint8_t*)str.c_str());
	#endif //DEBUG


		JSONItem *const l = JSONItem::parse(str.c_str());

		if (NULL == l) {
			LOGV("json_parse failed");
			return r;
		}


		for (int i=0;i<l->length;i++) {
			JSONItem *const it = l->item(i);

			JSONItem *const lin = it->get("line");
			JSONItem *const dat = it->get("date");
			JSONItem *const tim = it->get("time");

			if (NULL == lin) {
				LOGV("null line i:%d",i);
				
			//	dump(str.length(),(uint8_t*)str.c_str());
			//	dump(l);
				
				continue;
			}
			if (NULL == dat) {
				LOGV("null date i:%d",i);
				
			//	dump(str.length(),(uint8_t*)str.c_str());
			//	dump(l);
				
				continue;
			}
			if (NULL == tim) {
				LOGV("null time i:%d",i);
				
			//	dump(str.length(),(uint8_t*)str.c_str());
			//	dump(l);
				
				continue;
			}

			if (NULL == lin || NULL == dat || NULL == tim) {
				LOGV("null values i:%d",i);
				continue;
			}

			if ((0 == strcmp(lin->text_value,line)) &&
			    (0 == strcmp(dat->text_value,date)) &&
			    (0 == strcmp(tim->text_value,time))) {
			//	LOGV("found it");
			//	r.append(toString(it));
				std::string s = toString(it);
			//	LOGV("found %s",s.c_str());
				r.append(s);
				break;
			}		
		}

		JSONItem::free(l);
	}

	return r;
}


void pruneLog(const char *const file,const char *const line,const char *const date,const char *const time) {
    //LOGD("pruneLog %s",file);

/*
    const uint8_t *const buf = readFile(file);

    if (NULL == buf) {
        LOGV("readFile failed");
        return;
    }
*/

	std::ostringstream ostr;
	{
	    std::string str = parseIntoJsonArray(readFile(file));

	#ifdef DEBUG
	//	if (LOGGING_DATA) dump(str.length(),(uint8_t*)str.c_str());
	#endif //DEBUG


		JSONItem *const l = JSONItem::parse(str.c_str());

		if (NULL == l) {
			LOGV("json_parse failed");
			return;
		}


		for (int i=0;i<l->length;i++) {
			JSONItem *const it = l->item(i);

			JSONItem *const lin = it->get("line");
			JSONItem *const dat = it->get("date");
			JSONItem *const tim = it->get("time");

			if (NULL == lin) {
				LOGV("null line i:%d",i);
				
			//	dump(str.length(),(uint8_t*)str.c_str());
			//	dump(l);
				
				continue;
			}
			if (NULL == dat) {
				LOGV("null date i:%d",i);
				
			//	dump(str.length(),(uint8_t*)str.c_str());
			//	dump(l);
				
				continue;
			}
			if (NULL == tim) {
				LOGV("null time i:%d",i);
				
			//	dump(str.length(),(uint8_t*)str.c_str());
			//	dump(l);
				
				continue;
			}

			if (NULL == lin || NULL == dat || NULL == tim) {
				LOGV("null values i:%d",i);
				continue;
			}

			if ((0 == strcmp(lin->text_value,line)) &&
			    (0 == strcmp(dat->text_value,date)) &&
			    (0 == strcmp(tim->text_value,time))) {
			//	LOGV("found it");
				continue;
			}

			ostr << toString(it) << "\n";
		}

		JSONItem::free(l);
	}

	{
		std::string r = ostr.str();

		{
			char f[PATH_MAX]; sprintf(f,"%s.old",file);
			rename(file,f);
		}
		{
			FILE *const fd = fopen(file,"w");
			const size_t wr = 0 == r.size() ?1 :fwrite(r.c_str(),r.size(),1,fd);
			fclose(fd);
			if (1 != wr) {
				LOGD("fwrite failed wr:%d len:%d file:%s",wr,(unsigned)r.size(),file);
			}
		}
	}
}



//
//
//
#include "config.h"
extern Config config;
extern void threadServiceSchudule();

void cacheComment(const unsigned rating,const char *const comment) {
	if ((1 > rating || rating > 5) && 0 == strlen(comment)) {
		LOGV("invalid data rating:%d comment:%s",rating,comment);
		return;
	}

	FILE *const fd = fopen(PATH_COMMENT_CACHE,"a");
	fprintf(fd,"{ \"email\":\"%s\",\"rating\":%d,\"comment\":\"%s\" }\n",config.email.c_str(),rating,comment);
	fclose(fd);

	threadServiceSchudule();
}
void cacheBugReport(const char *const summary,const char *const comment) {
	if (0 == strlen(summary) && 0 == strlen(comment)) {
		LOGV("invalid data summary:%d comment:%s",summary,comment);
		return;
	}

	FILE *const fd = fopen(PATH_BUGS_CACHE,"a");
	fprintf(fd,"{ \"email\":\"%s\",\"summary\":\"%s\",\"comment\":\"%s\" }\n",config.email.c_str(),summary,comment);
	fclose(fd);

	threadServiceSchudule();
}


std::string getLatestComments() {
	std::string r = readFile(PATH_COMMENT_CACHE);
	if (0 < r.length()) {

		if (0 != unlink(PATH_COMMENT_CACHE)) {
			LOGV("unlink failed '%s' errno:%d",PATH_COMMENT_CACHE,errno);
		}

		char f[PATH_MAX]; strcpy(f,PATH_COMMENT_CACHE); strcat(f,".old");
		FILE *const fd = fopen(f,"a");
		const size_t wr = fwrite(r.c_str(),r.size(),1,fd);
		fclose(fd);
		if (1 != wr) {
			LOGD("fwrite failed len:%d file:%s",(unsigned)r.size(),f);
		}
	}

	return r;
}

std::string getLatestBugReports() {
	std::string r = readFile(PATH_BUGS_CACHE);
	if (0 < r.length()) {

		if (0 != unlink(PATH_BUGS_CACHE)) {
			LOGV("unlink failed '%s' errno:%d",PATH_BUGS_CACHE,errno);
		}

		char f[PATH_MAX]; strcpy(f,PATH_BUGS_CACHE); strcat(f,".old");
		FILE *const fd = fopen(f,"a");
		const size_t wr = fwrite(r.c_str(),r.size(),1,fd);
		fclose(fd);
		if (1 != wr) {
			LOGD("fwrite failed len:%d file:%s",(unsigned)r.size(),f);
		}
	}

	return r;
}


std::string parseIntoJsonArray(std::string buf) {

	std::ostringstream ss;

    if (0 == buf.length()) {
        //LOGV("readFile failed %s",file);
        ss << "[]";
    } else {
        ss << "[";
        char *tok = strtok((char*)buf.c_str(),"\n");
        unsigned c = 0;
        while (NULL != tok) {
            if (0 < c++) ss << ",";
            ss << tok;
            tok = strtok(NULL,"\n");
        }
        ss << "]";
    }
    return ss.str();
}


