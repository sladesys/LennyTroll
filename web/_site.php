<?php



//
//
//

function showHeader($title,$desc,$keywords) {
	$r  = 
'<!DOCTYPE html><html lang="en"><head>
<title>'.$title.'</title>
<meta name="description" content="'.$desc.'"/>
<meta name="keywords" content="'.$keywords.'"/>
<meta name="publisher" content="Slade Systems"/>

<link rel="shortcut icon" href="favicon.ico"/>
<link rel="apple-touch-icon" href="img/lenny_180.png"/>
<link rel="manifest" href="l.manifest"/>

<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1.0,minimum-scale=1.0, maximum-scale=1.0,viewport-fit=cover"/>';

	if (false) {
		$r .= '<link rel="stylesheet" href="css/l.css"/>';
		$r .= '<link rel="stylesheet" href="css/ld.css" id="theme_dark" disabled="disabled"/>';
		$r .= '<link rel="stylesheet" href="css/ll.css" id="theme_lite"/>';
	} else {
		$lf = [ "\r","\n" ];
		$r .= '<style>';

		$r .= preg_replace('/^((?=^)(\s*))|((\s*)(?>$))|(\r)|(\n)/sim','',file_get_contents('css/l.css'));
		$r .= preg_replace('/^((?=^)(\s*))|((\s*)(?>$))|(\r)|(\n)/sim','',file_get_contents('css/ll.css'));

//		$r .= str_replace($lf,'',file_get_contents('css/l.css'));
//		$r .= str_replace($lf,'',file_get_contents('css/ll.css'));

		$r .= '
#github {
padding:0.5em 0 0.5em 0;text-align:center;background-color:#282;
	}
#github,#github a {
	color:#fff;font-size:1em
}
#github a:hover {
	text-decoration:underline;
	background-color:#282;
	color:#fff;
}
</style>';
	}

	if (false) {
		$r .= '<script src="js/l.js"></script>';
		$r .= '<script src="js/ld.js"></script>';
	} else {
		$r .= '<script>';
		$r .= file_get_contents('js/l.js');
		$r .= file_get_contents('js/ld.js');
		$r .= '</script>';
	}

	$r .= '
</head><body>
';
	echo($r);
}


function showSimpleHeader() {
	$r  = 
'<div id="github">Lenny is now open source at <a href="https://github.com/sladesys/LennyTroll">GitHub</a></div>
<div id="title">
<h1>Lenny - The Telemarketing Troll</h1>
<h2>The Telemarketer’s Worst Nightmare</h2>

</div>

<div style="position:absolute;top:4px;right:10px"><img width="100" height="74" style="margin-top:2em;width:auto;height:74px" src="img/lenny_side.png" alt="lenny"/></div>

<div id="wrapper">
';
	echo($r);
}


