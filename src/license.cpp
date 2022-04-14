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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#define SHA1_OUTPUT_LEN 20
#define SECONDS_IN_DAY 86400

#include "license.h"


//
// Internals
//
static void _symmetricGenerate(const char *symm_key,uint64_t target_id,uint16_t exp_date,uint16_t data,char *license);
static bool _symmetricVerify(const char *license,const char *symm_key,uint64_t target_id,uint16_t *exp_date,uint16_t *data);
static void Pearson64(const char *src, size_t srclen, char *dest, size_t destlen);
static void HMAC_sha1(const uint8_t *text, size_t text_len,char *key,size_t key_len,uint8_t *digest);

static bool dateHasPassed(unsigned epochInDays);
static void str2bin(const char *src, uint8_t *dest, size_t dest_len);


//
//
//
License::License() :date(0),data(0) {}

void License::generate(const char *const shared_secret,const char *const licensee_id,char *const license_dest) {
	char p64[64] = {0};
	Pearson64(licensee_id,strlen(licensee_id),p64,sizeof(p64)-1);

	char hash[11];
	sprintf(hash,"0x%.8s",p64);

	const uint64_t phash = strtoull(hash,NULL,16);

	// license generate
	_symmetricGenerate(shared_secret,phash,date,data,license_dest);
}

Result License::verify(const char *const shared_secret, const char *const licensee_id,const char *const license_src) {
	char p64[64] = {0};
	Pearson64(licensee_id,strlen(licensee_id),p64,sizeof(p64)-1);

	char hash[11];
	sprintf(hash,"0x%.8s",p64);

	const uint64_t phash = strtoull(hash,NULL,16);

	// license validate
	const bool r = _symmetricVerify(license_src,shared_secret,phash,&date,&data);

	return
		!r                    ?LICENSE_INVALID   // not valid
		: dateHasPassed(date) ?LICENSE_EXPIRED   // expired
		:                      LICENSE_VERIFIED; // validated
}



//
//
//

bool dateHasPassed(const unsigned epochInDays) {
	return todayInDays() > epochInDays;
}
unsigned todayInDays() {
	time_t t; time(&t);                    // get current epoch time (sec)
	t -= (t % SECONDS_IN_DAY);             // round to date boundary
	return (unsigned)(t / SECONDS_IN_DAY); // return days
}

void str2bin(const char *const src, uint8_t *const dest,const size_t dest_len) {
	const char *pos = src;
	for (unsigned i=0;i<dest_len;i++) {
		sscanf(pos,"%2hhx",&dest[i]);
		pos += 2;
	}
}

//
// Symmetric key license generation
//
void _symmetricGenerate(const char *symm_key,uint64_t target_id,uint16_t exp_date,uint16_t data,char *license) {
	//printf("symm_key:%s target_id:%llu\n",symm_key,target_id);

	// create data into place
	uint8_t licdata[12];
	memcpy(licdata, (char *)&target_id,sizeof(target_id));
	memcpy(licdata + sizeof(target_id), (char *)&exp_date,sizeof(exp_date));
	memcpy(licdata + sizeof(target_id) + sizeof(exp_date), (char *)&data,sizeof(data));

	uint8_t hmac[SHA1_OUTPUT_LEN];
	HMAC_sha1(licdata,sizeof(licdata),(char*)symm_key,strlen(symm_key),hmac);

	// create the license string
	uint8_t userlic[10];
	memcpy(userlic, (char *)&exp_date, sizeof(exp_date));
	memcpy(userlic + sizeof(exp_date), (char *)&data, sizeof(data));
	memcpy(userlic + sizeof(exp_date) + sizeof(data),hmac,sizeof(userlic) - (sizeof(exp_date) + sizeof(data)));

	// to hex
	{
		const unsigned x = sizeof(userlic);
		char *ptr = license;

		for (unsigned i=0;i<x;i++) {
			ptr += sprintf(ptr,"%02X",userlic[i]);
		}
	}

	license[LICENSE_SIZE] = '\0';
}

//
// Symmetric key license verification
//
bool _symmetricVerify(const char *license,const char *symm_key,uint64_t target_id,uint16_t *exp_date,uint16_t *data) {
	//printf("symm_key:%s target_id:%llu license:%s\n",symm_key,target_id,license);

	// parse the license
	uint8_t lic_bin[10];
	str2bin(license,lic_bin,sizeof(lic_bin));
	memcpy(exp_date,lic_bin,sizeof(*exp_date));
	memcpy(data,lic_bin + sizeof(*exp_date),sizeof(*data));

	// reconstruct the HMACed data
	uint8_t licdata[12];
	memcpy(licdata, (char *)&target_id,sizeof(target_id));
	memcpy(licdata + sizeof(target_id), (char *)exp_date,sizeof(*exp_date));
	memcpy(licdata + sizeof(target_id) + sizeof(*exp_date), (char *)data,sizeof(*data));

	uint8_t hmac[SHA1_OUTPUT_LEN];
	HMAC_sha1(licdata,sizeof(licdata),(char*)symm_key,strlen(symm_key),hmac);

	// compare the leftmost bits
	{
		const int offset = sizeof(*exp_date) + sizeof(*data);
		if (0 == memcmp(lic_bin + offset,hmac,sizeof(lic_bin) - offset)) {
			return true;
		}
	}

	return false;
}

