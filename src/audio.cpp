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
//#include <stdio.h>
#include "utils.h"


/*
extern unsigned pcmGetMaxVolume(unsigned len,uint8_t *buf);
unsigned pcmGetMaxVolume(const unsigned len,uint8_t *const buf) {
	unsigned max = 0;

	for (unsigned i=0;i<len;i++) {
		const int s = buf[i]; // 0 - 0x7F/0x80 - 0xFF
		const unsigned v = abs(s - 0x7F);
		max = MAX(max,v);
	}
	return max;
}

extern void pcmVolumeAdjust(float factor,unsigned len,uint8_t *buf);
void pcmVolumeAdjust(const float factor,const unsigned len,uint8_t *const buf) {
	if (0 == factor) return;
	if (1 == factor) return;

	for (unsigned i=0;i<len;i++) {
		const int s = buf[i];   // 0 - 127/128 - 255
		const int v = s - 0x7F; // -127   0    - +127

		// silence threashold, where silece = -3 - +3
		if (10 > abs(v)) continue;

		buf[i] = (uint8_t)(MIN(0x7F,factor * v) +0x7F);
	}

}
*/






extern FILE *wavOpen(const char *file);
extern FILE *wavCreate(const char *file);
extern void wavWriteHeader(FILE *const fd,const unsigned len);


/*

http://soundfile.sapp.org/doc/WaveFormat/

*/

#define CHANNELS 1
#define SAMPLE_RATE 8000
#define BITS_PER_SAMPLE 8



FILE *wavCreate(const char *const file) {
	FILE *const fd = fopen(file,"wb");

	if (NULL == fd) {
		LOGV("fopen failed: %s",file);
		return NULL;
	}

	wavWriteHeader(fd,0);
	return fd;
}