function showMenuHeader() {
	showSimpleHeader();

//	$r .= '<nav role="navigation">';

	$r .= 
'<div id="menuToggle">

<input id="menu_check" type="checkbox"/>
<span></span><span></span><span></span>

<div id="menu">
<div style="text-align:center"><img width="83" height="100" style="width:83px;height:100px" src="img/lenny_toon.png" alt="lenny"/></div>
<a href="/">Lenny</a>
<a href="about.php">How It Works</a>
<a href="hardware.php">Do It Yourself</a>
<a href="download.php">Download</a>
<a href="start.php">Getting Started</a>
<a href="support.php">Support</a>

<div style="margin:1em 0 0 1em">
	Share on:

	<a class="" href="https://www.facebook.com/sharer/sharer.php?u=https://lennytroll.com" target="_blank">
	<svg height="15px" version="1.1" viewBox="0 0 60.734 60.733" width="15px" x="0px" xml:space="preserve" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns="http://www.w3.org/2000/svg" y="0px">
	<path class="svg-path" d="M57.378,0.001H3.352C1.502,0.001,0,1.5,0,3.353v54.026c0,1.853,1.502,3.354,3.352,3.354h29.086V37.214h-7.914v-9.167h7.914v-6.76c0-7.843,4.789-12.116,11.787-12.116c3.355,0,6.232,0.251,7.071,0.36v8.198l-4.854,0.002c-3.805,0-4.539,1.809-4.539,4.462v5.851h9.078l-1.187,9.166h-7.892v23.52h15.475c1.852,0,3.355-1.503,3.355-3.351V3.351C60.731,1.5,59.23,0.001,57.378,0.001z" fill="#222222"></path>
	</svg>
	Facebook
	</a>

	<a class="" href="https://www.linkedin.com/sharing/share-offsite/?url=https://lennytroll.com" target="_blank">
	<svg height="15px" width="15px" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
	<path fill="#222222" d="m20.447 20.452h-3.554v-5.569c0-1.328-.027-3.037-1.852-3.037-1.853 0-2.136 1.445-2.136 2.939v5.667h-3.554v-11.452h3.414v1.561h.046c.477-.9 1.637-1.85 3.37-1.85 3.601 0 4.267 2.37 4.267 5.455v6.286zm-15.11-13.019c-1.144 0-2.063-.926-2.063-2.065 0-1.138.92-2.063 2.063-2.063 1.14 0 2.064.925 2.064 2.063 0 1.139-.925 2.065-2.064 2.065zm1.782 13.019h-3.564v-11.452h3.564zm15.106-20.452h-20.454c-.979 0-1.771.774-1.771 1.729v20.542c0 .956.792 1.729 1.771 1.729h20.451c.978 0 1.778-.773 1.778-1.729v-20.542c0-.955-.8-1.729-1.778-1.729z"></path>
	</svg>
	LinkedIn
	</a>

	<a class="" href="http://pinterest.com/pin/create/button/?url=https://lennytroll.com" target="_blank">
	<svg height="15px" width="15px" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
	<path fill="#222222" d="m12.017 0c-6.621 0-11.988 5.367-11.988 11.987 0 5.079 3.158 9.417 7.618 11.162-.105-.949-.199-2.403.041-3.439.219-.937 1.406-5.957 1.406-5.957s-.359-.72-.359-1.781c0-1.663.967-2.911 2.168-2.911 1.024 0 1.518.769 1.518 1.688 0 1.029-.653 2.567-.992 3.992-.285 1.193.6 2.165 1.775 2.165 2.128 0 3.768-2.245 3.768-5.487 0-2.861-2.063-4.869-5.008-4.869-3.41 0-5.409 2.562-5.409 5.199 0 1.033.394 2.143.889 2.741.099.12.112.225.085.345-.09.375-.293 1.199-.334 1.363-.053.225-.172.271-.401.165-1.495-.69-2.433-2.878-2.433-4.646 0-3.776 2.748-7.252 7.92-7.252 4.158 0 7.392 2.967 7.392 6.923 0 4.135-2.607 7.462-6.233 7.462-1.214 0-2.354-.629-2.758-1.379l-.749 2.848c-.269 1.045-1.004 2.352-1.498 3.146 1.123.345 2.306.535 3.55.535 6.607 0 11.985-5.365 11.985-11.987 0-6.623-5.378-11.987-11.985-11.987z"></path>
	</svg>
	Pinterest
	</a>

	<a class="" href="https://reddit.com/submit?title=Troll Telemarketers with Lenny Troll&amp;url=https://lennytroll.com" target="_blank">
	<svg height="15px" width="15px" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
	<path fill="#222222" d="m12 0a12 12 0 0 0 -12 12 12 12 0 0 0 12 12 12 12 0 0 0 12-12 12 12 0 0 0 -12-12zm5.01 4.744c.688 0 1.25.561 1.25 1.249a1.25 1.25 0 0 1 -2.498.056l-2.597-.547-.8 3.747c1.824.07 3.48.632 4.674 1.488.308-.309.73-.491 1.207-.491.968 0 1.754.786 1.754 1.754 0 .716-.435 1.333-1.01 1.614a3.111 3.111 0 0 1 .042.52c0 2.694-3.13 4.87-7.004 4.87s-7.004-2.176-7.004-4.87c0-.183.015-.366.043-.534a1.748 1.748 0 0 1 -1.039-1.6c0-.968.786-1.754 1.754-1.754.463 0 .898.196 1.207.49 1.207-.883 2.878-1.43 4.744-1.487l.885-4.182a.342.342 0 0 1 .14-.197.35.35 0 0 1 .238-.042l2.906.617a1.214 1.214 0 0 1 1.108-.701zm-7.76 7.256c-.689 0-1.25.562-1.25 1.25 0 .687.561 1.248 1.25 1.248.687 0 1.248-.561 1.248-1.249s-.561-1.249-1.249-1.249zm5.5 0c-.687 0-1.248.561-1.248 1.25 0 .687.561 1.248 1.249 1.248s1.249-.561 1.249-1.249c0-.687-.562-1.249-1.25-1.249zm-5.466 3.99a.327.327 0 0 0 -.231.094.33.33 0 0 0 0 .463c.842.842 2.484.913 2.961.913s2.105-.056 2.961-.913a.361.361 0 0 0 .029-.463.33.33 0 0 0 -.464 0c-.547.533-1.684.73-2.512.73s-1.979-.196-2.512-.73a.326.326 0 0 0 -.232-.095z"></path></svg>
	Reddit
	</a>

	<a class="" href="https://twitter.com/intent/tweet?text=Troll Telemarketers with Lenny Troll&amp;url=https://lennytroll.com&amp;via=twitter" target="_blank">
	<svg height="15px" version="1.1" viewBox="0 0 612 612" width="15px" x="0px" xml:space="preserve" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns="http://www.w3.org/2000/svg" y="0px">
	<path class="svg-path" d="M612,116.258c-22.525,9.981-46.694,16.75-72.088,19.772c25.929-15.527,45.777-40.155,55.184-69.411c-24.322,14.379-51.169,24.82-79.775,30.48c-22.907-24.437-55.49-39.658-91.63-39.658c-69.334,0-125.551,56.217-125.551,125.513c0,9.828,1.109,19.427,3.251,28.606C197.065,206.32,104.556,156.337,42.641,80.386c-10.823,18.51-16.98,40.078-16.98,63.101c0,43.559,22.181,81.993,55.835,104.479c-20.575-0.688-39.926-6.348-56.867-15.756v1.568c0,60.806,43.291,111.554,100.693,123.104c-10.517,2.83-21.607,4.398-33.08,4.398c-8.107,0-15.947-0.803-23.634-2.333c15.985,49.907,62.336,86.199,117.253,87.194c-42.947,33.654-97.099,53.655-155.916,53.655c-10.134,0-20.116-0.612-29.944-1.721c55.567,35.681,121.536,56.485,192.438,56.485c230.948,0,357.188-191.291,357.188-357.188l-0.421-16.253C573.872,163.526,595.211,141.422,612,116.258z" fill="#222222"></path>
	</svg>
	Twitter
	</a>

	<a class="" href="https://api.whatsapp.com/send?text=Troll Telemarketers with Lenny Troll%20https://lennytroll.com" target="_blank">
	<svg height="15px" width="15px" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
	<path fill="#222222" d="m17.472 14.382c-.297-.149-1.758-.867-2.03-.967-.273-.099-.471-.148-.67.15-.197.297-.767.966-.94 1.164-.173.199-.347.223-.644.075-.297-.15-1.255-.463-2.39-1.475-.883-.788-1.48-1.761-1.653-2.059-.173-.297-.018-.458.13-.606.134-.133.298-.347.446-.52.149-.174.198-.298.298-.497.099-.198.05-.371-.025-.52s-.669-1.612-.916-2.207c-.242-.579-.487-.5-.669-.51-.173-.008-.371-.01-.57-.01-.198 0-.52.074-.792.372-.272.297-1.04 1.016-1.04 2.479 0 1.462 1.065 2.875 1.213 3.074.149.198 2.096 3.2 5.077 4.487.709.306 1.262.489 1.694.625.712.227 1.36.195 1.871.118.571-.085 1.758-.719 2.006-1.413s.248-1.289.173-1.413c-.074-.124-.272-.198-.57-.347m-5.421 7.403h-.004a9.87 9.87 0 0 1 -5.031-1.378l-.361-.214-3.741.982.998-3.648-.235-.374a9.86 9.86 0 0 1 -1.51-5.26c.001-5.45 4.436-9.884 9.888-9.884 2.64 0 5.122 1.03 6.988 2.898a9.825 9.825 0 0 1 2.893 6.994c-.003 5.45-4.437 9.884-9.885 9.884m8.413-18.297a11.815 11.815 0 0 0 -8.413-3.488c-6.555 0-11.89 5.335-11.893 11.892 0 2.096.547 4.142 1.588 5.945l-1.688 6.163 6.305-1.654a11.882 11.882 0 0 0 5.683 1.448h.005c6.554 0 11.89-5.335 11.893-11.893a11.821 11.821 0 0 0 -3.48-8.413z"></path>
	</svg>
	WhatsApp
	</a>

</div>

</div>

</div>';

//	$r .= '</nav>';

	echo($r);
}



//
//
//
function showFooter() {

	$r = '</div>

		<div id="bottom">
		<div style="padding:20px;">

		<div style="display:inline-block;width:200px;vertical-align:top;">
		<h1>Resources</h1>
		<p><a href="contact.php" target="lenny">Contact Us</a></p>';
	// <p><a href="returns.html" target="lenny">Return Policy</a></p>
	$r .= '
		<p><a href="terms_use.html" target="lenny">Terms of Use</a></p>
		<p><a href="terms_service.html" target="lenny">Terms and Conditions</a></p>
		</div>

		<div id="follow">
		<h1>Follow Lenny Troll</h1>';
		// <a href="mailto://thelennytroll.gmail.com" target="lenny"><img width="30" height="30" src="img/logo_email.svg">Email</img></a>
	$r .= '
	    <a href="https://github.com/sladesys/LennyTroll" target="lenny"><img width="30" height="30" src="img/logo_github.svg" alt="GitHub"/>GitHub</a>
		<a href="https://www.facebook.com/lenny.troller.10" target="lenny"><img width="30" height="30" src="img/logo_facebook.svg" alt="Facebook"/>Facebook</a>
		<a href="https://www.reddit.com/r/itslenny/" target="lenny"><img width="30" height="30" src="img/logo_reddit.svg" alt="Reddit"/>Reddit</a>';
	// <a href="https://www.youtube.com/channel/UCrBZYWrikliO6EPZKM7KxVQ" target="lenny"><img width="30" height="30" src="img/logo_youtube.svg" alt="YouTube"/>YouTube</a>
	$r .= '
		</div>

		</div>
		</div>

		<div id="footer">Copyright &copy; '.date('Y').' <a href="https://sladesys.com">Slade Systems</a></div>
		</body>
	';

	$r .='
</html>';

	echo($r);
}








