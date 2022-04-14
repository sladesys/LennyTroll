/*


*/
'use strict';


//
const appVersion = '0.0';

var supportsPassive = false;
var focus = true;
var lenny = null,ws = null;

var heightCalc = window.innerHeight,heightMinCalc = window.innerHeight;
var portrait = window.innerWidth < window.innerHeight;
var popupPortraitHeight = Math.floor(window.innerHeight *0.8);

var audioCanvas = { w:180,h:60 };
var isMobile = false,isMobileBrowser = false;

var dbg = true;
var test = null;
const parms = getUrlParams(window.location);

const arrowLeft  = '&nbsp;&#x276E;&nbsp;'; // &#x21E6;
const arrowRight = '&nbsp;&#x276F;&nbsp;'; // &#x21E8;

//console.log('outer '+window.outerWidth+'x'+window.outerHeight+' inner:'+window.innerWidth+'x'+window.innerHeight);
//console.log('orig:'+JSON.stringify(origDim));

(() => {
	//if (dbg) console.log('parms: '+JSON.stringify(parms));

	if (parms.hasOwnProperty('dbg')) dbg = true;

	if (parms.hasOwnProperty('test')) {
		document.write('<script type="text/javascript" src="js/testui.js" ></script>');
	}




/*
	if (p && 0 < Object.keys(p).length) {
		const parseBool = (v) => { return ((/^(true|1)$/i).test(v) && v) ?true :false; };
		if (p.test && parseBool(p.test)) {
			document.write('<script type="text/javascript" src="js/testui.js" ></script>');
		}
	}
*/
/*
	const l = document.createElement('script'); 
	l.src = 'js/testui.js';  
	document.head.appendChild(l);
*/
/*
	const l = document.createElement('link'); 
	l.rel  = 'stylesheet';  
	l.type = 'text/css'; 
	l.href = 'testui.css';  
	document.getElementsByTagName('HEAD')[0].appendChild(l);  
*/




	//
	// check passive : TODO is this needed?
	//
	try {
		//
		// Test via a getter in the options object to see if the passive property is accessed
		//  https://github.com/WICG/EventListenerOptions/blob/gh-pages/explainer.md
		//
		const opts = Object.defineProperty({},'passive',{
			get:() => {
				//if (dbg) console.log('supportsPassive');
				supportsPassive = { passive:true };
			}
		});
		window.addEventListener(   'testPassive',null,opts);
		window.removeEventListener('testPassive',null,opts);
	} catch (err) { if (dbg) console.log('supportsPassive err:'+err); }

	//
	//
	//
	if (dbg) console.log('document'+JSON.stringify(document,null,2));
//	if (dbg) console.log('navigator'+JSON.stringify(navigator,null,2));

	isMobile = (/iPhone|iPod|iPad|Android|BlackBerry/).test(navigator.userAgent);
	if (dbg) console.log('os:'+getHostOS()+' platform:'+navigator.platform+' mobile:'+isMobile+' userAgent:'+navigator.userAgent);


	//
	// https://web.dev/customize-install/#detect-mode
	//
	// if (window.matchMedia('(display-mode: standalone)').matches) {
	//   console.log('display-mode is standalone');
	// }
	//


	window.addEventListener('appinstalled',(ev) => { if (dbg) console.log('lenny installed'); });


	if (navigator.standalone) {
		if (dbg) console.log('Installed (iOS)'+(isMobile?' mobile':''));
	} else
	if (matchMedia('(display-mode: standalone)').matches) {
		if (dbg) console.log('Launched: Installed'+(isMobile?' mobile':''));
	} else {
		if (dbg) console.log('Launched: Browser Tab'+(isMobile?' mobile':''));
//		isMobileBrowser = ('iOS' === getHostOS()) && isMobile;
	}

})();






//
// Lenny app
//
document.addEventListener('DOMContentLoaded',() => {

	// TESTing
//	document.getElementById('theme_mobile').disabled = !isMobile;


	//
	// Listeners
	//

	window.addEventListener('resize',(ev) => {
		if (lenny) setTimeout(() => { lenny.resizeUI(); },0);
	},false);


	//
	// lenny
	//

	lenny = new Lenny();
	ws = new WS();

	if (test) { test(); return; }

	setTimeout( (ev) => { lenny.start(); },0);


	//
	// background service
	//
	if ('serviceWorker' in navigator) {
		navigator.serviceWorker.register('js/sw.js').then((r) => {
			if (dbg) console.log('service worker registration successful with scope:',r.scope);
		}).catch((err) => {
			if (dbg) console.log('service worker registration failed:',err);
		});

		//
		if (navigator.serviceWorker.controller) {
			if (dbg) console.log('service worker page : ',navigator.serviceWorker.controller);
		}
		navigator.serviceWorker.oncontrollerchange = () => {
			if (dbg) console.log('service worker oncontrollerchange  page : ',navigator.serviceWorker.controller);
		};

	} else {
		if (dbg) console.log('no serviceWorker');
	}

});




//
//
//

class Lenny {

	constructor() {

		this.includeLinesInUi = false;
		this.flagCreateUI = true;

		//
		//
		//

		this.binaryCallback    = (buf    ) => { if (dbg) console.log('binaryCallback dropping len:'+buf.buf.length); };

		this.eventError        = (type,ev) => {
			if (dbg) console.log('eventError '+type,ev);
		};
		this.eventConnected    = (       ) => { if (dbg) console.log('eventConnected'   ); this.onEventConnected(); };
		this.eventDisconnected = (       ) => { if (dbg) console.log('eventDisconnected'); this.onEventDisconnected(); };

		this.eventInfo         = (info   ) => { if (dbg) console.log('eventInfo'        ); this.onEventInfo(info); };
		this.eventSignIn       = (acct   ) => { if (dbg) console.log('eventSignIn'      ); this.signedIn = acct; ws.sendGetConfig(); };
		this.eventConfig       = (cfg    ) => { if (dbg) console.log('eventConfig'      ); this.onEventConfig(cfg); };
		this.eventStatus       = (status ) => { if (dbg) console.log('eventStatus'      ); this.onEventStatus(status); };
		this.eventSkiplist     = (list   ) => { if (dbg) console.log('eventSkiplist'    ); this.onEventSkiplist(list); };
		this.eventProfiles     = (pr     ) => { if (dbg) console.log('eventProfiles'    ); this.profiles = pr; }
		this.eventCalls        = (calls  ) => { if (dbg) console.log('eventCalls'       ); this.onEventCalls(calls); };
	}



	//
	//
	//
	start() {
		if (dbg) console.log('start');

		{
			const t = localStorage.getItem('theme');
			if ('dark' === t) {
				document.getElementById('theme_dark').disabled = false;
				document.getElementById('theme_lite').disabled = true;
			} else {
				document.getElementById('theme_dark').disabled = true;
				document.getElementById('theme_lite').disabled = false;
			}
			if (localStorage.getItem('dbg')) dbg = true;
		}

		this.navNone();
		this.resizeUI();

		const load = () => {
			new Popup().showLoading();
			ws.connect();
		};

		{
			const v = localStorage.getItem('starts');
			const starts = !v || isNaN(v) ?0 :parseInt(v);
			localStorage.setItem('starts',1+starts);

			if (!parms.hasOwnProperty('skip_start') && 0 == starts) {
				this.showTitle();
				new Popup().showFirstTime(() => { load(); });
				return;
			}
		}

		load();
	}



	//
	//
	//
	showTitle() {
		if (dbg) console.log('showTitle');
		const addTitle = () => {

			const div = document.createElement('div');
			document.body.appendChild(div);

			const h1 = document.createElement('h1');
			const h2 = document.createElement('h2');
			const h3 = document.createElement('h3');
			div.appendChild(h1);
			div.appendChild(h2);
			div.appendChild(h3);

			div.id = 'title';

			h1.innerText = 'Lenny - The Telemarketing Troll';

			if (this.cfg && this.cfg.config && this.cfg.config.version) {
				const ver = this.cfg.config.version.split(':')[0];
				if (appVersion == ver) {
					h2.innerText = ver+' beta';
				} else {
					h2.innerHTML = '<span style="color:red">'+appVersion+' ~ '+ver+' beta';
				}
			} else {
				h2.innerHTML = '<span style="color:red">'+appVersion+' beta';
			}

			h3.innerText = 'by LennyTroll.com'
		};

		Popup.closeAll();
		document.body.innerText = '';
		addTitle();
	}

	createUI() {

		// activity
		const addActivity = () => {

			const div = document.createElement('div');
			document.body.appendChild(div);

			const fr = document.createElement('div');
			const fs = document.createElement('fieldset');
			{
				const leg = document.createElement('legend');
				const sp = document.createElement('span');

				leg.appendChild(sp);
				sp.innerText = 'Activity';

				fs.appendChild(leg);
			}
			div.appendChild(fr);
			fr.appendChild(fs);

			const d = document.createElement('div');
			fs.appendChild(d);

			div.id = 'activity';
			fr.id = 'activity_frame';
			d.id = 'activity_items';
		}

		// messages
		const addMessages = () => {

			const div = document.createElement('div');
			document.body.appendChild(div);

			const fr = document.createElement('div');
			const fs = document.createElement('fieldset');
			{
				const leg = document.createElement('legend');
				fs.appendChild(leg);

				const leg_sp = document.createElement('span');
				const leg_cnt = document.createElement('span');

				leg.appendChild(leg_sp);
				leg.appendChild(leg_cnt);

				leg_sp.innerText = 'Calls';
				leg_cnt.id = 'calls_count'
			}
			div.appendChild(fr);
			fr.appendChild(fs);

			const d = document.createElement('div');
			fs.appendChild(d);

			div.id = 'messages';
			fr.id = 'calls_frame';
			d.id = 'calls_items';
		}

		// history
		const addHistory = () => {

			const div = document.createElement('div');
			document.body.appendChild(div);

			const fr = document.createElement('div');
			const fs = document.createElement('fieldset');
			{
				const leg = document.createElement('legend');
				fs.appendChild(leg);

				const leg_sp = document.createElement('span');
				const leg_cnt = document.createElement('span');
			//	leg_span = '';
			//	leg_cnt.id = 'calls_count';

			//	const d = document.createElement('div');

				leg.appendChild(leg_sp);
				leg.appendChild(leg_cnt);

				leg_sp.innerText = 'History';
				leg_cnt.id = 'history_count'
			}
			{
				const d = document.createElement('div');
				const selectAll = document.createElement('input');
				const deleteAll  = document.createElement('button');
				selectAll.type = 'button';
				selectAll.id = 'history_selectall_but';
				selectAll.value = 'Select All';
				deleteAll.id = 'history_delete_but';
				deleteAll.classList = 'right';
				deleteAll.innerHTML = '&#x1F5D1; Delete Permenantly';

				d.appendChild(selectAll);
				d.appendChild(deleteAll);
				fs.appendChild(d);
			}
			div.appendChild(fr);
			fr.appendChild(fs);

			const d = document.createElement('div');
			fs.appendChild(d);

			div.id = 'history';
			fr.id = 'history_frame';
			d.id = 'history_items';
		};

		// settings
		const addSettings = () => {

			const div = document.createElement('div');
			document.body.appendChild(div);

			const fr = document.createElement('div');
			const fs = document.createElement('fieldset');
			{
				const leg = document.createElement('legend');
				fs.appendChild(leg);

				const sp = document.createElement('span');

				leg.appendChild(sp);
				sp.innerText = 'Settings';
			}

			div.appendChild(fr);
			fr.appendChild(fs);

			const d = document.createElement('div');
			fs.appendChild(d);

			div.id = 'settings';
			fr.id = 'settings_frame';
			d.id = 'settings_items';
		};

		//
		const addNav = () => {

			const nav_bar = document.createElement('div');
			document.body.appendChild(nav_bar);
			nav_bar.id ='nav_bar';

			{
				const p = window.innerWidth < window.innerHeight;
				if (p) {
					if (700 > window.innerWidth) {
						nav_bar.style.height = '76px';
					} else {
						nav_bar.style.height = '84px';
					}
				} else {
					nav_bar.style.height = '76px';
				}
			}

/*

&#128222; - offhook phone
&#x2706;

&#x260E;

&#x1F4C5; - calendar
&#x1F5D1; - trash

&#x263c; - settings
&#x2744;

*/
			const m = [
				{n:'Activity',t:'<div style="height:30px;font-size:32px">&#128222;</div>Activity',f:(ev) => { this.navActivity(); }},
				{n:'Calls'   ,t:'<div style="height:30px;font-size:32px">&#x260E;</div>Calls',f:(ev) => { this.navMessages(); }},
				{n:'History' ,t:'<div style="height:30px;font-size:32px">&#x1F5D1;</div>History',f:(ev) => { this.navHistory(); }},
				{n:'Settings',t:'<div style="height:30px;font-size:32px">&#x263c;</div>Settings',f:(ev) => { this.navSettings(); }}
			];

			const tbl = document.createElement('table');
			nav_bar.appendChild(tbl);

			const tr = tbl.insertRow();

			m.forEach( (it) => {

				{
					const fr = document.createElement('div');
					{
						const legend = document.createElement('legend');
						const legend_span_title = document.createElement('span');
						const legend_span_count = document.createElement('span');

						legend.appendChild(legend_span_title);
						legend.appendChild(legend_span_count);

						fr.appendChild(legend);

						const fieldset = document.createElement('fieldset');
						fieldset.appendChild(legend);

						const data = document.createElement('div');
						fieldset.appendChild(data);

						if ('History' === it.n) {
							legend_span_count.id = 'history_count';
						}
						if ('Messages' === it.n) {
							legend_span_count.id = 'calls_count';
						}
					}

					const div = document.createElement('div');
					div.appendChild(fr);
				}

/*
		const l = document.getElementById('nav_activity');
		const l = document.getElementById('nav_history');
		const l = document.getElementById('nav_calls');
		const l = document.getElementById('nav_settings');

		this.nav_activity = nav_activity;
		this.nav_history  = nav_history;
		this.nav_calls = nav_calls;
		this.nav_settings = nav_settings;

*/

				{
					const nav_cell     = document.createElement('div');
					const nav_activity = document.createElement('div');
					const nav_img      = document.createElement('div');
					const nav_text     = document.createElement('div');

					tr.insertCell().appendChild(nav_cell);

					nav_cell       .appendChild(nav_activity);
					nav_activity   .appendChild(nav_img);
					nav_img        .appendChild(nav_text);

					nav_cell    .classList = 'nav_cell';
					nav_activity.classList = 'nav_tab';
					nav_img     .classList = 'nav_img';
					nav_text    .classList = 'nav_text';

					if ('Calls' === it.n) {
						const nav_badge           = document.createElement('div');
						const nav_badge_count     = document.createElement('div');
						nav_badge      .classList = 'badge_container';
						nav_badge_count.classList = 'badge hidden';

						nav_badge_count.id = 'message_count';
			
						nav_badge.innerText = it.n;
//						nav_badge.innerHTML = it.t;
						nav_badge.appendChild(nav_badge_count);
						nav_text .appendChild(nav_badge);

					} else {

						nav_text.innerText = it.n;
//						nav_text.innerHTML = it.t;
					}

					nav_cell.parentElement.onclick = it.f;

					nav_activity.id = 'nav_'+it.n.toLowerCase();
				}
			});
		};

		//
		//
		//
		this.showTitle();
		addActivity();
		addMessages();
		addHistory();
		addSettings();
		addNav();
	}

	//
	//
	//
	resizeUI() {

		if (!isMobile) {
			portrait = true;
		} else {
			portrait = window.innerWidth < window.innerHeight;
		}

	//	const width  = isMobile ?window.outerWidth :window.innerWidth;
	//	const height = isMobile ?window.outerHeight :window.innerHeight;
		const width  = Math.min(window.outerWidth,window.innerWidth);
		const height = Math.min(window.outerHeight,window.innerHeight);

		let heightTitle = 50,heightNavBar = 76,heightLegend = 50 + (isMobileBrowser ?100 :0);

		{
			const t = document.getElementById('title');
			const n = document.getElementById('nav_bar');
			if (t) heightTitle = t.getBoundingClientRect().height;
			if (n) heightNavBar = n.getBoundingClientRect().height;

			if (dbg) console.log('title height:'+heightTitle);
			if (dbg) console.log('nav bar height:'+heightNavBar);
		}

		const fudge = window.outerHeight >= window.innerHeight ?0 :Math.floor( (window.innerHeight - window.outerHeight) /window.devicePixelRatio /2);

	//	heightCalc = window.innerHeight - fudge;
		heightCalc = height - fudge;
		heightMinCalc = heightTitle +heightNavBar + heightLegend + 15;// +90;
		// +(700 > window.innerWidth ?26 :70); // title + legend
		//
		popupPortraitHeight = (heightCalc - 150 +24 +24 -60 -(isMobileBrowser ?100 :0));
		//popupPortraitHeight = (heightCalc -90 -100);

		if (dbg) console.log('heightCalc:'+heightCalc+' fudge:'+fudge+' popupPortraitHeight:'+popupPortraitHeight+' window size outer '+window.outerWidth+'x'+window.outerHeight+' inner:'+window.innerWidth+'x'+window.innerHeight+' r:'+window.devicePixelRatio);

		const ch = heightCalc - heightMinCalc;

		{
			const l = document.getElementById('activity_items');
			const m = document.getElementById('calls_items');
			const h = document.getElementById('history_items');
			const s = document.getElementById('settings_items');
			const n = document.getElementById('nav_bar');

			if (l) l.style.height = ch+'px';
			if (m) m.style.height = ch+'px';
			if (s) s.style.height = ch+'px';
			if (h) h.style.height = (ch-45)+'px'; // nav bar + margin
		}

		if (400 > window.innerWidth) {
			audioCanvas = { w:180,h:60 };
		} else
		if (600 > window.innerWidth) {
			audioCanvas = { w:270,h:90 };
		} else {
			audioCanvas = { w:360,h:120 };
		}

	}

	//
	//
	//
	resizeUI_HistoryButtons(show) {
		if (dbg) console.log('resizeButtons show:'+show);

		const ch = heightCalc - heightMinCalc;

		const h = document.getElementById('history_items');

		if (!show) {
			h.style.height = ch+'px'; // nav bar + margin
		} else {
			h.style.height = (ch-45)+'px'; // nav bar + margin
		}
	}


	//
	//
	//
	resetUI() {
		if (dbg) console.log('resetUI');
		this.flagCreateUI = true;

		//delete this['cfg'];
		delete this['signedIn'];
		delete this['lines'];
		delete this['skiplist'];
		delete this['skiplistNumbers'];
		delete this['profiles'];
		delete this['lastCalls'];
		delete this['lastHist'];
		delete this['lastMsgs'];

		{
			const e = document.getElementById('message_count');
			if (e) e.classList.add('hidden');
		}
		{
			const l = document.getElementById('activity_items');
			if (l) while (0 < l.childNodes.length) l.removeChild(l.childNodes[0]);
		}
		{
			const l = document.getElementById('calls_items');
			if (l) while (0 < l.childNodes.length) l.removeChild(l.childNodes[0]);
		}
		{
			const l = document.getElementById('history_items');
			if (l) while (0 < l.childNodes.length) l.removeChild(l.childNodes[0]);
		}
		{
			const l = document.getElementById('settings_items');
			if (l) while (0 < l.childNodes.length) l.removeChild(l.childNodes[0]);
		}
		{
			const l = document.getElementsByClassName('side_popup');
			if (l) while (0 < l.length) l[0].parentElement.removeChild(l[0]);
		}

		delete this['settings_skiplist_but'];
		delete this['settings_skiplist_txt'];
		delete this['settings_skiplist_enabled'];
		delete this['user_security_enabled'];
		delete this['user_security_txt'];
		delete this['user_security_fieldset'];		
	}

	//
	// navigation
	//

	navActivity() {
		if (!document.getElementById('nav_bar')) return;
		this.navNone();
		document.getElementById('activity').classList.remove('hidden');
		document.getElementById('nav_activity').parentElement.parentElement.classList.add('active');
	}
	navHistory() {
		if (!document.getElementById('nav_bar')) return;
		this.navNone();
		document.getElementById('history').classList.remove('hidden');
		document.getElementById('nav_history').parentElement.parentElement.classList.add('active');
	}
	navMessages() {
		if (!document.getElementById('nav_bar')) return;
		this.navNone();
		document.getElementById('messages').classList.remove('hidden');
		document.getElementById('nav_calls').parentElement.parentElement.classList.add('active');
	}
	navSettings() {
		if (!document.getElementById('nav_bar')) return;
		this.navNone();
		document.getElementById('settings').classList.remove('hidden');
		document.getElementById('nav_settings').parentElement.parentElement.classList.add('active');
	}
	navNone() {
		if (!document.getElementById('nav_bar')) return;

		document.getElementById('activity').classList.add('hidden');
		document.getElementById('history' ).classList.add('hidden');
		document.getElementById('messages').classList.add('hidden');
		document.getElementById('settings').classList.add('hidden');

		document.getElementById('nav_activity').parentElement.parentElement.classList.remove('active');
		document.getElementById('nav_history' ).parentElement.parentElement.classList.remove('active');
		document.getElementById('nav_calls'   ).parentElement.parentElement.classList.remove('active');
		document.getElementById('nav_settings').parentElement.parentElement.classList.remove('active');
	}



	//
	// settings
	//

	toggleUserSecurity() {
		if (dbg) console.log('toggleUserSecurity');
		this.cfg.config.user_security_enabled = !this.cfg.config.user_security_enabled;
		ws.sendSetConfig(0,'user_security_enabled',this.cfg.config.user_security_enabled);
	}
	toggleSkiplist() {
		if (dbg) console.log('toggleSkiplist');

		this.cfg.config.skiplist_enabled = !this.cfg.config.skiplist_enabled;
		this.updateSettings();

		ws.sendSkiplistToggle(this.cfg.config.skiplist_enabled);
	}




	//
	// connection
	//

	onEventConnected() {
		ws.sendGetSystemInfo();
		ws.sendGetConfig();

		Popup.closeAll();
	}
	onEventDisconnected() {
		this.showTitle();

		this.navNone();
		new Popup().showDisconnected();

		this.resetUI();
	}



	//
	// data
	//

	onEventInfo(info) {
		//if (dbg) console.log('onEventInfo');
		if (dbg) console.log(JSON.stringify(info,null,2));

		this.info = info;
		setTimeout(() => { try { this.updateSettings(); } catch (err) { if (dbg) console.log('err'+err); }  },0);
	}

	onEventConfig(cfg) {
		//if (dbg) console.log('onEventConfig');
		if (dbg) console.log(JSON.stringify(cfg,null,2));

		this.cfg = cfg;

//
// tests
//

//cfg.config.server_version = '0.8:2';
//cfg.config.server_version = '1.7:1';
//cfg.config.server_version = '0.8:1';

//cfg.config.user_security = true;
// signin err: -10,-11,-20,-21

//delete cfg.config['email'];
//delete cfg.config['license'];
//delete cfg.config.license['str'];
//cfg.config.license.str = '';
//cfg.config.license.err = -1; //invalid
//cfg.config.license.err = -2; //expired
//delete cfg.config.license.err = 0; // -1 = expired

//delete cfg['lines'];
//cfg.lines = [];


		//
		// Create display version strings
		//
		{
			const a = cfg.config.version.split(':');
			const v = a[0].split('.');
			const mjr = parseInt(v[0]),mnr = parseInt(v[1]),lic = parseInt(a[1]);
			cfg.config.version_str = mjr+'.'+mnr;

			if (cfg.config.server_version) {
				const sa = cfg.config.server_version.split(':');
				const sv = sa[0].split('.');
				const smjr = parseInt(sv[0]),smnr = parseInt(sv[1]),slic = parseInt(sa[1]);
				cfg.config.server_version_str = smjr+'.'+smnr;
			}

			if (dbg) console.log('app v'+mjr+'.'+mnr+':'+lic);
		}


		//
		// Show popup if license expired
		//
		if (false) {
			const a = cfg.config.version.split(':');
			const v = a[0].split('.');
			const mjr = parseInt(v[0]),mnr = parseInt(v[1]),lic = parseInt(a[1]);

			if (cfg.config.server_version) {
				const sa = cfg.config.server_version.split(':');
				const sv = sa[0].split('.');
				const smjr = parseInt(sv[0]),smnr = parseInt(sv[1]),slic = parseInt(sa[1]);

				if (lic < slic) {
					if (dbg) console.log('expired version, update required');

					this.resetUI();
					this.showTitle();
					new Popup().showAppUpdate(cfg.config,true);
					return;
				} else
				if (mjr < smjr) {
					if (dbg) console.log('update recommended');

					this.resetUI();
					this.showTitle();
					new Popup().showAppUpdate(cfg.config,false);
					return;
				} else
				if (mnr < smnr) {
					if (dbg) console.log('update available');
				}
			}
		}

		//
		// Security logic is simple
		//
		if (!this.signedIn) {

			//
			// signin required
			//
			if (cfg.config.user_security) {
				if (dbg) console.log('not signed in');

				//setTimeout(() => {
					Popup.closeAll();
					new Popup().showSignIn();
				//},1000);

				return;
			}

			//
			// signin disabled
			//
			this.signedIn = true;

			//setTimeout(() => {
			//	Popup.closeAll();
			//},1000);
		}


		if (false) {
			//
			// Check registered email presence
			//
			if (!cfg.config.email || 0 == cfg.config.email) {
				//
				// show email
				//
				this.showTitle();
				new Popup().showLicenseEmail(cfg.config);
				return;
			}


			//
			// Check license presence
			//
			if (!cfg.config.license || !cfg.config.license.str || 0 == cfg.config.license.str.length) {
				//
				// show license - need link to send email again
				//
				this.resetUI();
				this.showTitle();
				new Popup().showLicenseUpdate(cfg.config);
				return;
			}


			//
			// Check license errors
			//
			if (cfg.config.license.err && 0 != cfg.config.license.err) {
				this.resetUI();
				this.showTitle();
				new Popup().showLicenseUpdate(cfg.config);
				return;
			}
		}


		//
		//
		//
		if (0 == cfg.lines.length) {
			if (this.lines && 0 < this.lines.length) {
				//
				// clean up ui
				//
			}
			this.resetUI();
			this.lines = [];
			this.showTitle();
			new Popup().showLineConfig( { idx:0,cfg:{device:'',nam:'',num:''} } );
			return;
		}

		if (!this.lines || this.lines.length != Object.keys(cfg.lines).length) {
			if (dbg) console.log('lazy create lines:'+(null == cfg.lines ?0 :Object.keys(cfg.lines).length));

			this.resetUI();
			this.lines = [];

			for (let i=1;i<=2;i++) {
				const c = cfg.lines[i.toString()];
				if (null == c) continue;

				const l = new Line(i,c);
				this.lines.push(l);
			}

			this.includeLinesInUi = 1 < this.lines.length;
		}


		//
		//
		//
		setTimeout(() => { try { this.updateSettings(); } catch (err) { if (dbg) console.log('err'+err); }  },0);
		setTimeout(() => { try { this.updateActivity(); } catch (err) { if (dbg) console.log('err'+err); }  },0);



		//
		//
		//
		if (this.flagCreateUI) {
			delete this['flagCreateUI'];

			setTimeout(() => {
				this.createUI();
				this.resizeUI();
			},0);

			setTimeout(() => {
				ws.sendGetStatus();
				ws.sendGetCalls();
				ws.sendGetSkiplist();
			},0);

			setTimeout(() => {
				this.navActivity();
			},0);
		}
	}

