/*


*/
'use strict';

console.log('included test');


//
test = () => { console.log('start test'); }

//
//
//
var logDiv = null;

function log(msg) {
	logDiv.innerText += msg+'\n';
	logDiv.scrollTop = logDiv.scrollHeight;	
}


//
//
//
document.addEventListener('DOMContentLoaded',() => {

	console.log('included test running');

	//
	logDiv = document.createElement('div');
	logDiv.style = 'width:85%;height:'+((window.innerHeight -80)+'px')+';margin:20px;padding:20px;border:2px solid #888;background-color:#000;color:#4f4;font-family:monospace;font-size:10px;overflow-y:scroll;';
	document.body.appendChild(logDiv);
	logDiv.innerText += 'Lenny Tester ready...\n\n';


	//
	//
	//

});


class Test_Lenny {

	constructor() {
		this.eventError        = (type,ev) => { console.log('error '+type+' '+ev); };
		this.eventDisconnected = (       ) => { console.log('diconnected'); };
		this.eventConnected    = (       ) => { console.log('connected'); };

	}


	handlers() {

		this.binaryCallback    = (buf    ) => { lenny.binaryCallback(buf); };

		this.eventError        = (type,ev) => { lenny.eventError(type,ev); };
		this.eventConnected    = (       ) => { lenny.eventConnected(); };
		this.eventDisconnected = (       ) => { lenny.eventDisconnected(); };

		this.eventInfo         = (info   ) => { lenny.eventInfo(info); };
		this.eventSignIn       = (acct   ) => { lenny.eventSignIn(acct); };
		this.eventConfig       = (cfg    ) => { lenny.eventConfig(cfg); };
		this.eventStatus       = (status ) => { lenny.eventStatus(status); };
		this.eventSkiplist     = (list   ) => { lenny.eventSkiplist(list); };
		this.eventProfiles     = (pr     ) => { lenny.eventProfiles(pr); };
		this.eventCalls        = (calls  ) => { lenny.eventCalls(calls); };

	}

	test() {

		new Popup().showFirstTime(() => { });
		new Popup().showLoading();

		new Popup().showSendComment();
		new Popup().showSendBugReport();

		new Popup().showAppUpdate(cfg.config,true);

		new Popup().showDisconnected();
		new Popup().showSignIn();

		new Popup().showLicenseEmail(this.cfg.config);
		new Popup().showLicenseUpdate(cfg.config);
		new Popup().showLineConfig( { idx:0,cfg:{device:'',nam:'',num:''} } );
		new Popup().showCallMessage( { c:calls,i:cidx } );

		new Popup().showLineConfig(this);

		new Popup().showPlayDisconnected();
		new Popup().showOGM( this );


		new Popup().showSkiplist(this.skiplist); 
		new Popup().showSkiplistItem( {n:[{name:'',number:'',note:''}],i:0,new:true} );
		new Popup().showSkiplistItem( {n:list,i:i} );


		new Popup().showLennyProfile(this);
		new Popup().showLennyNewProfile(line,(reload) => { if (reload) load(); });
		new Popup().showLennyProfileEdit(v);


	}

}

