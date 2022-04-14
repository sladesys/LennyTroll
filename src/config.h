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

class ServerVersion {
public:	
	unsigned mjr,mnr,lic;
	ServerVersion();
	ServerVersion(const char *str);
	std::string toString() const;
	void parse(const char *str);
	bool hasUpdate() const;
	bool requiresUpdate() const;
};


class ConfigLine {
public:
	unsigned line;
	char name[PATH_MAX],number[PATH_MAX],device[PATH_MAX];

	bool lennyEnabled,disconnectedEnabled,tadEnabled;
	std::string lennyProfile;
	unsigned tadRings;

	ConfigLine();
	std::string toString();
};

#include <vector>

class Config {
public:
	std::string srcFile;

	ServerVersion *sv;
	time_t svCheckLast;

	std::string license,email;
	std::vector<std::string> profiles;

	unsigned httpPort;
	bool userSecurity,skiplistEnabled;

	unsigned silenceVolumeThreshold;
	unsigned silenceSwitchTime;
	unsigned endCallSilenceTime,endCallBusyCount,endCallBusyTime;

	ConfigLine line1,line2;

	void load(const char *file);
	void save();
	std::string toString();

	Config();
	void loadServerVersion(const char *str);
};