FILE *wavOpen(const char *const file) {
	LOGC("wavOpen '%s'",file);

	FILE *const fd = fopen(file,"rb");

	if (NULL == fd) {
		LOGV("fopen failed: %s",file);
		return NULL;
	}

	uint8_t hdr[64];
	{
		const size_t r = fread(hdr,1,4,fd);

		if (0 >= r) {
			LOGV("fread failed err:%d %s",r,file);
			fclose(fd);
			return NULL;		
		}

		if ('R' != hdr[0] || 'I' != hdr[1] || 'F' != hdr[2] ||'F' != hdr[3]) {
			LOGV("RIFF not found : %s",file);
			fclose(fd);
			return NULL;		
		}
	}
	{
		const size_t r = fread(hdr,1,4,fd);

		if (0 >= r) {
			LOGV("fread failed err:%d %s",r,file);
			fclose(fd);
			return NULL;		
		}

		const unsigned chunkSize = (unsigned)hdr[0] | ((unsigned)hdr[1] << 8) | ((unsigned)hdr[2] << 16) | ((unsigned)hdr[3] << 24);

		if (0 == chunkSize) {
			LOGV("chunk size is zero : %s",file);
			fclose(fd);
			return NULL;		
		}
	}
	{
		const size_t r = fread(hdr,1,4,fd);

		if (0 >= r) {
			LOGV("fread failed err:%d %s",r,file);
			fclose(fd);
			return NULL;		
		}

		if ('W' != hdr[0] || 'A' != hdr[1] || 'V' != hdr[2] ||'E' != hdr[3]) {
			LOGV("WAVE not found : %s",file);
			fclose(fd);
			return NULL;		
		}
	}
	{
		const size_t r = fread(hdr,1,4,fd);

		if (0 >= r) {
			LOGV("fread failed err:%d %s",r,file);
			fclose(fd);
			return NULL;		
		}

		if ('f' != hdr[0] || 'm' != hdr[1] || 't' != hdr[2] ||' ' != hdr[3]) {
			LOGV("'fmt ' not found : %s",file);
			fclose(fd);
			return NULL;		
		}
	}
	unsigned fmtChunkSize = 0;
	{
		const size_t r = fread(hdr,1,4,fd);

		if (0 >= r) {
			LOGV("fread failed err:%d %s",r,file);
			fclose(fd);
			return NULL;		
		}

		fmtChunkSize = (unsigned)hdr[0] | ((unsigned)hdr[1] << 8) | ((unsigned)hdr[2] << 16) | ((unsigned)hdr[3] << 24);

		if (16 > fmtChunkSize) {
			LOGV("fmtChunkSize size invalid:%d : %s",fmtChunkSize,file);
			fclose(fd);
			return NULL;		
		}
	}
	{
		//LOGV("fmtChunkSize:%d",fmtChunkSize);

		const size_t r = fread(hdr,1,fmtChunkSize,fd);

		if (0 >= r) {
			LOGV("fread failed err:%d %s",r,file);
			fclose(fd);
			return NULL;		
		}

		// PCM - 1 (Linear Quantization)
		const unsigned audioFormat = (unsigned)hdr[0] | ((unsigned)hdr[1] << 8);

		// NumChannels : Mono = 1, Stereo = 2, etc.
		const unsigned numChannels = (unsigned)hdr[2] | ((unsigned)hdr[3] << 8);

		// SampleRate : 8000, 44100, etc.
		const unsigned sampleRate = (unsigned)hdr[4] | ((unsigned)hdr[5] << 8) | ((unsigned)hdr[6] << 16) | ((unsigned)hdr[7] << 24);

		// ByteRate : SampleRate * NumChannels * BitsPerSample/8
		const unsigned byteRate = (unsigned)hdr[8] | ((unsigned)hdr[9] << 8) | ((unsigned)hdr[10] << 16) | ((unsigned)hdr[11] << 24);
		
		// BlockAlign : NumChannels * BitsPerSample/8 : The number of bytes for one sample including all channels.
		const unsigned blockAlign = (unsigned)hdr[12] | ((unsigned)hdr[13] << 8);
		
		// BitsPerSample : 8 bits = 8, 16 bits = 16, etc.
		const unsigned bitsPerSample = (unsigned)hdr[14] | ((unsigned)hdr[15] << 8);

		LOGD("fmt:%d ch:%d rate:%d br:%d align:%d bits:%d",audioFormat,numChannels,sampleRate,byteRate,blockAlign,bitsPerSample);
	}

	while (true) {
		const size_t r = fread(hdr,1,8,fd);

		if (0 >= r) {
			LOGV("fread failed err:%d %s",r,file);
			fclose(fd);
			return NULL;		
		}

		if ('d' == hdr[0] || 'a' == hdr[1] || 't' == hdr[2] ||'a' == hdr[3]) {
			break;
		}

		const unsigned chunkSize = (unsigned)hdr[4] | ((unsigned)hdr[5] << 8) | ((unsigned)hdr[6] << 16) | ((unsigned)hdr[7] << 24);
		fseek(fd,chunkSize,SEEK_CUR);
	}

	return fd;
}


