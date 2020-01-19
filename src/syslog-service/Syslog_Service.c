/*
 * Syslog_Service.c
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
 * Description: CIM Linux Syslog Service Provider
 */


#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>
#include <string.h>

#include "Syslog_ServiceUtils.h"
#include "util/syslogserviceutil.h"
#define LOCALCLASSNAME "Syslog_Service"

static const CMPIBroker *_broker;

#ifdef CMPI_VER_100
#define Syslog_ServiceSetInstance Syslog_ServiceModifyInstance
#endif

/* Instance MI Cleanup */

CMPIStatus Syslog_ServiceCleanup( CMPIInstanceMI *mi,
                                 const CMPIContext *ctx, CMPIBoolean terminate)
{
  CMReturn(CMPI_RC_OK);
}

/* Instance MI Functions */

CMPIStatus Syslog_ServiceEnumInstanceNames( CMPIInstanceMI *mi,
                                           const CMPIContext *ctx,
                                           const CMPIResult *rslt,
                                           const CMPIObjectPath *ref) 
{
  CMPIObjectPath *op = NULL;
  CMPIStatus st = {CMPI_RC_OK,NULL};
  void *enumhdl = NULL;
  Syslog_Service servicebuf;

  syslog_debug(stderr,"--- Syslog_ServiceEnumInstanceNames() \n");

  bzero(&servicebuf, sizeof(Syslog_Service));

  enumhdl = Syslog_Service_Begin_Enum();
  if (enumhdl == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			 "Could not begin syslog service names enumeration");
    return st;
  } else {
    if (Syslog_Service_Next_Enum(enumhdl,&servicebuf)) {
      // build object path from buffer 
      op = makeServicePath(_broker,
                           LOCALCLASSNAME,
                           CMGetCharPtr(CMGetNameSpace(ref,NULL)),
                           &servicebuf);
      if (CMIsNullObject(op)) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not construct object path");
        goto label;
      }
      CMReturnObjectPath(rslt,op);
    }
label:
    CMReturnDone(rslt);
    Syslog_Service_End_Enum(enumhdl);
  }
  return st;
}

CMPIStatus Syslog_ServiceEnumInstances( CMPIInstanceMI *mi, 
                                       const CMPIContext *ctx, 
                                       const CMPIResult *rslt, 
                                       const CMPIObjectPath *ref, 
                                       const char **properties) 
{
  CMPIInstance *in = NULL; 
  CMPIStatus st = {CMPI_RC_OK,NULL};
  void *enumhdl = NULL;
  Syslog_Service servicebuf;
  
  syslog_debug(stderr,"--- SyslogServiceEnumInstances() \n");

  bzero(&servicebuf, sizeof(Syslog_Service));

  enumhdl = Syslog_Service_Begin_Enum();
  if (enumhdl == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			 "Could not begin syslog service instances enumeration");
    return st;
  } else {
    if (Syslog_Service_Next_Enum(enumhdl,&servicebuf)) {
      // build object instance from buffer 
      in = makeServiceInstance(_broker, 
                               LOCALCLASSNAME,
                               CMGetCharPtr(CMGetNameSpace(ref,NULL)), 
                               &servicebuf);
      if (CMIsNullObject(in)) {
	CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			     "Could not construct instance");
	goto label;
      }
      CMReturnInstance(rslt,in);
    }
label:
    CMReturnDone(rslt);
    Syslog_Service_End_Enum(enumhdl);
  }
  return st;
}

