/*


*/
'use strict';


//
//
//
var dbg = true;
var showCommentForm = false;



//
//
//

function postLennyTroll(url,info,cb) {
	try {
		const rq = new XMLHttpRequest();
		rq.timeout = 4000;

		rq.onload = (ev) => { console.log('found'); };
		rq.onerror = (ev) => { console.log('failed'); }
		rq.ontimeout = (ev) => { console.log('timeout'); };
		rq.onreadystatechange = () => {
			if (4 != rq.readyState) return;
			console.log('status:'+rq.status);

			if (cb) cb(2 == rq.status /100);
		};

		rq.open('POST',url);
		rq.setRequestHeader('Content-Type','application/json;charset=UTF-8');
		rq.send(JSON.stringify(info));


	} catch (err) {
		console.log('err:'+err);
	}
}




document.addEventListener('DOMContentLoaded',() => {

//	document.getElementById('theme_dark').disabled = false;
//	document.getElementById('theme_lite').disabled = true;

/*
	const t = localStorage.getItem('theme');
	if ('dark' === t) {
		document.getElementById('theme_dark').disabled = false;
		document.getElementById('theme_lite').disabled = true;
	} else {
	// if ('lite' === t) {
		document.getElementById('theme_dark').disabled = true;
		document.getElementById('theme_lite').disabled = false;
//	} else {
//		document.getElementById('theme_dark').disabled = true;
//		document.getElementById('theme_lite').disabled = true;
	}
*/


	//
	//
	//

/*
	console.log('size outer '+window.outerWidth+'x'+window.outerHeight+' inner:'+window.innerWidth+'x'+window.innerHeight+' r:'+window.devicePixelRatio);
	window.addEventListener('resize',(ev) => {
		console.log('resize outer '+window.outerWidth+'x'+window.outerHeight+' inner:'+window.innerWidth+'x'+window.innerHeight+' r:'+window.devicePixelRatio);
	},false);
*/

	//if (document.getElementById('home')) 
	{
		ui = new UI();
		ui.handleParms();

		//loadHomeArticles();
		loadSupportFAQ();

		load2();
	}// else
	if (document.getElementById('online_support')) {
		showCommentForm = true;		
	}

	loadSupportContact();
	//loadFollowLinks();
});




function load2() {

	const vid = document.getElementById('license_add_vid');
	if (!vid) return;

	vid.src = 'img/ss/license_add.mp4';
	vid.style = '';

	const p = vid.parentElement;
	const cvr_fr = document.createElement('div');
	cvr_fr.className = 'play_cover_fr';
	p.appendChild(cvr_fr);

	const cvr = '<div class="play_cover"><div class="play">&#x25b6;</div></div>'
	cvr_fr.innerHTML = cvr;

	vid.addEventListener('pause',() => {
		console.log('paused');
		if (0 == cvr_fr.childElementCount) cvr_fr.innerHTML = cvr;
	});
	vid.addEventListener('playing',() => {
		console.log('playing');
		while (0 < cvr_fr.childElementCount) cvr_fr.removeChild(cvr_fr.children[0]);
	});

	const toggle = () => {
		if (vid.playing) {
			vid.pause();
		} else {
		//	if (0 == vid.currentTime) vid.currentTime = 1;
			vid.play();
		}
	};

	cvr_fr.onclick = (ev) => { ev.stopPropagation(); toggle(); };
	vid.onclick    = (ev) => { ev.stopPropagation(); toggle(); };
}




//
//
//
var ui = null;

class UI {
	constructor() {

		this.div_home     = document.getElementById('home');
		this.div_about    = document.getElementById('about');
		this.div_hardware = document.getElementById('hardware');
		this.div_download = document.getElementById('download');
		this.div_start    = document.getElementById('start');
		this.div_support  = document.getElementById('support');

		this.menu_check    = document.getElementById('menu_check');

/*
		this.menu_home     = document.getElementById('menu_home');
		this.menu_lenny    = document.getElementById('menu_lenny');
		this.menu_hardware = document.getElementById('menu_hardware');
		this.menu_download = document.getElementById('menu_download');
		this.menu_start    = document.getElementById('menu_start');
		this.menu_support  = document.getElementById('menu_support');

		this.menu_home    .onclick = (ev) => { this.show_home(); };
		this.menu_lenny   .onclick = (ev) => { this.show_lenny(); }
		this.menu_hardware.onclick = (ev) => { this.show_hardware(); }
		this.menu_download.onclick = (ev) => { this.show_download(); }
		this.menu_start   .onclick = (ev) => { this.show_start(); }
		this.menu_support .onclick = (ev) => { this.show_support(); }

		{
			const f = document.getElementById('footer');
			f.innerHTML = 'Copyright &copy; '+new Date().getFullYear()+' <a href="https://sladesys.com">Slade Systems</a>';
		}
*/

	}