//
//
//
void Pearson64(const char *const src,const size_t srclen,char *const dest,const size_t destlen) {
	static const uint8_t T[256] = {
		// 256 values 0-255 in any (random) order suffices
		 98,  6, 85,150, 36, 23,112,164,135,207,169,  5, 26, 64,165,219, //  1
		 61, 20, 68, 89,130, 63, 52,102, 24,229,132,245, 80,216,195,115, //  2
		 90,168,156,203,177,120,  2,190,188,  7,100,185,174,243,162, 10, //  3
		237, 18,253,225,  8,208,172,244,255,126,101, 79,145,235,228,121, //  4
		123,251, 67,250,161,  0,107, 97,241,111,181, 82,249, 33, 69, 55, //  5
		 59,153, 29,  9,213,167, 84, 93, 30, 46, 94, 75,151,114, 73,222, //  6
		197, 96,210, 45, 16,227,248,202, 51,152,252,125, 81,206,215,186, //  7
		 39,158,178,187,131,136,  1, 49, 50, 17,141, 91, 47,129, 60, 99, //  8
		154, 35, 86,171,105, 34, 38,200,147, 58, 77,118,173,246, 76,254, //  9
		133,232,196,144,198,124, 53,  4,108, 74,223,234,134,230,157,139, // 10
		189,205,199,128,176, 19,211,236,127,192,231, 70,233, 88,146, 44, // 11
		183,201, 22, 83, 13,214,116,109,159, 32, 95,226,140,220, 57, 12, // 12
		221, 31,209,182,143, 92,149,184,148, 62,113, 65, 37, 27,106,166, // 13
		  3, 14,204, 72, 21, 41, 56, 66, 28,193, 40,217, 25, 54,179,117, // 14
		238, 87,240,155,180,170,242,212,191,163, 78,218,137,194,175,110, // 15
		 43,119,224, 71,122,142, 42,160,104, 48,247,103, 15, 11,138,239  // 16
	};

	uint8_t hh[8];
	for (unsigned j=0;j<8;j++) {
		uint8_t h = T[((unsigned)src[0] + j) % 256];

		for (unsigned i=1;i< srclen;i++) {
			h = T[h ^ src[i]];
		}

		hh[j] = h;
	}

	// to hex
	snprintf(dest,destlen,"%02X%02X%02X%02X%02X%02X%02X%02X",hh[0],hh[1],hh[2],hh[3],hh[4],hh[5],hh[6],hh[7]);
}


//
//
//
#define SHA1_BLOCK_SIZE	64

#if defined(NO_OPENSSL) || defined(OPENSSL_NO_SHA1)
#include <sha.h>
#else
#include <openssl/sha.h>
#endif

void HMAC_sha1(const uint8_t *text, size_t text_len,char *key, size_t key_len,uint8_t *digest) {
  
	uint8_t tk[SHA1_OUTPUT_LEN];

	// if key is longer than 64 bytes reset it to key=SHA1(key)
	if (SHA1_BLOCK_SIZE < key_len) {

#ifndef OPENSSL_NO_SHA1

		SHA_CTX ctx;
		SHA1_Init(&ctx);
		SHA1_Update(&ctx,key,key_len);
		SHA1_Final(tk,&ctx);

#else //NO_OPENSSL

		SHA1Context ctx;
		SHA1Reset(&ctx);
		SHA1Input(&ctx,key,key_len);
		SHA1Result(&ctx,tk);

#endif //NO_OPENSSL

		key = (char*)&tk;
		key_len = SHA1_OUTPUT_LEN;
	}

	//
	// the HMAC_SHA1 transform looks like:
	//
	// SHA1(K XOR opad, SHA1(K XOR ipad, text))
	//
	// where K is an n byte key
	// ipad is the byte 0x36 repeated 64 times
	//
	// opad is the byte 0x5C repeated 64 times
	// and text is the data being protected
	//

	// start out by storing key in pads
	uint8_t k_ipad[SHA1_BLOCK_SIZE+1] = {0}; // inner padding key XORd with ipad
	uint8_t k_opad[SHA1_BLOCK_SIZE+1] = {0}; // outer padding key XORd with opad

	memcpy(k_ipad,key,key_len);
	memcpy(k_opad,key,key_len);

	// XOR key with ipad and opad values
	for (unsigned i=0;i<SHA1_BLOCK_SIZE;i++) {
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5C;
	}

	//
	// perform inner SHA1
	//

#ifndef OPENSSL_NO_SHA1

		{
			SHA_CTX ctx;
			SHA1_Init(&ctx);
			SHA1_Update(&ctx,k_ipad,SHA1_BLOCK_SIZE); // inner pad
			SHA1_Update(&ctx,text,text_len);          // add text
			SHA1_Final(digest,&ctx);                  // finish up 1st pass perform outer SHA1
		}
		{
			SHA_CTX ctx;
			SHA1_Init(&ctx);
			SHA1_Update(&ctx,k_opad,SHA1_BLOCK_SIZE); // outer pad
			SHA1_Update(&ctx,digest,SHA1_OUTPUT_LEN); // addresults of 1st hash
			SHA1_Final(digest,&ctx);                  // finish up 2nd pass
		}

#else //NO_OPENSSL

		{
			SHA1Context ctx;
			SHA1Reset(&ctx);
			SHA1Input(&ctx,k_ipad,SHA1_BLOCK_SIZE); // inner pad
			SHA1Input(&ctx,text,text_len);          // add text
			SHA1Result(&ctx,digest);                // finish up 1st pass perform outer SHA1
		}
		{
			SHA1Context ctx;
			SHA1Reset(&ctx);
			SHA1Input(&ctx,k_opad,SHA1_BLOCK_SIZE); // outer pad
			SHA1Input(&ctx,digest,SHA1_OUTPUT_LEN); // addresults of 1st hash
			SHA1Result(&ctx,digest);                // finish up 2nd pass
		}

#endif //NO_OPENSSL

}

