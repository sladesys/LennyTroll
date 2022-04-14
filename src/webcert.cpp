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

#define CERT_DAYS_MAX (365 *2) // for iOS, 825 is max


#ifndef NO_SSL
#include "utils.h"
#include "butils.h"
#include "webcert.h"


void webCertCreate() {
    WebCert c;

    if (!c.create(CERT_DAYS_MAX)) { LOGC("create failed"); return; }
    if (!c.save()) { LOGC("save failed"); return; }

}

std::string webCertJsonInfo() {
    WebCert c;

    if (!c.loadPEM()) {
//        if (!c.create()) { LOGC("create failed"); return "{}"; }
        return "{}";
    }
    return c.getJsonInfo();
}


std::string webCertInfo() {
    WebCert c;

    if (!c.loadPEM()) {
        //LOGC("create");

        if (!c.create(CERT_DAYS_MAX)) { LOGC("create failed"); return ""; }
        if (!c.save()) { LOGC("save failed"); }
    }
    return c.getInfo();

/*
    BIO *const bio = BIO_new(BIO_s_mem());
    BIO_puts(bio,pem.c_str());
    X509 *const x509 = PEM_read_bio_X509(bio,NULL,NULL,NULL);
    BIO_free(bio);
    if (!x509) return "";
    WebCert c; c.x509 = x509;
    str::string s = c.getInfo();
    return s;
*/
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
//
//

static std::string pkcs12FileName();
static std::string csrFileName();
static std::string pemFileName();
static std::string keyFileName();

#include "data.h"
#include <stdio.h>
#include <fcntl.h>

std::string pkcs12FileName() {  // .pfx .p12
    char path[PATH_MAX]; snprintf(path,sizeof(path),"%s/lenny.pfx",PATH_ETC);
    return std::string(path);
}
std::string csrFileName() {
    char path[PATH_MAX]; snprintf(path,sizeof(path),"%s/lenny.csr",PATH_ETC);
    return std::string(path);
}
std::string pemFileName() {
    char path[PATH_MAX]; snprintf(path,sizeof(path),"%s/lenny.pem",PATH_ETC);
    return std::string(path);
}
std::string keyFileName() {
    char path[PATH_MAX]; snprintf(path,sizeof(path),"%s/lenny.key",PATH_ETC);
    return std::string(path);
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <sstream>
#include <map>
#include <vector>


//
// internals
//
//static std::string _asn1int(ASN1_INTEGER *bs);
//static std::string _asn1string(ASN1_STRING *d);
static std::string asn1datetime_isodatetime(const ASN1_TIME *tm);
//static std::string asn1date_isodate(const ASN1_TIME *tm);
static void _asn1dateparse(const ASN1_TIME *time, int& year, int& month, int& day, int& hour, int& minute, int& second);

static std::string _subject_as_line(X509_NAME *subj_or_issuer);
//static std::map<std::string,std::string> _subject_as_map(X509_NAME *subj_or_issuer);

static std::string public_key_type(X509 *x509);
static int public_key_size(X509 *x509);
//static std::string public_key_ec_curve_name(X509 *x509);

//static std::string thumbprint(X509 *x509);
static std::vector<std::string> subject_alt_names(X509 *x509);
//static std::vector<std::string> ocsp_urls(X509 *x509);
//static std::vector<std::string> crl_urls(X509 *x509);




// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <openssl/asn1.h> //ASN1_STRING_data

std::string WebCert::getInfo() const {

    std::ostringstream ss;

    ss <<"  Cert: " <<public_key_type(x509) <<"-" <<public_key_size(x509)<<" { " << asn1datetime_isodatetime(X509_get_notBefore(x509)) <<" - "<< asn1datetime_isodatetime(X509_get_notAfter(x509)) <<" }"<<std::endl;

    ss <<"    for: ";
    std::vector<std::string> sans = subject_alt_names(x509);
    for (unsigned i=0,c=(unsigned)sans.size();i<c;i++) { if (0 < i) ss <<", "; ss <<sans[i]; }
    ss <<std::endl;

    std::string issuer = _subject_as_line(X509_get_issuer_name(x509)),subject = _subject_as_line(X509_get_subject_name(x509));
    if (0 == issuer.compare(subject)) {        
//      ss <<"    subject: " <<subject  <<std::endl;
    } else {
        ss <<"    issuer : " <<issuer  <<std::endl;
        ss <<"    subject: " <<subject  <<std::endl;
    }

    return ss.str();
}

std::string WebCert::getJsonInfo() const {

    std::ostringstream ss;

    ss <<"{";
    ss <<"\"type\":\"" <<public_key_type(x509) <<"-" <<public_key_size(x509) <<"\"";
    ss <<",";

    {
        std::string issuer = _subject_as_line(X509_get_issuer_name(x509)),subject = _subject_as_line(X509_get_subject_name(x509));
        if (0 == issuer.compare(subject)) {        
            ss <<"\"subject\":\"" <<subject <<"\"";
        } else {
            ss <<"\"subject\":\"" <<subject <<"\"";
            ss <<",";
            ss <<"\"issuer\":\"" <<issuer <<"\"";
        }
    }

    ss <<",";
    ss <<"\"from\":\"" << asn1datetime_isodatetime(X509_get_notBefore(x509)) <<"\"";
    ss <<",";
    ss <<"\"to\":\"" << asn1datetime_isodatetime(X509_get_notAfter(x509)) <<"\"";
    ss <<",";
    ss <<"\"hosts\":[";
    {
        std::vector<std::string> sans = subject_alt_names(x509);
        for (unsigned i=0,c=(unsigned)sans.size();i<c;i++) {
            if (0 < i) ss <<",";
            ss <<"\"" <<sans[i] <<"\"";
        }
    }
    ss <<"]";
    ss <<"}";

    return ss.str();
}



/*
std::string WebCert::getInfo() {

	std::ostringstream ss;

    OpenSSL_add_all_algorithms();

    //
    // 0 = not CA certificate
    // 1 = is proper X509v3 CA certificate with basicConstraints extension CA:TRUE
    // 3 = is self-signed X509 v1 certificate
    // 4 = is certificate with keyUsage extension with bit keyCertSign set, but without basicConstraints
    // 5 = has outdated Netscape Certificate Type extension telling that it is CA certificate
    //

    ss <<"CheckCA: " << X509_check_ca(x509) <<std::endl;

    ss <<"Thumbprint: " << thumbprint(x509) <<std::endl;
//    ss <<"Version: "    <<(1+ X509_get_version(x509)) <<std::endl;
    ss <<"Serial: "     <<_asn1int(X509_get_serialNumber(x509)) <<std::endl;

    ss <<"Issuer: " << _subject_as_line(X509_get_issuer_name(x509)) <<std::endl;

    if (false) {
	    std::map<std::string,std::string> ifields = _subject_as_map(X509_get_issuer_name(x509));
	    for (std::map<std::string,std::string>::iterator i = ifields.begin(), ix = ifields.end(); i != ix; i++ )
	        ss << " * " << i->first << " : " << i->second <<std::endl;
	}

    ss <<"Subject: "    << _subject_as_line(X509_get_subject_name(x509)) <<std::endl;


    if (false) {
	    std::map<std::string,std::string> sfields = _subject_as_map(X509_get_subject_name(x509));
	    for (std::map<std::string,std::string>::iterator i = sfields.begin(), ix = sfields.end(); i != ix; i++ )
	        ss << " * " <<  i->first << " : " << i->second <<std::endl;
	}


    ss <<"SignatureAlgorithm: "<< std::string( OBJ_nid2ln( OBJ_obj2nid((x509)->sig_alg->algorithm) ) ) <<std::endl;
    ss <<"PublicKeyType: "     << public_key_type(x509) << public_key_ec_curve_name(x509) <<std::endl;
    ss <<"PublicKeySize: "     << public_key_size(x509) <<std::endl;
    ss <<"NotBefore: "         << asn1datetime_isodatetime(X509_get_notBefore(x509)) <<std::endl;
    ss <<"NotAfter: "          << asn1datetime_isodatetime(X509_get_notAfter(x509)) <<std::endl;

    ss <<"SubjectAltName(s):" <<std::endl;
    std::vector<std::string> sans = subject_alt_names(x509);
    for (int i=0, ix=sans.size(); i<ix; i++) {
        ss << " " << sans[i] <<std::endl;
    }

    ss <<"CRL URLs:" <<std::endl;
    std::vector<std::string> crls = crl_urls(x509);
    for (int i=0, ix=crls.size(); i<ix; i++) ss << " " << crls[i] <<std::endl;

    ss <<"OCSP URLs:" <<std::endl;
    std::vector<std::string> urls = ocsp_urls(x509);
    for (int i=0, ix=urls.size(); i<ix; i++) ss << " " << urls[i] <<std::endl;

    return ss.str();
}
*/



#include <openssl/pem.h>
#include <openssl/bio.h>




WebCert::WebCert() : x509(NULL),pkey(NULL),rsa(NULL),bn(NULL) {}
WebCert::~WebCert() {

#ifndef OPENSSL_101
    if (NULL != bn) {
        BN_free(bn);
    }
    if (NULL != rsa) {
        RSA_free(rsa);
    }
#endif //OPENSSL_101

	if (NULL != pkey) {
	    EVP_PKEY_free(pkey);
	}
	if (NULL != x509) {
		X509_free(x509);
	}
}

#include <openssl/err.h>


bool WebCert::load(SSL_CTX *const ctx) {

#ifdef TEST

    bool result = false;

//    std::string pem = getPEM(),key = getKEY();
    std::string pem = readFile(pemFileName().c_str()),key = readFile(keyFileName().c_str());

    // create BIO
    BIO *const bio = BIO_new_mem_buf(pem.c_str(),-1);

    // read PEM formatted certificate from memory into an X509 structure
    X509 *const cert = PEM_read_bio_X509(bio,NULL,0,NULL);

    // create a bio for the RSA key
    BIO *const kbio = BIO_new_mem_buf(key.c_str(),-1);

    // read the key bio into an RSA object
//  RSA *const rsa = PEM_read_bio_RSAPrivateKey(kbio,NULL,0,NULL);
    EVP_PKEY *const pkey = PEM_read_bio_PrivateKey(kbio,NULL,0,NULL);

    if (NULL == bio) {
        LOGC("BIO_new_mem_buf failed");
    } else
    if (NULL == cert) {
        LOGC("PEM_read_bio_X509 failed");
    } else

    if (NULL == kbio) {
        LOGC("BIO_new_mem_buf failed");
    } else

    if (NULL == pkey) {
        LOGC("PEM_read_bio_PrivateKey failed");
    } else

    if (1 != SSL_CTX_use_certificate(ctx,cert)) {
        LOGVC("SSL_CTX_use_certificate_file failed");
        if (LOGGING_CONTROL) ERR_print_errors_fp(stderr);
    } else

    if (1 != SSL_CTX_use_PrivateKey(ctx,pkey)) {
        LOGC("SSL_CTX_use_PrivateKey_file failed");
        if (LOGGING_CONTROL) ERR_print_errors_fp(stderr);
    } else {
        //
        // OK
        //
        result = true;
    }

    //
    // free resources that have been allocated by openssl functions
    //
    if (pkey) EVP_PKEY_free(pkey);
    if (kbio) BIO_free(kbio);
    if (cert) X509_free(cert);
    if (bio) BIO_free(bio);

    return result;

#else //TEST

    if (!loadPEM() || !loadPKey()) {
        return false;
    }

    if (1 != SSL_CTX_use_certificate(ctx,x509)) {
        LOGC("SSL_CTX_use_certificate_file failed");
        if (LOGGING_CONTROL) ERR_print_errors_fp(stderr);
        return false;
    }

    if (1 != SSL_CTX_use_PrivateKey(ctx,pkey)) {
        LOGC("SSL_CTX_use_PrivateKey_file failed");
        if (LOGGING_CONTROL) ERR_print_errors_fp(stderr);
        return false;
    }

#ifdef DEBUG
    if (1 != SSL_CTX_check_private_key(ctx)) {
        LOGC("SSL_CTX_check_private_key failed");
        if (LOGGING_CONTROL) ERR_print_errors_fp(stderr);
    //  return false;
    }
#endif //DEBUG

#endif //TEST


    // Used only if client authentication will be used
    //  SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,NULL);
    //  SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE,NULL);

/*
    // Load certificates of trusted CAs based on file provided
    if (1 != SSL_CTX_load_verify_locations(ctx,PEM_FILE,NULL)) {
        LOGC("Error setting the verify locations.\n");
        if (LOGGING_CONTROL) ERR_print_errors_fp(stderr);
    //  return false;
    }
*/

    return true;
}


bool WebCert::loadPEM() {

	if (true) {
        std::string pem = readFile(pemFileName().c_str());

        if (0 == pem.length()) {

            if (0 == pem.length()) {
                //LOGC("no certificate");
                return false;
            }
        }

        {
    		BIO *const bio = BIO_new(BIO_s_mem());
    	//	BIO_write(bio,pem.c_str(),pem.length());
    		BIO_puts(bio,pem.c_str());

    		x509 = PEM_read_bio_X509(bio,NULL,NULL,NULL);

    		BIO_free(bio);
        }

		if (!x509) {
		    fprintf(stderr,"unable to parse certificate in memory\n");
            return false;
		}
        return true;
    }

    if (false) {
		FILE *const f = fopen(pemFileName().c_str(),"r");

		if (!f) {
			LOGC("fopen failed file: %s",pemFileName().c_str());
            return false;
		}

		x509 = PEM_read_X509(f,NULL,NULL,NULL);
		fclose(f);

		if (!x509) {
			LOGC("PEM_read_X509 failed file:%s",pemFileName().c_str());
            return false;
		}
        return true;
	}

    return false;
}

bool WebCert::loadPKey() {
    if (true) {

        std::string key = readFile(keyFileName().c_str());

        {
            BIO *const bio = BIO_new_mem_buf((void*)key.c_str(),-1);

            if (NULL == bio) {
                LOGC("BIO_new_mem_buf failed");
                return false;
            }

            pkey = PEM_read_bio_PrivateKey(bio,NULL,0,NULL);

            if (NULL == pkey) {
                LOGC("PEM_read_bio_PrivateKey failed");
                return false;
            }
            if (bio) BIO_free(bio);
        }
    }

    return true;
}




#include <openssl/pkcs12.h>

bool WebCert::save() {

    if (false) {
        //
        // openssl pkcs12 -export -out lenny.pfx -inkey lenny.key -in lenny.pem
        //
        PKCS12 *const p12 = PKCS12_create(NULL,"Lenny",pkey,x509,NULL, -1,-1,-1,-1,0);
        if (NULL == p12) {
            LOGC("PKCS12_create failed");
        } else {

            std::string path = pkcs12FileName();
            FILE *const f = fopen(path.c_str(),"wb");
            if (!f) {
                LOGC("fopen failed file:%s",path.c_str());
                PKCS12_free(p12);
                return false;
            }
        
            const bool ret = i2d_PKCS12_fp(f,p12);
            PKCS12_free(p12);
            fclose(f);
        
            if (!ret) {
                LOGC("PEM_write_X509_REQ failed file:%s",path.c_str());
                return false;
            }


             // no password
             if (!PKCS12_verify_mac(p12,NULL,0)) {
                 // empty password
                 if (!PKCS12_verify_mac(p12,"",-1)) {
                    LOGC("PKCS12_verify_mac failed both no and empty password");
                 }
             }
        }
    }

    if (false) {
        X509_REQ *const req = X509_REQ_new();
        if (!req) {
            LOGC("X509_REQ_new failed");
            return false;
        }

        if (0 > X509_REQ_set_pubkey(req,pkey)) {
            LOGC("X509_REQ_set_pubkey failed");
            X509_REQ_free(req);
            return false;
        }

        if (0 > X509_REQ_sign(req,pkey,EVP_sha256())) {
            LOGE("X509_REQ_sign failed");
            X509_REQ_free(req);
            return false;
        }

        std::string path = csrFileName();
        FILE *const f = fopen(path.c_str(),"wb");
        if (!f) {
            LOGC("fopen failed file:%s",path.c_str());
            return false;
        }
    
        const bool ret = PEM_write_X509_REQ(f,req);
        fclose(f);
    
        if (!ret) {
            LOGE("PEM_write_X509_REQ failed file:%s",path.c_str());
            return false;
        }
    }
	{
        std::string path = pemFileName();
		FILE *const f = fopen(path.c_str(),"wb");
		if (!f) {
			LOGC("fopen failed file:%s",path.c_str());
			return false;
		}
    
	    const bool ret = PEM_write_X509(f,x509);
		fclose(f);
    
		if (!ret) {
			LOGC("PEM_write_X509 failed file:%s",path.c_str());
			return false;
		}
	}
	{
        std::string path = keyFileName();
		FILE *const f = fopen(path.c_str(),"wb");
		if (!f) {
			LOGC("fopen failed file:%s",path.c_str());
			return false;
		}
    
	    const bool ret = PEM_write_PrivateKey(f,pkey,NULL,NULL,0,NULL,NULL);
		fclose(f);
    
		if (!ret) {
			LOGC("PEM_write_PrivateKey failed file:%s",path.c_str());
			return false;
		}
	}

	return true;
}


unsigned WebCert::getSerialNo() const {
    ASN1_INTEGER *const v = X509_get_serialNumber(x509);
    const unsigned r = (unsigned)ASN1_INTEGER_get(v);
    ASN1_INTEGER_free(v);
    return r;
}

//
//
//
#include <openssl/bn.h>

#define RSA_BITS_MIN 2048
#define X509_VERSION 2

static bool add_ext(X509 *const x509,const int nid,const char *const value);

bool WebCert::create(const unsigned days) {
	//LOGC(" ");
    if (x509) { LOGC("x509 not NULL"); return false; }

    std::string san;

    // for development
    //san.append("DNS: localhost,DNS: localhost.localdomain,IP: 127.0.0.1,IP: ::1"

    std::string host = getHostName();
    if (0 < host.length()) {
        san.append("DNS: ").append(host).append(",");
    } else {
        host = "lenny";
    }

    {
        //san.append("critical,");
        std::map<std::string,std::string> m = getNetworkIpV4Addresses();
        unsigned c = 0;
        for (std::map<std::string,std::string>::iterator it = m.begin(); it != m.end(); it++) {
            if (0 < c++) san.append(",");
            san.append("IP: ").append(it->second);
        }
    }

    //LOGC("hostname:%s san:%s",host.c_str(),san.c_str());

	x509 = X509_new();

	if (NULL == x509) {
		LOGE("X509_new failed");
		return false;
	}


	{
        fprintf(stdout,"%s\n"," Lenny creating new RSA secure key, may take a few seconds...");
        fflush(stdout);

#ifdef OPENSSL_101

		rsa = RSA_generate_key(RSA_BITS_MIN,RSA_3,NULL,NULL);

		if (NULL == rsa) {
			LOGE("RSA_generate_key failed");
			return false;
		}

#else //OPENSSL_101

        rsa = RSA_new();
        bn = BN_new();

        if (1 != BN_set_word(bn,RSA_F4)) {
            LOGC("BN_set_word failed");
        }

        if (1 != RSA_generate_key_ex(rsa,2048,bn,NULL)) {
            LOGC("RSA_generate_key_ex failed");
        }

#endif //OPENSSL_101

        fprintf(stdout,"%s\n"," Lenny finsihed creating new 2048 bit RSA key");
        fflush(stdout);
    }

    //
    // Container
    //
    {
        pkey = EVP_PKEY_new();

        if (NULL == pkey) {
            LOGE("EVP_PKEY_new failed");
            return false;
        }

		if (!EVP_PKEY_assign_RSA(pkey,rsa)) {
			LOGE("EVP_PKEY_assign_RSA failed");
			return false;		
		}

        static unsigned rseed = 0;
        if (0 == rseed) { srand(rseed = (unsigned)time(NULL)); }
		ASN1_INTEGER_set(X509_get_serialNumber(x509),rand_r(&rseed));

        X509_set_version(x509,X509_VERSION);
		X509_gmtime_adj(X509_get_notBefore(x509),0);
		X509_gmtime_adj(X509_get_notAfter( x509),(long)(24*60*60) * days);

		X509_set_pubkey(x509,pkey);
	}

	{
		X509_NAME *const name = X509_get_subject_name(x509);

		X509_NAME_add_entry_by_txt(name,"C" ,MBSTRING_ASC,(uint8_t*)"US"            ,-1,-1,0);
		X509_NAME_add_entry_by_txt(name,"ST",MBSTRING_ASC,(uint8_t*)"CA"            ,-1,-1,0);
		X509_NAME_add_entry_by_txt(name,"L" ,MBSTRING_ASC,(uint8_t*)"Saratoga"      ,-1,-1,0);
		X509_NAME_add_entry_by_txt(name,"O" ,MBSTRING_ASC,(uint8_t*)"LennyTroll"    ,-1,-1,0);
		X509_NAME_add_entry_by_txt(name,"OU",MBSTRING_ASC,(uint8_t*)"LennyTroll"    ,-1,-1,0);
		X509_NAME_add_entry_by_txt(name,"CN",MBSTRING_ASC,(uint8_t*)host.c_str()    ,-1,-1,0);

		// self signed
		X509_set_issuer_name(x509,name);
	}

	add_ext(x509,NID_subject_key_identifier   ,"hash");
    add_ext(x509,NID_authority_key_identifier ,"keyid,issuer");
	add_ext(x509,NID_basic_constraints        ,"critical,CA:TRUE");
    add_ext(x509,NID_key_usage                ,"critical,keyCertSign,cRLSign,digitalSignature");
	add_ext(x509,NID_ext_key_usage            ,"serverAuth");

	// san = subject alternative names
    add_ext(x509,NID_subject_alt_name         ,san.c_str());

	if (!X509_sign(x509,pkey,EVP_sha256())) {
		LOGE("X509_sign failed");
		return false;
	}

	return true;
}

#include <openssl/x509v3.h>

bool add_ext(X509 *const x509,const int nid,const char *const value) {
    
    // Set 'context' of the extensions, no configuration database
    X509V3_CTX ctx;
    X509V3_set_ctx_nodb(&ctx);

    //
    // Issuer and subject certs: both the target since it is self signed, no request and no CRL
    //
    X509V3_set_ctx(&ctx,x509,x509,NULL,NULL,0);
    X509_EXTENSION *const ex = X509V3_EXT_conf_nid(NULL,&ctx,nid,(char*)value);

    if (!ex) {
        LOGE("X509V3_EXT_conf_nid failed nid:%d v:%s",nid,value);
        return false;
    }

    X509_add_ext(x509,ex,-1);

    X509_EXTENSION_free(ex);
    return true;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*
std::string _asn1int(ASN1_INTEGER *bs) {
    static const char hexbytes[] = "0123456789ABCDEF";
    std::ostringstream ashex;
    for(int i=0; i<bs->length; i++) {
        ashex << hexbytes[ (bs->data[i]&0xf0)>>4  ] ;
        ashex << hexbytes[ (bs->data[i]&0x0f)>>0  ] ;
    }
    return ashex.str();
}
std::string _asn1string(ASN1_STRING *const d) {
    std::string asn1_string;
    if (ASN1_STRING_type(d) != V_ASN1_UTF8STRING) {
        unsigned char *utf8;
        int length = ASN1_STRING_to_UTF8( &utf8, d );
        asn1_string= std::string( (char*)utf8, length );
        OPENSSL_free( utf8 );
    } else { 
        asn1_string = std::string( (char*)ASN1_STRING_data(d), ASN1_STRING_length(d) );
    }
    return asn1_string;
}
*/
std::string asn1datetime_isodatetime(const ASN1_TIME *const tm) {
    int year=0, month=0, day=0, hour=0, min=0, sec=0;
    _asn1dateparse(tm,year,month,day,hour,min,sec);
 
    char buf[64] ="";
    snprintf(buf, sizeof(buf)-1,"%04d-%02d-%02d %02d:%02d:%02d GMT",year,1+month,day,hour,min,sec);
    return std::string(buf);
}
/*
std::string asn1date_isodate(const ASN1_TIME *const tm) {
    int year=0, month=0, day=0, hour=0, min=0, sec=0;
    _asn1dateparse(tm,year,month,day,hour,min,sec);
 
    char buf[25]="";
    snprintf(buf, sizeof(buf)-1, "%04d-%02d-%02d", year, month, day);
    return std::string(buf);
}
*/
void _asn1dateparse(const ASN1_TIME *time,int& year,int& month,int& day,int& hour,int& minute,int& second) {
    const char *str = (const char*) time->data;
    size_t i = 0;
    if (time->type == V_ASN1_UTCTIME) {/* two digit year */
        year = (str[i++] - '0') * 10;
        year += (str[i++] - '0');
        year += (year < 70 ? 2000 : 1900);
    } else if (time->type == V_ASN1_GENERALIZEDTIME) {/* four digit year */
        year = (str[i++] - '0') * 1000;
        year+= (str[i++] - '0') * 100;
        year+= (str[i++] - '0') * 10;
        year+= (str[i++] - '0');
    }
    month  = (str[i++] - '0') * 10;
    month += (str[i++] - '0') - 1; // -1 since January is 0 not 1.
    day    = (str[i++] - '0') * 10;
    day   += (str[i++] - '0');
    hour   = (str[i++] - '0') * 10;
    hour  += (str[i++] - '0');
    minute  = (str[i++] - '0') * 10;
    minute += (str[i++] - '0');
    second  = (str[i++] - '0') * 10;
    second += (str[i++] - '0');
}
std::string _subject_as_line(X509_NAME *subj_or_issuer) {
    BIO * bio_out = BIO_new(BIO_s_mem());
    X509_NAME_print(bio_out,subj_or_issuer,0);
    BUF_MEM *bio_buf;
    BIO_get_mem_ptr(bio_out, &bio_buf);
    std::string issuer = std::string(bio_buf->data, bio_buf->length);
    BIO_free(bio_out);
    return issuer;
}
/*
std::map<std::string,std::string> _subject_as_map(X509_NAME *subj_or_issuer) {
    std::map<std::string,std::string> m;    
    for (int i = 0; i < X509_NAME_entry_count(subj_or_issuer); i++) {
        X509_NAME_ENTRY *e = X509_NAME_get_entry(subj_or_issuer, i);
        ASN1_STRING *d = X509_NAME_ENTRY_get_data(e);
        ASN1_OBJECT *o = X509_NAME_ENTRY_get_object(e);
        const char* key_name = OBJ_nid2sn( OBJ_obj2nid( o ) );
        m[key_name] = _asn1string(d);
    }
    return m;
}
*/

#ifndef OPENSSL_101
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <crypto/evp.h>
#pragma GCC diagnostic pop
#endif //OPENSSL_101

std::string public_key_type(X509 *const x509) {
    EVP_PKEY *const pkey = X509_get_pubkey(x509);
    if (NULL == pkey) return "RSA";

  //const int t = EVP_PKEY_type(pkey->type);
    const int t = EVP_PKEY_base_id(pkey);
    EVP_PKEY_free(pkey);

    switch (t) {
        case EVP_PKEY_RSA: return "RSA";
        case EVP_PKEY_DSA: return "DSA";
        case EVP_PKEY_DH : return "DH";
        case EVP_PKEY_EC : return "ECC";
        default          : return "UNK";
    }
}

int public_key_size(X509 *const x509) {
    return 8*EVP_PKEY_size(X509_get_pubkey(x509));
}

std::vector<std::string> subject_alt_names(X509 *const x509) {
    std::vector<std::string> list;
    GENERAL_NAMES* subjectAltNames = (GENERAL_NAMES*)X509_get_ext_d2i(x509, NID_subject_alt_name, NULL, NULL);
    for (int i = 0; i < sk_GENERAL_NAME_num(subjectAltNames); i++) {
        GENERAL_NAME* gen = sk_GENERAL_NAME_value(subjectAltNames, i);
        if (gen->type == GEN_URI || gen->type == GEN_DNS || gen->type == GEN_EMAIL) {
            ASN1_IA5STRING *asn1_str = gen->d.uniformResourceIdentifier;
#ifdef OPENSSL_101
            std::string san = std::string( (char*)ASN1_STRING_data(asn1_str),(std::string::size_type)ASN1_STRING_length(asn1_str) );
#else //OPENSSL_101
            std::string san = std::string( (char*)ASN1_STRING_get0_data(asn1_str),(std::string::size_type)ASN1_STRING_length(asn1_str) );
#endif //OPENSSL_101
            list.push_back( san );
        } else 

        if (gen->type == GEN_IPADD) {
            unsigned char *p = gen->d.ip->data;
            if (gen->d.ip->length == 4) {
                std::ostringstream ip;
                ip << (int)p[0] << '.' << (int)p[1] << '.' << (int)p[2] << '.' << (int)p[3];
                list.push_back( ip.str() );
            }
            else //if(gen->d.ip->length == 16) //ipv6?
            {
                //std::cerr << "Not implemented: parse sans ("<< __FILE__ << ":" << __LINE__ << ")" << endl;
            }
        } else 
        {
            //std::cerr << "Not implemented: parse sans ("<< __FILE__ << ":" << __LINE__ << ")" << endl;
        }
    }
    GENERAL_NAMES_free(subjectAltNames);
    return list;
}

#endif //NO_SSL
