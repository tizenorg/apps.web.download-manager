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
	closeDB();\
	return false; \
}

sqlite3 *DownloadHistoryDB::historyDb = NULL;

DownloadHistoryDB::DownloadHistoryDB()
{
}

DownloadHistoryDB::~DownloadHistoryDB()
{
}

bool DownloadHistoryDB::openDB()
{
	DM_LOGV("");
	int ret = 0;
	if (historyDb == NULL) {
		DM_LOGD("TRY to open [%s]", DBDATADIR"/"HISTORYDB);
		if (sqlite3_open_v2(DBDATADIR"/"HISTORYDB, &historyDb,
				SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK) {
			DM_LOGD("[ERROR][%s][%s]", DBDATADIR"/"HISTORYDB,
				sqlite3_errmsg(historyDb));

			int errorcode = sqlite3_errcode(historyDb);
			closeDB();
			if (errorcode == SQLITE_CORRUPT) {
				DM_LOGD("unlink [%s]", DBDATADIR"/"HISTORYDB);
				unlink(DBDATADIR"/"HISTORYDB);
				errorcode = SQLITE_CANTOPEN;
			}

			if (errorcode == SQLITE_CANTOPEN) {
				DM_LOGD("Error :: CANTOPEN");
				int trycount = 0;
				while ((loadSqlSchema() == false) && (++trycount <= 3));
				if (trycount > 3)
					DM_LOGD("[ERROR] Could not restore DB");
				return isOpen();
			}
			return false;
		}
		ret = sqlite3_exec(historyDb, "PRAGMA journal_mode=PERSIST;", 0, 0, 0);
		if (SQLITE_OK != ret)
			return false;
	}

	return isOpen();
}

void DownloadHistoryDB::closeDB()
{
	DM_LOGV("");
	if (historyDb) {
		sqlite3_close(historyDb);
		historyDb = NULL;
	}
}

bool DownloadHistoryDB::createItemToDB(Item *item)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGV("");

	if (!item) {
		DM_LOGE("NULL Check:Item");
		return false;
	}

	if (item->getHistoryId() < 1) {
		DM_LOGE("Cannot add to DB. Because historyId is invaild");
		return false;
	}

	if (!openDB()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}

	const string statement = "insert into history (historyid, \
		downloadtype, contenttype, state, name, url, filename, \
		headerfield, headervalue, installdir, date) \
		values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	ret = sqlite3_prepare_v2(historyDb, statement.c_str(), -1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	/* binding values */
	if (sqlite3_bind_int(stmt, 1, item->getHistoryId()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 2, item->getDownloadType()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 3, item->getContentType()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 4, item->getState()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 5, item->getContentName().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 6, item->getUrl().c_str(), -1, NULL) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 7, item->getFileName().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 8, item->getReqHeaderField().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 9, item->getReqHeaderValue().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 10, item->getInstallDir().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_double(stmt, 11, time(NULL)) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_double:%s",
				sqlite3_errmsg(historyDb));
		return false;
	}

	ret = sqlite3_step(stmt);

	DM_LOGD("SQL return:%s",
			(ret == SQLITE_DONE || ret == SQLITE_OK)?"Success":"Fail");

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s", sqlite3_errmsg(historyDb));

	return (ret == SQLITE_DONE || ret == SQLITE_OK);
}