	handleParms() {

/*
		const u = decodeURI(window.location);
		if (typeof u != 'string') {
			if (dbg) console.log('not a string error failed type:'+(typeof u));
			return;
		} 
		{
			const s = u.split(/[?#]/);
			if (s && 1 < s.length) {
				switch (s[1]) {
					default:
					case 'home'    : this.show_home(); break;
					case 'about'   : this.show_lenny(); break;
					case 'hardware': this.show_hardware(); break;
					case 'download': this.show_download(); break;
					case 'start'   : this.show_start(); break;
					case 'faq'     : this.show_support(); break;
					case 'support' : this.show_support(); break;

					case 'comment' : showCommentForm = true; break;
				}
			}
		}
		{
			const s = u.split('?');
			const p = (1 == s.length ?u :s[1]).split('&');
			for (let i=0;i<p.length;i++) {
				let v = p[i];
				if (v.indexOf('#')) {
					v = v.substr(0,v.indexOf('#'));
				}
				switch (v) {
					default: break;
					case 'comment' : showCommentForm = true; break;
				}				
			}			
		}
*/
	}

/*
	menu_hide() {
		this.menu_check.checked = false;
		this.div_home     .classList.add('hidden');
		this.div_about    .classList.add('hidden');
		this.div_hardware .classList.add('hidden');
		this.div_download .classList.add('hidden');
		this.div_start    .classList.add('hidden');
		this.div_support  .classList.add('hidden');

		window.scrollTo(0,0);
	}

	show_home() {
		console.log('home');
		this.menu_hide();
		this.div_home.classList.remove('hidden');
		document.title = 'Lenny - The Telemarketing Troll';
	}
	show_lenny() {
		console.log('lenny');
		this.menu_hide();
		this.div_about.classList.remove('hidden');
		document.title = 'How Lenny Works';
	}
	show_hardware() {
		console.log('hardware');
		this.menu_hide();
		this.div_hardware.classList.remove('hidden');
		document.title = 'Lenny Do It Yourself Hardware';
	}
	show_download() {
		console.log('download');
		this.menu_hide();
		this.div_download.classList.remove('hidden');
		document.title = 'Lenny Download';
	}
	show_start() {
		console.log('start');
		this.menu_hide();
		this.div_start.classList.remove('hidden');
		document.title = 'Lenny Getting Started';
	}
	show_support() {
		console.log('support');
		this.menu_hide();
		this.div_support.classList.remove('hidden');
		document.title = 'Lenny Support';
	}
*/

}





//
//
//
/*
function loadFollowLinks() {

	const div = document.getElementById('follow');
	if (!div) return;

	const d = document.createElement('div');
	const h1  = document.createElement('h1');
	h1.innerText = 'Follow Lenny';

	div.appendChild(h1);
	div.appendChild(d);

	list_Follows.forEach( (it) => {
		const img = document.createElement('img');
		img.src = it.img;
		img.target = 'lenny';
		img.alt = it.name;

		const dd = document.createElement('div');
		dd.style.display = 'inline-block';

		dd.appendChild(img);
		d.appendChild(dd);

		dd.onclick = (ev) => {
			console.log('clicked:'+it.name);
			window.open(it.url,'lenny');
		};
	});
}
*/


//
//
//

/*
function loadHomeArticles() {
	const d = document.getElementById('home_body_side_list');
	d.appendChild( getArticles(list_Articles) );
}

function getArticles(links) {
	const div = document.createElement('div');
	div.className = 'articles';

	links.forEach( (it) => {

		const date = document.createElement('p');
		date.className = 'date';
		date.innerText = formatDate(it.date);

		const title = document.createElement('p');
		title.className = 'title';
		title.innerText = it.title;

		const a = document.createElement('a');
		a.href = it.url;
		a.target = 'lenny';

		const d = document.createElement('div');
		d.appendChild(date);
		d.appendChild(title);
		a.appendChild(d);

		const fr = document.createElement('div');
		fr.className = 'frame';
		fr.appendChild(a);

		div.appendChild(fr);
	});

	return div;
}

function formatDate(date) {
	const months = [ 'Jan','Feb','Mar','Apr','May','June','July','Aug','Sep','Oct','Nov','Dec' ];
	const days   = [ 'Sun','Mon','Tue','Wed','Thu','Fri','Sat' ];
	const d = new Date(date.substr(0,4),date.substr(4,2)-1,date.substr(6,2));
	return days[d.getDay()]+', '+months[d.getMonth()]+' '+d.getDate()+','+d.getFullYear();
}
*/

