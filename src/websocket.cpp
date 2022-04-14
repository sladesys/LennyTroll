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
#include "webserver.h"
#include "utils.h"


enum {
    WS_OP_CONTINUATION = 0x00,
    WS_OP_TEXT         = 0x01,
    WS_OP_BINARY       = 0x02,
    WS_OP_CLOSE        = 0x08,
    WS_OP_PING         = 0x09,
    WS_OP_PONG         = 0x0A,
};

//
WebSocket_Callbacks::~WebSocket_Callbacks() { LOGT("destructor"); }

//
WebSocket::WebSocket(Socket &s,WebSocket_Callbacks &cb) : sock(s),cbs(cb),active(false) { LOGT("WebSocket constructor"); }
WebSocket::~WebSocket() { LOGT("WebSocket destructor"); }

void WebSocket::run() {
    LOGT("run +");

    LOGT("active = true");
    active = true;

    const unsigned timeout = 10000;

    cbs.onOpen(this);

    while (true) {
        if (0 == sock.poll(timeout,POLLIN)) {
            continue;
        }
        if (!decode()) {
            LOGT("decode return false");
            break;
        }
    }

    close();    

    LOGT("run -");
}
void WebSocket::close() {
    LOGT(" ");

    LOGT("active = false");
    active = false;

    if (0 < sock.fd) {
        sock.close();
        cbs.onClose(this);
    }
}

bool WebSocket::sendText(const char *const txt) {
	if (!active) { LOGC("active = false"); return -1; }
	const size_t len = strlen(txt);
	LOGT("sendText len:%d",len);
	int l = (int)strlen(txt);
    uint8_t *const p = encode(WS_OP_TEXT,false,(unsigned*)&l,(uint8_t*)txt);
    const int r = sock.write((unsigned)l,p);
    free(p);
    return r == l;
}
bool WebSocket::sendBinary(const unsigned len,const uint8_t *const buf) {
	if (!active) { LOGC("active = false"); return -1; }
	//LOGT("sendBinary len:%d",len);
	int l = (int)len;
	uint8_t *const p = encode(WS_OP_BINARY,false,(unsigned*)&l,buf);
    const int r = sock.write((unsigned)l,p);
    free(p);
    return r == l;
}
bool WebSocket::sendPong(const unsigned len,const uint8_t *const buf) {
	if (!active) { LOGC("active = false"); return -1; }
	LOGT("sendPing len:%d",len);
    int l = (int)len;
    uint8_t *const p = encode(WS_OP_PONG,false,(unsigned*)&l,buf);
    const int r = sock.write((unsigned)l,p);
    free(p);
    return r == l;
}
bool WebSocket::sendPing() {	
	if (!active) { LOGC("active = false"); return -1; }
	LOGT("sendPing");
    int l = 0;
    uint8_t *const p = encode(WS_OP_PING,false,(unsigned*)&l,NULL);
    const int r = sock.write((unsigned)l,p);
    free(p);
    return r == l;
}


//
// http://tools.ietf.org/html/rfc6455
// 
// Framing Protocol
// 
//     0                   1                   2                   3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +-+-+-+-+-------+-+-------------+-------------------------------+
//    |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
//    |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
//    |N|V|V|V|       |S|             |   (if payload len==126/127)   |
//    | |1|2|3|       |K|             |                               |
//    +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
//    |     Extended payload length continued, if payload len == 127  |
//    + - - - - - - - - - - - - - - - +-------------------------------+
//    |                               |Masking-key, if MASK set to 1  |
//    +-------------------------------+-------------------------------+
//    | Masking-key (continued)       |          Payload Data         |
//    +-------------------------------- - - - - - - - - - - - - - - - +
//    :                     Payload Data continued ...                :
//    + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
//    |                     Payload Data continued ...                |
//    +---------------------------------------------------------------+
//
//
//	b[0] = (byte)0x81; //FIN=0x80 + 0x00=continuation,0x01=text,0x02=binary,0x08=connection close,0x09=ping,0x0A=pong
//
//

