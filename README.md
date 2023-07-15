# Lenny - Your Telemarketer Troll 

<p align="center">
 <i>Lenny Troll</i> is a Smart Answering Machine for Raspberry Pi and Plain Old Telephone Systems.
</p>

<p align="center">
	<img alt="Lenny Troll" src="web/img/lenny_512.png" width="400"/>
	<br/>
    <img alt="Platform" src="https://img.shields.io/badge/platform-linux-blue.svg"/>
    <img alt="Language" src="https://img.shields.io/badge/language-cpp-red.svg"/>
    <img alt="Version" src="https://img.shields.io/badge/version-v1.00-brightgreen.svg"/>
</p>

<p align="center">
  <a href="#introduction">Introduction</a> •
  <a href="#build-for-raspberry-pi">Build for Raspberry Pi</a> •
  <a href="#install-binary-package">Install</a> •
  <a href="#license">License</a>
</p>


## Introduction

**Lenny Troll** is a full-featured multi-threaded Linux Daemon with an embedded Web Server providing a Browser based Web Application.

**Lenny Troll** is written in C++ using standard USB based Voice Modems that answers your telephone with a Script based responses keeping Telemarketers busy wasting their time while entertaining us.


## Build for Raspberry Pi
Raspberry Pi is an awesome Linux product with a USB Voice Modem make the foundation for running **Lenny Troll** as an appliance.

### Install development libraries
Building **Lenny Troll** requires two development libraries to be installed:
  * [LibUDev](https://github.com/systemd/systemd/tree/main/src/libudev)
  * [LibUSB](https://github.com/libusb/libusb)

```bash

  # sudo apt install libudev-dev libusb-dev 

```

### Download and Build OpenSSL development library
**Lenny Troll** is built providing a Web based User-Interface with the ability to record new voice messages requiring Microphone access with a Secure HTTPS session.

**Lenny Troll** uses the [OpenSSL](https://www.openssl.org) library to create and manage the unsigned certificate while provide TLS1.3 session for your Web Browser.

**Lenny Troll** web site will require bypassing the security exception to accept the unsigned certificate.

Building **Lenny Troll** requires a one-time download and build of the OpenSSL development library.

```bash

  # make lib_openssl

```

### Build Debug & Release binary

#### Build the **Lenny Troll** binary for debugging
Build **Lenny Troll**.

```bash

  # make -build=DEBUG

```

#### Build the **Lenny Troll** binary distributions
Build **Lenny Troll**.

```bash

  # make -build=release

```

#### Build **Lenny Troll** binary packages
Create **Lenny Troll** binary distribution tar and zip files.

```bash

  # make dist

```

#### Build LennyTroll.com web package

**Lenny Troll** is a Progressive Web App with lots of Browser based Javacript UI code that uses an obfuscation pass as part of the packaging layer.
  * [Javascript-Obfuscator](https://github.com/javascript-obfuscator/javascript-obfuscator)

```bash

  # npm install javascript-obfuscator

```

Create **Lenny Troll** web site distribution tar file.

```bash

  # make dist_web

```

### Install Binary Package

Follow the step-by-step [**Lenny Troll** Getting Started](https://lennytroll.com/start.php) Guide to run the newly built Lenny Troll daemon.


## License

**Lenny Troll** is an Open Source project using the Apache 2 license.

You may use the code according to the terms of the license (see [LICENSE](LICENSE)).

