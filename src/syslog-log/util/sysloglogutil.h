/*
 * sysloglogutil.h
 * 
 * Copyright (c) IBM Corp. 2003, 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE 
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE 
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Author: R Sharada <sharada@in.ibm.com>
 *
 * Interface Type: Common Manageability Programming Interface ( CMPI )
 *
 * Description: Utility functions for CIM Linux Syslog Log Provider
 */


#ifndef SYSLOGLOGUTIL_H
#define SYSLOGLOGUTIL_H

#include <time.h>
#include <stdio.h>
#include "config.h"

#ifdef __cplusplus 
/*extern "C" {*/
#endif

#ifdef DEBUG
#define syslog_debug(fd, args...) fprintf(fd,args)
#else
#define syslog_debug(a,b...)
#endif
  
/* ------------------------------------------------------------------
 * Utilities for file info retrieval
 * ----------------------------------------------------------------- */

#define MAXSIZE    1025

#define SYSLOG_CONFPATH SYSLOG_CONF_DIR "/" SYSLOG_CONF_FILE

struct _Syslog_MessageLog {
  char  *svName;
  char *svCCname;
};

typedef struct _Syslog_MessageLog Syslog_MessageLog;

struct _Syslog_LogRecord {
 char *logCCname;
 char *logname;
 char *CCname;
 char *recordid;
 char *timestamp;
 char *hostname;
 char *data;
};

typedef struct _Syslog_LogRecord Syslog_LogRecord;

static char finalyear[5];

/*
 ------------------------------------------------------------------
 * Log Enumeration Support, use like this:
 *
 *  Syslog_MessageLog logbuf;
 *  void *hdl = Syslog_Log_Begin_Enum(logpathname);
 *  if (hdl) {
 *    while(Syslog_Log_Next_Enum(hdl,&logbuf) {...}
 *    Syslog_Log_End_Enum(hdl);
 *  }
 * ----------------------------------------------------------------- */


void* Syslog_MessageLog_Begin_Enum(const char *name);
int Syslog_MessageLog_Next_Enum(void *handle, Syslog_MessageLog *logbuf);
void Syslog_MessageLog_End_Enum(void *handle);

/* Similarly for the Log Record class enumeration */
void* Syslog_LogRecord_Begin_Enum(const char *name);
int Syslog_LogRecord_Next_Enum(void *handle, Syslog_LogRecord *logrecbuf);
void Syslog_LogRecord_End_Enum(void *handle);

void getyear(const char *name);
int timeformat(const char *itimebuf, char *otimebuf);

void releaseMsgLogBuf(Syslog_MessageLog *p);
void releaseLogRecBuf(Syslog_LogRecord *p);
#ifdef __cplusplus 
/*}*/
#endif

#endif