//
//
//

function showHome() {
	$r = '<div id="home">
		<div id="home_fr">
		<h1>Lenny Trolls Telemarketers</h1>

		<div>';

	$r .= '<div id="home_top">
		<div id="home_img"><img src="img/splash.jpg" width="1000" height="556" alt="Hello, this is Lenny"/></div>
		<div id="home_audio">
		<fieldset><legend>Lenny Troll Really Works!</legend>
		<p style="padding:10px;background-color:#ddd;">I used to get lots of calls at 8am,<br/> days go by without them now.</p>
		<div id="lenny_at_work_list">'.showHomeAudio().'</div>
		</fieldset>
		</div>
		</div>';

	$r .= '<div id="home_body"><div id="home_body_txt">';

/*
	$r .= '<ul class="">
			<li class="">
			Share on:
			</li>

			<li class="">
			<a class="" href="https://www.facebook.com/sharer/sharer.php?u=https://lennytroll.com" target="_blank">
			<svg height="15px" version="1.1" viewBox="0 0 60.734 60.733" width="15px" x="0px" xml:space="preserve" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns="http://www.w3.org/2000/svg" y="0px">
			<path class="svg-path" d="M57.378,0.001H3.352C1.502,0.001,0,1.5,0,3.353v54.026c0,1.853,1.502,3.354,3.352,3.354h29.086V37.214h-7.914v-9.167h7.914v-6.76c0-7.843,4.789-12.116,11.787-12.116c3.355,0,6.232,0.251,7.071,0.36v8.198l-4.854,0.002c-3.805,0-4.539,1.809-4.539,4.462v5.851h9.078l-1.187,9.166h-7.892v23.52h15.475c1.852,0,3.355-1.503,3.355-3.351V3.351C60.731,1.5,59.23,0.001,57.378,0.001z" fill="#222222"></path>
			</svg>
			Facebook
			</a>
			</li>

			<li class="">
			<a class="" href="https://www.linkedin.com/sharing/share-offsite/?url=https://lennytroll.com" target="_blank">
			<svg height="15px" width="15px" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
			<path fill="#222222" d="m20.447 20.452h-3.554v-5.569c0-1.328-.027-3.037-1.852-3.037-1.853 0-2.136 1.445-2.136 2.939v5.667h-3.554v-11.452h3.414v1.561h.046c.477-.9 1.637-1.85 3.37-1.85 3.601 0 4.267 2.37 4.267 5.455v6.286zm-15.11-13.019c-1.144 0-2.063-.926-2.063-2.065 0-1.138.92-2.063 2.063-2.063 1.14 0 2.064.925 2.064 2.063 0 1.139-.925 2.065-2.064 2.065zm1.782 13.019h-3.564v-11.452h3.564zm15.106-20.452h-20.454c-.979 0-1.771.774-1.771 1.729v20.542c0 .956.792 1.729 1.771 1.729h20.451c.978 0 1.778-.773 1.778-1.729v-20.542c0-.955-.8-1.729-1.778-1.729z"></path>
			</svg>
			LinkedIn
			</a>
			</li>

			<li class="">
			<a class="" href="http://pinterest.com/pin/create/button/?url=https://lennytroll.com" target="_blank">
			<svg height="15px" width="15px" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
			<path fill="#222222" d="m12.017 0c-6.621 0-11.988 5.367-11.988 11.987 0 5.079 3.158 9.417 7.618 11.162-.105-.949-.199-2.403.041-3.439.219-.937 1.406-5.957 1.406-5.957s-.359-.72-.359-1.781c0-1.663.967-2.911 2.168-2.911 1.024 0 1.518.769 1.518 1.688 0 1.029-.653 2.567-.992 3.992-.285 1.193.6 2.165 1.775 2.165 2.128 0 3.768-2.245 3.768-5.487 0-2.861-2.063-4.869-5.008-4.869-3.41 0-5.409 2.562-5.409 5.199 0 1.033.394 2.143.889 2.741.099.12.112.225.085.345-.09.375-.293 1.199-.334 1.363-.053.225-.172.271-.401.165-1.495-.69-2.433-2.878-2.433-4.646 0-3.776 2.748-7.252 7.92-7.252 4.158 0 7.392 2.967 7.392 6.923 0 4.135-2.607 7.462-6.233 7.462-1.214 0-2.354-.629-2.758-1.379l-.749 2.848c-.269 1.045-1.004 2.352-1.498 3.146 1.123.345 2.306.535 3.55.535 6.607 0 11.985-5.365 11.985-11.987 0-6.623-5.378-11.987-11.985-11.987z"></path>
			</svg>
			Pinterest
			</a>
			</li>

			<li class="">
			<a class="" href="https://reddit.com/submit?title=Troll Telemarketers with Lenny Troll&amp;url=https://lennytroll.com" target="_blank">
			<svg height="15px" width="15px" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
			<path fill="#222222" d="m12 0a12 12 0 0 0 -12 12 12 12 0 0 0 12 12 12 12 0 0 0 12-12 12 12 0 0 0 -12-12zm5.01 4.744c.688 0 1.25.561 1.25 1.249a1.25 1.25 0 0 1 -2.498.056l-2.597-.547-.8 3.747c1.824.07 3.48.632 4.674 1.488.308-.309.73-.491 1.207-.491.968 0 1.754.786 1.754 1.754 0 .716-.435 1.333-1.01 1.614a3.111 3.111 0 0 1 .042.52c0 2.694-3.13 4.87-7.004 4.87s-7.004-2.176-7.004-4.87c0-.183.015-.366.043-.534a1.748 1.748 0 0 1 -1.039-1.6c0-.968.786-1.754 1.754-1.754.463 0 .898.196 1.207.49 1.207-.883 2.878-1.43 4.744-1.487l.885-4.182a.342.342 0 0 1 .14-.197.35.35 0 0 1 .238-.042l2.906.617a1.214 1.214 0 0 1 1.108-.701zm-7.76 7.256c-.689 0-1.25.562-1.25 1.25 0 .687.561 1.248 1.25 1.248.687 0 1.248-.561 1.248-1.249s-.561-1.249-1.249-1.249zm5.5 0c-.687 0-1.248.561-1.248 1.25 0 .687.561 1.248 1.249 1.248s1.249-.561 1.249-1.249c0-.687-.562-1.249-1.25-1.249zm-5.466 3.99a.327.327 0 0 0 -.231.094.33.33 0 0 0 0 .463c.842.842 2.484.913 2.961.913s2.105-.056 2.961-.913a.361.361 0 0 0 .029-.463.33.33 0 0 0 -.464 0c-.547.533-1.684.73-2.512.73s-1.979-.196-2.512-.73a.326.326 0 0 0 -.232-.095z"></path></svg>
			Reddit
			</a>
			</li>

			<li class="">
			<a class="" href="https://twitter.com/intent/tweet?text=Troll Telemarketers with Lenny Troll&amp;url=https://lennytroll.com&amp;via=twitter" target="_blank">
			<svg height="15px" version="1.1" viewBox="0 0 612 612" width="15px" x="0px" xml:space="preserve" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns="http://www.w3.org/2000/svg" y="0px">
			<path class="svg-path" d="M612,116.258c-22.525,9.981-46.694,16.75-72.088,19.772c25.929-15.527,45.777-40.155,55.184-69.411c-24.322,14.379-51.169,24.82-79.775,30.48c-22.907-24.437-55.49-39.658-91.63-39.658c-69.334,0-125.551,56.217-125.551,125.513c0,9.828,1.109,19.427,3.251,28.606C197.065,206.32,104.556,156.337,42.641,80.386c-10.823,18.51-16.98,40.078-16.98,63.101c0,43.559,22.181,81.993,55.835,104.479c-20.575-0.688-39.926-6.348-56.867-15.756v1.568c0,60.806,43.291,111.554,100.693,123.104c-10.517,2.83-21.607,4.398-33.08,4.398c-8.107,0-15.947-0.803-23.634-2.333c15.985,49.907,62.336,86.199,117.253,87.194c-42.947,33.654-97.099,53.655-155.916,53.655c-10.134,0-20.116-0.612-29.944-1.721c55.567,35.681,121.536,56.485,192.438,56.485c230.948,0,357.188-191.291,357.188-357.188l-0.421-16.253C573.872,163.526,595.211,141.422,612,116.258z" fill="#222222"></path>
			</svg>
			Twitter
			</a>
			</li>

			<li class="">
			<a class="" href="https://api.whatsapp.com/send?text=Troll Telemarketers with Lenny Troll%20https://lennytroll.com" target="_blank">
			<svg height="15px" width="15px" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
			<path fill="#222222" d="m17.472 14.382c-.297-.149-1.758-.867-2.03-.967-.273-.099-.471-.148-.67.15-.197.297-.767.966-.94 1.164-.173.199-.347.223-.644.075-.297-.15-1.255-.463-2.39-1.475-.883-.788-1.48-1.761-1.653-2.059-.173-.297-.018-.458.13-.606.134-.133.298-.347.446-.52.149-.174.198-.298.298-.497.099-.198.05-.371-.025-.52s-.669-1.612-.916-2.207c-.242-.579-.487-.5-.669-.51-.173-.008-.371-.01-.57-.01-.198 0-.52.074-.792.372-.272.297-1.04 1.016-1.04 2.479 0 1.462 1.065 2.875 1.213 3.074.149.198 2.096 3.2 5.077 4.487.709.306 1.262.489 1.694.625.712.227 1.36.195 1.871.118.571-.085 1.758-.719 2.006-1.413s.248-1.289.173-1.413c-.074-.124-.272-.198-.57-.347m-5.421 7.403h-.004a9.87 9.87 0 0 1 -5.031-1.378l-.361-.214-3.741.982.998-3.648-.235-.374a9.86 9.86 0 0 1 -1.51-5.26c.001-5.45 4.436-9.884 9.888-9.884 2.64 0 5.122 1.03 6.988 2.898a9.825 9.825 0 0 1 2.893 6.994c-.003 5.45-4.437 9.884-9.885 9.884m8.413-18.297a11.815 11.815 0 0 0 -8.413-3.488c-6.555 0-11.89 5.335-11.893 11.892 0 2.096.547 4.142 1.588 5.945l-1.688 6.163 6.305-1.654a11.882 11.882 0 0 0 5.683 1.448h.005c6.554 0 11.89-5.335 11.893-11.893a11.821 11.821 0 0 0 -3.48-8.413z"></path>
			</svg>
			WhatsApp
			</a>
			</li>

			</ul>';
*/

	$r .= '
		<p>
		Lenny Troll is the <b>Smart Answering Machine</b>.
		</p>

		<p>
		<a href="https://en.wikipedia.org/wiki/Lenny_(bot)" target="lenny">Lenny</a> - The
		<a href="https://en.wikipedia.org/wiki/Telemarketing" target="lenny">Telemarketing</a> Troll - is a
		<a href="https://en.wikipedia.org/wiki/Chatbot" target="lenny">chatbot</a>
		which answers your home telephone and plays pre-recorded messages interacting with 
		<a href="https://en.wikipedia.org/wiki/Caller_ID_spoofing" target="lenny">Caller-Id</a>
		spammers while recording the call for your pleasure and to easily share with the larger 
		<a href="https://www.reddit.com/r/itslenny/" target="lenny">Lenny community</a>.
		</p>';

	$r .= '
		<p>
		Just in the US alone there were more than 4 billion
		<a href="https://en.wikipedia.org/wiki/Telemarketing" target="lenny">Telemarketing</a> calls last month.

		About 75% of generic 
		<a href="https://en.wikipedia.org/wiki/Telemarketing_fraud" target="lenny">fraud-related</a> crimes cite the telephone as the initial contact method.

		These calls mostly are
		<a href="https://en.wikipedia.org/wiki/Robocall" target="lenny">Robocalls</a>
		playing pre-recorded messages or maybe a real person in a
		<a href="https://en.wikipedia.org/wiki/Call_centre" target="lenny">call center</a> anywhere in the world.
		</p>';

	$r .= '
		<p>
		Lenny Troll runs best on 
		<a href="https://en.wikipedia.org/wiki/Raspberry_Pi" target="lenny">The Raspberry Pi</a> with a 
		<a href="https://www.usr.com/products/56k-dialup-modem/usr5637/" target="lenny">USRobotics USR5637 Voice Modem</a>
		connected to your home telephone, often called
		<a href="https://en.wikipedia.org/wiki/Landline" target="lenny">a landline</a> or
		<a href="https://en.wikipedia.org/wiki/Plain_old_telephone_service" target="lenny">a plain old telephone system (POTS)</a>, replacing traditional 

		<a href="https://en.wikipedia.org/wiki/Answering_machine" target="lenny">telephone answering machines</a>
		with a Graphical User Interface optimzed for Smartphones including and iOS and Android.

		<a href="https://www.raspberrypi.org/help/what-%20is-a-raspberry-pi/" target="lenny">RasberryPi</a>
		is a low cost (&lt;$40US), credit-card sized computer capable of doing everything you’d expect a desktop can do with MicroSD storage, a familiar Graphical UI, HDMI TV/Monitor and USB keyboard / mouse. 
		</p>';


	$r .= '<fieldset>
		<legend>Hardware Needed</legend>

		<div class="images" style="vertical-align:top">

			<div class="item" style="width:150px;">
			<div><img width="135" height="90" src="img/pi4_alpha.png" alt="pi4" style="width:90%;height:auto;"/></div>
			Raspberry Pi4
			</div>

			<div class="item" style="width:150px;">
			<div><img width="105" height="66" src="img/usr-modem.png" alt="pi4" style="width:70%;height:auto;"/></div>
			USRobotics Modem
			</div>

			<div class="item" style="width:100px;">
			<div><img width="70" height="70" src="img/rj11_wire.png" alt="rj11 wire" style="width:70%;height:auto;"/></div>
			RJ11 Connector
			</div>

			<div class="item" style="width:100px;">
			<div><img width="70" height="70" src="img/rj11_plug.png" alt="rj11 plug" style="width:70%;height:auto;"/></div>
			Telephone Jack
			</div>

			<div class="item" style="width:100px">
			<div><img width="40" height="80" src="img/smartphone.jpg" alt="smartphone" style="width:auto;height:80px;"/></div>
			Smartphone or PC
			</div>

		</div>
	</fieldset>';


	$r .= '</div>
		<div id="home_body_side"><fieldset><legend>Lenny Articles</legend>
			<div id="home_body_side_list">'.showHomeArticles().'</div>
		</fieldset></div>
		</div>
	</div>


	</div>
	</div>';

	echo($r);

}




