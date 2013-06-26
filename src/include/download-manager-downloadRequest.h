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
 * @file 	download-manager-downloadRequest.h
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	Download Request class
 */

#ifndef DOWNLOAD_MANAGER_DOWNLOAD_REQUEST_H
#define DOWNLOAD_MANAGER_DOWNLOAD_REQUEST_H

#include <string>

using namespace std;

class DownloadRequest
{
public:
	DownloadRequest(string url, string cookie,
			string reqHeaderField, string reqHeaderValue, string installDir);
	DownloadRequest(DownloadRequest &rRequest);
	~DownloadRequest(void);

	string getUrl(void);
	string getCookie(void);
	string getReqHeaderField(void);
	string getReqHeaderValue(void);
	string getInstallDir(void);
	string getSender(void);
	bool isUrlEmpty(void);
	bool isCookieEmpty(void);
	bool isReqHeaderEmpty(void);
	bool isInstallDir(void);
	void setUrl(string url);
	void setCookie(string cookie);
	void setReqHeaderField(string reqHeaderField);
	void setReqHeaderValue(string reqHeaderValue);
	void setInstallDir(string installDir);
private:
	void extractSenderName(void);
	string m_url;
	string m_cookie;
	string m_sender;
	string m_reqHeaderField;
	string m_reqHeaderValue;
	string m_installDir;
};

#endif /* DOWNLOAD_MANAGER_DOWNLOAD_REQUEST_H */
