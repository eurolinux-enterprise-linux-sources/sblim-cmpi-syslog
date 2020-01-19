/*
 * syslogconfutil.h
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
 * Description: Utility functions for CIM Linux Syslog Configuration Provider
 */


#ifndef SYSLOGCONFUTIL_H
#define SYSLOGCONFUTIL_H

#include <time.h>
#include <stdio.h>
#include "config.h"

#define LogConfFile  SYSLOG_CONF_DIR "/" SYSLOG_CONF_FILE
#define ConfFileName  SYSLOG_CONF_FILE
#define TmpConfFile  SYSLOG_CONF_DIR "/" SYSLOG_CONF_FILE
#define ConfInFile  "/tmp/tmpsyslogtest.conf"
#define ConfOutFile  "/tmp/syslogtest.conf"

#ifdef __cplusplus 
/*extern "C" {*/
#endif

#ifdef DEBUG
#define syslog_debug(fd, args...) fprintf(fd,args)
#else
#define syslog_debug(a,b...)
#endif
  
#define MAXSIZE    1025

struct _Syslog_Setting {
	char facility[256];
	char priority[256];
	char target[1025];
	};

typedef struct _Syslog_Setting Syslog_Setting;

struct _Syslog_File {
	char fname[1025];
	struct Syslog_Setting *record;
	};

typedef struct _Syslog_File Syslog_Configuration;

typedef enum {
	ACTION_CREATE,
	ACTION_DELETE
} SyslogParseAction;

void* Syslog_Conf_Begin_Enum();
void* Syslog_Setting_Begin_Enum();
int Syslog_Conf_Next_Enum(void *handle, Syslog_Configuration *cwsf);
int Syslog_Setting_Next_Enum(void *handle, Syslog_Setting *cwsf);
void Syslog_Conf_End_Enum(void *handle);
void Syslog_Setting_End_Enum(void *handle);
int Syslog_Setting_Get_Instance(Syslog_Setting *cfile, char *fac, char *prio, char *path);

int setting_read();
int setting_parse(const char *facility, const char *priority, const char *path, SyslogParseAction action);
int parse_list( const char *line );

void chomp (char *s);
#ifdef __cplusplus 
#endif

#endif
