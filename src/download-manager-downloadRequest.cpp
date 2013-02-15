/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
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
 * @file	download-provier-downloadRequest.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	download request data class
 */
#include "download-manager-downloadRequest.h"
/*DownloadRequest::DownloadRequest()
	: m_url(NULL)
	, m_cookie(NULL)
{
}*/

DownloadRequest::DownloadRequest(string url, string cookie)
	: m_url(url)
	, m_cookie(cookie)
{
}

DownloadRequest::DownloadRequest(DownloadRequest &rRequest)
{
	m_url.assign(rRequest.getUrl());
	m_cookie.assign(rRequest.getCookie());
	extractSenderName();
}

DownloadRequest::~DownloadRequest()
{
//	DP_LOG_FUNC();
}

string &DownloadRequest::getUrl()
{
	return m_url;
}

string &DownloadRequest::getCookie()
{
	return m_cookie;
}

string DownloadRequest::getSender()
{
	return m_sender;
}


bool DownloadRequest::isUrlEmpty()
{
	return m_url.empty();
}

bool DownloadRequest::isCookieEmpty()
{
	return m_cookie.empty();
}

void DownloadRequest::setUrl(string url)
{
	m_url.assign(url);
	extractSenderName();
}

void DownloadRequest::setCookie(string cookie)
{
	m_cookie.assign(cookie);
}

void DownloadRequest::extractSenderName()
{
	size_t found;
	string temp;
	if (m_url.empty()) {
		m_sender = string();
		return;
	}
	found = m_url.find("://");
	if (found != string::npos) {
		temp = m_url.substr(found + 3);
	} else {
		temp = m_url;
	}
	found = temp.find("/");
	if (found != string::npos) {
		m_sender = temp.substr(0, found);
	} else {
		m_sender = temp;
	}
}