uint8_t* WebSocket::encode(const unsigned opcode,const bool mask,unsigned *const pLength,const uint8_t *const data) {
	const unsigned length = *pLength;
    uint8_t *const p = (uint8_t *)malloc(16 +4 +8 +length);

    unsigned headerLength = 2;

    unsigned maskKeys[] = {0,0,0,0};
    if (mask) {
        headerLength += 4;

        // TODO Create random mask
    //	for (unsigned i=0;i<4;i++) maskKeys[i] = (uint8_t)((rand() / (float)RAND_MAX) * 0xFF);
    }

    unsigned payloadLen = 0;

	if (0x10000 <= length) {
		headerLength += 8;
		
		payloadLen = 0x7F; //127
	} else
	if (0x7E <= length) {
        headerLength += 2;

        payloadLen = 0x7E; //126
    } else {
        payloadLen = length;
    }

    {
        uint8_t *const b = p;
        b[0] = (uint8_t)(0x80 | opcode); // FIN and opcode
        b[1] = (uint8_t)((!mask ?0x0 :0x80) | (payloadLen & 0x7F));

        switch (payloadLen) {
            default:
                if (mask) {
                    memcpy(b+2,&maskKeys,sizeof(maskKeys));
                }
                break;
            case 0x7E:
                b[2] = (uint8_t)(length >>8 &0xFF);
                b[3] = (uint8_t)(length     &0xFF);

                if (mask) {
                    memcpy(b+4,&maskKeys,sizeof(maskKeys));
                }
                break;
            case 0x7F: {
                unsigned long long l = length;

                b[2] = (uint8_t)(l>>56 &0xFF);
                b[3] = (uint8_t)(l>>48 &0xFF);
                b[4] = (uint8_t)(l>>40 &0xFF);
                b[5] = (uint8_t)(l>>32 &0xFF);
                b[6] = (uint8_t)(l>>24 &0xFF);
                b[7] = (uint8_t)(l>>16 &0xFF);
                b[8] = (uint8_t)(l>> 8 &0xFF);
                b[9] = (uint8_t)(l     &0xFF);

                if (mask) {
                    memcpy(b+10,&maskKeys,sizeof(maskKeys));
                }

				break;
            }
        }

        memcpy(b + headerLength,data,length);

        if (mask) {
            // encode the data in place
            uint8_t *const d = b + headerLength;
            for (unsigned i=0;i<length;i++) d[i] = (uint8_t)(d[i] ^ maskKeys[i %4]);
        }
    }

    //
    *pLength = headerLength + length;
    return p;
}
bool WebSocket::decode() {
    uint8_t b[2];
	long len = sock.read(sizeof(b),b);

    if (sizeof(b) != len) {
    	LOGT("read error:%li",len);
        return false;
    }

    const unsigned opcode = (b[0] & 0x0F);
    //const int fin  = 0 != (b[0] & 0x80);
    const int mask = 0 != (b[1] & 0x80);
    
    uint64_t length = 0;
    unsigned headerOffset = 2;
    
    const unsigned payloadLen = (b[1] & 0x7F);
    switch (payloadLen) {
        default:
            length = (uint64_t)payloadLen;
            break;
        case 0x7E: {
            headerOffset += 2;
            
            uint8_t bb[2];
			len = sock.read(sizeof(bb),bb);
            if (sizeof(bb) != len) {
                LOGC("read error:%li",len);
                return false;
            }

            const int b2 = bb[0];
            const int b3 = bb[1];
            if (-1 == b2 || -1 == b3) {
                LOGC("protocol error");
                return false;
            }
            length = (uint64_t)((b2 &0xFF) <<8) + (b3 &0xFF);

			//LOGD("2 byte length:%llu",length);
            break;
        }
        case 0x7F: {
            headerOffset += 8;
            
            uint8_t bb[8];
			len = sock.read(sizeof(bb),bb);
            if (sizeof(bb) != len) {
                LOGC("read error:%li",len);
                return false;
            }
            
            const int64_t b2 = bb[0],b3 = bb[1],b4 = bb[2],b5 = bb[3],b6 = bb[4],b7 = bb[5],b8 = bb[6],b9 = bb[7];
            
            if (-1 == b2 || -1 == b3 || -1 == b4 || -1 == b5 || -1 == b6 || -1 == b7 || -1 == b8 || -1 == b9) {
                LOGC("protocol error");
                return false;
            }
            
            length = (uint64_t)(((b2 &0xFF) <<56) + ((b3 &0xFF) <<48) + ((b4 &0xFF) <<40) + ((b5 &0xFF) <<32) + ((b6 &0xFF) <<24) + ((b7 &0xFF) <<16) + ((b8 &0xFF) <<8) + (b9 &0xFF));
			//LOGD("8 byte length:%llu",length);
            break;
        }
    }
	
    //LOGC("opcode:0x%X fin:%i mask:%i len:%i",opcode,fin,mask,(unsigned)length);
    
    uint8_t maskKeys[] = {0,0,0,0};

    if (mask) {
		len = sock.read(sizeof(maskKeys),(uint8_t*)&maskKeys);
        if (sizeof(maskKeys) != len) {
            LOGC("read error:%li",len);
            return false;
        }
    }

	{
		uint8_t *d = NULL;

		if (0 < length) {

			//
			// read all bytes
			//
			d = (uint8_t*)malloc((size_t)(length +1));
            d[length] = '\0';
			{
				uint8_t *const p = d;
				unsigned i;
				for (i=0;i<length;) {
					len = sock.read((unsigned)(length - i),&p[i]);
					if (0 >= len) {
						LOGC("read error:%li",len);
						return false;
					}
					i += len;
				}
			}
			
			if (mask) {
				// Decode data in place
				uint8_t *const mb = d;
				for (unsigned i=0;i<length;i++) mb[i] = (mb[i] ^maskKeys[i %4]);
			}
		}
		
		// internal
		switch (opcode) {
			default: LOGW("unhandled code:0x%X",opcode); return false;

			case WS_OP_PING: sendPong((unsigned)length,d); return true;
			case WS_OP_PONG: return true;
				
			case WS_OP_TEXT:
				if (NULL == d) { LOGD("data buffer is NULL"); return false; }

				cbs.onText(this,(unsigned)length,(char*)d); return true;

			case WS_OP_BINARY:
				if (NULL == d) { LOGD("data buffer is NULL"); return false; }

				cbs.onBinary(this,(unsigned)length,d); return true;

			case WS_OP_CLOSE:
				LOGT("WS_OP_CLOSE");

				//cbs.onClose(this);
                return false;
		}		
	}
}

