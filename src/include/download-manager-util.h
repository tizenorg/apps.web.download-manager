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

#define DM_DEFAULT_PHONE_TEMP_DIR "/opt/usr/media/.temp_download"
#define DM_DEFAULT_PHONE_INSTALL_DIR "/opt/usr/media/Downloads/"
#define DM_DEFAULT_MMC_TEMP_DIR "/opt/storage/sdcard/.temp_download"
#define DM_DEFAULT_MMC_INSTALL_DIR "/opt/storage/sdcard/Downloads/"

class FileUtility {
public:
	FileUtility() {}
	~FileUtility() {}

	bool checkTempDir(string userInstallDir);
	bool openFile(string path, int contentType);
	void openMyFilesApp(void);
	bool isExistedFile(string path, bool isDir);
	bool renameFile(string from, string to);
	bool copyFile(string from, string to);
	void cleanTempDir(void);
	static string getDefaultPath(bool optionTempDir);
};

class DownloadUtil
{
public:
	static DownloadUtil& getInstance(void) {
		static DownloadUtil inst;
		return inst;
	}

	int getContentType(const char *mimem, const char *filePath);
	bool registerContent(string filePath, string &thumbnailPath);
	string saveContent(string filePath, string userInstallDir);
	string getUserAgent(void);

private:
	DownloadUtil(void);
	~DownloadUtil(void) {}
	bool isAmbiguousMIMEType(const char *mimeType);
};

#endif//DOWNLOAD_MANAGER_UTIL_H
