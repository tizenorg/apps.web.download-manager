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
 * @file 	download-manager-history-db.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	Manager for a download history DB
 */

#include <sstream>
#include "download-manager-common.h"
#include "download-manager-history-db.h"

#define FINALIZE_ON_ERROR( stmt ) { \
	DM_LOGE("SQL error:%d[%s]", ret, sqlite3_errmsg(historyDb));\
	if (sqlite3_finalize(stmt) != SQLITE_OK)\
		DM_LOGE("sqlite3_finalize is failed");\
	close();\
	return false; \
}

sqlite3 *DownloadHistoryDB::historyDb = NULL;

DownloadHistoryDB::DownloadHistoryDB()
{
}

DownloadHistoryDB::~DownloadHistoryDB()
{
}

bool DownloadHistoryDB::open()
{
	int ret = 0;

	DM_LOGD("");

	close();

	ret = db_util_open(DBDATADIR"/"HISTORYDB, &historyDb,
		DB_UTIL_REGISTER_HOOK_METHOD);

	if (ret != SQLITE_OK) {
		DM_LOGE("open fail:%s", sqlite3_errmsg(historyDb));
		db_util_close(historyDb);
		historyDb = NULL;
		return false;
	}

	return isOpen();
}

void DownloadHistoryDB::close()
{
	DM_LOGD("");
	if (historyDb) {
		db_util_close(historyDb);
		historyDb = NULL;
	}
}

bool DownloadHistoryDB::createItemToDB(Item *item)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGI("");

	if (!item) {
		DM_LOGE("NULL Check:Item");
		return false;
	}

	if (item->historyId() < 1) {
		DM_LOGE("Cannot add to DB. Because historyId is invaild");
		return false;
	}

	if (!open()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}

	const string statement = "insert into history (historyid, \
		downloadtype, contenttype, state, name, url, cookie, \
		headerfield, headervalue, installdir, date) \
		values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	ret = sqlite3_prepare_v2(historyDb, statement.c_str(), -1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	/* binding values */
	if (sqlite3_bind_int(stmt, 1, item->historyId()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		if (sqlite3_bind_int(stmt, 2, item->downloadType()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_int(stmt, 3, item->contentType()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_int(stmt, 4, item->state()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 5, item->contentName().c_str(), -1, NULL) !=
			SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 6, item->url().c_str(), -1, NULL) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 7, item->cookie().c_str(), -1, NULL) !=
			SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 8, item->reqHeaderField().c_str(), -1, NULL) !=
			SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 9, item->reqHeaderValue().c_str(), -1, NULL) !=
			SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 10, item->installDir().c_str(), -1, NULL) !=
			SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_double(stmt, 11, time(NULL)) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_double:%s",
				sqlite3_errmsg(historyDb));

	ret = sqlite3_step(stmt);

	DM_LOGI("SQL return:%s",
			(ret == SQLITE_DONE || ret == SQLITE_OK)?"Success":"Fail");

	close();

	return (ret == SQLITE_DONE || ret == SQLITE_OK);
}

