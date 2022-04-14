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
#include "json.h"
#include "lenny.h"
#include "biz.h"
#include "license.h"
#include <unistd.h> // unlink


//
// Externals
//
extern Lenny *getLenny(unsigned line);
extern std::string getConfig();
extern std::string getStatus();

//
// Internals
//
static void handleRequest(void *sock,JSONItem *json);
static void handleUpdateCheck(void *sock,JSONItem *json);
static void handleLicenseAction(void *sock,JSONItem *json);
static void handleSignInAction(void *sock,JSONItem *json);
static void handleCallAction(void *sock,JSONItem *json);
static void handleProfileAction(void *sock,JSONItem *json);
static void handleSkiplistAction(void *sock,JSONItem *json);
static void handleLineAction(void *sock,JSONItem *json);

static void sendSystemInfo(void *sock);
extern void sendStatus(void *sock =NULL);
static void sendConfig(void *sock =NULL);

static std::string getConfig(Lenny &l);
static std::string getStatus(Lenny &l);



//
//
//

extern Config config;
extern WSCallback cbs;
static std::map<unsigned,Lenny*> mapLineToLenny;
static std::vector<void*> listSockets;
static char openFile[PATH_MAX];



//
//
//
void startLenny() {
	if (0 < mapLineToLenny.size()) { LOGC("alredy running"); return; }
	LOGC(" ");

#ifdef LINUX
	if (0 < strlen(config.line1.device)) {
		LOGC("line 1");

		Lenny *const l = new Lenny(0);
		if (!l->start(config.line1)) {
			delete l;
			//
			// Modem connect failed
			//
		} else {

			mapLineToLenny[1] = l;

			LOGV("sleep +");
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			LOGV("sleep -");
		}

	}
	if (0 < strlen(config.line2.device)) {
		LOGC("line 2");
		Lenny *const l = new Lenny(1);
		if (!l->start(config.line2)) {
			delete l;
			//
			// Modem connect failed
			//
		} else {

			mapLineToLenny[2] = l;

		}
	}
#endif //LINUX

	cacheSkiplist();

	sendConfig();
	sendStatus();
}

void stopLenny() {
	if (0 == mapLineToLenny.size()) return;
	LOGC(" ");

	std::vector<unsigned> v;
	for (std::map<unsigned,Lenny*>::iterator it = mapLineToLenny.begin(); it != mapLineToLenny.end(); it++) {
		v.push_back(it->first);
	}

/*
	for (std::vector<unsigned>::iterator it = v.begin(); it != v.end(); it++) {
		const unsigned idx = *it;

		Lenny *const l = mapLineToLenny[idx];
		l->state = Lenny::CLOSED;
	}

	// thread finish time
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
*/

	for (std::vector<unsigned>::iterator it = v.begin(); it != v.end(); it++) {
		const unsigned idx = *it;

		Lenny *const l = mapLineToLenny[idx];
		mapLineToLenny.erase(idx);
		delete l;
	}
}


//
//
//
Lenny *getLenny(const unsigned line) {
	std::map<unsigned,Lenny*>::iterator it = mapLineToLenny.find(line);
	if (mapLineToLenny.end() == it) { LOGV("failed line:%d",line); return NULL; }
	LOGC("line:%d",line);
	return it->second;
}





// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