void wavWriteHeader(FILE *const fd,const unsigned len) {
	LOGC("wavWriteHeader len:%d",len);

	uint8_t hdr[44]; //memset(hdr'\0',sizeof(hdr));

	hdr[ 0] = 'R'; hdr[ 1] = 'I'; hdr[ 2] = 'F'; hdr[ 3] = 'F'; 

	hdr[ 4] = (uint8_t)(( (8+len) >> 0) &0xFF); 
	hdr[ 5] = (uint8_t)(( (8+len) >> 8) &0xFF);
	hdr[ 6] = (uint8_t)(( (8+len) >>16) &0xFF);
	hdr[ 7] = (uint8_t)(( (8+len) >>24) &0xFF);

	hdr[ 8] = 'W'; hdr[ 9] = 'A'; hdr[10] = 'V'; hdr[11] = 'E'; 
	hdr[12] = 'f'; hdr[13] = 'm'; hdr[14] = 't'; hdr[15] = ' ';

	const unsigned
		sampleRate = SAMPLE_RATE,
		channels = CHANNELS,
		bitsPerSample = BITS_PER_SAMPLE,
		format = 1,
		subChunk1Size = 16,
		byteRate = sampleRate * channels * (bitsPerSample /8),
		blockAlign =            channels * (bitsPerSample /8);
	;

	hdr[19] = (uint8_t)((subChunk1Size >>24) &0xFF); // 16 - size of this chunk
	hdr[18] = (uint8_t)((subChunk1Size >>16) &0xFF);
	hdr[17] = (uint8_t)((subChunk1Size >> 8) &0xFF);
	hdr[16] = (uint8_t)((subChunk1Size >> 0) &0xFF);

	hdr[21] = (uint8_t)((format >> 8) &0xFF); // 20 - what is the audio format? 1 for PCM = Pulse Code Modulation
	hdr[20] = (uint8_t)((format >> 0) &0xFF);

	hdr[23] = (uint8_t)((channels >> 8) &0xFF); // 22 - mono or stereo? 1 or 2?  (or 5 or ???)
	hdr[22] = (uint8_t)((channels >> 0) &0xFF);

	hdr[27] = (uint8_t)((sampleRate >>24) &0xFF); // 24 - samples per second (numbers per second)
	hdr[26] = (uint8_t)((sampleRate >>16) &0xFF);
	hdr[25] = (uint8_t)((sampleRate >> 8) &0xFF);
	hdr[24] = (uint8_t)((sampleRate >> 0) &0xFF);

	hdr[31] = (uint8_t)((byteRate >>24) &0xFF); // 28 - bytes per second
	hdr[30] = (uint8_t)((byteRate >>16) &0xFF);
	hdr[29] = (uint8_t)((byteRate >> 8) &0xFF);
	hdr[28] = (uint8_t)((byteRate >> 0) &0xFF);

	hdr[33] = (uint8_t)((blockAlign >> 8) &0xFF); // 32 - # of bytes in one sample, for all channels
	hdr[32] = (uint8_t)((blockAlign >> 0) &0xFF);

	hdr[35] = (uint8_t)((bitsPerSample >> 8) &0xFF); // 34 - how many bits in a sample(number)?  usually 16 or 24
	hdr[34] = (uint8_t)((bitsPerSample >> 0) &0xFF);

	hdr[36] = 'd'; hdr[37] = 'a'; hdr[38] = 't'; hdr[39] = 'a'; 

	hdr[43] = (uint8_t)((len >>24) &0xFF);
	hdr[42] = (uint8_t)((len >>16) &0xFF);
	hdr[41] = (uint8_t)((len >> 8) &0xFF);
	hdr[40] = (uint8_t)((len >> 0) &0xFF); 

	fseek(fd,0,SEEK_SET);
	if (1 != fwrite(hdr,sizeof(hdr),1,fd)) {
		LOGE("fwrite failed");
	}
}







#ifdef INCLUDE_ULAW


extern int ulawDecompressTable[];

void saveAudioULawToPCM(const unsigned len,uint8_t *const buf) {
	if (NULL == outFd) return;

	// int i;
	// for (i=0;i<len;i+=2) {
	// 	*((unsigned short *)buf+i) += 32768;
	// }

	int offset = 0;
	while (len > offset) {
		uint8_t pcm[512];
		const unsigned chunk = sizeof(pcm)/2 <(len -offset) ?sizeof(pcm)/2 :(len - offset);

		//LOGV("fwrite chunk:%d off:%d len:%d",chunk,offset,len);

		int i,j;
		for (i=0,j=0;i<chunk;i++) {
			const int v = ulawDecompressTable[ buf[offset++] &0x00FF ];
			pcm[j++] = (uint8_t)((v >> 8)& 0xFF);
			pcm[j++] = (uint8_t)((v >> 0)& 0xFF);
		}

		outTotal += j;
		if (1 != fwrite(pcm,j,1,outFd)) {
		}
	}
}


//
//
//


