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

/*
 * @file	download-manager-util.h
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief   Utility APIs and interface with content player
 */

#ifndef DOWNLOAD_MANAGER_UTIL_H
#define DOWNLOAD_MANAGER_UTIL_H

#include <string>
#include "download-manager-common.h"

using namespace std;
class FileOpener {
public:
	FileOpener() {}
	~FileOpener() {}

	bool openFile(string &path, int contentType);
};

class DownloadUtil
{
public:
	static DownloadUtil& getInstance(void) {
		static DownloadUtil inst;
		return inst;
	}

	int getContentType(const char *mimem, const char *filePath);
	void registerContent(string filePath);

private:
	DownloadUtil(void);
	~DownloadUtil(void) {}
	bool isAmbiguousMIMEType(const char *mimeType);
};

#endif//DOWNLOAD_MANAGER_UTIL_H