//
// WS API
//
void handleRequest(void *const sock,JSONItem *const json) {

	JSONItem *const act = json->get("a");
    if (NULL == act) { LOGV("missing action"); return; }

    const char *const a = act->text_value;


    //
    //
    //
    if (0 == strcmp(a,"restart")) { stopLenny(); startLenny(); return; }

    //
    if (0 == strcmp(a,"log_upload")) { sendLennyLog(); return; }

/* // TODO cannot upload rotated log files yet

    if (0 == strcmp(a,"log_rotate")) {
    	LOGC("log_rotate");
		{
			char f[PATH_MAX]; sprintf(f,"%s.old",PATH_LOG_FILE);
			rename(PATH_LOG_FILE,f);
		}
		return;
	}
*/

    //
    if (0 == strcmp(a,"update_check")) { handleUpdateCheck(sock,json); return; }

    //
    if (0 == strcmp(a,"comment")) {
    	JSONItem *const rating = json->get("r");
    	JSONItem *const comment = json->get("c");

    	if (NULL == rating  || JSON_TYPE_INTEGER != rating->type ) { LOGV("rating type is not INTEGER"); return; }
    	if (NULL == comment || JSON_TYPE_TEXT    != comment->type) { LOGV("comment type is not STRING"); return; }

    	cacheComment((unsigned)rating->int_value,comment->text_value);

    	return;
    }
    if (0 == strcmp(a,"bug")) {
    	JSONItem *const summary = json->get("s");
    	JSONItem *const comment = json->get("c");

    	if (NULL == summary || JSON_TYPE_TEXT != summary->type) { LOGV("summary type is not STRING"); return; }
    	if (NULL == comment || JSON_TYPE_TEXT != comment->type) { LOGV("comment type is not STRING"); return; }

    	cacheBugReport(summary->text_value,comment->text_value);

    	return;
    }



    //
    if (0 == strcmp(a,"signin")) { handleSignInAction(sock,json); return; }

    //
    if (0 == strcmp(a,"license")) { handleLicenseAction(sock,json); return; }

    //
    if (0 == strcmp(a,"info_get")) { sendSystemInfo(sock); return; }
    if (0 == strcmp(a,"config_get")) { sendConfig(sock); return; }
    if (0 == strcmp(a,"status_get")) { sendStatus(sock); return; }

    //
    if (0 == strcmp(a,"skiplist_get")) { sendSkiplist(sock); return; }
    if (0 == strcmp(a,"skiplist_set")) { handleSkiplistAction(sock,json); return; }

    if (0 == strcmp(a,"profiles_get")) { sendLennyProfiles(config.profiles,sock); return; }
    if (0 == strcmp(a,"profile_set")) { handleProfileAction(sock,json); return; }

    if (0 == strcmp(a,"disconnected_message")) { shareWAVFile(sock,0,PATH_DISCONNECTED); return; }

    if (0 == strcmp(a,"calls_get")) { sendCalls(sock); return; }
	if (0 == strcmp(a,"call_set")) { handleCallAction(sock,json); return; }


	//
	// get call message audio file
	//

    if (0 == strcmp(a,"call_message")) {

    	JSONItem *const m = json->get("m");
    	if (NULL == m) {
    		LOGV("message is null");
    		return;
    	}

    	char f[PATH_MAX]; sprintf(f,"%s/%s",PATH_CALLS,m->text_value);
    	LOGC("call_message f:%s",f);

    	{
			FILE *const fd = fopen(f,"rb");

			if (NULL == fd) {
				LOGV("call_message not found file:%s",f);
				return;
			}
			fclose(fd);
		}

		shareWAVFile(sock,0,f);
        return;
    }


    //
	// get profile message audio file
    //
    if (0 == strcmp(a,"profile_message")) {

	    {
	    	JSONItem *const m = json->get("m");

	    	if (NULL == m) {
	    		LOGV("message is null");
	    		return;
	    	}

	    	//
	    	//
	    	//
	    	char f[PATH_MAX];
	    	strcpy(f,(0 == strncmp("lenny",m->text_value,5) ?PATH_OPT_LENNY :PATH_VAR_LENNY));
	    	strcat(f,"/");
	    	strcat(f,m->text_value);

	    	LOGC("profile_message f:%s",f);

	    	{
				FILE *const fd = fopen(f,"rb");

				if (NULL == fd) {
					LOGV("get_msg not found file:%s",f);
				} else {
					fclose(fd);

					//
					// valid file checked!
					//
					shareWAVFile(sock,0,f);
				}
			}
		}

        return;
    }

    //
	// save profile message audio file
    //
    if (0 == strcmp(a,"profile_message_set")) {

    	JSONItem *const name  = json->get("n");

    	if (NULL == name) {
    		LOGV("name is null");
    		return;
    	}

    	JSONItem *const idx  = json->get("i");

    	if (NULL == idx) {
    		LOGV("idx is null");
    		return;
    	}

		// copy lowercase
    	char p[32] = {0};
    	{
    		const unsigned max = (unsigned)strlen(name->text_value);
    		for (unsigned i=0;i<max;i++) { p[i] = (char)tolower(name->text_value[i]); }
    	}

    	sprintf(openFile,"%s/%s_%02d.wav",(0 == strcmp("lenny",p) ?PATH_OPT_LENNY :PATH_VAR_LENNY),p,(unsigned)idx->int_value);
    	LOGC("profile_message_set f:%s",openFile);

		return;
    }


    //
    //
    //
    if (0 == strcmp(a,"config_set")) {
    	JSONItem *const k = json->get("key");
    	JSONItem *const v = json->get("value");

    	if (NULL == k) { LOGV("error key is NULL"); return; }
    	if (NULL == v) { LOGV("error value is NULL"); return; }


    	if (0 == strcmp("user_security_enabled",k->text_value)) {
			if (JSON_TYPE_BOOL != v->type) { LOGV("value type is not BOOL"); return; }

	    	config.userSecurity = v->int_value;
			config.save();

	    	//
	    	// send config to everyone
	    	//
	    	sendConfig();

    		return;
    	}

    	if (0 == strcmp("log_verbosity",k->text_value)) {
			if (JSON_TYPE_INTEGER != v->type) { LOGV("value type is not INTEGER"); return; }

			LOGV("log_verbosity from:%d",verbosity);
	    	verbosity = (unsigned)v->int_value;
			LOGV("log_verbosity to:%d",verbosity);

    		return;
    	}

/*
    	if (0 == strcmp("lenny_profile_add",k->text_value)) {
			if (JSON_TYPE_TEXT != v->type) { LOGV("value type is not STRING"); return; }

	    	l.configLine->lennyProfile = v->text_value;
			config.save();

	    	//
	    	// send config to everyone
	    	//
	    	sendConfig();

    		return;
    	}
    	if (0 == strcmp("lenny_profile_remove",k->text_value)) {
			if (JSON_TYPE_TEXT != v->type) { LOGV("value type is not STRING"); return; }

	    	l.configLine->lennyProfile = v->text_value;
			config.save();

	    	//
	    	// send config to everyone
	    	//
	    	sendConfig();

    		return;
    	}
*/

	}

    //
    // Handle line actions
    //
	handleLineAction(sock,json);
}