int ulawDecompressTable[] = {                          
	0x8482,0x8486,0x848a,0x848e,0x8492,0x8496,0x849a,0x849e,
	0x84a2,0x84a6,0x84aa,0x84ae,0x84b2,0x84b6,0x84ba,0x84be,
	0x84c1,0x84c3,0x84c5,0x84c7,0x84c9,0x84cb,0x84cd,0x84cf,
	0x84d1,0x84d3,0x84d5,0x84d7,0x84d9,0x84db,0x84dd,0x84df,
	0x04e1,0x04e2,0x04e3,0x04e4,0x04e5,0x04e6,0x04e7,0x04e8,
	0x04e9,0x04ea,0x04eb,0x04ec,0x04ed,0x04ee,0x04ef,0x04f0,
	0xc4f0,0x44f1,0xc4f1,0x44f2,0xc4f2,0x44f3,0xc4f3,0x44f4,
	0xc4f4,0x44f5,0xc4f5,0x44f6,0xc4f6,0x44f7,0xc4f7,0x44f8,
	0xa4f8,0xe4f8,0x24f9,0x64f9,0xa4f9,0xe4f9,0x24fa,0x64fa,
	0xa4fa,0xe4fa,0x24fb,0x64fb,0xa4fb,0xe4fb,0x24fc,0x64fc,
	0x94fc,0xb4fc,0xd4fc,0xf4fc,0x14fd,0x34fd,0x54fd,0x74fd,
	0x94fd,0xb4fd,0xd4fd,0xf4fd,0x14fe,0x34fe,0x54fe,0x74fe,
	0x8cfe,0x9cfe,0xacfe,0xbcfe,0xccfe,0xdcfe,0xecfe,0xfcfe,
	0x0cff,0x1cff,0x2cff,0x3cff,0x4cff,0x5cff,0x6cff,0x7cff,
	0x88ff,0x90ff,0x98ff,0xa0ff,0xa8ff,0xb0ff,0xb8ff,0xc0ff,
	0xc8ff,0xd0ff,0xd8ff,0xe0ff,0xe8ff,0xf0ff,0xf8ff,0x0000,
	0x7c7d,0x7c79,0x7c75,0x7c71,0x7c6d,0x7c69,0x7c65,0x7c61,
	0x7c5d,0x7c59,0x7c55,0x7c51,0x7c4d,0x7c49,0x7c45,0x7c41,
	0x7c3e,0x7c3c,0x7c3a,0x7c38,0x7c36,0x7c34,0x7c32,0x7c30,
	0x7c2e,0x7c2c,0x7c2a,0x7c28,0x7c26,0x7c24,0x7c22,0x7c20,
	0xfc1e,0xfc1d,0xfc1c,0xfc1b,0xfc1a,0xfc19,0xfc18,0xfc17,
	0xfc16,0xfc15,0xfc14,0xfc13,0xfc12,0xfc11,0xfc10,0xfc0f,
	0x3c0f,0xbc0e,0x3c0e,0xbc0d,0x3c0d,0xbc0c,0x3c0c,0xbc0b,
	0x3c0b,0xbc0a,0x3c0a,0xbc09,0x3c09,0xbc08,0x3c08,0xbc07,
	0x5c07,0x1c07,0xdc06,0x9c06,0x5c06,0x1c06,0xdc05,0x9c05,
	0x5c05,0x1c05,0xdc04,0x9c04,0x5c04,0x1c04,0xdc03,0x9c03,
	0x6c03,0x4c03,0x2c03,0x0c03,0xec02,0xcc02,0xac02,0x8c02,
	0x6c02,0x4c02,0x2c02,0x0c02,0xec01,0xcc01,0xac01,0x8c01,
	0x7401,0x6401,0x5401,0x4401,0x3401,0x2401,0x1401,0x0401,
	0xf400,0xe400,0xd400,0xc400,0xb400,0xa400,0x9400,0x8400,
	0x7800,0x7000,0x6800,0x6000,0x5800,0x5000,0x4800,0x4000,
	0x3800,0x3000,0x2800,0x2000,0x1800,0x1000,0x0800,0x0000,
};


