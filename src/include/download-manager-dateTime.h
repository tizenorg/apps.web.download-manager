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
 * @file 	download-manager-dateTime.h
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	data and utility APIs for Date and Time
 */

#ifndef DOWNLOAD_MANAGER_DATE_TIME_H
#define DOWNLOAD_MANAGER_DATE_TIME_H

#include <time.h>
#include <utils_i18n.h>

#include "download-manager-common.h"

using namespace std;

class DateUtil {
public:
	static DateUtil& getInstance(void) {
		static DateUtil inst;
		return inst;
	}
	void updateLocale(void);
	int getDiffDays(time_t nowTime, time_t refTime);
	void getDateStr(double finishTime, string &outBuf);

private:
	DateUtil(void);
	~DateUtil(void);
	void deinitLocaleData();
	i18n_udate_format_h getBestPattern(const char *formatString,
			i18n_udatepg_h patternGenerator, const char *locale);
	/* Update this in case of follows
	 * 1. show main view.
	 * 2. add new item
	 * 3. create today group
	**/
	i18n_udate_format_h dateShortFormat;
	i18n_udate_format_h timeFormat12H;
	i18n_udate_format_h timeFormat24H;

};

#endif /* DOWNLOAD_MANAGER_DATE_TIME_H */
