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
#include <sys/types.h>
#include <dirent.h>
#include <storage.h>
#include "xdgmime.h"
#include "app_control.h"
#include "media_content.h"
#include "media_info.h"
#ifdef _ENABLE_WAITING_RO
#include "drm_client.h"
#endif
#include "vconf.h"
#include "mime_type.h"

#include "download-manager-util.h"

struct MimeTableType
{
	const char *mime;
	int contentType;
};

#define MAX_SUFFIX_COUNT		1000000000
#define MAX_MIME_TABLE_NUM 	15
const char *ambiguousMIMETypeList[] = {
		"text/plain",
		"application/octet-stream"
};

struct MimeTableType MimeTable[]={
		// PDF
		{"application/pdf",DM_CONTENT_PDF},
		// word
		{"application/msword",DM_CONTENT_WORD},
		{"application/vnd.openxmlformats-officedocument.wordprocessingml.document",DM_CONTENT_WORD},
		// ppt
		{"application/vnd.ms-powerpoint",DM_CONTENT_PPT},
		{"application/vnd.openxmlformats-officedocument.presentationml.presentation",DM_CONTENT_PPT},
		// excel
		{"application/vnd.ms-excel",DM_CONTENT_EXCEL},
		{"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",DM_CONTENT_EXCEL},
		// html
		{"text/html",DM_CONTENT_HTML},
		// txt
		{"text/txt",DM_CONTENT_TEXT},
		{"text/plain",DM_CONTENT_TEXT},
		// DRM
		{"application/vnd.oma.drm.content",DM_CONTENT_SD_DRM},
		{"application/vnd.oma.drm.message",DM_CONTENT_DRM},
		{"application/x-shockwave-flash", DM_CONTENT_FLASH},
		{"application/vnd.tizen.package", DM_CONTENT_TPK},
		{"text/calendar",DM_CONTENT_VCAL},
};

void FileUtility::cleanTempDir()
{
    struct dirent entry;
    struct dirent *dirInfo = NULL;

	DIR *dir;
	string filePath = string();
	string defTempPath = FileUtility::getDefaultPath(true);

	if(isExistedFile(defTempPath, true)) {
		dir = opendir(defTempPath.c_str());
		if(dir == NULL) {
			DM_LOGE("Fail to call opendir");
			return;
		} else {
		    while(readdir_r(dir, &entry, &dirInfo) == 0 && dirInfo != NULL) {
				DM_SLOGD("%s",dirInfo->d_name);
				if (0 == strncmp(dirInfo->d_name,".",strlen(".")) ||
						0 == strncmp(dirInfo->d_name,"..",strlen("..")))
					continue;
				filePath.append(defTempPath);
				filePath.append("/");
				filePath.append(dirInfo->d_name);
				/* The sub-directory should not be created under temporary directory */
				if (isExistedFile(filePath, false)) {
					if (remove(filePath.c_str())<0)
						DM_LOGE("Fail to Remove");
				} else {
					DM_LOGE("Cannot enter here type[%d]", dirInfo->d_type);
				}
				filePath.clear();
			}
			closedir(dir);
		}
	}
}

