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

#include "json.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>


//
//
//
JSONItem::JSONItem(const JSONType t) : type(t),text_value(NULL),int_value(0),dbl_value(0),key(NULL),length(0),child(NULL),next(NULL),last(NULL) {}
JSONItem::JSONItem() : JSONItem(JSON_TYPE_NULL) {}

JSONItem::JSONItem(const JSONType typ,const char *const k,JSONItem *const parent) : JSONItem(typ) {
	key = k;
	if (!parent->last) {
		parent->child = parent->last = this;
	} else {
		parent->last->next = this;
		parent->last = this;
	}
	parent->length++;
}

void JSONItem::free(const JSONItem *const js) {
	if (NULL == js) return;
  JSONItem* p = js->child;
  while (p) {
	JSONItem *const n = p->next;
	delete(p);
	p = n;
  }
  delete(js);
}


JSONItem* JSONItem::parse(const char *const text) {
  JSONItem js;
  if (!js.parse(NULL,text)) {
	if (js.child) JSONItem::free(js.child);
	return 0;
  }
  return js.child;
}


//
//
//
static const char* parse_key(const char** key,const char *txt);
static const char* skip_block_comment(const char *p);
static const char* unescape_string(const char *s,const char **end);




//
//
//
const char* JSONItem::parse(const char *const skey,const char *p) {
	if (NULL == p) return NULL;

	JSONItem* js;

	while (true) {
		switch (*p) {
			default: LOGE("unexpected chars : %s", p); return 0;
			case '\0': LOGE("unexpected end of text : %s", p); return 0; // error
			case ' ': case '\t': case '\n': case '\r': case ',': p++; break; // skip
			case '{':
				js = new JSONItem(JSON_TYPE_OBJECT,skey,this);
				p++;

				while (true) {
					const char *new_key = NULL;
					p = parse_key(&new_key,p);
					if (!p) return 0; // error
					if (*p == '}') return p+1; // end of object

					p = js->parse(new_key,p);
					if (!p) return 0; // error
				}
				break;

			case '[':
				js = new JSONItem(JSON_TYPE_ARRAY,skey,this);
				p++;
				while (true) {
					p = js->parse(NULL,p);
					if (!p) return 0; // error
					if (*p == ']') return p+1; // end of array
				}
				break;

			case ']': return p;

			case '"':
				p++;
				js = new JSONItem(JSON_TYPE_TEXT,skey,this);
				js->text_value = unescape_string(p,&p);
				if (!js->text_value) return 0; // propagate error
				return p;

			case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			{
				js = new JSONItem(JSON_TYPE_INTEGER,skey,this);
				char* pe;
				js->int_value = strtoll(p,&pe,0);

				if (pe == p/* || errno == ERANGE*/) { LOGE("invalid number : %s",p); return 0; }

				// double value
				if (*pe == '.' || *pe == 'e' || *pe == 'E') {
					js->type = JSON_TYPE_DOUBLE;
					js->dbl_value = strtod(p, &pe);
					if (pe == p/* || errno == ERANGE*/) { LOGE("invalid number : %s",p); return 0; }

				} else {
					js->dbl_value = (double)js->int_value;
				}
				return pe;
			}

			case 't':
				if (0 != strncmp(p,"true",4)) { LOGE("unexpected chars : %s",p); return 0; }
				js = new JSONItem(JSON_TYPE_BOOL,skey,this);
				js->int_value = 1;
				return p+4;

			case 'f':
				if (0 != strncmp(p,"false",5)) { LOGE("unexpected chars : %s",p); return 0; }
				js = new JSONItem(JSON_TYPE_BOOL,skey,this);
				js->int_value = 0;
				return p+5;

			case 'n':
				if (0 != strncmp(p,"null",4)) { LOGE("unexpected chars : %s",p); return 0; }
				//new JSONItem(JSON_TYPE_NULL,skey,this);
				return p+4;

			// comment
			case '/':
				// line comment
				if (p[1] == '/') {
				  char *const ps = (char*)p;
				  p = strchr(p+2,'\n');
				  if (NULL == p) { LOGE("endless comment : %s",ps); return 0; }

				  p++;
				} else 

				// block comment
				if (p[1] != '*') { LOGE("unexpected chars : %s",p); return 0; }

				p = skip_block_comment(p+2);
				if (!p) return 0;

				break;
		}
  	}
}


JSONItem* JSONItem::get(const char *const skey) {
	if (!skey) { LOGE("skey is null"); return NULL; }

	for (JSONItem *js = child; js; js = js->next) {
		if (!js->key) continue;
		if (0 != strcmp(js->key,skey)) continue;
		return js;
	}
	return NULL;
}

JSONItem* JSONItem::item(int idx) {
	for (JSONItem *js = child; js; js = js->next) {
		if (!idx--) return js;
	}
	return NULL;
}


//
//
//
#define IS_WHITESPACE(c) ((unsigned char)(c) <= (unsigned char)' ')

