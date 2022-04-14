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

#pragma once
#include <stdlib.h>

typedef enum {
	JSON_TYPE_NULL =0, // null value
	JSON_TYPE_INTEGER, // integer in int_value
	JSON_TYPE_DOUBLE,  // double in dbl_value
	JSON_TYPE_BOOL,    // boolean in int_value 
	JSON_TYPE_TEXT,    // string in text_value
	JSON_TYPE_ARRAY,   // array with items in child nodes
	JSON_TYPE_OBJECT,  // object with properties in child nodes
} JSONType;


class JSONItem {
public:
	JSONType type;           // type of json node, see above
	const char *text_value;  // text value of STRING node
	long long int_value;     // the value of INTEGER or BOOL node
	double dbl_value;        // the value of DOUBLE node

	const char *key;         // key of the property; for object's children only
	int length;              // number of children of OBJECT or ARRAY

	JSONItem *child,*next,*last;

	JSONItem();
	JSONItem(const JSONType type);
	JSONItem(const JSONType type,const char *key,JSONItem *parent);

	static JSONItem* parse(const char *txt);
	static void free(const JSONItem *js);

	const char* parse(const char *key,const char *txt);
	JSONItem* get(const char* key);
	JSONItem* item(int idx);
};