	onEventSkiplist(list) {
		//if (dbg) console.log('onEventSkiplist '+JSON.stringify(list));

		this.skiplist = list;
		setTimeout(() => { try { this.updateSettings(); } catch (err) { if (dbg) console.log('err'+err); }  },0);

		this.skiplistNumbers = {};
		const max = this.skiplist.length;
		for (let i=0;i<max;i++) {
			const it = this.skiplist[i];
			this.skiplistNumbers[it.number] = it;
		}
	}

	onEventStatus(status) {
		if (dbg) console.log('onEventStatus');
		if (dbg) console.log(JSON.stringify(status));

		if (!this.lines) {
			if (dbg) console.log('onEventStatus lines is null');
			return;
		}

		const max = status.length;

		for (let i=0;i<max;i++) {
			const s = status[i];

			if (1 != s.line && 2 != s.line) {
				if (dbg) console.log('invalid line id:'+s.line);
				return;
			}

			const l = this.lines[s.line-1];
			if (!l) continue;

			const cid = s.cid;

			if (null != cid && (0 < cid.na.length || 0 < cid.nu.length)) {

				if ('P' === cid.nu && 'O' === cid.na) {
					cid.nu = 'Private';
					cid.na = 'Out of Area';
				}

				l.showCallerId(cid.na,cid.nu);
			}

			switch (s.status) {
				case 'ONHOOK': {
					l.showOnHook();
					l.sendRecordStop();
					l.sendListenStop();
					l.showMessage(s.status);

					if (l.hasNewCall) {
						delete l['hasNewCall'];

						ws.sendGetCalls();
					}
					break;
				}
				case 'RING': {
					l.showRing();
					l.showMessage(s.status);
					l.hasNewCall = true;
					break;
				}
				case 'OFFHOOK': {
					l.showOffHook();

					switch (s.mode) {
						case 'READY'       : l.showMessage('OFFHOOK'); break;
						case 'LENNY'       : l.showMessage(s.mode); break;
						case 'DISCONNECTED': l.showMessage(s.mode); break;
						case 'MESSAGE'     : l.showMessage(s.mode); break;
						case 'RECORD'      : l.showMessage(s.mode); break;
					}
					break;
				}
				case 'CLOSED': {
					if (dbg) console.log('closed');

					l.showClosed();
					break;
				}
			}
		}
	}

	onEventCalls(calls) {
		if (dbg) console.log('onEventCalls answered:'+calls.answered.length+' history:'+calls.history.length+' msgs:'+calls.messages.length);

		const c = (a,b) => {
			if (parseInt(a.date) < parseInt(b.date)) return -1;
			if (parseInt(a.date) > parseInt(b.date)) return  1;
			if (parseInt(a.time) < parseInt(b.time)) return -1;
			if (parseInt(a.time) > parseInt(b.time)) return  1;
			return 0;
		}  

		this.lastCalls   = calls.answered.sort(c);
		this.lastHist    = calls.history.sort(c);
		this.lastMsgs    = calls.messages;
		this.lastNewMsgs = calls.new;

		{
			const c = this.lastNewMsgs.length;
			const cnt = document.getElementById('message_count');
			if (cnt) {
				cnt.innerText = c;

				if (0 == c) {
					cnt.classList.add('hidden');
					document.title = 'Lenny - The Telemarketer Troll';
				} else {
					cnt.classList.remove('hidden');
					//document.title = 'Lenny ('+c+') - The Telemarketer Troll';
					document.title = '('+c+') Lenny - The Telemarketer Troll';
				}
			}
		}

		//
		// sync data
		//
		{
			// copy data
			const calls = JSON.parse(JSON.stringify(this.lastCalls));

			//
			// the sync
			//
			const max = this.lastMsgs.length;
			for (let i=0;i<max;i++) {
				const msg = this.lastMsgs[i];

				const date = msg.file.substr(0,8);
				const time = msg.file.substr(8,4);
				const line = msg.file.substr(15,1);

				let found = false;
				if (0 < calls.length) {
					// Sync with calls
					const mmax = calls.length;
					for (let j=0;j<mmax;j++) {
						const it = calls[j];

						if ('P' === it.number && 'O' === it.name) {
							it.number = 'Private';
							it.name = 'Out of Area';
						}

						if (this.includeLinesInUi && line != it.line) continue;
//						if (line != it.line) continue;
						if (date != it.date) continue;
						if (time != it.time) continue;
						found = true;

						it.file = msg.file;
						it.ms = msg.ms;
						break;
					}
				}

				if (0 < this.lastHist.length) {
					// Sync with history
					const mmax = this.lastHist.length;
					for (let j=0;j<mmax;j++) {
						const it = this.lastHist[j];

						if (this.includeLinesInUi && line != it.line) continue;
//						if (line != it.line) continue;
						if (date != it.date) continue;
						if (time != it.time) continue;
						found = true;

						it.file = msg.file;
						it.ms = msg.ms;
						break;
					}
				}

				if (!found) {
					//if (dbg) console.log('creating call from message f:'+msg.file);
					calls.push( { line:line,date:date,time:time,number:'unknown',name:'unknown',file:msg.file,ms:msg.ms,unknown:true } );
				}
			}

			//
			// the 'new' flag
			//
			if (this.lastNewMsgs) {
				const newm = this.lastNewMsgs;

				const max = calls.length;
				for (let i=0;i<max;i++) {
					const c = calls[i];

					const mmax = newm.length;
					for (let j=0;j<mmax;j++) {
						const n = newm[j];

						if (n.line != c.line) continue;
						if (n.date != c.date) continue;
						if (n.time != c.time) continue;

						c.new = true;
					}
				}
			}

			//
			// date sort
			//
			{
				const s = (a,b) => {
					if (parseInt(a.date) < parseInt(b.date)) return -1;
					if (parseInt(a.date) > parseInt(b.date)) return  1;
					if (parseInt(a.time) < parseInt(b.time)) return -1;
					if (parseInt(a.time) > parseInt(b.time)) return  1;
					return 0;
				}  

				this.currentCalls = 0 ==         calls.length ?[] :        calls.sort(s);
				this.currentHist  = 0 == this.lastHist.length ?[] :this.lastHist.sort(s);
			}
		}

		//
		// ui update
		//		
		setTimeout(() => { try { this.updateCalls(); } catch (err) { if (dbg) console.log('err'+err); }  },0);
		setTimeout(() => { try { this.updateHistory(); } catch (err) { if (dbg) console.log('err'+err); }  },0);
		setTimeout(() => { try { this.updateActivity(); } catch (err) { if (dbg) console.log('err'+err); }  },0);
	}


	//
	// ui update
	//
	updateActivity() {
		if (!document.getElementById('activity_items')) return;
		if (dbg) console.log('updateActivity');
		const its = document.getElementById('activity_items');
		while (0 < its.childNodes.length) its.removeChild(its.childNodes[0]);
		this.lines.forEach( (l) => { its.appendChild(l.activity); });
		if (1 >= this.lines.length) its.appendChild(this.getNewCalls());
	}

	getNewCalls() {
		if (!this.currentCalls) return;

		const calls = [];

		const max = this.currentCalls.length;
		for (let i=max-1;i>=0;i--) {
			const c = this.currentCalls[i];
			if (!c.new) continue;
			calls.push(c);
		}

		const fieldset = document.createElement('fieldset');
		fieldset.id = 'calls_new';
		const legend = document.createElement('legend');
		const span = document.createElement('span');
		legend.appendChild(span);
		fieldset.appendChild(legend);

		span.innerText = 'New Calls '+ (0 == calls.length ?'[none]' :('['+calls.length+']'));

		const tbl = document.createElement('div');
		tbl.className = 'table';
		fieldset.appendChild(tbl);

		const pctName = (100 -(this.includeLinesInUi ?5 :0) -30 -25 -8) +'%' ;

		let idx = calls.length;
		calls.forEach( (c) => {
			const tr = document.createElement('div');
			tr.className = 'tableRow';
			tbl.appendChild(tr);


			if (this.includeLinesInUi) {
				const tdLine = document.createElement('div');
				tdLine.style.width = '3%';
				tdLine.style.display = 'inline-block';
				tdLine.style.paddingLeft = '5px';
				tr.appendChild(tdLine);
				tdLine.innerText = c.line;
			}

			{
				const tdDate = document.createElement('div');
				tr.appendChild(tdDate);
				tdDate.style.width = '28%';
				tdDate.style.display = 'inline-block';
				tdDate.style.paddingLeft = '5px';
				tdDate.innerText = formatCallTimeDayOfWeek(c);
			}

			{
				const tdNum = document.createElement('div');
				tdNum.style.width = '25%';
				tdNum.style.display = 'inline-block';
				tr.appendChild(tdNum);
				tdNum .innerText = c.number;
			}
			{
				const tdName = document.createElement('div');
				tdName.style.display = 'inline-block';
				tdName.style.width = pctName;
				tr.appendChild(tdName);
				tdName.innerText = c.name;
			}

			if (c.new) {
				const td = document.createElement('div');
				tr.appendChild(td);
				td.style.display = 'inline-block';
				td.style.width = '8%';
				td.innerHTML = '<span class="new"/>';
			} else {
				const td = document.createElement('div');
				td.style.display = 'inline-block';
				td.style.width = '8%';
				tr.appendChild(td);
			}

			const cidx = --idx;
			tr.addEventListener('click',(ev) => { new Popup().showCallMessage( { c:calls,i:cidx } ); },supportsPassive);

			{
				const tr = document.createElement('div');
				tr.className = 'tableRow not_active';
				tr.style.height = '6px';
				tbl.appendChild(tr);

				const td = document.createElement('div');
				td.className = 'tableCell spacer';
				td.style.paddingTop = '0';
				td.style.paddingBottom = '0';
				tr.appendChild(td);
			}
		});

		return fieldset;
	}



	//
	//
	//
	updateCalls() {
		if (!document.getElementById('calls_items')) return;
		if (dbg) console.log('updateCalls');


		//
		//
		//
		const showHelp = () => {
			const div = document.createElement('div');
			div.className = 'help';

			document.getElementById('calls_items').appendChild(div);

			{
				const img = document.createElement('img');
				img.classList = 'top';
				img.src = 'img/lenny_toon.png';
				div.appendChild(img);
			}
			{
				const p = document.createElement('p');
				p.innerHTML = 'Calls contain all the received telephone calls and their audio messages.';
				div.appendChild(p);
			}
			{
				const p = document.createElement('p');
				p.innerHTML = 'Calls include audio messages from Lenny answering, recorded telephone answering machine messages and your recorded calls.';
				div.appendChild(p);
			}
			{
				const p = document.createElement('p');
				p.innerHTML = 'You may skiplist the Caller Id number allowing these calls to skip Lenny allowing the answering machine to pick them up.';
				div.appendChild(p);
			}
		};


		if (!this.currentCalls) return;
		const calls = this.currentCalls;

		//
		// Build sort by days
		//

		const sh = getDaysFromCalls(calls);


		//
		//
		//
		const cnt = document.getElementById('calls_count');
		cnt.innerText = 0 == calls.length ?'[none]' :('['+calls.length+']');

		const d = document.getElementById('calls_items');
		while (0 < d.childNodes.length) d.removeChild(d.childNodes[0]);


		//
		//
		//
		if (0 == calls.length) {
			showHelp();
		} else {
			// days
			const ds = [];
			for (let d in sh) { ds.push(d); }


			//
			//
			//
			const pctName = (100 -(this.includeLinesInUi ?5 :0) -30 -25 -8) +'%' ;

			const tbl = document.createElement('div');
			tbl.className = 'table';
			d.appendChild(tbl);

			let idx = 0;
			const dmax = ds.length;
			for (let i=dmax-1;i>=0;i--) {
				const tag = ds[i];

				const yr = tag.substr(0,4),mo = tag.substr(4,2),da = tag.substr(6,2);

				const tr = document.createElement('div');
				tr.className = 'tableRow';
				tbl.appendChild(tr);
		
				const th = document.createElement('div');
				th.className = 'tableHeader';
				th.style.width = '100%';
//				th.style.backgroundColor = '#eee'
				tr.appendChild(th);
				th.innerText = formatDateMMDD(yr,mo,da);

				const dcalls = sh[ ds[i] ];
				const cmax = dcalls.length;
				for (let j=0;j<cmax;j++,idx++) {
					const c = dcalls[j];

					const tr = document.createElement('div');
					tr.className = 'tableRow';
					tbl.appendChild(tr);

					if (this.includeLinesInUi) {
						const tdLine = document.createElement('div');
						tdLine.style.width = '2%';
						tdLine.style.display = 'inline-block';
						tdLine.style.paddingLeft = '5px';
						tr.appendChild(tdLine);
						tdLine.innerText = c.line;
					}

					{
						const tdDate = document.createElement('div');
						tr.appendChild(tdDate);
						tdDate.style.width = '28%';
						tdDate.style.display = 'inline-block';
						tdDate.style.paddingLeft = '5px';
						tdDate.innerText = formatCallDateTime(c);
					}

					{
						const tdNum = document.createElement('div');
						tdNum.style.width = '25%';
						tdNum.style.display = 'inline-block';
						tr.appendChild(tdNum);
						tdNum .innerText = c.number;
					}
					{
						const tdName = document.createElement('div');
						tdName.style.display = 'inline-block';
						tdName.style.width = pctName;
						tr.appendChild(tdName);
						tdName.innerText = c.name;
					}

					if (c.new) {
						const td = document.createElement('div');
						tr.appendChild(td);
						td.style.display = 'inline-block';
						td.style.width = '8%';
						td.innerHTML = '<span class="new"/>';
					} else {
						const td = document.createElement('div');
						td.style.display = 'inline-block';
						td.style.width = '8%';
						tr.appendChild(td);
					}

					const cidx = idx;
					tr.addEventListener('click',(ev) => { new Popup().showCallMessage( { c:calls,i:cidx } ); },supportsPassive);

					{
						const tr = document.createElement('div');
						tr.className = 'tableRow not_active';
						tr.style.height = '6px';
						tbl.appendChild(tr);

						const td = document.createElement('div');
						td.className = 'tableCell spacer';
						td.style.paddingTop = '0';
						td.style.paddingBottom = '0';
						tr.appendChild(td);
					}

				}
			}
		}
	}

	//
	//
	//
	updateHistory() {
		if (!document.getElementById('history_items')) return;
		if (dbg) console.log('updateHistory');



		//
		//
		//
		const showHelp = () => {
			const div = document.createElement('div');
			div.className = 'help';

			document.getElementById('history_items').appendChild(div);

			{
				const img = document.createElement('img');
				img.classList = 'top';
				img.src = 'img/lenny_toon.png';
				div.appendChild(img);
			}
			{
				const p = document.createElement('p');
				p.innerHTML = 'History contains all the trash from the Calls section.  Removing calls from History is permenant and will also free up valuable disk space.';
				div.appendChild(p);
			}
			{
				const p = document.createElement('p');
				p.innerHTML = 'Before final removal, you may still skiplist the Caller Id number allowing these calls to skip Lenny allowing the answering machine to pick them up.';
				div.appendChild(p);
			}
		};



		//
		const hist = this.currentHist;

		//
		// Build sort by days
		//
		const sh = getDaysFromCalls(hist);



		//
		//
		//
		const selectAll = document.getElementById('history_selectall_but');

		let selectedAll = false;
		selectAll.onclick = (ev) => {
			const l = document.getElementsByClassName('trash');
			selectedAll = !selectedAll;
			const max = l.length;
			for (let i=0;i<max;i++) l[i].checked = selectedAll;
			selectAll.value = selectedAll ?'Unselect All' :'Select All';
			deleteAllUpdate();
		};

		//
		//
		//
		const deleteAll = document.getElementById('history_delete_but');
		deleteAll.style.display = 'none';

		const deleteAll_save = 'inline-block';
		deleteAll.onclick = (ev) => {
			const d = [];

			{
				const l = document.getElementsByClassName('trash');

				// build list
				{
					const max = l.length;
					for (let i=0;i<max;i++) {
						if (!l[i].checked) continue;
						const ck = l[i];
						d.push(ck.msg);
					}
				}

				// confirm with count
				if (!confirm('Remove '+d.length+' message(s) permanently. Are you sure?')) return;
				if (dbg) console.log('deleteAll confirmed l:'+d.length);

				// ui update control
				deleteAll.style.display = 'none';
				selectAll.value = 'Select All';
				selectAll.classList.add('hidden');

				// ui process
				if (l.length == d.length) {
					this.currentHist = [];
				} else {

					let nhist = hist;
					const max = l.length;
					for (let i=max-1;i>=0;i--) {
						if (!l[i].checked) continue;
						const idx = l[i].msgIdx;
						nhist = nhist.slice(0,idx).concat(nhist.slice(idx +1));
					}

					this.currentHist = nhist;
				}

				// finish ui update
				lenny.updateHistory();
			}

			ws.sendDeleteMessage(d);
		};

		const deleteAllUpdate = () => {

			{
				let cnt = 0;

				const l = document.getElementsByClassName('trash');
				const max = l.length;
				for (let i=0;i<max;i++) { if (l[i].checked) cnt++; }

				if (dbg) console.log('deleteAllUpdate cnt:'+cnt);

				if (0 == cnt) {
					deleteAll.style.display = 'none';
				} else {
					deleteAll.style.display = deleteAll_save;
				}
			}


			//
			//
			//
			this.resizeUI_HistoryButtons(0 != Object.keys(sh).length);

			if (0 == Object.keys(sh).length) {
				selectAll.classList.add('hidden');
			} else {
				selectAll.classList.remove('hidden');
			}
		};



		//
		//
		//

		const cnt = document.getElementById('history_count');
		cnt.innerText = 0 == hist.length ?'[none]' :('['+hist.length+']');

		const d = document.getElementById('history_items');
		while (0 < d.childNodes.length) d.removeChild(d.childNodes[0]);


		//
		//
		//
		if (0 == Object.keys(sh).length) {
			showHelp();
		} else {
			// days
			const ds = [];
			for (let d in sh) { ds.push(d); }

			
			//
			//
			//
			const pctName = (100 -5 -(this.includeLinesInUi ?5 :0) -30 -25) +'%' ;

			const tbl = document.createElement('div');
			tbl.className = 'table';
			d.appendChild(tbl);

			let idx = 0;
			const dmax = ds.length-1;
			for (let i=dmax;i>=0;i--) {

				const tr = document.createElement('div');
				tr.className = 'tableRow';
				tbl.appendChild(tr);

				const th = document.createElement('div');
				th.className = 'tableHeader';
				th.style.width = '100%';
//				th.style.backgroundColor = '#eee'
				tr.appendChild(th);
				th.innerText = formatHistDate(ds[i]);


				//
				//
				//
				const calls = sh[ ds[i] ];
				const cmax = calls.length;
				for (let j=0;j<cmax;j++,idx++) {
					const c = calls[j];

					const tr = document.createElement('div');
					tr.className = 'tableRow';
					tbl.appendChild(tr);

					{
						const tdCheck = document.createElement('div');
						tdCheck.className = 'trash';
						tdCheck.style.width = '5%';
						tdCheck.style.display = 'inline-block';
						tdCheck.style.marginLeft = '10px';
						tr.appendChild(tdCheck);
						{
							const ck = document.createElement('input');
							tdCheck.appendChild(ck);

							ck.type = 'checkbox';
							ck.style = 'transform:scale(1.5)';
							ck.className = 'trash';
							ck.msg = c;
							ck.msgIdx = idx;

							ck.addEventListener('click',(ev) => {
								if (dbg) console.log('ck click');
								ev.stopPropagation();
								deleteAllUpdate();
							});

							tdCheck.addEventListener('click',(ev) => {
								if (dbg) console.log('td click');
								ck.checked = !ck.checked;
								ev.stopPropagation();
								deleteAllUpdate();
							});
						}
					}

					if (this.includeLinesInUi) {
						const tdLine = document.createElement('div');
						tdLine.style.width = '3%';
						tdLine.style.display = 'inline-block';
						tdLine.style.paddingLeft = '5px';
						tr.appendChild(tdLine);
						tdLine.innerText = c.line;
					}

					{
						const tdDate = document.createElement('div');
						tdDate.style.width = '26%';
						tdDate.style.display = 'inline-block';
						tdDate.style.paddingLeft = '5px';
						tr.appendChild(tdDate);
						tdDate.innerText = formatCallDateTime(c);
					}

					{
						const tdNum = document.createElement('div');
						tdNum.style.width = '25%';
						tdNum.style.display = 'inline-block';
						tr.appendChild(tdNum);
						tdNum.innerText = c.number;
					}
					{
						const tdName = document.createElement('div');
						tdName.style.width = pctName;
						tdName.style.display = 'inline-block';
						tr.appendChild(tdName);
						tdName.innerText = c.name;
					}

					const cidx = idx;
					tr.addEventListener('click',(ev) => { new Popup().showCallMessage( { c:hist,i:cidx },true ); },supportsPassive);

					{
						const tr = document.createElement('div');
						tr.className = 'tableRow not_active';
						tr.style.height = '6px';
						tbl.appendChild(tr);

						const td = document.createElement('div');
						td.className = 'tableCell spacer';
						td.style.paddingTop = '0';
						td.style.paddingBottom = '0';
						tr.appendChild(td);
					}
				}
			}
		}

		deleteAllUpdate();
	}


