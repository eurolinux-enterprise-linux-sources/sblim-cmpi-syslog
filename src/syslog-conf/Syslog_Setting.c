/*
 * Syslog_Setting.c
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
 * Description: CIM Linux Syslog Configuration Provider
 */


#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>
#include <string.h>
#include <stdio.h>

#include "Syslog_ConfUtils.h"
#include "util/syslogconfutil.h"
#define LOCALCLASSNAME "Syslog_Setting"

static const CMPIBroker *_broker;

#ifdef CMPI_VER_100
#define Syslog_SettingSetInstance Syslog_SettingModifyInstance
#endif

/* Instance MI Cleanup */

CMPIStatus Syslog_SettingCleanup( CMPIInstanceMI *mi, 
                                 const CMPIContext *ctx,
				 CMPIBoolean terminate)
{
  CMReturn(CMPI_RC_OK);
}

/* Instance MI Functions */

CMPIStatus Syslog_SettingEnumInstanceNames( CMPIInstanceMI *mi, 
                                           const CMPIContext *ctx, 
                                           const CMPIResult *rslt, 
                                           const CMPIObjectPath *ref) 
{
  CMPIObjectPath *op = NULL;
  CMPIStatus st = {CMPI_RC_OK,NULL};
  void *enumhdl = NULL;
  Syslog_Setting filebuf;

  syslog_debug(stderr,"--- Syslog_SettingEnumInstanceNames() \n");
  
  bzero(&filebuf, sizeof(Syslog_Setting));

  enumhdl = Syslog_Setting_Begin_Enum();
  if (enumhdl == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			 "Could not do syslog setting enumeration");
    return st;
  } else {
    while (Syslog_Setting_Next_Enum(enumhdl,&filebuf)) {
      // build setting object path from buffer 
      op = makeSettingPath(_broker,
                           LOCALCLASSNAME,
                           CMGetCharPtr(CMGetNameSpace(ref,NULL)),
                           &filebuf);
      if (CMIsNullObject(op)) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not construct object path");
        break;
      }
      CMReturnObjectPath(rslt,op);
    }
    CMReturnDone(rslt);
    Syslog_Setting_End_Enum(enumhdl);
  }

  return st;
}

CMPIStatus Syslog_SettingEnumInstances( CMPIInstanceMI *mi, 
                                       const CMPIContext *ctx, 
                                       const CMPIResult *rslt, 
                                       const CMPIObjectPath *ref, 
                                       const char **properties) 
{
  CMPIInstance *in = NULL; 
  CMPIStatus st = {CMPI_RC_OK,NULL};
  void *enumhdl = NULL;
  Syslog_Setting filebuf;
  
  syslog_debug(stderr,"--- Syslog_SettingEnumInstances() \n");
  
  bzero(&filebuf, sizeof(Syslog_Setting));

  enumhdl = Syslog_Setting_Begin_Enum();
  if (enumhdl == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			 "Could not do syslog setting enumeration");
    return st;
  } else {
    while (Syslog_Setting_Next_Enum(enumhdl,&filebuf)) {
      // build setting object instance from buffer 
      in = makeSettingInstance(_broker, 
                               LOCALCLASSNAME,
                               CMGetCharPtr(CMGetNameSpace(ref,NULL)), 
                               &filebuf);
      if (CMIsNullObject(in)) {
	CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			     "Could not construct instance");
	break;
      }
      CMReturnInstance(rslt,in);
    }
    CMReturnDone(rslt);
    Syslog_Setting_End_Enum(enumhdl);
  }

  return st;
}

CMPIStatus Syslog_SettingGetInstance( CMPIInstanceMI *mi, 
                                     const CMPIContext *ctx, 
                                     const CMPIResult *rslt, 
                                     const CMPIObjectPath *cop, 
                                     const char **properties) 
{
  CMPIInstance *in = NULL;
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIData nd;
  Syslog_Setting setting;
  char *facility = NULL;
  char *priority = NULL;
  char *path = NULL;
  int state = 0;

  syslog_debug(stderr,"--- Syslog_SettingGetInstance() \n");

  bzero(&setting, sizeof(Syslog_Setting));

  nd = CMGetKey(cop, "Facility", &st);
  if (st.msg != NULL || nd.value.string == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                         "Cannot read instance attribute.");
    return st;
  }
  facility = CMGetCharPtr(nd.value.string);

  /* We allow Priority to be empty string */
  nd = CMGetKey(cop, "Priority", &st);
  if (st.msg != NULL || (nd.state & CMPI_notFound) == CMPI_notFound) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                         "Cannot read instance attribute.");
    return st;
  }
  /* Workaround for CIMOM transforming empty string to a null string */
  if ((nd.state & CMPI_nullValue) == CMPI_nullValue || nd.value.string == NULL)
    priority = "";
  else
    priority = CMGetCharPtr(nd.value.string);

  nd = CMGetKey(cop, "Target", &st);
  if (st.msg != NULL || nd.value.string == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                         "Cannot read instance attribute.");
    return st;
  }
  path = CMGetCharPtr(nd.value.string);

  state = Syslog_Setting_Get_Instance(&setting, facility, priority, path);
  if (!state) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_NOT_FOUND,
                         "Setting instance does not exist.");
    return st;
  } else {
    // build setting object instance from buffer
    in = makeSettingInstance(_broker,
                             LOCALCLASSNAME,
                             CMGetCharPtr(CMGetNameSpace(cop,NULL)),
                             &setting);
    if (CMIsNullObject(in)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not construct instance");
      return st;
    }
  }
  CMReturnInstance(rslt,in);
  CMReturnDone(rslt);

  return st;
}

