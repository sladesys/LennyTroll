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


#include <string>

extern void webCertCreate();
extern std::string webCertInfo();
extern std::string webCertJsonInfo();


#ifndef NO_SSL
#include <openssl/ssl.h>
#include <openssl/x509.h>

class WebCert {
public:
    X509 *x509;
    EVP_PKEY *pkey;

#ifndef OPENSSL_101
    RSA *rsa;
    BIGNUM *bn;
#endif //OPENSSL_101

    WebCert();
    ~WebCert();

    bool load(SSL_CTX *ctx);
    bool loadPEM();
    bool loadPKey();

    unsigned getSerialNo() const;
    std::string getInfo() const;
    std::string getJsonInfo() const;

    bool create(unsigned days);
    bool save();
};
#endif //NO_SSL