	//
	//
	//
	updateSettings() {
		if (!document.getElementById('settings_items')) return;
		if (dbg) console.log('updateSettings');

		const settings = document.getElementById('settings_items');
		if (!settings) return;

		//
		// Info
		//

		const addAppComment = () => {

			{
				const info = document.createElement('fieldset');

				const legend = document.createElement('legend');
				const span = document.createElement('span');
				legend.appendChild(span);
				info.appendChild(legend);
				settings.appendChild(info);

				span.innerText = 'Support Lenny';

				//
				const d = document.createElement('div');
				d.id = 'settings_info';
				info.appendChild(d);

				const fr = document.createElement('div');
				fr.id = 'sw_update';
				d.appendChild(fr);

				{
					const but = document.createElement('button');
					fr.appendChild(but);
					but.style.margin = '10px';
					but.innerHTML = 'Rate &amp; Comment on the Lenny App';

					but.onclick = (ev) => {
						ev.stopPropagation();
						new Popup().showSendComment();
					};
				}

				{
					const but = document.createElement('button');
					fr.appendChild(but);
					but.style.margin = '10px';
					but.innerHTML = 'Report a Lenny Defect';

					but.onclick = (ev) => {
						ev.stopPropagation();
						new Popup().showSendBugReport();
					};
				}

				{
					const but = document.createElement('button');
					fr.appendChild(but);
					but.style.margin = '10px';
					but.innerHTML = 'Show Lenny Tour';

					but.onclick = (ev) => {
						ev.stopPropagation();
						new Popup().showTour();
					};
				}
			}
		};

		const addUpdate = () => {

			{
				const info = document.createElement('fieldset');

				const legend = document.createElement('legend');
				const span = document.createElement('span');
				legend.appendChild(span);
				info.appendChild(legend);
				settings.appendChild(info);

				span.innerText = 'Software Update';

				//
				const d = document.createElement('div');
				d.id = 'settings_info';
				info.appendChild(d);

				const fr = document.createElement('div');
				fr.id = 'sw_update';
				d.appendChild(fr);

				let hasUpdate = true;

				{
	//				const dd = document.createElement('div');
	//				dd.id = 'sw_update_info';
	//				fr.appendChild(dd);

					{
						const a = this.cfg.config.version.split(':');
						const v = a[0].split('.');
						const mjr = parseInt(v[0]),mnr = parseInt(v[1]),lic = parseInt(a[1]);

						if (!this.cfg.config.server_version) {
							hasUpdate = false;
							let s = '<div class="green">No updates available for v'+this.cfg.config.version_str+'</div>';
							fr.innerHTML = s;
						} else {
							const sa = this.cfg.config.server_version.split(':');
							const sv = sa[0].split('.');
							const smjr = parseInt(sv[0]),smnr = parseInt(sv[1]),slic = parseInt(sa[1]);

							if (lic < slic) {
								let s = '<div class="red">Retired version v'+this.cfg.config.version_str+', an update is required</div>';
								s += '<br/>';
								s += 'Please <a href="http://lennytroll.com#download" target="lenny">download</a> the latest version: v'+this.cfg.config.server_version_str+' is available.'
								fr.innerHTML = s;
							} else
							if (mjr < smjr) {
								let s = '<div class="yellow">Deprecated version v'+this.cfg.config.version_str+', an update is required</div>';
								s += '<br/>';
								s += 'Please <a href="http://lennytroll.com#download" target="lenny">download</a> the latest version: v'+this.cfg.config.server_version_str+' is available.'
								fr.innerHTML = s;
							} else
							if (mnr < smnr) {
								let s = '<div class="yellow">Older version v'+this.cfg.config.version_str+', an update is available</div>';
								s += '<br/>';
								s += 'Please <a href="http://lennytroll.com#download" target="lenny">download</a> the latest version: v'+this.cfg.config.server_version_str+' is available.'
								fr.innerHTML = s;
							} else {
								hasUpdate = false;
								let s = '<div class="green">No updates available for v'+this.cfg.config.version_str+'</div>';
								fr.innerHTML = s;
							}
						}
						if (this.cfg.config.server_check_last) {
							const formatTimeShort = (d) => {
								let s = d.getHours();
								s += ':';
								if (10 > d.getMinutes()) s += '0';
								s +=  d.getMinutes();
								s += ':';
								if (10 > d.getSeconds()) s += '0';
								s +=  d.getSeconds();
								return s;
							};
							const d = new Date(this.cfg.config.server_check_last);
							let s = '<div style="display:block;font-size:10px">Last checked: '+formatDateShort(d)+' @ '+formatTimeShort(d)+'</div>';
							fr.innerHTML += s;
						}
					}
				}

				if (!hasUpdate) {
					const but = document.createElement('input');
					fr.appendChild(but);

					but.type = 'button';
					but.style.float = 'right';
					but.style.marginRight = '5px';
					but.value = 'Check for Updates ...';

					but.onclick = (ev) => {
						ev.stopPropagation();

						if (dbg) console.log('update check');
						ws.sendUpdateCheck();
					};
				}

			}
		};

		const addLicense = () => {
			{
				const info = document.createElement('fieldset');

				const legend = document.createElement('legend');
				const span = document.createElement('span');
				legend.appendChild(span);
				info.appendChild(legend);
				settings.appendChild(info);

				span.innerText = 'License Key';

				//
				const d = document.createElement('div');
				d.id = 'settings_info';
				info.appendChild(d);

				const fr = document.createElement('div');
				fr.id = 'license';
				d.appendChild(fr);



				//
				//
				//
				const tfr = document.createElement('div');
				tfr.id = 'license_email';
				fr.appendChild(tfr);

				const lfr = document.createElement('div');
				lfr.id = 'license_info';
				fr.appendChild(lfr);



				//
				//
				//
				{
					const dd = document.createElement('div');
					tfr.appendChild(dd);

					const p = document.createElement('div');
					p.style.display = 'inline-block';
					p.style.fontVariant = 'small-caps';

					p.innerText = 'Evaluation Beta License';
					dd.appendChild(p);

				/*
					const rd = document.createElement('div');
					rd.style.display = 'inline-block';
					dd.appendChild(p);
					dd.appendChild(rd);

					rd.style.float = 'right';
					rd.style.right = '10px';

					//
					const rbut = document.createElement('button');
					rbut.className = 'email';
					rd.appendChild(rbut);

					rbut.onclick = (ev) => {
						if (dbg) console.log('license click');
						new Popup().showLicenseEmail(this.cfg.config);
					};

					//
					//
					//
					if (!this.cfg.config.email || 0 == this.cfg.config.email.length) {
						p.innerText = 'Get a FREE 30-day evaluation license!';
						
						rbut.className = 'green';
						rbut.style.fontVariant = 'small-caps';
						rbut.style.fontSize = '20px';
						rbut.innerText = 'Email a new License Key now';
					} else {
						p.innerText = 'Registered email:';
						rbut.style.fontVariant = 'normal';
						rbut.innerText = this.cfg.config.email;
					}
				*/

				}



				//
				//
				//
				if (lenny.cfg.config.email && 0 < lenny.cfg.config.email.length) {
					const but = document.createElement('button');
					but.className = 'license_key';
					lfr.appendChild(but);

					but.onclick = (ev) => {
						if (dbg) console.log('license click');
					//	ws.sendCreateLicense();
						new Popup().showLicenseUpdate(lenny.cfg.config);
					};


					if (!this.cfg.config.license || 0 == this.cfg.config.license.str.length) {
						but.classList.add('green');
						but.innerText = 'New License Key';
					} else {

						if (0 > this.cfg.config.license.err) {
							but.style.fontFamily = 'mono';
							but.classList.add('green');
							but.innerText = this.cfg.config.license.str;

						} else {
							but.style.fontFamily = 'mono';
							but.classList.add('green');
							but.innerText = this.cfg.config.license.str;

							{
								const date = this.cfg.config.license.date;
								const d = new Date(date.substr(0,4),date.substr(5,2)-1,date.substr(8,2));
								const ex = new Date(d); ex.setDate(new Date(d.getDate() -5));
								const expiresSoon = ex < new Date();
								if (expiresSoon) {
									but.style.color = '#ff0';
								}

								const dd = document.createElement('div');
								dd.style.fontVariant = 'small-caps';
								lfr.appendChild(dd);

								let r = '';
								r += (this.cfg.config.license.eval ?'Evaluation v' :'Release v') + this.cfg.config.license.version;
								r += '<br/>';
								r += (!expiresSoon ?'Expires: ' :'Expires soon: ') + formatDateShort(d)+', '+d.getFullYear();
								dd.innerHTML = r;
							}
						}
					}
				}
			}
		};

		const addOSInfo = () => {

			{
				const info = document.createElement('fieldset');

				const legend = document.createElement('legend');
				const span = document.createElement('span');
				legend.appendChild(span);
				info.appendChild(legend);
				settings.appendChild(info);

				span.innerText = 'OS Host';

				//
				const d = document.createElement('div');
				d.id = 'settings_info';
				info.appendChild(d);


				//
				// todo: table --> div
				//
//	div.download_contact { display:flex; }
//	div.download_contact_txt { flex:1; }
//	div.download_contact_side { flex:0 0 240px; }

//				const tbl = document.createElement('div');
//				d.appendChild(tbl);

				//const tr = tbl.insertRow();
				const tr = document.createElement('div');
				tr.style.display = 'flex';
				d.appendChild(tr);
//				tbl.appendChild(tr);

				{
//					const td = tr.insertCell();
					const td = document.createElement('div');
					tr.appendChild(td);
//					td.style = 'display:inline-block';
					const img = document.createElement('img');
					img.style.width = Math.min(100,Math.floor(window.innerWidth /4)) +'px';//'didisplay:inline-block';
					img.src = 'img/logo_'+this.info.os.logo+'.png';
					td.appendChild(img);
				}
				{
//					const td = tr.insertCell();
//					const td = document.createElement('div');
					const td = document.createElement('div');
					tr.appendChild(td);
//					td.style = 'flex-grow:100';
//					td.style = 'display:inline-block;margin-left:20px';
					td.style = 'margin-left:20px';
					const dd = document.createElement('div');
					td.appendChild(dd);

					dd.id = 'settings_info_info';

					//
					const format = (t) => {
						if (1000000000 < t) return (t /1000000000).toFixed(1) +'g';
						if (1000000    < t) return (t /1000000   ).toFixed(1) +'m';
						                    return (t /1000      ).toFixed(1) +'k';
					}
					const plural = (v,s) => { return v.toFixed(0)+' '+s+(1 == v ?'' :'s'); }
					const formatTime = (t) => {
						const w = Math.floor(t/(7*24*60*60)) ,d=Math.floor(t/(24*60*60))%7;
						const h = Math.floor(t/(60*60))%24   ,m=Math.floor(t/60)%60 ,s=t%60;

						if (       60  >t) return plural(t,'second');
						if (     60*60 >t) return plural(m,'minute')+', '+plural(s,'second');
						if (  24*60*60 >t) return plural(h,'hour'  )+', '+plural(m,'minute');
						if (7*24*60*60 >t) return plural(d,'day'   )+', '+plural(h,'hour'  )+', '+plural(m,'minute');

						let r = '';
						if (0 < w) r += plural(w,'week' ) +', ';
						if (0 < d) r += plural(d,'day'  ) +', ';
						if (0 < h) r += plural(h,'hour' ) +', ';
						r += plural(m,'minute');
						return r;

					//	return plural(w,'week'  )+', '+plural(d,'day'   )+', '+plural(h,'hour'  )+', '+plural(m,'minute');
					}

					const dtotal = format(this.info.space.disk.block_size  * this.info.space.disk.total);
					const mtotal = format(this.info.space.memory.page_size * this.info.space.memory.total);
					const dfree = format(this.info.space.disk.block_size  * this.info.space.disk.free);
					const mfree = format(this.info.space.memory.page_size * this.info.space.memory.free);

					const mfreePct = (100 *(this.info.space.memory.free / this.info.space.memory.total)).toFixed(1) +'%';
					const dfreePct = (100 *(this.info.space.disk.free / this.info.space.disk.total)).toFixed(1) +'%';

					let str =
						this.info.os.issue+'<br/>' +
						this.info.os.uname+'<br/>' +
						'OS uptime: '  +this.info.os.uptime+'<br/>' +
						'Available Memory: ' +mfreePct +' - '+mfree+' / '+mtotal+'<br/>' +
						'Available Disk: ' +dfreePct +' - '+dfree+' / '+dtotal+'<br/>' +
						'<br/>' +
						'Home: '    +this.info.app.pwd+'<br/>' +
						'App uptime: ' +formatTime(this.info.app.uptime)+'<br/>' +
						'<br/>' +
						'Hostname: '+this.info.network.hostname+'<br/>';
					;

					for (let i=0,m=this.info.network.local.length;i<m;i++) {
						str += 'Local Ip: '+this.info.network.local[i].ip;
						//str += ' '+this.info.network.local[i].mac;
						str += '<br/>';
					}

					let showCreate = true;
					if (this.cfg.config.certificate && this.cfg.config.certificate.type) {
						// 'YYYY-MM-DD HH:mm:ss GMT'
						const d = this.cfg.config.certificate.to;
						const exp = Date.UTC(d.substr(0,4),d.substr(5,2)-1,d.substr(8,2),d.substr(11,2),d.substr(14,2),0);
						const dexp = new Date(exp);
						const now = Date.now();

						let color;
						if (exp < now) {
							color = '#f00';
						} else
						if (exp + 7 *24 * 60 * 60 *1000 < now) {
							color = '#aa0';
						} else {
							color = '#0a0';
							showCreate = false;
						}

						str += 'WebCert: <a href="/etc/lenny.pem" style="color:'+color+'">';
						str += this.cfg.config.certificate.type +'bit expires: ' +formatDateShort(dexp)+', '+dexp.getFullYear();
						str += '</a>';
						str += '<br/>';
						str += '<br/>';
					}
					if (showCreate) {
						const but = document.createElement('input');
						td.appendChild(but);

						but.type = 'button';
						but.value = 'Create New Certificate';
						but.style = 'float:right;right:10px;margin-top:10px;';
						but.onclick = (ev) => { ev.stopPropagation(); ws.sendCreateCertificate(); };
					}

					dd.innerHTML = str;
				}
			}
		};




		//
		// Options
		//
		const addAppOptions = () => {

			if (!this.hasOwnProperty('user_security_fieldset')) {

				//
				// todo: table --> div
				//
				const tbl = document.createElement('table');
				tbl.id = 'settings_options';

				const addTheme = () => {
					const dark = document.getElementById('theme_lite').disabled;

					const enabled = document.createElement('input');
					enabled.type = 'checkbox';
					enabled.checked = !dark;

					const span_sw = document.createElement('span');
					const span_sl = document.createElement('span');
					span_sw.className = 'switch';
					span_sl.className = 'slider round';
					span_sw.appendChild(enabled);
					span_sw.appendChild(span_sl);

					const tr = tbl.insertRow();
					const td = tr.insertCell();
					td.appendChild(span_sw);

					const span = document.createElement('span');
					span.innerText = (dark ?'DARK' :'LITE') +' COLOR THEME';
					td.appendChild(span);

					span_sw.parentElement.addEventListener('click',(ev) => {
						ev.stopPropagation();

						const dark = !document.getElementById('theme_lite').disabled;
						if (!dark) {
							document.getElementById('theme_dark').disabled = true;
							document.getElementById('theme_lite').disabled = false;
							localStorage.removeItem('theme');

							span.innerText = 'LITE COLOR THEME';
							enabled.checked = true;
						} else {
							document.getElementById('theme_dark').disabled = false;
							document.getElementById('theme_lite').disabled = true;
							localStorage.setItem('theme','dark');

							span.innerText = 'DARK COLOR THEME';
							enabled.checked = false;
						}

					},supportsPassive);
				};

				const addDebug = () => {
					const enabled = document.createElement('input');
					enabled.type = 'checkbox';
					enabled.checked = dbg;

					const span_sw = document.createElement('span');
					const span_sl = document.createElement('span');
					span_sw.className = 'switch';
					span_sl.className = 'slider round';
					span_sw.appendChild(enabled);
					span_sw.appendChild(span_sl);

					const tr = tbl.insertRow();
					const td = tr.insertCell();
					td.appendChild(span_sw);

					const span = document.createElement('span');
					span.innerText = dbg ?'LOGGING VERBOSE' :'LOGGING ERRORS ONLY';
					td.appendChild(span);

					span_sw.parentElement.addEventListener('click',(ev) => {
						ev.stopPropagation();
						console.log('logging '+(dbg?'stopped':'started'));

						dbg = !dbg;
						localStorage.setItem('dbg',dbg);

						span.innerText = dbg ?'LOGGING VERBOSE' :'LOGGING ERRORS ONLY';
						enabled.checked = dbg;

						ws.sendSetConfig(0,'log_verbosity',dbg?10:2);
						if (!dbg) ws.sendLogUpload();

					},supportsPassive);
				};

				const addSecurity = () => {
					this.user_security_enabled = document.createElement('input');
					this.user_security_enabled.type = 'checkbox';
					this.user_security_enabled.checked = this.cfg.config.user_security;

					const span_sw = document.createElement('span');
					const span_sl = document.createElement('span');
					span_sw.className = 'switch';
					span_sl.className = 'slider round';
					span_sw.appendChild(this.user_security_enabled);
					span_sw.appendChild(span_sl);

					const tr = tbl.insertRow();
					const td = tr.insertCell();
					td.appendChild(span_sw);

					const span = document.createElement('span');
					span.innerText = !this.user_security_enabled.checked ?'LOGIN NOT REQUIRED' :'LOGIN REQUIRED';
					this.user_security_txt = span;
					td.appendChild(span);

					span_sw.parentElement.addEventListener('click',(ev) => {
						ev.stopPropagation();

						if (!this.user_security_enabled.checked) {
							if (!confirm('Future access will require a valid User Signin?  Are you sure?')) return;
						}

						this.user_security_enabled.checked = !this.user_security_enabled.checked;
						span.innerText = !this.user_security_enabled.checked ?'LOGIN NOT REQUIRED' :'LOGIN REQUIRED';

						this.toggleUserSecurity();
					},supportsPassive);
				};

				const addSkiplist = () => {
					this.settings_skiplist_enabled = document.createElement('input');
					this.settings_skiplist_enabled.type = 'checkbox';
					this.settings_skiplist_enabled.checked = this.cfg.config.skiplist_enabled;

					const span_sw = document.createElement('span');
					const span_sl = document.createElement('span');
					span_sw.className = 'switch';
					span_sl.className = 'slider round';
					span_sw.appendChild(this.settings_skiplist_enabled);
					span_sw.appendChild(span_sl);

					const tr = tbl.insertRow();
					const td = tr.insertCell();
					td.appendChild(span_sw);

					const span = document.createElement('span');
					span.innerText = 'SKIPLIST ' +(this.settings_skiplist_enabled.checked ?'ACTIVE' :'NOT ACTIVE');
					this.settings_skiplist_txt = span;
					td.appendChild(span);

					{
						const div = document.createElement('div');
						td.appendChild(div);

						div.id = 'settings_ogm';

						this.settings_skiplist_but = document.createElement('input');
						div.appendChild(this.settings_skiplist_but);

						this.settings_skiplist_but.type = 'button';
						this.settings_skiplist_but.value = 'Manage ('+(!this.skiplist ?0 :this.skiplist.length) +')';

						if (!this.cfg.config.skiplist_enabled) {
							this.settings_skiplist_but.classList.add('hidden');
						} else {
							this.settings_skiplist_but.classList.remove('hidden');
						}

						this.settings_skiplist_but.onclick = (ev) => { ev.stopPropagation(); new Popup().showSkiplist(this.skiplist); };
					}

					span_sw.parentElement.addEventListener('click',(ev) => { this.toggleSkiplist(); },supportsPassive);
				};


				//
				// Layout
				//
				{
					const opts = document.createElement('fieldset');
					this.user_security_fieldset = opts;

					const legend = document.createElement('legend');
					const span = document.createElement('span');
					span.innerText = 'Options';
					legend.appendChild(span);
					opts.appendChild(legend);

					opts.appendChild(tbl);
	
					settings.appendChild(opts);
				}

				//
				//
				//
				addSkiplist();
				addSecurity();
				addTheme();
				addDebug();

			} else {
				settings.appendChild(this.user_security_fieldset);
			}
		};


		const updateSettings = () => {
			this.user_security_enabled.checked = this.cfg.config.user_security;
			this.user_security_txt.innerText = !this.user_security_enabled.checked ?'LOGIN NOT REQUIRED' :'LOGIN REQUIRED';

			this.settings_skiplist_enabled.checked = this.cfg.config.skiplist_enabled;
			this.settings_skiplist_txt.innerText = 'SKIPLIST ' +(this.settings_skiplist_enabled.checked ?'ACTIVE' :'NOT ACTIVE');

			this.settings_skiplist_but.value = 'Manage ('+(!this.skiplist ?0 :this.skiplist.length) +')';
			if (!this.cfg.config.skiplist_enabled) {
				this.settings_skiplist_but.classList.add('hidden');
			} else {
				this.settings_skiplist_but.classList.remove('hidden');
			}
		};

		//
		// Update lines
		//
		const addLines = () => {
			const l = document.getElementsByClassName('line');

//
// FIXME no lines - display a message
//

			if (l.length != this.lines.length) {
				this.lines.forEach( (l) => {
					if (!settings) return;

					l.updateSettingsView();
					settings.appendChild(l.settings);
				});

			} else {
				for (let i=1;i<=2;i++) {
					const c = this.cfg.lines[i.toString()];
					if (null == c) continue;

					const l = this.lines[i-1];
					l.cfg = c;
					l.updateSettingsView();
				}
			}
		};


		while (0 < settings.childNodes.length) settings.removeChild(settings.childNodes[0]);

		if (!this.info || !this.cfg) return;

		addLines();
		addAppOptions();
		addUpdate();
		addLicense();
		addAppComment();
		addOSInfo();

		updateSettings();
	}

}





//
//
//

class Line {
	constructor(idx,cfg) {
		this.idx = idx;
		this.cfg = cfg;

		this.listenAudio = [];

		this.createActivityView();
		this.createSettingsView();
		this.updateSettingsView();
		this.showOnHook();
	}



	//
	//
	//
	showClosed() {
		if (dbg) console.log('showClosed');
		this.showOnHook();
		this.showMessage('CLOSED');
		this.callRecord.classList.add('hidden');
	}
	showOnHook() {
		if (dbg) console.log('showOnHook');
		this.status.innerText = 'ONHOOK';
		this.tdNam.innerText = '';
		this.tdNum.innerText = '';

		this.call_state.classList.add('hidden');

		this.call_actions       .classList.add('hidden');

		this.action_onhook      .classList.add('hidden');

	//	this.action_offhook     .classList.add('hidden');
	//	this.action_lenny       .classList.add('hidden');
	//	this.action_disconnected.classList.add('hidden');
	//	this.action_messages    .classList.add('hidden');

		this.answer_actions.classList.remove('open');
		this.answer_actions.classList.add('gone');

		this.callRecord.classList.remove('hidden');
		this.showRecording(false);
	}
	showRing() {
		if (dbg) console.log('showRing');
		this.status.innerText = 'RING';
	}
	showCallerId(nam,num) {
		if (dbg) console.log('showCallerId');
		this.tdNam.innerText = nam;
		this.tdNum.innerText = num;

		this.call_state.classList.remove('hidden');

//		this.call_actions       .classList.remove('hidden');

		this.action_onhook      .classList.add(   'hidden');
		this.action_listen      .classList.add(   'hidden');

	//	this.action_offhook     .classList.remove('hidden');
	//	this.action_lenny       .classList.remove('hidden');
	//	this.action_disconnected.classList.remove('hidden');
	//	this.action_messages    .classList.remove('hidden');

		this.answer_actions.classList.remove('gone');
		this.answer_actions.classList.add('open');
	}
	showOffHook() {
		if (dbg) console.log('showOffHook');
		this.status.innerText = 'OFFHOOK';

		this.call_actions       .classList.remove('hidden');

	//	this.action_onhook      .classList.remove('hidden');
	//	this.action_offhook     .classList.add(   'hidden');

		this.answer_actions.classList.remove('open');
		this.answer_actions.classList.add('gone');
	}
	showMessage(msgRaw) {
		const msg = msgRaw.toLowerCase();
		if (dbg) console.log('showMessage '+msg);

		this.status.innerText = msg.toUpperCase();

		if ('start' === msg.substring(0,5)) {
			this.callRecord.classList.remove('hidden');
		} else
		if ('disconnected' === msg) {
			this.action_onhook.classList.remove('hidden');
			this.callRecord.classList.add('hidden');
		} else
		if ('message' === msg) {
			this.action_onhook.classList.remove('hidden');
			this.action_listen.classList.remove('hidden');
			this.callRecord.classList.add('hidden');
		} else
		if ('lenny' === msg) {
			this.action_onhook.classList.remove('hidden');
			this.action_listen.classList.remove('hidden');
			this.callRecord.classList.add('hidden');
		} else {
			this.action_onhook.classList.remove('hidden');
			this.action_listen.classList.remove('hidden');
			this.callRecord.classList.remove('hidden');
		}

		this.answer_actions.classList.remove('open');
		setTimeout(() => { this.answer_actions.classList.add('gone'); },300);
	}

	toggleRecording() {
		if (!this.recordingStart) {
			this.recordingStart = new Date();
			this.sendRecordToggle(true);
			this.showRecording(true);
		} else {
			this.sendRecordToggle(false);
			this.showRecording(false);
		}
	}
	showRecording(enabled) {
		if (enabled) {
			this.callRecordButton.innerHTML = '&#x2589; STOP';
			this.status.innerHTML = '&#x1F3A4; RECORDING';
		} else {
			this.callRecordButton.innerHTML = '<span class="red">&#x1F3A4; RECORD NOW</span>';
			//this.status.innerHTML = '&#x2589; STOPPED';
			delete this['recordingStart'];
		}
	}