function displayTime(audioTime) {
	const v = Math.round(audioTime);
	const sec = Math.floor(v %60);
	const min = Math.floor(v /60);
	return min + ':' +(10 >sec ?('0'+sec) :sec);
}






//
//
//
function playHomeAudio(div,audio) {

	const s = div.getElementsByClassName('audio_clip_time')[0];

	let aud = div.getElementsByClassName('audio_ctrl');
	if (0 == aud.length) {
		const a = document.createElement('audio');
		a.className = 'audio_ctrl';

		a.src = audio;
		a.controls = false;

		a.addEventListener('loadedmetadata',(ev) => {
			console.log('loadedmetadata duration:'+a.duration);
			s.innerHTML = '0:00 / '+displayTime(a.duration);
		});

		a.addEventListener('timeupdate',(ev) => {
			console.log('timeupdate current:'+a.currentTime+' duration:'+a.duration);
			s.innerHTML = ''+displayTime(a.currentTime)+' / '+displayTime(a.duration);
		});

		a.addEventListener('ended',(ev) => {
			console.log('ended duration:'+a.duration);
			s.innerHTML = '0:00 / '+displayTime(a.duration);
		});

		div.insertBefore(a,div.childNodes[0]);
		aud = a;

	} else {
		aud = aud[0];
	}

	if (aud.paused) {
		aud.play();
	} else {
		aud.pause();
		aud.currentTime = 0;
	}
}




//
//
//
function loadSupportContact() {
	if (!showCommentForm) return;

	const s = document.getElementById('online_support');
	if (s) {
		{
			const h2 = document.createElement('h2');
			h2.innerText = 'Online Support';
			s.appendChild(h2);
		}
		s.appendChild(getCommentForm(
			null,
			(info) => {
				postLennyTroll('js/?cmnt',info,(success) => {
					if (!success) {
					} else {
					}
				})
			})
		);
		return;
	}
}

function loadSupportFAQ() {

	const faq = document.getElementById('faq');
	if (!faq) return;

	const ul = document.createElement('ul');
	faq.appendChild(ul);
	ul.style.listStyle = 'none';

	const add = (it) => {
		const l = document.createElement('li');
		l.className = 'question';
		ul.appendChild(l);

	//	l.style = ':before { content:"+";margin-right:4px;}';
	//	l.style = 'list-style-image:url(\'...\');';

		l.innerHTML = it.q;

		const d = document.createElement('div');
		l.appendChild(d);
		d.className = 'answer hidden';
	//	d.innerHTML = it.a;
	//	d.innerHTML = '<div>'+it.a+'</div>';

		const toggle = () => {
			if (!d.classList.contains('hidden')) {
				d.classList.add('hidden');
			} else {
				d.classList.remove('hidden');
			}
		};
		l.onclick = (ev) => { ev.stopPropagation(); toggle(); };
		//d.onclick = (ev) => { ev.stopPropagation(); toggle(); };


		if (it.vid) {

			const w = Math.min(80,(window.innerWidth - 50 - 30));

			const addVid = (src) => {
				const afr = document.createElement('div');
				afr.className = 'screen_vid';

				afr.style.width = w+'px';

				const vid = document.createElement('video');
				vid.currentTime = 2;
				vid.src = src;
				//d.style.width = '80px';
				//d.style.height = '160px';
				d.style.minHeight = '170px';
				vid.style.width = '80px';
				vid.style.height = '142px';
				vid.controls = false;
				vid.crossorigin = false;

				afr.appendChild(vid);
				d.appendChild(afr);

				const cvr_fr = document.createElement('div');
				cvr_fr.className = 'play_cover_fr';
				afr.appendChild(cvr_fr);


				// Play : 	&#x25b6;  &#9654;
				const cvr = '<div class="play_cover"><div class="play">&#x25b6;</div></div>'
				cvr_fr.innerHTML = cvr;

				vid.addEventListener('pause',() => {
					console.log('paused');
				//	cvr.classList.remove('hidden');
				//	cvr.style.diplay = 'block';

					if (0 == cvr_fr.childElementCount) cvr_fr.innerHTML = cvr;
				});
				vid.addEventListener('playing',() => {
					console.log('playing');
				//	cvr.classList.add('hidden');
				//	cvr.style.diplay = 'none';;

					while (0 < cvr_fr.childElementCount) cvr_fr.removeChild(cvr_fr.children[0]);
				});
				vid.addEventListener('ended',() => {
					console.log('ended');
				});

				cvr_fr.onclick = (ev) => {
					ev.stopPropagation();

					if (vid.playing) {
						vid.pause();
					} else {
						if (0 == vid.currentTime) vid.currentTime = 2;
						vid.play();
					//	cvr.classList.add('hidden');
					}
				};

				const a1 = document.createElement('a');
				a1.href = src;
				a1.target = 'lenny';
				a1.style.fontSize = '20px';
				a1.style.padding = '2px 10px 2px 10px';
				a1.innerHTML = '&#x1F50E;';
				afr.appendChild(a1);
			};

			addVid(it.vid);

			const p = document.createElement('p');
			p.innerHTML = it.a;
			d.appendChild(p);

		} else

		if (it.imgs && 0 < it.imgs.length) {

			const fr = document.createElement('div');
			fr.className = 'screen_shots';

			const l = it.imgs ? it.imgs.length :1;
			const w = Math.min(80,(window.innerWidth - 50 - (30 *l)) / l);

			const addImg = (img) => {
				const a1 = document.createElement('a');
				a1.href = img.src;
				a1.target = 'lenny';

				const afr = document.createElement('div');
				afr.className = 'screen_shot';
				afr.style.width = w+'px';

				const im = document.createElement('img');
				im.src = img.src;
				im.innerText = img.t;
			//	im.style.width = '80px';
			//	im.style.height = 'auto';
			//	im.style.width = 'auto';
			//	im.style.height = '160px';

				afr.appendChild(im);

				if (img.t && 0 < img.t.length) {
					const t = document.createElement('div');
					t.innerText = img.t;
					afr.appendChild(t);
				}

				a1.appendChild(afr);
				fr.appendChild(a1);
			};

			if (it.imgs) {
				it.imgs.forEach( (it) => { addImg(it); });
			} 

			{
				const dd = document.createElement('div');
				dd.style = 'margin:10px 0 0 0;padding:0';

				{
					const p = document.createElement('p');
					p.innerHTML = it.a;
					dd.appendChild(p);
				}

				dd.appendChild(fr);
				d.appendChild(dd);
			}

		} else {

			const p = document.createElement('p');
			p.innerHTML = it.a;
			d.appendChild(p);

		}

	};

	list_FAQ.forEach( (it) => { add(it); });	
}











