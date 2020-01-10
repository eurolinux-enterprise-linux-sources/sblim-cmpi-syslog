/*
 * Syslog_LogRecord.c
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
#include <string.h>

#include "Syslog_LogUtils.h"
#include "util/sysloglogutil.h"
#define LOCALCLASSNAME "Syslog_LogRecord"

static const CMPIBroker *_broker;

#ifdef CMPI_VER_100
#define Syslog_LogRecordSetInstance Syslog_LogRecordModifyInstance
#endif

/* Instance MI Cleanup */

CMPIStatus Syslog_LogRecordCleanup( CMPIInstanceMI *mi, 
                                   const CMPIContext *ctx,
				   CMPIBoolean terminate)
{
  CMReturn(CMPI_RC_OK);
}

/* Instance MI Functions */

CMPIStatus Syslog_LogRecordEnumInstanceNames( CMPIInstanceMI *mi, 
                                             const CMPIContext *ctx, 
                                             const CMPIResult *rslt, 
                                             const CMPIObjectPath *ref) 
{
  CMPIObjectPath *op = NULL;
  CMPIStatus st = {CMPI_RC_OK,NULL};
  void *enumhdl1 = NULL;
  void *enumhdl2 = NULL;
  Syslog_MessageLog logbuf;
  Syslog_LogRecord logrecbuf;

  syslog_debug(stderr,"--- Syslog_LogRecordEnumInstanceNames() \n");

  bzero(&logbuf, sizeof(Syslog_MessageLog));
  bzero(&logrecbuf, sizeof(Syslog_LogRecord));

  enumhdl1 = Syslog_MessageLog_Begin_Enum(SYSLOG_CONFPATH);
  if (enumhdl1 == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                         "Could not do syslog messagelog enumeration");
    return st;
  } else {
    while (Syslog_MessageLog_Next_Enum(enumhdl1,&logbuf)) {
      // enumerate the log records for the given log file 
      logrecbuf.logname=strdup(logbuf.svName); 
      enumhdl2 = Syslog_LogRecord_Begin_Enum(logbuf.svName);
      while (Syslog_LogRecord_Next_Enum(enumhdl2, &logrecbuf)) {
        // build messagelog object path from buffer
	logrecbuf.logname=strdup(logbuf.svName);
        op = makeLogRecordPath(_broker, 
                               LOCALCLASSNAME, 
                               CMGetCharPtr(CMGetNameSpace(ref,NULL)), 
                               &logrecbuf);
	releaseLogRecBuf(&logrecbuf);

        if (CMIsNullObject(op)) {
          CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                               "Could not construct object path");
          return st;
        }
        CMReturnObjectPath(rslt,op);
      }	
      Syslog_LogRecord_End_Enum(enumhdl2);
      releaseMsgLogBuf(&logbuf);
    }
    Syslog_MessageLog_End_Enum(enumhdl1);
  }
  CMReturnDone(rslt);

  return st;
}

CMPIStatus Syslog_LogRecordEnumInstances( CMPIInstanceMI *mi, 
                                         const CMPIContext *ctx, 
                                         const CMPIResult *rslt, 
                                         const CMPIObjectPath *ref, 
                                         const char **properties) 
{
  CMPIInstance *in = NULL;
  CMPIStatus st = {CMPI_RC_OK,NULL};
  void *enumhdl1 = NULL;
  void *enumhdl2 = NULL;
  Syslog_MessageLog logbuf;
  Syslog_LogRecord logrecbuf;

  syslog_debug(stderr,"--- Syslog_LogRecordEnumInstances() \n");

  bzero(&logbuf, sizeof(Syslog_MessageLog));
  bzero(&logrecbuf, sizeof(Syslog_LogRecord));

  enumhdl1 = Syslog_MessageLog_Begin_Enum(SYSLOG_CONFPATH);
  if (enumhdl1 == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                         "Could not do syslog messagelog enumeration");
    return st;
  } else {
    while (Syslog_MessageLog_Next_Enum(enumhdl1,&logbuf)) {
      // enumerate the log records for the given log file 
      logrecbuf.logname=strdup(logbuf.svName);
      enumhdl2 = Syslog_LogRecord_Begin_Enum(logbuf.svName);
      while (Syslog_LogRecord_Next_Enum(enumhdl2, &logrecbuf)) {
        logrecbuf.logname=strdup(logbuf.svName);
        // build messagelog object instance from buffer
        in = makeLogRecordInstance(_broker,
                                   LOCALCLASSNAME,
                                   CMGetCharPtr(CMGetNameSpace(ref,NULL)),
                                   &logrecbuf);

	releaseLogRecBuf(&logrecbuf);

        if (CMIsNullObject(in)) {
          CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                               "Could not construct object instance");
          return st;
        }
        CMReturnInstance(rslt,in);
      }
      Syslog_LogRecord_End_Enum(enumhdl2);
      releaseMsgLogBuf(&logbuf);
    }
    Syslog_MessageLog_End_Enum(enumhdl1);
  }
  CMReturnDone(rslt);

  return st;
}