	//
	//
	//
	updateSettingsView() {
		this.input_number              .value   = this.cfg.number;
		this.input_lenny_enabled       .checked = this.cfg.lenny_enabled;
		this.input_disconnected_enabled.checked = this.cfg.disconnected_enabled;
		this.input_tad_enabled         .checked = this.cfg.tad_enabled;

		if (!this.cfg.lenny_enabled) {
			this.settings_profile_but.classList.add('hidden');
		} else {
			this.settings_profile_but.classList.remove('hidden');
			this.settings_profile_but.value = this.cfg.lenny_profile +' Profile';
		}
		if (!this.cfg.disconnected_enabled) {
			this.settings_disconnected_but.classList.add('hidden');
		} else {
			this.settings_disconnected_but.classList.remove('hidden');
		}
		if (!this.cfg.tad_enabled) {
			this.settings_ogm_but.classList.add('hidden');
		} else {
			this.settings_ogm_but.classList.remove('hidden');
		}
	}


	//
	//
	//

	createActivityView() {

		if (!this.activity) {

			this.activity = document.createElement('fieldset');
			this.activity.className = 'activity_line';

			{
				this.name = document.createElement('legend')
				this.activity.appendChild(this.name);

				const span = document.createElement('span')
				span.innerText = 'LINE '+this.idx;
				this.name.appendChild(span);
			}

			{
				const div = document.createElement('div')

				this.time = document.createElement('div')
				this.time.className = 'time';
				this.time.innerText = '';

				this.status = document.createElement('div')
				this.status.className = 'status';
				this.status.innerText = 'ONHOOK';

				div.appendChild(this.time);
				div.appendChild(this.status);
				this.activity.appendChild(div);
			}

			{
				this.callRecord = document.createElement('div');
				this.activity.appendChild(this.callRecord);

				this.callRecord.className = 'call_record';

				this.callRecordButton = document.createElement('button');
				this.callRecordButton.className = 'record_stop';
				this.callRecord.appendChild(this.callRecordButton);

				this.callRecordButton.onclick = (ev) => { this.toggleRecording(); };
				this.showRecording(false);
			}

			{
				this.call_state = document.createElement('div')
				this.call_state.className = 'call_state';
				this.activity.appendChild(this.call_state);
			}

			{
				const tbl = document.createElement('table');
				this.call_state.appendChild(tbl);

				const tr = tbl.insertRow();

				this.tdNam = tr.insertCell();
				this.tdNam.innerText = "Name";

				this.tdNum = tr.insertCell();
				this.tdNum.innerText = "Number";
			}
		}

		//
		//
		//
		{
			this.call_actions = document.createElement('div');
			this.call_actions.className = 'call_actions';
			this.activity.appendChild(this.call_actions);

			this.action_onhook       = document.createElement('input');
			this.action_listen       = document.createElement('input');

			this.action_lenny        = document.createElement('input');
			this.action_disconnected = document.createElement('input');
			this.action_messages     = document.createElement('input');

		//	this.call_actions.appendChild(this.action_offhook);
			this.call_actions.appendChild(this.action_onhook);
			this.call_actions.appendChild(this.action_listen);

			this.action_listen      .type  = 'button';
			this.action_listen      .value = 'LISTEN';
			this.action_onhook      .type  = 'button';
			this.action_onhook      .value = 'HANG UP';
			this.action_lenny       .type  = 'button';
			this.action_lenny       .value = 'LENNY';
			this.action_disconnected.type  = 'button';
			this.action_disconnected.value = 'DISCONNECTED';
			this.action_messages.type  = 'button';
			this.action_messages.value = 'MESSAGE';

			this.action_onhook      .addEventListener('click',(ev) => { this.sendOnHook(); },supportsPassive);
			this.action_listen      .addEventListener('click',(ev) => { this.sendListenToggle(); },supportsPassive);
			this.action_lenny       .addEventListener('click',(ev) => { this.sendLenny(); },supportsPassive);
			this.action_disconnected.addEventListener('click',(ev) => { this.sendDisconnected(); },supportsPassive);
			this.action_messages    .addEventListener('click',(ev) => { this.sendMessages(); },supportsPassive);

			//
			//
			//
			{
				this.answer_actions = document.createElement('div');
				this.answer_actions.id = 1 == this.idx ?'left' :'right';
				this.answer_actions.className = 'side_popup gone';
				{
					const div = document.createElement('div');
					div.innerText = 'LINE '+this.idx;
					this.answer_actions.appendChild(div);
				}
				this.answer_actions.appendChild(this.action_lenny);
				this.answer_actions.appendChild(this.action_disconnected);
				this.answer_actions.appendChild(this.action_messages);

				document.body.appendChild(this.answer_actions);
			}
		}

	}


	createSettingsView() {
		if (this.input_number) return;

		this.input_number = document.createElement('input');
		this.input_number.type = 'text';
		this.input_number.placeholder = 'NUMBER';
		this.input_number.readOnly = true;

		this.input_lenny_enabled = document.createElement('input');
		this.input_lenny_enabled.type = 'checkbox';

		this.input_disconnected_enabled = document.createElement('input');
		this.input_disconnected_enabled.type = 'checkbox';

		this.input_tad_enabled = document.createElement('input');
		this.input_tad_enabled.type = 'checkbox';

		this.settings = document.createElement('fieldset');
		this.settings.className = 'line';

		this.name = document.createElement('legend');
		{
			const span = document.createElement('button');
			span.className = 'number';
			span.innerHTML = '&#x260E; LINE '+this.idx+' : '+this.cfg.number;
			this.name.appendChild(span);
			span.onclick = (ev) => {
				if (dbg) console.log('telephone number click');
				new Popup().showLineConfig(this);
			};
		}
		this.settings.appendChild(this.name);

		const tbl = document.createElement('table');
		tbl.id = 'settings_options';
		this.settings.appendChild(tbl);

/*
		{
			const tr = tbl.insertRow();
//			tr.insertCell().innerText = "NUMBER";
			const td = tr.insertCell();
			td.appendChild(this.input_number);

			const span = document.createElement('span');
			td.appendChild(span);
			span.innerHTML = '&#x1F512; &#x1F511';
		}
*/

		{
			const span_sw = document.createElement('span');
			const span_sl = document.createElement('span');
			span_sw.className = 'switch';
			span_sl.className = 'slider round';
			span_sw.appendChild(this.input_lenny_enabled);
			span_sw.appendChild(span_sl);

			const tr = tbl.insertRow();
			const td = tr.insertCell();
			td.appendChild(span_sw);
			const span = document.createElement('span');
			span.innerText = 'LENNY';
			td.appendChild(span);

			{
				const div = document.createElement('div');
				td.appendChild(div);

				div.id = 'settings_ogm';

				this.settings_profile_but = document.createElement('input');
				div.appendChild(this.settings_profile_but);

				this.settings_profile_but.type = 'button';
				this.settings_profile_but.value = this.cfg.lenny_profile +' Profile';
				this.settings_profile_but.onclick = (ev) => {
					ev.stopPropagation();
					new Popup().showLennyProfile(this);
				};
			}

			span_sw.parentElement.addEventListener('click',(ev) => {
				ev.stopPropagation();
				this.toggleLenny();
			},supportsPassive);
		}

		{
			const span_sw = document.createElement('span');
			const span_sl = document.createElement('span');
			span_sw.className = 'switch';
			span_sl.className = 'slider round';
			span_sw.appendChild(this.input_disconnected_enabled);
			span_sw.appendChild(span_sl);

			const tr = tbl.insertRow();
			const td = tr.insertCell();
			td.appendChild(span_sw);
			const span = document.createElement('span');
			span.innerText = 'DISCONNECTED';
			td.appendChild(span);

			{
				const div = document.createElement('div');
				td.appendChild(div);

				div.id = 'settings_ogm';

				this.settings_disconnected_but = document.createElement('button');
				div.appendChild(this.settings_disconnected_but);

				this.settings_disconnected_but.innerHTML = '&#x25B6;';
				this.settings_disconnected_but.onclick = (ev) => {
					ev.stopPropagation();
					new Popup().showPlayDisconnected();
				};
			}

			span_sw.parentElement.addEventListener('click',(ev) => {
				ev.stopPropagation();
				this.toggleDisconnected();
			},supportsPassive);
		}

		{
			const span_sw = document.createElement('span');
			const span_sl = document.createElement('span');
			span_sw.className = 'switch';
			span_sl.className = 'slider round';
			span_sw.appendChild(this.input_tad_enabled);
			span_sw.appendChild(span_sl);

			const tr = tbl.insertRow();
			const td = tr.insertCell();
			td.appendChild(span_sw);
			const span = document.createElement('span');
			span.innerText = 'MESSAGES';
			td.appendChild(span);

			{
				const div = document.createElement('div');
				td.appendChild(div);

				div.id = 'settings_ogm';

				this.settings_ogm_but = document.createElement('input');
				div.appendChild(this.settings_ogm_but);

				this.settings_ogm_but.type = 'button';
				this.settings_ogm_but.value = 'OGM';
				this.settings_ogm_but.onclick = (ev) => {
					ev.stopPropagation();
					new Popup().showOGM( this );
				};
			}

			span_sw.parentElement.addEventListener('click',(ev) => {
				ev.stopPropagation();
				this.toggleMessages();
			},supportsPassive);
		}
	}



	//
	// Activity actions
	//
	sendListenToggle() {
		if (dbg) console.log('sendListenToggle');
		if (!this.listening) {

			this.listening = new PlayAudioStream();

			ws.sendListenOn(this.idx);
		} else {
			this.sendListenStop();
		}
	}
	sendListenStop() {
		if (!this.listening) return;

		if (dbg) console.log('sendListenStop');
		ws.sendListenOff(this.idx);

		this.listening.stop();
		delete this['listening'];
	}

	//
	sendRecordToggle(start) {
		if (dbg) console.log('sendRecordToggle '+(start?'START':'STOP'));
		this.recording = start;
		if (start) {
			ws.sendRecordOn(this.idx);
		} else {
			ws.sendRecordOff(this.idx);			
		}
	}
	sendRecordStop() {
		if (!this.recording) return;
		this.recording = false;

		if (dbg) console.log('sendRecordStop');
		ws.sendRecordOff(this.idx);			
	}

	//
	sendOffHook() {
		if (dbg) console.log('sendOffHook');
		ws.sendAction(this.idx,'offhook');
	}
	sendOnHook() {
		if (dbg) console.log('sendOnHook');
		ws.sendAction(this.idx,'onhook');

		this.sendListenStop();
		this.sendRecordStop();
	}
	sendLenny() {
		if (dbg) console.log('sendLenny');
		ws.sendAction(this.idx,'lenny');
		this.showMessage('starting lenny');
	}
	sendDisconnected() {
		if (dbg) console.log('sendDisconnected');
		ws.sendAction(this.idx,'disconnected');
		this.showMessage('starting disconnected');
	}
	sendMessages() {
		if (dbg) console.log('sendMessages');
		ws.sendAction(this.idx,'messages');
		this.showMessage('starting messages');
	}
	sendIgnore() {
		if (dbg) console.log('sendIgnore');
		ws.sendAction(this.idx,'ignore');
	}



	//
	// Settings actions
	//

	toggleLenny() {
		if (dbg) console.log('toggleLenny');
		this.cfg.lenny_enabled = !this.cfg.lenny_enabled;
		this.updateSettingsView();
		ws.sendSetConfig(this.idx,'lenny_enabled',this.cfg.lenny_enabled);
	}
	toggleDisconnected() {
		if (dbg) console.log('toggleDisconnected');
		this.cfg.disconnected_enabled = !this.cfg.disconnected_enabled;
		this.updateSettingsView();
		ws.sendSetConfig(this.idx,'disconnected_enabled',this.cfg.disconnected_enabled);
	}
	toggleMessages() {
		if (dbg) console.log('toggleMessages');
		this.cfg.tad_enabled = !this.cfg.tad_enabled;
		this.updateSettingsView();
		ws.sendSetConfig(this.idx,'tad_enabled',this.cfg.tad_enabled);
	}

}


















//
//
//

class Popup {

	static closeAll() {
		const dd = document.getElementsByClassName('popup-frame');
		if (dbg) console.log('popups closeAll l:'+dd.length);
		for (let i=0;i<dd.length;i++) dd[i].parentElement.removeChild(dd[i]);
	}

	close() {
		if (dbg) console.log('popup close');
//		if (this.onclose && this.onclose()) return;
		if (this.onclose) {
			const r = this.onclose();
			if (r) return;
		}
		if (this.frame.parentElement) this.frame.parentElement.removeChild(this.frame);
	}



	//
	//
	//
	//

	showTour(cb) {
		if (dbg) console.log('showTour');
		const pages = [
			{ img:'img/lenny_toon.png',tit:'Welcome to Lenny'       ,txt1:'Lenny the Telemarketer Troll ...',txt2:'About Lenny ...' },
			{ img:'img/ss_calls.png'  ,tit:'Smart Answering Machine',txt1:'Lenny is a Smart Telephone Answering Machine ...',txt2:'More about TAD ...' },
			{ img:'img/raspbian.png'  ,tit:'DIY with RaspberryPi'   ,txt1:'Lenny is best when running on RaspberryPi ...',txt2:'More about RPi ...' },
			{ img:'img/usr_modem.png' ,tit:'Plain Old Telephone'    ,txt1:'Best when using a USB Modem ...',txt2:'Amazon and USRobotics ...' },
			{ img:'img/lenny_toon.png',tit:'Free Evaluation Beta'   ,txt1:'Getting started ...',txt2:'Try Lenny Troll using this FREE Evaluation Beta Version ...' }
		//	{ img:'img/lenny_toon.png',tit:'Free Evaluation License',txt1:'Getting started ...',txt2:'Get a FREE Evaluation License Key ...' }
		];
		this.showWizard('Lenny Tour','Finished',pages);
	}

	showFirstTime(cb) {
		if (dbg) console.log('showFirstTime');

		const pages = [
			{ img:'img/lenny_toon.png',tit:'Welcome to Lenny'       ,txt1:'Lenny the Telemarketer Troll ...',txt2:'About Lenny ...' },
			{ img:'img/ss_calls.png'  ,tit:'Smart Answering Machine',txt1:'Lenny is a Smart Telephone Answering Machine ...',txt2:'More about TAD ...' },
			{ img:'img/raspbian.png'  ,tit:'DIY with RaspberryPi'   ,txt1:'Lenny is best when running on RaspberryPi ...',txt2:'More about RPi ...' },
			{ img:'img/usr_modem.png' ,tit:'Plain Old Telephone'    ,txt1:'Best when using a USB Modem ...',txt2:'Amazon and USRobotics ...' },
			{ img:'img/lenny_toon.png',tit:'Free Evaluation Beta'   ,txt1:'Getting started ...',txt2:'Try Lenny Troll using this FREE Evaluation Beta Version ...' }
		//	{ img:'img/lenny_toon.png',tit:'Free Evaluation License',txt1:'Getting started ...',txt2:'Get a FREE Evaluation License Key ...' }
		];
		this.showWizard('Getting Started','Get Started Now',pages,false,cb);
	}

	showWizard(title,action,pages,hasClose=true,cb =null) {
		if (dbg) console.log('showWizard');

		const next = document.createElement('a');
		const prev = document.createElement('a');

		const counter = document.createElement('div');
		counter.id = 'counter';

		const butDone = document.createElement('input');
		butDone.type = 'button';
		butDone.value = action;

		butDone.onclick = (ev) => {
			Popup.closeAll();
			if (cb) cb();
		};
//		this.close = () => { }

		const tit = document.createElement('div');


		const getPage = (pg) => {
			const div = document.createElement('div');
			div.className = 'help';

			{
				const dd = document.createElement('div');
				const img = document.createElement('img');
				img.classList = 'top';
				img.src = pg.img;
				dd.appendChild(img);
				div.appendChild(dd);
			}
			{
				const p = document.createElement('p');
				p.innerHTML = pg.txt1;
				div.appendChild(p);
			}
			{
				const p = document.createElement('p');
				p.innerHTML = pg.txt2;
				div.appendChild(p);
			}

			return div;
		};


		//
		//
		//
		let it = {i:0,n:[]};
		let pageIdx = 0;

		const dd = document.createElement('div');
		dd.style.height = 'auto';

		const show = () => {
			counter.innerText = (1+pageIdx)+' of '+pages.length;

			if (pages.length <= pageIdx+1) { next.innerHTML = '&nbsp; &nbsp;'; } else { next.innerHTML = arrowRight; }
			if (           0 == pageIdx  ) { prev.innerHTML = '&nbsp; &nbsp;'; } else { prev.innerHTML = arrowLeft; }

			while (0 < dd.childNodes.length) dd.removeChild(dd.childNodes[0]);

			const p = pages[pageIdx];
			tit.innerText = p.tit;
			dd.appendChild( getPage(p) );

			if (pages.length == pageIdx+1) {
				butDone.classList.remove('hidden');
			} else {
				butDone.classList.add('hidden');
			}
		};

		//
		show();
		prev.onclick = (ev) => { if (dbg) console.log('prev click'); if (0 < pageIdx) { pageIdx--; show(); } };
		next.onclick = (ev) => { if (dbg) console.log('next click'); if (pages.length > 1+pageIdx) { pageIdx++; show(); } };


		//
		// Layout
		//
		const div = document.createElement('div');
		div.className = 'popup_inner';
		div.style.height = (heightCalc - 80)+'px';

		const fr = document.createElement('div');
		div.appendChild(fr);

		{
			const dd = document.createElement('div');
			dd.id = 'popup_history_top';
			fr.appendChild(dd);

			tit.id = 'first_time_title';
			tit.style.width = '100%';
			tit.style.display = 'block';
			tit.style.fontSize = '28px';
			tit.style.fontWeight = 'bold';
			dd.appendChild(tit);


			dd.appendChild(counter);

			{
				const nav = document.createElement('div');
				dd.appendChild(nav);

				nav.id = 'popup_history_nav';

				nav.appendChild(prev);
				nav.appendChild(next);
			}

		}


		//
		//
		//
		fr.appendChild(dd);
		fr.appendChild(butDone);


		//
		// Layout
		//
		this.showPopup(div,title,hasClose);
	}


	//
	//
	//

	showSendComment() {
		if (dbg) console.log('showSendComment');

		const div = document.createElement('div');
		div.className = 'popup_inner';

		{
			const tit = document.createElement('div');
			tit.innerHTML = '<h1>Lenny Community</h1>';
			div.appendChild(tit);
		}

		//
		//
		//
		const r = localStorage.getItem('rating');
		const ratingStart = !r || isNaN(r) ?0 :parseInt(r);
		let rating = ratingStart;

		let enable = () => {};

		{
			const dd = document.createElement('div');
			dd.id = 'report';
			div.appendChild(dd);

			{
				const rdd = document.createElement('fieldset');
				{
					const l = document.createElement('legend');
					l.innerText = 'Your Rating';
					rdd.appendChild(l);
				}

				const r = document.createElement('div');
				r.id = 'rating';

				const rate = () => {
					const m = r.childNodes.length;
					for (let i=0;i<m;i++) {
						r.childNodes[i].src = (i >= rating) ?'img/star0.svg' :'img/star1.svg';
					}
					enable();
				};

				for (let i=0;i<5;i++) {
					const inp = document.createElement('img');
					inp.src = 'img/star0.svg';
					r.appendChild(inp);

					const idx = i;
					inp.onclick = (ev) => {
						ev.stopPropagation();

						rating = 1+ idx;
						rate();
					};
				}
				rate();

				rdd.appendChild(r);
				dd.appendChild(rdd);
			}

			const rfr = document.createElement('div');
			rfr.id = 'rating_fr';

			const comment = document.createElement('textarea');
			comment.placeholder = 'Your comment about Lenny.';
			rfr.appendChild(comment);
			dd.appendChild(rfr);

			{
				const but = document.createElement('button');
				but.innerHTML = 'Cancel';
				dd.appendChild(but);

				but.onclick = () => {
					this.close();
				};
			}
			{
				const but = document.createElement('button');
				but.innerHTML = 'Submit';
				dd.appendChild(but);

				but.onclick = () => {
					if (0 == rating && 0 == comment.value.trim().length) {
					} else {
						if (dbg) console.log('submit rating:'+rating+' comment:'+comment.value);
						localStorage.setItem('rating',rating);

						ws.sendComment(rating,comment.value.trim());
						alert('Thanks for supporting Lenny!');
					}
					this.close();
				};

				const bgColor = but.style.backgroundColor;
				enable = () => {
					but.disabled = rating == ratingStart && 0 == comment.value.length;
					if (dbg) console.log('rating submit '+(but.disabled?'disabled':'enable'));
					but.style.backgroundColor = but.disabled ?'rgba(255,255,255,0.3)' :bgColor;
				};

				enable();
				comment.onkeyup = () => { enable(); };
			}

			this.showPopup(div,'Send a Comment',true);
			comment.focus();
		}
	}

	showSendBugReport() {
		if (dbg) console.log('showSendBugReport');

		const div = document.createElement('div');
		div.className = 'popup_inner';

		{
			const tit = document.createElement('div');
			tit.innerHTML = '<h1>Report an Issue</h1>';
			div.appendChild(tit);
		}

		const defaultSummary = 'Brief description';
		const defaultComment = '\n\nSteps to Reproduce\n1) ...\n2) ...\n3) ...\n\nExpected Result:\n\nActual Result:\n\nNotes:\n\n';

		{
			const dd = document.createElement('div');
			dd.id = 'report';

			dd.style.margin = '14px';
			div.appendChild(dd);

			const bsummary = document.createElement('input');
			bsummary.type = 'text';

			const bcomment = document.createElement('textarea');
		//	bcomment.rows = 15;
			bcomment.style.height = '16em';
			bcomment.style.fontSize = '16px';

			bsummary.placeholder = defaultSummary;
			bcomment.value = defaultComment;

			dd.appendChild(bsummary);
			dd.appendChild(bcomment);

			{
				const but = document.createElement('button');
				but.innerHTML = 'Cancel';
				dd.appendChild(but);

				but.onclick = () => {
					this.close();
				};
			}
			{
				const but = document.createElement('button');
				but.innerHTML = 'Submit';
				dd.appendChild(but);

				but.onclick = () => {
					if (dbg) console.log('bug submit');

					ws.sendBugReport(bsummary.value.trim(),bcomment.value.trim());
					alert('Report sent!');

					this.close();
				};

				const bgColor = but.style.backgroundColor;
				const enable = () => {
					but.disabled =
						0 == bsummary.value.trim().length &&
						(0 == bcomment.value.trim().length || bcomment.value.trim() === defaultComment.trim());
					but.style.backgroundColor = but.disabled ?'rgba(255,255,255,0.3)' :bgColor;
				};

				enable();
				bsummary.onkeyup = () => { enable(); };
				bcomment.onkeyup = () => { enable(); };
			}
			this.showPopup(div,'Defect Report',true);
			bsummary.focus();
		}

	}

	//
	//
	//
	showAppUpdate(cfg,required) {
		if (dbg) console.log('showAppUpdate');

		const div = document.createElement('div');
//		div.id = 'signin';
		div.className = 'popup_inner';

		{
			const img = document.createElement('img');
			img.id = 'popup_img';
			img.id = 'lenny';
			div.appendChild(img);
		}
		{
			const tit = document.createElement('div');
			tit.innerHTML = '<h1>Lenny Update</h1>';
			div.appendChild(tit);
		}
		{
			let str = required
				?('This Lenny version v'+cfg.version_str+' is retired!')
				:('This Lenny version v'+cfg.version_str+' is deprecated.');
			str += '<br/><br/>';
			str += 'Please <a href="http://lennytroll.com#download" target="lenny">download</a> the latest version of Lenny, v'+cfg.server_version_str+' is available.'

			const dd = document.createElement('div');
			dd.style.margin = '30px';
			dd.style.fontSize = '20px';
			dd.innerHTML = str;
			div.appendChild(dd);

			this.onclose = () => { /* disables close */ };
		}

		this.showPopup(div,'Software Update',false);
	}

	//
	//
	//
	showLoading() {
		if (dbg) console.log('showLoading');

		const div = document.createElement('div');
		div.className = 'popup_inner';

		{
			const img = document.createElement('img');
			img.id = 'popup_img';
			img.src = 'img/lenny.png';
			div.appendChild(img);
		}
		{
			const tit = document.createElement('div');
			tit.innerHTML = '<h1>Lenny</h1>';
			div.appendChild(tit);
		}
		{
			const d = document.createElement('div');
			d.className = 'loader';
			div.appendChild(d);
		}
		{
			const d = document.createElement('div');
			d.innerText = 'loading ...';
			div.appendChild(d);
		}
		this.showPopup(div,null,false);
	}