//
//
//
function onDownload(href) {

	const pop_fr = document.createElement('div');
	pop_fr.className = 'popup-frame';

	close = () => { if (pop_fr.parentElement) pop_fr.parentElement.removeChild(pop_fr); }

	// Outside of modal = closes
	pop_fr.onclick = (ev) => { close(); };


	//
	//
	//
	const div = document.createElement('div');
	pop_fr.appendChild(div);

	// Inside of modal
	div.onclick = (ev) => { ev.stopPropagation(); };

	div.className = 'popup-box';

	{
		const fr = document.createElement('div');
		//fr.className = 'popup-inner';
		fr.style.overflowX = 'scroll';

		const ifr = document.createElement('div');
		ifr.className = 'popup-inner';
		//ifr.style.overflowX = 'scroll';


		const hdr = document.createElement('div');
		hdr.style.display = 'inline-block';
		hdr.style.minWidth = '100%';
		hdr.style.height = '80px';
		hdr.style.backgroundColor = '#ddd';
		hdr.style.textAlign = 'left';
		hdr.innerHTML = 
'<div style="display:inline-block;padding:20px 10px;font-size:22px;color:#444">Join the Community</div>'+
'<div style="float:right;"><img style="width:100px;height:74px" src="img/lenny_side.png"/></div>'
		;
//		hdr.innerHTML = '<div style="display:inline-block;padding:20px 10px;font-size:22px;color:#444">Join the Community</div><div style="display:inline-block;float:right;"><img style="width:auto;height:74px" src="img/lenny_side.png"/></div>';
		ifr.appendChild(hdr);

		//div.appendChild(fr);
		ifr.appendChild(fr);
		div.appendChild(ifr);

		{
			const clr = document.createElement('div');
			clr.style.clear = 'both';
			fr.appendChild(clr);
		}

		const dd = document.createElement('div');
		dd.className = 'download_contact';
		dd.style.width = '100%';
		dd.style.height = Math.floor((window.innerHeight *0.8) -74)+'px';
		fr.appendChild(dd);

		const dl = document.createElement('div');
		const dr = document.createElement('div');
		dl.className = 'download_contact_txt';
		dr.className = 'download_contact_side';

		dd.appendChild(dr);
		dd.appendChild(dl);

		{
			const fr = document.createElement('div');

			{
				const t = document.createElement('div');
				t.textAlign = 'center';
				t.style.fontSize = '22px';
				t.style.fontWeight = 'bold';
				t.innerText = 'Get your Free Lenny Download';

				fr.appendChild(t);
			}
			{
				const t = document.createElement('div');
				t.style.fontSize = '12px';
				t.style.marginLeft = '20px';
				t.style.marginRight = '20px';
				t.style.textAlign = 'left';
				t.innerHTML =
'<p>'+
'An email with your link to download this Lenny version will be sent to your Email Inbox.'
+'</p><p>'+
'Please fill in the required <big><b>*</b></big> fields and complete CAPTCHA helping avoid the evil RoBOTs.'
+'</p>'
				;
				fr.appendChild(t);
			}

			fr.appendChild(getContactForm('Download Info','Email Download Link',(info) => {
				info.f = href;
				postLennyTroll('js/?dl',info,(success) => {
					if (!success) {
						alert('Failed creating your Download Link, has one already been create?');
						return;
					}
					alert('Check your Email Inbox for your Download Link');
					close();
				});
			}));
			dr.appendChild(fr);
		}
		{
			const fr = document.createElement('div');
			fr.style.textAlign = 'right';
			fr.style.padding = '10px';

			const inp = document.createElement('input');
			inp.style.margin = '10px';

			inp.style.color = '#888';
			inp.type = 'button';
			inp.value = 'Back';

			const info = getDownloadInfo();

			fr.appendChild(inp);
			info.appendChild(fr);
			dl.appendChild( info );

			inp.onclick = (ev) => {
				ev.stopPropagation();
				close();
			};
		}
	}
	document.body.appendChild(pop_fr);
}


