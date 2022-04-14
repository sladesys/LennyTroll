/*


*/
'use strict';


var supportsPassive = false;
var lenny = null,ws = null;
var isMobile = false;


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

	//
	logDiv = document.createElement('div');
	logDiv.style = 'width:85%;height:'+((window.innerHeight -80)+'px')+';margin:20px;padding:20px;border:2px solid #888;background-color:#000;color:#4f4;font-family:monospace;font-size:10px;overflow-y:scroll;';
	document.body.appendChild(logDiv);
	logDiv.innerText += 'Lenny Tester ready...\n\n';

	//
	isMobile = (/iPhone|iPod|iPad|Android|BlackBerry/).test(navigator.userAgent);

	try {
		//
		// Test via a getter in the options object to see if the passive property is accessed
		//  https://github.com/WICG/EventListenerOptions/blob/gh-pages/explainer.md
		//
		const opts = Object.defineProperty({},'passive',{
			get:() => {
				//console.log('supportsPassive');
				supportsPassive = { passive:true };
			}
		});
		window.addEventListener(   'testPassive',null,opts);
		window.removeEventListener('testPassive',null,opts);
	} catch (err) { console.log('supportsPassive err:'+err); }


	ws = new WS();
	lenny = new Lenny();





	{
		let i = 1;

		let restart = null;
		const start = () => {
			log('  '+i+': START');

//			lenny.test_Connect( () => { log('  '+i+': FINISHED'); restart(); } );
			lenny.test_GetAllInfo( () => { log('  '+i+': FINISHED'); restart(); } );

		};
		restart = () => {
			if (++i > 300) {
				log('\nTEST SUITE finished');
				return;
			}
			setTimeout(() => { start(); },100);
		};

		log('TEST SUITE starting\n');
		start();
	}


/*
//	lenny.testIframe();


	lenny.testReConnect(1);
	lenny.testReConnect_GetAllInfo(1);

	lenny.test_GetAllInfo();

	lenny.testGetLicense();
	lenny.testSignIn();

	lenny.testGetAllMessages();
	lenny.testGetAllCalls();

	lenny.test_LennyEnabled_Toggle();
	lenny.test_DisconnectedEnabled_Toggle();
	lenny.test_MessagesEnabled_Toggle();

	lenny.test_OGM();
	lenny.test_Disconnectd();
	lenny.test_LennyProfile();
	lenny.test_Profile_AddRemove();

	lenny.test_Skiplist_AddRemove();

	lenny.test_ReadNewMessage();
	lenny.test_MoveAMessage();
// */

});


class Lenny {

	constructor() {
		this.eventError = (type,ev) => { console.log('error '+type+' '+ev); };
		this.eventDisconnected = (ev) => { console.log('diconnected'); };
		this.eventConnected = (ev) => { console.log('connected'); };
	}


/*
	testIframe() {
		const iframe = document.createElement('iframe');
		iframe.src = 'index.html';
		iframe.onload = (ev) => { console.log('iframe onload'); };
		iframe.onerror = (ev) => { console.log('iframe onerror'); };
		document.body.appendChild(iframe);
	}
// */

	testActions() {

		ws.sendAction(1,'listen_on');
		ws.sendAction(1,'listen_off');
		ws.sendAction(1,'record_on');
		ws.sendAction(1,'record_off');

		ws.sendAction(1,'offhook');
		ws.sendAction(1,'onhook');
		ws.sendAction(1,'lenny');
		ws.sendAction(1,'disconnected');
		ws.sendAction(1,'messages');
		ws.sendAction(1,'ignore');	
	}



