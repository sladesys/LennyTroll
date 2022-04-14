##
##
##	This file is part of Lenny Troll project
##	Copyright 2020 Slade Systems
##
##	Licensed under the Apache License, Version 2.0 (the "License");
##	you may not use this file except in compliance with the License.
##	You may obtain a copy of the License at
##
##    	http://www.apache.org/licenses/LICENSE-2.0
##
##	Unless required by applicable law or agreed to in writing, software
##	distributed under the License is distributed on an "AS IS" BASIS,
##	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##	See the License for the specific language governing permissions and
##	limitations under the License.
##
##

##
##
## make BUILD=release
##
##

ifeq ($(VERSION_MJR),)
VERSION_MJR=0
VERSION_MNR=1
endif

CFLAGS += -D APP_VERSION_MAJOR=$(VERSION_MJR) -D APP_VERSION_MINOR=$(VERSION_MNR)
CFLAGS += -D LICENSE_APP_VERSION=1
CFLAGS += -D NO_LICENSE

#
# LICENSE_APP_VERSION = 1; // 0 - 255 ~ 00.1 - 25.5 ~ 0.01 - 2.55
#


all: lenny

lenny: \
	main.cpp logo.cpp \
	biz.cpp butils.cpp license.cpp \
	lenny.cpp lennyApi.cpp config.cpp data.cpp stats.cpp json.cpp \
	webclient.cpp webserver.cpp websocket.cpp webcert.cpp \
	modem.cpp audio.cpp utils.cpp threader.cpp

	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $^ $(LDLIBS)


# @echo "OS = $(OS)"
# @echo "shell = $(shell sh -c 'uname 2>/dev/null || echo Unknown')"
info:
	@echo "BUILD: $(BUILD)"
	@echo "Host OS: $(HOST_OS)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDLIBS: $(LDLIBS)"

clean:
	-rm -r *.dSYM
	-rm lenny lenny-arm32 lenny-arm64



## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ##

#
# http://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
# https://www.gnu.org/software/make/manual/html_node/Implicit-Variables.html
#

CFLAGS  += -std=c++11
CFLAGS  += -W -Wall -Wextra
#CFLAGS  += -static -static-libgcc -static-libstdc++

# make BUILD=release
ifeq ($(BUILD),release)
#echo "RELEASE build"
CFLAGS  += -Os -s
else
#echo "DEBUG build"
CFLAGS  += -g -O0 -DDEBUG
endif


#
# gcc as lint
#
# https://invisible-island.net/personal/lint-tools.html
#
# -ansi -pedantic: best to compile in ANSI mode.
# -W -Wall: enable a bunch of warnings.
# -Wundef: undefined macro variables used in #if.
# -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations: make sure that functions are prototyped properly.
# -O2: enable enough optimizations that gcc can identify variables that may be used before being defined.
# There are other options that you may or may not wish to use. 
# In particular, the -Wcast-qual and -Wwrite-strings are useful if you are starting a project from scratch, and are willing to add const to your function prototypes.
# -Wtraditional is useful if you are going to have to port to systems for which no ANSI compiler is available.
#
#CFLAGS    += -Wstrict-prototypes -Wmissing-prototypes
CXXFLAGS  += -Wshadow -Wconversion




## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ##

#
# OpenSSL additions
#

LDLIBS  += -lssl -lcrypto


#
#
#

ifeq ($(OS),Windows_NT)
    HOST_OS := "Windows"
else
    HOST_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

ifeq ($(HOST_OS),Darwin) # Mac OSX
    CFLAGS   += -D OSX
    CFLAGS   += -DTARGET_OS_SIMULATOR -DTARGET_OS_IPHONE -DTARGET_CPU_X86_64

    CFLAGS   += -I ../lib/openssl/include -I ../lib/openssl/include/openssl
    LDLIBS   += -L ../lib/openssl
endif
ifeq ($(HOST_OS),Linux)
    CFLAGS   += -D LINUX
    LDLIBS   += -lm -lcrypt -ldl

    CFLAGS   += -I ../lib/openssl/include -I ../lib/openssl/include/openssl
    LDLIBS   += -L ../lib/openssl

    LDLIBS   += -lusb -ludev
endif



## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ##

#
#
#

LDLIBS  += -lstdc++ -lpthread

