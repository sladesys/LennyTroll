/*

*/
'use strict';


//
//
//
const list_Follows = [
	 { name:'email'     ,img:'img/logo_email.svg'     ,url:'mailto://thelennytroll.gmail.com'}
	,{ name:'reddit'    ,img:'img/logo_reddit.svg'    ,url:'https://www.reddit.com/r/itslenny/'}
    ,{ name:'youtube'   ,img:'img/logo_youtube.svg'   ,url:'https://www.youtube.com/channel/UCrBZYWrikliO6EPZKM7KxVQ' }
	,{ name:'facebook'  ,img:'img/logo_facebook.svg'  ,url:'https://www.facebook.com/pages/category/Public-Figure/Hello-This-is-Lenny-393897501387454/' }
//  ,{ name:'googleplus',img:'img/logo_googleplus.svg',url:'' }
//  ,{ name:'instagram' ,img:'img/logo_instagram.svg' ,url:'' }
//  ,{ name:'linkedin'  ,img:'img/logo_linkedin.svg'  ,url:'' }
//  ,{ name:'pinterest' ,img:'img/logo_pinterest.svg' ,url:'' }
//  ,{ name:'tumblr'    ,img:'img/logo_tumblr.svg'    ,url:'' }
//  ,{ name:'twitter'   ,img:'img/logo_twitter.svg'   ,url:'' }
];