function getDownloadInfo() {
	const div = document.createElement('div');

	const dd = document.createElement('div');
//	dd.style.padding = '6px';

	{
		const d = document.createElement('div');
		d.style.fontSize = '12px';
		d.style.marginLeft = '20px';
		d.style.marginRight = '20px';
		d.style.textAlign = 'left';
		d.innerHTML =
'<p><b>Hello from the Lenny Team!</b></p>' +
'<p>We offer FREE Evaluation Licenses during this Beta Release Period wanting your feedback to make Lenny better for everyone except the Telemarketing and Robo Callers.</p>' +
'<p>Help us grow the Lenny Community with your support.</p>'
		;
		dd.appendChild(d);
	}


	{
		const fr = document.createElement('div');
		fr.style.width = '100%';
		fr.style.backgroundColor = '#ccc';
		fr.style.marginBottom = '6px';
		fr.style.padding = '4px';
		fr.style.textAlign = 'center';

		const h1 = document.createElement('h2');
		h1.innerText = 'Lenny - The Telemarketing Troll';

		fr.appendChild(h1);
		dd.appendChild(fr);
	}

	{
		const fl = document.createElement('div');
		fl.style.display = 'grid';
//		fl.style.gridTemplateColumns = 'repeat(auto-fill,140px)';//'repeat(3,1fr)';
		fl.style.gridTemplateColumns = 'repeat(3,1fr)';
		fl.style.alignItems = 'stretch';

		{
			const fr = document.createElement('div');
			fr.style.textAlign = 'center';

			fr.appendChild(fl);
			dd.appendChild(fr);
		}

		const addMsg = (it) => {
			const fr = document.createElement('div');
			fr.style.display = 'flex';
			fr.style.alignItems = 'stretch';
			fr.style.margin = '3px';
			fr.style.padding = '2px';
			fr.style.textAlign = 'left';
//			fr.style.backgroundColor = '#ccc';
			fr.style.border = '1px solid #ccc';
			fr.style.borderRadius = '50%';

			const dd = document.createElement('div');
			dd.style.width = '100%';
			dd.style.margin = '4px';
//			dd.style.minHeight = '100px';
			dd.style.lineHeight = '1em';
			dd.style.color = '#888';
			dd.innerHTML = 

'<div style="text-align:center;font-size:1.5em;padding:.5em 0 10px 0;">'+it.chr+'</div>' +
'<div style="text-align:center;font-size:1em;">'+it.txt+'</div>' +
'<div style="text-align:center;font-size:0.7em;margin:6px 0 6px 0;line-height:1.1em">'+it.msg+'</div>'

			;

			fr.appendChild(dd);
			fl.appendChild(fr);
		};

		const l = [
			{ chr:'&#x1F6AB;',txt:'Stop Robocalls'      ,msg:'It Really Works!' },
			{ chr:'&#x1F37F;',txt:'Trolls Telemaketers' ,msg:'Fun to Listen To!' },
			{ chr:'&#x1F4F1' ,txt:'Smartphone Enabled'  ,msg:'A Smart Answering Machine' },
			{ chr:'&#x1F512;',txt:'Secured'             ,msg:'On Raspberry Pi with HTTPS + TLS1.3' },
			{ chr:'&#x1F5FD;',txt:'Built in the USA'    ,msg:'Supported locally' },
			{ chr:'&#x2B50;' ,txt:'Lenny just Works'    ,msg:'AWESOME!' },
			{ chr:'&#x1F48B;',txt:'Share with Others'   ,msg:'FUN!' },
			{ chr:'&#x1F44D;',txt:'Support the Team'    ,msg:'Add your Comments' },
			{ chr:'&#x1F6A7;',txt:'Help Grow Lenny'     ,msg:'THANK YOU!!!' }
		];

		l.forEach( (it) => { addMsg(it); });
	}

/*
	if (false) {
		const fr = document.createElement('div');
		dd.appendChild(fr);
		fr.innerHTML = 
			'<span style="font-size:88px">'
			+'  &#x2706; &#x260F; &#x262F &#x263c; &#x2744; &#x273C;  &#x273D; &#x2714; &#x260E; &#x1F91A;  &#x1F6AB;  &#x1F6A7;  &#x1F596;  &#x1F576;  &#x1F525; &#x1F37F;  &#x1F512;  &#x1F513;  &#x1F4F1;  &#x1F48B;  &#x1F47D;  &#x1F453;  &#x1F44D;  &#x1F3C1; &#x1F37A; &#x1F378; &#x1F310;  &#x1F30E; &#x1F197; &#x1F195; &#x1F192; &#x1F193; &#x2B50;  &#x274C;  &#x23F3;  &#x23F0;   &#x231B;'
			+'    &#x1f5FD;'
			+'  &#x1f57e; &#x1f57f; &#x1f580; &#x1f5E8; &#x1f5E9; &#x1f5EA; &#x1f5EB; '
			+' </span>';
	}
*/

	div.appendChild(dd);
	return div;
}





