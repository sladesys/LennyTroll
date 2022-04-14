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
#include "butils.h"

extern void appConfigLoad();
extern bool appStart();
extern void appStop();
extern void appUpdate();
extern std::string appInfo();
extern void appWebAppStart();

extern void appCheckLicense();
extern int appGetLicense(const char *license,time_t &dateMs,LicenseFlags &flags);
extern bool appCreateLicense(unsigned days,bool evalFlag);

bool appGetServerVersion();