//
//
//
void handleUpdateCheck(void *sock,JSONItem *json) {
	(void)sock; (void)json;

	static time_t timeLastCheck = 0;
	const time_t last = timeLastCheck;
	timeLastCheck = time(NULL);

	config.svCheckLast = timeLastCheck;

	// only one every 24 hours
	if (0 < last && 24 > (timeLastCheck - last) /60) {
		//LOGC("appGetServerVersion not yet elapsed:%ds",timeLastCheck - last);
	} else

	if (!appGetServerVersion()) {
		LOGV("appGetServerVersion failed");

		//
		// service is down?
		//
		timeLastCheck = 0;
	}

	config.save();
	sendConfig();

/*
	if (config.sv->requiresUpdate()) {
		//
		//
		//
		return;
	}

	if (config.sv->hasUpdate()) {
		//
		//
		//
		return;
	}
*/

}


//
//
//
void handleLicenseAction(void *sock,JSONItem *json) {
	(void)sock; (void)json;

	JSONItem *const type = json->get("t");  // email,save,create

	if (NULL == type) {
		LOGV("update is null");
		return;
	}

	if (0 == strcmp("email",type->text_value)) {
		LOGC("email");
    	JSONItem *const email = json->get("v");

    	if (NULL == email) {
    		LOGV("value is null");
    		return;
    	}

    	if (0 == config.email.length()) {
    		// new email
    		config.email = email->text_value;
			config.save();
		    sendConfig();

			LOGC("license new email: %s",config.license.c_str());

    	} else
    	if (0 != config.email.compare(email->text_value)) {
    		// email changed
    		config.email = email->text_value;
			config.save();

		    //
		    // change invalidates current license key 
		    //

			if (0 < config.license.size()) {
				LOGC("license changed email: %s",config.license.c_str());

			//	config.license = "";

			}

		    sendConfig();

    	} else {
    		// no change
    	}

      	return;
	}


	if (0 == strcmp("save",type->text_value)) {
		LOGC("save");
    	JSONItem *const lic = json->get("v");

    	if (NULL == lic) {
    		LOGV("value is null");
    		return;
    	}

		extern void appUpdate();

    	if (0 == config.license.length()) {
    		LOGC("new license");
    		config.license = lic->text_value;
			config.save();
		    appUpdate();

		    sendConfig();

    	} else
    	if (0 != config.license.compare(lic->text_value)) {
    		LOGC("license changed");
    		config.license = lic->text_value;
			config.save();
		    appUpdate();

		    sendConfig();
    	} else {
    		//
    		// no change
    		//
    	}

      	return;
	}

	if (0 == strcmp("create",type->text_value)) {
		LOGC("create");


		//
		//
		//
    	if (!appGetServerVersion()) {
    		LOGV("appGetServerVersion failed");

    		//
    		// service is down?
    		//
    		return;
    	}

    	if (config.sv->requiresUpdate()) {
			sendConfig();
    		return;
    	}


    	//
    	//
    	//
		#define DEFAULT_EVAL_DAYS 30

    	JSONItem *const d = json->get("d");
    	const unsigned days = (unsigned)(NULL == d || JSON_TYPE_INTEGER != d->type ?DEFAULT_EVAL_DAYS :d->int_value);

    	JSONItem *const e = json->get("e");
    	const bool eval = NULL == e || JSON_TYPE_BOOL != e->type ?true :e->int_value;

		if (!appCreateLicense(days,eval)) {
			LOGV("appCreateLicense failed days:%d eval:%s",days,eval?"true":"false");
			return;
		}

		return;
	}

	LOGV("not handled skiplist_set type:%s",type->text_value);
}

