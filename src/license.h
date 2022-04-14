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

extern unsigned todayInDays();

#define LICENSE_SIZE 20
typedef enum { LICENSE_VERIFIED=0,LICENSE_INVALID,LICENSE_EXPIRED } Result;

#include <stdint.h>

class License {
public:
	uint16_t date; // in days since epoch: the expiration date or issue date depending the expires flag
	uint16_t data;
	License();
	void generate(const char *shared_secret,const char *licensee_id,char *license_dest);
	Result verify(const char *shared_secret,const char *licensee_id,const char *license_src);
};