//
//
//

function showAbout() {
	$r = '
		<div id="about">
		<div id="about_fr">
			<h1>How Lenny Troll Works</h1>
			<div id="about_body"><div id="about_body_txt">


		<div style="float:right;right:10px;top:10px;margin:10px;"><img width="120" height="81" src="img/tad.jpg" alt="old Answering Machine"/></div>

		<p>
		Lenny Troll is the first-in-class Smart Answering Machine, an automated Troll and more that you use from your Android and iOS Smartphones or anything with a Web Browser.
		</p>

		<p>
		Replace or enhance your old Answering Machine with something much more entertaining and functional while
		sharing your Lenny Troll adventures with others socially at 
		<a href="https://www.reddit.com/r/itslenny/" target="lenny">Reddit</a>,
		<a href="https://www.youtube.com/channel/UCrBZYWrikliO6EPZKM7KxVQ" target="lenny">YouTube</a> and
		<a href="https://www.facebook.com/pages/category/Public-Figure/Hello-This-is-Lenny-393897501387454/" target="lenny">Facebook</a>.
		</p>

		<p>
		Lenny Troll is a Linux-based software service running on a 
		<a href="https://www.raspberrypi.org/" target="lenny">RasberryPi</a>
		controlled by a
		<a href="https://developer.mozilla.org/en-US/docs/Web/Progressive_web_apps" target="lenny">Progressive Web App (PWA)</a>, the next generation Mobile technology
		so it runs everywhere without installing from the App Store,
		on your Smartphone, Tablet and PC, as your new Smart Answering Machine.
		</p>';


	$r .= '
		<div style="margin:10px">
		<fieldset style="width:90%"><legend>Smartphone Screen Shots</legend>
		<div class="screen_shots">
		<div class="screen_shot"><a href="img/ss/tab_activity.png" target="lenny"><img width="50" height="89" src="img/ss/tab_activity.png" alt="screen shot"/>Activity</a></div>
		<div class="screen_shot"><a href="img/ss/tab_calls.png" target="lenny" class="screen_shot"><img width="50" height="89" src="img/ss/tab_calls.png" alt="screen shot"/>Calls</a></div>
		<div class="screen_shot"><a href="img/ss/tab_hist_item.png" target="lenny" class="screen_shot"><img width="50" height="89" src="img/ss/tab_hist_item.png" alt="screen shot"/>Messages</a></div>
		<div class="screen_shot"><a href="img/ss/profile_list.png" target="lenny" class="screen_shot"><img width="50" height="89" src="img/ss/profile_list.png" alt="screen shot"/>Choice</a></div>
		<div class="screen_shot"><a href="img/ss/profile_msg_lenny.png" target="lenny" class="screen_shot"><img width="50" height="89" src="img/ss/profile_msg_lenny.png" alt="screen shot"/>Profile</a></div>
		<div class="screen_shot"><a href="img/ss/ogm_new.png" target="lenny" class="screen_shot"><img width="50" height="89" src="img/ss/ogm_new.png" alt="screen shot"/>OGM</a></div>
		</div>
		</fieldset>
		</div>';

	$r .= '<p>
		All your favorite
		<a href="https://www.soundboard.com/sb/Lennytrollforme" target="lenny">Lenny Chatbot Messages</a>
		are included as default with the \'Lenny Troll Profile\' and you can add new \'Profiles\' using the Lenny Troll Progressive Web App and your creativity.
		</p>
		<p>
		The
		<a href="https://blog.acolyer.org/2017/08/28/using-chatbots-against-voicespam-analyzing-lennys-effectiveness/
		" target="lenny">Psychology of the Lenny Chatbot Profile</a> is designed to simulate a &quot;Telemarketer’s worst nightmare&quot;, an elderly man who is a bit senile and slightly hard of hearing and when he starts to repeat himself, it is easily confused with him just being a forgetful old man.
		</p>
		<p>
		Its fun to create new profiles with your own voice, maybe your sick, or sneezing, or sleepy, or an elderly man whos is a bit senile.
		</p>';

/*
	$r .= '<div id="about_body_ads"><fieldset><legend>Lenny Community</legend></fieldset></div>';

	$r .= '<div id="about_body_ads"><fieldset><legend>Getting Started</legend>

			<div style="display:flex;text-align:center;">
				<div style="flex:1;display:inline-block;">
					<div><img width="135" height="90" src="img/pi4_alpha.png" alt="pi4" style="width:70%;height:auto;"/></div>
					Raspberry Pi4
				</div>

				<div style="flex:1;display:inline-block;">
					<div><img width="105" height="66" src="img/usr-modem.png" alt="pi4" style="width:70%;height:auto;"/></div>
					USRobotics Modem
				</div>
			</div>
		</fieldset>
	</div>';

	$p .= '<p>If you do not already have a RasberryPi, there are a number of distributors</p>';
*/

	$r .= '</div></div>
		</div>
		</div>';

	echo($r);
}




