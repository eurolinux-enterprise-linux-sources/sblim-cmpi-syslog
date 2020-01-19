/*
 * Syslog_RecordInLog.c
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
#include <libgen.h>
#include <stdio.h>

#include "Syslog_LogUtils.h"
#include "util/sysloglogutil.h"

#define LOCALCLASSNAME "Syslog_RecordInLog"
#define MESSAGELOGCLASSNAME "Syslog_MessageLog"
#define LOGRECORDCLASSNAME "Syslog_LogRecord"

static const CMPIBroker *_broker;

#ifdef CMPI_VER_100
#define Syslog_RecordInLogSetInstance Syslog_RecordInLogModifyInstance
#endif

static CMPIStatus returnAssocName( const CMPIResult *rslt,
                                  const CMPIObjectPath *cop,
                                  char *clsname,
                                  char *fname)
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIObjectPath *op = NULL;
  Syslog_MessageLog logbuf;
  Syslog_LogRecord logrecbuf;
  void *enumhdl = NULL;

  syslog_debug(stderr, "--- returnAssocName() \n");

  bzero(&logbuf, sizeof(Syslog_MessageLog));
  bzero(&logrecbuf, sizeof(Syslog_LogRecord));

  if (strcasecmp(MESSAGELOGCLASSNAME,clsname)==0 && fname) {
    // build messagelog object path from buffer 
    logbuf.svName=strdup(fname);
    op = makeMessageLogPath(_broker, clsname, CMGetCharPtr(CMGetNameSpace(cop,NULL)), &logbuf);
    releaseMsgLogBuf(&logbuf);
    if (CMIsNullObject(op)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not construct object path");
      return st;
    }
    CMReturnObjectPath(rslt,op);
  } // if clsname == MESSAGELOGCLASSNAME 

  else if (strcasecmp(LOGRECORDCLASSNAME,clsname)==0 && fname) {
    logrecbuf.logname=strdup(fname);
    enumhdl = Syslog_LogRecord_Begin_Enum(fname);
    while (Syslog_LogRecord_Next_Enum(enumhdl, &logrecbuf)) {
      // build logrecord object path from buffer
      op = makeLogRecordPath(_broker, 
                             clsname, 
                             CMGetCharPtr(CMGetNameSpace(cop,NULL)), 
                             &logrecbuf);
      if (CMIsNullObject(op)) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not construct object path");
	releaseLogRecBuf(&logrecbuf);
        return st;
      }
      CMReturnObjectPath(rslt,op);
    } // end while 
    Syslog_LogRecord_End_Enum(enumhdl);
    releaseLogRecBuf(&logrecbuf);
  } // if clsname == LOGRECORDCLASSNAME 

  else {
    char mesg[1024];
    sprintf(mesg, "--- unsupported class %s\n",CMGetCharPtr(CMGetClassName(cop,NULL)));
    syslog_debug(stderr, mesg);
  } // else unsupported class 

  return st;
}   

static CMPIStatus returnRefName( const CMPIResult *rslt,
                                const CMPIObjectPath *cop,
                                char *clsname,
                                char *fname)
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIObjectPath *op = NULL;
  CMPIObjectPath *opRef = NULL;
  Syslog_MessageLog logbuf;
  Syslog_LogRecord logrecbuf;
  void *enumhdl = NULL;

  syslog_debug(stderr, "--- returnRefName() \n");

  bzero(&logbuf, sizeof(Syslog_MessageLog));
  bzero(&logrecbuf, sizeof(Syslog_LogRecord));

  if (strcasecmp(MESSAGELOGCLASSNAME,clsname)==0) {
    // build messagelog object path from buffer
    logbuf.svName=strdup(fname);
    op = makeMessageLogPath(_broker, 
                            clsname, 
                            CMGetCharPtr(CMGetNameSpace(cop,NULL)), 
                            &logbuf);
    releaseMsgLogBuf(&logbuf);
    if (CMIsNullObject(op)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not construct object path");
      CMReturnObjectPath(rslt,op);
      return st;
    }
    // make reference object path 
    opRef = CMNewObjectPath(_broker,
                            CMGetCharPtr(CMGetNameSpace(cop,&st)),
                            LOCALCLASSNAME,
                            &st);
    if (CMIsNullObject(opRef)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not construct object path");
      return st;
    }
    CMAddKey(opRef,"MessageLog",&op,CMPI_ref);
    CMAddKey(opRef,"LogRecord",&cop,CMPI_ref);
    CMReturnObjectPath(rslt,opRef);
  } // if clsname == MESSAGELOGCLASSNAME 

  else if (strcasecmp(clsname, LOGRECORDCLASSNAME)==0) {
    logrecbuf.logname=strdup(fname);
    enumhdl = Syslog_LogRecord_Begin_Enum(fname);
    while (Syslog_LogRecord_Next_Enum(enumhdl, &logrecbuf)) {
      // build logrecord object path from buffer
      op = makeLogRecordPath(_broker,
                             clsname,
			     CMGetCharPtr(CMGetNameSpace(cop,NULL)),
			     &logrecbuf);
      if (CMIsNullObject(op)) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not construct object path");
         releaseLogRecBuf(&logrecbuf);
        return st;
      }
      // make reference object path 
      opRef = CMNewObjectPath(_broker,
                              CMGetCharPtr(CMGetNameSpace(cop,&st)),
                              LOCALCLASSNAME,
                              &st);
      if (CMIsNullObject(opRef)) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not construct object path");
        releaseLogRecBuf(&logrecbuf);
        return st;
      }
      CMAddKey(opRef,"MessageLog",&cop,CMPI_ref);
      CMAddKey(opRef,"LogRecord",&op,CMPI_ref);
      CMReturnObjectPath(rslt,opRef);
    } // end of while 
    Syslog_LogRecord_End_Enum(enumhdl);
    releaseLogRecBuf(&logrecbuf);
  } // else if clsname == LOGRECORDCLASSNAME 
                                                                                
  else {
     char mesg[1024];
     sprintf(mesg, "--- unsupported class %s\n", CMGetCharPtr(CMGetClassName(cop,NULL)));
     syslog_debug(stderr, mesg);
  } // else unsupported class 

  return st;
}  

static CMPIStatus returnAssoc( const CMPIResult *rslt,
                              const CMPIObjectPath *cop,
                              char *clsname,
                              char *fname)
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIInstance *in = NULL;
  Syslog_MessageLog logbuf;
  Syslog_LogRecord logrecbuf;
  void *enumhdl = NULL;

  syslog_debug(stderr, "--- returnAssoc() \n"); 

  bzero(&logbuf, sizeof(Syslog_MessageLog));
  bzero(&logrecbuf, sizeof(Syslog_LogRecord));

  if (strcasecmp(MESSAGELOGCLASSNAME,clsname)==0 && fname) {
    // build messagelog object instance from buffer 
    logbuf.svName=strdup(fname);
    in = makeMessageLogInstance(_broker,
                                clsname,
                                CMGetCharPtr(CMGetNameSpace(cop,NULL)),
                                &logbuf);
    releaseMsgLogBuf(&logbuf);
    if (CMIsNullObject(in)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not construct object instance");
      return st;
    }
    CMReturnInstance(rslt, in);
  } // if clsname == MESSAGELOGCLASSNAME 

  else if (strcasecmp(LOGRECORDCLASSNAME,clsname)==0 && fname) {
    logrecbuf.logname=strdup(fname);
    enumhdl = Syslog_LogRecord_Begin_Enum(fname);
    while (Syslog_LogRecord_Next_Enum(enumhdl, &logrecbuf)) { 
       // build logrecord object instance from buffer 
      in = makeLogRecordInstance(_broker,
                                 clsname,
                                 CMGetCharPtr(CMGetNameSpace(cop,NULL)),
                                 &logrecbuf);
      if (CMIsNullObject(in)) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not construct object path");
        releaseLogRecBuf(&logrecbuf);
        return st;
      }
      CMReturnInstance(rslt,in);
    } // end while 
    Syslog_LogRecord_End_Enum(enumhdl);
    releaseLogRecBuf(&logrecbuf);
  } // if clsname == LOGRECORDCLASSNAME 

  else {
    char mesg[1024];
    sprintf(mesg, "--- unsupported class %s\n",CMGetCharPtr(CMGetClassName(cop,NULL)));
    syslog_debug(stderr, mesg);
  } // else unsupported class 

  return st;
}

static CMPIStatus returnRef( const CMPIResult *rslt,
                            const CMPIObjectPath *cop,
                            char *clsname,
                            char *fname)
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIObjectPath *op = NULL;
  CMPIObjectPath *opRef = NULL;
  CMPIInstance *inRef = NULL;
  Syslog_LogRecord logrecbuf;
  Syslog_MessageLog logbuf;
  void *enumhdl = NULL;
                                                                                
  syslog_debug(stderr, "--- returnRef() \n");

  bzero(&logbuf, sizeof(Syslog_MessageLog));
  bzero(&logrecbuf, sizeof(Syslog_LogRecord));

  if (strcasecmp(MESSAGELOGCLASSNAME,clsname)==0) {
    logbuf.svName=strdup(fname);
    // build messagelog object path from buffer
    op = makeMessageLogPath(_broker,
                            clsname,
                            CMGetCharPtr(CMGetNameSpace(cop,NULL)),
                            &logbuf);
    releaseMsgLogBuf(&logbuf);
    if (CMIsNullObject(op)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not construct object path");
      return st;
    }

    // make reference object path 
    opRef = CMNewObjectPath(_broker, 
                            CMGetCharPtr(CMGetNameSpace(cop,&st)), 
                            LOCALCLASSNAME, 
                            NULL);
    if (CMIsNullObject(opRef)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not construct object path");
      return st;
    }
    // make reference object instance
    inRef = CMNewInstance(_broker,
                          opRef,
                          NULL);
    if (CMIsNullObject(inRef)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not construct object instance");
      return st;
    }
    CMSetProperty(inRef,"MessageLog",&op,CMPI_ref);
    CMSetProperty(inRef,"LogRecord",&cop,CMPI_ref);
    CMReturnInstance(rslt,inRef);
  } // if clsname == MESSAGELOGCLASSNAME 

  else if (strcasecmp(clsname, LOGRECORDCLASSNAME)==0) {
    logrecbuf.logname=strdup(fname);
    enumhdl = Syslog_LogRecord_Begin_Enum(fname);
    while (Syslog_LogRecord_Next_Enum(enumhdl, &logrecbuf)) {
      // build logrecord object path from buffer
      op = makeLogRecordPath(_broker,
                             clsname,
                             CMGetCharPtr(CMGetNameSpace(cop,NULL)),
                             &logrecbuf);
      if (CMIsNullObject(op)) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not construct object path");
        releaseLogRecBuf(&logrecbuf);
        return st;
      }
      // make reference object path
      opRef = CMNewObjectPath(_broker,
                              CMGetCharPtr(CMGetNameSpace(cop,&st)),
                              LOCALCLASSNAME,
                              NULL);
      if (CMIsNullObject(opRef)) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not construct object path");
        releaseLogRecBuf(&logrecbuf);
        return st;
      }
      // make reference object instance
      inRef = CMNewInstance(_broker,
                            opRef,
                            NULL);
      if (CMIsNullObject(inRef)) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not construct object instance");
	releaseLogRecBuf(&logrecbuf);
        return st;
      }
      CMSetProperty(inRef,"MessageLog",&cop,CMPI_ref);
      CMSetProperty(inRef,"LogRecord",&op,CMPI_ref);
      CMReturnInstance(rslt,inRef);
    } // end while 
    Syslog_LogRecord_End_Enum(enumhdl);
    releaseLogRecBuf(&logrecbuf);
  } // else if clsname == LOGRECORDCLASSNAME 

  else {
    char mesg[1024];
    sprintf(mesg, "--- unsupported class %s\n", CMGetCharPtr(CMGetClassName(cop,NULL)));
    syslog_debug(stderr, mesg);
  } // else unsupported class 

  return st;
}

static CMPIStatus associatorHelper( const CMPIResult *rslt,
                                   const CMPIObjectPath *cop,
                                   int associators, 
                                   int names )
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIString *clsname = NULL;
  CMPIData data;
  char logname[256];

  syslog_debug(stderr,"--- associatorHelper()\n");

  bzero(&logname, sizeof(logname));

  // Check if the object path belongs to a supported class
  clsname = CMGetClassName(cop,NULL);
  if (clsname) {
    if (strcasecmp(LOGRECORDCLASSNAME,CMGetCharPtr(clsname))==0) {
      // we have the LogRecord and can return the MessageLog 
      data = CMGetKey(cop,"LogName",NULL);
      strcpy(logname, CMGetCharPtr(data.value.string));
      logname[strlen(logname)]='\0';
      if (names) {
        if (associators) {
          st = returnAssocName(rslt,cop,MESSAGELOGCLASSNAME, logname);
        } 
        else {
          st = returnRefName(rslt,cop,MESSAGELOGCLASSNAME, logname);
        }
      } // if names, return paths
      else {
        if (associators) {
          st = returnAssoc(rslt,cop,MESSAGELOGCLASSNAME, logname);
        } 
        else {
          st = returnRef(rslt,cop,MESSAGELOGCLASSNAME, logname);
        }
      }	// if !names, return instances      
    } // if clsname == LOGRECORDCLASSNAME 

    else if (strcasecmp(MESSAGELOGCLASSNAME,CMGetCharPtr(clsname))==0) {
      // we have the MessageLog and can return the LogRecords 
      data = CMGetKey(cop,"Name",NULL);
      strcpy(logname, CMGetCharPtr(data.value.string));
      logname[strlen(logname)]='\0';
      if (names) {
        if (associators) {
          st = returnAssocName(rslt,cop,LOGRECORDCLASSNAME, logname);
        } 
        else {
          st = returnRefName(rslt,cop,LOGRECORDCLASSNAME, logname);
        }
      } // if names, return paths 
      else {
        if (associators) {
          st = returnAssoc(rslt,cop,LOGRECORDCLASSNAME, logname);
        } 
        else {
          st = returnRef(rslt,cop,LOGRECORDCLASSNAME, logname);
        }
      } // if !names, return instances
    } // else if clsname == MESSAGELOGCLASSNAME 
                                                                                
    else {
      char mesg[1024];
      sprintf(mesg, "--- unsupported class %s\n",CMGetCharPtr(clsname));
      syslog_debug(stderr, mesg);
    }
    CMReturnDone(rslt);
  } // if (clsname) 

  return st;
}

/* Instance MI Cleanup */

