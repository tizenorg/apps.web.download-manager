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
 * @file 	download-manager-network.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	Download netowkr manager
 */

#include <stdlib.h>
#include "download-manager-common.h"
#include "download-manager-network.h"

enum {
	NET_INACTIVE = 0,
	NET_WIFI_ACTIVE,
	NET_CELLULAR_ACTIVE
};

NetMgr::NetMgr()
	:m_handle(NULL)
{
}

NetMgr::~NetMgr()
{
}

void NetMgr::initNetwork()
{
	if (connection_create(&m_handle) < 0) {
		DP_LOGE("Fail to create network handle");
		return;
	}

	if (connection_set_type_changed_cb(m_handle, netTypeChangedCB, NULL)
			< 0) {
		DP_LOGE("Fail to register network state changed cb");
		return;
	}
	if (connection_set_ip_address_changed_cb(m_handle, netConfigChangedCB, NULL)
			< 0) {
		DP_LOGE("Fail to register ip address changed cb");
		return;
	}
}

void NetMgr::deinitNetwork()
{
	if (connection_unset_type_changed_cb(m_handle) < 0) {
		DP_LOGE("Fail to unregister network state changed cb");
	}
	if (connection_unset_ip_address_changed_cb(m_handle) < 0) {
		DP_LOGE("Fail to unregister ip address changed cb");
	}
	if (connection_destroy(m_handle) < 0) {
		DP_LOGE("Fail to destroy network handle");
	}
}

int NetMgr::getConnectionState()
{
	connection_type_e type = CONNECTION_TYPE_DISCONNECTED;
	int ret = 0;

	if (!m_handle) {
		DP_LOGE("handle is NULL");
		return NET_INACTIVE;
	}
	if (connection_get_type(m_handle, &type) < 0) {
		DP_LOGE(" Fail to get network status");
		return NET_INACTIVE;
	}

	switch (type) {
	case CONNECTION_TYPE_DISCONNECTED:
		DP_LOG("CONNECTION_NETWORK_STATE_DISCONNECTED");
		ret = NET_INACTIVE;
		break;
	case CONNECTION_TYPE_WIFI:
		DP_LOG("CONNECTION_NETWORK_STATE_WIFI");
		ret = getCellularStatus();
		break;
	case CONNECTION_TYPE_CELLULAR:
		DP_LOG("CONNECTION_NETWORK_STATE_CELLULAR");
		ret = getWifiStatus();
		break;
	default:
		DP_LOGE("Cannot enter here");
		ret = NET_INACTIVE;
		break;
	}
	return ret;
}

int NetMgr::getCellularStatus()
{
	connection_cellular_state_e status = CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE;
	int ret = 0;

	if (!m_handle) {
		DP_LOGE("handle is NULL");
		return NET_INACTIVE;
	}

	if (connection_get_cellular_state(m_handle, &status) < 0) {
		DP_LOGE(" Fail to get cellular status");
		return NET_INACTIVE;
	}

	switch(status) {
	case CONNECTION_CELLULAR_STATE_AVAILABLE:
		DP_LOG("CONNECTION_CELLULAR_STATE_AVAILABLE");
		ret = NET_CELLULAR_ACTIVE;
		break;
	case CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE:
		DP_LOG("CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE");
		ret = NET_INACTIVE;
		break;
	case CONNECTION_CELLULAR_STATE_FLIGHT_MODE:
		DP_LOG("CONNECTION_CELLULAR_STATE_FLIGHT_MODE");
		ret = NET_INACTIVE;
		break;
	case CONNECTION_CELLULAR_STATE_ROAMING_OFF:
		DP_LOG("CONNECTION_CELLULAR_STATE_ROAMING_OFF");
		ret = NET_INACTIVE;
		break;
	case CONNECTION_CELLULAR_STATE_CALL_ONLY_AVAILABLE:
		DP_LOG("CONNECTION_CELLULAR_STATE_CALL_ONLY_AVAILABLE");
		ret = NET_INACTIVE;
		break;
	default:
		DP_LOGE("Cannot enter here");
		ret = NET_INACTIVE;
		break;
	}
	return ret;

}