//
//
//

function showHardware() {
	$r = '
		<div id="hardware">
		<div id="hardware_fr">

			<h1>Do It Yourself Setup</h1>

				<p>
					Lenny Troll is software installed on a RasberryPi with a USB Voice Modem replacing your current old Answering Machine function.
				</p>
				<ol>
					<li>Plug USB Modem into the Raspberry Pi and into the Telephone Jack using RJ11 Connector</li>
					<li>Setup Raspberry Pi with Lenny Troll Software</li>
					<li>Use Smartphone Browser to connect to the Lenny Troll Web Application GUI</li>
					<li>Add your own Lenny Troll Profile Messages</li>
					<li>Have fun and share!</li>
				</ol>

				<div id="hardware_getting_started">

					<fieldset><legend>What is needed</legend>
						<div style="display:flex;align-items:center;">
						<div style="flex:1;">
						<div><img width="135" height="90" src="img/pi4_alpha.png" alt="pi4"/></div>
						Raspberry Pi
						</div>
						<div style="flex:1;">
						<div><img width="105" height="66" src="img/usr-modem.png" alt="pi4"/></div>
						USRobotics Modem
						</div>
						<div style="flex:1;">
						<div><img width="70" height="70" src="img/rj11_wire.png" alt="rj11 wire"/></div>
						RJ11 Connector
						</div>
						<div style="flex:1;">
						<div><img width="70" height="70" src="img/rj11_plug.png" alt="rj11 plug"/></div>
						Telephone Jack
						</div>
						<div style="flex:1">
						<div><img width="40" height="80" src="img/smartphone.jpg" alt="smartphone" style="width:auto;height:120px;"/></div>
						Smartphone or PC
						</div>
						</div>

					</fieldset>
				</div>

				<p>
					Many answering machines are built into the cordless phone we use at home, simply turning off the answering machine feature or setting it to answer after 4 rings allows Lenny Troll to handle all your calls.
				</p>
				<p>
					Using the Lenny Troll Skiplist feature, your family and friends will skip Lenny Troll Profiles going right to the Lenny Troll Answering Machine and your recorded Outgoing Message before the beep tone.
				</p>




			<div style="padding:10px;">

			<fieldset><legend>Raspberry Pi</legend>
			<div style="display:flex;padding:1px;">
			<div style="flex:3;min-height:200px;width:100%;height:100%;border:1px solid #ddd;font-size:12px;">

				<p>
					Raspberry Pi is a tiny, desktop computer putting the power of computing into the hands of hobbiest all over the world.
					Choose the recommended
					<a href="https://www.raspberrypi.org/documentation/hardware/raspberrypi/power/README.md" target="lenny">power adapter</a>
					with micro USB for your country / location.
					Also, choose an inexpensive keyboard, mouse, and appropriate HDMI cables for your HDMI-TV / Monitor.
				</p>
				<p>
					Lenny Troll has been validated to work on the Raspberry Model: Pi4, Pi3 B+, Pi3.
					There are many distributors of Raspberry Pi computers and accessories including the <a href="https://www.raspberrypi.org/products/" target="lenny">Raspberry Pi Foundation</a>.
				</p>
				<p>
					The Lenny Troll software application has a very small memory footprint.
					The Rasberry Pi uses a
					<a href="https://en.wikipedia.org/wiki/SD_card" target="lenny">SD card</a> as its hard-disk,
					an 8gig card is a recommended minimum capacity.
				</p>

				<p>
					The Raspberry Pi is very easy to configure on your local Wifi network using the 
					<a href="https://www.raspberrypi.org/downloads/noobs/" target="lenny">NOOBS</a> GUI.
					There are many <a href="https://projects.raspberrypi.org/en/projects/raspberry-pi-getting-started" target="lenny">Getting Started</a> guides and on-line videoes helping connect your Raspberry Pi to your local Network and getting all the up-to-date software updates. 
					<br/><br/><br/>Videos<br/><br/><br/>
		<!--
		https://www.pcmag.com/how-to/beginners-guide-how-to-get-started-with-raspberry-pi
		-->
				</p>
						</div>

						<div style="flex:1 0 50px;">
							<div style="text-align:center">
								<img width="128" height="75" style="width:94%;height:auto;display:block" src="img/pi4_board.png" alt="hw"/>
								<img width="54" height="36" style="width:40%;height:auto;margin:2px 2px 4px 4px;" src="img/pi4.png" alt="hw"/>
								<img width="54" height="36" style="width:40%;height:auto;margin:2px 4px 2px 2px;" src="img/pi3.png" alt="hw"/>
								<img width="54" height="36" style="width:40%;height:auto;margin:2px 2px 4px 4px;" src="img/pi_keyboard.png" alt="hw"/>
								<img width="54" height="36" style="width:40%;height:auto;margin:2px 4px 2px 2px;" src="img/pi_mouse.png" alt="hw"/>
							</div>
						</div>

					</div>
				</fieldset>



					<br/>



				<fieldset><legend>USRobotics Voice Modem</legend>
				<div style="display:flex;padding:1px;">
				<div style="flex:3;min-height:200px;width:100%;height:100%;border:1px solid #ddd;font-size:12px;">

					<p>
						Amazon offers the
					<a href="https://www.amazon.com/Robotics-USR5637-Controller-Dial-Up-External/dp/B0013FDLM0/ref=sr_1_2?keywords=USRobotics+Voice+Modem&amp;qid=1578586931&amp;sr=8-2" target="lenny">
					USRobotics USR5637
					</a> 56K USB Controller Dial-Up External Fax Modem with Voice
					</p>
					<p>This USRobotics modem is a full controller unlike a
						<a href="https://en.wikipedia.org/wiki/Softmodem" target="lenny">Soft / Win Modem</a>,
						which does not work with Raspberry Pi
						<a href="https://www.raspberrypi.org/documentation/raspbian/" target="lenny">Raspberry Pi OS</a>.
					</p>
				</div>

				<div style="flex:1 0 50px;">
					<div style="">
						<a target="lenny" href="https://support.usr.com/support/product-template.asp?prod=5637">
						<img width="105" height="66" style="width:100%;height:auto;" src="img/usr-modem.png" alt="hw"/>
						</a>
					</div>
				</div>

				</div>
				</fieldset>

			</div>
		</div>
		</div>';

	echo ($r);
}




