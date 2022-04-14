<?php
//phpinfo(); exit();
//print_r($_SERVER); print_r($_REQUEST); exit();
date_default_timezone_set('America/Los_Angeles');

define('DIR_DATA', '../data'); 
define('DIR_REL' , DIR_DATA . '/rel'); 
define('DIR_LOGS', DIR_DATA . '/logs'); 
define('DIR_MSGS', DIR_DATA . '/msgs'); 
define('FILE_LIC', DIR_DATA . '/lic.log'); 

define('FILE_DL'     , DIR_DATA . '/dl.log');
define('FILE_DL_OLD' , DIR_DATA . '/dlo.log');
define('FILE_LOCK'   , DIR_DATA . '/lck.tmp');

//
//
//
define('LATEST_VERSION_MAJOR',0);
define('LATEST_VERSION_MINOR',0);
define('LATEST_VERSION_LICENSE',1);


//
//
//
//$actual_link = (!$secure ?"http" :"https") . "://$_SERVER[HTTP_HOST]$_SERVER[REQUEST_URI]";
$secure = true;//( ! empty( $_SERVER['HTTPS'] ) && $_SERVER['HTTPS'] == 'on' );

$rqst = $_SERVER['QUERY_STRING'];
$meth = $_SERVER['REQUEST_METHOD'];

function isJsonPost() { global $meth,$secure; return secure && 'POST' === $meth && FALSE !== strpos($_SERVER['CONTENT_TYPE'],'application/json'); }
function isFormPost() { global $meth,$secure; return secure && 'POST' === $meth && FALSE !== strpos($_SERVER['CONTENT_TYPE'],'application/x-www-form-urlencoded'); }
function isPost() { global $meth,$secure; return secure && 'POST' === $meth; }
function isGet() { global $meth,$secure; return secure && 'GET' === $meth; }




//
//
//
$l = explode('&',$rqst);
$l = explode('=',$l[0]);
//print_r($l); exit();


switch ($l[0]) {
	default: exit404();


	//
	// not used ???
	//

	case 'msg': {
		if (!isPost()) exit404();

		$r = file_get_contents('php://input');

		$l = htmlspecialchars($_GET['l']);
		if (0 == strlen(trim($l))) exit404();

		$cdir = DIR_MSGS.'/'.$l;
		$fil  = $cdir.'/'.date('Ymd_His',time()).'_'.$_SERVER['REMOTE_ADDR'].'.txt';

		if (!is_dir(DIR_MSGS)) mkdir(DIR_MSGS);
		if (!is_dir($cdir)) mkdir($cdir);

		file_put_contents($fil,$r);

		exit();
	}


	//
	// from website
	//
	case 'cmnt': {
		if (!isJsonPost()) exit404();

		handleLog('web-comment');
		exit();
	}

	case 'dl': {
		if (isGet()) {
			handleDownloadGet();
			exit();
		}

		if (isPost()) {
			handleDownloadPost();
			exit();
		}

		exit404();
	}



/*
//const char *const WS_GET_PUBLIC_IP      =  "GET /js/?ip";
//const char *const WS_GET_VERSION_INFO   =  "GET /js/?version";
const char *const WS_POST_LICENSE_EMAIL = "POST /js/?license";
const char *const WS_POST_BUG_REPORT    = "POST /js/?bug";
const char *const WS_POST_COMMENT       = "POST /js/?comment";
const char *const WS_POST_LOG_FILE      = "POST /js/?log";
*/

	//
	// from daemon
	//

	case'ip': {
		if (!isGet()) exit404();
		header('Content-type: text/plain');
		exit( $_SERVER['REMOTE_ADDR'] );
	}
	
	case 'version': {
		if (!isGet()) exit404();
		header('Content-type: application/json');
		exit( '{ "v":"'.LATEST_VERSION_MAJOR.'.'.LATEST_VERSION_MINOR.':'.LATEST_VERSION_LICENSE.'" }' );
	}

	case 'license': {
		if (!isJsonPost()) exit404();

		handleLicense();
		exit();
	}

	case 'bug': {
		if (!isJsonPost()) exit404();

		handleLog('bug');
		exit();
	}

	case 'comment': {
		if (!isJsonPost()) exit404();

		handleLog('comment');
		exit();
	}

	case 'stat': {
		if (!isFormPost()) exit404();

		handleLog('stat');
		exit();
	}

	case 'log': {
		if (!isFormPost()) exit404();

		handleLog('log');
		exit();
	}
}