bool FileUtility::isExistedFile(string path, bool isDir)
{
	struct stat fileState;
	int stat_ret;

//	DM_LOGD("");

	if (path.empty())
		return false;
	stat_ret = stat(path.c_str(), &fileState);
	if (stat_ret != 0)
		return false;

	if (isDir) {
		if (!S_ISDIR(fileState.st_mode)) {
			DM_LOGE("The directory is not existed");
			return false;
		}
	} else {
		if (!S_ISREG(fileState.st_mode)) {
			DM_LOGE("The file is not existed");
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
		DM_LOGE("rename failed:err");
		if (errno == EXDEV) {
			DM_LOGE("File system is diffrent. Try to copy a file");
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

bool FileUtility::deleteFile(string filePath)
{
	if (filePath.empty())
		return false;
	if (isExistedFile(filePath, false)) {
		if (unlink(filePath.c_str()) < 0) {
			DM_LOGE("Fail to Remove file err");
			return false;
		}
	} else {
		DM_LOGE("File does not exist");
		return false;
	}
	return true;
}

bool FileUtility::checkAvailableMemory(unsigned long long size)
{
	struct statvfs file_info;
	unsigned long long avail;
	int r = storage_get_internal_memory_size(&file_info);
	if (r < 0) {
		DM_LOGE("storage_get_internal_memory_size API failed");
		return false;
	} else {
		avail = (unsigned long long)file_info.f_bsize * file_info.f_bavail;
		DM_SLOGD("avail: %llu, size: %llu", avail, size);
		if (size < avail)
			return true;
		else
			return false;
	}
}

bool FileUtility::copyFile(string from, string to)
{
	FILE *fs = NULL;
	FILE *fd = NULL;
	int readNum = 0;
	int writeNum = 0;
	char buff[4096] = {0};
	char *validate_path = NULL;

	validate_path = strpbrk((char *)from.c_str(), DM_INVALID_PATH_STRING);
	if (validate_path != NULL) {
		DM_LOGE("Invalid source Path");
		return false;
	}

	validate_path = strpbrk((char *)to.c_str(), DM_INVALID_PATH_STRING);
	if (validate_path != NULL){
		DM_LOGE("Invalid destination Path");
		return false;
	}

	fs = fopen(from.c_str(), "rb");
	if (!fs) {
		DM_LOGE("Fail to open src file");
		return false;
	}

	fd = fopen(to.c_str(), "wb");
	if (!fd) {
		DM_LOGE("Fail to open dest file");
		fclose(fs);
		return false;
	}

	while (!feof(fs)) {
		memset(buff, 0x00, 4096);
		readNum = fread(buff, sizeof(char), sizeof(buff), fs);
		if (readNum > 0) {
			writeNum = fwrite(buff, sizeof(char), readNum, fd);
			if (writeNum <= 0) {
				DM_LOGE("written:err");
				break;
			}
		} else {
			DM_LOGV("read[%d]", readNum);
			break;
		}
	}
	fclose(fd);
	fclose(fs);
	return true;
}


bool FileUtility::openFile(string path, int contentType)
{
	app_control_h handle = NULL;
	string filePath;

	if (path.empty())
		return false;

	DM_SLOGD("path [%s]", path.c_str());
	if (app_control_create(&handle) < 0) {
		DM_LOGE("Fail to create app_control handle");
		return false;
	}

	if (!handle) {
		DM_LOGE("NULL Check: app_control handle");
		return false;
	}

	if (app_control_set_operation(handle, APP_CONTROL_OPERATION_VIEW) < 0) {
		DM_LOGE("Fail to set app_control operation");
		app_control_destroy(handle);
		return false;
	}

	if (contentType == DM_CONTENT_HTML || contentType == DM_CONTENT_TEXT) {
		filePath = "file://";
		filePath.append(path.c_str());
		if (app_control_set_mime(handle, "text/html") < 0) {
			DM_LOGE("Fail to set mime");
			app_control_destroy(handle);
			return false;
		}
	} else {
		filePath = path;
	}
	if (app_control_set_uri(handle, filePath.c_str()) < 0) {
		DM_LOGE("Fail to set uri");
		app_control_destroy(handle);
		return false;
	}

	if (app_control_send_launch_request(handle, NULL, NULL) < 0) {
		DM_LOGE("Fail to launch app_control");
		app_control_destroy(handle);
		return false;
	}

	app_control_destroy(handle);

	return true;
}

void FileUtility::openMyFilesApp()
{
	app_control_h handle = NULL;
	DM_LOGD("");

	if (app_control_create(&handle) < 0) {
		DM_LOGE("Fail to create app_control handle");
		return;
	}

	if (!handle) {
		DM_LOGE("NULL Check: app_control handle");
		return;
	}

	if (app_control_set_app_id(handle, MYFILE_PKG_NAME) < 0) {
		DM_LOGE("Fail to set app_control operation");
		app_control_destroy(handle);
		return;
	}

	if (app_control_send_launch_request(handle, NULL, NULL) < 0) {
		DM_LOGE("Fail to launch app_control");
		app_control_destroy(handle);
		return;
	}
	app_control_destroy(handle);
	return;
}

bool FileUtility::checkTempDir(string userInstallDir)
{
	string defaultDir;
	DM_LOGV("");
	if (userInstallDir.empty()) {
		defaultDir = FileUtility::getDefaultPath(true);
	} else {
		defaultDir = userInstallDir;
		defaultDir.append(DM_TEMP_DIR_NAME);
	}

	DM_SLOGD("temp dir:[%s]", defaultDir.c_str());
	if (!isExistedFile(defaultDir, true)) {
		if (mkdir(defaultDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO)) {
			DM_LOGE("Fail to create directory");
			return false;
		} else {
			DM_SLOGI("[%s] is created!", defaultDir.c_str());
		}
	}
	return true;
}

string FileUtility::getDefaultPath(bool optionTempDir)
{
	string path;

	if (optionTempDir)
		path.assign(DM_DEFAULT_PHONE_TEMP_DIR);
	else
		path.assign(DM_DEFAULT_PHONE_INSTALL_DIR);

   return path;
}

unsigned long long FileUtility::getFileSize(string filePath)
{
	struct stat st;
	int ret = 0;
	if (filePath.empty())
		return 0;

	ret = stat(filePath.c_str(), &st);
	if(ret != 0) {
		DM_LOGE("stat error:%d", errno);
		return 0;
	}
	return (unsigned long long)st.st_size;
}

DownloadUtil::DownloadUtil()
{
}

int DownloadUtil::getContentType(const char *mime, const char *filePath)
{
	int i = 0;
	int type = DM_CONTENT_UNKOWN;
	char *tempMime = NULL;
	DM_LOGV("");
	if (mime == NULL || strlen(mime) < 1)
		return DM_CONTENT_UNKOWN;

	if ((filePath != NULL) && (strlen(filePath) > 0) &&
			isAmbiguousMIMEType(mime)) {
		const char *ext = strrchr(filePath, '.');
		if (ext == NULL) {
			DM_LOGE("File Extension is NULL");
			return type;
		}
		mime_type_get_mime_type(ext + 1, &tempMime);
	}
	if (tempMime == NULL) {
		tempMime = (char *)calloc(1, MAX_FILE_PATH_LEN);
		if (tempMime == NULL) {
			DM_LOGE("Fail to call calloc");
			return type;
		}
		strncpy(tempMime, mime, MAX_FILE_PATH_LEN - 1);
	}
	DM_SLOGD("mime type [%s]", tempMime);

	/* Search a content type from mime table. */
	for (i = 0; i < MAX_MIME_TABLE_NUM; i++) {
		if (strncmp(MimeTable[i].mime, tempMime, strlen(tempMime)) == 0){
			type = MimeTable[i].contentType;
			break;
		}
	}
	/* If there is no mime at mime table, check the category with the first
	 *   domain of mime string
	 * ex) video/... => video type */
	if (type == DM_CONTENT_UNKOWN) {
		const char *unaliasedMime = NULL;
		/* unaliased_mimetype means representative mime among similar types */
		unaliasedMime = xdg_mime_unalias_mime_type(tempMime);

		if (unaliasedMime != NULL) {
			DM_SLOGD("unaliased mime type[%s]",unaliasedMime);
			if (strstr(unaliasedMime,"video/") != NULL)
				type = DM_CONTENT_VIDEO;
			else if (strstr(unaliasedMime,"audio/") != NULL)
				type = DM_CONTENT_MUSIC;
			else if (strstr(unaliasedMime,"image/") != NULL)
				type = DM_CONTENT_IMAGE;
		}
	}
	free(tempMime);
	DM_LOGV("type[%d]",type);
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
			DM_LOGV("It is ambiguous");
			return true;
		}
	}

	return false;
}

string DownloadUtil::saveContent(string filePath, string userInstallDir)
{
	string finalPath = string();
	string dirPath;
	size_t found1 = string::npos;
	size_t found2= string::npos;
	size_t len = string::npos;
	string str1 = ".temp_download";
	string contentName;
	string extensionName;
	string countStr;
	int count = 0;
	char tempStr[10] = {0};
	FileUtility fileObj;

	if (userInstallDir.empty()) {
		dirPath = FileUtility::getDefaultPath(false);
	} else {
		dirPath = userInstallDir;
	}

	found1 = filePath.rfind("/");
	if (found1 == string::npos)
		return finalPath;
	string tempContentName = filePath.substr(found1, filePath.length());
	found2 = tempContentName.rfind(".");
	if (found2 == string::npos) {
		extensionName = string();
	} else {
		len = tempContentName.length() - found2;
		extensionName = tempContentName.substr(found2, len);
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

		if (count > MAX_SUFFIX_COUNT) {
			finalPath.clear();
			break;
		}
		memset(tempStr, 0, 10);
	}

	DM_SLOGI("finalPath[%s]", finalPath.c_str());
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
		DM_LOGE("file path is NULL");
		return false;
	}

	thumbnailPath = string();
	DM_SLOGD("Register file [%s]", filePath.c_str());

	ret = media_content_connect();
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		DM_LOGE("Fail to connect media db");
		return false;
	}
	ret = media_info_insert_to_db(filePath.c_str(), &info);
	if (ret != MEDIA_CONTENT_ERROR_NONE || info == NULL) {
		DM_LOGE("Fail to insert media db [%d]", ret);
		media_content_disconnect();
		if (info)
			media_info_destroy(info);
		return false;
	}
	DM_LOGD("insert DB Done");
	ret = media_info_get_thumbnail_path(info, &tempPath);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		DM_LOGE("Fail to insert media db [%d]", ret);
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
		DM_LOGE("Fail to connect media db");
	}
	DM_LOGD("Register file Done");
	return true;
}