	//
	//
	//

/*
	testReConnect(count) {
		if (0 >= count) {
			console.log('testReConnect finished');
			return;
		}

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				console.log('testConnect - close');

				this.eventDisconnected = (ev) => {
					console.log('eventDisconnected - test success - disconnect');
				};

				if (ws.websocket && WebSocket.OPEN == ws.websocket.readyState) {
					ws.websocket.close(1000);
				}

				setTimeout( (ev) => {
					this.testReConnect(count-1);
				},100);

			},100);
		};

		console.log('testReConnect count:'+count);
		ws.connect();
	}
*/


/*
	testReConnect_GetAllInfo(count) {
		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				setTimeout( (ev) => {
					console.log('testConnect - close');

					this.eventDisconnected = (ev) => {
						console.log('eventDisconnected - test success - disconnect');
					};

					if (ws.websocket && WebSocket.OPEN == ws.websocket.readyState) {
						ws.websocket.close();
					}

					setTimeout( (ev) => {
						this.testReConnect_GetAllInfo(count-1);
					},100);

				},1000);

			},100);
		};

		console.log('testReConnect count:'+count);
		ws.connect();
	}
// */









	//
	//
	//
	test_Connect(finished) {

		this.eventError = (type,ev) => {
			console.log('error '+type+' '+ev);
			finished();
		};

		this.eventDisconnected = (ev) => {
			console.log('fin');
			finished();
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => { ws.websocket.close(); },10);
		};

