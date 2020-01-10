/*
 * Syslog_MessageLog.c
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
 * Interface Type : Common Manageability Programming Interface ( CMPI )
 *
 * Description: CIM Linux Syslog Log Provider
 */


#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>
#include <string.h>

#include "Syslog_LogUtils.h"
#include "util/sysloglogutil.h"
#define LOCALCLASSNAME "Syslog_MessageLog"

static const CMPIBroker *_broker;

#ifdef CMPI_VER_100
#define Syslog_MessageLogSetInstance Syslog_MessageLogModifyInstance
#endif
/* Instance MI Cleanup */

CMPIStatus Syslog_MessageLogCleanup( CMPIInstanceMI *mi, 
                                    const CMPIContext *ctx,
				    CMPIBoolean terminate)
{
  CMReturn(CMPI_RC_OK);
}

/* Instance MI Functions */

CMPIStatus Syslog_MessageLogEnumInstanceNames( CMPIInstanceMI *mi, 
                                              const CMPIContext *ctx, 
                                              const CMPIResult *rslt, 
                                              const CMPIObjectPath *ref) 
{
  CMPIObjectPath *op = NULL;
  CMPIStatus st = {CMPI_RC_OK,NULL};
  void *enumhdl = NULL;
  Syslog_MessageLog logbuf;

  syslog_debug(stderr,"--- Syslog_MessageLogEnumInstanceNames() \n");

  bzero(&logbuf, sizeof(Syslog_MessageLog));

  enumhdl = Syslog_MessageLog_Begin_Enum(SYSLOG_CONFPATH);
  if (enumhdl == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			 "Could not do syslog messagelog enumeration");
    return st;
  } else {
    while (Syslog_MessageLog_Next_Enum(enumhdl,&logbuf)) {
      // build messagelog object path from buffer 
      op = makeMessageLogPath(_broker,
                    LOCALCLASSNAME,
                    CMGetCharPtr(CMGetNameSpace(ref,NULL)),
                    &logbuf);
      if (CMIsNullObject(op)) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not construct object path");
        break;
      }
      CMReturnObjectPath(rslt,op);
    }
    CMReturnDone(rslt);
    Syslog_MessageLog_End_Enum(enumhdl);
  }

  return st;
}

CMPIStatus Syslog_MessageLogEnumInstances( CMPIInstanceMI *mi, 
                                          const CMPIContext *ctx, 
                                          const CMPIResult *rslt, 
                                          const CMPIObjectPath *ref, 
                                          const char **properties) 
{
  CMPIInstance *in = NULL; 
  CMPIStatus st = {CMPI_RC_OK,NULL};
  void *enumhdl = NULL;
  Syslog_MessageLog logbuf;
  
  syslog_debug(stderr,"--- Syslog_MessageLogEnumInstances() \n");

  bzero(&logbuf, sizeof(Syslog_MessageLog));

  enumhdl = Syslog_MessageLog_Begin_Enum(SYSLOG_CONFPATH);
  if (enumhdl == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			 "Could not begin syslog messagelog enumeration");
    return st;
  } else {
    while (Syslog_MessageLog_Next_Enum(enumhdl,&logbuf)) {
      // build messagelog object instance from buffer 
      in = makeMessageLogInstance(_broker, 
                                  LOCALCLASSNAME,
                                  CMGetCharPtr(CMGetNameSpace(ref,NULL)), 
                                  &logbuf);
      if (CMIsNullObject(in)) {
	CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			     "Could not construct instance");
	goto label;
      }
      CMReturnInstance(rslt,in);
    }
label:
    CMReturnDone(rslt);
    Syslog_MessageLog_End_Enum(enumhdl);
  }

  return st;
}

CMPIStatus Syslog_MessageLogGetInstance( CMPIInstanceMI *mi, 
                                        const CMPIContext *ctx, 
                                        const CMPIResult *rslt, 
                                        const CMPIObjectPath *cop, 
                                        const char **properties) 
{
  CMPIInstance *in = NULL;
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIData nd;
  Syslog_MessageLog logbuf;
  char *name = NULL;

  syslog_debug(stderr,"--- Syslog_MessageLogGetInstance() \n");

  bzero(&logbuf, sizeof(Syslog_MessageLog));

  nd = CMGetKey(cop, "Name", &st); 
  name = CMGetCharPtr(nd.value.string);
  logbuf.svName=strdup(name);
  if (st.rc == CMPI_RC_OK &&
      nd.type == CMPI_string) 
    // build messagelog object instance from buffer
    in = makeMessageLogInstance(_broker, 
                                LOCALCLASSNAME,
                                CMGetCharPtr(CMGetNameSpace(cop,NULL)), 
                                &logbuf);
  releaseMsgLogBuf(&logbuf); 
  if (CMIsNullObject(in)) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			 "Could not find or construct instance");
  } else {
    CMReturnInstance(rslt,in);    
    CMReturnDone(rslt);
  }

  return st;
}

CMPIStatus Syslog_MessageLogCreateInstance( CMPIInstanceMI *mi, 
                                           const CMPIContext *ctx, 
                                           const CMPIResult *rslt, 
                                           const CMPIObjectPath *cop, 
                                           const CMPIInstance *ci) 
{
  syslog_debug(stderr,"--- Syslog_MessageLogCreateInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_MessageLogSetInstance( CMPIInstanceMI *mi, 
                                        const CMPIContext *ctx, 
                                        const CMPIResult *rslt, 
                                        const CMPIObjectPath *cop,
                                        const CMPIInstance *ci, 
                                        const char **properties) 
{
  syslog_debug(stderr,"--- Syslog_MessageLogSetInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_MessageLogDeleteInstance( CMPIInstanceMI *mi, 
                                           const CMPIContext *ctx, 
                                           const CMPIResult *rslt, 
                                           const CMPIObjectPath *cop) 
{ 
  syslog_debug(stderr,"--- Syslog_MessageLogDeleteInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_MessageLogExecQuery( CMPIInstanceMI *mi, 
                                      const CMPIContext *ctx, 
                                      const CMPIResult *rslt, 
                                      const CMPIObjectPath *cop, 
                                      const char *lang, 
                                      const char *query) 
{
  syslog_debug(stderr,"--- Syslog_MessageLogExecQuery() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

/* Instance MI Factory */

CMInstanceMIStub( Syslog_MessageLog,
                  Syslog_MessageLog,
                  _broker,
                  CMNoHook);

