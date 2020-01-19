/*
 * syslogtimeparse.c
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
 * Description: Time parse utility functions for CIM Linux Syslog Log Provider
 */

                                                                               
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
                                                                                
#include "sysloglogutil.h"

struct monst {
  char name[4];
  char num[3];
};
                                                                                
struct monst months[12] = {{"Jan\0","01\0"},{"Feb\0","02\0"},{"Mar\0","03\0"},
                           {"Apr\0","04\0"},{"May\0","05\0"},{"Jun\0","06\0"},
			   {"Jul\0","07\0"},{"Aug\0","08\0"},{"Sep\0","09\0"},
			   {"Oct\0","10\0"},{"Nov\0","11\0"},{"Dec\0","12\0"}};


void getyear(const char *name)
{
  struct tm *tp = NULL;
  struct stat statbuf;
  time_t timep;
  char logname[50]; 
  int localdate, localmonth, localyear;
  int logdate, logmonth;
  int tmpyear = 0;
  int year = 1900;

  syslog_debug(stderr, "--- getyear() \n");

  bzero(&finalyear, sizeof(finalyear));
  bzero(&logname, sizeof(logname));
  bzero(&timep, sizeof(time_t));
  bzero(&statbuf, sizeof(struct stat));

  time(&timep);
  tp = localtime(&timep);
  localdate = tp->tm_mday;
  localmonth = tp->tm_mon;
  localyear = tp->tm_year;
  tmpyear = localyear + year;

  tp = NULL;
  strcpy(logname, name);
  logname[strlen(logname)] = '\0';

  stat(logname, &statbuf);
  tp = localtime(&(statbuf.st_atime));
  logmonth = tp->tm_mon;
  logdate = tp->tm_mday;

  if (( logmonth > localmonth) && (logdate > localdate))
    tmpyear = tmpyear - 1;

  snprintf(finalyear, sizeof(finalyear), "%d", tmpyear);

  return;
}

int timeformat(const char *itimebuf, char *otimebuf)
{
  char timestamp[30];
  char *p, *q, *tmp = NULL;
  int single, i = 0;

  syslog_debug(stderr, "--- timeformat() \n");

  bzero(&timestamp, sizeof(timestamp));
  
  strcpy(timestamp, itimebuf);

  tmp = strdup((char *)itimebuf);
  p = strtok(tmp, " ");
  strcpy((char *)otimebuf, (const char *)finalyear);
  for (i=0;i<12;i++) {
    if (strcmp(months[i].name, p) == 0) {
      strcat((char *)otimebuf, (const char *)months[i].num);
      break;
    }
  }
  free(tmp);
  tmp = NULL;
  p = NULL;

  p = timestamp + 4;
  q = strtok(p, " ");
  single = atoi(q);
  if ( single >0 && single <= 9 )
    strcat((char *)otimebuf,(const char *)"0");
  strcat((char *)otimebuf,(const char *)q);

  p = timestamp + 7;
  i = 0;
  while(i<3) {
    q = strtok(p, ":");
    strcat((char *)otimebuf,(const char *)q);
    p = NULL;
    i++;
  }
  strcat((char *)otimebuf,(const char *)".000000+000");

  return 0;
}