//
//
//

require('_dl.php');

function showDownload() {
	global $dlVerRelease,$dlVerStr,$dlTzArm32MD5,$dlZipArm32MD5,$dlDateRelease;
	$dlVerStr = $dlVerRelease.' Beta';
	$dlTzArm32Rel  = 'lenny-arm32-'.$dlVerRelease.'.tar.gz';
	$dlZipArm32Rel = 'lenny-arm32-'.$dlVerRelease.'.zip';

	$r = '
		<div id="download">
		<div id="download_fr">
			<h1>Lenny Troll Software Download</h1>

			<div id="download_body">

				<fieldset><legend>Release Versions</legend>
				<div id="download_versions">
					<table>
					<tr><th>Raspberry Pi OS - arm32</th><th>Date</th><th>Version</th></tr>

					<tr><td>lenny-'.$dlVerRelease.'</td><td>'.$dlDateRelease.'</td><td>v'.$dlVerStr.'</td></tr>
					<tr><td colspan="3">

<a href="javascript:void(0)" onclick="onDownload(\''.$dlTzArm32Rel.'\');">'.$dlTzArm32Rel.'<span style="font-size:10px;margin-left:20px">md5: '.$dlTzArm32MD5.'</span></a>

<a href="javascript:void(0)" onclick="onDownload(\''.$dlZipArm32Rel.'\');">'.$dlZipArm32Rel.'<span style="font-size:10px;margin-left:20px">md5: '.$dlZipArm32MD5.'</span></a>

					</td></tr>
					</table>

					<p>
					Note: Release notes can be found in the VERSION.txt file included with the distribution.
					</p>

				</div>
				</fieldset>



				<div id="download_release_info" class="border">
					<h2>Beta Release Cycle</h2>

					<p>
					Lenny Troll is still in development where the Lenny Troll Team is finding and fixing issues daily ensuring the best quality software for the Lenny Community.
					</p>
					<p>
					The <b>Beta</b> release cycle will last a few months working towards the <b>Full v1.0</b> release.
					During this time, the <b>Beta Licenses</b> is free as we encourage partication and feedback about \'The Good, Bad and Ugly\' parts of Lenny Troll.
					</p>

					<h2>Release Feedback</h2>
					<p>
					The Lenny Troll Team is small, self-funded group with big ideas that is founded on the shoulders of the Lenny Community at large.
					We are actively using email and working towards a forum for better open-communication.
					</p>
					<p>
					Time, Resources and Quality are our partners with your help.
					</p>

				</div>

			</div>

		</div>
		</div>';

	echo($r);
}





//
//
//

