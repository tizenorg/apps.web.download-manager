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

/*
 * @file	download-manager-util.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief   Utility APIs and interface with content player
 */

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "aul.h"
#include "xdgmime.h"
#include "app_service.h"
#include "media_content.h"
#include "media_info.h"

#include "download-manager-util.h"

struct MimeTableType
{
	const char *mime;
	int contentType;
};

#define MAX_MIME_TABLE_NUM 12
const char *ambiguousMIMETypeList[] = {
		"text/plain",
		"application/octet-stream"
};

struct MimeTableType MimeTable[]={
		// PDF
		{"application/pdf",DP_CONTENT_PDF},
		// word
		{"application/msword",DP_CONTENT_WORD},
		{"application/vnd.openxmlformats-officedocument.wordprocessingml.document",DP_CONTENT_WORD},
		// ppt
		{"application/vnd.ms-powerpoint",DP_CONTENT_PPT},
		{"application/vnd.openxmlformats-officedocument.presentationml.presentation",DP_CONTENT_PPT},
		// excel
		{"application/vnd.ms-excel",DP_CONTENT_EXCEL},
		{"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",DP_CONTENT_EXCEL},
		// html
		{"text/html",DP_CONTENT_HTML},
		// txt
		{"text/txt",DP_CONTENT_TEXT},
		{"text/plain",DP_CONTENT_TEXT},
		// DRM
		{"application/vnd.oma.drm.content",DP_CONTENT_DRM},
		{"application/vnd.oma.drm.message",DP_CONTENT_DRM},
};

bool FileUtility::isExistedFile(string path, bool isDir)
{
	struct stat fileState;
	int stat_ret;

	DP_LOGV_FUNC();

	if (path.empty())
		return false;
	DP_LOGD("path [%s]", path.c_str());
	stat_ret = stat(path.c_str(), &fileState);
	if (stat_ret != 0)
		return false;

	if (isDir) {
		if (!S_ISDIR(fileState.st_mode)) {
			DP_LOGE("The directory [%s] is not existed", path.c_str());
			return false;
		}
	} else {
		if (!S_ISREG(fileState.st_mode)) {
			DP_LOGE("The file [%s] is not existed", path.c_str());
			return false;
		}
	}
	return true;
}

bool FileUtility::renameFile(string from, string to)
{
	const char *fromStr = from.c_str();
	const char *toStr = to.c_str();
	if (rename(fromStr, toStr) != 0 ) {
		DP_LOGE("rename failed : syserr[%s]", strerror(errno));
		if (errno == EXDEV) {
			DP_LOGE("File system is diffrent. Try to copy a file");
			if (copyFile(from, to)) {
				unlink(fromStr);
				return true;
			} else {
				if(isExistedFile(to, false))
					unlink(toStr);
			}
			return false;
		}
		return false;
	}
	return true;
}

bool FileUtility::copyFile(string from, string to)
{
	FILE *fs = NULL;
	FILE *fd = NULL;
	int readNum = 0;
	int writeNum = 0;
	char buff[4096] = {0};

	fs = fopen(from.c_str(), "rb");
	if (!fs) {
		DP_LOGE("Fail to open src file");
		return false;
	}

	fd = fopen(to.c_str(), "wb");
	if (!fd) {
		DP_LOGE("Fail to open dest file");
		fclose(fs);
		return false;
	}

	while (!feof(fs)) {
		memset(buff, 0x00, 4096);
		readNum = fread(buff, sizeof(char), sizeof(buff), fs);
		if (readNum > 0) {
			writeNum = fwrite(buff, sizeof(char), readNum, fd);
			if (writeNum <= 0) {
				DP_LOGE("written = %d",writeNum);
				break;
			}
		} else {
			DP_LOGV("read = %d", readNum);
			break;
		}
	}
	fclose(fd);
	fclose(fs);
	return true;
}


bool FileUtility::openFile(string path, int contentType)
{
	service_h handle = NULL;
	string filePath;

	DP_LOGD_FUNC();

	if (path.empty())
		return false;

	DP_LOGD("path [%s]", path.c_str());
	if (service_create(&handle) < 0) {
		DP_LOGE("Fail to create service handle");
		return false;
	}

	if (!handle) {
		DP_LOGE("service handle is null");
		return false;
	}

	if (service_set_operation(handle, SERVICE_OPERATION_VIEW) < 0) {
		DP_LOGE("Fail to set service operation");
		service_destroy(handle);
		return false;
	}

	if (contentType == DP_CONTENT_HTML || contentType == DP_CONTENT_TEXT) {
		filePath = "file://";
		filePath.append(path.c_str());
		if (service_set_mime(handle, "text/html") < 0) {
			DP_LOGE("Fail to set mime");
			service_destroy(handle);
			return false;
		}
	} else {
		filePath = path;
	}
	if (service_set_uri(handle, filePath.c_str()) < 0) {
		DP_LOGE("Fail to set uri");
		service_destroy(handle);
		return false;
	}

	if (service_send_launch_request(handle, NULL, NULL) < 0) {
		DP_LOGE("Fail to launch service");
		service_destroy(handle);
		return false;
	}

	service_destroy(handle);

	return true;
}

bool FileUtility::checkTempDir()
{
	string defaultDir = string(DM_DEFAULT_TEMP_DIR);

	DP_LOGD_FUNC();
	if (!isExistedFile(defaultDir, true)) {
		if (mkdir(defaultDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO)) {
			DP_LOGE("Fail to create directory [%s]", defaultDir.c_str());
			return false;
		} else {
			DP_LOGD("[%s] is created!", defaultDir.c_str());
		}
	}
	return true;
}


DownloadUtil::DownloadUtil()
{
}