//
//
//
const list_FAQ = [

	{ q:'How to install Lenny Progressive Web App on my iOS Smartphone?',
	  a:''
	  +'<p>Lenny will appear as an iOS app after being installed.</p>'
	  +'<p>Use iOS\'s Safari browser to install the Lenny Progressive Web App using the \'Add to Home Screen\' option.</p>'
	  +'<ul>'
	  +'<li>Select the lower center option.</li>'
	  +'<li>Select the \'Add to Home Screen\' option.</li>'
	  +'<li>Select the \'Done\' option.</li>'
	  +'</ul>',
	  vid:'img/ss/add2home_ios.mp4'
	},
	{ q:'How to install Lenny Progressive Web App (PWA) on my Android Smartphone?',
	  a:''
	  +'<p>Lenny will appear on you Home Screen after being installed.</p>'
	  +'<p>Use Android\'s Chrome browser to install the Lenny Progressive Web App using the \'Add to Home Screen\' option.</p>'
	  +'<ul>'
	  +'<li>Select the upper settings option.</li>'
	  +'<li>Select the \'Add to Home screen\' option.</li>'
	  +'<li>Select the \'Add\' option.</li>'
	  +'<li>Select the \'Add Automatically\' option.</li>'
	  +'</ul>',
	  vid:'img/ss/add2home_and.mp4'
	},


	{ q:'How to access Safari iOS Microphone by connecting to the HTTPS / Secure Lenny Progressive Web App?',
	  a:''
	  +'<p>Lenny running on Raspberry Pi on your local network acts as an Internet of Things (IOT) device which cannot have properly signed HTTPS certificates.</p>'
	  +'<p>Lenny uses a locally generated self-signed certificate that can be installed in your iOS device allowing access to the Microphone to record new Profile and Outgoing messages.</p>'
	  +'<ul>'
	  +'<li>When connecting to the https secure link, a message will show an warning message, select the \'Show Details\' link.</li>'
	  +'<li>More text will show, scroll to and click the \'visit the website\' link.</li>'
	  +'<li>A popup message will show, \'Are you sure you want to visit this website ...\', select the \'Visit Website\' option.</li>'
	  +'<li>Navigate to Lenny \'Settings\' tab, in the \'OS Host\' section, select the \'WebCert\' link option.</li>'
	  +'<li>A popup message will show \'This website is trying to download a configuration profile ....\' message, selec the \'Allow\' option</li>'
	  +'<li>The popop message will change, \'Profile Downloaded\', select the \'Close\' option.</li>'
	  +'<li>Navigate to the \'Settings\' app, select the top option \'Profile Downloaded\' and select to \'Install\' the new Profile through the next three Popups.</li>'
	  +'<li>Navigate to the \'Settings\' app, select the \'About\' option, scroll down to and select the \'Certificate Trust Settngs\' option.</li>'
	  +'<li>Toggle the \'ENABLE FULL TRUST FOR ROOT CERTIFICATES\' option for the new Lenny certificate.</li>'
	  +'<li>The popup message \'Root Certificate\' warning message will show, select the \'Continue\' option</li>'
	  +'<li>Navigate to the \'Lenny\' app and reload the HTTPS session allowing Microphone access.</li>'
	  +'</ul>',
	  vid:'img/ss/http_cert_add_ios.mp4'
	},

	{ q:'How to access Android Microphone by connecting to the HTTPS / Secure Lenny Progressive Web App?',
	  a:'Very similar to how its done with iOS.'
	},


	{ q:'How to Record a New Outgoing Messages?',
	  a:''
	  +'<ul>'
	  +'<li>Navigate to Lenny \'Settings\' tab, scroll down to the \'Line\' section.</li>'
	  +'<li>Enable the Outgoing Message toggle and select the \'OGM\' option.</li>'
	  +'<li>On the \'Outgoing Message \' popup, select the \'Record\' option.</li>'
	  +'<li>After talking, select the \'Stop\' option.</li>'
	  +'<li>Playback the message by clicking the audio graphic to play.</li>'
	  +'</ul>'
	  +'<p><b>NOTE:</b> If the Browser prevents Microphone access because it requires a secure HTTPS session, an error message will display with a \'Redirect to Secure Session\'.</p>',
	  imgs:[
	  	{t:'Record and Stop',src:'img/ss/ogm.png'},
	  	{t:'Review and Save',src:'img/ss/ogm_new.png'}
	  ]
	},

	{ q:'How to create a new Lenny Profile?',
	  a:''
	  +'<p>Navigate to Lenny \'Settings\' tab, select \'Lenny Profile\' option.</p>'
	  +'<ul>'
	  +'<li>Navigate to Lenny \'Settings\' tab, scroll down to the \'Line\' section.</li>'
	  +'<li>Enable the Outgoing Message toggle and select the \'Lenny Profile\' option.</li>'
	  +'<li>On the \'Lenny Profile\' popup, select the \'Add New Profile\' option.\'</li>'
	  +'<li>On the \'Add New Profile\' form, enter a new \'Unque Name\' and select the \'Create\' option.\'</li>'
	  +'<li>On the \'Update Profile\' form, press the \'Record\' option to create your first new Profile Message.</li>'
	  +'<li>Press \'Stop\' when finished recording.</li>'
	  +'<li>Record as many new Messages as you want, drag and drop to reorder them.</li>'
	  +'<li>When finished, press the \'Save\' option keeping your new Profile Messages.</li>'
	  +'</ul>'
	  +'<p><b>NOTE:</b> If the Browser prevents Microphone access because it requires a secure HTTPS session, an error message will display with a \'Redirect to Secure Session\'.</p>',
	  vid:'img/ss/profile_new.mp4'
	},



	{ q:'How to report a Lenny Bug to the Lenny Team?',
	  a:''
	  +'<ul>'
	  +'<li>Navigate to the Lenny Settings tab, scroll to the &quot;Support Lenny&quot; section.</li>'
	  +'<li>Select &quot;Report a Lenny Defect&quot; option.</li>'
	  +'<li>Update the subject field with a short summary.</li>'
	  +'<li>Fill in the comment fields outline and press \'Save\' option when complete.</li>'
	  +'<li>The Lenny Team will receive your messages.</li>'
	  +'</ul>',
	  vid:'img/ss/support_bug.mp4'
	},
	{ q:'How to send a comment to the Lenny Team?',
	  a:''
	  +'<p>Navigate to the Lenny &quot;Settings&quot; tab, scroll to press the &quot;Rate and Comment&quot; button option.</p>'
	  +'<ul>'
	  +'<li>Navigate to the Lenny Settings tab, scroll to the &quot;Support Lenny&quot; section.</li>'
	  +'<li>Select &quot;Rate and Comment&quot; option.</li>'
	  +'<li>Update your Star Rating.</li>'
	  +'<li>Add helpful comments in the text field and press \'Save\' option when complete.</li>'
	  +'<li>The Lenny Team thanks you for your comments.</li>'
	  +'</ul>',
	  vid:'img/ss/support_rate.mp4'
	},

	{ q:'How to check for a Lenny software update?',
	  a:''
	  +'<p></p>'
	  +'<ul>'
	  +'<li>Navigate to the Lenny Settings tab, scroll to the &quot;Software Update&quot; section.</li>'
	  +'<li>Select &quot;Check for Updates ...&quot; option.</li>'
	  +'<li>This may take a few seconds, watch for a new software update message.</li>'
	  +'</ul>',
	  vid:'img/ss/lenny_update_check.mp4'
	},

	{ q:'How to install a Lenny software update?',
	  a:''
		+'<p>With the download lenny version in your Raspberry Pi home directory.</p>'
		+'<ul>'
		+'<li></li>'
		+'<li></li>'
		+'<li></li>'
		+'<li></li>'
		+'</ul>',
	},
/*
	{ q:'My Lenny License is close to expiring, how can I request another new FREE EVALUATION LICENSE?',
	  a:''
		+'<ul>'
		+'<li>Navigate to \'Settings\' tab and scroll to bottom to the \'LicenseKey\' section.</li>'
		+'<li>In the \'License Key\' screen, backspace erase the current \'License Key\' and press the \'Save\' button.  A popup will ask to confirm asking \'Remove your current license key\', press the \'OK\' button.</li>'		
		+'<li>In the \'License Key\' screen, the \'Request a NEW FREE Evaluation License Key\' button will appear, press it to initiate sending an Email to your Inbox.</li>'
		+'<li>In your Email Inbox, copy the License Key and paste into the Lenny Key field, press the \'Save\' button.</li>'
		+'<li>The Lenny UI will start showing the \'Activity\' tab, press the \'Settings\' tab.</li>'
		+'<li>Scroll to the bottom of the \'Settings\' tab and review your License Key, Email address and Expiration date.</li>'
		+'</ul>',
	  vid:'img/ss/license_update.mp4'
	},
*/
	{ q:'What third party libraries are used?',
	  a:''
		+'<ul>'
		+'<li>The default RasberryPi OpenSSL Project version v1.0.0 does support TLS/1.2, this beta version of Lenny is compiled with OpenSSL Project version 1.1.1 including TLS/1.3 support.<br/><br/>This product includes software developed by the OpenSSL Project for use in the OpenSSL Toolkit. (http://www.openssl.org/)<br/><br/>Copyright &copy; 1998-2019 The OpenSSL Project.  All rights reserved.<br/>'
		+'</li>'
		+'</ul>'
	}


];