uint8_t uLawCompressSample(uint16_t sample) {
	static uint8_t TABLE[] = {
		0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
		4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
	};

	const int sign = (sample >> 8) &0x80;
	if (0 != sign) sample *= -1;

	static const int CLIP = 32635,BIAS = 0x84;
	if (CLIP < sample) sample = CLIP;
	sample += BIAS;

	const int exponent = TABLE[(sample >> 7) & 0xFF];
	const int mantissa = (sample >> (exponent + 3)) & 0x0F;
	const uint8_t ulawbyte = ~(sign | (exponent << 4) | mantissa);
	if (ulawbyte == 0) return 0x02;	// ZEROTRAP optional CCITT trap
	return ulawbyte & 0xFF;
}

unsigned uLawCompress(const unsigned len,uint8_t *const inb,uint8_t *const outb) {
//	byte[] inb = new byte[1024]; // 16bit PCM data
//	byte[] outb = new byte[512]; //  8bit ulaw data
	unsigned i = 0,off = 0;
	while (i < len) {
		int sample = 0;
		sample |= (inb[i++] <<8);
		sample |= (inb[i++] &0xFF);
		outb[off++] = (uint8_t)uLawCompressSample((short)sample);
	}
	return off;
}
#endif //INCLUDE_ULAW





#ifdef INCLUDE_ALSA

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

static snd_pcm_t *hAudio = NULL;
static unsigned audioBufSize,audioBufTime;
static snd_pcm_uframes_t audioBufFrames = 128;;

void audioOpen() {
	LOGV("audioOpen");

	int rc = snd_pcm_open(&hAudio,"default",SND_PCM_STREAM_PLAYBACK,0);
	if (0 >= rc) {
		LOGV("snd_pcm_open failed");
		return;
	}

	{
		snd_pcm_hw_params_t *p;
		snd_pcm_hw_params_alloca(&p);
		snd_pcm_hw_params_any(hAudio,p);

		int val = 8000,dir = 0;
		snd_pcm_hw_params_set_rate_near(hAudio,p,&val,&dir);
		snd_pcm_hw_params_set_channels(hAudio,p,1);
		snd_pcm_hw_params_set_access(hAudio,p,SND_PCM_ACCESS_RW_INTERLEAVED);
		snd_pcm_hw_params_set_access(hAudio,p,SND_PCM_FORMAT_S16_LE);

		rc = snd_pcm_hw_params(hAudio,p);
		if (0 >= rc) {
			LOGV("snd_pcm_hw_params failed");
			return;
		}

		snd_pcm_hw_params_get_period_size(p,&audioBufFrames,&dir);
		audioBufSize = audioBufFrames * 2;
		
		snd_pcm_hw_params_get_period_time(p,&audioBufTime,NULL);
	}

	LOGV("snd_pcm buf frames:%d size:%d time:%d",audioBufFrames,audioBufSize,audioBufTime);
}

void audioClose() {
	if (NULL == hAudio) return;
	LOGV("audioClose");
	snd_pcm_drain(hAudio);
	snd_pcm_close(hAudio);
	hAudio = NULL;
}

void audioWrite(const unsigned len,uint8_t *const buf) {
	const int w = snd_pcm_writei(hAudio,buf,len);
	if (0 > w) {
		LOGV("snd_pcm_writei failed:%s",snd_strerror(errno));
	} else
	if (errno == -EPIPE) {
		LOGV("-EPIPE");
		snd_pcm_prepare(hAudio);
	} else {
		LOGV("snd_pcm_writei wrote:%d",w);
	}
}

/*
void audioWrite(const unsigned len,uint8_t *const buf) {
	char ulawBuf[4096];

	int i,j;
	for (i=0,j=0;i<len;i++) {
		const int v = ulawDecompressTable[ buf[i] &0x00FF ];
		ulawBuf[j++] = (char)((v >> 8) &0xFF);
		ulawBuf[j++] = (char)((v >> 0) &0xFF);
	}

	const w = snd_pcm_writei(hAudio,ulawBuf,j);
	if (0 > w) {
		LOGV("snd_pcm_writei failed:%s",snd_strerror(errno));
	} else
	if (errno == -EPIPE) {
		LOGV("-EPIPE");
		snd_pcm_prepare(hAudio);
	} else {
		LOGV("snd_pcm_writei wrote:%d",w);
	}
}
*/


#endif //INCLUDE_ALSA
