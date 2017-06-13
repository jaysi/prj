﻿#ifndef __DAY13_H
#define __DAY13_H

#include <sys/types.h>
#include <sys/time.h>
#ifdef linux
#include <time.h>
#endif

#include "error13.h"

#ifdef __WIN32
#define BASEYEAR    1900
#else
#define BASEYEAR    1970
#endif

#define D13_ITEMS	6 //year; month; day; hour; minute; second

#define MAXTIME     20  //yyyy-mm-dd-hh-mm-ss

#define d13_datesep "/.-"

#ifdef __cplusplus
	extern "C" {
#endif

//date-conversion
error13_t d13_g2j(int g_y, int g_m, int g_d, int jdate[D13_ITEMS]);
error13_t d13_j2g(int j_y, int j_m, int j_d, int gdate[D13_ITEMS]);
error13_t d13_today(int date[D13_ITEMS]);
error13_t d13_now(char t13[MAXTIME]);
error13_t d13_time13(time_t* t, char t13[MAXTIME]);
error13_t d13_resolve_date(char* date, int d[D13_ITEMS]);
error13_t d13_fix_jdate(char* src, char date[MAXTIME]);

//gregorian
error13_t d13_time13g(time_t* t, char t13[MAXTIME]);
error13_t d13_todayg(int date[D13_ITEMS]);
error13_t d13_nowg(char t13[MAXTIME]);

uint32_t d13_jdayno(int j_y, int j_m, int j_d);
error13_t d13_jdayno2jdate(unsigned long j_day_no, int jdate[3]);

/*
 * date compare rules:
 * 0 means any...
 * i.e. 0000/??/?? means a specific day in month in all years
 * or ????/00/?? means a specific day in in all months of a specific year.
*/
int d13_cmp_time13(int t13_1[D13_ITEMS], int t13_2[D13_ITEMS]);

#ifdef __cplusplus
	}
#endif

#else

#endif