//
//
//


function getContactForm(title,submitTxt,submitCallback =null) {
	const div = document.createElement('div');
	div.id = 'contact_form';

	{
		const fs = document.createElement('fieldset');
		fs.style.margin = '10px';
		fs.style.display = 'inline-block';
		div.appendChild(fs);

		{
			const l = document.createElement('legend');
			fs.appendChild(l);
			l.innerText = title;
		}

		const form = document.createElement('div');
		fs.appendChild(form);

		const tbl = document.createElement('div');
		form.appendChild(tbl);
		tbl.className = 'contact';

		const msgFName = document.createElement('input');
		const msgLName = document.createElement('input');
		const msgCName = document.createElement('input');
		const msgEmail = document.createElement('input');
		const msgText  = document.createElement('textarea');

		{
			const d = document.createElement('label');
			d.innerHTML = 'First Name <big><b>*</b></big>';
			msgFName.type = 'text';
			msgFName.autocomplete = 'fname given-name';
			msgFName.required = 'true';
			tbl.appendChild(d);
			tbl.appendChild(msgFName);

			setTimeout( () => { msgFName.focus(); },20);
		}
		{
			const d = document.createElement('label');
			d.innerHTML = 'Last Name <big><b>*</b></big>';
			msgLName.type = 'text';
			msgLName.autocomplete = 'lname family-name';
			msgLName.required = 'true';
			tbl.appendChild(d);
			tbl.appendChild(msgLName);
		}
		{
			const d = document.createElement('label');
			d.innerText = 'Company Name';
			msgCName.type = 'text';
			msgCName.autocomplete = 'organization';
			tbl.appendChild(d);
			tbl.appendChild(msgCName);
		}
		{
			const d = document.createElement('label');
			d.innerHTML = 'Email <big><b>*</b></big>';
			msgEmail.type = 'text';
			msgEmail.autocomplete = 'email';
			msgEmail.required = 'true';
			tbl.appendChild(d);
			tbl.appendChild(msgEmail);
		}
		{
			const d = document.createElement('label');
			d.innerText = 'Comments';
			msgText.rows = 5;
			msgText.placeholder = 'How did you hear about Lenny';
			tbl.appendChild(d);
			tbl.appendChild(msgText);
		}

		const validate = (txt) => {
			if (0 == msgFName.value.trim().length) {
				txt.style.color = '#f44';
				txt.innerText = 'First Name is empty';
				msgFName.focus();
				return false;
			}
			if (0 == msgLName.value.trim().length) {
				txt.style.color = '#f44';
				txt.innerText = 'Last Name is empty';
				msgLName.focus();
				return false;
			}
			{
				const re = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;

		 		if (!re.test(String(msgEmail.value).toLowerCase())) {
					txt.style.color = '#f44';
					txt.innerText = 'Invalid email format, please try again';
					msgEmail.focus();
					return false;
				}
			}

			txt.style.color = '#4a4';
			txt.innerText = 'Sending ...';

			const info = { fn:msgFName.value.trim(),ln:msgLName.value.trim(),cn:msgCName.value.trim(),e:msgEmail.value.trim(),t:msgText.value.trim() };

			if (submitCallback) submitCallback(info);

			msgText.value = '';
			txt.innerText = 'Sent';
		};

		addCaptcha(submitTxt,fs,validate);

		return div;
	}
}