function showGettingStarted() {
	$r = '
		<div id="start">
		<div id="start_fr">
			<h1>Getting Lenny Troll Started</h1>

			<div id="start_body">

				<div id="xstart_body_txt"></div>
				<div style="width:100%;text-align:center" class="items">
					<div style="text-align:left;">

			<h2>Locate Lenny Troll package after Download</h2>
			<p>
				First, locate the download Lenny Troll download package.
				For simplicity, on your Raspberry Pi with the NOOBS Graphical UI, start a Terminal window.
			</p>
			<p>
				Locate Raspberry Pi OS\'s ~/Downloads sub-directory containing what was retrieved using your RPi Chromium browser.
				Move the download package to your home directory.
			</p>
			<p>
				<b>TIP:</b> Do not include the <span style="font-size:16px">$</span> character when cut-and-pasting to run the command in your Terminal window.
			</p>
			<p>
				<b>TIP:</b> The <span style="font-size:20px">~</span> character is replaced by your home directory name (for Raspberry Pi, usually \'/home/pi\') in the Terminal window.
			</p>

						<div class="terminal_fr">
							<div class="terminal"><pre>

			$ mv ~/Downloads/lenny* ~<br/>
			<br/>

							</pre></div>
						</div>



						<br/>
						<h2>Unpackage Lenny</h2>
						<ol>
							<li>Check download was completed using md5sum utility.</li>
							<li>Unpackage Lenny Troll into a sub-directory.</li>
							<li>List contents of Lenny Troll software sub-directory.</li>
						</ol>

						<div class="terminal_fr">
							<h4>for TAR Ball</h4>
							<div class="terminal"><pre>

			$ tar -zxf lenny-arm32-0.6.tar.gz<br/>
			<br/>

			$ ls lenny-0.6/<br/>
			VERSION.txt&nbsp;&nbsp;bin&nbsp;&nbsp;&nbsp;etc&nbsp;&nbsp;&nbsp;var&nbsp;&nbsp;&nbsp;web<br/>
			<br/>

							</pre></div>
						</div>

						<div class="terminal_fr">
							<h4>for ZIP Package</h4>
							<div class="terminal"><pre>

			$ unzip -q lenny-0.6.zip<br/>
			<br/>

			$ ls lenny-0.6/<br/>
			VERSION.txt&nbsp;&nbsp;bin&nbsp;&nbsp;&nbsp;etc&nbsp;&nbsp;&nbsp;var&nbsp;&nbsp;&nbsp;web<br/>
			<br/>

							</pre></div>
						</div>


						<br/>
						<h2>Check Lenny Troll Information</h2>
						<p>
						With your U.S. Robotics USB Modem plugged into your Raspberry Pi, start Lenny Troll retrieving its configuration.
						</p>

						<div class="terminal_fr">
							<div class="terminal"><pre>

			$ lenny-0.6/bin/lenny -i<br/>
			Lenny - The Tele-Marketing Troll<br/>
			Copyright (c) 2020, LennyTroll.com<br/>
			<br/>
			Lenny v0.6:1 (cpu:arm build:32bit)<br/>
			 root: /home/pi/lenny-0.6<br/>
			<br/>
			Evaluation Beta License<br/>
			<br/>
			WebApp Access:<br/>
			 HTTP :<br/>
			 HTTPS:<br/>
			  Cert: RSA-2048 { 2020-04-01 23:59:59 GMT }<br/>
			    for: pi, 192.168.1.110, 192.168.1.111<br/>
			<br/>
			Host Info:<br/>
			 os: Raspberry Pi OS GNU/Linux 8  / Linux 4.14.93-v7+ armv7l<br/>
			 hw: Raspberry Pi 3 Model B Rev 1.2 id:14fc6cc0399945fc9f65d5a4bb594fff serial:000000008802b4ff<br/>
			 cpu: 32bit<br/>
			 mac: eth0:b8:27:eb:02:b4:ff<br/>
			 mac: wlan0:b8:27:eb:57:e1:ff<br/>
			<br/>
			Modem Info:<br/>
			 line1: 004 USR5637 56K Faxmodem U.S. Robotics /dev/ttyACM0<br/>
			<br/>

							</pre></div>
						</div>



						<br/>
						<h2>Install Lenny Troll as a Linux Service</h2>
						<ol>
							<li>Copy lenny_service.txt file to systemd managed area</li>
							<li>Refresh systemd reading new Lenny Troll service file</li>
							<li>Start the Lenny Troll service</li>
							<li>Enable the Lenny Troll service to run at boot</li>
						</ol>

						<div class="terminal_fr">
							<div class="terminal"><pre>

			<br/>
			$ sudo cp etc/lenny_service.txt /lib/systemd/system/lenny.service<br/>
			<br/>
			$ sudo systemctl daemon-reload<br/>
			<br/>
			$ sudo systemctl start lenny<br/>
			<br/>
			$ sudo systemctl enable lenny<br/>
			<br/>

							</pre></div>
						</div>


						<br/>
						<h2>Connect to the Lenny Troll Web Applicationon</h2>
			<p>
			Or, from the Terminal window, Lenny Troll can launch the Rasberry Pi Browser.
			</p>

						<div class="terminal_fr">
							<div class="terminal"><pre>

			$ lenny-0.6/bin/lenny -l<br/>
			<br/>

							</pre></div>
						</div>


			<p>
			Or, on your Smartphone\'s browser, enter the web link show in the information displayed at startup.
			In this example, the URL is <b>http://192.168.1.110:5150</b>.
			</p>

						<div class="terminal_fr">
							<div class="terminal"><pre>

			$ cat lenny-0.6/var/log/lenny.log |grep http:<br/>
			 HTTP : http://192.168.1.110:5150, http://192.168.1.111:5150<br/>
			<br/>

							</pre></div>
						</div>


';
/*
						<br/>
						<h2>Free Evaluation License Key</h2>

						<div>

							<div style="float:right;top:10px;right:10px">
								<div style="position:relative"><video id="license_add_vid"></video></div>
							</div>

							<div>

							<p>
							To use Lenny Troll software, you need a valid license.
							You can register for a FREE 60-day Evaluation License key and immediately received it in your configured Email Inbox.
							</p>

							<ol>
								<li>Start the Lenny Troll Web App and finish the First-Time Tour screens showing you the \'License Email\' screen.</li>
								<li>Enter the Email address where you want to register Lenny Troll with and then recieve the FREE Evaluation License Key, press Save button.</li>
								<li>In the \'License Key\' screen, press the \'Request a NEW 30-Day Evaluation License Key\' button.</li>
								<li>In your Email Inbox, copy the License Key and paste into the Lenny Troll Key field, press the Save button.</li>
								<li>The Lenny Troll UI will start showing the \'Activity\' tab, press the \'Settings\' tab.</li>
								<li>Scroll to the bottom of the \'Settings\' tab and review your License Key, Email address and Expiration date.</li>
							</ol>
							</div>

						</div>
*/

				$r .= '
						<br/>
						<h2>Lenny Troll is Ready to Answer Calls</h2>
						<p>TIPS:</p>
						<ul>
						<li>Lenny Troll defaults to answering calls in two rings once the caller identification is received.</li>
						<li>Skiplisted numbers are not answered by Lenny Troll or the Disconnected message.</li>
						<li>Skiplisted numbers will be answered by Lenny Troll\'s Answering Machine mode after the configured number of rings.</li>
						<li>With the \'Messages\' mode disabled, your previous Answering Machine can answer those calls as before.</li>
						</ul>
						<br/>


						<br/>
					</div>

				</div>

			</div>
		</div>
		</div>';

	echo($r);
}




//
//
//

function showSupport() {
	$r .= '
		<div id="support">
		<div id="support_fr">
			<h1>Lenny Troll Support</h1>

			<div style="min-height:150px;padding:2px;">
				<h2>FAQs</h2>
				<div id="faq">'.showFAQ().'</div>
			</div>

			<div id="xonline_support"></div>

		</div>
		</div>';

	echo($r);
}



function showAudioTrack($track,$time) {
	$r .= '<div class="audio_clip" onclick="playHomeAudio(this,\''.$track.'\')">
	<span class="audio_clip_but">&#x23ef;</span>
	<span class="audio_clip_time">0:00 / '.$time.'</span>
	</div>';
	return $r;
}

function OLD_showAudioTrack($track) {
	$r .= '<div class="audio_clip">
	<audio class="audio_ctrl" src="'.$track.'"></audio>
	<span class="audio_clip_but">&#x23ef;</span>
	<span class="audio_clip_time">0:00 / 0:00</span>
	</div>';
	return $r;
}




function showHomeAudio() {

	$r  = '<div class="audio">';
	$r .= showAudioTrack('snd/lenny_001.mp4','0:20');
	$r .= showAudioTrack('snd/lenny_002.mp4','0:22');
	$r .= showAudioTrack('snd/lenny_003.mp4','0:15');
	$r .= showAudioTrack('snd/lenny_004.mp4','7:33');
	$r .= '</div>';

	$r .= getSocialSites();
	return $r;
}


function getSocialSite($img,$url) {

	$r  = '
		<div style="display:inline-block;;margin-left:10px;">
		<a target="lenny" href="'.$url.'">
		<img width="30" height="30" style="width:auto;height:30px" src="'.$img.'" alt="social"/>
		</a>
		</div>';
	return $r;
}

function getSocialSites() {
	$r = '<div id="audio_others">';

	$r .= '<div>and other\'s stories at ...</div>';

	$r .= getSocialSite('img/logo_reddit.svg' ,'https://www.reddit.com/r/itslenny/');
	$r .= getSocialSite('img/logo_youtube.svg','https://www.youtube.com/channel/UCrBZYWrikliO6EPZKM7KxVQ');

	$r .= '</div>';

	return $r;
}






define('FILE_ARTICLES' ,'_articles.json');


function showHomeArticles() {
	$r = '<div class="articles"><a target="lenny" href="https://www.businessnewsdaily.com/11268-worst-telemarketing-experiences.html"><div class="frame"><div><p class="date">Thu, Feb 7, 2019</p><p class="title">How to Learn from the Six Worst Telemarketing Experiences</p></div></div></a><a target="lenny" href="https://www.komando.com/privacy/the-best-way-to-stop-robocalls/531616/"><div class="frame"><div><p class="date">Sat, Jan 19, 2019</p><p class="title">Complete guide to stopping robocalls in 2019</p></div></div></a><a target="lenny" href="https://www.inc.com/bill-murphy-jr/this-brilliantly-simple-trick-stops-telemarketers-robocalls-by-killing-their-business-model-but-it-makes-everyone-else-really-happy.html"><div class="frame"><div><p class="date">Sat, Dec 1, 2018</p><p class="title">This Brilliantly Simple Trick Destroys Telemarketers and Kills Their Business Model. (But It Makes Everyone Else Really Happy)</p></div></div></a><a target="lenny" href="https://www.techspot.com/news/77583-lenny-chatbot-trolls-telemarketers.html"><div class="frame"><div><p class="date">Mon, Nov 26, 2018</p><p class="title">Lenny is a chatbot that trolls telemarketers</p></div></div></a><a target="lenny" href="http://bitshare.cm/news/the-story-of-lenny-the-internets-favorite-telemarketing-troll/"><div class="frame"><div><p class="date">Wed, Nov 21, 2018</p><p class="title">The Story of Lenny, the Internet\'s Favorite Telemarketing Troll</p></div></div></a><a target="lenny" href="https://pcper.com/2018/11/have-you-heard-of-lenny-some-telemarketers-certainly-have/"><div class="frame"><div><p class="date">Wed, Nov 21, 2018</p><p class="title">Have you heard of Lenny? Some Telemarketers certainly have!</p></div></div></a><a target="lenny" href="https://www.vice.com/en_us/article/d3b7na/the-story-of-lenny-the-internets-favorite-telemarketing-troll"><div class="frame"><div><p class="date">Wed, Nov 21, 2018</p><p class="title">The Story of Lenny, the Internet\'s Favorite Telemarketing Troll</p></div></div></a><a target="lenny" href="https://www.robokiller.com/blog/satisfying-robocall-revenge-laugh/"><div class="frame"><div><p class="date">Fri, Jan 26, 2018</p><p class="title">The Most Satisfying Robocall Revenge is the One that Makes You Laugh</p></div></div></a><a target="lenny" href="https://www.washingtonexaminer.com/weekly-standard/telemarketers-ahoy"><div class="frame"><div><p class="date">Fri, Dec 22, 2017</p><p class="title">Telemarketers, Ahoy</p></div></div></a><a target="lenny" href="https://www.theverge.com/2017/11/10/16632724/scam-chatbot-ai-email-rescam-netsafe"><div class="frame"><div><p class="date">Fri, Nov 10, 2017</p><p class="title">Send scam emails to this chatbot and it’ll waste their time for you</p></div></div></a><a target="lenny" href="https://freethoughtblogs.com/stderr/2020/01/24/fascinating-lenny/"><div class="frame"><div><p class="date">Mon, Aug 1, 2016</p><p class="title">Fascinating Lenny</p></div></div></a><a target="lenny" href="https://www.nytimes.com/2016/02/25/fashion/a-robot-that-has-fun-at-telemarketers-expense.html"><div class="frame"><div><p class="date">Wed, Feb 24, 2016</p><p class="title">A Robot That Has Fun at Telemarketers’ Expense</p></div></div></a><a target="lenny" href="https://www.independent.co.uk/life-style/gadgets-and-tech/news/lenny-telemarketer-bot-robot-prank-a6813081.html"><div class="frame"><div><p class="date">Thu, Jan 14, 2016</p><p class="title">Meet Lenny - The Internet\'s favourite Telemarketer-Tricking Robot</p></div></div></a><a target="lenny" href="https://ottawacitizen.com/news/local-news/pitch-perfect-prank-lenny-answers-the-politicians-call"><div class="frame"><div><p class="date">Sat, Aug 29, 2015</p><p class="title">Pitch-perfect prank: \'Lenny\' answers the campaign\'s call</p></div></div></a><a target="lenny" href="https://nationalpost.com/news/politics/lenny-the-call-bot-tortures-telemarketers-just-ask-the-woman-calling-on-behalf-of-pierre-poilievre"><div class="frame"><div><p class="date">Fri, Aug 28, 2015</p><p class="title">Lenny the call-bot tortures telemarketers — just ask the woman calling on behalf of Pierre Poilievre</p></div></div></a></div>';

	return $r;
?>

<?php
}

function XshowHomeArticles() {

	$r = '<div class="articles">';

	$l = json_decode(file_get_contents(FILE_ARTICLES),true);
//print_r($l); exit();

	foreach ($l as $a) {
		$r .= '<a target="lenny" href="'.$a['url'].'">';
		$r .= '<div class="frame"><div>';
		$r .= '<p class="date">'.date('D, M j, Y',strtotime($a['date'])).'</p>';
		$r .= '<p class="title">'.$a['title'].'</p>';
		$r .= '</div></div></a>';
	}
	$r .= '</div>';

	echo($r);
}








function showContactForm() {
	$r = '<div id="support">';
	$r .= '<div id="support_fr">';
	$r .= '<h1>Contact</h1>';

	$r .= '<div style="min-height:150px;padding:2px;">';
	$r .= '<div id="online_support"></div>';
	$r .= '</div>';
	$r .= '</div>';
	$r .= '</div>';
	echo($r);
}





function showFAQ() {}


/*
function addFAQ($q,$a,$vid) {
	$r = '<li class="question" onclick="toggleHidden()">';
	$r .= $q;

	$r .= '<div class="answer hidden" onclick="this.classList.contains('hidden') ?this.classList.add('hidden') :this.classList.remove('hidden')">';

	if ($vid) {

	} else {

	}

	$r .= $a;
	$r .= '</div>';
	$r .= '</li>';
	return $r;
}

function showFAQ() {

	$r = '<ul style="list-style:none">';


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

*/



?>