		ws.connect();
	}

	test_GetAllInfo(finished) {

		this.eventError = (type,ev) => {
			console.log('error '+type+' '+ev);
			finished();
		};

		this.eventDisconnected = (ev) => {
			console.log('fin');
			finished();
		};

		const getInfo = (cb) => {
			this.eventInfo = (inf) => {
				console.log('eventInfo '+JSON.stringify(inf));
				cb();
			};
			console.log('sendGetSystemInfo');
			ws.sendGetSystemInfo();
		};

		const getConfig = (cb) => {
			this.eventConfig = (cfg) => {
				console.log('eventConfig '+JSON.stringify(cfg));
				cb();
			};
			console.log('sendGetConfig');
			ws.sendGetConfig();
		};

		const getStatus = (cb) => {
			this.eventStatus = (sts) => {
				console.log('eventStatus '+JSON.stringify(sts));
				cb();
			};
			console.log('sendGetStatus');
			ws.sendGetStatus();
		};


		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {

				getInfo(() => {
				getConfig(() => {
				getStatus(() => {
					setTimeout( (ev) => { ws.websocket.close(); },100);
				})
				})
				});
			},100);
		};

		console.log('test_GetAllInfo');
		ws.connect();
	}



	//
	testGetLicense() {

		this.eventDisconnected = (ev) => { console.log('eventDisconnected - test failed'); };

		const createLicense = (cb) => {

			this.eventConfig = (cfg) => {
				console.log('eventConfig :'+JSON.stringify(cfg));

				if (!cfg.config.license || !cfg.config.license.str || 0 == cfg.config.license.str.length) {
					consoel.log('license failed');
					return;
				}

				cb();
			};

			ws.sendCreateLicense();
		};

		const getLicense = (cb) => {

			this.eventConfig = (cfg) => {
				console.log('eventConfig :'+JSON.stringify(cfg));

				if (cfg.config.license && cfg.config.license.str && 0 < cfg.config.license.str.length) {
					console.log('license already set, test stalled');
					return;
				}

				cb();
			};


			ws.sendGetConfig();
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				getLicense(() => {
				createLicense(() => {
					console.log('fin');
				})
				})
			},100);
		};

		console.log('testGetLicense');
		ws.connect();
	}


	//
	testSignIn() {

		this.eventDisconnected = (ev) => { console.log('eventDisconnected - test failed'); };

		const signIn = (cb) => {

			this.eventSignIn = (acct) => {
				console.log('eventSignIn '+JSON.stringify(acct));

				if (0 != acct.err) {
					console.log('test failed');
					return;
				}

				cb();
			};

			const usr = 'pi',pwd = 'raspberry';
			ws.sendSignIn(usr,pwd);
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				signIn(() => { console.log('fin'); });
			},100);
		};

		console.log('testSignIn');
		ws.connect();
	}










	//
	//
	//
	testGetAllCalls() {

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};


		const loadCalls = (cb) => {
			this.eventCalls = (calls) => {
				console.log('eventCalls '+JSON.stringify(calls));
				cb();
			};
			ws.sendGetCalls();
		};


		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				loadCalls(() => {
				loadHistory(() => {
				loadNewMessages(() => {
					console.log('fin'); });
				});
				});
			},100);
		};

		console.log('testGetAllCalls');
		ws.connect();
	}


	testGetAllMessages() {

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};

		const loadCalls = (cb) => {

			const loadFile = (f,cb) => {
				console.log('loadFile '+f);
				this.binaryCallback = (buf) => {
				//	console.log('binaryCallback');
					setTimeout( () => { cb(); },100);
				};
				ws.sendGetMessage(f);
			};

			this.eventCalls = (msgs) => {
				console.log('eventCalls '+JSON.stringify(msgs));

				const max = msgs.length;
				const next = (i) => {
					if (i >= max) {
						cb();
						return;
					}
					loadFile(msgs[i].file,() => {
						setTimeout( () => { next(1+i); },100);
					});
				};
				next(0);
			};
			ws.sendGetCalls();
		};


		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				loadMessages(() => { console.log('fin'); });
			},100);
		};

		console.log('testGetAllMessages');
		ws.connect();
	}



	//
	//
	//
	test_ReadNewMessage() {

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};

		const viewMessage = (msg,cb) => {
			this.eventNewMessages = (new_msgs) => {
				console.log('eventNewMessages '+JSON.stringify(new_msgs));
	
				setTimeout( () => { cb(); },100);
			};
			ws.sendViewedMessage(msg);
		};

		const loadMessages = (cb) => {
			this.eventCalls = (msgs) => {
				console.log('eventCalls '+JSON.stringify(msgs));

				if (0 == msgs.length) {
					console.log('no messages, test stalled');
					return;
				}

				setTimeout( () => { viewMessage(msgs[0],cb); },100);
			};
			ws.sendGetCalls();
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				loadMessages(() => { console.log('fin'); });
			},100);
		};

		console.log('testGetAllMessages');
		ws.connect();
	}




	//
	//
	//
	test_ReadAndHistoryMessage() {

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};

		const historyMessage = (msg,cb) => {	
			this.eventHistory = (history) => {
				console.log('eventHistory '+JSON.stringify(history));

				setTimeout( () => { cb(); },100);
			};	
			ws.sendHistoryMessage(msg);
		};

		const loadCalls = (cb) => {
			this.eventCalls = (calls) => {
				console.log('eventCalls '+JSON.stringify(calls));

				if (0 == calls.length) {
					console.log('no calls, test stalled');
					return;
				}

				setTimeout( () => { historyMessage(calls[0],cb); },100);
			};
			ws.loadCalls();
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				loadCalls(() => { console.log('fin'); });
			},100);
		};

		console.log('testGetAllMessages');
		ws.connect();
	}


	//
	//
	//
	test_ReadHistoryAndDeleteMessage() {

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};

		const deleteMessage = (msg,cb) => {	
			this.eventHistory = (history) => {
				console.log('eventHistory '+JSON.stringify(history));

				setTimeout( () => { cb(); },100);
			};
			ws.sendDeleteMessage(msg);
		};

		const loadHistory = (cb) => {
			this.eventHistory = (history) => {
				console.log('loadHistory '+JSON.stringify(history));

				if (0 == history.length) {
					console.log('no history, test stalled');
					return;
				}

				setTimeout( () => { deleteMessage(history[0],cb); },100);
			};
			ws.loadHistory();
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				loadMessages(() => { console.log('fin'); });
			},100);
		};

		console.log('testGetAllMessages');
		ws.connect();
	}




	//
	//
	//