function getCommentForm(title,submitCallback) {
	const div = document.createElement('div');
	div.id = 'comment_form';

	{
		const fs = document.createElement('fieldset');
		fs.style.margin = '10px';
		div.appendChild(fs);

		if (title) {
			const l = document.createElement('legend');
			fs.appendChild(l);
			l.innerText = title;
		}

		const form = document.createElement('div');
		fs.appendChild(form);

		const tbl = document.createElement('div');
		form.appendChild(tbl);
		tbl.style.display = 'table';
		tbl.className = 'comment';

		let msgName = null,msgEmail = null,msgText = null;

		{
			const tr = document.createElement('div');
			tr.style.display = 'table-row';
			tbl.appendChild(tr);
			{
				const td = document.createElement('div');
				td.style.display = 'table-cell';
				td.style.whiteSpace = 'nowrap';
				tr.appendChild(td);

				td.innerText = 'Your Name:';
			}
			{
				const td = document.createElement('div');
				td.style.display = 'table-cell';
				td.style.width = '100%';
				tr.appendChild(td);

				msgName = document.createElement('input');
				msgName.type = 'text';
				td.appendChild(msgName);
				msgName.style.width = '96%';

				msgName.placeholder = 'Lenny User';
			}
		}

		{
			const tr = document.createElement('div');
			tr.style.display = 'table-row';
			tbl.appendChild(tr);
			{
				const td = document.createElement('div');
				td.style.display = 'table-cell';
				td.style.whiteSpace = 'nowrap';
				tr.appendChild(td);

				td.innerText = 'Your Email:';
			}
			{
				const td = document.createElement('div');
				td.style.display = 'table-cell';
				td.style.width = '100%';
				tr.appendChild(td);

				msgEmail = document.createElement('input');
				msgEmail.type = 'text';
				td.appendChild(msgEmail);
				msgEmail.style.width = '96%';

				msgEmail.placeholder = 'name@host.com';
			}
		}

		{
			const tr = document.createElement('div');
			tr.style.display = 'table-row';
			tbl.appendChild(tr);
			{
				const td = document.createElement('div');
				td.style.display = 'table-cell';
				td.style.whiteSpace = 'nowrap';
				tr.appendChild(td);

				td.innerText = 'Your Message:';
			}
			{
				const td = document.createElement('div');
				td.style.display = 'table-cell';
				tr.appendChild(td);

				msgText = document.createElement('textarea');
				msgText.style.width = '96%';
				td.appendChild(msgText);
				msgText.rows = 5;

				msgText.placeholder = 'How much fun Lenny is ...';
			}
		}


		const validate = (txt) => {
			if (4 > msgName.value.trim().length) {
				txt.style.color = '#aa4';
				txt.innerText = 'Name should be more than 4 characters';
				msgName.focus();
				return false;
			}
			{
				const re = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;

		 		if (!re.test(String(msgEmail.value).toLowerCase())) {
					txt.style.color = '#aa4';
					txt.innerText = 'Invalid email format, please try again';
					msgEmail.focus();
					return false;
				}
			}
			if (4 > msgText.value.trim().length) {
				txt.style.color = '#aa4';
				txt.innerText = 'Message should be more than 4 characters';
				msgText.focus();
				return false;
			}

			console.log('submit email comment');
			txt.style.color = '#4a4';
			txt.innerText = 'Sending message ... ';

			const info = { e:msgEmail.value.trim(),n:msgName.value.trim(),t:msgText.value.trim() };

			if (submitCallback) submitCallback(info);

			txt.innerText = 'Message sent';
			msgText.value = '';

			return true;
		};

		addCaptcha('Send Now',fs,validate);

		return div;
	}
}






//
//
//

var captchaRetry =0;