static const char* parse_key(const char **key,const char *p) {
	char c;
	while ((c = *p++)) {

		if (c == '"') {
			*key = unescape_string(p,&p);
			if (NULL == *key) return 0; // propagate error

			while (*p && IS_WHITESPACE(*p)) p++;
			if (*p == ':') return p +1;

			LOGE("unexpected chars : %s",p);
			return 0;
		}

		if (IS_WHITESPACE(c)) continue;
		if (c == ',') continue;
		if (c == '}')  return p -1;

		// If not a comment
		if (c != '/') {
			LOGE("unexpected chars : %s",p-1);
			return 0; // error
		}

		 // line comment
		if (*p == '/') {
			char* ps = (char*)p-1;
			p = strchr(p+1, '\n');
			if (!p) {
				LOGE("endless comment : %s",ps);
				return 0; // error
			}
			p++;
		} else

		// block comment
		if (*p != '*') {
			LOGE("unexpected chars : %s",p-1);
			return 0; // error
		}

		p = skip_block_comment(p+1);
		if (!p) return 0;
	}

	LOGE("unexpected chars : %s",p-1);
	return 0; // error
}




const char* skip_block_comment(const char *p) {
  // assume p[-2]=='/' && p[-1]=='*'
  char* ps = (char*)p-2;
  if (!*p) { LOGE("endless comment : %s",ps); return 0; }

  while (true) {
	  p = strchr(p+1, '/');
	  if (!p) {
		LOGE("endless comment : %s",ps);
		return 0;
	  }
	  if (p[-1]!='*') continue;
	  break;
	}
	return p+1;
}


#ifdef HANDLE_UNICODE
static int unicode_to_utf8(unsigned int codepoint,char *p,char **endp);
static int hex_val(char c);
#endif //HANDLE_UNICODE

const char* unescape_string(const char *s,const char **end) {
  char *p = (char*)s,*d = (char*)s;
  char c;

  while ((c = *p++)) {
	if (c == '"') { *d='\0'; *end = p; return s; }

	if (c != '\\') { *d++ = c; continue; }

	switch (*p) {
		default: *d++ = c; break; // leave untouched

		case '\\':
		case '/':
		case '"': *d++ = *p++; break;
		case 'b': *d++ = '\b'; p++; break;
		case 'f': *d++ = '\f'; p++; break;
		case 'n': *d++ = '\n'; p++; break;
		case 'r': *d++ = '\r'; p++; break;
		case 't': *d++ = '\t'; p++; break;

	#ifndef HANDLE_UNICODE
		case 'u': *d++ = c; break; // leave unicode untouched

	#else //HANDLE_UNICODE
		case 'u': // unicode
		{
		  char* ps = p-1;
		  int h1, h2, h3, h4;
		  if ((h1 = hex_val(p[1])) <0 || (h2 = hex_val(p[2])) <0 || (h3 = hex_val(p[3])) <0 || (h4 = hex_val(p[4])) <0) {
			LOGE("invalid unicode escape : %s",p-1);
			return 0;
		  }

		  unsigned int codepoint = h1<<12 | h2<<8 | h3<<4 | h4;

		  if ((codepoint & 0xfc00) == 0xd800) { // high surrogate; need one more unicode to succeed
			p += 6;
			if (p[-1] != '\\' || *p != 'u' || (h1 = hex_val(p[1])) <0 || (h2 = hex_val(p[2])) <0 || (h3 = hex_val(p[3])) <0 || (h4 = hex_val(p[4])) <0) {
			  LOGE("invalid unicode surrogate : %s",ps);
			  return 0;
			}

			unsigned int codepoint2 = h1<<12 | h2<<8 | h3<<4 | h4;
			if ((codepoint2 & 0xfc00) != 0xdc00) {
			  LOGE("invalid unicode surrogate : %s",ps);
			  return 0;
			}

			codepoint = 0x10000 + ((codepoint - 0xd800) <<10) + (codepoint2 - 0xdc00);
		  }

		  if (!unicode_to_utf8(codepoint, d, &d)) {
			LOGE("invalid codepoint : %s",ps);
			return 0;
		  }
		  p += 5;
		  break;
		}
	#endif //HANDLE_UNICODE
		}
	}

	LOGE("no closing quote for string : %s",s);
	return 0;
}



#ifdef HANDLE_UNICODE

int unicode_to_utf8(const unsigned int codepoint,char *p,char **endp) {
  // code from http://stackoverflow.com/a/4609989/697313
  if (codepoint < 0x80) *p++ = codepoint;
  else 
  if (codepoint < 0x800) *p++= 192 + codepoint/64, *p++ = 128 + codepoint%64;
  else 
  if (codepoint - 0xd800u < 0x800) return 0; // surrogate must have been treated earlier
  else 
  if (codepoint < 0x10000) *p++ = 224 + codepoint/4096, *p++ = 128 + codepoint/64 %64, *p++=128+codepoint%64;
  else 
  if (codepoint < 0x110000) *p++ = 240 + codepoint/262144, *p++ = 128 + codepoint/4096 %64, *p++=128+codepoint/64%64, *p++=128+codepoint%64;
  else 
  return 0; // error
  *endp = p;
  return 1;
}

int hex_val(const char c) {
	if (c>='0' && c<='9') return c-'0';
	if (c>='a' && c<='f') return c-'a'+10;
	if (c>='A' && c<='F') return c-'A'+10;
	return -1;
}
#endif //HANDLE_UNICODE


