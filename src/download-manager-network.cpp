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
 * @file 	download-manager-network.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	Download netowkr manager
 */

#include <stdlib.h>
#include "download-manager-common.h"
#include "download-manager-network.h"

#include "net_connection.h"

NetMgr::NetMgr()
{
}

NetMgr::~NetMgr()
{
}

string NetMgr::getProxy()
{
	connection_h handle;
	char *proxy = NULL;
	string proxyStr = string();
	connection_address_family_e family = CONNECTION_ADDRESS_FAMILY_IPV4;

	if (connection_create(&handle) < 0) {
		DM_LOGE("Fail to create network handle");
		return proxyStr;
	}
	if (connection_get_proxy(handle, family, &proxy) < 0) {
		DM_LOGE("Fail to get ip address");
		if (connection_destroy(handle) < 0) {
			DM_LOGE("Fail to destroy network handle");
		}
		return proxyStr;
	}
	if (proxy) {
		DM_SLOGI("Proxy address[%s]", proxy);
		proxyStr.assign(proxy);
		free(proxy);
		proxy = NULL;
	}
	if (connection_destroy(handle) < 0) {
		DM_LOGE("Fail to destroy network handle");
	}
	return proxyStr;
}