/*
	test_MoveAMessage() {

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};

		const deleteMessage = (msg,cb) => {	
			this.eventHistory = (history) => {
				console.log('eventHistory '+JSON.stringify(history));

				setTimeout( () => { cb(); },100);
			};
			ws.sendDeleteMessage(msg);
		};

		const historyMessage = (msg,cb) => {	
			this.eventHistory = (history) => {
				console.log('eventHistory '+JSON.stringify(history));

				setTimeout( () => { deleteMessage(msg,cb); },100);
			};	
			ws.sendHistoryMessage(msg);
		};

		const viewMessage = (msg,cb) => {
			this.eventNewMessages = (new_msgs) => {
				console.log('eventNewMessages '+JSON.stringify(new_msgs));
	
				setTimeout( () => { historyMessage(msg,cb); },100);
			};
			ws.sendViewedMessage(msg);
		};

		const loadMessages = (cb) => {
			this.eventCalls = (msgs) => {
				console.log('eventCalls '+JSON.stringify(msgs));

				if (0 == msgs.length) {
					console.log('no messages, test stalled');
					return;
				}

				setTimeout( () => { viewMessage(msgs[0],cb); },100);
			};
			ws.sendGetCallss();
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				loadMessages(() => { console.log('fin'); });
			},100);
		};

		console.log('testGetAllMessages');
		ws.connect();
	}
// */



	test_Disconnectd() {

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};


		const loadDisconnected = (cb) => {
			this.binaryCallback = (buf) => {
			//	console.log('binaryCallback');
				setTimeout( () => { cb(); },100);
			};

			ws.sendGetDisconnectedMessage();
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				loadOGM(() => { console.log('fin'); });
			},100);
		};

		console.log('test_Disconnectd');
		ws.connect();
	}

	test_OGM() {
		const line = 1;

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};


		const loadOGM = (cb) => {
			this.binaryCallback = (buf) => {
			//	console.log('binaryCallback');
				setTimeout( () => { cb(); },100);
			};

			ws.sendGetCurrentOGM(line);
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				loadOGM(() => { console.log('fin'); });
			},100);
		};

		console.log('test_OGM');
		ws.connect();
	}
	//
	//
	//
	test_LennyProfile() {

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};


		const loadProfiles = (cb) => {

			const loadFile = (f,cb) => {
				console.log('loadFile '+f);
				this.binaryCallback = (buf) => {
				//	console.log('binaryCallback');
					setTimeout( () => { cb(); },100);
				};
				ws.sendGetProfileMsg(f);
			};

			this.eventProfiles = (profiles) => {
				console.log('eventProfiles '+JSON.stringify(profiles));

				const files = profiles['Lenny'];
				const max = files.length;
				const next = (i) => {
					if (i >= max) {
						cb();
						return;
					}
					loadFile(files[i],() => {
						setTimeout( () => { next(1+i); },100);
					});
				};
				next(0);
			};
			ws.sendGetProfiles();
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				loadProfiles(() => { console.log('fin'); });
			},100);
		};

		console.log('test_LennyProfile');
		ws.connect();
	}



	//
	//
	//
	test_Profile_AddRemove() {

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};


		const name = 'test';

		const remove = (cb) => {
			this.eventProfiles = (profiles) => {
				console.log('eventProfiles '+JSON.stringify(profiles));

				if (profiles.hasOwnProperty(name)) {
					LOGV('failed to remove');
					return;
				}

				console.log('test passed');
			};
			ws.sendProfileRemove(name);
		};

		const add = (cb) => {
			this.eventProfiles = (profiles) => {
				console.log('eventProfiles '+JSON.stringify(profiles));

				if (!profiles.hasOwnProperty(name)) {
					LOGV('failed to add');
					return;
				}

				setTimeout( () => { remove(); },1000);
			};
			ws.sendProfileAdd(name);
		};

		const load = (cb) => {
			this.eventProfiles = (profiles) => {
				console.log('eventProfiles '+JSON.stringify(profiles));

				if (profiles.hasOwnProperty(name)) {
					console.log('skip add whn already contains :'+name);
					setTimeout( () => { remove(); },1000);
					return;
				}
				setTimeout( () => { add(); },1000);
			};
			ws.sendGetProfiles();
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				load(() => {
				add( () => {
				remove( () => {
					console.log('fin'); });
				})
				})
			},100);
		};

		console.log('test_Profile_AddRemove');
		ws.connect();
	}





	//
	//
	//
	test_LennyEnabled_Toggle() {
		this.test_Line_Config_Enable_Toggle(1,'lenny_enabled');
	}
	test_DisconnectedEnabled_Toggle() {
		this.test_Line_Config_Enable_Toggle(1,'disconnectd_enabled');
	}
	test_MessagesEnabled_Toggle() {
		this.test_Line_Config_Enable_Toggle(1,'tad_enabled');
	}


	test_Line_Config_Enable_Toggle(lineName,key) {
		console.log('test_Line_Config_Enable_Toggle lineName:'+lineName+' key:'+key);

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};

		const toggle = (enabled,cb) => {
			this.eventConfig = (cfg) => {
				console.log('eventConfig '+JSON.stringify(cfg));

				if (cfg.line[0][key] == enabled) {
					LOGV('failed to remove');
					return;
				}

				console.log('test passed');
				setTimeout( () => { cb(); },100);
			};
			ws.sendSetConfig(lineName,key,enabled);
		};

		let line = null;

		const getConfig = (cb) => {
			this.eventConfig = (cfg) => {
				console.log('eventConfig '+JSON.stringify(cfg));
				line = cfg.line[0];

				setTimeout( () => { cb(); },100);
			};
			ws.sendGetConfig();
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				getConfig(() => {
				toggle(!line[key],() => {
				toggle( line[key],() => {
				toggle(!line[key],() => {
				toggle( line[key],() => {
					console.log('fin');
				});
				})
				})
				})
				})
			},100);
		};

		console.log('test_Skiplist_Toggle');
		ws.connect();
	}

	//
	//
	//

	test_Skiplist_Toggle() {

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};

		const toggle = (enabled,cb) => {
			this.eventConfig = (cfg) => {
				console.log('eventConfig '+JSON.stringify(cfg));

				if (cfg.config.skiplist_enabled == enabled) {
					LOGV('failed to remove');
					return;
				}

				console.log('test passed');
				setTimeout( () => { cb(); },100);
			};
			ws.sendGetSkiplistToggle(enabled);
		};

		let config = null;

		const getConfig = (cb) => {
			this.eventConfig = (cfg) => {
				console.log('eventConfig '+JSON.stringify(cfg));
				config = cfg.config;

				setTimeout( () => { cb(); },100);
			};
			ws.sendGetConfig();
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				getConfig(() => {
				toggle(!config.skiplist_enabled,() => {
				toggle( config.skiplist_enabled,() => {
				toggle(!config.skiplist_enabled,() => {
				toggle( config.skiplist_enabled,() => {
					console.log('fin');
				});
				})
				})
				})
				})
			},100);
		};

		console.log('test_Skiplist_Toggle');
		ws.connect();
	}


	//
	//
	//
	test_Skiplist_AddRemove() {

		this.eventDisconnected = (ev) => {
			console.log('eventDisconnected - test failed');
		};



		//
		//
		//

		const add = (cb) => {
			const testNumber = '8005551212',testName = 'help',testNote = 'test';

			this.eventSkiplist = (wl) => {
				console.log('eventSkiplist '+JSON.stringify(wl));

				const max = wl.length;
				for (let i=0;i<max;i++) {
					const it = wl[i];
					if (testNumber != it.number) continue;
					if (testName != it.name) continue;

					console.log('test passed');
					setTimeout( () => {
						cb();
					},100);
					return;
				}
				console.log('test failed');
			};
			ws.sendSkiplistAdd( {number:testNumber,'name':testName},testNote);
		};

		const remove = (cb) => {
			const testNumber = '8005551212';

			this.eventSkiplist = (wl) => {
				console.log('eventSkiplist '+JSON.stringify(wl));

				const max = wl.length;
				for (let i=0;i<max;i++) {
					const it = wl[i];
					if (testNumber != it.number) continue;
					if (testName != it.name) continue;

					console.log('test failed');
					return;
				}
				console.log('test passed');
				setTimeout( () => {
					cb();
				},100);
			};
			ws.sendSkiplistRemove( {number:testNumber});
		};

		const load = (cb) => {
			this.eventSkiplist = (wl) => {
				console.log('eventSkiplist '+JSON.stringify(wl));
				setTimeout( () => { cb(); },100);
			};
			ws.sendGetSkiplist();
		};

		this.eventConnected = (ev) => {
			console.log('eventConnected - test success - connect');

			setTimeout( (ev) => {
				load(() => {
				add( () => {
				remove( () => {
					console.log('fin'); });
				});
				});
			},100);
		};

		console.log('test_Skiplist_AddRemove');
		ws.connect();
	}

}