void handleSignInAction(void *sock,JSONItem *json) {

	std::ostringstream ss; ss << "{\"signin\":{";

	JSONItem *const n = json->get("n");
	JSONItem *const p = json->get("p");

	if (NULL == n) {
		ss << "\"err\":-10,\"err_msg\":\"name is null\"";
	} else
	if (0 == strlen(n->text_value)) {
		ss << "\"err\":-11,\"err_msg\":\"name is empty\"";
	} else
	if (NULL == p) {
		ss << "\"err\":-20,\"err_msg\":\"password is null\"";
	} else
	if (0 == strlen(p->text_value)) {
		ss << "\"err\":-21,\"err_msg\":\"password is empty\"";
	} else
	if (!signinValidate(n->text_value,p->text_value,ss)) {
		LOGV("failed");
	} else {
		LOGC("success");
	}

	ss << "}}";

	wsSendText(sock,ss.str().c_str());
}

void handleSkiplistAction(void *sock,JSONItem *json) {
	(void)sock;

	JSONItem *const type = json->get("t");  // add,del,active

	if (NULL == type) {
		LOGV("type is null");
		return;
	}

	if (0 == strcmp("active",type->text_value)) {
		LOGV("");
    	JSONItem *const value = json->get("value");

    	if (NULL == value) {
    		LOGV("value is null");
    		return;
    	}

    	config.skiplistEnabled = value->int_value;
		config.save();
	    sendConfig();
      	return;
	}

	//
	//
	//
	JSONItem *const num  = json->get("number");

	if (NULL == num) {
		LOGV("number is null");
		return;
	}

	if (0 == strcmp("add",type->text_value) || 0 == strcmp("update",type->text_value)) {
    	JSONItem *const name = json->get("name");
    	JSONItem *const note = json->get("note");

    	if (NULL == name) {
    		LOGV("name is null");
    		return;
    	}

		LOGV("skiplist add/update name:%s num:%s note:%s",name->text_value,num->text_value,NULL == note ?"" :note->text_value);

		addSkiplist(name->text_value,num->text_value,NULL == note ?"" :note->text_value);
	    sendSkiplist(NULL);
      	return;
	}

	if (0 == strcmp("del",type->text_value)) {
		LOGC("skiplist prune num:%s",num->text_value);
		pruneSkiplist(num->text_value);
	    sendSkiplist(NULL);
      	return;
	}

	LOGV("not handled skiplist_set type:%s",type->text_value);
}

void handleProfileAction(void *sock,JSONItem *json) {
	(void)sock;

	JSONItem *const type = json->get("t");  // add,del

	if (NULL == type) {
		LOGV("type is null");
		return;
	}

	//
	//
	//
	JSONItem *const name  = json->get("name");

	if (NULL == name) {
		LOGV("name is null");
		return;
	}

	if (0 == strcmp("add",type->text_value)) {

		LOGC("profile add name:%s",name->text_value);
		addLennyProfile(config.profiles,name->text_value);
		config.save();
	    sendConfig();
      	return;
	}

	if (0 == strcmp("del",type->text_value)) {

    	if (0 == strcasecmp("lenny",name->text_value)) {
    		LOGV("del lenny not available");
    		return;
    	}

    	JSONItem *const idx  = json->get("i");

    	if (NULL == idx) {
    		LOGC("profile prune name:%s",name->text_value);
			removeLennyProfile(config.profiles,name->text_value);
			config.save();
		    sendConfig();
	      	return;
	    }

		// profile name: copy lowercase
    	char p[32];
    	{
    		const unsigned max = (unsigned)strlen(name->text_value);
    		for (unsigned i=0;i<max;i++) { p[i] = (char)tolower(name->text_value[i]); }
    	}

    	char f[PATH_MAX]; sprintf(f,"%s/%s_%02d.wav",PATH_VAR_LENNY,p,(unsigned)idx->int_value);
		LOGC("unlink profile file:%s",f);

		if (0 != unlink(f)) {
			LOGV("unlink failed '%s' errno:%d",f,errno);
		}
		return;
	}

	LOGV("not handled skiplist_set type :%s",type->text_value);
}

