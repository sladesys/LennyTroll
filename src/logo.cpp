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

#include "utils.h"
#include <string>
#include <sstream>

extern void drawTitle();
extern void drawUsage();


static const char *TITLE_STRING_FMT = \
	"Lenny - The Tele-Marketing Troll\n" \
	"Copyright (c) %d, LennyTroll.com\n" \
	"\n";




/*
#ifdef LINUX

static const char USAGE_OPTS[] = "lv:h?";

static const char *USAGE_STRING = \
	"\n" \
	"Lenny v0.90.1\n" \
	"Have Lenny answer your calls and troll the Tele-Marketers\n" \
	"\n" \
	"Usage: Lenny [-lh] [-v num]\n" \
	"Options:\n" \
	"  -l  list devices\n" \
	"  -v  <level> Verbose messages level (0-9)\n" \
	"  -h  this help message\n" \
	"\n";
//	"\n\nReport bugs to <support@lennytroll.com>.\n"

#else //LINUX
*/

extern const char USAGE_OPTS[];
const char USAGE_OPTS[] = "icldv:h?";

static const char *USAGE_STRING = \
	"\n" \
	"Lenny v0.90.1\n" \
	"Have Lenny answer your calls and troll the Tele-Marketers\n" \
	"\n" \
	"Usage: Lenny [-h] [-v num]\n" \
	"Options:\n" \
	"  -i  Configuration Information\n" \
	"  -c  SSL Certification Genrator\n" \
	"  -v  <level> Verbose messages level (0-9)\n" \
	"  -h  this help message\n" \
	"\n";
//	"\n\nReport bugs to <support@lennytroll.com>.\n"

/*
#endif //LINUX
*/


#ifdef DRAW_LOGO
static void drawLogo();
#endif //DRAW_LOGO

void drawTitle() {

#ifdef DRAW_LOGO
	drawLogo();
#endif //DRAW_LOGO

	const time_t now = time(NULL);
	struct tm *const ltm = localtime(&now);
	fprintf(stdout,TITLE_STRING_FMT,1900+ltm->tm_year);
}
void drawUsage() {
	fprintf(stdout,"%s",USAGE_STRING);
}



#ifdef DRAW_LOGO

static char logo[] = "\n" \
"  ##            ###########   ####       ##  ####       ##  ###          ##\n" \
"  ##            ############  #####      ##  #####      ##   ###        ##\n" \
"  ##            ##            ## ###     ##  ## ###     ##    ###      ##\n" \
"  ##            ##            ##  ###    ##  ##  ###    ##     ###    ##\n" \
"  ##            #####         ##   ###   ##  ##   ###   ##      ###  ##\n" \
"  ##            ######        ##    ###  ##  ##    ###  ##       #####\n" \
"  ##            ##            ##     ### ##  ##     ### ##        ###\n" \
"  ##            ##            ##      #####  ##      #####        ###\n" \
"  ###########   ###########   ##       ####  ##       ####        ###\n" \
"  ############  ############  ##        ###  ##        ###        ###\n" \
"\n";

enum { vtNormal=0,vtFGGreen=32,vtBGGreen=42 };

//static inline void vtSetTextColor(std::ostringstream &s,const unsigned c) { s << "\x01B[38;5;" <<c <<"m"; }
static inline void vtSetAttribute(std::ostringstream &s,const int m) { s << "\x01B[" <<m <<"m"; }

/*

enum {
 vtNormal=0,vtBold=1,vtDim=2,vtItalic=3,vtUnderscore=4,vtSlowBlink=5,vtFastBlink=6,vtReverse=7,vtHidden=8,vtStrikeThrough=9,
 vtFGBlack=30,vtFGRed=31,vtFGGreen=32,vtFGYellow=33,vtFGBlue=34,vtFGMagenta=35,vtFGCyan=36,vtFGWhite=37,
 vtBGBlack=40,vtBGRed=41,vtBGGreen=42,vtBGYellow=43,vtBGBlue=44,vtBGMagenta=45,vtBGCyan=46,vtBGWhite=47,
 vtFramed=51,vtCircled=52,vtOverlined=53
};

static inline void vtMoveTo(std::ostringstream &s,const unsigned x,const unsigned y) { s << "\x01B[" << (y+1) <<";" << (x+1) <<"H"; }
static inline void vtClear(std::ostringstream &s) { s << "\x01B[2J"; }
static inline void vtCursorOff(std::ostringstream &s) { s << "\x01B[?25l"; }
static inline void vtCursorOn(std::ostringstream &s) { s << "\x01B[?25h"; }
static inline void vtSetModeLineGraphics(std::ostringstream &s) { s << "\x01B(0"; }
static inline void vtSetModeUSText(std::ostringstream &s) { s << "\x01B(B"; }
static void _vtCharOut(std::ostringstream &s,const unsigned x,const unsigned y,const char c,unsigned count) {
	vtMoveTo(s,x,y);
	while (count--) { s << c; }
}
*/

void drawLogo() {
	std::ostringstream ss;
	for (unsigned i=0;i<strlen(logo);i++) {
	//	s << '#' == logo[i] ?"\xB2" :logo[i];
		if ('#' == logo[i]) {
			ss << "\u2588";
		} else {
			ss << logo[i];
		}
	}
	fprintf(stdout,"%s",ss.str().c_str());
}
#endif //DRAW_LOGO


#ifdef NOT_INCLUDED

void XXdrawLogo() {
	std::ostringstream ss;
	vtSetAttribute(s,vtFGGreen);
	for (unsigned i=0;i<strlen(logo);i++) {
//		if ('#' == logo[i]) { vtSetAttribute(ss,vtBGGreen); } else
//		if (' ' == logo[i]) { vtSetAttribute(ss,vtNormal); }
	//	ss << ( '\n' == logo[i] ?'\n' :( '#' == logo[i] ?0xB2 :logo[i] ) );

//		if ('#' == logo[i]) { ss << 'O'; continue; }
//		if ('#' == logo[i]) { ss <<"\x0B2"; continue; }
//		if ('#' == logo[i]) { ss <<"\x0DB"; continue; }
//		if ('#' == logo[i]) { ss << std::hex << 0xB2; continue; }

		ss << logo[i];
	}
	vtSetAttribute(s,vtNormal);

	fprintf(stdout,"%s",ss.str().c_str());
}



void XdrawLogo() {
	bool c = false;
	std::ostringstream ss;
	vtSetAttribute(s,vtNormal);
	for (unsigned i=0;i<strlen(logo);i++) {
		if ('#' == logo[i] && !c) { c = true; vtSetAttribute(ss,vtBGGreen); } else
		if (' ' == logo[i] &&  c) { c = false; vtSetAttribute(ss,vtNormal); }
		ss << ('\n' == logo[i] ?'\n' :' ');
	}
	vtSetAttribute(ss,vtNormal);

	fprintf(stdout,"%s",ss.str().c_str());
}

#endif //NOT_INCLUDED