function addCaptcha(submitTxt,form,validate) {

	const captcha = new Captcha();

	let captchaFrame = null,captchaFill = null,captchaClear = null;
	let close = null;

	{
		const tr = document.createElement('div');
		tr.id = 'cha_box';
		form.appendChild(tr);

		const inp = document.createElement('input');
		tr.appendChild(inp);
		inp.type = 'checkbox';
		const t = document.createElement('span');
		tr.appendChild(t);
		t.innerText = 'I am not a robot!';

		const show = () => {
			if (inp.checked) {
				captchaFrame.classList.remove('hidden');
				//setTimeout(captchaFill,1000);
				captchaFill();
			} else {
				captchaFrame.classList.add('hidden');
				captchaClear();
			}				
		};

		inp.addEventListener('click',(ev) => {
			console.log('inp click');
			ev.stopPropagation();
			show();
		});
		tr.addEventListener('click',(ev) => {
			console.log('tr click');
			ev.stopPropagation();
			inp.checked = !inp.checked;
			show();
		});
		close = () => { inp.checked = false; show(); }
	}

	{
		captchaFrame = document.createElement('div');
		captchaFrame.id = 'cha';
		captchaFrame.className = 'hidden';
		form.appendChild(captchaFrame);

		const fs = document.createElement('fieldset');
		captchaFrame.appendChild(fs);

		const l = document.createElement('legend');
		l.innerText = 'Are you a Human?';
		fs.appendChild(l);

		const dd = document.createElement('div');
		dd.id = 'cha_img';
		fs.appendChild(dd);

		const cap = document.createElement('canvas');
		cap.width = 130; cap.height = 30;

		dd.appendChild(cap);

		//
		//
		//
		const txtStart = 'Enter text from CAPTCHA image above';

		const txt = document.createElement('div');
		const inp = document.createElement('input');
		inp.id = 'cha_inp';
		inp.type = 'text';
		inp.maxLength = 7;

		captchaClear = () => {
			const ctx = cap.getContext('2d');
			ctx.save();
			ctx.clearRect(0,0,cap.width,cap.height);
			ctx.restore();

			txt.style.color = '#666';
			txt.innerText = txtStart;
			inp.value = '';
		};

		captchaFill = () => {
			const retryValue = ++captchaRetry;

			txt.style.color = '#666';
			txt.innerText = txtStart;

			const write = (str) => {
				const ctx = cap.getContext('2d');
				ctx.save();
				ctx.clearRect(0,0,cap.width,cap.height);

				ctx.font = '16px Courier';
				ctx.fillStyle = '#888';

				const t = str;
				ctx.fillText(t,3,20);
				ctx.restore();
			};

			const c = captcha.get();
			const s = c.split('');
			let str = '';
			const writeChar = (idx) => { write(str += s[idx] ); };

			for (let i=0;i<s.length;i++) {
				const idx = i;
				setTimeout(() => {
					if (retryValue == captchaRetry) writeChar(idx);
				},500* idx);
			}
		};

		cap.addEventListener('click',(ev) => {
			console.log('cap click');
			ev.stopPropagation();

			captchaRetry++;

			txt.style.color = '#000';
			txt.innerText = 'Please wait for new CAPTCHA';

			const ctx = cap.getContext('2d');
			ctx.save();
			ctx.clearRect(0,0,cap.width,cap.height);
			ctx.restore();

			//setTimeout(captchaFill,1000);
			captchaFill();
		});

		{
			const dd = document.createElement('div');
			dd.id = 'cha_fr';
			txt.id = 'cha_msg';

			txt.style.color = '#666';
			txt.innerText = txtStart;


			fs.appendChild(dd);
			dd.appendChild(txt);
			dd.appendChild(inp);
		}

		{
			const tr = document.createElement('div');
			tr.id = 'cha_but';
			fs.appendChild(tr);

			{
				const td = document.createElement('div');
				tr.appendChild(td);

				const but = document.createElement('input');
				td.appendChild(but);

				but.type = 'button';
				but.value = submitTxt;

				but.onclick = (ev) => {
					ev.preventDefault();

					if (!captcha.check(inp.value)) {
						console.log('captcha invalid');

						txt.style.color = '#f44';
						txt.innerText = 'CAPTCHA text entered is not valid';
						return;
					}

					if (!validate(txt)) {
						return;
					}

					close();
				};
			}
		}
	}
}


//
//
//
class Captcha {

	get() {
	    const alpha = new Array(
			'A','B','C','D','E','F','G','H','I','J','K','L','M','N',  'P',    'R','S','T','U','V','W','X','Y','Z',
			    'b','c','d',    'f','g','h',    'j','k',    'm','n',  'p','q','r','s','t',    'v','w','x','y','z', 
			'2','3','4','5','6','7','8','9'
		);

		const a = alpha[Math.floor(Math.random() * alpha.length)];
		const b = alpha[Math.floor(Math.random() * alpha.length)];
		const c = alpha[Math.floor(Math.random() * alpha.length)];
		const d = alpha[Math.floor(Math.random() * alpha.length)];
		const e = alpha[Math.floor(Math.random() * alpha.length)];
		const f = alpha[Math.floor(Math.random() * alpha.length)];
		const g = alpha[Math.floor(Math.random() * alpha.length)];

		this.code = a+''+b+''+c+''+d+''+e+''+f+''+g
		return a+' '+b+' '+c+' '+d+' '+e+' '+f+' '+g;
	}

	check(value) {
	//	const v = value.split(' ').join('');
		const v = value.replace(/ /g,'');
		return v == this.code;
	}
}