	//
	//
	//
	showDisconnected() {
		if (dbg) console.log('showDisconnected');

		const div = document.createElement('div');
		div.className = 'popup_inner';

		{
			const img = document.createElement('img');
			img.id = 'popup_img';
			img.id = 'lenny';
			div.appendChild(img);
		}
		{
			const tit = document.createElement('div');
			tit.innerHTML = '<h1>Lenny is Disconnected</h1>';
			div.appendChild(tit);
		}
		{
			const but = document.createElement('button');
			but.innerHTML = '&#x21BB; Reconnect';
			div.appendChild(but);

		//	but.focus();

			but.onclick = () => { delete this['onclose']; reload(); };
			this.onclose = () => { delete this['onclose']; reload(); };

			const reload = (ev) => {
				if (dbg) console.log('reconnect click');
				this.close();
				new Popup().showLoading();
				setTimeout(() => { ws.connect(); },500);
			};
		}

		if (WS.isSecure() && ws.hasError) {
			const p = document.createElement('p');
			p.innerText = 'This browser session may not like the TLS/SSL Certificate, you will need to reload the unsecure HTTP session to continue.  Your browser can be configured to support Lenny, please visit http://lennytroll.com#faqs to find out how.';
			div.appendChild(p);

			const but = document.createElement('button');
			but.innerHTML = 'Goto HTTP / Unsecure Version now';
			div.appendChild(but);

			but.onclick = (ev) => { WS.sendToUnsecure(); };
		}

		this.showPopup(div,null,false);
	}




	//
	//
	//
	showSignIn() {
		if (dbg) console.log('showSignIn');

		const namMsg = document.createElement('div');
		const pwdMsg = document.createElement('div');
		namMsg.className = 'red';
		pwdMsg.className = 'red';

		const nam = document.createElement('input');
		const pwd = document.createElement('input');
		const but = document.createElement('input');
		nam.type = 'text';
		nam.placeholder = 'Username';
		nam.autocomplete = 'username';

		pwd.type = 'password';
		pwd.placeholder = 'Password';
		pwd.autocomplete = 'current-password';

		but.value = 'Submit';
		but.type = 'button';

		pwd.disabled = true;
		but.disabled = true;

		nam.oninput = (ev) => {
		//	if (dbg) console.log('oninput v:'+nam.value);
			if (0 == nam.value.length) {
				pwd.disabled = true;
				but.disabled = true;
			} else {
				pwd.disabled = false;
			}
			namMsg.innerText = '';
		}
		pwd.oninput = (ev) => {
		//	if (dbg) console.log('oninput v:'+pwd.value);
			if (0 == pwd.value.length) {
				but.disabled = true;
			} else {
				but.disabled = false;
			}
			pwdMsg.innerText = '';
		}
		but.onclick = (ev) => {
			if (dbg) console.log('but onclick');

			const save_eventSignIn = lenny.eventSignIn;
			lenny.eventSignIn = (ev) => {
				if (dbg) console.log('eventSignIn :'+JSON.stringify(ev));
				lenny.eventSignIn = save_eventSignIn;

				if (!ev.hasOwnProperty('err')) {
					if (dbg) console.log('missing err:'+JSON.stringify(ev));
					return;
				}
				if (0 == ev.err) {
					lenny.eventSignIn(ev);
					this.close();
					return;
				}

				const cat = parseInt(Math.abs(ev.err) /10);
				if (1 == cat) { //user error
					namMsg.innerText = ev.err_msg;
					pwdMsg.innerText = '';
				} else
				if (2 == cat) { //pwd error
					namMsg.innerText = '';
					pwdMsg.innerText = ev.err_msg;
				} else {
					if (dbg) console.log('unexpected err:'+JSON.stringify(ev));
					return;
				}
			};
			ws.sendSignIn(nam.value,pwd.value);
		};

		const tbl = document.createElement('table');
		{
			const tr = tbl.insertRow();
			const td = document.createElement('td');

			const img = document.createElement('img');
			img.src = 'img/lenny.png';
			tr.appendChild(td);

			const tit = document.createElement('div');
			tit.innerHTML = '<h1>Lenny Sign In</h1>';

			td.appendChild(tit);
			td.appendChild(img);
		}
		{
			const tr = tbl.insertRow();
//			const th = document.createElement('th');
			const td = document.createElement('td');
//			tr.appendChild(th);
			tr.appendChild(td);
//			th.innerText = 'Username';
			td.appendChild(nam);
			td.appendChild(namMsg);
		}
		{
			const tr = tbl.insertRow();
//			const th = document.createElement('th');
			const td = document.createElement('td');
//			tr.appendChild(th);
			tr.appendChild(td);
//			th.innerText = 'Password';
			td.appendChild(pwd);
			td.appendChild(pwdMsg);
		}
		{
			const tr = tbl.insertRow();
			const td = document.createElement('td');
			tr.appendChild(td);
			td.appendChild(but);
		}

		//
		// Layout
		//
		const div = document.createElement('div');
		div.id = 'signin';
		div.className = 'popup_inner';

		const form = document.createElement('div');
		form.appendChild(tbl)
		div.appendChild(form);

		this.showPopup(div,null,false);
		nam.focus();

		if (dbg) console.log('height:'+div.getBoundingClientRect().height+' client:'+div.clientHeight+' offset:'+div.offsetHeight+' scroll:'+div.scrollHeight);
	}






	//
	//
	//
	showLicenseEmail(cfg) {
		if (dbg) console.log('showLicenseEmail');

		const fr = document.createElement('div');
		fr.className = 'popup_inner';

		{
			const dd = document.createElement('div');
			dd.classList = 'profile_lenny_img_fr';
			fr.appendChild(dd);

			{
				const img = document.createElement('img');
				dd.appendChild(img);
				img.id = 'profile_lenny_img';
				img.style.margin = '10px 0 0 10px';
				img.src = 'img/lenny.png';
			}
			{
				const inf = document.createElement('div');
				inf.id = 'skiplist_about_fr';
				dd.appendChild(inf);

				{
					const txt = document.createElement('div');
					txt.id = 'skiplist_about'
					txt.innerText = 
						'Your email address is the registered user for this Lenny instance on this device.  '+
						'This inbox will received the new License Key enabling Lenny.';
					inf.appendChild(txt);
				}
			}
		}


		//
		//
		//
		const emailMsg = document.createElement('div');
		emailMsg.className = 'red';

		const email = document.createElement('input');
		email.type = 'text';
		email.maxLength = 120;
		email.placeholder = 'Your Email Address';
		email.autocomplete = 'email';

		const butSave = document.createElement('input');
		butSave.value = 'Save';
		butSave.type = 'button';

		const bgColor = butSave.style.backgroundColor;
		const enable = () => {
			butSave.disabled =
				0 == email.value.trim().length ||
				(cfg.email && cfg.email.trim() === email.value.trim());
			butSave.style.backgroundColor = butSave.disabled ?'rgba(255,255,255,0.3)' :bgColor;
		};

		email.oninput = (ev) => {
		//	if (dbg) console.log('oninput v:'+email.value);
			enable();
			emailMsg.innerText = '';
		}
		butSave.onclick = (ev) => {
			if (dbg) console.log('but onclick');

			const re = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;

	 		if (!re.test(String(email.value).toLowerCase())) {
				emailMsg.innerText = 'Invalid email format, please try again';
				email.focus();
				return;
			}

			emailMsg.innerText = '';
			ws.sendLicenseEmail(email.value.trim());

		//
		// not required when lenny returns new config info update
		//
		//	this.close();
		//	new Popup().showLoading();
		};

		const butCancel = document.createElement('input');
		butCancel.value = 'Cancel';
		butCancel.type = 'button';
		butCancel.style.marginRight = '10px';
		butCancel.onclick = (ev) => {
			if (dbg) console.log('butCancel onclick');
			this.close();
		};

		const tbl = document.createElement('table');
		{
			const tr = tbl.insertRow();
			const td = document.createElement('td');
			tr.appendChild(td);
			td.appendChild(email);
			td.appendChild(emailMsg);
		}
		{
			const tr = tbl.insertRow();
			const td = document.createElement('td');
			tr.appendChild(td);

			const dd = document.createElement('div');
			if (cfg.email && 0 < cfg.email.length) {
				dd.appendChild(butCancel);
				
				if (!cfg.license || !cfg.license.str || 0 == cfg.license.str || 0 != cfg.license.err) {
					butCancel.onclick = (ev) => {
						if (dbg) console.log('butCancel onclick');
						Popup.closeAll();
						new Popup().showLicenseUpdate(cfg);
					};
//					this.close = () => { butCancel.onclick; }
				}
			}
			dd.appendChild(butSave);
			td.appendChild(dd);
		}

		//
		// Layout
		//
		const div = document.createElement('div');
		div.id = 'license';

		const form = document.createElement('div');
		form.appendChild(tbl)
		div.appendChild(form);

		fr.appendChild(div);

		this.showPopup(fr,'Lenny License Email',false);

		if (cfg.email && 0 < cfg.email.length) {
			email.value = cfg.email;
		}
		enable();
		email.focus();

		if (dbg) console.log('height:'+div.getBoundingClientRect().height+' client:'+div.clientHeight+' offset:'+div.offsetHeight+' scroll:'+div.scrollHeight);
	}

	//
	//
	//
	showLicenseUpdate(cfg) {
		if (dbg) console.log('showLicenseUpdate');

		const fr = document.createElement('div');
		fr.className = 'popup_inner';

		{
			const dd = document.createElement('div');
			dd.classList = 'profile_lenny_img_fr';
			fr.appendChild(dd);

			{
				const img = document.createElement('img');
				dd.appendChild(img);
				img.id = 'profile_lenny_img';
				img.style.margin = '10px 0 0 10px';
				img.src = 'img/lenny.png';
			}
			{
				const inf = document.createElement('div');
				inf.id = 'skiplist_about_fr';
				dd.appendChild(inf);

				{
					const txt = document.createElement('div');
					txt.id = 'skiplist_about'
					txt.innerText = 'Update your Lenny License Key.';
					inf.appendChild(txt);
				}
			}
		}


		//
		//
		//
		const licenseMsg = document.createElement('div');
		licenseMsg.className = 'red';

		const license = document.createElement('input');
		license.type = 'text';
		license.maxLength = 24; //0000-0000-0000-0000-0000
		license.placeholder = 'License Key';

		const butSave = document.createElement('input');
		butSave.type = 'button';
		butSave.value = 'Save';

		const butCancel = document.createElement('input');
		butCancel.type = 'button';
		butCancel.value = 'Cancel';
		butCancel.style.marginRight = '10px';


		const bgColor = butSave.style.backgroundColor;
		const enable = () => {
			butSave.disabled = (0 != license.value.trim() && 24 != license.value.trim().length) ||
			(cfg.license && cfg.license.str && cfg.license.str === license.value);
			butSave.style.backgroundColor = butSave.disabled ?'rgba(255,255,255,0.3)' :bgColor;
		};

		//
		//
		//

		license.oninput = (ev) => {
		//	if (dbg) console.log('oninput v:'+license.value);
			enable();
			licenseMsg.innerText = '';
		}

		butSave.onclick = (ev) => {
			if (dbg) console.log('butSave onclick');

			if (0 == license.value.length) {
				if (dbg) console.log('submit empty license');

				if (cfg.license && cfg.license.str && 24 == cfg.license.str.trim().length && 0 == cfg.license.err) {
					if (!confirm('Remove your current license key '+cfg.email+' address? Are you sure?')) return;
				}

				ws.sendLicenseKey('');

				Popup.closeAll();
				new Popup().showLoading();
				return;
			}

			// match 0000-0000-0000-0000-0000
			const re = /^[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}$/i;

	 		if (!re.test(String(license.value).toLowerCase())) {
				licenseMsg.innerText = 'Invalid license format, please try again';
				license.focus();
				return;
			}

			if (cfg.license && cfg.license.str && 24 == cfg.license.str.length && 0 == cfg.license.err) {
				if (!confirm('Save this as your new License key? Are you sure?')) return;
			}

			if (dbg) console.log('submit license:'+license.value);
			ws.sendLicenseKey(license.value);

			Popup.closeAll();
			new Popup().showLoading();
		};

		butCancel.onclick = (ev) => {
			if (dbg) console.log('butCancel onclick');
			this.close();
		};


		//
		// Layout
		//
		const tbl = document.createElement('table');
		{
			const tr = tbl.insertRow();
			const td = document.createElement('td');
			tr.appendChild(td);
			td.appendChild(license);
			td.appendChild(licenseMsg);
		}
		{
			const tr = tbl.insertRow();
			const td = document.createElement('td');
			tr.appendChild(td);

			const dd = document.createElement('div');
			dd.appendChild(butCancel);
			dd.appendChild(butSave);
			td.appendChild(dd);
		}


		//
		//
		//
		if ((cfg.email && 0 < cfg.email.length) &&
		   (!cfg.license || !cfg.license.str || 0 == cfg.license.str.length || 0 != cfg.license.err)) {

			const but = document.createElement('input');
			but.type = 'button';
			but.value = 'Request a new 30-day evaluation License Key';

			but.onclick = (ev) => {
				if (dbg) console.log('butCreate onclick');

				if (!confirm('Email a new License key to your '+cfg.email+' address? Are you sure?')) return;
				ws.sendCreateLicense();
			};

			{
				const tr = tbl.insertRow();
				const td = document.createElement('td');
				tr.appendChild(td);

				const dd = document.createElement('div');
				dd.appendChild(but);
				td.appendChild(dd);
			}
		}


		//
		// Layout
		//
		const div = document.createElement('div');
		div.id = 'license';

		const form = document.createElement('div');
		form.appendChild(tbl)
		div.appendChild(form);

		fr.appendChild(div);

		this.showPopup(fr,'Lenny License Key',false);

		if (dbg) console.log('height:'+div.getBoundingClientRect().height+' client:'+div.clientHeight+' offset:'+div.offsetHeight+' scroll:'+div.scrollHeight);


		//
		//
		//
		const showEmailUpdate = () => {
			butCancel.value = 'Email Address';
			butCancel.onclick = (ev) => {
				if (dbg) console.log('butResend onclick');
				Popup.closeAll();
				new Popup().showLicenseEmail(cfg);
			};
		};

		if (!cfg.license || !cfg.license.str || 0 == cfg.license.str.length) {
			showEmailUpdate();
//			this.close = () => { butCancel.onclick; }
		} else {
			license.value = cfg.license.str;

			if (cfg.license.err && 0 != cfg.license.err) {
				if (-1 == cfg.license.err) {
					licenseMsg.innerText = 'License has expired, a new license will need to requested';
				} else
				if (-2 == cfg.license.err) {
					licenseMsg.innerText = 'License is invalid, a new license will need to requested';
				} else {
					licenseMsg.innerText = 'License has unknown error, a new license will need to requested';				
				}
	
				showEmailUpdate();
			}
		}

		enable();
		license.focus();
	}









	//
	//
	//
	showLineConfig(it) {
		if (dbg) console.log('showLineConfig');

		const fr = document.createElement('div');
		fr.id = 'config';
		fr.className = 'popup_inner';

		//
		// Header layout
		//
		{
			const dd = document.createElement('div');
			dd.classList = 'profile_lenny_img_fr';
			fr.appendChild(dd);

			{
				const img = document.createElement('div');
				dd.appendChild(img);
				img.id = 'popup_history_img';
				img.innerHTML = '&#x260E;';
			}
			{
				const inf = document.createElement('div');
				inf.id = 'skiplist_about_fr';
				dd.appendChild(inf);

				const txt = document.createElement('h1');
				txt.innerText = 'Line '+it.idx;
				inf.appendChild(txt);
			}
		}


		const namMsg = document.createElement('div');
		namMsg.className = 'red';
		const numMsg = document.createElement('div');
		numMsg.className = 'red';

		const nam = document.createElement('input');
		const num = document.createElement('input');

		const butSave = document.createElement('input');
		const butCancel = document.createElement('input');

		nam.type = 'text';
		num.type = 'text';
		nam.placeholder = 'Friendly Name';
		num.placeholder = 'Telephone Number';

		butSave.value = 'Save';
		butSave.type = 'button';
		butCancel.value = 'Cancel';
		butCancel.type = 'button';

		const bgColor = butSave.style.backgroundColor;
		const enable = () => {
			butSave.disabled = 
				(0 == nam.value.trim().length || 0 == num.value.trim().length) ||
				(nam.value === it.cfg.name && 
				 num.value === it.cfg.number);
			butSave.style.backgroundColor = butSave.disabled ?'rgba(255,255,255,0.3)' :bgColor;
		};

		num.onkeyup = (ev) => { enable(); };
		nam.onkeyup = (ev) => { enable(); };

		nam.value = it.cfg.name;
		num.value = it.cfg.number;
		enable();

		butSave.onclick = (ev) => {
			if (dbg) console.log('butSave onclick');

			it.cfg.name = nam.value;
			it.cfg.number = num.value;

			//
			// TODO
			//

			this.close();
		};

		butCancel.onclick = (ev) => {
			if (dbg) console.log('butCancel onclick');
			this.close();
		};

		const tbl = document.createElement('table');
		{
			const tr = tbl.insertRow();
			const th = document.createElement('th');
			const td = document.createElement('td');
			tr.appendChild(th);
			tr.appendChild(td);
			th.innerText = 'Device';
			th.style.verticalAlign = 'top';
			th.style.paddingTop = '10px';
			td.style.textAlign = 'left';
			td.style.fontVariant = 'normal';
			td.innerText = it.cfg.device;

			td.innerHTML += '<div style="color:#4c4;font-weight:bold">Status: GREEN</div>';

			td.innerHTML += '<div style="margin-top:14px;font-size:14px;">'+it.cfg.modem.product+'<br/>'+it.cfg.modem.vendor+'</div>';
		}
/*
		{
			const tr = tbl.insertRow();
			const th = document.createElement('th');
			tr.appendChild(th);
		//	th.innerText = 'Modem';

			const td = document.createElement('td');
			td.colSpan = 2;
			tr.appendChild(td);
//			td.style.fontSize = '12px';
			td.style.textAlign = 'left';
			td.style.fontVariant = 'normal';
			td.innerHTML = it.cfg.modem.product+'<br/>'+it.cfg.modem.vendor;
		}
*/

		{
			const tr = tbl.insertRow();
			const th = document.createElement('th');
			const td = document.createElement('td');
			tr.appendChild(th);
			tr.appendChild(td);
			th.innerText = 'Name';
			td.appendChild(nam);
			td.appendChild(namMsg);
		}
		{
			const tr = tbl.insertRow();
			const th = document.createElement('th');
			const td = document.createElement('td');
			tr.appendChild(th);
			tr.appendChild(td);
			th.innerText = 'Number';
			td.appendChild(num);
			td.appendChild(numMsg);
		}
		{
			const tr = tbl.insertRow();
			const td = document.createElement('td');
			td.colSpan = 2;
			butSave.style.marginRight = '20px';
			tr.appendChild(td);
			td.appendChild(butCancel);
			td.appendChild(butSave);
		}

		//
		// Layout
		//
		fr.appendChild(tbl);


		this.showPopup(fr,'&#x260E; Line Configuration',false);
	}











	//
	//
	//
	showSkiplist(list) {
		if (dbg) console.log('showSkiplist');

		const butDone = document.createElement('button');
		butDone.classList = 'done';
		butDone.innerText = 'Done';

		butDone.onclick = (ev) => {
			if (dbg) console.log('butDone onclick');
			this.close();
		};

/*
		const butSave = document.createElement('button');
		const butDelete = document.createElement('button');
		const butCancel = document.createElement('button');

		butSave.innerText = 'Save';
		butDelete.innerHTML = '&#x1F5D1; Remove';
		butCancel.innerText = 'Cancel';

		butSave.onclick = (ev) => {
			if (dbg) console.log('butSave onclick');
			//
			// todo
			//
			this.close();
		};
		butDelete.onclick = (ev) => {
			if (dbg) console.log('butDelete onclick');
			if (!confirm('Remove this permanently? Are you sure?')) return;

			//
			// TODO
			ws.sendSkiplistRemove()
			//
			this.close();
		};
		butCancel.onclick = (ev) => {
			if (dbg) console.log('butCancel onclick');
			this.close();
		};
*/

		//
		//
		//
		const fr = document.createElement('div');

		{
			const dd = document.createElement('div');
			dd.classList = 'profile_lenny_img_fr';
			fr.appendChild(dd);

			{
				const img = document.createElement('div');
				dd.appendChild(img);
				img.id = 'popup_history_img';
				img.innerHTML = '&#x260E;';
			}
			{
				const inf = document.createElement('div');
				inf.id = 'skiplist_about_fr';
				dd.appendChild(inf);

				{
					const txt = document.createElement('div');
					txt.id = 'skiplist_about'
					txt.innerText = 'Telephone numbers listed will be answered normally or sent to the Telephone Answering Machine (TAD) service, and not answerd by Lenny.';
					inf.appendChild(txt);
				}
				{
					const dd = document.createElement('div');
					dd.id = 'skiplist_add'
					const but = document.createElement('button');
					but.innerText = 'ADD';
					but.onclick = (ev) => {
						if (dbg) console.log('skiplist add click');
						new Popup().showSkiplistItem( {n:[{name:'',number:'',note:''}],i:0,new:true} );

						this.close();
					};
					inf.appendChild(dd);
					dd.appendChild(but);
				}
			}
		}


/*
		{
			const div = document.createElement('div');
			div.id = 'skiplist_about_fr';

			const txt = document.createElement('div');
			txt.id = 'skiplist_about'
			txt.innerText = 'Numbers listed will not be answered by Lenny or Disconnected allowing these calls be answered normally or sent to the Answering Machine service.';
			div.appendChild(txt);

			const div = document.createElement('div');
			const but = document.createElement('button');
			but.innerText = 'ADD';
			but.onclick = (ev) => { if (dbg) console.log('skiplist add click'); new Popup().showSkiplistItem( {n:[{name:'',number:'',note:''}],i:0,new:true} ); };
			div.appendChild(but);
			fr.appendChild(div);
		}
*/


		const tbl = document.createElement('table');
		const d = document.createElement('div');
		d.style.textAlign = 'center';
		fr.appendChild(d);
		d.appendChild(tbl);

		const lmax = list.length;
		for (let i=0;i<lmax;i++) {
			const it = list[i];

			const tr = tbl.insertRow();
			tr.onclick = (ev) => { if (dbg) console.log('skiplist item'); new Popup().showSkiplistItem( {n:list,i:i} ); };

			const tdNum  = tr.insertCell();
			tdNum.classList = 'number';
			const tdName = tr.insertCell();
			const tdNote = tr.insertCell();
			tdNote.classList = 'to_right';

			tdNum.innerText = it.number;
			tdName.innerText = it.name;
			tdNote.innerText = it.note;

			{
				const but = document.createElement('button');
				but.classList = 'delete';
				but.innerHTML = '&#x1F5D1;';

				const tdTrash = tr.insertCell();
				tdTrash.appendChild(but);

				but.onclick = (ev) => {
					ev.stopPropagation();

					if (!confirm('Remove this skiplist number?  Are you sure?')) return;
					if (dbg) console.log('delete skiplist :'+it.number +' clicked');

					tr.parentElement.removeChild(tr);

					{
						const max = lenny.skiplist.length;
						for (let i=0;i<max;i++) {
							if (it.number != lenny.skiplist[i].number) continue;

							lenny.skiplist = lenny.skiplist.slice(0,i).concat(lenny.skiplist.slice(i +1));
							break;
						}
					}

					ws.sendSkiplistRemove(it);
				}
			}
		}

		{
			const tr = tbl.insertRow();
			tr.className = 'not_active';
			const td = tr.insertCell();
			td.colSpan = 4;
			td.appendChild(butDone);
		}


		//
		// Layout
		//
		const div = document.createElement('div');
		div.id = 'skiplist';
		div.className = 'popup_inner';
		div.appendChild(fr);

		this.showPopup(div,'Skiplist',true);
	}



