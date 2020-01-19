/*
 * sysloglogutil.c
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "sysloglogutil.h"

typedef struct {
  FILE *fp;
  char name[L_tmpnam];
  
  char **values;
  unsigned long int pos;
} Syslog_Control;

/* Some helper funcs */
void releaseMsgLogBuf(Syslog_MessageLog *p)
{
	if (p->svName) {
		free(p->svName);
		p->svName=NULL;
	}
	if (p->svCCname) {
		free(p->svCCname);
		p->svCCname=NULL;
	}
}
                                                                                
void releaseLogRecBuf(Syslog_LogRecord *p)
{
	if (p->logCCname) {
		free(p->logCCname);
		p->logCCname=NULL;
	}
	if (p->logname) {
		free(p->logname);
		p->logname=NULL;
	}
	if (p->CCname) {
		free(p->CCname);
		p->CCname=NULL;
	}
	if (p->recordid) {
		free(p->recordid);
		p->recordid=NULL;
	}
	if (p->timestamp) {
		free(p->timestamp);
		p->timestamp=NULL;
	}
	if (p->hostname) {
		free(p->hostname);
		p->hostname=NULL;
	}
	if (p->data) {
		free(p->data);
		p->data=NULL;
	}
}

static void
strv_add_element (char ***values, unsigned long int *len, char *add)
{
  *values = realloc (*values, sizeof (char *) * (*len + 1));
  (*values)[*len] = add;
  (*len)++;
}

static void
parse_conf (const char *conf_file, Syslog_Control *cc, unsigned long int *len)
{
  char cmdbuffer[1024];
  char *buf;
  FILE *f;

  bzero(&cmdbuffer, sizeof(cmdbuffer));

  sprintf(cmdbuffer,
	  "files=(%s); cat \"${files[@]}\" | grep 'var/log' | awk '!/^#/ && !/^$/ {print $NF}' | sed 's/\\-//g'",
	  conf_file);
  f = popen (cmdbuffer, "r");
  if (f != NULL) {
    while (fscanf (f, "%ms", &buf) > 0)
      strv_add_element (&cc->values, len, buf);
    pclose (f);
  }
}

static void
parse_rsyslog_includes (const char *master_conf, Syslog_Control *cc, unsigned long int *len)
{
  char cmdbuffer[1024];
  char *buf;
  FILE *f;

  /* first find all $IncludeConfig directives */
  sprintf(cmdbuffer,
          "cat \"%s\" | grep '^$IncludeConfig' | awk '!/^#/ && !/^$/ {print $2}'",
          master_conf);
  f = popen (cmdbuffer, "r");
  if (f != NULL) {
    while (fscanf (f, "%ms", &buf) > 0) {
      parse_conf (buf, cc, len);
      free (buf);
    }
    pclose (f);
  }
}

void* Syslog_MessageLog_Begin_Enum(const char *name)
{
  /* begin enumeration */
  Syslog_Control *cc = malloc(sizeof(Syslog_Control));
  unsigned long int len = 0;

  syslog_debug(stderr, "--- Syslog_MessageLog_Begin_Enum() \n");

  if (cc == NULL)
    return NULL;
  bzero(cc, sizeof(Syslog_Control));

  parse_conf (name, cc, &len);

  if (strcasecmp (SYSLOG_DAEMON, "rsyslogd") == 0) {
    parse_rsyslog_includes (name, cc, &len);
  }

  strv_add_element (&cc->values, &len, NULL);

  return cc;
}


void* Syslog_LogRecord_Begin_Enum(const char *name)
{
  char cmdbuffer[1024];
  Syslog_Control *cc = malloc(sizeof(Syslog_Control));
  
  syslog_debug(stderr, "--- Syslog_LogRecord_Begin_Enum() \n");

  bzero(cmdbuffer, sizeof(cmdbuffer));

  getyear(name);
  // begin enumeration 
  if (cc && tmpnam(cc->name)) {
    sprintf(cmdbuffer,
	    "cat %s"
	    "> %s 2>/dev/null",
	    name, cc->name);
    if (system(cmdbuffer)==0)
      cc->fp = fopen(cc->name,"r");
    else {
      free(cc);
      cc=NULL;
    }
  }

  return cc;
}


int Syslog_MessageLog_Next_Enum(void *handle, Syslog_MessageLog* svc)
{
  Syslog_Control *cc = (Syslog_Control *)handle;
  int state = 0;

  syslog_debug(stderr, "--- Syslog_MessageLog_Next_Enum()  \n");

  svc->svName = NULL;

  if (cc && cc->values != NULL && cc->values[cc->pos] != NULL) {
    svc->svName = strdup (cc->values[cc->pos]);
    cc->pos++;
    state = 1;
  }

  return state;
}


int Syslog_LogRecord_Next_Enum(void *handle, Syslog_LogRecord* svc)
{
  char result[2000];
  Syslog_Control *cc = (Syslog_Control *)handle;
  int state = 0;
  char *p, *q = NULL;
  static int recordid = 0;
  char recbuf[10];
  char timestamp[30];
  char tmptimestamp[30];

  syslog_debug(stderr, "--- Syslog_LogRecord_Next_Enum() \n");

  bzero(&result, sizeof(result));
  bzero(&timestamp, sizeof(timestamp));
  bzero(&tmptimestamp, sizeof(tmptimestamp));
  bzero(&recbuf, sizeof(recbuf));

  // read next entry from result file 
  if (cc && svc && (fgets(result,sizeof(result),cc->fp) != NULL)) {
    if (!isalnum(result[0])) {
      syslog_debug(stdout, "Junk character in log record! quit reading log...\n");
      goto out;
    } 
    sprintf(recbuf, "%d", ++recordid);
    svc->recordid=strdup(recbuf);
    syslog_debug(stderr, "recordid:%s\n",svc->recordid);
    p = result;

    strncpy(tmptimestamp, result, 15);
    tmptimestamp[15]='\0';
    if ((timeformat(tmptimestamp, timestamp)) == 0)
      svc->timestamp=strdup(timestamp);
    p = p+16;
    q = strtok(p, " ");
    svc->hostname=strdup(q);
    q = strtok(NULL, "\n");
    svc->data=strdup(q);
    state = 1;
    return state;
  }		 
out: 
  recordid = 0; 

  return state;
}


void Syslog_MessageLog_End_Enum(void *handle)
{
  Syslog_Control *cc = (Syslog_Control *)handle;
  char **v;

  // end enumeration 
  if (cc) {
    if (cc->values) {
      for (v = cc->values; *v; v++)
        free (*v);
      free (cc->values);
    }
    free(cc);
  }
}


void Syslog_LogRecord_End_Enum(void *handle)
{
  Syslog_Control *cc = (Syslog_Control *)handle;
  // end enumeration 
  if (cc) {
    fclose(cc->fp);
    remove(cc->name);
    free(cc);
  }
}