CMPIStatus Syslog_ServiceGetInstance( CMPIInstanceMI *mi, 
                                     const CMPIContext *ctx, 
                                     const CMPIResult *rslt, 
                                     const CMPIObjectPath *cop, 
                                     const char **properties) 
{
  CMPIInstance *in = NULL;
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIData nd;
  void *enumhdl = NULL;
  Syslog_Service servicebuf;
  char *name = NULL;

  syslog_debug(stderr,"--- SyslogServiceGetInstance() \n");

  bzero(&servicebuf, sizeof(Syslog_Service));

  nd = CMGetKey(cop, "Name", &st);
  if (st.msg != NULL || nd.value.string == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                         "Cannot read instance attribute.");
    return st;
  }
  name = CMGetCharPtr(nd.value.string);

  enumhdl = Syslog_Service_Begin_Enum();
  if (enumhdl == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                         "Could not begin syslog service instances enumeration");
    return st;
  } else {
    if (Syslog_Service_Next_Enum(enumhdl,&servicebuf)) {
      if (strncmp(name, servicebuf.svName, 6)==0) {   
        // build object instance from buffer 
        in = makeServiceInstance(_broker,
                                 LOCALCLASSNAME,
	                         CMGetCharPtr(CMGetNameSpace(cop,NULL)),
                                 &servicebuf);
        if (CMIsNullObject(in)) {
          CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                               "Could not construct instance");
          goto label;
        }
        CMReturnInstance(rslt,in);
      }
    }
label:
    CMReturnDone(rslt);
    Syslog_Service_End_Enum(enumhdl);
  }
  return st;
}

CMPIStatus Syslog_ServiceCreateInstance( CMPIInstanceMI *mi, 
                                        const CMPIContext *ctx, 
                                        const CMPIResult *rslt, 
                                        const CMPIObjectPath *cop, 
                                        const CMPIInstance *ci) 
{
  syslog_debug(stderr,"--- SyslogServiceCreateInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_ServiceSetInstance( CMPIInstanceMI *mi, 
                                     const CMPIContext *ctx, 
                                     const CMPIResult *rslt, 
                                     const CMPIObjectPath *cop,
                                     const CMPIInstance *ci, 
                                     const char **properties) 
{
  syslog_debug(stderr,"--- SyslogServiceSetInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_ServiceDeleteInstance( CMPIInstanceMI *mi, 
                                        const CMPIContext *ctx, 
                                        const CMPIResult *rslt, 
                                        const CMPIObjectPath *cop) 
{ 
  syslog_debug(stderr,"--- SyslogServiceDeleteInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_ServiceExecQuery( CMPIInstanceMI *mi, 
                                   const CMPIContext *ctx, 
                                   const CMPIResult *rslt, 
                                   const CMPIObjectPath *cop, 
                                   const char *lang, 
                                   const char *query) 
{
  syslog_debug(stderr,"--- SyslogServiceExecQuery() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

/* Method MI Cleanup */                                                 

CMPIStatus Syslog_ServiceMethodCleanup( CMPIMethodMI *mi,
                                        const CMPIContext *ctx,
					CMPIBoolean terminate)
{
  syslog_debug(stderr,"--- SyslogServiceMethodCleanup() \n");
  CMReturn(CMPI_RC_OK);
}

/* Method MI Functions */                                               
                                                                                
CMPIStatus Syslog_ServiceInvokeMethod( CMPIMethodMI *mi,
                                      const CMPIContext *ctx,
                                      const CMPIResult *rslt,
                                      const CMPIObjectPath *cop,
                                      const char *method,
                                      const CMPIArgs *in,
                                      CMPIArgs *out)
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIData dt;
  char typebuf[1000];
                                                                                
  syslog_debug(stderr,"--- Syslog_ServiceInvokeMethod() \n");

  bzero(&typebuf, sizeof(typebuf));
                                                                                
  dt=CMGetKey(cop,"Name",&st);
  if (st.rc != CMPI_RC_OK) {
    CMSetStatusWithChars(_broker,&st,CMPI_RC_ERR_FAILED,
                         "Could not get instance name");

  } else if (Syslog_Service_Operation(method, typebuf, sizeof(typebuf))) { 
    // if the function returns 1, then failure 
    CMSetStatusWithChars(_broker,&st,CMPI_RC_ERR_FAILED,
                         "Could not get instance type");
  } else {
    CMReturnData(rslt,typebuf,CMPI_chars);
    CMReturnDone(rslt);
  }
  return st;
}

/* Instance MI Factory */

CMInstanceMIStub( Syslog_Service,
                  Syslog_Service,
                  _broker,
                  CMNoHook);

/* Method MI Factory */
                                                                                
CMMethodMIStub( Syslog_Service,
                Syslog_Service,
                _broker,
                CMNoHook);