int DownloadUtil::getContentType(const char *mime, const char *filePath)
{
	int i = 0;
	int type = DP_CONTENT_UNKOWN;
	int ret = 0;
	char tempMime[MAX_FILE_PATH_LEN] = {0,};
	DP_LOGV_FUNC();
	if (mime == NULL || strlen(mime) < 1)
		return DP_CONTENT_UNKOWN;

	DP_LOGD("mime[%s]",mime);
	strncpy(tempMime, mime, MAX_FILE_PATH_LEN-1);
	if (isAmbiguousMIMEType(mime)) {
		if (filePath) {
			memset(tempMime, 0x00, MAX_FILE_PATH_LEN);
			ret = aul_get_mime_from_file(filePath,tempMime,sizeof(tempMime));
			if (ret < AUL_R_OK )
				strncpy(tempMime, mime, MAX_FILE_PATH_LEN-1);
			else
				DP_LOGD("mime from extension name[%s]",tempMime);
		}
	}

	/* Search a content type from mime table. */
	for (i = 0; i < MAX_MIME_TABLE_NUM; i++) {
		//DP_LOGD("TableMime[%d][%s]",i,MimeTable[i].mime);
		if (strncmp(MimeTable[i].mime, tempMime, strlen(tempMime)) == 0){
			type = MimeTable[i].contentType;
			break;
		}
	}
	/* If there is no mime at mime table, check the category with the first
	 *   domain of mime string
	 * ex) video/... => video type */
	if (type == DP_CONTENT_UNKOWN) {
		const char *unaliasedMime = NULL;
		/* unaliased_mimetype means representative mime among similar types */
		unaliasedMime = xdg_mime_unalias_mime_type(tempMime);

		if (unaliasedMime != NULL) {
			DP_LOGD("unaliased mime type[%s]\n",unaliasedMime);
			if (strstr(unaliasedMime,"video/") != NULL)
				type = DP_CONTENT_VIDEO;
			else if (strstr(unaliasedMime,"audio/") != NULL)
				type = DP_CONTENT_MUSIC;
			else if (strstr(unaliasedMime,"image/") != NULL)
				type = DP_CONTENT_IMAGE;
		}
	}
	DP_LOGD("type[%d]\n",type);
	return type;
}

bool DownloadUtil::isAmbiguousMIMEType(const char *mimeType)
{

	if (!mimeType)
		return false;

	int index = 0;
	int listSize = sizeof(ambiguousMIMETypeList) / sizeof(const char *);
	for (index = 0; index < listSize; index++) {
		if (0 == strncmp(mimeType, ambiguousMIMETypeList[index],
				strlen(ambiguousMIMETypeList[index]))) {
			DP_LOGD("It is ambiguous! [%s]", ambiguousMIMETypeList[index]);
			return true;
		}
	}

	return false;
}

string DownloadUtil::saveContent(string filePath)
{
	string finalPath = string();
	string dirPath = string(DM_DEFAULT_INSTALL_DIR);
	size_t found1 = string::npos;
	size_t found2= string::npos;
	size_t len = string::npos;
	string str1 = ".temp_download";
	string str2 = "Downloads/";
	string contentName;
	string extensionName;
	string countStr;
	int count = 0;
	char tempStr[10] = {0};
	FileUtility fileObj;
	found1 = filePath.rfind("/");
	found2 = filePath.rfind(".");
	if (found1 == string::npos)
		return finalPath;
	if (found2 == string::npos) {
		extensionName = string();
	} else {
		len = filePath.length() - found2;
		extensionName = filePath.substr(found2, len);
	}
	len = filePath.length() - found1 - 1; // -1 means"/"
	if (!extensionName.empty())
		len = len - extensionName.length();// -1 means "."
	contentName = filePath.substr(found1 + 1, len);

	if (!extensionName.empty())
		finalPath = dirPath + contentName + extensionName;
	else
		finalPath = dirPath + contentName;

	while(fileObj.isExistedFile(finalPath, false)) {
		count++;
		snprintf(tempStr, 10, "_%d", count);
		countStr.assign(tempStr);
		finalPath.clear();
		if (!extensionName.empty())
			finalPath = dirPath + contentName + countStr + extensionName;
		else
			finalPath = dirPath + contentName + countStr;

		if (count > 1000) {
			finalPath.clear();
			break;
		}
		memset(tempStr, 0, 10);
	}

	DP_LOGD("finalPath[%s]", finalPath.c_str());
	if (!fileObj.renameFile(filePath, finalPath))
		finalPath.clear();

	return finalPath;
}

bool DownloadUtil::registerContent(string filePath, string &thumbnailPath)
{
	int ret = -1;
	media_info_h info = NULL;
	char *tempPath = NULL;

	if (filePath.empty()) {
		DP_LOGE("file path is NULL");
		return false;
	}

	thumbnailPath = string();
	DP_LOGD("Register file [%s]", filePath.c_str());

	ret = media_content_connect();
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		DP_LOGE("Fail to connect media db");
		return false;
	}

	ret = media_info_insert_to_db(filePath.c_str(), &info);
	if (ret != MEDIA_CONTENT_ERROR_NONE || info == NULL) {
		DP_LOGE("Fail to insert media db [%d]", ret);
		media_content_disconnect();
		if (info)
			media_info_destroy(info);
		return false;
	}
	ret = media_info_get_thumbnail_path(info, &tempPath);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		DP_LOGE("Fail to insert media db [%d]", ret);
		media_info_destroy(info);
		media_content_disconnect();
		return false;
	}
	if (tempPath) {
		thumbnailPath.assign(tempPath);
		free(tempPath);
	}
	media_info_destroy(info);
	ret = media_content_disconnect();
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		DP_LOGE("Fail to connect media db");
	}
	return true;
}