CMPIStatus Syslog_LogRecordGetInstance( CMPIInstanceMI *mi, 
                                       const CMPIContext *ctx, 
                                       const CMPIResult *rslt, 
                                       const CMPIObjectPath *cop, 
                                       const char **properties) 
{
  CMPIInstance *in = NULL;
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIData nd;  
  char *logname = NULL;
  char *recordid = NULL;
  char *timestamp = NULL;
  Syslog_LogRecord logrecbuf;
  void *enumhdl = NULL;

  syslog_debug(stderr,"--- Syslog_LogRecordGetInstance() \n");

  bzero(&logrecbuf, sizeof(Syslog_LogRecord));

  nd = CMGetKey(cop, "LogName", &st);
  logname = CMGetCharPtr(nd.value.string);

  nd = CMGetKey(cop, "RecordId", &st);
  recordid = CMGetCharPtr(nd.value.string);

  nd = CMGetKey(cop, "MessageTimestamp", &st);
  timestamp = CMGetCharPtr(nd.value.string);
	  
  logrecbuf.logname=strdup(logname);
  enumhdl = Syslog_LogRecord_Begin_Enum(logname);
  while (Syslog_LogRecord_Next_Enum(enumhdl, &logrecbuf)) {
    logrecbuf.logname=strdup(logname);
    if (strncmp(recordid, logrecbuf.recordid, strlen(recordid))==0 && 
        strncmp(timestamp,logrecbuf.timestamp,strlen(timestamp))==0) {	  
      // build logrecord object instance from buffer	    
      in = makeLogRecordInstance(_broker,
                                 LOCALCLASSNAME,
                                 CMGetCharPtr(CMGetNameSpace(cop,NULL)),
                                 &logrecbuf);
      releaseLogRecBuf(&logrecbuf);
      if (CMIsNullObject(in)) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not construct object instance");
        return st;
      }
      CMReturnInstance(rslt,in);
    } // if
  } // while
  Syslog_LogRecord_End_Enum(enumhdl);
  CMReturnDone(rslt);

  return st;
}

CMPIStatus Syslog_LogRecordCreateInstance( CMPIInstanceMI *mi, 
                                          const CMPIContext *ctx, 
                                          const CMPIResult *rslt, 
                                          const CMPIObjectPath *cop, 
                                          const CMPIInstance *ci) 
{
  syslog_debug(stderr,"--- Syslog_LogRecordCreateInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_LogRecordSetInstance( CMPIInstanceMI *mi, 
                                       const CMPIContext *ctx, 
                                       const CMPIResult *rslt, 
                                       const CMPIObjectPath *cop,
                                       const CMPIInstance *ci, 
                                       const char **properties) 
{
  syslog_debug(stderr,"--- Syslog_LogRecordSetInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_LogRecordDeleteInstance( CMPIInstanceMI *mi, 
                                          const CMPIContext *ctx, 
                                          const CMPIResult *rslt, 
                                          const CMPIObjectPath *cop) 
{ 
  syslog_debug(stderr,"--- Syslog_LogRecordDeleteInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_LogRecordExecQuery( CMPIInstanceMI *mi, 
                                     const CMPIContext *ctx, 
                                     const CMPIResult *rslt, 
                                     const CMPIObjectPath *cop, 
                                     const char *lang, 
                                     const char *query) 
{
  syslog_debug(stderr,"--- Syslog_LogRecordExecQuery() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

/* Instance MI Factory */

CMInstanceMIStub( Syslog_LogRecord,
                  Syslog_LogRecord,
                  _broker,
                  CMNoHook);

