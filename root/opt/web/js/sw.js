/*


*/
'use strict';


/*

Client.postMessage()


*/

/*
addEventListener('fetch',ev => {
  ev.waitUntil(async function() {

    // Exit early if we don't have access to the client.  Eg, if it's cross-origin.
    if (!ev.clientId) return;

    // Get the client.
    const client = await clients.get(ev.clientId);

    // Exit early if we don't get the client. Eg, if it closed.
    if (!client) return;

    // Send a message to the client.
    client.postMessage({
      msg: "Hey I just got a fetch from you!",
      url: event.request.url
    });
   
  }());
});



function svcSend(msg) {
    const client = await clients.get(event.clientId);
    // Exit early if we don't get the client.
    // Eg, if it closed.
    if (!client) return;
    	client.postMessage(msg);
}

navigator.serviceWorker.addEventListener('message',function(ev) {
	console.log('service worker message: '+ev.data.msg,event.data.url);
});

*/


/*
self.addEventListener('notificationclick', function(event) {
  console.log('On notification click: ', event.notification.tag);
  event.notification.close();

  // This looks to see if the current is already open and
  // focuses if it is
  event.waitUntil(clients.matchAll({
    type: "window"
  }).then(function(clientList) {
    for (var i = 0; i < clientList.length; i++) {
      var client = clientList[i];
      if (client.url == '/' && 'focus' in client) {
        client.focus();
        break;
      }
    }
    if (clients.openWindow)
      return clients.openWindow('/');
  }));
});
*/






const CACHE_VERSION = 'Lenny_v1';

const CACHE_URLS = [
	'/index.html',
	'/favicon.ico',
	'/l.webmanifest',
	'/sitemap.xml',

	'/js/l.js',
	'/js/sw.js',

	'/css/lt.css',
	'/css/ltm.css',
	'/css/ltd.css',
	'/css/ltl.css',
	'/css/img.css',

	'/img/lenny.png',
	'/img/lenny_180.png',
	'/img/lenny_192.png',
	'/img/lenny_512.png',
	'/img/lenny_toon.png',

	'/img/star0.svg',
	'/img/star1.svg',

	'/img/ss_cals.png',
	'/img/usr_modem.png',

	'/img/logo_apple.png',
	'/img/logo_arch.png',
	'/img/logo_centos.png',
	'/img/logo_debian.png',
	'/img/logo_fedora.png',
	'/img/logo_gentoo.png',
	'/img/logo_linux.png',
	'/img/logo_mint.png',
	'/img/logo_opensuse.png',
	'/img/logo_raspbian.png',
	'/img/logo_redhat.png',
	'/img/logo_slackware.png',
	'/img/logo_suse.png',
	'/img/logo_ubuntu.png',
	'/img/logo_win.png'
];

self.addEventListener('install',function(ev) {
	if (dbg) console.log('service worker install ev:'+JSON.stringify(ev));

	event.waitUntil(
		caches.open(CACHE_VERSION)
			.then((cache) => {
			//	return cache.addAll(CACHE_URLS);
				return cache.addAll(CACHE_URLS
					.map( (url) => { return new Request(url,{ mode:'no-cors' }); }))
					.then(() => { if (dbg) console.log('service worker cache loaded'); });
			})
			.catch((err) => { console.error('error',err); })
	);
});

self.addEventListener('activate',function(ev) {
	if (dbg) console.log('service worker activate ev:'+JSON.stringify(ev));
});

self.addEventListener('fetch',function(ev) {
	if (dbg) console.log('service worker fetch ev:'+JSON.stringify(ev));

/*
	{
		const rq = ev.request;
		if ('GET' !== rq.method) {
		    ev.respondWith(fetch(rq));
		    return;
	 	}

	 	const fetchedFromNetwork = function(rsp) {
			const c = rsp.clone();
			caches.open(CACHE_VERSION).then(function add(cache) {
				cache.put(rq,c);
			});
			return esp;
		};

		const unableToResolve = function() { return new Response('', { status:503,statusText:'Service Unavailable' }); };

		const queriedCache = function(cached) {
		  const networked = fetch(rq)
		    .then(fetchedFromNetwork,unableToResolve)
		    .catch(unableToResolve);
		  return cached || networked;
		};

		ev.respondWith(caches
			.match(rq) 
			.then(queriedCache)
		);
	}
*/


/*
	ev.respondWith(
		fetch(ev.request)
			.then((rsp) => networkThenCache(rsp,ev))
			.catch(() => pullFromCache(ev))
	);
*/

});



self.addEventListener('push',function(ev) {  
	if (dbg) console.log('service worker push ev:'+JSON.stringify(ev));

	const title = 'Push Message';  
	const body = 'A push message was received.';  
	const icon = '/img/lenny_512.png';  
	const tag = 'simple-push-example-tag';
	ev.waitUntil( self.registration.showNotification(title,{ body:body,icon:icon,tag:tag }) );  
});


/*
self.addEventListener('controllerchange',function(ev) {  
	if (dbg) console.log('service worker controllerchange ev:'+JSON.stringify(ev));
}
self.addEventListener('error',function(ev) {  
	if (dbg) console.log('service worker error ev:'+JSON.stringify(ev));
}
self.addEventListener('message',function(ev) {  
	if (dbg) console.log('service worker message ev:'+JSON.stringify(ev));
}

self.addEventListener('notificationclick',function(ev) {
	if (dbg) console.log('service worker notificationclick ev:'+JSON.stringify(ev));
}
*/