void handleCallAction(void *const sock,JSONItem *const json) {
	(void)sock;

	JSONItem *const type = json->get("t");  // viewed,skiplist,history,delete
	if (NULL == type) {
		LOGV("type not found");
		return;
	}

	JSONItem *const msg = json->get("m");
	if (NULL == msg) {
		LOGV("message not found");
		return;
	}

	if (JSON_TYPE_ARRAY == msg->type) {

    	for (JSONItem *it = msg->child;NULL != it;it = it->next) {
	    	JSONItem *const date = it->get("date");
	    	JSONItem *const time = it->get("time");
	    	JSONItem *const line = it->get("line");

	    	if (NULL == date) { LOGV("date is null"); continue; }
	    	if (NULL == time) { LOGV("time is null"); continue; }
	    	if (NULL == line) { LOGV("line is null"); continue; }


	    	//
	    	//
	    	//
/*
	    	if (0 == strcmp("skiplist",type->text_value)) {
	    		//
	    		// TODO add to skiplist
	    		//

	    		// update new
	    		pruneNewCalls(line->text_value,date->text_value,time->text_value);
	    		continue;
	    	}
*/

	    	//
	    	//
	    	//

	    	if (0 == strcmp("viewed",type->text_value)) {
	    		// prune new
	    		pruneNewCalls(line->text_value,date->text_value,time->text_value);
	    		continue;
	    	}
	    	if (0 == strcmp("history",type->text_value)) {
	    		// calls => history
	    		// prune calls
	    	//	pruneCalls(line->text_value,date->text_value,time->text_value);
	    		addHistory(it,line->text_value,date->text_value,time->text_value);
	    		continue;
			}
	    	if (0 == strcmp("del",type->text_value)) {
	    		// prune history
	    		pruneHistory(line->text_value,date->text_value,time->text_value);

	    		{
			    	JSONItem *const file = it->get("file");
			    	if (NULL != file) {
			    		pruneMessage(file->text_value);
			    	} else {
			    	//	char f[PATH_MAX]; sprintf(f,"%s%s00_%s.wav",date->text_value,time->text_value,line->text_value);
			    	//	pruneMessage(f);
			    	}
			    }
	    		continue;
	    	}

	    	LOGV("call_set not handled invalid type:%s",type->text_value);
	    	return;
		}

		sendCalls();
    	return;
	}

	LOGV("call_set not handled object:%d for action type:%s",msg->type,type->text_value);
}

