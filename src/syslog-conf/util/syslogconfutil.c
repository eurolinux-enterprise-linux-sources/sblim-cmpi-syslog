/*
 * syslogconfutil.c
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "syslogconfutil.h"

typedef struct {
  FILE *fp;
  char name[L_tmpnam];
} Syslog_Control;

void* Syslog_Conf_Begin_Enum()
{
  char cmdbuffer[1024];
  Syslog_Control *cc = malloc(sizeof(Syslog_Control));

  syslog_debug(stderr, "--- Syslog_Conf_Begin_Enum() \n");
  
  bzero(&cmdbuffer, sizeof(cmdbuffer));

  // begin enumeration
  if (cc && tmpnam(cc->name)) {
    sprintf(cmdbuffer,
	     "find /etc -maxdepth 1 -name %s"
             " > %s",
             ConfFileName, cc->name);
    if (system (cmdbuffer) != 0 ||
        (cc->fp = fopen(cc->name,"r")) == NULL) {
      free(cc);
      cc=NULL;
    }
  }

  return cc;
}

void* Syslog_Setting_Begin_Enum()
{
  Syslog_Control *cc = malloc(sizeof(Syslog_Control));
  int ret = 0;

  syslog_debug(stderr, "--- Syslog_Setting_Begin_Enum() \n");

  // begin enumeration
  ret = setting_read();	  
  if (!ret && (cc->fp = fopen(ConfInFile, "r")) != NULL) {
    strcpy(cc->name, ConfInFile);	    
  }
  else {
    syslog_debug(stderr, "Unable to sort the file\n");
    free(cc);
    cc=NULL;
   }

  return cc;
}

int Syslog_Conf_Next_Enum(void *handle, Syslog_Configuration* cfile)
{
  char result[1024];
  Syslog_Control *cc = (Syslog_Control *)handle;
  int state = 0;

  syslog_debug(stderr, "--- Syslog_Conf_Next_Enum() \n");

  bzero(&result, sizeof(result));
  
  // read next entry from result file 
  if (cc && cfile && cc->fp && fgets(result,sizeof(result),cc->fp)) {
    state=0<sscanf(result,"%s ",
                   cfile->fname);
  }

  return state;
}

void chomp (char *s)
{
  int l = s ? strlen (s) : 0;
  if (l >= 0 && s[l - 1] == '\n')
    s[l - 1] = '\0';
}

int Syslog_Setting_Next_Enum(void *handle, Syslog_Setting* cfile)
{
  Syslog_Control *cc = (Syslog_Control *)handle;
  char facility[255], priority[255], pathbuf[1024];

  syslog_debug(stderr, "--- Syslog_Setting_Next_Enum() \n");

  if (cfile == NULL || cc->fp == NULL)
    return 0;

  bzero(&facility, sizeof(facility));
  bzero(&priority, sizeof(priority));
  bzero(&pathbuf, sizeof(pathbuf));

  // read next entry from result file
  if (!fgets (facility, sizeof (facility), cc->fp))
    return 0;
  chomp (facility);
  strncpy (cfile->facility, facility, sizeof (facility));
  if (!fgets (priority, sizeof (priority), cc->fp))
    return 0;
  chomp (priority);
  strncpy (cfile->priority, priority, sizeof (priority));
  if (!fgets (pathbuf, sizeof (pathbuf), cc->fp))
    return 0;
  chomp (pathbuf);
  strncpy (cfile->target, pathbuf, sizeof (pathbuf));

  return 1;
}

void Syslog_Conf_End_Enum(void *handle)
{
  Syslog_Control *cc = (Syslog_Control *)handle;

  // end enumeration 
  if (cc) {
    if (cc->fp)
      fclose(cc->fp);
    remove(cc->name);
    free(cc);
  }
}

void Syslog_Setting_End_Enum(void *handle)
{
  Syslog_Control *cc = (Syslog_Control *)handle;

  // end enumeration 
  if (cc) {
    if (cc->fp)
      fclose(cc->fp);
    remove(cc->name);
    free(cc);
  }
}

int Syslog_Setting_Get_Instance(Syslog_Setting *cfile, char *fac, char *prio, char *path)
{
  void *enumhdl = NULL;
  Syslog_Control *cc = NULL;
  char facility[255], priority[255], pathbuf[1024];
  char result[1024];
  int st = 0;
	 
  syslog_debug(stderr, "--- Syslog_Setting_Get_Instance() \n");

  bzero(&result, sizeof(result));
  bzero(&facility, sizeof(facility));
  bzero(&priority, sizeof(priority));
  bzero(&pathbuf, sizeof(pathbuf));

  enumhdl = Syslog_Setting_Begin_Enum();
  cc = (Syslog_Control *)enumhdl;
  if (enumhdl == NULL || cfile == NULL || cc->fp == NULL) {
    syslog_debug(stderr, "Could not begin syslog setting enumeration\n");
    return st;
  } else {
    while (fgets (facility, sizeof (facility), cc->fp) &&
           fgets (priority, sizeof (priority), cc->fp) &&
           fgets (pathbuf, sizeof (pathbuf), cc->fp)) {
      chomp (facility);
      chomp (priority);
      chomp (pathbuf);
      if (!strcmp(fac, facility) && !strcmp(prio, priority)
          && !strcmp(path, pathbuf)) {
        st = 1;
        strncpy (cfile->facility, facility, sizeof (facility));
        strncpy (cfile->priority, priority, sizeof (priority));
        strncpy (cfile->target, pathbuf, sizeof (pathbuf));
        Syslog_Setting_End_Enum(enumhdl);
        return st;
      }
    }
  }
  Syslog_Setting_End_Enum(enumhdl);

  return st;
}	