//
// Biz Logic : 
//

function handleDownloadGet() {

	$dl = null;

	{
		$key = $_GET['dl'];
		
		$filtered = ''; $old = '';

		lock();

		try {
			$d24 = strtotime('-24 hours',time());

			$l = strtok(file_get_contents(FILE_DL),PHP_EOL);

			while (false !== $l) {
				$r = json_decode($l,TRUE);

				$d = date_parse($r['date']);
		 		$t = mktime($d['hour'],$d['minute'],$d['second'],$d['month'],$d['day'],$d['year']);

				if ($d24 < $t) {
					if ($key === $r['key']) {
						$dl = $r;
					} else {
					    $filtered .= $l .PHP_EOL;
					}
				} else {
				    $old .= $l .PHP_EOL;
				}

			    $l = strtok(PHP_EOL);
			}

			file_put_contents(FILE_DL,$filtered,LOCK_EX);
			if (0 < strlen($old)) file_put_contents(FILE_DL_OLD,$old,FILE_APPEND | LOCK_EX);

		} catch (Exception $err) {
		} finally {
			unlock();
		}
	}

	if (!$dl) {
		//
		// TODO track bad requests?
		//
		exit404();
	}

	$file  = $dl['file'];
	$path = DIR_REL.'/'.$file;

	if (!file_exists($path)) {
		$j = json_encode($dl).PHP_EOL;
		file_put_contents(FILE_DL_OLD,$j,FILE_APPEND | LOCK_EX);

		exit404();
	}

	{
		$dl['get_count']++;
		$j = json_encode($dl).PHP_EOL;
		file_put_contents(FILE_DL,$j,FILE_APPEND | LOCK_EX);
	}

	header($_SERVER['SERVER_PROTOCOL'] . ' 200 OK');
	header('Cache-Control: public'); // needed for internet explorer ???

	header('Content-Length:'.filesize($path));
	header('Content-Type: '.mime_content_type($path)); // 'application/gzip','application/zip'
	header('Content-Transfer-Encoding: Binary');
	header('Content-Disposition: attachment; filename=' . $file);
	readfile($path);

	exit();
}

function handleDownloadPost() {

	$r = json_decode(file_get_contents('php://input'),TRUE);

	$file  = $r['f'];
	$email = $r['e'];
	$fname = $r['fn'];
	$lname = $r['ln'];
	$cname = $r['cn'];
	$txt   = $r['t'];

	//
	// validate
	//
	if (!filter_var($email,FILTER_VALIDATE_EMAIL)) { exit404(); }
	if (0 == strlen($file) || 0 == strlen($fname) || 0 == strlen($lname)) { exit404(); }

	$path = DIR_REL.'/'.$file;

	if (!is_readable($path) || !is_file($path)) {
		exit404();
	}

	//
	// CRYPT_STD_DES
	// This is a standard DES-based hash with 2-character salt from the character range /0-9A-Za-z.
	//
	$d = date('Y-m-d');
	if (22 <= date('H')) {
		$d = strtotime('+1 day',$d);
	}

	$str = "$d_$email";
	$salt = 'OU'; //OU812

	$enc = substr(crypt($str,$salt),2);
	$url = "https://lennytroll.com/js/?dl=$enc";



	//
	// Save
	//
	{
		$d = date('Y-m-d H:i:s',time());
		$dl = "{ \"key\":\"$enc\",\"date\":\"$d\",\"file\":\"$file\",\"email\":\"$email\",\"fname\":\"$fname\",\"lname\":\"$lname\",\"cname\":\"$cname\",\"txt\":\"$txt\" }".PHP_EOL;

		$filtered = '';

		lock();

		try {
			$l = strtok(file_get_contents(FILE_DL),PHP_EOL);

			while (false !== $l) {
				$r = json_decode($l,TRUE);
				if ($enc == $r['key']) {
					$dl = $l;
				} else {
				    $filtered .= $l .PHP_EOL;
				}
			    $l = strtok(PHP_EOL);
			}

			{
				$j = json_decode($dl,TRUE);
				$j['post_count']++;
				$dl = json_encode($j).PHP_EOL;
				$filtered .= $dl;
			}

			file_put_contents(FILE_DL,$filtered,LOCK_EX);

		} catch (Exception $err) {
		} finally {
			unlock();
		}
	}

	//
	// Email
	//
	// http://phpmailer.worxware.com/
	// https://swiftmailer.symfony.com/
	//
	{
		$from     = 'sales@lennytroll.com';
		$fromName = 'Support';
		$name     = 'Lenny User';
		$subject  = 'Lenny Download Link';
		$msg      = "Thank you for trying Lenny Troll.\n\nYour download link is $url.";

		$hdr = 'From: sales@lennytroll.com';

		$h = mail($email,$subject,$msg,$hdr);
		if (!$h) exit('FAIL');
	}
}