/* FIXME : Hitory entry limitation ?? */
bool DownloadHistoryDB::updateHistoryToDB(Item *item)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGV("");

	if (!item) {
		DM_LOGE("NULL Check:Item");
		return false;
	}

	if (item->getHistoryId() < 1) {
		DM_LOGE("Cannot add to DB. Because historyId is invaild");
		return false;
	}

	if (!openDB()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}
	const string statement = "update history set downloadtype=?,\
		contenttype=?, state=?, err=?, name=?, path=?, url=?, filename=?,\
		headerfield=?, headervalue=?, installdir=?, date=? where historyid = ?";

	ret = sqlite3_prepare_v2(historyDb, statement.c_str(), -1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	/* binding values */
	if (sqlite3_bind_int(stmt, 1, item->getDownloadType()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 2, item->getContentType()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 3, item->getState()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 4, item->getErrorCode()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 5, item->getTitle().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 6, item->getRegisteredFilePath().c_str(),
			-1, NULL) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 7, item->getUrl().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 8, item->getFileName().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 9, item->getReqHeaderField().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 10, item->getReqHeaderValue().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 11, item->getInstallDir().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_double(stmt, 12, item->getFinishedTime()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_double:%s",
				sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 13, item->getHistoryId()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	ret = sqlite3_step(stmt);

	DM_LOGD("SQL return: %s",
			(ret == SQLITE_DONE || ret == SQLITE_OK)?"Success":"Fail");

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s", sqlite3_errmsg(historyDb));

	return (ret == SQLITE_DONE || ret == SQLITE_OK);
}

bool DownloadHistoryDB::updateCanceledItemToDB(Item *item)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGV("");

	if (!item) {
		DM_LOGE("NULL Check:Item");
		return false;
	}

	if (item->getHistoryId() < 1) {
		DM_LOGE("Cannot add to DB. Because historyId is invaild");
		return false;
	}

	if (!openDB()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}
	const string statement = "update history set state=?, err=?, \
		date=? where historyid = ?";

	ret = sqlite3_prepare_v2(historyDb, statement.c_str(), -1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	/* binding values */
	if (sqlite3_bind_int(stmt, 1, item->getState()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 2, item->getErrorCode()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_double(stmt, 3, item->getFinishedTime()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_double:%s",
				sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 4, item->getHistoryId()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	ret = sqlite3_step(stmt);

	DM_LOGD("SQL return: %s",
			(ret == SQLITE_DONE || ret == SQLITE_OK)?"Success":"Fail");

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s", sqlite3_errmsg(historyDb));

	return (ret == SQLITE_DONE || ret == SQLITE_OK);
}

#ifdef _ENABLE_OMA_DOWNLOAD
bool DownloadHistoryDB::updateNotiUrlToHistoryDB(Item *item)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGV("");

	if (!item) {
		DM_LOGE("NULL Check:Item");
		return false;
	}

	if (item->getHistoryId() < 1) {
		DM_LOGE("Cannot add to DB. Because historyId is invaild");
		return false;
	}

	if (!openDB()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}
	const string statement = "update history set installnotifyurl=? where historyid = ?";

	ret = sqlite3_prepare_v2(historyDb, statement.c_str(), -1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	/* binding values */

	if (sqlite3_bind_text(stmt, 1, item->installNotifyUrl().c_str(), -1, NULL) !=
		SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 2, item->getHistoryId()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	ret = sqlite3_step(stmt);

	DM_LOGD("SQL return: %s",
			(ret == SQLITE_DONE || ret == SQLITE_OK)?"Success":"Fail");

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s", sqlite3_errmsg(historyDb));

	return (ret == SQLITE_DONE || ret == SQLITE_OK);
}
#endif

bool DownloadHistoryDB::updateDownloadInfoToDB(Item *item)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGV("");

	if (!item) {
		DM_LOGE("NULL Check:Item ");
		return false;
	}

	if (item->getHistoryId() < 1) {
		DM_LOGE("Cannot add to DB. Because historyId is invaild");
		return false;
	}

	if (!openDB()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}
#ifdef _ENABLE_OMA_DOWNLOAD
	const string statement = "update history set downloadtype=?,\
		contenttype=?, state=?, name=?, etag=?, tempfilepath=?,\
		installnotifyurl=?, date=?, filesize=? where historyid = ?";
#else
	const string statement = "update history set downloadtype=?,\
		contenttype=?, state=?, name=?, etag=?, tempfilepath=?,\
		date=?, filesize=? where historyid = ?";
#endif
	ret = sqlite3_prepare_v2(historyDb, statement.c_str(), -1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	/* binding values */
	if (sqlite3_bind_int(stmt, 1, item->getDownloadType()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 2, item->getContentType()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 3, item->getState()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 4, item->getTitle().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 5, item->getEtag().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_text(stmt, 6, item->getTempPath().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
#ifdef _ENABLE_OMA_DOWNLOAD
	if (sqlite3_bind_text(stmt, 7, item->installNotifyUrl().c_str(), -1, NULL) !=
			SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_text:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_double(stmt, 8, time(NULL)) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_double:%s",
				sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int64(stmt, 9, (sqlite_int64)item->getFileSize()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int64:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 10, item->getHistoryId()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
#else
	if (sqlite3_bind_double(stmt, 7, time(NULL)) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_double:%s",
				sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int64(stmt, 8, (sqlite_int64)item->getFileSize()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int64:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 9, item->getHistoryId()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
#endif
	ret = sqlite3_step(stmt);

	DM_LOGD("SQL return: %s",
			(ret == SQLITE_DONE || ret == SQLITE_OK)?"Success":"Fail");

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s", sqlite3_errmsg(historyDb));

	return (ret == SQLITE_DONE || ret == SQLITE_OK);
}

bool DownloadHistoryDB::updateStateToDB(Item *item)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGV("");

	if (!item) {
		DM_LOGE("NULL Check:Item");
		return false;
	}

	if (item->getHistoryId() < 1) {
		DM_LOGE("Cannot add to DB. Because historyId is invaild");
		return false;
	}

	if (!openDB()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}

	const string statement = "update history set state=? \
		where historyid = ?";

	ret = sqlite3_prepare_v2(historyDb, statement.c_str(), -1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	/* binding values */
	if (sqlite3_bind_int(stmt, 1, item->getState()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 2, item->getHistoryId()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}

	ret = sqlite3_step(stmt);

	DM_LOGD("SQL return: %s",
			(ret == SQLITE_DONE || ret == SQLITE_OK)?"Success":"Fail");

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s", sqlite3_errmsg(historyDb));

	return (ret == SQLITE_DONE || ret == SQLITE_OK);
}


bool DownloadHistoryDB::updateDownloadIdToDB(Item *item)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGV("");

	if (!item) {
		DM_LOGE("NULL Check:Item");
		return false;
	}

	if (item->getHistoryId() < 1) {
		DM_LOGE("Cannot add to DB. Because historyId is invaild");
		return false;
	}

	if (!openDB()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}

	const string statement = "update history set downloadid=? \
		where historyid = ?";

	ret = sqlite3_prepare_v2(historyDb, statement.c_str(), -1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	/* binding values */
	if (sqlite3_bind_int(stmt, 1, item->getId()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}
	if (sqlite3_bind_int(stmt, 2, item->getHistoryId()) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s", sqlite3_errmsg(historyDb));
		return false;
	}

	ret = sqlite3_step(stmt);

	DM_LOGD("SQL return: %s",
			(ret == SQLITE_DONE || ret == SQLITE_OK)?"Success":"Fail");

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s", sqlite3_errmsg(historyDb));

	return (ret == SQLITE_DONE || ret == SQLITE_OK);
}

bool DownloadHistoryDB::getCountOfHistory(int *count)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGV("");

	if (!openDB()) {
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
	return true;
}

bool DownloadHistoryDB::createRemainedItemsFromHistoryDB(int limit, int offset)
{
	int ret = 0;
	stringstream limitStr;
	stringstream offsetStr;
	sqlite3_stmt *stmt = NULL;
	string tmp;

	DM_LOGV("");

	if (!openDB()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}
	limitStr << limit;
	offsetStr << offset;

	tmp.append("select historyid, downloadid, downloadtype, contenttype, \
			state, err, ");
#ifdef _ENABLE_OMA_DOWNLOAD
	tmp.append("name, path, url, filename, headerfield, headervalue, \
			installdir, etag, tempfilepath, installnotifyurl, date, filesize \
			from history order by ");
#else
	tmp.append("name, path, url, filename, headerfield, headervalue, \
			installdir, etag, tempfilepath, date, filesize from history order by ");
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
			string fileName = string();
			string reqHeaderField = string();
			string reqHeaderValue = string();
			string installDir = string();
			string tempFilePath = string();
			string etag = string();
			Item *item = Item::createHistoryItem();
			if (!item) {
				DM_LOGE("Fail to create item");
				break;
			}
			item->setHistoryId(sqlite3_column_int(stmt, 0));
			item->setId(sqlite3_column_int(stmt, 1));
			item->setDownloadType((DL_TYPE::TYPE)sqlite3_column_int(stmt, 2));
			item->setContentType(sqlite3_column_int(stmt, 3));
			item->setState((ITEM::STATE)sqlite3_column_int(stmt, 4));
			item->setErrorCode((ERROR::CODE)sqlite3_column_int(stmt, 5));
			tempStr = (const char *)(sqlite3_column_text(stmt, 6));
			if (tempStr)
				arg = tempStr;
			else
				arg = string();
			item->setTitle(arg);
			tempStr = (const char *)(sqlite3_column_text(stmt, 7));
			if (tempStr)
				arg = tempStr;
			else
				arg = string();
			item->setRegisteredFilePath(arg);
			tempStr = (const char *)(sqlite3_column_text(stmt, 8));
			if (tempStr)
				url = tempStr;
			else
				url = string();
			tempStr = (const char *)(sqlite3_column_text(stmt, 9));
			if (tempStr)
				fileName = tempStr;
			else
				fileName = string();
			tempStr = (const char *)(sqlite3_column_text(stmt, 10));
			if (tempStr)
				reqHeaderField = tempStr;
			else
				reqHeaderField = string();
			tempStr = (const char *)(sqlite3_column_text(stmt, 11));
			if (tempStr)
				reqHeaderValue = tempStr;
			else
				reqHeaderValue = string();
			tempStr = (const char *)(sqlite3_column_text(stmt, 12));
			if (tempStr)
				installDir = tempStr;
			else
				installDir = string();
			tempStr = (const char *)(sqlite3_column_text(stmt, 13));
			if (tempStr)
				etag = tempStr;
			else
				etag = string();
			tempStr = (const char *)(sqlite3_column_text(stmt, 14));
			if (tempStr)
				tempFilePath = tempStr;
			else
				tempFilePath = string();
#ifdef _ENABLE_OMA_DOWNLOAD
			tempStr = (const char *)(sqlite3_column_text(stmt, 15));
			if (tempStr) {
				string tempStrObj = string(tempStr);
				item->setInstallNotifyUrl(tempStrObj);
			}
			item->setFinishedTime(sqlite3_column_double(stmt, 16));
			item->setFileSize((unsigned long long)sqlite3_column_int64(stmt, 17));
#else
			item->setFinishedTime(sqlite3_column_double(stmt, 15));
			item->setFileSize((unsigned long long)sqlite3_column_int64(stmt, 16));
#endif
			item->attachHistoryItem();
			item->setRetryData(url, reqHeaderField, reqHeaderValue,
					installDir, fileName, tempFilePath, etag);
		} else
			break;
	}
	DM_LOGD("SQL return: %s",
				(ret == SQLITE_ROW || ret == SQLITE_OK)?"Success":"Fail");

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("sqlite3_finalize is failed:[%s]", sqlite3_errmsg(historyDb));

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

	DM_LOGV("");

	if (!openDB()) {
		DM_LOGE("NULL check:historyDB");
		return false;
	}
	limitStr << LOAD_HISTORY_COUNT;
	tmp.append("select historyid, downloadid, downloadtype, contenttype, state, err, ");
#ifdef _ENABLE_OMA_DOWNLOAD
	tmp.append("name, path, url, filename, headerfield, headervalue, installdir, \
			etag, tempfilepath, installnotifyurl, date, filesize from history order by ");
#else
	tmp.append("name, path, url, filename, headerfield, headervalue, \
			installdir, etag, tempfilepath, date, filesize from history order by ");
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
			string fileName = string();
			string reqHeaderField = string();
			string reqHeaderValue = string();
			string installDir = string();
			string tempFilePath = string();
			string etag = string();
			Item *item = Item::createHistoryItem();
			if (!item) {
				DM_LOGE("Fail to create item");
				break;
			}
			item->setHistoryId(sqlite3_column_int(stmt, 0));
			item->setId(sqlite3_column_int(stmt, 1));
			item->setDownloadType((DL_TYPE::TYPE)sqlite3_column_int(stmt, 2));
			item->setContentType(sqlite3_column_int(stmt, 3));
			item->setState((ITEM::STATE)sqlite3_column_int(stmt, 4));
			item->setErrorCode((ERROR::CODE)sqlite3_column_int(stmt, 5));
			tempStr = (const char *)(sqlite3_column_text(stmt, 6));
			if (tempStr) {
				arg = tempStr;
				item->setTitle(arg);
			}
			tempStr = (const char *)(sqlite3_column_text(stmt, 7));
			if (tempStr)
				arg = tempStr;
			item->setRegisteredFilePath(arg);
			tempStr = (const char *)(sqlite3_column_text(stmt, 8));
			if (tempStr)
				url = tempStr;
			tempStr = (const char *)(sqlite3_column_text(stmt, 9));
			if (tempStr)
				fileName = tempStr;
			tempStr = (const char *)(sqlite3_column_text(stmt, 10));
			if (tempStr)
				reqHeaderField = tempStr;
			tempStr = (const char *)(sqlite3_column_text(stmt, 11));
			if (tempStr)
				reqHeaderValue = tempStr;
			tempStr = (const char *)(sqlite3_column_text(stmt, 12));
			if (tempStr)
				installDir = tempStr;
			tempStr = (const char *)(sqlite3_column_text(stmt, 13));
			if (tempStr)
				etag = tempStr;
			tempStr = (const char *)(sqlite3_column_text(stmt, 14));
			if (tempStr)
				tempFilePath = tempStr;
#ifdef _ENABLE_OMA_DOWNLOAD
			tempStr = (const char *)(sqlite3_column_text(stmt, 15));
			if (tempStr) {
				string tempStrObj = string(tempStr);
				item->setInstallNotifyUrl(tempStrObj);
			}
			item->setFinishedTime(sqlite3_column_double(stmt, 16));
			item->setFileSize((unsigned long long)sqlite3_column_int64(stmt, 17));
#else
			item->setFinishedTime(sqlite3_column_double(stmt, 15));
			item->setFileSize((unsigned long long)sqlite3_column_int64(stmt, 16));
#endif
			item->attachHistoryItem();
			item->setRetryData(url, reqHeaderField, reqHeaderValue,
					installDir, fileName, tempFilePath, etag);
		} else
			break;
	}

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s",
				sqlite3_errmsg(historyDb));

	if (ret == SQLITE_DONE || ret == SQLITE_OK)
		return true;
	else
		return false;
}

bool DownloadHistoryDB::deleteItem(unsigned int historyId)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGV("");

	if (!openDB()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}

	ret = sqlite3_prepare_v2(historyDb, "delete from history where historyid=?",
		-1, &stmt, NULL);

	if (ret != SQLITE_OK)
		FINALIZE_ON_ERROR(stmt);
	if (sqlite3_bind_int(stmt, 1, historyId) != SQLITE_OK) {
		DM_LOGE("Fail to call sqlite3_bind_int:%s",
				sqlite3_errmsg(historyDb));
		return false;
	}
	ret = sqlite3_step(stmt);
	if (ret != SQLITE_OK && ret != SQLITE_DONE)
		FINALIZE_ON_ERROR(stmt);

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s",
				sqlite3_errmsg(historyDb));
	return true;
}

bool DownloadHistoryDB::deleteMultipleItem(queue <unsigned int> &q)
{
	int ret = 0;
	unsigned int historyId = -1;
	sqlite3_stmt *stmt = NULL;
	char *errmsg = NULL;

	DM_LOGV("");

	if (!openDB()) {
		DM_LOGE("NULL Check:historyDB");
		return false;
	}
	ret = sqlite3_exec(historyDb, "PRAGMA synchronous=OFF;\
		PRAGMA count_changes=OFF; PRAGMA temp_store=memory;",
		NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		sqlite3_free(errmsg);
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
		if (sqlite3_bind_int(stmt, 1, historyId) != SQLITE_OK) {
			DM_LOGE("Fail to call sqlite3_bind_int:%s",
					sqlite3_errmsg(historyDb));
			return false;
		}
		ret = sqlite3_step(stmt);
		if (ret != SQLITE_OK && ret != SQLITE_DONE)
			FINALIZE_ON_ERROR(stmt);
	}

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		DM_LOGE("Fail to call sqlite3_finalize:%s",
				sqlite3_errmsg(historyDb));
	return true;
}

bool DownloadHistoryDB::clearData(void)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	DM_LOGD("");

	if (!openDB()) {
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
	return true;
}

bool DownloadHistoryDB::loadSqlSchema(void)
{
	if (sqlite3_open_v2(DBDATADIR"/"HISTORYDB, &historyDb,
				SQLITE_OPEN_READWRITE |SQLITE_OPEN_CREATE, NULL) != SQLITE_OK) {
		DM_LOGD("[ERROR][%s][%s]", DBDATADIR"/"HISTORYDB,
				sqlite3_errmsg(historyDb));
		closeDB();
		return false;
	}

	char *error_msg = NULL;
	int ret = 0;
	ret = sqlite3_exec(historyDb, REBUILD_QUERY, 0, 0, &error_msg);
	if((error_msg != NULL) || (SQLITE_OK != ret)) {
		DM_LOGD("SQL error :: [%s]",error_msg);
		sqlite3_free(error_msg);
		error_msg = NULL;
		closeDB();
		return false;
	}

	ret = sqlite3_exec(historyDb, REBUILD_QUERY_INDEX, 0, 0, &error_msg);
	if((error_msg != NULL) || (SQLITE_OK != ret)) {
		DM_LOGD("SQL error :: [%s]",error_msg);
		sqlite3_free(error_msg);
		error_msg = NULL;
	}
	return true;
}

