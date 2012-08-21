/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.tizenopensource.org/license
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

#include "net_connection.h"
#include "download-manager-event.h"

class NetMgr {
public:
	static NetMgr& getInstance(void) {
		static NetMgr inst;
		return inst;
	}
	void initNetwork(void);
	void deinitNetwork(void);
	inline void subscribe(Observer *o) { m_subject.attach(o); }
	inline void deSubscribe(Observer *o) { m_subject.detach(o); }
	static void netTypeChangedCB(connection_type_e type, void *data);
	static void netConfigChangedCB(const char *ip,
		const char *ipv6, void *data);
private:
	NetMgr(void);
	~NetMgr(void);
	void netTypeChanged(void);
	void netConfigChanged(string ip);
	int getConnectionState(void);
	int getCellularStatus(void);
	int getWifiStatus(void);
	void getProxy(void);
	void getIPAddress(void);
	inline void notify(void) { m_subject.notify(); }
	int m_netStatus;
	Subject m_subject;
	connection_h m_handle;
};

#endif
