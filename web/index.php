<?php

require('_site.php');

$t = 'Lenny - The Telemarketer Troll';
$d = 'Record all the fun as Lenny the Telemarketer Troll bot answers your home telephone.  It runs on your Raspberry Pi with USB Modems and your Home Telephone.  Create new Lenny profiles or use the included Lenny script.';
$k = 'Lenny,Lenny Troll,Lenny Telemarketer,Troll,Telemarketer,Robocall,spam,Telephone Answering Device,TAD,Smart Answering Machine,Home Telephone,Home Phone,POTS,Plain Old Telephone System,Analog Telephone,RasberryPi,RPi,USB Modem,Voice Modem,Modem,RasberryPi,Raspberry Pi,RPi,Do It Yourself,DoItYourself,DIY';

showHeader($t,$d,$k);
showMenuHeader();

showHome();

showFooter();

?>