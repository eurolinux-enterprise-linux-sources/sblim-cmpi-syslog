/*
 * Syslog_Configuration.c
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
 * Description: CIM Linux Syslog Configuration Provider
 */


#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>
#include <string.h>

#include "Syslog_ConfUtils.h"
#include "util/syslogconfutil.h"
#define LOCALCLASSNAME "Syslog_Configuration"

static const CMPIBroker * _broker;

#ifdef CMPI_VER_100
#define Syslog_ConfigurationSetInstance Syslog_ConfigurationModifyInstance
#endif

/* Instance MI Cleanup */

CMPIStatus Syslog_ConfigurationCleanup( CMPIInstanceMI *mi, 
                                       const CMPIContext *ctx,
				       CMPIBoolean terminate)
{
  CMReturn(CMPI_RC_OK);
}

/* Instance MI Functions */

CMPIStatus Syslog_ConfigurationEnumInstanceNames( CMPIInstanceMI *mi, 
                                                 const CMPIContext *ctx, 
                                                 const CMPIResult *rslt, 
                                                 const CMPIObjectPath *ref) 
{
  CMPIObjectPath *op = NULL;
  CMPIStatus st = {CMPI_RC_OK,NULL};
  void *enumhdl = NULL;
  Syslog_Configuration filebuf;

  syslog_debug(stderr,"--- Syslog_ConfigurationEnumInstanceNames() \n");

  bzero(&filebuf, sizeof(Syslog_Configuration));

  enumhdl = Syslog_Conf_Begin_Enum();
  if (enumhdl == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			 "Could not do syslog configuration file enumeration");
    return st;
  } else {
    if (Syslog_Conf_Next_Enum(enumhdl,&filebuf)) {
      // build configuration object path from buffer 
      op = makeConfPath(_broker,
                        LOCALCLASSNAME,
                        CMGetCharPtr(CMGetNameSpace(ref,NULL)),
                        &filebuf);
      if (CMIsNullObject(op)) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not construct object path");
        goto done;
      }
      CMReturnObjectPath(rslt,op);
    }
done:
    CMReturnDone(rslt);
    Syslog_Conf_End_Enum(enumhdl);
  }

  return st;
}

CMPIStatus Syslog_ConfigurationEnumInstances( CMPIInstanceMI *mi, 
                                             const CMPIContext *ctx, 
                                             const CMPIResult *rslt, 
                                             const CMPIObjectPath *ref, 
                                             const char **properties) 
{
  CMPIInstance *in = NULL; 
  CMPIStatus st = {CMPI_RC_OK,NULL};
  void *enumhdl = NULL;
  Syslog_Configuration filebuf;
  
  syslog_debug(stderr,"--- Syslog_ConfigurationEnumInstances() \n");

  bzero(&filebuf, sizeof(Syslog_Configuration));

  enumhdl = Syslog_Conf_Begin_Enum();
  if (enumhdl == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			 "Could not begin syslog configuration enumeration");
    return st;
  } else {
    if (Syslog_Conf_Next_Enum(enumhdl,&filebuf)) {
      // build configuration object instance from buffer 
      in = makeConfInstance(_broker, 
                    	    LOCALCLASSNAME,
                            CMGetCharPtr(CMGetNameSpace(ref,NULL)), 
                            &filebuf);
      if (CMIsNullObject(in)) {
	CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			     "Could not construct instance");
	goto done;	
      }
      CMReturnInstance(rslt,in);
    }
done:
    CMReturnDone(rslt);
    Syslog_Conf_End_Enum(enumhdl);
  }

  return st;
}

CMPIStatus Syslog_ConfigurationGetInstance( CMPIInstanceMI *mi, 
                                           const CMPIContext *ctx, 
                                           const CMPIResult *rslt, 
                                           const CMPIObjectPath *cop, 
                                           const char **properties) 
{
  syslog_debug(stderr,"--- Syslog_ConfigurationGetInstance() \n");
  return Syslog_ConfigurationEnumInstances( mi, ctx, rslt, cop, properties);
}

CMPIStatus Syslog_ConfigurationCreateInstance( CMPIInstanceMI *mi, 
                                              const CMPIContext *ctx, 
                                              const CMPIResult *rslt, 
                                              const CMPIObjectPath *cop, 
                                              const CMPIInstance *ci) 
{
  syslog_debug(stderr,"--- Syslog_ConfigurationCreateInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_ConfigurationSetInstance( CMPIInstanceMI *mi, 
                                           const CMPIContext *ctx, 
                                           const CMPIResult *rslt, 
                                           const CMPIObjectPath *cop,
                                           const CMPIInstance *ci, 
                                           const char **properties) 
{
  syslog_debug(stderr,"--- Syslog_ConfigurationSetInstance() \n");
  CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus Syslog_ConfigurationDeleteInstance( CMPIInstanceMI *mi, 
                                              const CMPIContext *ctx, 
                                              const CMPIResult *rslt, 
                                              const CMPIObjectPath *cop) 
{ 
  syslog_debug(stderr,"--- Syslog_ConfigurationDeleteInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_ConfigurationExecQuery( CMPIInstanceMI *mi, 
                                         const CMPIContext *ctx, 
                                         const CMPIResult *rslt, 
                                         const CMPIObjectPath *cop, 
                                         const char *lang, 
                                         const char *query) 
{
  syslog_debug(stderr,"--- Syslog_ConfigurationExecQuery() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

/* Instance MI Factory */

CMInstanceMIStub( Syslog_Configuration,
                  Syslog_Configuration,
                  _broker,
                  CMNoHook);