int NetMgr::getWifiStatus()
{
	connection_wifi_state_e status = CONNECTION_WIFI_STATE_DEACTIVATED;
	int ret = 0;

	if (!m_handle) {
		DP_LOGE("handle is NULL");
		return NET_INACTIVE;
	}

	if (connection_get_wifi_state(m_handle, &status) < 0) {
		DP_LOGE(" Fail to get wifi status");
		return NET_INACTIVE;
	}

	switch(status) {
	case CONNECTION_WIFI_STATE_CONNECTED:
		DP_LOG("CONNECTION_WIFI_STATE_CONNECTED");
		ret = NET_WIFI_ACTIVE;
		break;
	case CONNECTION_WIFI_STATE_DISCONNECTED:
		DP_LOG("CONNECTION_WIFI_STATE_DISCONNECTED");
		ret = NET_INACTIVE;
		break;
	case CONNECTION_WIFI_STATE_DEACTIVATED:
		DP_LOG("CONNECTION_WIFI_STATE_DEACTIVATED");
		ret = NET_INACTIVE;
		break;
	default:
		DP_LOGE("Cannot enter here");
		ret = NET_INACTIVE;
		break;
	}
	return ret;
}

void NetMgr::netTypeChanged()
{
	int changedStatus = NET_INACTIVE;
	changedStatus = getConnectionState();
	DP_LOG("Previous[%d] Changed[%d]", m_netStatus, changedStatus);
	if (m_netStatus != changedStatus) {
		if (changedStatus == NET_INACTIVE)
			DP_LOG("Netowrk is disconnected");
		else
			DP_LOG("Network is connected");
		m_netStatus = changedStatus;
	} else {
		DP_LOG("Network berer type is changed. ex.3G->WIFI");
	}
}

/* This routine should be operated in case of downloading state.
 * After the download is finished, network event handler should be removed.
 */
void NetMgr::netConfigChanged(string ipAddr)
{

	DP_LOG_FUNC();

	if (ipAddr.length() > 1) {/* network is connected */
		getProxy();
		getIPAddress();
		/* This notify is only for suspend event.
		 * If othere network event is added, it is needed to save event types
		 * and get function for it
		**/
		notify();
	} else {
		DP_LOGE("Network connection is disconnected");
	}
}

void NetMgr::getProxy()
{
	char *proxy = NULL;
	connection_address_family_e family = CONNECTION_ADDRESS_FAMILY_IPV4;

	if (!m_handle) {
		DP_LOGE("handle is NULL");
		return;
	}
	if (connection_get_proxy(m_handle, family, &proxy) < 0) {
		DP_LOGE("Fail to get ip address");
		return;
	}
	if (proxy) {
		DP_LOG("===== Proxy address[%s] =====", proxy);
		free(proxy);
		proxy = NULL;
	}
}

void NetMgr::getIPAddress()
{
	char *ipAddr = NULL;
	connection_address_family_e family = CONNECTION_ADDRESS_FAMILY_IPV4;
	if (!m_handle) {
		DP_LOGE("handle is NULL");
		return;
	}
	if (connection_get_ip_address(m_handle, family, &ipAddr) < 0) {
		DP_LOGE("Fail to get ip address");
		return;
	}
	if (ipAddr) {
		DP_LOG("===== IP address[%s] =====", ipAddr);
		free(ipAddr);
		ipAddr= NULL;
	}
}

void NetMgr::netTypeChangedCB(connection_type_e state, void *data)
{
	NetMgr inst = NetMgr::getInstance();
	inst.netTypeChanged();
}

void NetMgr::netConfigChangedCB(const char *ip, const char *ipv6,
		void *data)
{
	string ipAddr = ip;
	NetMgr inst = NetMgr::getInstance();
	inst.netConfigChanged(ipAddr);
}