	//
	//
	//
	showSkiplistItem(it) {
		if (dbg) console.log('showSkiplistItem '+(!it ?'new' :JSON.stringify(it)));

		const next = document.createElement('a');
		const prev = document.createElement('a');
		const counter = document.createElement('div');
		counter.id = 'counter';

		const namMsg = document.createElement('div');
		const numMsg = document.createElement('div');
		namMsg.className = 'red';
		numMsg.className = 'red';

		const num = document.createElement('input');
		const nam = document.createElement('input');
		const note = document.createElement('input');
		const butSave = document.createElement('input');
		const butCancel = document.createElement('input');
		const butDone = document.createElement('input');

		num.type = 'text';
		num.placeholder = 'Number';

		nam.type = 'text';
		nam.placeholder = 'Name';

		note.type = 'text';
		note.placeholder = 'Note';

		butSave.type = 'button';
		butCancel.type = 'button';
		butDone.type = 'button';
		butSave.value = !it.new ?'Save' :'Create';
		butCancel.value = 'Cancel';
		butDone.value = 'Done';

		butDone.onclick = (ev) => {
			this.close();
		};

		const isDirty = () => {
			const nit = it.n[it.i];
			return (nam.value != nit.name || num.value != nit.number || note.value != nit.note);
		};

		const cancelBecauseNotSaved = () => {
			if (isDirty() && !confirm('Cancel changes? Are you sure?')) return false;
			return true;
		}

		numMsg.innerHTML = '&nbsp;';
		namMsg.innerHTML = '&nbsp;';
		num.oninput = (ev) => { numMsg.innerHTML = '&nbsp;'; }
		nam.oninput = (ev) => { namMsg.innerHTML = '&nbsp;'; }

		const changed = () => {
			if (!isDirty()) {
				butDone.classList.remove('hidden');
				butSave.classList.add('hidden');
				butCancel.classList.add('hidden');
			    return true;
			}
			butDone.classList.add('hidden');
			butSave.classList.remove('hidden');
			butCancel.classList.remove('hidden');
			return false;
		};

		num.onkeyup = (ev) => { changed(); };
		nam.onkeyup = (ev) => { changed(); };
		note.onkeyup = (ev) => { changed(); };

		const show = (ev) => {
			counter.innerText = (1+it.i)+' of '+it.n.length;

			if (it.n.length <= it.i+1) { next.innerHTML = '&nbsp; &nbsp;'; } else { next.innerHTML = arrowRight; }
			if (          0 == it.i  ) { prev.innerHTML = '&nbsp; &nbsp;'; } else { prev.innerHTML = arrowLeft; }

			const nit = it.n[it.i];

			nam.value = nit.name;
			num.value = nit.number;
			note.value = nit.note;

			butDone.classList.remove('hidden');
			butSave.classList.add('hidden');
			butCancel.classList.add('hidden');

			butSave.onclick = (ev) => {
				if (dbg) console.log('butSave onclick');

				if (0 == num.value.length) {
					numMsg.innerText = 'Number is empty';
					return;
				}
				if (isNaN(num.value)) {
					numMsg.innerText = 'Number contains invalid characters';
					return;
				}
				if (7 > num.value.length) {
					numMsg.innerText = 'Number is minimum 7 characters';
					return;
				}
				if (0 == nam.value.length) {
					namMsg.innerText = 'Name is empty';
					return;
				}

				//
				if (it.new) {
					const ait = { name:nam.value.trim(),number:num.value.trim(),note:note.value.trim() };
					ws.sendSkiplistAdd(ait);
				} else {
					const uit = { name:nit.name,number:nit.number,note:note.value.trim() };
					ws.sendSkiplistUpdate(uit);
				}

				this.close();
			};

			butCancel.onclick = (ev) => {
				if (dbg) console.log('butCancel onclick');
				show();
			};
		};

		//
		show();
		prev.onclick = (ev) => { if (dbg) console.log('prev click'); if (cancelBecauseNotSaved() && 0 < it.i) { it.i--; show(); } };
		next.onclick = (ev) => { if (dbg) console.log('next click'); if (cancelBecauseNotSaved() && it.n.length > 1+it.i) { it.i++; show(); } };


		//
		// Layout
		//
		const div = document.createElement('div');
		div.id = 'popup_history';
		div.className = 'popup_inner';

		{
			const dd = document.createElement('div');
			dd.id = 'popup_history_top';
			div.appendChild(dd);

			if (1 < it.n.length) {
				dd.appendChild(counter);

				const img = document.createElement('div');
				dd.appendChild(img);
				img.id = 'popup_history_img';
				img.innerHTML = '&#x260E;';

				{
					const nav = document.createElement('div');
					dd.appendChild(nav);

					nav.id = 'popup_history_nav';

					nav.appendChild(prev);
					nav.appendChild(next);
				}
			} else {
				const img = document.createElement('div');
				dd.appendChild(img);
				img.id = 'popup_history_img';
				img.innerHTML = '&#x260E;';				
			}
/*
			{
				const txt = document.createElement('div');
				txt.id = 'skiplist_about'
				txt.style.width = '79%';
				txt.innerText = 'Telephone numbers listed will be answered normally or sent to the Telephone Answering Machine (TAD) service, and not answerd by Lenny.';
				dd.appendChild(txt);
			}
*/
		}


		//
		//
		//
		const tbl = document.createElement('table');
		{
			const tr = tbl.insertRow();
			const th = document.createElement('th');
			const td = document.createElement('td');
			tr.appendChild(th);
			tr.appendChild(td);
			th.innerText = 'Number';
			td.appendChild(num);
		}
		{
			const tr = tbl.insertRow();
			const td = tr.insertCell();
			td.colSpan = 2;
			td.style.textAlign = 'center';
			td.appendChild(numMsg);
		}
		{
			const tr = tbl.insertRow();
			const th = document.createElement('th');
			const td = document.createElement('td');
			tr.appendChild(th);
			tr.appendChild(td);
			th.innerText = 'Name';
			td.appendChild(nam);
		}
		{
			const tr = tbl.insertRow();
			const td = tr.insertCell();
			//document.createElement('td');
			//tr.appendChild(td);
			td.colSpan = 2;
			td.style.textAlign = 'center';
			td.appendChild(namMsg);
		}
		{
			const tr = tbl.insertRow();
			const th = document.createElement('th');
			const td = document.createElement('td');
			tr.appendChild(th);
			tr.appendChild(td);
			th.innerText = 'Note (optional)';
			td.appendChild(note);
		}
		{
			const tr = tbl.insertRow();
			const td = tr.insertCell();
			td.colSpan = 2;
			td.style.height = '20px';
		}
		{
			const tr = tbl.insertRow();
			const td = document.createElement('td');
			tr.appendChild(td);
			td.colSpan = 2;

			td.appendChild(butDone);
			td.appendChild(butSave);
			td.appendChild(butCancel);
		}

		//
		// Layout
		//
		{
			const d = document.createElement('div');
			d.id = 'skiplisted';

			d.appendChild(tbl);
			div.appendChild(d);
		}

		this.showPopup(div,'Skiplist',true);

		if (1 == it.n.length) {
			num.focus();
		} else {
			note.focus();
		}

	}






	//
	//
	//

	showCallMessage(it,asHistory = false) {
		if (dbg) console.log('showCallMessage asHistory:'+asHistory);

		const tbl = document.createElement('table');

		const next = document.createElement('a');
		const prev = document.createElement('a');
		const counter = document.createElement('div');
		counter.id = 'counter';


		const skiplist = !lenny.cfg.config.skiplist_enabled ?null :document.createElement('button');
		if (skiplist) skiplist.innerHTML = 'Skiplist';

		const trash = document.createElement('button');
		trash.style.minWidth = '100px';
		trash.innerHTML = !asHistory ?'&#x1F5D1; Delete' :'&#x1F5D1; Delete Permanently';

		const done = document.createElement('input');
		done.value = 'Done';
		done.type = 'button';
		done.onclick = (ev) => {
			if (dbg) console.log('done onclick');
			this.close();
		};

		const track = document.createElement('div');
		track.width  = audioCanvas.w +'px';
		track.height = audioCanvas.h +'px';

		if (dbg) console.log('audio_canvas w:'+track.width+' h:'+track.height);

		let prevActive = false,nextActive = false;
		prev.onmouseover = (ev) => { ev.stopPropagation(); if (prevActive) prev.classList.add('hover'); };
		prev.onmouseout = (ev) => { ev.stopPropagation(); prev.classList.remove('hover'); };
		next.onmouseover = (ev) => { ev.stopPropagation(); if (nextActive) next.classList.add('hover'); };
		next.onmouseout = (ev) => { ev.stopPropagation(); next.classList.remove('hover'); };

		const show = () => {
			counter.innerText = (1+it.i)+' of '+it.c.length;

			nextActive = !(it.c.length <= it.i+1);
			prevActive = !(0 == it.i)
			if (!nextActive) { next.innerHTML = '&nbsp; &nbsp;'; } else { next.innerHTML = arrowRight; }
			if (!prevActive) { prev.innerHTML = '&nbsp; &nbsp;'; } else { prev.innerHTML = arrowLeft; }
		//	if (it.c.length <= it.i+1) { next.innerHTML = '&nbsp; &nbsp;'; } else { next.innerHTML = arrowRight; }
		//	if (          0 == it.i  ) { prev.innerHTML = '&nbsp; &nbsp;'; } else { prev.innerHTML = arrowLeft; }

			const msg = it.c[it.c.length - it.i -1];
			const skiplisted = null != lenny.skiplistNumbers[msg.number];

			if (!asHistory && msg.new) {
				delete msg['new'];
				ws.sendViewedMessage([msg]);
			}


			//
			//
			//
			if (!msg.file) {
				track.classList.add('hidden');
			} else {
				track.classList.remove('hidden');

				const save = lenny.binaryCallback;
				lenny.binaryCallback = (buf) => {
					lenny.binaryCallback = save;

					if (this.onclose) this.onclose();

					const p = new PlayableAudio(buf.buf.subarray(buf.offset,buf.buf.length - buf.offset));
					while (0 < track.childNodes.length) track.removeChild(track.childNodes[0]);
					p.setup(track,track.width,track.height);

					this.onclose = () => { p.stop(); delete this['onclose']; }
				};
				ws.sendGetMessage(msg.file);
			}


			//
			//
			//
			{
				const d = parseHistDateTime(msg.date,msg.time);
				const yr = d.getFullYear(),mo = 1+d.getMonth(),da = d.getDate();
				const hr = d.getHours(),mn = d.getMinutes(),se = d.getSeconds();
				const line = msg.line;

				let tm = hr + ':';
				if (10 >mn) tm += '0';
				tm += mn;

				let s =
					'<tr><th>Date:</th><td>'  +formatDateShort(new Date(yr,mo-1,da,hr,mn,se))+'</td></tr>' +
					'<tr><th>Time:</th><td>'  +tm        +'</td></tr>' +
					'<tr><th>Name:</th><td>'  +msg.name  +'</td></tr>' +
					'<tr><th>Number:</th><td>'+msg.number+'</td></tr>';

				if (lenny.includeLinesInUi) {
					s += '<tr><th>Line:</th><td>'  +line      +'</td></tr>';
				}

				tbl.innerHTML = s;
			}

			if (skiplist) {
				//
				const showSkiplisted = () => {
					skiplist.disabled = true;
					skiplist.classList.add('not_active');
					skiplist.innerHTML = 'Skiplisted';

					skiplist.onmouseover = (ev) => {};
					skiplist.onclick = (ev) => {};
				};
				const showNotSkiplisted = () => {
					skiplist.disabled = false;
					skiplist.classList.remove('not_active');
					skiplist.innerHTML = ' Skiplist ';
					skiplist.onmouseover = null;

					skiplist.onclick = (ev) => {
						if (dbg) console.log('skiplist clicked');

						showSkiplisted();
						ws.sendSkiplistAdd(msg);
					};
				}

				if (skiplisted) {
					showSkiplisted();
				} else {
					showNotSkiplisted();
				}
			}

			//
			trash.onclick = (ev) => {
				ev.stopPropagation();

				//
				// send action
				//
				if (!asHistory) {
					ws.sendHistoryMessage([msg]);
				} else {
					//
					// TBA does this need a confirm?
					//
					ws.sendDeleteMessage([msg]);
				}


				//
				// ui update
				//
				const idx = it.c.length - it.i -1;
				it.c = it.c.slice(0,idx).concat(it.c.slice(idx +1));

				if (!asHistory) {
					lenny.displayCalls = it.c;
					lenny.updateCalls();
				} else {
					lenny.displayHistory = it.c;
					lenny.updateHistory();
				}

				if (0 == it.c.length) {
					if (dbg) console.log('close on empty list');
					this.close();
					return;
				}

				// Handle end of list
				if (it.i >= it.c.length) it.i--;

				show();
			};
		};

		//
		show();
		prev.onclick = (ev) => { if (dbg) console.log('prev click'); if (0 < it.i) { it.i--; show(); } };
		next.onclick = (ev) => { if (dbg) console.log('next click'); if (it.c.length > 1+it.i) { it.i++; show(); } };

		//
		// Layout
		//
		const div = document.createElement('div');
		div.className = 'popup_inner';
		div.id = 'popup_history';

		const fr = document.createElement('div');
		//fr.id = 'popup_history_fr';
		div.appendChild(fr);

		{
			const dd = document.createElement('div');
			dd.id = 'popup_history_top';
			fr.appendChild(dd);

			dd.appendChild(counter);

			const img = document.createElement('div');
			dd.appendChild(img);
			img.id = 'popup_history_img';
			img.innerHTML = '&#x260E;';

			const nav = document.createElement('div');
			dd.appendChild(nav);

			nav.id = 'popup_history_nav';

			nav.appendChild(prev);
			nav.appendChild(next);
		}

		//
		const sfr = document.createElement('div');
		sfr.id = 'popup_history_info_scr';
		fr.appendChild(sfr);

		const scr = document.createElement('div');
		scr.id = 'popup_history_info_fr';
		sfr.appendChild(scr);

		{
			const info = document.createElement('div');
			info.id = 'popup_history_info';

			scr.appendChild(info);

			info.appendChild(tbl);
		}
		{
			const opts = document.createElement('div');
			opts.style = 'margin-left:20px;margin-right:20px;'

			scr.appendChild(opts);

			if (skiplist) opts.appendChild(skiplist);
			opts.appendChild(trash);

			opts.appendChild(done);
		}
		{
			const aud = document.createElement('div');
			aud.id = 'popup_history_aud';
			aud.appendChild(track);
			scr.appendChild(aud);
		}

		this.showPopup(div,'&#x260F; '+(!asHistory?'Call' :'History'),true);

		//
		// Adjust height
		//
		const h = div.getBoundingClientRect().height;

		// 512
		if (dbg) console.log('popup height:'+h+' client:'+div.clientHeight+' offset:'+div.offsetHeight+' scroll:'+div.scrollHeight);

//		if (heightCalc < (h * 1.2)) {
//			fr.style.height = '90%';
//		} else {
//			div.style.height = (heightCalc -80) +'px';
//		}
	}
	




	//
	//
	//

	showPlayDisconnected() {
		if (dbg) console.log('showPlayDisconnected');


		const fr = document.createElement('div');
//		fr.id = 'popup_history_info_scr';


		//
		// Header layout
		//
		{
			const dd = document.createElement('div');
			dd.classList = 'profile_lenny_img_fr';
			fr.appendChild(dd);

			{
				const img = document.createElement('div');
				dd.appendChild(img);
				img.id = 'popup_history_img';
				img.innerHTML = '&#x260E;';
			}
			{
				const inf = document.createElement('div');
				inf.id = 'skiplist_about_fr';
				dd.appendChild(inf);

				{
					const txt = document.createElement('h1');
					txt.innerText = 'Disconnected';
					inf.appendChild(txt);
				}
			}
		}



		//
		//
		//
		const track = document.createElement('div');
		fr.appendChild(track);
		track.width  = audioCanvas.w +'px';
		track.height = audioCanvas.h +'px';

		{
			const save = lenny.binaryCallback;
			lenny.binaryCallback = (buf) => {
				lenny.binaryCallback = save;

				if (this.onclose) this.onclose();

				const p = new PlayableAudio(buf.buf.subarray(buf.offset,buf.buf.length - buf.offset));
				while (0 < track.childNodes.length) track.removeChild(track.childNodes[0]);
				p.setup(track,track.width,track.height);

				this.onclose = () => { p.stop(); delete this['onclose']; }
			};

			ws.sendGetDisconnectedMessage();
		}


		//
		// Layout
		//
		const div = document.createElement('div');
		div.id = 'popup_history';
		div.className = 'popup_inner';

		div.appendChild(fr);

		
		{
			const inp = document.createElement('input');
			inp.classList = 'done';
			inp.type = 'button';
			inp.value = 'Done';

			fr.appendChild(inp);

			inp.onclick = (ev) => {
				if (dbg) console.log('done clicked');
				this.close();
			};
		}

		this.showPopup(div,'&#x260F; '+'Disconnected');


		//
		// Measure
		//
		const h = div.getBoundingClientRect().height;

		// 512
		if (dbg) console.log('popup height:'+h+' client:'+div.clientHeight+' offset:'+div.offsetHeight+' scroll:'+div.scrollHeight);

	//	if (heightCalc < (h * 1.2)) {
	//		fr.style.height = '90%';
	//	} else {
	//		fr.style.height = (heightCalc -200) +'px'; //h +'px';
	//	}

	}









	//
	//
	//

	showOGM(line) {
		if (dbg) console.log('showOGM');

		const div = document.createElement('div');
		div.className = 'popup_inner';
	//	div.id = 'popup_ogm';

		const current = document.createElement('div');
		current.id = 'popup_ogm_current';


		{
			const dd = document.createElement('div');
			dd.classList = 'profile_lenny_img_fr';
			div.appendChild(dd);

			{
				const img = document.createElement('div');
				dd.appendChild(img);
				img.id = 'popup_history_img';
				img.innerHTML = '&#x260E;';
			}
			{
				const inf = document.createElement('div');
				inf.id = 'skiplist_about_fr';
				dd.appendChild(inf);

				{
					const txt = document.createElement('div');
					txt.id = 'popup_ogm_line_title'
					txt.innerText = 'Line: '+line.cfg.name;
					inf.appendChild(txt);



					//
					//
					//
					const divSelect = document.createElement('div');
					inf.appendChild(divSelect);
					divSelect.id = 'answer_rings_fr';
					{
						const sp = document.createElement('span');
						divSelect.appendChild(sp);
						sp.innerText = 'Answer Rings';

						const select = document.createElement('select');
						divSelect.appendChild(select);

						{
							const ringChoice = line.cfg.tad_rings;
							const rings = [1,2,3,4];

							select.onchange = () => {
								select.childNodes.forEach( (it) => {
									if (!it.selected) return;
									if (dbg) console.log(it.value + ' selected');

									line.cfg.tad_rings = it.value;
									ws.sendSetConfig(line.idx,'tad_rings',parseInt(it.value));
								});
							};

							rings.forEach((r) => {
								const opt = document.createElement('option');
								opt.selected = r == ringChoice;
								opt.appendChild( document.createTextNode(r) );
								select.appendChild(opt);
							});
						}
					}

				}
			}
		}


		{
			const tit = document.createElement('div');
			current.appendChild(tit);
			tit.id = 'popup_ogm_current_title';
			tit.innerText = 'Current';
		}


		//
		let players = [];
		this.onclose = () => {
			if (dbg) console.log('onclose l:'+players.length);
			while (0 < players.length) {
				const p = players.pop();
				p.stop();
			}
		}





		//
		// PLAYER
		//
		{
			const save = lenny.binaryCallback;
			lenny.binaryCallback = (buf) => {
				lenny.binaryCallback = save;

				const ar = buf.buf.subarray(buf.offset,buf.buf.length - buf.offset);
				const p = new PlayableAudio(ar);
				players.push(p);

				if (!line.cfg.tad_ogm_default) {
					p.ontrash = () => {
						if (!confirm('Remove current message and use the default message. Are you sure?')) return;
						if (dbg) console.log('trash confirmed');
						this.close();

						ws.sendRemoveCurrentOGM(line);
					};
				}

				p.setup(current,audioCanvas.w +'px',audioCanvas.h +'px');
			}

			//
			ws.sendGetCurrentOGM(line.idx);
		}



		//
		// Layout
		//

		div.appendChild(current);

		//
		//
		//
		const getUserMedia = !navigator.mediaDevices ?null :navigator.mediaDevices.getUserMedia;


		const showErr = () => {
			const dd = document.createElement('div');
			dd.id = 'err_mic'
			div.appendChild(dd);

			if (!WS.isSecure()) {
				const p = document.createElement('p');
				p.innerText = 'This browser session does not support recording, you will need to reload the secure HTTPS session to support browser based microphone recording to create new Outgoing Messages.';
				dd.appendChild(p);

				const but = document.createElement('button');
				but.innerHTML = '&#x21BB; Reload Now';
				dd.appendChild(but);
				but.onclick = (ev) => {
					WS.sendToSecure();
				};

			} else {
				dd.innerText = 'This browser does not support recording, please use another device that support browser based microphone recording to create new Outgoing Messages.';
			}
		};

		if (!getUserMedia) {
			showErr();

		} else {
			const meter = document.createElement('canvas');

			meter.style.width  = audioCanvas.w +'px';
			meter.style.height = audioCanvas.h +'px';

			const aud = new RecordAudio();
			aud.setup(meter);

			players.push(aud);


			{
				const opts = document.createElement('div');
				div.appendChild(opts);

				const but = document.createElement('button');
				but.className = 'record_stop';
				but.innerHTML = '<span class="red">&#x1F3A4;  Record</span>';

				this.aud = aud;

				const butOn = (ev) => {
					ev.stopPropagation();

					this.download.classList.add('hidden');
					this.stop.classList.add('hidden');

					navigator.mediaDevices
					  .getUserMedia({ audio:true })
					  .then((stream) => { if (dbg) console.log('media start'); aud.start(stream); })
					  .catch((err) => {
					  	aud.stop();
						opts.parentElement.removeChild(opts);
						meter.parentElement.parentElement.removeChild(meter.parentElement);

						if (typeof(err) === 'SecurityError') {
							if (dbg) console.log('not granted');
							return;
						}

					 	if (dbg) console.log('error :',err);

/*
						navigator.mediaDevices.enumerateDevices()
						.then(function(devices) {

							devices.forEach(function(device) {
							//	console.log('device: '+device.kind + " : " + device.label +" id:" + device.deviceId);
								console.log(JSON.stringify(device));
							});

							const supportedConstraints = navigator.mediaDevices.getSupportedConstraints();

							for (let constraint in supportedConstraints) {
								if (supportedConstraints.hasOwnProperty(constraint)) {
									console.log('constraint:'+constraint);
								} else {
							  		console.log('xconstraint:'+constraint);  	
								}
							}

						})
						.catch(function(err) {
						  console.log(err.name + ": " + err.message);
						});
*/

						showErr();
					});

					but.onclick = butOff;
					but.innerHTML = '&#x2589; Stop';
				};

				const butOff = (ev) => {
					ev.stopPropagation();
					this.aud.stop();

					but.onclick = butOn;
					but.innerHTML = '<span class="red">&#x1F3A4; Record</span>';

					this.download.href = URL.createObjectURL(this.aud.blob);
					this.download.classList.remove('hidden');

					this.stop.classList.remove('hidden');
				};

				but.onclick = butOn;
				opts.appendChild(but);
			}

			{
				const aud = document.createElement('div');
				div.appendChild(aud);

				aud.id = 'popup_ogm_aud'
				aud.appendChild(meter);

				aud.style.width  = meter.style.width;
				aud.style.height = meter.style.height;

				{
					const a = document.createElement('a');
					aud.appendChild(a);

					a.innerHTML = 'download';
					a.download  = 'audio.wav';
					this.download = a;

					a.classList.add('hidden');
				}
			}

			{
				const dd = document.createElement('div');
				div.appendChild(dd);

				this.stop = document.createElement('button');
				dd.appendChild(this.stop);

				this.stop.className = 'save hidden';
				this.stop.innerHTML = 'Save';
				this.stop.onclick = (ev) => {
					if (!confirm('Replace the current Outgoing Message with this new recording. Are you sure?')) return;

					if (dbg) console.log('save confirmed');
					this.close();

					const r = new FileReader();
					r.onload = (ev) => {
						const buf = ev.target.result;
						if (dbg) console.log('save len:'+buf.byteLength);

						ws.sendSaveCurrentOGM(line,buf);
					};
					r.readAsArrayBuffer(this.aud.blob);

/*
					this.aud.blob.arrayBuffer().then(buf => {
						if (dbg) console.log('save len:'+buf.byteLength);

						ws.sendSaveCurrentOGM(line,buf);
					});
*/
				};
			}
		}

		{
			const inp = document.createElement('input');
			inp.classList = 'done';
			inp.type = 'button';
			inp.value = 'Done';

			div.appendChild(inp);

			inp.onclick = (ev) => {
				if (dbg) console.log('done clicked');
				this.close();
			};
		}

		this.showPopup(div,'&#x260F; Outgoing Message');

		//
		// Adjust
		//
		if (dbg) {
			const h = div.getBoundingClientRect().height;
			console.log('popup height:'+h+' client:'+div.clientHeight+' offset:'+div.offsetHeight+' scroll:'+div.scrollHeight);
		}
	}