void handleLineAction(void *const sock,JSONItem *const json) {

    JSONItem *const ll = json->get("l");

    if (NULL == ll || JSON_TYPE_INTEGER != ll->type) {
        LOGV("line value missing");
        return;
    }

    const char *const a = json->get("a")->text_value;

    if (NULL == a) {
		LOGV("action not found");
		return;
    }

	const unsigned line = (unsigned)ll->int_value;

	if (1 != line && 2 != line) {
		LOGV("invalid line:%d",line);
		return;
	}

	//
	Lenny *const tl = getLenny(line);

	if (NULL == tl) {
		LOGV("not found line:%d",line);
		return;
	}


	//
	//
	//
	{
	    Lenny &l = *tl;


	    //
	    // action
	    //
	    if (0 == strcmp(a,"onhook")) {
	    	if (!l.answerModeActive) return;
			l.answerModeActive = false;

			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			l.queue.post(&l.onHook,500);

			LOGC("schedule waitForCall");
			l.queue.post(&l.waitForCall,500);

	        return;
	    }


	    //
	    // deprecated ???
	    //

	    if (0 == strcmp(a,"offhook")) {
	    	if (l.answerModeActive) return;

			//postMainThread(&l.offHook);
			l.queue.post(&l.offHook);

			std::this_thread::sleep_for(std::chrono::milliseconds(500));
	        return;
	    }

	    if (0 == strcmp(a,"lenny")) {
			l.queue.post(&l.answerWithLenny);
	        return;
	    }

	    if (0 == strcmp(a,"disconnected")) {
			l.queue.post(&l.answerWithDisconnected);
	        return;
	    }

	    if (0 == strcmp(a,"tad")) {
			l.queue.post(&l.answerWithMessage);
	        return;
	    }



		//
		//
		//
		if (0 == strcmp(a,"listen_on" )) { l.listenAdd(sock); return; }
		if (0 == strcmp(a,"listen_off")) { l.listenRemove(sock); return; }



	    //
	    //
	    //
	    if (0 == strcmp(a,"record_on")) {
	    	LOGC("record ON");

			if (Lenny::ONHOOK != l.state) {
				LOGV("already OFFHOOK");
				return;
			}

			{
				const time_t now = time(0);
				struct tm *const ltm = localtime(&now);
				snprintf(l.outFile,sizeof(l.outFile),"%s/%d%02d%02d%02d%02d%02d_%d.wav",PATH_CALLS,1900+ltm->tm_year,1+ltm->tm_mon,ltm->tm_mday,ltm->tm_hour,ltm->tm_min,0/*ltm->tm_sec*/,l.configLine->line);
				LOGV("DEFAULT: outFile:%s",l.outFile);
			}

			l.queue.post(&l.answerWithRecord);
	        return;
	    }
	    if (0 == strcmp(a,"record_off")) {
	    	LOGC("record OFF");
	    	l.answerModeActive = false;
	        return;
	    }



		//
		// get tad audio file
		//

	    if (0 == strcmp(a,"tad_message")) {
	      	//LOGC("tad_message");

	    	char f[PATH_MAX]; sprintf(f,PATH_TAD_OGM_FMT,l.configLine->line);
	    	LOGC("tad_message f:%s",f);

	    	//
	    	// is valid file
	    	//
			FILE *const fd = fopen(f,"rb");
			if (NULL != fd) {
				fclose(fd);
				shareWAVFile(sock,l.configLine->line,f);
			} else {
				shareWAVFile(sock,l.configLine->line,PATH_TAD_DEFAULT);
			}

	        return;
	    }

		//
		// save / delete tad audio file
		//

	    if (0 == strcmp(a,"tad_message_set")) {
	    	JSONItem *const type = json->get("t");

	    	if (NULL == type) {
	    		LOGV("type is null");
	    		return;
	    	}
		    if (0 == strcmp(type->text_value,"add")) {
		    	sprintf(openFile,PATH_TAD_OGM_FMT,l.configLine->line);

		    	LOGC("tad_message_set add %s",openFile);

			    sendConfig();
			    return;
		    }
		    if (0 == strcmp(type->text_value,"del")) {
		    	char f[PATH_MAX]; sprintf(f,PATH_TAD_OGM_FMT,l.configLine->line);

		    	LOGC("tad_message_set del %s",f);

			    if (0 != unlink(f)) {
			    	LOGV("unlink failed %s errno:%d",f,errno);
			    }

			    sendConfig();
				return;
		    }

	    	LOGV("tad_message_set not handled type:%s",type->text_value);
			return;
	    }



	    //
	    //
	    //

	    if (0 == strcmp(a,"config_set")) {
	    	JSONItem *const k = json->get("key");
	    	JSONItem *const v = json->get("value");

	    	if (NULL == k            ) { LOGV("error key is NULL"); return; }
	    	if (NULL == k->text_value) { LOGV("error key text_value is NULL"); return; }
	    	if (NULL == v            ) { LOGV("error value is NULL"); return; }

	    	if (0 == strcmp("lenny_enabled",k->text_value)) {
				if (JSON_TYPE_BOOL != v->type) { LOGV("value type is not BOOL"); return; }

		    	l.configLine->lennyEnabled = v->int_value;
				config.save();

		    	//
		    	// send config to everyone
		    	//
		    	sendConfig();

	    		return;
	    	}
	    	if (0 == strcmp("disconnected_enabled",k->text_value)) {
				if (JSON_TYPE_BOOL != v->type) { LOGV("value type is not BOOL"); return; }

		    	l.configLine->disconnectedEnabled = v->int_value;
				config.save();

		    	//
		    	// send config to everyone
		    	//
		    	sendConfig();

	    		return;
	    	}
	    	if (0 == strcmp("tad_enabled",k->text_value)) {
				if (JSON_TYPE_BOOL != v->type) { LOGV("value type is not BOOL"); return; }

		    	l.configLine->tadEnabled = v->int_value;
				config.save();

		    	//
		    	// send config to everyone
		    	//
		    	sendConfig();

	    		return;
	    	}
	    	if (0 == strcmp("tad_rings",k->text_value)) {
				if (JSON_TYPE_INTEGER != v->type) { LOGV("value type is not INTEGER"); return; }

		    	l.configLine->tadRings = (unsigned)v->int_value;
				config.save();

		    	//
		    	// send config to everyone
		    	//
		    	sendConfig();

	    		return;
	    	}
	    	if (0 == strcmp("lenny_profile_set",k->text_value)) {
				if (JSON_TYPE_TEXT != v->type) { LOGV("value type is not STRING"); return; }

		    	l.configLine->lennyProfile = v->text_value;
				config.save();

		    	//
		    	// send config to everyone
		    	//
		    	sendConfig();

	    		return;
	    	}

	    	LOGV("config_set not handled key:%s",k->text_value);
	        return;
	    }
	}

    LOGW("not handled action:%s",a);
}


#include <time.h> //strftime

//
//
//