#if 0
string DownloadUtil::getUserAgent()
{
	string userAgentStr = string();
	char *str = NULL;

	str = vconf_get_str(VCONFKEY_BROWSER_USER_AGENT);
	if (str) {
		DM_SLOGD("User agent str[%s]", str);
		userAgentStr.assign(str);
	}
	free(str);
	return userAgentStr;
}
#endif

#ifdef _ENABLE_OMA_UNSUPPROTED_CONTENT
/* If the error is happened, the true is returned */
bool app_matched_cb(app_control_h handle, const char *appid, void *user_data)
{
	int *count = NULL;
	count = (int *)user_data;
	if (appid) {
		DM_SLOGD("appid[%s]",appid);
		*count = *count + 1;
	}
	return true;
}

bool DownloadUtil::isSupportedMIMEType(string mime)
{
	app_control_h handle = NULL;
	int count = 0;
	if (mime.empty())
		return true;

	if (app_control_create(&handle) != APP_CONTROL_ERROR_NONE) {
		DM_LOGE("Fail to app_control create");
		return true;
	}
	if (app_control_set_operation(handle, APP_CONTROL_OPERATION_VIEW) !=
			APP_CONTROL_ERROR_NONE) {
		DM_LOGE("Fail to app_control set operation");
		app_control_destroy(handle);
		return true;
	}
	if (app_control_set_mime(handle, mime.c_str()) != APP_CONTROL_ERROR_NONE) {
		DM_LOGE("Fail to app_control set mime");
		app_control_destroy(handle);
		return true;
	}
	if (app_control_foreach_app_matched(handle, app_matched_cb, &count) != APP_CONTROL_ERROR_NONE) {
		DM_LOGE("Fail to app_control set mime");
		app_control_destroy(handle);
		return true;
	}
	DM_LOGV("Available application list count[%d]", count);
	app_control_destroy(handle);
	if (count > 0)
		return true;
	else
		return false;
}
#endif

#ifdef _ENABLE_WAITING_RO
bool DownloadDrm::validRo(const char *filePath)
{
	int ret = 0;
	drm_permission_type_e permType = DRM_PERMISSION_TYPE_PLAY;
	drm_license_status_e status = DRM_LICENSE_STATUS_UNDEFINED;
	ret = drm_get_license_status(filePath, permType, &status);
	if (ret != DRM_RETURN_SUCCESS)
		return false;
	if (status == DRM_LICENSE_STATUS_VALID)
		return true;
	return false;
}
#endif