//
//
//
/*
const list_Articles = [
	{
		"date" :"20190207",
		"img"  :"https://www.businessnewsdaily.com/favicon.ico",
		"url"  :"https://www.businessnewsdaily.com/11268-worst-telemarketing-experiences.html",
		"title":"How to Learn from the Six Worst Telemarketing Experiences"
	},
	{
		"date" :"20190119",
		"img"  :"https://www.komando.com/wp-content/uploads/2019/11/cropped-favicon512x512-32x32.png",
		"url"  :"https://www.komando.com/privacy/the-best-way-to-stop-robocalls/531616/",
		"title":"Complete guide to stopping robocalls in 2019"
	},
	{
		"date" :"20181201",
		"img"  :"https://www.inc.com/favicon.ico",
		"url"  :"https://www.inc.com/bill-murphy-jr/this-brilliantly-simple-trick-stops-telemarketers-robocalls-by-killing-their-business-model-but-it-makes-everyone-else-really-happy.html",
		"title":"This Brilliantly Simple Trick Destroys Telemarketers and Kills Their Business Model. (But It Makes Everyone Else Really Happy)"
	},
	{
		"date" :"20181126",
		"img"  :"https://static.techspot.com/images/favicon30.ico",
		"url"  :"https://www.techspot.com/news/77583-lenny-chatbot-trolls-telemarketers.html",
		"title":"Lenny is a chatbot that trolls telemarketers"
	},
	{
		"date" :"20181121",
		"img"  :"http://bitshare.cm/wp-content/uploads/2016/02/bitshare-logo-150x150.png",
		"url"  :"http://bitshare.cm/news/the-story-of-lenny-the-internets-favorite-telemarketing-troll/",
		"title":"The Story of Lenny, the Internet's Favorite Telemarketing Troll"
	},
	{
		"date" :"20181121",
		"img"  :"https://pcper.com/wp-content/uploads/2019/03/cropped-8a2f-podcast163-mp3-image.png",
		"url"  :"https://pcper.com/2018/11/have-you-heard-of-lenny-some-telemarketers-certainly-have/",
		"title":"Have you heard of Lenny? Some Telemarketers certainly have!"
	},
	{
		"date" :"20181121",
		"img"  :"https://vice-web-statics-cdn.vice.com/favicons/vice/apple-touch-icon-60x60.png",
		"url"  :"https://www.vice.com/en_us/article/d3b7na/the-story-of-lenny-the-internets-favorite-telemarketing-troll",
		"title":"The Story of Lenny, the Internet's Favorite Telemarketing Troll"
	},
	{
		"date" :"20180126",
		"img"  :"https://robokiller.wpengine.com/wp-content/themes/teltechbase/images/robokillericon.png",
		"url"  :"https://www.robokiller.com/blog/satisfying-robocall-revenge-laugh/",
		"title":"The Most Satisfying Robocall Revenge is the One that Makes You Laugh"
	},
	{
		"date" :"20171222",
		"img"  :"https://www.washingtonexaminer.com/favicon-32x32.png",
		"url"  :"https://www.washingtonexaminer.com/weekly-standard/telemarketers-ahoy",
		"title":"Telemarketers, Ahoy"
	},
	{
		"date" :"20171110",
		"img"  :"https://cdn.vox-cdn.com/uploads/chorus_asset/file/7395363/favicon-32x32.0.png",
		"url"  :"https://www.theverge.com/2017/11/10/16632724/scam-chatbot-ai-email-rescam-netsafe",
		"title":"Send scam emails to this chatbot and it’ll waste their time for you"
	},
	{
		"date" :"20160801",
		"img"  :"https://freethoughtblogs.com/favicon.ico",
		"url"  :"https://freethoughtblogs.com/stderr/2020/01/24/fascinating-lenny/",
		"title":"Fascinating Lenny"
	},
	{
		"date" :"20160224",
		"img"  :"https://www.nytimes.com/vi-assets/static-assets/favicon-4bf96cb6a1093748bf5b3c429accb9b4.ico",
		"url"  :"https://www.nytimes.com/2016/02/25/fashion/a-robot-that-has-fun-at-telemarketers-expense.html",
		"title":"A Robot That Has Fun at Telemarketers’ Expense"
	},
	{
		"date" :"20160114",
		"img"  :"https://www.independent.co.uk/favicon.ico",
		"url"  :"https://www.independent.co.uk/life-style/gadgets-and-tech/news/lenny-telemarketer-bot-robot-prank-a6813081.html",
		"title":"Meet Lenny - The Internet's favourite Telemarketer-Tricking Robot"
	},
	{
		"date" :"20150829",
		"img"  :"https://secure.gravatar.com/blavatar/034e689d25278f80f8281f2c424607c3?s=32",
		"url"  :"https://ottawacitizen.com/news/local-news/pitch-perfect-prank-lenny-answers-the-politicians-call",
		"title":"Pitch-perfect prank: 'Lenny' answers the campaign's call"
	},
	{
		"date" :"20150828",
		"img"  :"https://secure.gravatar.com/blavatar/bf69214e83fdd5520e4b5d91ba3b7d64?s=16",
		"url"  :"https://nationalpost.com/news/politics/lenny-the-call-bot-tortures-telemarketers-just-ask-the-woman-calling-on-behalf-of-pierre-poilievre",
		"title":"Lenny the call-bot tortures telemarketers — just ask the woman calling on behalf of Pierre Poilievre"
	}
];
*/

