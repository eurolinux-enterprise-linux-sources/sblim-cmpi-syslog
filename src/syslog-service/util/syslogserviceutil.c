/*
 * syslogserviceutil.c
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "syslogserviceutil.h"

typedef struct {
  FILE *fp;
  char name[L_tmpnam];
} Syslog_Control;


void* Syslog_Service_Begin_Enum()
{
  char cmdbuffer[2000];
  Syslog_Control *cc = malloc(sizeof(Syslog_Control));

  bzero(&cmdbuffer, sizeof(cmdbuffer));

  // begin enumeration
  if (cc && tmpnam(cc->name)) {
    sprintf(cmdbuffer, "syslog-service.sh status"
            "> %s", cc->name);
    if (system(cmdbuffer)==0)
      cc->fp = fopen(cc->name,"r");
    else {
      free(cc);
      cc=NULL;
    }
  }

  return cc;
}

int Syslog_Service_Next_Enum(void *handle, Syslog_Service* svc)
{
  char result[2000];
  char svname[256]; // temporary place holder for service name
  int pid = 0;
  Syslog_Control *cc = (Syslog_Control *)handle;
  int state = 0, ret = 0;

  bzero(&result, sizeof(result));
  bzero(&svname, sizeof(svname));

  // read next entry from result file
  if (cc && svc )
  {
    while ( fgets(result,sizeof(result),cc->fp) != NULL) {
      if (strncmp(result, "stopped", 7) == 0) {
        svc->syslogd = 0;
        svc->klogd = 0;
	ret = 1;
      }
      else {
        state=sscanf(result,"%d %s",
                     &pid,
                     svname);
        if (strcmp(svname, "syslogd")==0) svc->syslogd = pid;
        if (strcmp(svname, "klogd")==0) svc->klogd = pid;
        if (state) ret = 1;
      } 
      svc->svName = strdup("syslog");
      if (svc && svc->syslogd && svc->klogd) {
        svc->svStarted = 1;
        svc->svStatus = strdup("OK");
      }
      else {
        svc->svStarted = 0;
        svc->svStatus = strdup("Stopped");
      }
    }
  }

  return ret;
}

void Syslog_Service_End_Enum(void *handle)
{
  Syslog_Control *cc = (Syslog_Control *)handle;

  // end enumeration
  if (cc) {
    fclose(cc->fp);
    remove(cc->name);
    free(cc);
  }
}

int Syslog_Service_Operation(const char *method, char *result, int resultlen)
{
 char cmdbuffer[300];
 char cmdout[300];
 FILE *fcmdout = NULL;
 char *op = NULL;
 
 bzero(&cmdbuffer, sizeof(cmdbuffer));
 bzero(&cmdout, sizeof(cmdout));

 // RedHat and SuSE common methods
 if (!strcasecmp(method,"startservice"))
   op="start";
 else if (!strcasecmp(method,"stopservice"))
   op="stop";
 else if (!strcasecmp(method,"restartservice"))
   op="restart";
 else if (!strcasecmp(method,"reloadservice"))
   op="reload";
 // RedHat specific method
 else if (!strcasecmp(method,"condrestartservice"))
   op="condrestart";
 // SuSE specific methods
 else if (!strcasecmp(method,"forcereloadservice"))
   op="force-reload";
 else if (!strcasecmp(method,"tryrestartservice"))
   op="try-restart";
 else if (!strcasecmp(method,"probeservice"))
   op="probe";
 else
   return -1;
 if (op && tmpnam(cmdout)) {
   sprintf(cmdbuffer, "syslog-service.sh %s > %s", op, cmdout);
   if (system(cmdbuffer)==0 && (fcmdout = fopen(cmdout,"r")) &&
       fgets(result,resultlen,fcmdout)) {
     return 0;
   }		 
 } 

 return 1;
}


