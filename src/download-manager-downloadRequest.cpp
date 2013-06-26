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
 * @file	download-provier-downloadRequest.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	download request data class
 */
#include "download-manager-downloadRequest.h"

DownloadRequest::DownloadRequest(string url, string cookie,
		string reqHeaderField, string reqHeaderValue, string installDir)
	: m_url(url)
	, m_cookie(cookie)
	, m_reqHeaderField(reqHeaderField)
	, m_reqHeaderValue(reqHeaderValue)
	, m_installDir(installDir)
{
}

DownloadRequest::DownloadRequest(DownloadRequest &rRequest)
{
	m_url.assign(rRequest.getUrl());
	m_cookie.assign(rRequest.getCookie());
	m_reqHeaderField.assign(rRequest.getReqHeaderField());
	m_reqHeaderValue.assign(rRequest.getReqHeaderValue());
	m_installDir.assign(rRequest.getInstallDir());
	extractSenderName();
}

DownloadRequest::~DownloadRequest()
{
}

string DownloadRequest::getUrl()
{
	return m_url;
}

string DownloadRequest::getCookie()
{
	return m_cookie;
}

string DownloadRequest::getSender()
{
	return m_sender;
}

string DownloadRequest::getReqHeaderField()
{
	return m_reqHeaderField;
}

string DownloadRequest::getReqHeaderValue()
{
	return m_reqHeaderValue;
}

string DownloadRequest::getInstallDir()
{
	return m_installDir;
}

bool DownloadRequest::isUrlEmpty()
{
	return m_url.empty();
}

bool DownloadRequest::isCookieEmpty()
{
	return m_cookie.empty();
}

bool DownloadRequest::isReqHeaderEmpty()
{
	if (m_reqHeaderField.empty() || m_reqHeaderValue.empty())
		return true;
	return false;
}

bool DownloadRequest::isInstallDir()
{
	return m_installDir.empty();
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

void DownloadRequest::setReqHeaderField(string reqHeaderField)
{
	m_reqHeaderField.assign(reqHeaderField);
}

void DownloadRequest::setReqHeaderValue(string reqHeaderValeu)
{
	m_reqHeaderValue.assign(reqHeaderValeu);
}

void DownloadRequest::setInstallDir(string installDir)
{
	m_installDir.assign(installDir);
}

void DownloadRequest::extractSenderName()
{
	size_t found;
	size_t found1;
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
	// For credential URL
	found = m_sender.find("@");
	found1 = m_sender.find(":");
	if (found != string::npos && found1 != string::npos &&
			found1 < found) {
		string tmp = m_sender.substr(found + 1, m_sender.length());
		m_sender.assign(tmp);
	}
}