CMPIStatus Syslog_RecordInLogCleanup( CMPIInstanceMI *mi, 
                                     const CMPIContext *ctx,
				     CMPIBoolean terminate)
{
  CMReturn(CMPI_RC_OK);
}

/* Instance MI Functions */

CMPIStatus Syslog_RecordInLogEnumInstanceNames( CMPIInstanceMI *mi, 
                                               const CMPIContext *ctx, 
                                               const CMPIResult *rslt, 
                                               const CMPIObjectPath *ref) 
{
  syslog_debug(stderr, "--- Syslog_RecordInLogEnumInstanceNames() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );	
}

CMPIStatus Syslog_RecordInLogEnumInstances( CMPIInstanceMI *mi, 
                                           const CMPIContext *ctx, 
                                           const CMPIResult *rslt, 
                                           const CMPIObjectPath *ref, 
                                           const char **properties) 
{
  syslog_debug(stderr, "--- Syslog_RecordInLogEnumInstances() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );	
}

CMPIStatus Syslog_RecordInLogGetInstance( CMPIInstanceMI *mi, 
                                         const CMPIContext *ctx, 
                                         const CMPIResult *rslt, 
                                         const CMPIObjectPath *cop, 
                                         const char **properties) 
{
  syslog_debug(stderr, "--- Syslog_RecordInLogGetInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );	
}

CMPIStatus Syslog_RecordInLogCreateInstance( CMPIInstanceMI *mi, 
                                            const CMPIContext *ctx, 
                                            const CMPIResult *rslt, 
                                            const CMPIObjectPath *cop, 
                                            const CMPIInstance *ci) 
{
  syslog_debug(stderr, "--- Syslog_RecordInLogCreateInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_RecordInLogSetInstance( CMPIInstanceMI *mi, 
                                         const CMPIContext *ctx, 
                                         const CMPIResult *rslt, 
                                         const CMPIObjectPath *cop,
                                         const CMPIInstance *ci, 
                                         const char **properties) 
{
  syslog_debug(stderr, "--- Syslog_RecordInLogSetInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_RecordInLogDeleteInstance( CMPIInstanceMI *mi, 
                                            const CMPIContext *ctx, 
                                            const CMPIResult *rslt, 
                                            const CMPIObjectPath *cop) 
{ 
  syslog_debug(stderr, "--- Syslog_RecordInLogDeleteInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_RecordInLogExecQuery( CMPIInstanceMI *mi, 
                                       const CMPIContext *ctx, 
                                       const CMPIResult *rslt, 
                                       const CMPIObjectPath *cop, 
                                       const char *lang, 
                                       const char *query) 
{
  syslog_debug(stderr, "--- Syslog_RecordInLogExecQuery() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

/* Association MI Cleanup */

CMPIStatus Syslog_RecordInLogAssociationCleanup( CMPIAssociationMI *mi,
                                                const CMPIContext *ctx,
						CMPIBoolean terminate)
{
  CMReturn(CMPI_RC_OK);
}

/* Association MI Functions */
                                                                                
CMPIStatus Syslog_RecordInLogAssociators( CMPIAssociationMI *mi,
                                         const CMPIContext *ctx,
                                         const CMPIResult *rslt,
                                         const CMPIObjectPath *cop,
                                         const char *assocClass,
                                         const char *resultClass,
                                         const char *role,
                                         const char *resultRole,
                                         const char **propertyList )
{
  syslog_debug(stderr, "--- Syslog_RecordInLogAssociators() \n");
  return associatorHelper(rslt, cop, 1, 0);
}

CMPIStatus Syslog_RecordInLogAssociatorNames( CMPIAssociationMI *mi,
                                             const CMPIContext *ctx,
                                             const CMPIResult *rslt,
                                             const CMPIObjectPath *cop,
                                             const char *assocClass,
                                             const char *resultClass,
                                             const char *role,
                                             const char *resultRole )
{
  syslog_debug(stderr, "--- Syslog_RecordInLogAssociatorNames() \n");
  return associatorHelper(rslt, cop, 1, 1);
}
                                                           
CMPIStatus Syslog_RecordInLogReferences( CMPIAssociationMI *mi,
                                        const CMPIContext *ctx,
                                        const CMPIResult *rslt,
                                        const CMPIObjectPath *cop,
                                        const char *assocClass,
                                        const char *role,
                                        const char **propertyList )
{
  syslog_debug(stderr, "--- Syslog_RecordInLogReferences() \n");
  return associatorHelper(rslt, cop, 0, 0);
}

CMPIStatus Syslog_RecordInLogReferenceNames( CMPIAssociationMI *mi,
                                            const CMPIContext *ctx,
                                            const CMPIResult *rslt,
                                            const CMPIObjectPath *cop,
                                            const char *assocClass,
                                            const char *role)
{
  syslog_debug(stderr, "--- Syslog_RecordInLogReferenceNames()\n");
  return associatorHelper(rslt, cop, 0, 1);
}

/* Instance MI Factory */

CMInstanceMIStub( Syslog_RecordInLog,
                  Syslog_RecordInLog,
                  _broker,
                  CMNoHook);

/* Association MI Factory */
                                                                                
CMAssociationMIStub( Syslog_RecordInLog,
                     Syslog_RecordInLog,
                     _broker,
                    CMNoHook);