//
//
//
var dbg = true;

class WS {
	constructor() {
		const url = new URL(document.location);
		this.ip = url.host;

		this.wsurl = (WS.isSecure() ?'wss://' :'ws://') +this.ip +'/ws';
	}

	static isSecure() {
		return 'https:' === location.protocol;
	}

	static sendToSecure() {
		const url = new URL(document.location);
		const l = 'https://' + url.hostname+':'+(1+ parseInt(url.port));
		console.log('sendToSecure '+l);
		document.location = l;
		return;		
	}


	//
	//
	//

	connect() {
		if (dbg) console.log('WS connect: '+this.wsurl);

		delete this['hasError'];

		this.websocket = new WebSocket(this.wsurl,'lenny');
		this.websocket.binaryType = 'arraybuffer';

		this.websocket.onopen    = (ev) => { if (dbg) console.log('websocket onOpen'); lenny.eventConnected(); };
		this.websocket.onerror   = (ev) => { if (dbg) console.error('websocket onError '+ev); this.hasError = true; lenny.eventError('error',ev); };
		this.websocket.ontimeout = (ev) => { if (dbg) console.error('websocket onTimeout '+ev); this.hasError = true; lenny.eventError('timeout',ev); }; 
		this.websocket.onclose   = (ev) => { if (dbg) console.log('websocket onClose'); delete this['websocket']; lenny.eventDisconnected(); };
		this.websocket.onmessage = (ev) => { if (typeof ev.data === 'object') { this.onBinaryMessage(ev); } else { this.onTextMessage(ev); } };
	}

