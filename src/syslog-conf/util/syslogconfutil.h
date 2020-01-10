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

#define LogConfFile  "/etc/syslog.conf"
#define ConfFileName  "syslog.conf"
#define TmpConfFile  "/etc/syslog.conf"
#define ConfInFile  "/tmp/tmpsyslogtest.conf"
#define ConfOutFile  "/tmp/syslogtest.conf"

#ifdef __cplusplus 
/*extern "C" {*/
#endif

#ifdef DEBUG
#define syslog_debug(a,b)       fprintf(a,b)
#else
#define syslog_debug(a,b);
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

void* Syslog_Conf_Begin_Enum();
void* Syslog_Setting_Begin_Enum();
int Syslog_Conf_Next_Enum(void *handle, Syslog_Configuration *cwsf);
int Syslog_Setting_Next_Enum(void *handle, Syslog_Setting *cwsf);
void Syslog_Conf_End_Enum(void *handle);
void Syslog_Setting_End_Enum(void *handle);
int Syslog_Setting_Get_Instance(Syslog_Setting *cfile, char *fac, char *prio, char *path);

int setting_read();
int setting_parse(char *facility, char *priority, char *path, char *action);
int parse_list( const char *line );
int parse_create(const char *line);
int parse_delete(const char *line);
int create_in_file(char *facility, char *priority, char *path);
int copy_to_file(char *buf);
#ifdef __cplusplus 
#endif

#endif