std::string getConfig() {

	std::ostringstream ss;
	ss << "{";

		ss << "\"config\":{";
			ss << "\"version\":\"" <<getAppVersion() <<"\"";

			if (config.sv) {
				ss << ",\"server_version\":\"" <<config.sv->toString() <<"\"";
			}

			if (0 < config.svCheckLast) {
				char t[32]; strftime(t,sizeof(t),"%F %T",localtime(&config.svCheckLast));
				ss << ",\"server_check_last\":\"" <<t <<"\"";
			}

			ss << ",\"http_port\":" <<config.httpPort;

			extern std::string webCertJsonInfo();
			ss << ",\"certificate\":" <<webCertJsonInfo();

			ss << ",\"user_security\":" <<(config.userSecurity ?"true" :"false");

			ss << ",\"skiplist_enabled\":" <<(config.skiplistEnabled ?"true" :"false");

			ss << ",\"log_verbosity\":" <<verbosity;

			ss << ",\"silence_volume_threshold\":"   <<config.silenceVolumeThreshold;
			ss << ",\"silence_switch_time\":"        <<config.silenceSwitchTime;
			ss << ",\"end_call_silence_time\":"      <<config.endCallSilenceTime;
			ss << ",\"end_call_busy_signal_count\":" <<config.endCallBusyCount;
			ss << ",\"end_call_busy_signal_time\":"  <<config.endCallBusyTime;


		//
		//
		//
		#ifndef NO_LICENSE

			ss << ",\"email\":\"" <<config.email <<"\"";

			ss << ",\"license\":{";
				ss << "\"str\":\"" <<config.license <<"\"";
				ss <<",";

			//	getAppLicenseInfo(config.license.c_str(),s);
				{
					time_t expiration;
					LicenseFlags flags;

					const int r = appGetLicense(config.license.c_str(),expiration,flags);

					if (LICENSE_INVALID == r) {
						ss << "\"err\":-2,\"err_msg\":\"invalid\"";
					} else {

						char date[20];
						formatDays((unsigned)(expiration / SECONDS_IN_DAY),date,sizeof(date));

						const bool expired = LICENSE_EXPIRED == r;

						if (expired) {
							ss << "\"err\":-1,\"err_msg\":\"expired\"";
						} else {
							// LICENSE_VERIFIED == r

							ss << "\"err\":0";
						}

						ss << ",\"date\":\""  << date << "\"";
						ss << ",\"version\":" << flags.ver;
						ss << ",\"eval\":"    << (flags.eval ?"true":"false");
						ss <<" ,\"flags\":"   << flags.flags;
					}
				}
			ss << "}";
		#endif //NO_LICENSE

		ss << "}";

		//
		ss << ",\"profiles\":[";
		    for (unsigned i=0; i<config.profiles.size();i++) {
		    	if (0 < i) ss <<",";
				ss <<"\"" <<config.profiles[i] <<"\"";
			}
		ss << "]";

		//
		ss << ",\"lines\":{";


		{
			unsigned c =0;
			for (std::map<unsigned,Lenny*>::iterator it = mapLineToLenny.begin(); it != mapLineToLenny.end(); it++) {
				Lenny &l = *it->second;
				if (NULL == l.configLine || 0 == l.configLine->line) {
					// not active
				} else {
					if (0 < c++) ss << ",";
					ss <<"\"" << l.configLine->line << "\":" << getConfig(l);
				}
			}
		}
		ss << "}";

	ss << "}";

	return ss.str();
}

std::string getStatus() {
	std::ostringstream ss;

	ss << "{";
	ss <<"\"status\":[";

	{
		unsigned c =0;
		for (std::map<unsigned,Lenny*>::iterator it = mapLineToLenny.begin(); it != mapLineToLenny.end(); it++) {
			Lenny &l = *it->second;
			if (0 < c++) ss << ",";
			ss << getStatus(l);
		}
	}

	ss <<"]";
	ss <<"}";

	return ss.str();
}



//
//
//
void sendSystemInfo(void *const sock) {
	LOGC("sendStatus");

	std::ostringstream ss;
	ss << "{\"info\":{";	
	ss << getSystemInfo();
	ss << "}}";

	if (NULL == sock) {
		wsSendAll(ss.str().c_str());
	} else {
		wsSendText(sock,ss.str().c_str());
	}
}

void sendConfig(void *const sock) {
	LOGC("sendStatus");

	std::string str = getConfig();

	if (NULL == sock) {
		wsSendAll(str.c_str());
	} else {
		wsSendText(sock,str.c_str());
	}
}

void sendStatus(void *const sock) {
	LOGC("sendStatus");
	std::string str = getStatus();

	if (NULL == sock) {
		wsSendAll(str.c_str());
	} else {
		wsSendText(sock,str.c_str());
	}
}