	//
	//
	//
	showLennyProfile(line) {
		if (dbg) console.log('showLennyProfile');

		const div = document.createElement('div');
		div.className = 'popup_inner';

		const fr = document.createElement('div');
		fr.classList = 'profiles_frame';
		div.appendChild(fr);

		const dd = document.createElement('div');
		dd.id = 'profile_choice';

		const tbl = document.createElement('table');
		dd.appendChild(tbl);
		div.appendChild(dd);


		//
		//
		//
		const activeProfile = document.createElement('span');
		const updateInfo = () => {
			activeProfile.innerText = line.cfg.lenny_profile;
		}

		{
			const dd = document.createElement('div');
			dd.classList = 'profile_lenny_img_fr';
			fr.appendChild(dd);

			{
				const img = document.createElement('img');
				img.src = 'img/lenny.png';
				dd.appendChild(img);
			}
			{
				const inf = document.createElement('div');
				dd.appendChild(inf);
				inf.style.position = 'relative';

				{
					const h1 = document.createElement('h1');
					const h2 = document.createElement('h2');
					inf.appendChild(h1);
					inf.appendChild(h2);
					h1.innerText = 'Active Profile: ';
					h1.appendChild(activeProfile);
					h2.innerText = 'Line '+line.idx;
				}

			// {
			// 	inf.innerHTML = '<h1>Line '+line.idx+'</h1><h2>Active Profile: <span>'+line.cfg.lenny_profile+'</span></h2>';
			// }

				updateInfo();

				{
					const inp = document.createElement('input');
					inp.type = 'button';
					inp.value = 'Add New Profile';
					inp.style.position = 'absolute'
					inp.style.bottom = '0';
					inp.style.right = '0';

					inf.appendChild(inp);

					inp.onclick = (ev) => {
						if (dbg) console.log('add clicked');
						new Popup().showLennyNewProfile(line,(reload) => { if (reload) load(); });
						this.close();
					};
				}
			}
		}

		const load = () => {

			while (0 < tbl.childNodes.length) tbl.removeChild(tbl.childNodes[0])

			const l = lenny.cfg.profiles;

			l.forEach((it) => {
				const tr = tbl.insertRow();

				{
					const inp = document.createElement('input');
					inp.type = 'radio';
					inp.id = 'profiles_inp';
					inp.name = 'profile_inp';
					inp.value = it;
					inp.checked = line.cfg.lenny_profile === it;

					const lbl = document.createElement('label');
					lbl.for = 'profiles_inp';
					lbl.innerText = it;

					const td = tr.insertCell();
					td.style.width = '80%';
					td.appendChild(inp);
					td.appendChild(lbl);

					lbl.onclick = (ev) => {
						if (inp.checked) return;
						if (dbg) console.log(it + ' clicked');
						inp.checked = true;

						line.cfg.lenny_profile = it;
						updateInfo();

						ws.sendSetConfig(line.idx,'lenny_profile_set',it);
					};
					inp.onchange = () => {
						if (line.cfg.lenny_profile === it) return;
						if (dbg) console.log(it + ' selected');

						inp.checked = true;

						line.cfg.lenny_profile = it;
						updateInfo();

						ws.sendSetConfig(line.idx,'lenny_profile_set',it);
					};

					tr.addEventListener('click',(ev) => { inp.onchange(); },supportsPassive);
				}

/*
				if ('Lenny' === it) {
					const td = tr.insertCell();
					td.style.width = '10%';
					td.style.height = '60px';

				} else
*/
				{
					const inp = document.createElement('button');

					inp.innerHTML = '&#x1F50E;';
//					inp.innerHTML = '&#x1F50D;';
//					inp.innerHTML = '&#x270E;';
					inp.style.margin = '6px';
					inp.style.paddingLeft = '10px';
					inp.style.paddingRight = '6px';

					const td = tr.insertCell();
					td.className = 'not_active';
					td.style.width = '10%';
					td.style.height = '60px';
					td.appendChild(inp);

					inp.onclick = (ev) => {
						ev.stopPropagation();
						if (dbg) console.log('edit :'+it +' clicked');
						new Popup().showLennyProfileEdit(it);
					};
				}

				if ('Lenny' === it) {
					const td = tr.insertCell();
					td.className = 'not_active';
					td.style.width = '10%';

				} else {
					const inp = document.createElement('button');
					inp.innerHTML = '&#x1F5D1;';
					inp.style.margin = '6px';
					inp.style.paddingLeft = '10px';
					inp.style.paddingRight = '6px';

					const td = tr.insertCell();
					td.className = 'not_active';
					td.style.width = '10%';
					td.appendChild(inp);

					inp.onclick = (ev) => {
						ev.stopPropagation();

						if (!confirm('Remove this Lenny profile and all its message files?  Are you sure?')) return;
						if (dbg) console.log('delete :'+it +' clicked');

						tr.parentElement.removeChild(tr);

						{
							const max = lenny.cfg.profiles.length;
							for (let i=0;i<max;i++) {
								if (it != lenny.cfg.profiles[i]) continue;

								lenny.cfg.profiles = lenny.cfg.profiles.slice(0,i).concat(lenny.cfg.profiles.slice(i +1));
								break;
							}
						}

						ws.sendProfileRemove(it);
					};
				}
			});

			{
				const tr = tbl.insertRow();
				const td = tr.insertCell();
				td.className = 'not_active done';
				td.colSpan = 3;
				td.style.textAlign = 'center';
				td.style.marginTop = '30px';

				{
					const inp = document.createElement('input');
					inp.classList = 'done';
					inp.type = 'button';
					inp.value = 'Done';

					td.appendChild(inp);

					inp.onclick = (ev) => {
						if (dbg) console.log('done clicked');
						this.close();
					};
				}
			}
		};

		load();

		//
		// Show
		//
		this.showPopup(div,'Lenny Profile',true);
	}


	//
	//
	//
	showLennyNewProfile(line,reload) {
		if (dbg) console.log('showLennyNewProfile');

		const div = document.createElement('div');
		div.className = 'popup_inner';

		const fr = document.createElement('div');
		fr.classList = 'profiles_frame';
		div.appendChild(fr);

		{
			const dd = document.createElement('div');
			dd.classList = 'profile_lenny_img_fr';
			fr.appendChild(dd);

			{
				const img = document.createElement('img');
				img.src = 'img/lenny.png';
				dd.appendChild(img);
			}
			{
				const inf = document.createElement('div');
				dd.appendChild(inf);
				inf.style.position = 'relative';

				{
					const h1 = document.createElement('h1');
				//	const h2 = document.createElement('h2');
					inf.appendChild(h1);
				//	inf.appendChild(h2);
					h1.innerText = 'Add New Profile';
				//	h2.innerText = 'Line '+line.idx;
				}
			}
		}

		const name = document.createElement('input');
		name.id = 'profile_name';
		name.type = 'text';
		name.maxLength = 20;
		name.placeholder = 'Unique Name';
		div.appendChild(name);

		{
			const dd = document.createElement('div');
			div.appendChild(dd);

			{
				const but = document.createElement('input');
				but.type = 'button';
				but.value = 'Create';
				but.style.margin = '10px';

				dd.appendChild(but);

				but.onclick = (ev) => {
					if (dbg) console.log('save clicked :'+name.value);

					//
					// validate
					//
					const v = name.value.trim();

					if (0 == v.length) {
						if (dbg) console.log('input value zero lenght')
						return;
					}

					//
					// add local, reload, add remote, close, next popup
					//
					lenny.cfg.profiles.push(v);
					if (reload) reload();
					ws.sendProfileAdd(v);

					this.close();
					new Popup().showLennyProfileEdit(v);

				};

				const bgColor = but.style.backgroundColor;
				const enable = () => {
					but.disabled = 0 == name.value.trim().length;
					but.style.backgroundColor = but.disabled ?'rgba(255,255,255,0.3)' :bgColor;
				};

				enable();
				name.onkeyup = () => { enable(); };
			}

			{
				const but = document.createElement('input');
				but.type = 'button';
				but.value = 'Cancel';

				dd.appendChild(but);

				but.onclick = (ev) => {
					if (dbg) console.log('done clicked');
					this.close();
				};
			}
		}

		//
		// Show
		//
		this.showPopup(div,'Lenny Profile',true);

		name.focus();
	}


	//
	//
	//
	showLennyProfileEdit(profile) {
		if (dbg) console.log('showLennyProfileEdit :'+profile);

		const div = document.createElement('div');
		div.className = 'popup_inner';
//		div.id = 'profiles';

		//
		const fr = document.createElement('div');
		fr.classList = 'profiles_frame';
		div.appendChild(fr);

		{
			const dd = document.createElement('div');
			dd.classList = 'profile_lenny_img_fr';
			fr.appendChild(dd);

			{
				const img = document.createElement('img');
				img.src = 'img/lenny.png';
				dd.appendChild(img);
			}
			{
				const inf = document.createElement('div');
				dd.appendChild(inf);
				inf.style.position = 'relative';

				{
					const h1 = document.createElement('h1');
					const h2 = document.createElement('h2');
					inf.appendChild(h1);
					inf.appendChild(h2);
					h1.innerHTML = 'Update Profile: <span>'+profile+'</span>';
					h2.innerText = 'Manage Messages';
				}
			}
		}


		//
		const players = [];

		//
		const stopPlayers = () => {
			players.forEach( (p) => { p.stop(); });
		};
		this.onclose = () => { stopPlayers(); }


		//
		const butDone = document.createElement('input');
		const butCancel = document.createElement('input');

		let dirty = false;
		const save = () => {
			if (dbg) console.log('save dirty:'+dirty);
			if (!dirty) return;

			const rows = tbl.childNodes[0].childNodes;
			const max = rows.length;
			let count = 0;
			for (let i=0;i<max;i++) {
				const tr = rows[i];
				if (!tr.player) continue;
	      		count++;

	      		const idx = i;
				const r = new FileReader();
				r.onload = (ev) => {
					const buf = ev.target.result;
					if (dbg) console.log('save idx:'+idx+' len:'+buf.byteLength);

					ws.sendSendNewProfileMsg(profile,tr.rowIndex);
		      		ws.websocket.send(buf);
				};
				r.readAsArrayBuffer(tr.player.blob);

/*
				tr.player.blob.arrayBuffer().then(buf => {
					if (dbg) console.log('save idx:'+i);

					ws.sendSendNewProfileMsg(profile,tr.rowIndex);
		      		ws.websocket.send(buf);
				});
*/
			}

			const files = lenny.profiles[profile];

			for (let i=count;i<files.length;i++) {
				if (dbg) console.log('remove idx:'+i);

				ws.sendProfileRemoveMsg(profile,i);
			}

			delete this['onclose'];
		};
		const setDirty = () => {
			if (dirty) return;
			dirty = true;
			butDone.value = 'Save Changes';
			butCancel.classList.remove('hidden');

		//	const save = this.onclose;
			this.onclose = () => {
				stopPlayers();
				return !confirm('Leave without saving changes?  Are you sure?');
			};
		};

		//
		const tbl = document.createElement('table');
		fr.appendChild(tbl);

		//
		const reindex = () => {
			const rows = tbl.childNodes[0].childNodes;
			const max = rows.length -2;
			for (let i=0;i<max;i++) {
				const tr = rows[i];
				tr.cells[0].innerText = 1+tr.rowIndex;
			}
		};


		//
		//
		//
		let trackCount = 0;
		const insertTrack = (buf) => {
			const tr = tbl.insertRow(trackCount);
			insertTrackRow(buf,tr);
		};

		const addTrack = (buf) => {
			const tr = tbl.insertRow();
			insertTrackRow(buf,tr);
		};


		//
		//
		//
		let dragRow = 0;

		const insertTrackRow = (buf,tr) => {

			//
			if ('Lenny' != profile) {
				tr.draggable = true;
				tr.ondragstart = (ev) => { dragRow = tr.rowIndex; };
				tr.ondragover = (ev) => { ev.preventDefault(); };
				tr.ondrop = (ev) => {
					if (tr.rowIndex == dragRow) return;
					const tb = tbl.childNodes[0];
					tb.insertBefore(tb.rows[dragRow],tb.rows[tr.rowIndex]);
	                //reindex();
	                setDirty();
				};
			}

			//
			const track = document.createElement('div');
			trackCount++;

			tr.innerHTML = '<th style="text-align:right;padding-right:10px">'+trackCount+'</th>';
			tr.insertCell().appendChild(track);

			track.width  = audioCanvas.w +'px';
			track.height = audioCanvas.h +'px';

			if ('Lenny' === profile) {
				const td = tr.insertCell();
				td.style.width = '10%';

			} else {

				const inp = document.createElement('button');
				inp.innerHTML = '&#x1F5D1;';
				inp.style.paddingLeft = '10px';
				inp.style.paddingRight = '6px';

				const td = tr.insertCell();
				td.appendChild(inp);

				inp.onclick = (ev) => {
					ev.stopPropagation();

					if (!confirm('Remove this Lenny message?  Are you sure?')) return;
				
					tr.parentElement.removeChild(tr);
					trackCount--;
	                setDirty();

					reindex();
				};
			}


			const p = new PlayableAudio(buf);
			p.hasOpts = false;
			p.setup(track,track.width,track.height);

			players.push(p);
			tr.player = p;

		//	tr.onclick = (ev) => { p.playToggle(); };
		};



		const showErr = (tr) => {
			const dd = document.createElement('div');
			dd.id = 'err_mic'
			div.appendChild(dd);

			if (!WS.isSecure()) {
				const p = document.createElement('p');
				p.innerText = 'This browser session does not support recording, you will need to reload the secure HTTPS session to support browser based microphone recording to create new Profile Messages.';
				dd.appendChild(p);

				const but = document.createElement('button');
				but.innerHTML = '&#x21BB; Reload Now';
				dd.appendChild(but);

				but.onclick = (ev) => {
					WS.sendToSecure();
				};

			} else {
				dd.innerText = 'This browser does not support recording, please use another device that support browser based microphone recording to create new Profile Messages.';
			}


			while (0 < tr.childNodes.length) tr.removeChild(tr.childNodes[0]);
			tr.classList = 'not_active';
			const td = tr.insertCell();
			td.colSpan = 3;

			td.appendChild(dd);
		};


		const addRecordButton = () => {

			if ('Lenny' === profile) {
				return;
			}

			//
			//
			//
			const getUserMedia = !navigator.mediaDevices ?null :navigator.mediaDevices.getUserMedia;

			if (!getUserMedia) {
				const tr = tbl.insertRow();
				showErr(tr);

			} else {

				const tr = tbl.insertRow();
				tr.classList = 'not_active';

				const td = tr.insertCell();
				td.colSpan = 3;

				const aud = document.createElement('div');
				aud.style.display = 'inline-block';
				aud.style.textAlign = 'center';

				const meter = document.createElement('canvas');
				meter.classList = 'green';
				aud.appendChild(meter);

				{
					aud.style.width  = audioCanvas.w +'px';
					aud.style.height = audioCanvas.h +'px';

					meter.style.width  = audioCanvas.w +'px';
					meter.style.height = audioCanvas.h +'px';
				}

				const raud = new RecordAudio();
				raud.setup(meter);
				this.aud = raud;
				players.push(raud);

				{
					const opts = document.createElement('div');
					div.style.textAlign = 'center';

					const but = document.createElement('button');
					but.className = 'record_stop';
					but.innerHTML = '<span class="red">&#x1F3A4; Record</span>';
					opts.appendChild(but);

					const butOn = (ev) => {
						ev.stopPropagation();
						if (dbg) console.log('record start');

						navigator.mediaDevices.getUserMedia({ audio:true })
						  .then((stream) => {
						  	if (dbg) console.log('media start');
						  	raud.start(stream);

						//	this.download.classList.add('hidden');
						//	this.save.classList.add('hidden');

							but.onclick = butOff;
							but.innerHTML = '&#x2589; Stop';
						  })
						  .catch((err) => {
						  	raud.stop();
							showErr(tr);

							if (typeof(err) === 'SecurityError') {
								if (dbg) console.log('not granted');
								return;
							}
						 	if (dbg) console.log(typeof(err)+' error :'+err);
						  }
						);
					};

					const butOff = (ev) => {
						ev.stopPropagation();
						if (dbg) console.log('record stop');

						raud.stop();

						but.onclick = butOn;
						but.innerHTML = '<span class="red">&#x1F3A4; Record</span>';

					//	this.download.href = URL.createObjectURL(this.aud.blob);
					//	this.download.classList.remove('hidden');

						this.save.classList.remove('hidden');

						//
						// Scroll down when showing button
						//
						fr.scrollIntoView(false);
						fr.scrollTop = fr.scrollHeight;
					};

					but.onclick = butOn;

					td.appendChild(opts);
					td.appendChild(aud);
				}

			/*
				{
					const aud = document.createElement('div');
					div.appendChild(aud);

					aud.id = 'popup_ogm_aud'
					aud.appendChild(meter);

					{
						const a = document.createElement('a');
						aud.appendChild(a);

						a.innerHTML = 'download';
						a.download  = 'audio.wav';
						this.download = a;

						a.classList.add('hidden');
					}
				}
			*/

				{
					this.save = document.createElement('button');
					this.save.classList = 'save';
					td.appendChild(this.save);

					this.save.classList.add('hidden');
					this.save.innerHTML = 'Save as new Message Clip';
					this.save.onclick = (ev) => {

						if (this.aud.active) return;

						//if (!confirm('Save as new Lenny profile message?')) return;
						//if (dbg) console.log('save confirmed');

						const r = new FileReader();
						r.onload = (ev) => {

							const buf = ev.target.result;
							if (dbg) console.log('onload len:'+buf.byteLength);

							this.save.classList.add('hidden');

							insertTrack(buf);
			                setDirty();

				      		this.aud.clear();
						};
						r.readAsArrayBuffer(this.aud.blob);

				/*
						this.aud.blob.arrayBuffer().then(buf => {
							if (dbg) console.log('save len:'+buf.byteLength);

							this.save.classList.add('hidden');

							insertTrack(buf);
			                setDirty();

				      		this.aud.clear();
						});
				*/

					};
				}
			}
		};


		const addDoneSaveCancelButtons = () => {
			butDone.classList = 'done';
			butDone.type = 'button';
			butDone.value = 'Done';

			butCancel.classList = 'done';
			butCancel.type = 'button';
			butCancel.value = 'Cancel';
			butCancel.classList.add('hidden');
			butCancel.style.marginLeft = '15px';

			{
				const tr = tbl.insertRow();
				tr.classList = 'not_active done';

				const td = tr.insertCell();
				td.colSpan = 3;
				td.appendChild(butDone);
				td.appendChild(butCancel);
			}

			butDone.onclick = (ev) => {
				if (dbg) console.log('done clicked');
				save();
				this.close();
			};
			butCancel.onclick = (ev) => {
				if (dbg) console.log('cancel clicked');
				delete this['onclose'];
				stopPlayers();
				this.close();
			};
		};

		//
		const get = (name) => {
			if (dbg) console.log('profile :'+name);	

			stopPlayers();
			while (0 < tbl.childNodes.length) tbl.removeChild(tbl.childNodes[0]);

			players.length = 0;

			{
				const save = lenny.eventProfiles;
				ws.sendGetProfiles();

				lenny.eventProfiles = (prs) => {
					lenny.eventProfiles = save;
					save(prs)

					const files = lenny.profiles[name];

					const next = (i) => {

						//
						// At last record, add record new button
						//
						if (i >= files.length) {
							addRecordButton();
							addDoneSaveCancelButtons();
							return;
						}

						const save = lenny.binaryCallback;
						lenny.binaryCallback = (buf) => {
							lenny.binaryCallback = save;

							addTrack(buf.buf.subarray(buf.offset,buf.buf.length - buf.offset));

							next(1+i)
						};

						ws.sendGetProfileMsg(files[i]);
					};

					next(0);
				}
			}

			//
		//	fr.style.height = (heightCalc - 150 +24 -60)+'px';
		//	fr.style.overflowY = 'scroll';
			fr.style.textAlign = 'center';
		};


		//
		//
		//
		get(profile);



		//
		// Show
		//
		this.showPopup(div,'Lenny Profile',true);
	}










	//
	//
	//
	showPopup(popup,title,close=true) {
		const div = this.getPopup(title,close);
		div.childNodes[0].appendChild(popup);

		if (portrait) popup.style.height = popupPortraitHeight +'px';

		document.body.appendChild(div);
	}

	getPopup(title,close=true) {

		const div = document.createElement('div');
		div.className = 'popup-box';

		if (title && 0 < title.length) {
			const left = document.createElement('div');
			left.className = 'popup-head-left';
			left.innerHTML = title;

			if (!close) {
				const head = document.createElement('div');
				head.className = 'popup-head';
				head.appendChild(left);

				const dclear = document.createElement('div');
				dclear.style.clear = 'both';
				head.appendChild(dclear);

				div.appendChild(head);

			} else {

				// 'x' closer
				const close = document.createElement('a');
				close.innerHTML = '&#10005;';
				close.onclick = (ev) => { this.close(); }
	
				const right = document.createElement('div');
				right.className = 'popup-head-right';
				right.appendChild(close);

				const dclear = document.createElement('div');
				dclear.style.clear = 'both';

				const head = document.createElement('div');
				head.className = 'popup-head';
				head.appendChild(left);
				head.appendChild(right);
				head.appendChild(dclear);
				div.appendChild(head);
			}
		}

		this.frame = document.createElement('div');
		this.frame.className = 'popup-frame';
		this.frame.appendChild(div);

		// Outside of modal closer
		this.frame.onclick = (ev) => { if (null != this.frame.parentElement) this.close(); };
		div.onclick = (ev) => { ev.stopPropagation(); };

		return this.frame;
	}
}





//
//
//

class PlayableAudio {

	constructor(ar) {
		this.array = new Uint8Array(ar,44);
		this.blob = new Blob([ar],{ type:'audio/wav' });

		this.hasOpts = true;
	}

	stop() {
		if (dbg) console.log('stop');

		if (this.aud) {
			if (dbg) console.log('stopping player');
			this.aud.pause();
			delete this['aud'];
			return;
		}
	}

	setup(parent,width,height) {
		//if (dbg) console.log('w:'+width+' h:'+height);

		const div = document.createElement('div');
		div.classList = 'audio_play';

		{
			const canvas = document.createElement('canvas');
			canvas.style.width = width;
			canvas.style.height = height;

		//	if (dbg) console.log('window.devicePixelRatio:'+window.devicePixelRatio);

			canvas.width = 2*parseInt(width);
			canvas.height = 2*parseInt(height);

			const ctx = canvas.getContext('2d');
			this.ctx = ctx;
			canvas.className = 'audio';
			div.appendChild(canvas);

			drawGraph(canvas,this.array);
			drawTimeSeconds(canvas,this.array.byteLength / 8000);

	      	drawTapToPlay(canvas);

			parent.appendChild(div);
			canvas.onclick = (ev) => {
				ev.stopPropagation();

				this.playToggle();
			};
		}

		if (this.hasOpts) {
			const opts = document.createElement('div');
			div.classList = 'audio_play_opts';
			div.appendChild(opts);
			{
				const a = document.createElement('a');
				opts.appendChild(a);
				a.innerHTML = 'download';
				a.download  = 'audio.wav';
				a.href      = URL.createObjectURL(this.blob);
			}
			if (this.ontrash) {
				const a = document.createElement('a');
				a.classList = 'trash';
				opts.appendChild(a);
				a.innerHTML = '&#x1F5D1;';
				a.onclick = (ev) => { if (dbg) console.log('trash'); this.ontrash(); };
			}
		}
	}

	playToggle() {

		if (this.aud) {
			if (dbg) console.log('stop');
			this.aud.pause();

			//floshes object
			this.aud.currentTime = 0;
			this.aud.load();

			delete this['aud'];
			return;
		}

		if (dbg) console.log('play');
		this.aud = new Audio(URL.createObjectURL(this.blob));
		this.aud.ontimeupdate = (ev) => {
		//	if (dbg) console.log('time:'+this.aud.currentTime);
			if (!this.aud) { if (dbg) console.log('aud is null'); return; }
			this.draw();
		}

		this.aud.onended = (ev) => {
			if (dbg) console.log('ended');
			drawGraph(this.ctx.canvas,this.array);
			drawTimeSeconds(this.ctx.canvas,this.array.byteLength / 8000,false);

			//floshes object
			this.aud.currentTime = 0;
			this.aud.load();

			delete this['aud'];
		}
		this.aud.play();
	}

	draw() {
		const ctx = this.ctx;

		{
			drawGraph(ctx.canvas,this.array);

			const cursor = Math.floor(ctx.canvas.width *(this.aud.currentTime / this.aud.duration));

			ctx.beginPath();
			ctx.moveTo(cursor,0);
			ctx.lineTo(cursor,ctx.canvas.height);
			ctx.lineWidth = 4;
			ctx.strokeStyle = '#ccc';
			ctx.stroke();
		}

		{
			drawTimeSeconds(ctx.canvas,this.array.byteLength / 8000,false);
			drawTimeSeconds(ctx.canvas,this.aud.currentTime,true);
		}

	}
}



//
//
//

class RecordAudio {

	constructor() {
		if (dbg) console.log('RecordAudio constructor');

	    this.chunks = [];
	    this.len = 0;
	}

	setup(canvas) {
		if (dbg) console.log('setup w:'+canvas.width+' h:'+canvas.height);

		this.ctx = canvas.getContext('2d');

		this.bufferSize = canvas.width;
		this.buffer = new Array(this.bufferSize).fill(0);
		this.cursor = 0;

		this.active = false;
		this.activeStart = null;
	}

	clear() {
		this.cursor = 0;
		this.buffer.fill(0);

		this.ctx.save();
		this.ctx.clearRect(0,0,this.ctx.canvas.width,this.ctx.canvas.height);
		this.ctx.restore();
	}

