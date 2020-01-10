/*
 * Syslog_LogUtils.c
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
 * Description: CIM Linux Syslog Log Provider
 */


#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>
#include <unistd.h>
#include <string.h>

#include "Syslog_LogUtils.h"
#include "util/sysloglogutil.h"

static char mlccn[] = "Syslog_MessageLog";
static char lrccn[] = "Syslog_LogRecord";

char *MessageLogCreationClassName()
{
  return mlccn;
}

char *LogRecordCreationClassName()
{
  return lrccn;
}

CMPIObjectPath *makeMessageLogPath( const CMPIBroker *broker, 
                                   const char *classname, 
                                   const char *namespace, 
                                   Syslog_MessageLog *svc)
{
  CMPIObjectPath *op = NULL;

  syslog_debug(stderr,"--- makeMessageLogPath() \n");

  op = CMNewObjectPath(broker, 
                       (char*)namespace,
                       (char*)classname,
                       NULL);
  if (!CMIsNullObject(op)) {
    CMAddKey(op,"CreationClassName",MessageLogCreationClassName(),CMPI_chars); 
    CMAddKey(op,"Name",svc->svName,CMPI_chars); 
  }

  return op;
}

CMPIObjectPath *makeLogRecordPath( const CMPIBroker *broker, 
                                  const char *classname, 
                                  const char *namespace, 
                                  Syslog_LogRecord *svc)
{
  CMPIObjectPath *op = NULL;
  CMPIValue val;
  
  syslog_debug(stderr,"--- makeLogRecordPath() \n");

  op = CMNewObjectPath(broker, 
                       (char*)namespace,
                       (char*)classname,
                       NULL);
  if (!CMIsNullObject(op)) {
    CMAddKey(op,"LogCreationClassName",MessageLogCreationClassName(),CMPI_chars); 
    CMAddKey(op,"LogName",svc->logname,CMPI_chars); 
    CMAddKey(op,"CreationClassName",LogRecordCreationClassName(),CMPI_chars); 
    CMAddKey(op,"RecordId",svc->recordid,CMPI_chars); 
    val.chars = svc->timestamp;
    val.dateTime = CMNewDateTimeFromChars(broker, val.chars, NULL);
    CMAddKey(op,"MessageTimeStamp",&val,CMPI_dateTime); 
  }

  return op;
}

CMPIInstance *makeMessageLogInstance( const CMPIBroker *broker, 
                                     const char *classname, 
                                     const char *namespace, 
                                     Syslog_MessageLog *svc)
{
  CMPIInstance *in = NULL;
  CMPIObjectPath *op = CMNewObjectPath(broker, 
                                       (char*)namespace,
                                       (char*)classname,
                                       NULL);
  
  syslog_debug(stderr, "--- makeMessageLogInstance() \n");

  if (!CMIsNullObject(op)) {
    in = CMNewInstance(broker,op,NULL);
    if (!CMIsNullObject(in)) {
      CMSetProperty(in,"CreationClassName",MessageLogCreationClassName(),CMPI_chars);
      CMSetProperty(in,"Name",svc->svName,CMPI_chars);
    }
  }

  return in;
}

CMPIInstance *makeLogRecordInstance( const CMPIBroker *broker, 
                                    const char *classname, 
                                    const char *namespace, 
                                    Syslog_LogRecord *svc)
{
  CMPIInstance *in = NULL;
  CMPIValue val;
  CMPIObjectPath *op = CMNewObjectPath(broker, 
                                       (char*)namespace,
                                       (char*)classname,
                                       NULL);
  
  syslog_debug(stderr, "--- makeLogRecordInstance() \n");

  if (!CMIsNullObject(op)) {
    in = CMNewInstance(broker,op,NULL);
    if (!CMIsNullObject(in)) {
      CMSetProperty(in,"LogCreationClassName",MessageLogCreationClassName(),CMPI_chars);
      CMSetProperty(in,"LogName",svc->logname,CMPI_chars);
      CMSetProperty(in,"CreationClassName",LogRecordCreationClassName(),CMPI_chars);
      CMSetProperty(in,"RecordId",svc->recordid,CMPI_chars);
      val.chars = svc->timestamp;
      val.dateTime = CMNewDateTimeFromChars(broker, val.chars, NULL);
      CMSetProperty(in,"MessageTimestamp",&val,CMPI_dateTime);
      CMSetProperty(in,"HostName",svc->hostname,CMPI_chars);
      CMSetProperty(in,"DataFormat",svc->data,CMPI_chars);
    }
  }

  return in;
}
