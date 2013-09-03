/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file 	download-manager-network.h
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	Download netowkr manager
 */

#ifndef DOWNLOAD_MANAGER_NETWORK_H
#define DOWNLOAD_MANAGER_NETWORK_H

#include "download-manager-event.h"

class NetMgr {
public:
	static NetMgr& getInstance(void) {
		static NetMgr inst;
		return inst;
	}
	string getProxy(void);
private:
	NetMgr(void);
	~NetMgr(void);
};

#endif