//
// Biz Logic : 
//

function handleLicense() {

	$r = json_decode(file_get_contents('php://input'),TRUE);

	$ver   = $r['v'];
	$email = $r['e'];
	$mac   = $r['m'];
	$lic   = $r['l'];
	$days  = $r['d'];

	//
	// validate
	//
	if (0 == strlen($ver)  || 0 == strlen($email) || 0 == strlen($mac) || 0 == strlen($lic)) { exit404(); }
	if (!filter_var($email,FILTER_VALIDATE_EMAIL)) { exit404(); }

	//
	// version
	//
	list($av,$lv) = explode(':',$ver);
	list($vmj,$vmn) = explode('.',$av);

	//
	// log
	//
	{
		$log = "{ \"v\":$vmj.$vmn,\"lv\":$lv,\"email\":\"$email\",\"mac\":\"$mac\",\"license\":\"$lic\",\"days\":$days }"."\n";
		file_put_contents(FILE_LIC,$log,FILE_APPEND | LOCK_EX);
	}

	//
	// Email
	//
	{
		$from     = 'sales@lennytroll.com';
		$fromName = 'Licensing';
		$name     = 'New Lenny User';
		$subject  = 'Lenny Troll Licensing';
		$msg      = "Thank you for trying Lenny Troll.\n\nYour new License key is $lic.";

		$hdr = 'From: sales@lennytroll.com';

		$h = mail($email,$subject,$msg,$hdr);
		if (!$h) exit('FAIL');
	}
}


//
// Biz Logic : 
//

function handleLog($type) {

	$r = file_get_contents('php://input');
	$l = htmlspecialchars($_GET['l']);

	$cdir = DIR_LOGS.'/'.$l;
	$fil  = $cdir.'/'.date('Ymd_His',time()).'_'.$type.'.txt';

	if (!is_dir(DIR_LOGS)) mkdir(DIR_LOGS);
	if (!is_dir($cdir)) mkdir($cdir);

	file_put_contents($fil,$r);
}



//
// Helpers
//


//
//
//
$fpLock = null;
function lock() {
	global $fpLock;
	if ($fpLock) return;

	while (true) {
		$fpLock = fopen(FILE_LOCK,'w+');

		if (!$fpLock) {
			sleep(1);
			exit('lock failed');
			continue;
		}
	
		ftruncate($fpLock,0);
		fwrite($fpLock,'locked\n');
		fflush($fpLock);
		flock($fpLock,LOCK_UN); 
		break;
	} 
}

function unlock() {
	global $fpLock;
	if (!$fpLock) return;

	flock($fpLock,LOCK_UN);
	fclose($fpLock);
	$fpLock = null;
	unlink(FILE_LOCK);
}

//
//
//
function exit404() {
	header($_SERVER['SERVER_PROTOCOL'].' 404 Not Found');
	exit();
}


?>