	onTextMessage(ev) {
		//if (dbg) console.log('onTextMessage json:'+ev.data);
//		setTimeout( () => {
			try {
				const rq = JSON.parse(ev.data);

				//if (dbg) console.log(JSON.stringify(rq));

				if (null != rq.info     ) { lenny.eventInfo(   rq.info); return; }
				if (null != rq.signin   ) { lenny.eventSignIn( rq.signin); return; }
				if (null != rq.config   ) { lenny.eventConfig( rq); return; }
				if (null != rq.status   ) { lenny.eventStatus( rq.status   ); return; }

				if (null != rq.skiplist) { lenny.eventSkiplist(rq.skiplist); return; }
				if (null != rq.profiles) { lenny.eventProfiles(rq.profiles); return; }

				if (null != rq.calls    ) { lenny.eventCalls(  rq.calls    ); return; }

				if (dbg) console.log('not handled:'+JSON.stringify(rq));

			} catch (err) {
				if (dbg) console.error('err:'+err);
				if (dbg) console.error('json:'+ev.data);
			}
//		},0);
	}	

	onBinaryMessage(ev) {

		if (!this.chunks) {
			const b = new Uint8Array(ev.data);
			const s = (b[0] >>>6 &0x03);

			if (1 != s) {
				if (dbg) console.error('invalid signature : '+s);
				return;
			}

			const line =  (b[0] &0x0F);
			const len  = ((b[1] &0xFF) <<16) + ((b[2] &0xFF) <<8) + (b[3] &0xFF);

			if (0 != line && 1 != line && 2 != line) {
				if (dbg) console.log('invalid line:'+line);
				return;
			}

			if (b.length == 4 + len) {
				lenny.binaryCallback({ buf:b,offset:4 });
				return;
			}

			if (b.length > 4 + len) {
				if (dbg) console.log('invalid len:'+(4+len)+' pkt len:'+b.length);
				return;
			}

			//
			// chunks
			//
			this.chunkLen = 4 + len;
			this.chunksCurrentLen = b.length;
			this.chunks = [];
			this.chunks.push(b);

		} else {

			const b = new Uint8Array(ev.data);
			this.chunksCurrentLen += b.length;
			this.chunks.push(b);

			if (this.chunksCurrentLen < this.chunkLen) {
				// not finished yet
			} else {
				const b = new Uint8Array(this.chunkLen);
				const cmax = this.chunks.length;
				for (let i=0,j=0;i<cmax;i++) {
					b.set(this.chunks[i],j);
					j += this.chunks[i].length;
				}

				this.currentChunkLen = 0;
				this.chunkLen = 0;
			    delete this['chunks'];

				lenny.binaryCallback({ buf:b,offset:4 });
			}
		}
	}


