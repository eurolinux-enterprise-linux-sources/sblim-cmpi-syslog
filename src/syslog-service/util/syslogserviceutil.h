/*
 * syslogserviceutil.h
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
 * Description: Utility functions for CIM Linux Syslog Service Provider
 */

#ifndef SYSLOGSERVICEUTIL_H
#define SYSLOGSERVICEUTIL_H

#include <time.h>
#include <stdio.h>

#ifdef __cplusplus 
/*extern "C" {*/
#endif

#ifdef DEBUG
#define syslog_debug(fd, args...) fprintf(fd,args)
#else
#define syslog_debug(a,b...)
#endif
  
#define MAXSIZE    1025

struct _Syslog_Service {
  char *svSystemCCname;
  char *svSystemname;
  char *svCCname;
  char *svName;
  char *svStatus;
  int svStarted;
  int syslogd;
  int klogd;
};

typedef struct _Syslog_Service Syslog_Service;

/*
 * Service Enumeration Support, use like this:
 *
 *  Syslog_Service servicebuf;
 *  void * hdl = Syslog_Begin_Enum();
 *  if (hdl) {
 *    while(Syslog_Next_Enum(hdl,&servicebuf) {...}
 *    Syslog_End_Enum(hdl);
 *  }
 */

void* Syslog_Service_Begin_Enum();
int Syslog_Service_Next_Enum(void *handle, Syslog_Service* svc);
void Syslog_Service_End_Enum(void *handle);
int Syslog_Service_Operation(const char *method, char *result, int resultlen);

#ifdef __cplusplus 
/*}*/
#endif

#endif
