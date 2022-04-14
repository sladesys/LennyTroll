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
## Release process
##
VERSION_MJR = 0
VERSION_MNR = 12

##
##
all : lenny


#
# RPi build prerequisite
#
# sudo apt install libudev-dev libusb-dev 
#

# default to debug build
#BUILD_RELEASE=BUILD=release
 BUILD_RELEASE=BUILD=$(BUILD)

VERSION = $(VERSION_MJR).$(VERSION_MNR)

lenny :
	make -C src $(BUILD_RELEASE) VERSION_MJR=$(VERSION_MJR) VERSION_MNR=$(VERSION_MNR);

clean:
	make -C src clean;
	@#-rm -r lib
	@#-rm -r dist



## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ##

##
## https://wiki.openssl.org/index.php/Compilation_and_Installation
##
OPENSSL_VER = 1.1.1
OPENSSL_VERSION = $(OPENSSL_VER)n


#
# git clone git://git.openssl.org/openssl.git
#

define SCRIPT_OPENSSL_BUILD_PI
	cd project; mkdir lib; cd lib; \
	curl -O https://www.openssl.org/source/openssl-$(OPENSSL_VERSION).tar.gz; \
	tar -xvzf openssl-$(OPENSSL_VERSION).tar.gz; \
	mv openssl-$(OPENSSL_VERSION) openssl; \
	cd openssl; ./Configure linux-armv4 -static; make; cd ..; \
	rm openssl-$(OPENSSL_VERSION).tar.gz;
endef

define SCRIPT_OPENSSL_BUILD_DARWIN
	cd project; mkdir lib; cd lib; \
	curl -O https://www.openssl.org/source/openssl-$(OPENSSL_VERSION).tar.gz; \
	tar -xvzf openssl-$(OPENSSL_VERSION).tar.gz; \
	mv openssl-$(OPENSSL_VERSION) openssl; \
	cd openssl; ./Configure darwin64-x86_64-cc -static; make; cd ..; \
	rm lib/openssl-$(OPENSSL_VERSION).tar.gz;
endef

.PHONY : lib_openssl_setup
lib_openssl:
	$(SCRIPT_OPENSSL_BUILD_PI)

lib_openssl_darwin:
	$(SCRIPT_OPENSSL_BUILD_DARWIN)



## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ##

##
## https://github.com/javascript-obfuscator/javascript-obfuscator
##

JAVASCRIPT_OBFUSCATOR = ~/Applications/javascript-obfuscator/bin/javascript-obfuscator
JAVASCRIPT_OBFUSCATOR_OPTS = \
	--compact true --self-defending false \
	--identifier-names-generator hexadecimal \
	--string-array true --shuffle-string-array true --string-array-threshold 0.75 --rotate-string-array true

DIST_DIR = ./dist

#
#  /bin
#  /etc
#  /opt
#  /var
#

#
#  /bin/lenny
#  /etc/lenny.cfg
#  /etc/lenny_service.txt
#
#  /opt/aud/disconnected.wav
#  /opt/aud/tad_default.wav
#  /opt/aud/lenny/lenny_00.wav
#
#  /opt/web
#  /opt/web/css
#  /opt/web/img
#  /opt/web/js
#
#  /var/call/new.log
#  /var/call/answer.log
#  /var/call/histry.log
#  /var/call/YYYYMMDDHHMMSS_1.wav
#
#  /var/lenny/profile_00.wav
#
#  /var/skiplist.txt
#  /var/tad_1.wav
#
#  /var/log/lenny.log
#