/*

//
//
//
const list_ArticlesAsterisk = [
	{
		"date" :"20180507",
		"img"  :"https://shaun.net/favicon.png",
		"url"  :"https://shaun.net/notes/introducing-lenny/",
		"title":"Introducing Lenny"
	},
	{
		"date" :"20151201",
		"img"  :"https://crosstalksolutions.com/wp-content/uploads/2014/10/favicon.png",
		"url"  :"https://crosstalksolutions.com/howto-pwn-telemarketers-with-lenny/",
		"title":"HowTo: Pwn Telemarketers with Lenny"
	},
	{
		"date" :"20151014",
		"img"  :"https://nathan.chantrell.net/favicon.png",
		"url"  :"https://nathan.chantrell.net/20151024/lenny-the-bot-that-fools-telesales-callers/",
		"title":"Lenny, The bot that fools telesales callers"
	},
	{
		"date" :"20140108",
		"img"  :"https://github.com/favicon.png",
		"url"  :"https://github.com/reconwireless/freepbx-itslenny",
		"title":"GitHubfreepbx-itslenny"
	},
	{
		"date" :"20130809",
		"img"  :"https://pbxinaflash.com/favicon.png",
		"url"  :"https://pbxinaflash.com/community/threads/lenny-is-back.13220/",
		"title":"Lenny Is Back!"
	}
];

*/