	//
	//
	//

	sendComment(rating,comment) {
		if (dbg) console.log('sendComment');
		const r = { a:'comment',r:rating,c:comment };
		this.websocket.send( JSON.stringify(r) );
	}
	sendBugReport(summary,comment) {
		if (dbg) console.log('sendBugReport');
		const r = { a:'bug',s:summary,c:comment };
		this.websocket.send( JSON.stringify(r) );
	}

	sendUpdateCheck() {
		if (dbg) console.log('sendUpdateCheck');
		const r = { a:'update_check' };
		this.websocket.send( JSON.stringify(r) );
	}
	sendLicenseEmail(email) {
		if (dbg) console.log('sendLicenseEmail');
		const r = { a:'license',t:'email',v:email };
		this.websocket.send( JSON.stringify(r) );
	}
	sendLicenseKey(key) {
		if (dbg) console.log('sendLicenseKey');
		const r = { a:'license',t:'save',v:key };
		this.websocket.send( JSON.stringify(r) );
	}
	sendCreateLicense(days) {
		if (dbg) console.log('sendCreateLicense');
		const r = { a:'license',t:'create',d:days };
		this.websocket.send( JSON.stringify(r) );
	}

	//
	sendSignIn(usr,pwd) {
		if (dbg) console.log('sendSignIn');
		const r = { a:'signin',n:usr,p:pwd };
		this.websocket.send( JSON.stringify(r) );
	}

	//
	sendGetSystemInfo() {
		if (dbg) console.log('sendGetSystemInfo');
		const r = { a:'info_get' };
		this.websocket.send( JSON.stringify(r) );
	}

	sendGetStatus() {
		if (dbg) console.log('sendGetStatus');
		const r = { a:'status_get' };
		this.websocket.send( JSON.stringify(r) );
	}

	sendGetConfig() {
		if (dbg) console.log('sendGetConfig');
		const r = { a:'config_get' };
		this.websocket.send( JSON.stringify(r) );
	}
	sendSetConfig(line,key,value) {
		if (dbg) console.log('sendSetConfig');
		const r = { a:'config_set',l:line,key:key,value:value };
		this.websocket.send( JSON.stringify(r) );
	}

	//
	sendGetSkiplist() {
		if (dbg) console.log('sendGetSkiplist');
		const r = { a:'skiplist_get' };
		this.websocket.send( JSON.stringify(r) );
	}
	sendSkiplistToggle(enabled) {
		if (dbg) console.log('sendSkiplistToggle '+enabled);
		const r = { a:'skiplist_set',t:'active',value:enabled };
		this.websocket.send( JSON.stringify(r) );
	}
	sendSkiplistAdd(msg) {
		if (dbg) console.log('sendSkiplistAdd');
		const r = { a:'skiplist_set',t:'add',name:msg.name,number:msg.number,note:msg.note };
		this.websocket.send( JSON.stringify(r) );
	}
	sendSkiplistUpdate(msg) {
		if (dbg) console.log('sendSkiplistUpdate');
		const r = { a:'skiplist_set',t:'update',name:msg.name,number:msg.number,note:msg.note };
		this.websocket.send( JSON.stringify(r) );
	}
	sendSkiplistRemove(msg) {
		if (dbg) console.log('sendSkiplistRemove');
		const r = { a:'skiplist_set',t:'del',number:msg.number };
		this.websocket.send( JSON.stringify(r) );
	}