std::string getConfig(Lenny &l) {
	std::vector<std::string> modems = enumUSBModemDevices(true);

	std::ostringstream ss;
	if (NULL == l.configLine || 0 == l.configLine->line) return ss.str();

	ss << "{";
		ss << "\"name\":\""   << l.configLine->name   <<"\"" <<",";
		ss << "\"number\":\"" << l.configLine->number <<"\"" <<",";
		ss << "\"device\":\"" << l.configLine->device <<"\"" <<",";

		if (modems.size() >= l.configLine->line) {
			ss << "\"modem\":" << modems[l.configLine->line -1] <<",";
		} else {

//
// TODO what if none found
//

		}

		ss << "\"status\":{";
			ss << "\"totals\":{";
				ss << "\"bytes_out\":" << l.modem.totalBytesOut      <<",";
				ss << "\"bytes_in\":"  << l.modem.totalBytesIn       <<",";
				ss << "\"calls\":"     << l.modem.totalCalls         <<",";
				ss << "\"rings\":"     << l.modem.totalCallRings     <<",";
				ss << "\"answered\":"  << l.modem.totalCallsAnswered <<",";
				ss << "\"seconds\":"   << l.modem.totalCallSeconds;
			ss <<"}";
		ss <<"},";

		ss << "\"lenny_enabled\":"        << (l.configLine->lennyEnabled        ?"true":"false") <<",";
		ss << "\"disconnected_enabled\":" << (l.configLine->disconnectedEnabled ?"true":"false") <<",";
		ss << "\"tad_enabled\":"          << (l.configLine->tadEnabled          ?"true":"false") <<",";

		ss << "\"lenny_profile\":\""      << l.configLine->lennyProfile <<"\"" <<",";
		ss << "\"tad_rings\":"            << l.configLine->tadRings     <<",";

    	char f[PATH_MAX]; sprintf(f,PATH_TAD_OGM_FMT,l.configLine->line);
		ss <<"\"tad_ogm_default\":" <<(fileExists(f) ?"false":"true");

	ss << "}";

	return ss.str();
}

std::string getStatus(Lenny &l) {
	std::ostringstream ss;

	if (NULL == l.configLine || 0 == l.configLine->line) return ss.str();

	ss <<"{";

	ss <<"\"line\":" << l.configLine->line;
	ss <<",";
	ss <<"\"name\":\"" << l.configLine->name <<"\"";
	ss <<",";

	//
	ss <<"\"cid\":{";
		ss <<"\"na\":" <<"\"" <<l.modem.lastCallerName <<"\"";
		ss <<",";
		ss <<"\"nu\":" <<"\"" <<l.modem.lastCallerNumber <<"\"";
	ss <<"}";

	ss <<",";

	static const char *const STATES[] = { "CLOSED","ONHOOK","RING","OFFHOOK" };
	static const char *const MODES[] = { "READY","LENNY","DISCONNECTED","MESSAGE","RECORD" };

	ss <<"\"status\":\"" <<STATES[l.state] <<"\"";
	ss <<",";
	ss <<"\"mode\":\"" <<MODES[l.mode] <<"\"";

	ss <<"}";

	return ss.str();
}





// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

//
// WS Events
//

void wsSendAll(const char *const data) {
	//LOGD("sendAll %s",data);
    for (unsigned i=0; i<listSockets.size();i++) {
        wsSendText(listSockets[i],data);
    }
}

void WSCallback::onOpen(void *const sock) {
    LOGC("onOpen");

#ifdef DEBUG
    for (unsigned i=0;i<listSockets.size();i++) {
        if (sock != listSockets[i]) continue;
	    LOGV("onOpen sock found");
        return;
    }
#endif //DEBUG

    listSockets.push_back(sock);
}

void WSCallback::onClose(void *const sock) {
    LOGC("onClose");

	for (std::map<unsigned,Lenny*>::iterator it = mapLineToLenny.begin(); it != mapLineToLenny.end(); it++) {
		Lenny &l = *it->second;
		l.listenRemove(sock);
	}

    for (unsigned i=0;i<listSockets.size();i++) {
        if (sock != listSockets[i]) continue;
        listSockets.erase(listSockets.begin()+i);
        return;
    }

    LOGV("onClose sock not found listSockets size:%d",listSockets.size());
}

void WSCallback::onBinary(void *const sock,const unsigned length,const uint8_t *const data) {
    LOGD("onBinary l:%d",length);
    (void)sock;

	FILE *const fd = fopen(openFile,"wb");

	if (NULL == fd) {
		LOGV("fopen failed: %s",openFile);
	} else {
		fwrite(data,length,1,fd);
		fclose(fd);
	}

	openFile[0] = '\0';

    if (data) free((char*)data);
}

void WSCallback::onText(void *const sock,const unsigned length,const char *const data) {
    LOGD("onText d:%s",data);
    (void)length;

	JSONItem *const json = JSONItem::parse((char *)data);

	if (NULL == json) {
		LOGV("json_parse failed");
	} else {
        handleRequest(sock,json);
		JSONItem::free(json);
    }

    if (data) free((char*)data);
}