.PHONY: dist
dist:
	-rm -r $(DIST_DIR)
	cp -r root $(DIST_DIR)

	mkdir -p $(DIST_DIR)/bin
	mkdir -p $(DIST_DIR)/var/call
	mkdir -p $(DIST_DIR)/var/lenny
	mkdir -p $(DIST_DIR)/var/log

	cp ./LICENSE  $(DIST_DIR)
	cp  src/lenny $(DIST_DIR)/bin

	@#
	@#
	@#
	-rm $(DIST_DIR)/opt/web/test.html
	-rm $(DIST_DIR)/opt/web/js/test.js
	-rm $(DIST_DIR)/opt/web/js/testui.js
	-rm $(DIST_DIR)/opt/web/script.html
	-rm $(DIST_DIR)/opt/web/script.json

	@#
	@# remove .DS_Store & .gitignore entries
	@#
	find $(DIST_DIR) -name .DS_Store  -exec rm '{}' \;
	find $(DIST_DIR) -name .gitignore -exec rm '{}' \;

	@#
	@# remove extended attributes entries
	@#
	@#find $(DIST_DIR) -print0 | xargs -0 xattr -c;

	@#
	@# fixup javascript ...
	@#

	@#sed -i .tmp "s/const appVersion = '0.0';/const appVersion = '$(VERSION) beta';/" $(DIST_DIR)/opt/web/js/l.js
	@#sed -i .tmp "s/var dbg = true;/var dbg = false;/" $(DIST_DIR)/opt/web/js/l.js
	sed -i "s/const appVersion = '0.0';/const appVersion = '$(VERSION) beta';/" $(DIST_DIR)/opt/web/js/l.js
	sed -i "s/var dbg = true;/var dbg = false;/" $(DIST_DIR)/opt/web/js/l.js

	@#
	@# obfuscate javascript
	@#
	@#$(JAVASCRIPT_OBFUSCATOR) $(DIST_DIR)/opt/web/js/l.js  --output $(DIST_DIR)/opt/web/js/l.js  $(JAVASCRIPT_OBFUSCATOR_OPTS)
	@#$(JAVASCRIPT_OBFUSCATOR) $(DIST_DIR)/opt/web/js/sw.js --output $(DIST_DIR)/opt/web/js/sw.js $(JAVASCRIPT_OBFUSCATOR_OPTS)

	@#
	@# cleanup
	@#
	-rm $(DIST_DIR)/opt/web/js/l.js.tmp

	@#
	@# package
	@#
	mv $(DIST_DIR) lenny-$(VERSION)
	mkdir $(DIST_DIR)
	tar -czf $(DIST_DIR)/lenny-arm32-$(VERSION).tar.gz lenny-$(VERSION)
	zip -qr  $(DIST_DIR)/lenny-arm32-$(VERSION).zip    lenny-$(VERSION)
	rm -r lenny-$(VERSION)

	md5sum $(DIST_DIR)/lenny-arm32-$(VERSION).tar.gz
	md5sum $(DIST_DIR)/lenny-arm32-$(VERSION).zip



## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ## ##

##
## web site
##
DIST_WDIR = ./wdist


define DL_PHP
<?php

$$dlDateRelease = '$(shell date +%Y-%B-%d)';
$$dlVerRelease = '$(VERSION)';
$$dlTzArm32MD5 = '$(shell md5sum $(DIST_DIR)/lenny-arm32-$(VERSION).tar.gz)';
$$dlZipArm32MD5 = '$(shell md5sum $(DIST_DIR)/lenny-arm32-$(VERSION).zip)';

?>
endef
export DL_PHP

.PHONY: dist_web
dist_web:
	-rm -r $(DIST_WDIR)
	mkdir $(DIST_WDIR)

	rsync -a --exclude='.*' --exclude='data' web/* $(DIST_WDIR)

	@#
	@#
	@#
	@echo "$$DL_PHP" >$(DIST_WDIR)/_dl.php
	@#sed -i .tmp -e "s/\'LATEST_VERSION_MAJOR\',0/\'LATEST_VERSION_MAJOR\',$(VERSION_MJR)/" $(DIST_WDIR)/js/index.php
	@#sed -i .tmp -e "s/\'LATEST_VERSION_MINOR\',0/\'LATEST_VERSION_MINOR\',$(VERSION_MNR)/" $(DIST_WDIR)/js/index.php
	sed -i -e "s/\'LATEST_VERSION_MAJOR\',0/\'LATEST_VERSION_MAJOR\',$(VERSION_MJR)/" $(DIST_WDIR)/js/index.php
	sed -i -e "s/\'LATEST_VERSION_MINOR\',0/\'LATEST_VERSION_MINOR\',$(VERSION_MNR)/" $(DIST_WDIR)/js/index.php

	@#
	@#
	@#
	@#sed -i .tmp -e "s/'..\/data'/'..\/..\/lenny_data'/"  $(DIST_WDIR)/js/index.php
	@#sed -i .tmp -e "s/'secure = true;\/\/'/'secure = '/" $(DIST_WDIR)/js/index.php
	sed -i -e "s/'..\/data'/'..\/..\/lenny_data'/"  $(DIST_WDIR)/js/index.php
	sed -i -e "s/'secure = true;\/\/'/'secure = '/" $(DIST_WDIR)/js/index.php
	@#-rm $(DIST_DIR)/js/index.php.tmp

	@#
	@#
	@#
	@#$(JAVASCRIPT_OBFUSCATOR) $(DIST_WDIR)/js/l.js  --output $(DIST_WDIR)/js/l.js  $(JAVASCRIPT_OBFUSCATOR_OPTS)
	@#$(JAVASCRIPT_OBFUSCATOR) $(DIST_WDIR)/js/ld.js --output $(DIST_WDIR)/js/ld.js $(JAVASCRIPT_OBFUSCATOR_OPTS)

	@#
	@# package
	@#
	mv $(DIST_WDIR) lenny-web
	@#mkdir $(DIST_DIR)
	tar -czf $(DIST_DIR)/lenny-web-$(VERSION).tar.gz lenny-web
	@#zip -qr  $(DIST_DIR)/lenny-arm32-$(VERSION).zip    lenny-web
	rm -r lenny-web