	//
	sendGetProfiles() {
		if (dbg) console.log('sendGetProfiles');
		const r = { a:'profiles_get' };
		this.websocket.send( JSON.stringify(r) );
	}
	sendProfileAdd(name) {
		if (dbg) console.log('sendProfileAdd');
		const r = { a:'profile_set',t:'add',name:name };
		this.websocket.send( JSON.stringify(r) );
	}
	sendProfileRemove(name) {
		if (dbg) console.log('sendProfileRemove');
		const r = { a:'profile_set',t:'del',name:name };
		this.websocket.send( JSON.stringify(r) );
	}
	sendProfileRemoveMsg(name,idx) {
		if (dbg) console.log('sendProfileRemoveMsg idx:'+idx);
		const r = { a:'profile_set',t:'del',name:name,i:idx };
		this.websocket.send( JSON.stringify(r) );
	}


	//
	//
	sendAction(line,act) {
		if (dbg) console.log('sendAction '+act+' line:'+line);
		const r = { a:act,l:line };
		this.websocket.send( JSON.stringify(r) );
	}

	//
	sendListenOn(line) {
		if (dbg) console.log('sendListenOn line:'+line);
		const r = { a:'listen_on',l:line };
		this.websocket.send( JSON.stringify(r) );
	}
	sendListenOff(line) {
		if (dbg) console.log('sendListenOff line:'+line);
		const r = { a:'listen_off',l:line };
		this.websocket.send( JSON.stringify(r) );
	}
	sendRecordOn(line) {
		if (dbg) console.log('sendRecordOn line:'+line);
		const r = { a:'record_on',l:line };
		this.websocket.send( JSON.stringify(r) );
	}
	sendRecordOff(line) {
		if (dbg) console.log('sendRecordOff line:'+line);
		const r = { a:'record_off',l:line };
		this.websocket.send( JSON.stringify(r) );
	}


	//
	sendGetCalls() {
		if (dbg) console.log('sendGetCalls');
		const r = { a:'calls_get' };
		this.websocket.send( JSON.stringify(r) );
	}

	//
	sendViewedMessage(msg) {
		if (dbg) console.log('sendViewedMessage');
		const r = { a:'call_set',t:'viewed',m:msg };
		this.websocket.send( JSON.stringify(r) );
	}
	sendHistoryMessage(msg) {
		if (dbg) console.log('sendHistoryMessage');
		const r = { a:'call_set',t:'history',m:msg };
		this.websocket.send( JSON.stringify(r) );
	}
	sendDeleteMessage(msg) {
		if (dbg) console.log('sendDeleteMessage');
		const r = { a:'call_set',t:'del',m:msg };
		this.websocket.send( JSON.stringify(r) );
	}

	//
	sendGetMessage(file) {
		if (dbg) console.log('sendGetMessage f:'+file);
		const r = { a:'call_message',m:file };
		this.websocket.send( JSON.stringify(r) );
	}
	sendGetProfileMsg(msg) {
		//if (dbg) console.log('sendGetProfileMsg m:'+msg);
		const r = { a:'profile_message',m:msg };
		this.websocket.send( JSON.stringify(r) );
	}
	sendGetDisconnectedMessage() {
		if (dbg) console.log('sendGetDisconnectedMessage');
		const r = { a:'disconnected_message' };
		this.websocket.send( JSON.stringify(r) );
	}
	sendGetCurrentOGM(line) {
		if (dbg) console.log('sendGetCurrentOGM l:'+line);
		const r = { a:'tad_message',l:line };
		this.websocket.send( JSON.stringify(r) );
	}

	sendSaveCurrentOGM(line,buf) {
		//if (dbg) console.log('sendSaveCurrentOGM l:'+line);
		const r = { a:'tad_set_message',t:'add',l:line.idx };
  		this.websocket.send( JSON.stringify(r) );
  		this.websocket.send(buf);
	}
	sendRemoveCurrentOGM(line) {
		//if (dbg) console.log('sendRemoveCurrentOGM l:'+line);
		const r = { a:'tad_set_message',t:'del',l:line.idx };
  		this.websocket.send( JSON.stringify(r) );
	}



	sendSendNewProfileMsg(profile,idx) {
		//if (dbg) console.log('sendSendNewProfileMsg p:'+profile+' idx:'+idx);
		const r = { a:'profile_set_message',n:profile,i:idx };
  		this.websocket.send( JSON.stringify(r) );
	}

}