	draw() {
		const ctx = this.ctx;
		ctx.save();

		// empty canvas
		ctx.clearRect(0,0,ctx.canvas.width,ctx.canvas.height);

		// draw audio waveform
		ctx.strokeStyle = '#fff';
		const mid = (ctx.canvas.height /2);

		const max = this.bufferSize;
		for (let i=0;i<max;i++) {
			const v = 0.5 * (ctx.canvas.height * this.buffer[max > this.cursor ?i :((this.cursor + i) %this.bufferSize)]);
			ctx.beginPath();
			ctx.moveTo(i,mid - v);
			ctx.lineTo(i,mid + v);
			ctx.stroke();
		}

		// draw time line
		if (this.bufferSize > this.cursor) {
			ctx.beginPath();
			ctx.moveTo(this.cursor,0);
			ctx.lineTo(this.cursor,ctx.canvas.height);
			ctx.strokeStyle = '#bbb';
			ctx.stroke();
		}

		// draw middle line
		ctx.beginPath();
		ctx.moveTo(  0,(ctx.canvas.height /2)); 
		ctx.lineTo(ctx.canvas.width,(ctx.canvas.height/2));
		ctx.strokeStyle = '#bbb';
		ctx.stroke();

		drawTimeMS(ctx.canvas,new Date() - this.activeStart);

		ctx.restore();
	}

	animate() {
		if (!this.active) return;
		requestAnimationFrame(() => {this.animate(); });
		this.draw();
	}


	//
	// Audio
	//

	start(stream) {	
      	if (dbg) console.log('RecordAudio start');

      	//
      	parent.onclick = null;
		this.chunks.length = 0;
      	this.len = 0;
      	delete this['blob'];


	 	//
	 	//
	 	//
		const audioContext = getAudioContext();
		if (dbg) console.log('audioContext sampleRate :'+audioContext.sampleRate);


		//
		// Optimize?
		//
		this.sampleRate = audioContext.sampleRate;
	//	const bufferSize = 8000 == this.sampleRate ?512 :1024;
		const bufferSize = 1024;

        this.mediaStreamSource = audioContext.createMediaStreamSource(stream);
        this.processor         = audioContext.createScriptProcessor(bufferSize,1,1);  //256, 512, 1024, 2048, 4096, 8192, 16384

        this.mediaStreamSource.connect(this.processor);
        this.processor.connect(audioContext.destination);


        //
        //
        //
      	this.clear();
      	this.activeStart = new Date();
      	this.active = true;
		this.animate();

		const lastSilenceChunks = [];


        this.processor.onaudioprocess = (data) => {
        	const len = data.inputBuffer.length;

			const d = data.inputBuffer.getChannelData(0);
			const a = new Float32Array(d);

			//
			// Peak Meter calc
			//
			{
				let i = 0;
				{
					let max = 0;

					for (let j=0;i<len && j<256;i++,j++) {
						const v = Math.abs(a[i]);
						if (max > v) continue;
						max = v;
					}

				/*
					if (0 == this.chunks.length) {

						if (0.05 > max) {
							if (dbg) console.log('skipping volume:'+parseInt(100*max)+' len:'+len);
							lastSilenceChunks.push(a);
							return;
						}

						if (0 < lastSilenceChunks.kength) {
							if (dbg) console.log('add silence chunk from count:'+lastSilenceChunks.kength);

							const c = lastSilenceChunks[lastSilenceChunk.length -1];
							this.chunks.push(c);
							this.len += c.length;

							lastSilenceChunks.length = 0;
						}
					}
				*/

					this.buffer[this.cursor++ % this.bufferSize] = max;
				
					//if (dbg) console.log('volume:'+parseInt(100*max)+' len:'+len);
				}
			}

			this.chunks.push(a);
			this.len += len;

			setTimeout( () => { this.draw(); },0);
        };
	}

	stop() {	

      	if (this.aud) {
	      	if (dbg) console.log('recorder stopping player');
	      	this.aud.pause();
	      	delete this['aud'];
      		return;
      	}

      	if (!this.active) return;
      	if (dbg) console.log('recorder stopping');

      	this.active = false;
      	delete this['activeStart'];

      	if (this.processor) {
			this.processor.disconnect();
			delete this['processor'];
		}
		if (this.mediaStreamSource) {
		    this.mediaStreamSource.mediaStream.getTracks().forEach( (tr) => { tr.stop(); });
			this.mediaStreamSource.disconnect();
			delete this['mediaStreamSource'];
		}

		//
		// Create one buffer
		//
		const d = new Float32Array(this.len);
		{
			const cmax = this.chunks.length;
			for (let i=0,j=0;i<cmax;i++) {
				d.set(this.chunks[i],j);
				j += this.chunks[i].length;
			}
		}

		//
		// Resample
		//
		const dd = 8000 == this.sampleRate ?d :downsample(d,this.sampleRate,8000);

		//
		// Create wav file
		//
		const enc = encodeWAV(dd);
		this.blob = new Blob([enc], { type:'audio/wav' });

		//
		// Wrap into array for display
		//
		const ar = new Uint8Array(enc.buffer,44);
      	drawGraph(this.ctx.canvas,ar);
      	drawTimeSeconds(this.ctx.canvas,ar.byteLength /8000);
      	drawTapToPlay(this.ctx.canvas);


		//
		// PLAYER
		//
		this.ctx.canvas.onclick = (ev) => {
			ev.stopPropagation();
			if (!this.blob) return;

			if (this.aud) {
				if (dbg) console.log('play stop');
				this.aud.pause();
				delete this['aud'];
				return;
			}

			const s = document.createElement('source');
			s.src = URL.createObjectURL(this.blob);
			s.type = 'audio/wav';

			this.aud = document.createElement('audio');
			this.aud.controls = false;
			this.aud.appendChild(s);

			this.aud.onerror = (ev) => { if (dbg) console.log('Error ' + this.aud.error.code + '; details: ' + this.aud.error.message); this.aud = null; }
			this.aud.onprogress = (ev) => {
				drawTimeSeconds(this.ctx.canvas,this.aud.duration,false);
				drawTimeSeconds(this.ctx.canvas,this.aud.currentTime,true);
			}
			this.aud.ontimeupdate = (ev) => {
				if (null == this.aud) return;

				const ctx = this.ctx;
				drawGraph(ctx.canvas,ar);

				{
					const cursor = Math.floor(ctx.canvas.width *(this.aud.currentTime / this.aud.duration));
					ctx.beginPath();
					ctx.moveTo(cursor,0);
					ctx.lineTo(cursor,ctx.canvas.height);
					ctx.lineWidth = 4;
					ctx.strokeStyle = '#ccc';
					ctx.stroke();
				}

				drawTimeSeconds(ctx.canvas,this.aud.duration,false);
				drawTimeSeconds(ctx.canvas,this.aud.currentTime,true);
			}
			this.aud.onended = (ev) => {
				if (dbg) console.log('ended');

				drawGraph(this.ctx.canvas,ar);
				if (this.aud) {
					drawTimeSeconds(this.ctx.canvas,this.aud.duration,false);
				}
		      	drawTapToPlay(this.ctx.canvas);

				delete this['aud'];
			}

			this.aud.play();
		};
	}
}


//
//
//

class PlayAudioStream {
	constructor() {
		if (dbg) console.log('PlayAudioStream constructor');
		this.running = true;

		this.save_binaryCallback = lenny.binaryCallback;
		lenny.binaryCallback = (buf) => { this.audioIn(buf); };
	}

	stop() {
      	if (dbg) console.log('PlayAudioStream stop');
      	this.running = false;

		lenny.binaryCallback = this.save_binaryCallback;
		delete this['save_binaryCallback'];

      	if (this.processor) {
      		if (this.gain) {
      			this.gain.disconnect();
			    delete this['gain'];
			}
		  //this.processor.disconnect();
		    delete this['processor'];
		}

	/*
		// is this ever needed?
		if (this.audioContext) {
		    this.audioContext.close();
		    delete this['audioContext'];
		}
	*/

	}

	audioIn(buf) {
		//if (dbg) console.log('audioIn running:'+this.running);
		if (!this.running) return;

		this.lastBuf = Date.now(); 

		if (null == this.audioContext) {
			if (dbg) console.log('startStreaming');

			this.audioSilence = 0;
			this.audioActive = true;
			this.audioInBuffs = [];
	
			this.audioContext = getAudioContext();

		    this.gain = this.audioContext.createGain();
		    this.gain.connect(this.audioContext.destination); // connect gain to speakers

		    this.processor = this.audioContext.createScriptProcessor(1024,1,1);
		    this.processor.connect(this.gain);

		  	//this.gain.gain.value = 1;
			//this.gain.gain.linearRampToValueAtTime(1,this.audioContext.currentTime() + 0.3);

		    this.processor.onaudioprocess = (data) => {
		    	const last = Date.now() - this.lastBuf;

		    	if (5000 < last) {
		    		if (dbg) console.log('auto stop after 5 seconds of no buffers');
		    		this.stop();
		    		return;
		    	}

		    	//if (dbg) console.log('last:'+last+' time:'+this.audioContext.getOutputTimestamp().contextTime.toFixed(2)+' performance:'+this.audioContext.getOutputTimestamp().performanceTime.toFixed(2));

				const d = data.outputBuffer.getChannelData(0);

		    	let samples = 0;
		    	const max = this.audioInBuffs.length;
		    	for (let i=0;i < max;i++) {
		    		const b = this.audioInBuffs[i];
		    		samples += b.buf.length - b.offset;
		    	}

		    	if (d.length > samples) {
		    		const dmax = d.length;
					for (let j=0;j < dmax;j++) d[j++] = 0;

					this.audioSilence++;

		    		if (dbg) console.log('SILENCE :'+this.audioSilence+' chans:'+data.outputBuffer.numberOfChannels);

				} else {
					this.audioSilence = 0;

					let j = 0;
					while (0 < this.audioInBuffs.length && j < d.length) {
			    		const b = this.audioInBuffs[0];
						const s = b.buf;

						const bmax = b.buf.length;
						const dmax = d.length;

						for (let i=b.offset;b.offset < bmax && j < dmax;b.offset++) {
							d[j++] = ((s[i++] &0xFF) -0x7F) /0xFF; // 8bit unsigned -> 16bit float
						}

						if (b.offset >= b.buf.length) {
							this.audioInBuffs.shift();
						}
					}
				}
		    };
		}

		this.audioInBuffs.push(buf);
	}
}



//
//
//

class WS {
	constructor() {
		const url = new URL(document.location);
		this.ip = url.host;
		this.wsurl = (WS.isSecure() ?'wss://' :'ws://') +this.ip +'/ws';
	}

	static sendToUnsecure() {
		const url = new URL(document.location);
		const l = 'http://' + url.hostname+':'+(parseInt(url.port) -1)+'?skip_start';
		if (dbg) console.log('sendToUnsecure '+l);
		document.location = l;
		return;		
	}

	static sendToSecure() {
		const url = new URL(document.location);
		const l = 'https://' + url.hostname+':'+(1+ parseInt(url.port))+'?skip_start';
		if (dbg) console.log('sendToSecure '+l);
		document.location = l;
		return;		
	}

	static isSecure() {
		return 'https:' === location.protocol;
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
//		this.websocket.onerror   = (ev) => { if (dbg) console.error('websocket onError '+ev); this.hasError = true; };
//		this.websocket.ontimeout = (ev) => { if (dbg) console.error('websocket onTimeout '+ev); this.hasError = true; }; 
		this.websocket.onclose   = (ev) => { if (dbg) console.log('websocket onClose'); delete this['websocket']; lenny.eventDisconnected(); };
		this.websocket.onmessage = (ev) => { if (typeof ev.data === 'object') { this.onBinaryMessage(ev); } else { this.onTextMessage(ev); } };
	}

	onTextMessage(ev) {
		//if (dbg) console.log('onTextMessage json:'+ev.data);
//		setTimeout( () => {
			try {
				const rq = JSON.parse(ev.data);

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
	sendLogUpload() {
		if (dbg) console.log('sendLogUpload');
		const r = { a:'log_upload' };
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
	sendCreateLicense(days=60) {
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
		const r = { a:'tad_message_set',t:'add',l:line.idx };
  		this.websocket.send( JSON.stringify(r) );
  		this.websocket.send(buf);
	}
	sendRemoveCurrentOGM(line) {
		//if (dbg) console.log('sendRemoveCurrentOGM l:'+line);
		const r = { a:'tad_message_set',t:'del',l:line.idx };
  		this.websocket.send( JSON.stringify(r) );
	}



	sendSendNewProfileMsg(profile,idx) {
		//if (dbg) console.log('sendSendNewProfileMsg p:'+profile+' idx:'+idx);
		const r = { a:'profile_message_set',n:profile,i:idx };
  		this.websocket.send( JSON.stringify(r) );
	}

}





//
//
//
function getDaysFromCalls(list) {
	const sh = {};
	{
		let lastDay = null;
		let lastDayMsgs = [];

		const max = list.length;
		for (let i=max-1;i>=0;i--) {
			const c = list[i]
			const d = parseHistDateTime(c.date,c.time);

			let dd = d.getDate(),mm = d.getMonth() +1; //January is 0!
			if (dd < 10) dd = '0' + dd;
			if (mm < 10) mm = '0' + mm;

			const day = d.getFullYear()+''+mm+''+dd;

			if (null == lastDay) {
				lastDay = day;
				lastDayMsgs.push(c);
			} else
			if (day != lastDay) {
				sh[lastDay] = lastDayMsgs;

				lastDay = day;

				lastDayMsgs = [];
				lastDayMsgs.push(c);
			} else {
				lastDayMsgs.push(c);
			}
		}

		if (0 < lastDayMsgs.length) {
			sh[lastDay] = lastDayMsgs;
		}
	}

	return sh;
}



function formatCallTimeDayOfWeek(call) {

	const days   = [ 'Sun','Mon','Tue','Wed','Thu','Fri','Sat' ];
	const d = parseHistDateTime(call.date,call.time);
	const hr = d.getHours(),mn = d.getMinutes(),se = d.getSeconds();

	let str = days[d.getDay()]+' ';
	if (10 > hr) str += '0';
	str += hr + ':';
	if (10 > mn) str += '0';
	str += mn;

	return str;
}

function formatCallTime(call) {

	const d = parseHistDateTime(call.date,call.time);
	const hr = d.getHours(),mn = d.getMinutes(),se = d.getSeconds();

	let str = '';
	if (10 > hr) str += '0';
	str += hr + ':';
	if (10 > mn) str += '0';
	str += mn;

	return str;
}

function formatCallDateTime(call) {

	let str = formatCallTime(call);

	if (!call.ms) {
	} else {
		let mins = Math.floor(call.ms /60000);
		let secs = Math.floor((call.ms /1000) %60);
		if (10 > mins) mins = '0'+mins;
		if (10 > secs) secs = '0'+secs;
		str += ' ('+mins+':'+secs+')';
	}
	return str;
}

//
function formatDateMMDD(yr,mo,da) { return formatDate(new Date(yr,mo-1,da)); }
function formatHistDate(date    ) { return formatDate(parseHistDate(date)); }
function formatDate(date) {
	const months = [ 'January','February','March','April','May','June','July','August','September','October','November','December' ];
	const days   = [ 'Sunday','Monday','Tuesday','Wednesday','Thursday','Friday','Saturday' ];
	return days[date.getDay()]+', '+months[date.getMonth()]+' '+date.getDate();
}
function formatDateShort(date) {
	const months = [ 'Jan','Feb','Mar','Apr','May','June','July','Aug','Sep','Oct','Nov','Dec' ];
	const days   = [ 'Sun','Mon','Tue','Wed','Thu','Fri','Sat' ];
	return days[date.getDay()]+', '+months[date.getMonth()]+' '+date.getDate();
}
function parseHistDate(   date) { return new Date(date.substr(0,4),date.substr(4,2)-1,date.substr(6,2)); }
function parseHistDateTime(date,time) {
	if (!/^(\d){8}$/.test(date)) { if (dbg) console.log('parseHistDate failed date:'+date+' time:'+time); return ''; }
	return new Date(date.substr(0,4),date.substr(4,2)-1,date.substr(6,2),time.substr(0,2),time.substr(2,2),0);
}

//
function drawGraph(canvas,array) {
	//if (dbg) console.log('drawGraph');

	const ctx = canvas.getContext('2d');
	const w = ctx.canvas.width,h = ctx.canvas.height;

	ctx.save();
	ctx.clearRect(0,0,w,h);
	ctx.strokeStyle = '#bbb';
	ctx.lineWidth = 1;

	const inc = Math.floor(array.byteLength / w);
	const mid = h/2;

	const max = array.byteLength;
	for (let i=0;i<max && i<w;i++) {
		const vv = array[inc*i]; // 0 - 255

		let v = 0.5* (h * ((vv - 127) / 127.0)); // -1.0 - 1.0
		v *= 4;

		ctx.beginPath();
		ctx.moveTo(i, mid - v);
		ctx.lineTo(i, mid + v);
		ctx.stroke();
	}

	// middle line
	ctx.beginPath();
	ctx.moveTo(  0,(h/2));
	ctx.lineTo(w,(h/2));
	ctx.strokeStyle = '#bbb';
	ctx.stroke();

	ctx.restore();
}

function drawTimeMS(canvas,millis) {
//	if (dbg) console.log('drawTime ms:'+millis);

	const ctx = canvas.getContext('2d');
	ctx.font = '18px Courier';
	ctx.fillStyle = '#fff';

	let s = (millis /1000).toFixed(0);
	let ms = ((millis %1000) /10).toFixed(0).substr(0,2);

	if (10 > s) s = '0'+s;
	if (2 > ms.length) ms += '0';

	const tm = s+':'+ms;
	const met = ctx.measureText(tm);
	ctx.fillText(tm,1,met.actualBoundingBoxAscent +met.actualBoundingBoxAscent/2);
}

function drawTimeSeconds(canvas,seconds,elapsed = false) {

	const ctx = canvas.getContext('2d');
	ctx.font = '18px Courier';
	ctx.fillStyle = '#fff';

	let ms = ((1000 * seconds) %1000).toFixed(0).substr(0,2);
	let s = Math.floor(seconds) %60;
	let m = Math.floor(seconds /60);

	let tm = '';
	if (10 > m) tm += '0';
	tm += m+':';
	if (10 > s) tm += '0';
	tm += s;

	const met = ctx.measureText(tm);
	if (elapsed) {
		ctx.fillText(tm,1,met.actualBoundingBoxAscent +met.actualBoundingBoxAscent/2);
	} else {
		ctx.fillText(tm,ctx.canvas.width - met.width,met.actualBoundingBoxAscent +met.actualBoundingBoxAscent/2);
	}

//	ctx.fillText(tm,10,ctx.canvas.height-10);
//	ctx.fillText(s+'.'+ms,cavas.width-180,canvas.height-100);
}

function drawTapToPlay(canvas) {
	const ctx = canvas.getContext('2d');
	ctx.save();
	ctx.font = '18px Courier';
	ctx.fillStyle = '#aaa';
	ctx.fillText(isMobile ?'TAP to PLAY' :'CLICK to PLAY',10,20);
	ctx.restore();
}


//
//
//
function get_css(selector,style) {
	for (let i=0;i<document.styleSheets.length;i++) {
		const mysheet = document.styleSheets[i];
		const myrules = mysheet.cssRules ? mysheet.cssRules : mysheet.rules;

		for (let j=0;j<myrules.length;j++) {
			if (myrules[j].selectorText && myrules[j].selectorText.toLowerCase() === selector) {
				return myrules[j].style[style];
			}
		}
	}
	return '';
}


//
// caching audio context fixes safari bug of only five new AudioContext before failure
//
var ac = null;
function getAudioContext() {
	if (!ac) {
	 	//
	 	//
	 	//
		const AudioContext = window.AudioContext // Default
		    || window.webkitAudioContext // Safari and old versions of Chrome
		    || false; 

		if (!AudioContext) {
		    // Web Audio API is not supported
		    alert('Sorry, but the Web Audio API is not supported by your browser. Please, consider upgrading to the latest version or downloading Google Chrome or Mozilla Firefox');
		    return;
		}

		const isChrome = /Chrome/.test(navigator.userAgent) && /Google Inc/.test(navigator.vendor);

		if (isChrome) {
			ac = new AudioContext( { sampleRate:8000 } );
		} else {
			ac = new AudioContext();
		}
		//if (dbg) console.log('audioContext sampleRate :'+ac.sampleRate);
	}
	return ac;	
}

function encodeWAV(samples,sampleRate =8000) {
	const len = samples.length;
	if (dbg) console.log('encodeWAV r:'+sampleRate+' len:'+len);

	const b = new ArrayBuffer(44 + len);
	const d = new DataView(b);

	d.setUint8( 0, 'R'.charCodeAt(0));
	d.setUint8( 1, 'I'.charCodeAt(0));
	d.setUint8( 2, 'F'.charCodeAt(0));
	d.setUint8( 3, 'F'.charCodeAt(0));

	d.setUint32(4, 8 + len,true); // file length

	d.setUint8( 8, 'W'.charCodeAt(0));
	d.setUint8( 9, 'A'.charCodeAt(0));
	d.setUint8(10, 'V'.charCodeAt(0));
	d.setUint8(11, 'E'.charCodeAt(0));

	d.setUint8(12, 'f'.charCodeAt(0));
	d.setUint8(13, 'm'.charCodeAt(0));
	d.setUint8(14, 't'.charCodeAt(0));
	d.setUint8(15, ' '.charCodeAt(0));

	d.setUint32(16, 16,true);                   // format chunk length
	d.setUint16(20,  1,true);                   // sample format (raw)
	d.setUint16(22,  1,true);                   // channel count
	d.setUint32(24, sampleRate,true);           // sample rate
	d.setUint32(28, sampleRate,true);           // byte rate (sample rate * block align)
	d.setUint16(32,  1,true);                   // block align (channel count * bytes per sample)
	d.setUint16(34,  8,true);                   // bits per sample
	
	d.setUint8(36, 'd'.charCodeAt(0));
	d.setUint8(37, 'a'.charCodeAt(0));
	d.setUint8(38, 't'.charCodeAt(0));
	d.setUint8(39, 'a'.charCodeAt(0));

	d.setUint32(40, len,true);       // data chunk length

	const smax = samples.length;
	for (let i=0,j=44;i<smax;i++,j++) {
		d.setUint8(j,(samples[i] *0x7F) +0x7F,true);
	}

	return d;
}

// buffer is a Float32Array
function downsample(buffer,fromSampleRate,toSampleRate) {
	if (dbg) console.log('downsample len:'+buffer.length+' fr:'+fromSampleRate+' to:'+toSampleRate);

   const rat = Math.round(fromSampleRate / toSampleRate);
   const nlen = Math.round(buffer.length / rat);
   const r = new Float32Array(nlen);

   let outOff = 0,inOff = 0;

   while (outOff < nlen) {

       const grp = Math.round((outOff +1) *rat);

       let accum = 0, count = 0;

       for (let i=inOff;i<grp && i<buffer.length; i++) {
           accum += buffer[i];
           count++;
       }

       r[outOff] = accum / count;

       outOff++;
       inOff = grp;
   }

   return r;
}

function getHostOS() {
	const macosPlatforms   = [ 'Macintosh','MacIntel','MacPPC','Mac68K' ];
	const windowsPlatforms = [ 'Win32','Win64','Windows','WinCE' ];
	const iosPlatforms     = [ 'iPhone','iPad','iPod' ];

	const userAgent = window.navigator.userAgent;
	const platform = window.navigator.platform;

	let os = null;

	if (-1 !== macosPlatforms  .indexOf(platform)) { os = 'MacOS'; } else
	if (-1 !== iosPlatforms    .indexOf(platform)) { os = 'iOS'; } else 
	if (-1 !== windowsPlatforms.indexOf(platform)) { os = 'Windows'; } else 
	if (/Android/.test(userAgent)                ) { os = 'Android'; } else 
	if (/Linux/  .test(platform)                 ) { os = 'Linux'; } else
	                                               { os = 'Unknown'; }
	return os;
}

function requestFullScreen() {
/*
	try {
		if (document.documentElement.requestFullscreen) {
			document.documentElement.requestFullscreen();
		} else
		if (document.documentElement.mozRequestFullScreen) {
			document.documentElement.mozRequestFullScreen();
		} else
		if (document.documentElement.webkitRequestFullscreen) {
			document.documentElement.webkitRequestFullscreen();
		} else
		if (document.documentElement.msRequestFullscreen) {
			document.documentElement.msRequestFullscreen();
		}
		screen.orientation.lock('portrait-primary');
	} catch (err) { if (dbg) console.log('supportsPassive err:'+err); }
*/
}

function getUrlParams(url) {
	const r = {};

	const u = decodeURI(url);
	if (typeof u != 'string') {
		if (dbg) console.log('getParamsFromUrl failed type:'+(typeof u));
	} else {
		const s = u.split('?');
		if (s && 1 < s.length) {
			const ps = s[1].split('&');
			if (ps && ps.length) {
				ps.map(pr => {
					const p = pr.split('=')
					const k = p[0];
					const v = p[1];
					r[k] = v;
				})
			}
		}
	}
	return r;
}