/* Create a setting instance, in the conf file, which amounts to a line of text
   in the configuration file */

CMPIStatus Syslog_SettingCreateInstance( CMPIInstanceMI *mi, 
                                        const CMPIContext *ctx, 
                                        const CMPIResult *rslt, 
                                        const CMPIObjectPath *cop, 
                                        const CMPIInstance *ci) 
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIObjectPath *op=NULL;
  Syslog_Setting setting;
  CMPIData nd;
  char *facility = NULL;
  char *priority = NULL; 
  char *target = NULL;
  int state = 0;

  syslog_debug(stderr,"--- Syslog_SettingCreateInstance() \n");

  nd = CMGetProperty(ci, "Facility", &st);
  if (st.msg != NULL || nd.value.string == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED, 
                         "Can not get key property Facility.");
    return st;
  }
  facility = CMGetCharPtr(nd.value.string);

  /* We allow Priority to be empty string */
  nd = CMGetProperty(ci, "Priority", &st);
  if (st.msg != NULL || (nd.state & CMPI_notFound) == CMPI_notFound) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED, 
                         "Can not get key property Priority.");
    return st;
  }
  /* Workaround for CIMOM transforming empty string to a null string */
  if ((nd.state & CMPI_nullValue) == CMPI_nullValue || nd.value.string == NULL)
    priority = "";
  else
    priority = CMGetCharPtr(nd.value.string);

  nd = CMGetProperty(ci, "Target", &st); 
  if (st.msg != NULL || nd.value.string == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED, 
                         "Can not get key property Target.");
    return st;
  }
  target = CMGetCharPtr(nd.value.string);

  if (setting_parse(facility, priority, target, ACTION_CREATE) != 0) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                                 "Could not create instance");
    return st;
  }

  state = Syslog_Setting_Get_Instance(&setting, facility, priority, target);
  if (!state) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_NOT_FOUND,
                         "Setting instance does not exist.");
    return st;
  }
  // build setting object path from buffer 
  op = makeSettingPath(_broker,
		       LOCALCLASSNAME,
		       CMGetCharPtr(CMGetNameSpace(cop,NULL)),
		       &setting);
  if (CMIsNullObject(op)) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			 "Could not construct object path");
  }
  CMReturnObjectPath(rslt,op);

  CMReturnDone(rslt);
  return st;
}

CMPIStatus Syslog_SettingSetInstance( CMPIInstanceMI *mi, 
                                     const CMPIContext *ctx, 
                                     const CMPIResult *rslt, 
                                     const CMPIObjectPath *cop,
                                     const CMPIInstance *ci, 
                                     const char **properties) 
{
  syslog_debug(stderr,"--- Syslog_SettingSetInstance() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

/* Delete a setting instance, or removing a line or record from the 
   configuration file */

CMPIStatus Syslog_SettingDeleteInstance( CMPIInstanceMI *mi, 
                                        const CMPIContext *ctx, 
                                        const CMPIResult *rslt, 
                                        const CMPIObjectPath *cop) 
{ 
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIData nd;
  char *facility = NULL;
  char *priority = NULL;
  char *target = NULL;
                                                                                
  syslog_debug(stderr,"--- Syslog_SettingDeleteInstance() \n");

  nd = CMGetKey(cop, "Facility", &st);
  if (st.msg != NULL || nd.value.string == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                         "Cannot read instance attribute.");
    return st;
  }
  facility = CMGetCharPtr(nd.value.string);

  /* We allow Priority to be empty string */
  nd = CMGetKey(cop, "Priority", &st);
  if (st.msg != NULL || (nd.state & CMPI_notFound) == CMPI_notFound) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                         "Cannot read instance attribute.");
    return st;
  }
  /* Workaround for CIMOM transforming empty string to a null string */
  if ((nd.state & CMPI_nullValue) == CMPI_nullValue || nd.value.string == NULL)
    priority = "";
  else
    priority = CMGetCharPtr(nd.value.string);

  nd = CMGetKey(cop, "Target", &st);
  if (st.msg != NULL || nd.value.string == NULL) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                         "Cannot read instance attribute.");
    return st;
  }
  target = CMGetCharPtr(nd.value.string);

  if (setting_parse(facility, priority, target, ACTION_DELETE) != 0) {
    CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                         "Could not delete instance");
    CMReturn(CMPI_RC_ERR_FAILED);
  }

  CMReturn(CMPI_RC_OK);
}

CMPIStatus Syslog_SettingExecQuery( CMPIInstanceMI *mi, 
                                   const CMPIContext *ctx, 
                                   const CMPIResult *rslt, 
                                   const CMPIObjectPath *cop, 
                                   const char *lang, 
                                   const char *query) 
{
  syslog_debug(stderr,"--- Syslog_SettingExecQuery() \n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

/* Instance MI Factory */

CMInstanceMIStub( Syslog_Setting,
                  Syslog_Setting,
                  _broker,
                  CMNoHook);