/* FIXME : Hitory entry limitation ?? */
bool DownloadHistoryDB::updateHistoryToDB(Item *item)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGD("");

	if (!item) {
		DM_LOGE("NULL Check:Item");
		return false;
	}

	if (item->historyId() < 1) {
		DM_LOGE("Cannot add to DB. Because historyId is invaild");
		return false;
	}

	if (!open()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}
	const string statement = "update history set downloadtype=?,\
		contenttype=?, state=?, err=?, name=?, path=?, url=?, cookie=?,\
		headerfield=?, headervalue=?, installdir=?, date=? where historyid = ?";

	ret = sqlite3_prepare_v2(historyDb, statement.c_str(), -1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	/* binding values */
	if (sqlite3_bind_int(stmt, 1, item->downloadType()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_int(stmt, 2, item->contentType()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_int(stmt, 3, item->state()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_int(stmt, 4, item->errorCode()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 5, item->title().c_str(), -1, NULL) !=
			SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(
			stmt, 6, item->registeredFilePath().c_str(), -1, NULL) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 7, item->url().c_str(), -1, NULL) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 8, item->cookie().c_str(), -1, NULL) !=
			SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 9, item->reqHeaderField().c_str(), -1, NULL) !=
			SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 10, item->reqHeaderValue().c_str(), -1, NULL) !=
			SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 11, item->installDir().c_str(), -1, NULL) !=
			SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_double(stmt, 12, item->finishedTime()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_double:%s",
				sqlite3_errmsg(historyDb));
	if (sqlite3_bind_int(stmt, 13, item->historyId()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	ret = sqlite3_step(stmt);

	DM_LOGI("SQL return: %s",
			(ret == SQLITE_DONE || ret == SQLITE_OK)?"Success":"Fail");

	close();

	return (ret == SQLITE_DONE || ret == SQLITE_OK);
}

bool DownloadHistoryDB::updateDownloadInfoToDB(Item *item)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGD("");

	if (!item) {
		DM_LOGE("NULL Check:Item ");
		return false;
	}

	if (item->historyId() < 1) {
		DM_LOGE("Cannot add to DB. Because historyId is invaild");
		return false;
	}

	if (!open()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}
#ifdef _ENABLE_OMA_DOWNLOAD
	const string statement = "update history set downloadtype=?,\
		contenttype=?, state=?, name=?, date=?, installnotifyurl=? \
		where historyid = ?";
#else
	const string statement = "update history set downloadtype=?,\
		contenttype=?, state=?, name=?, date=? \
		where historyid = ?";
#endif
	ret = sqlite3_prepare_v2(historyDb, statement.c_str(), -1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	/* binding values */
	if (sqlite3_bind_int(stmt, 1, item->downloadType()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_int(stmt, 2, item->contentType()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_int(stmt, 3, item->state()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_text(stmt, 4, item->title().c_str(), -1, NULL) !=
			SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
#ifdef _ENABLE_OMA_DOWNLOAD
	if (sqlite3_bind_text(stmt, 5, item->installNotifyUrl().c_str(), -1, NULL) !=
			SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_double(stmt, 6, time(NULL)) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_double:%s",
				sqlite3_errmsg(historyDb));
	if (sqlite3_bind_int(stmt, 7, item->historyId()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
#else
	if (sqlite3_bind_double(stmt, 5, time(NULL)) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_double:%s",
				sqlite3_errmsg(historyDb));
	if (sqlite3_bind_int(stmt, 6, item->historyId()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
#endif
	ret = sqlite3_step(stmt);

	DM_LOGI("SQL return: %s",
			(ret == SQLITE_DONE || ret == SQLITE_OK)?"Success":"Fail");

	close();

	return (ret == SQLITE_DONE || ret == SQLITE_OK);
}

bool DownloadHistoryDB::updateStateToDB(Item *item)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGD("");

	if (!item) {
		DM_LOGE("NULL Check:Item");
		return false;
	}

	if (item->historyId() < 1) {
		DM_LOGE("Cannot add to DB. Because historyId is invaild");
		return false;
	}

	if (!open()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}

	const string statement = "update history set state=? \
		where historyid = ?";

	ret = sqlite3_prepare_v2(historyDb, statement.c_str(), -1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	/* binding values */
	if (sqlite3_bind_int(stmt, 1, item->state()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_int(stmt, 2, item->historyId()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));

	ret = sqlite3_step(stmt);

	DM_LOGI("SQL return: %s",
			(ret == SQLITE_DONE || ret == SQLITE_OK)?"Success":"Fail");

	close();

	return (ret == SQLITE_DONE || ret == SQLITE_OK);
}


bool DownloadHistoryDB::updateDownloadIdToDB(Item *item)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGD("");

	if (!item) {
		DM_LOGE("NULL Check:Item");
		return false;
	}

	if (item->historyId() < 1) {
		DM_LOGE("Cannot add to DB. Because historyId is invaild");
		return false;
	}

	if (!open()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}

	const string statement = "update history set downloadid=? \
		where historyid = ?";

	ret = sqlite3_prepare_v2(historyDb, statement.c_str(), -1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	/* binding values */
	if (sqlite3_bind_int(stmt, 1, item->id()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
	if (sqlite3_bind_int(stmt, 2, item->historyId()) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));

	ret = sqlite3_step(stmt);

	DM_LOGI("SQL return: %s",
			(ret == SQLITE_DONE || ret == SQLITE_OK)?"Success":"Fail");

	close();

	return (ret == SQLITE_DONE || ret == SQLITE_OK);
}

bool DownloadHistoryDB::getCountOfHistory(int *count)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGD("");

	if (!open()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}
	ret = sqlite3_prepare_v2(historyDb, "select COUNT(*) from history", -1, &stmt, NULL);
	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);

	ret = sqlite3_step(stmt);
	DM_LOGD("SQL return: %s",
			(ret == SQLITE_ROW || ret == SQLITE_OK)?"Success":"Fail");
	if (ret == SQLITE_ROW) {
		*count = sqlite3_column_int(stmt,0);
		DM_LOGI("count[%d]",*count);
	} else {
		DM_LOGE("SQL query error:%s", sqlite3_errmsg(historyDb));
		*count = 0;
	}

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s", sqlite3_errmsg(historyDb));
	close();
	return true;
}

bool DownloadHistoryDB::createRemainedItemsFromHistoryDB(int limit, int offset)
{
	int ret = 0;
	stringstream limitStr;
	stringstream offsetStr;
	sqlite3_stmt *stmt = NULL;
	string tmp;

	DM_LOGD("");

	if (!open()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}
	limitStr << limit;
	offsetStr << offset;

	tmp.append("select historyid, downloadid, downloadtype, contenttype, \
			state, err, ");
#ifdef _ENABLE_OMA_DOWNLOAD
	tmp.append("name, path, url, cookie, headerfield, headervalue, \
			installdir, installnotifyurl, date from history order by ");
#else
	tmp.append("name, path, url, cookie, headerfield, headervalue, \
			installdir, date from history order by ");
#endif
	tmp.append("date DESC limit ");
	tmp.append(limitStr.str());
	tmp.append(" offset ");
	tmp.append(offsetStr.str());
	ret = sqlite3_prepare_v2(historyDb, tmp.c_str(), -1, &stmt, NULL);
	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);

	for (;;) {
		ret = sqlite3_step(stmt);
		if (ret == SQLITE_ROW) {
			const char *tempStr = NULL;
			string arg = string();
			string url = string();
			string cookie = string();
			string reqHeaderField = string();
			string reqHeaderValue = string();
			string installDir = string();
			Item *item = Item::createHistoryItem();
			if (!item) {
				DM_LOGE("Fail to create item");
				break;
			}
			item->setHistoryId(sqlite3_column_int(stmt,0));
			item->setId(sqlite3_column_int(stmt,1));
			item->setDownloadType((DL_TYPE::TYPE)sqlite3_column_int(stmt,2));
			item->setContentType(sqlite3_column_int(stmt,3));
			item->setState((ITEM::STATE)sqlite3_column_int(stmt,4));
			item->setErrorCode((ERROR::CODE)sqlite3_column_int(stmt,5));
			tempStr = (const char *)(sqlite3_column_text(stmt,6));
			if (tempStr)
				arg = tempStr;
			else
				arg = string();
			item->setTitle(arg);
			tempStr = (const char *)(sqlite3_column_text(stmt,7));
			if (tempStr)
				arg = tempStr;
			else
				arg = string();
			item->setRegisteredFilePath(arg);
			tempStr = (const char *)(sqlite3_column_text(stmt,8));
			if (tempStr)
				url = tempStr;
			else
				url = string();
			tempStr = (const char *)(sqlite3_column_text(stmt,9));
			if (tempStr)
				cookie = tempStr;
			else
				cookie = string();
			tempStr = (const char *)(sqlite3_column_text(stmt,10));
			if (tempStr)
				reqHeaderField = tempStr;
			else
				reqHeaderField = string();
			tempStr = (const char *)(sqlite3_column_text(stmt,11));
			if (tempStr)
				reqHeaderValue = tempStr;
			else
				reqHeaderValue = string();
			tempStr = (const char *)(sqlite3_column_text(stmt,12));
			if (tempStr)
				installDir = tempStr;
			else
				installDir = string();
#ifdef _ENABLE_OMA_DOWNLOAD
			tempStr = (const char *)(sqlite3_column_text(stmt,13));
			if (tempStr) {
				string tempStrObj = string(tempStr);
				item->setInstallNotifyUrl(tempStrObj);
			}
			item->setFinishedTime(sqlite3_column_double(stmt,14));
#else
			item->setFinishedTime(sqlite3_column_double(stmt,13));
#endif
			item->attachHistoryItem();
			item->setRetryData(url, cookie, reqHeaderField, reqHeaderValue,
					installDir);
		} else
			break;
	}
	DM_LOGD("SQL return: %s",
				(ret == SQLITE_ROW || ret == SQLITE_OK)?"Success":"Fail");

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("sqlite3_finalize is failed:[%s]", sqlite3_errmsg(historyDb));

	close();

	if (ret == SQLITE_DONE || ret == SQLITE_OK)
		return true;
	else
		return false;
}

bool DownloadHistoryDB::createItemsFromHistoryDB()
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	string tmp;
	stringstream limitStr;

	DM_LOGD();

	if (!open()) {
		DM_LOGE("NULL check:historyDB");
		return false;
	}
	limitStr << LOAD_HISTORY_COUNT;
	tmp.append("select historyid, downloadid, downloadtype, contenttype, state, err, ");
#ifdef _ENABLE_OMA_DOWNLOAD
	tmp.append("name, path, url, cookie, headerfield, headervalue, installdir, \
			installnotifyurl, date from history order by ");
#else
	tmp.append("name, path, url, cookie, headerfield, headervalue, \
			installdir, date from history order by ");
#endif
	tmp.append("date DESC limit ");
	tmp.append(limitStr.str());
	ret = sqlite3_prepare_v2(historyDb, tmp.c_str(), -1, &stmt, NULL);
	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);

	for (;;) {
		ret = sqlite3_step(stmt);
		if (ret == SQLITE_ROW) {
			const char *tempStr = NULL;
			string arg = string();
			string url = string();
			string cookie = string();
			string reqHeaderField = string();
			string reqHeaderValue = string();
			string installDir = string();
			Item *item = Item::createHistoryItem();
			if (!item) {
				DM_LOGE("Fail to create item");
				break;
			}
			item->setHistoryId(sqlite3_column_int(stmt,0));
			item->setId(sqlite3_column_int(stmt,1));
			item->setDownloadType((DL_TYPE::TYPE)sqlite3_column_int(stmt,2));
			item->setContentType(sqlite3_column_int(stmt,3));
			item->setState((ITEM::STATE)sqlite3_column_int(stmt,4));
			item->setErrorCode((ERROR::CODE)sqlite3_column_int(stmt,5));
			tempStr = (const char *)(sqlite3_column_text(stmt,6));
			if (tempStr) {
				arg = tempStr;
				item->setTitle(arg);
			}
			tempStr = (const char *)(sqlite3_column_text(stmt,7));
			if (tempStr)
				arg = tempStr;
			item->setRegisteredFilePath(arg);
			tempStr = (const char *)(sqlite3_column_text(stmt,8));
			if (tempStr)
				url = tempStr;
			tempStr = (const char *)(sqlite3_column_text(stmt,9));
			if (tempStr)
				cookie = tempStr;
			tempStr = (const char *)(sqlite3_column_text(stmt,10));
			if (tempStr)
				reqHeaderField = tempStr;
			tempStr = (const char *)(sqlite3_column_text(stmt,11));
			if (tempStr)
				reqHeaderValue = tempStr;
			tempStr = (const char *)(sqlite3_column_text(stmt,12));
			if (tempStr)
				installDir = tempStr;
#ifdef _ENABLE_OMA_DOWNLOAD
			tempStr = (const char *)(sqlite3_column_text(stmt,13));
			if (tempStr) {
				string tempStrObj = string(tempStr);
				item->setInstallNotifyUrl(tempStrObj);
			}
			item->setFinishedTime(sqlite3_column_double(stmt,14));
#else
			item->setFinishedTime(sqlite3_column_double(stmt,13));
#endif
			item->attachHistoryItem();
			item->setRetryData(url, cookie, reqHeaderField, reqHeaderValue,
					installDir);
		} else
			break;
	}

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s",
				sqlite3_errmsg(historyDb));
	close();

	if (ret == SQLITE_DONE || ret == SQLITE_OK)
		return true;
	else
		return false;
}

bool DownloadHistoryDB::deleteItem(unsigned int historyId)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGI("");

	if (!open()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}

	ret = sqlite3_prepare_v2(historyDb, "delete from history where historyid=?",
		-1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	if (sqlite3_bind_int(stmt, 1, historyId) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_bind_int:%s",
				sqlite3_errmsg(historyDb));
	ret = sqlite3_step(stmt);
	if (ret != SQLITE_OK && ret != SQLITE_DONE)
		FINALIZE_ON_ERROR(stmt);

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s",
				sqlite3_errmsg(historyDb));
	close();
	return true;
}

bool DownloadHistoryDB::deleteMultipleItem(queue <unsigned int> &q)
{
	int ret = 0;
	unsigned int historyId = -1;
	sqlite3_stmt *stmt = NULL;
	char *errmsg = NULL;

	DM_LOGI("");

	if (!open()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}
	ret = sqlite3_exec(historyDb, "PRAGMA synchronous=OFF;\
		PRAGMA count_changes=OFF; PRAGMA temp_store=memory;",
		NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		sqlite3_free(errmsg);
		close();
		return false;
	}

	DM_LOGI("queue size[%d]",q.size());
	while (!q.empty()) {
		ret = sqlite3_prepare_v2(historyDb, "delete from history where historyid=?",
			-1, &stmt, NULL);
		if (ret != SQLITE_OK)
			FINALIZE_ON_ERROR(stmt);
		historyId = q.front();
		q.pop();
		if (sqlite3_bind_int(stmt, 1, historyId) != SQLITE_OK)
			DM_LOGE("Fail to call sqlite3_bind_int:%s",
					sqlite3_errmsg(historyDb));
		ret = sqlite3_step(stmt);
		if (ret != SQLITE_OK && ret != SQLITE_DONE)
			FINALIZE_ON_ERROR(stmt);
	}

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s",
				sqlite3_errmsg(historyDb));
	close();
	return true;
}

bool DownloadHistoryDB::clearData(void)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGI("");

	if (!open()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}

	ret = sqlite3_prepare_v2(historyDb, "delete from history", -1, &stmt, NULL);
	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_OK && ret != SQLITE_DONE)
		FINALIZE_ON_ERROR(stmt);

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s",
				sqlite3_errmsg(historyDb));
	close();
	return true;
}

