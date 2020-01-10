/*
 * Syslog_SettingContext.c
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
#include <libgen.h>
#include <stdio.h>

#include "Syslog_ConfUtils.h"
#include "util/syslogconfutil.h"
#define LOCALCLASSNAME "Syslog_SettingContext"
#define CONFIGURATIONCLASSNAME "Syslog_Configuration"
#define SETTINGCLASSNAME "Syslog_Setting"

static const CMPIBroker *_broker;

#ifdef CMPI_VER_100
#define Syslog_SettingContextSetInstance Syslog_SettingContextModifyInstance
#endif

static CMPIStatus returnAssocName( const CMPIContext *ctx,
                                 const CMPIResult *rslt,
                                 const CMPIObjectPath *cop,
                                 char *clsname,
                                 Syslog_Configuration *buf)
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIObjectPath *op = NULL;
  Syslog_Configuration *confbufp = NULL;
  Syslog_Setting settingbuf;
  void *enumhdl = NULL;

  syslog_debug(stderr, "--- returnAssocName()\n"); 

  bzero(&settingbuf, sizeof(Syslog_Setting));

  if (strcasecmp(SETTINGCLASSNAME,clsname)==0 && buf) {
    enumhdl = Syslog_Setting_Begin_Enum();
    if (enumhdl == NULL) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not do syslog setting enumeration");
      return st;
    } else {
      while (Syslog_Setting_Next_Enum(enumhdl,&settingbuf)) {
        // build setting object path from buffer 
        op = makeSettingPath(_broker,
                             clsname,
                             CMGetCharPtr(CMGetNameSpace(cop,NULL)),
                             &settingbuf);
        if (CMIsNullObject(op)) {
          CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                               "Could not construct object path");
          return st;
        }
        CMReturnObjectPath(rslt,op);
      }// end while 
      Syslog_Setting_End_Enum(enumhdl);
    }// else 
  } // if clsname == SETTINGCLASSNAME 

  else if (strcasecmp(CONFIGURATIONCLASSNAME,clsname)==0 && buf) {
    confbufp = (Syslog_Configuration *)buf;
    // build configuration object path from buffer
    op = makeConfPath(_broker,
                      clsname,
                      CMGetCharPtr(CMGetNameSpace(cop,NULL)),
	              confbufp);
    if (CMIsNullObject(op)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not construct object path");
      return st;
    }
    CMReturnObjectPath(rslt,op);
  } // if clsname == CONFIGURATIONCLASSNAME 

  else {
    char mesg[1024];
    sprintf(mesg, "--- unsupported class %s\n", CMGetCharPtr(CMGetClassName(cop, NULL)));
    syslog_debug(stderr, mesg);
  } // else unsupported class 

  return st;
}

static CMPIStatus returnAssoc( const CMPIContext *ctx,
                              const CMPIResult *rslt,
                              const CMPIObjectPath *cop,
                              char *clsname,
                              Syslog_Configuration *buf )
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIInstance *in = NULL;
  Syslog_Setting settingbuf;
  Syslog_Configuration *confbufp = NULL;
  void *enumhdl = NULL;

  syslog_debug(stderr, "--- returnAssoc()\n");

  bzero(&settingbuf, sizeof(Syslog_Setting));

  if (strcasecmp(SETTINGCLASSNAME,clsname)==0 && buf) {
    enumhdl = Syslog_Setting_Begin_Enum();
    if (enumhdl == NULL) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not do syslog setting enumeration");
      return st;
    } else {
      while (Syslog_Setting_Next_Enum(enumhdl,&settingbuf)) {
        // build setting object instance from buffer 
        in = makeSettingInstance(_broker,
                  	         clsname,
                                 CMGetCharPtr(CMGetNameSpace(cop,NULL)),
                                 &settingbuf);
	if (CMIsNullObject(in)) {
	  CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
	                       "Could not construct object path");
	  return st;
	}
        CMReturnInstance(rslt,in);
      }	
      Syslog_Setting_End_Enum(enumhdl);
    }	
  } // if clsname == SETTINGCLASSNAME 

  else if (strcasecmp(CONFIGURATIONCLASSNAME,clsname)==0 && buf) {
    confbufp = (Syslog_Configuration *)buf;
    // build configuration object instance from buffer
    in = makeConfInstance(_broker,
                          clsname,
                          CMGetCharPtr(CMGetNameSpace(cop,NULL)),
	                  confbufp);
    if (CMIsNullObject(in)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not construct instance");
      return st;
    }
    CMReturnInstance(rslt,in);
  } // if clsname == CONFIGURATIONCLASSNAME 
                                                                                
  else {
    char mesg[1024];
    sprintf(mesg, "--- unsupported class %s\n", CMGetCharPtr(CMGetClassName(cop,NULL)));
    syslog_debug(stderr, mesg);
  } // else unsupported class 
                                                                                
  return st;
}

static CMPIStatus returnRefName( const CMPIContext *ctx,
                                const CMPIResult *rslt,
                                const CMPIObjectPath *cop,
                                char *clsname,
                                Syslog_Configuration *buf)
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIObjectPath *op = NULL;
  CMPIObjectPath *opRef = NULL;
  Syslog_Setting settingbuf;
  Syslog_Configuration *confbufp = NULL;
  void *enumhdl = NULL;
                                                                                
  syslog_debug(stderr, "--- returnRefName()\n");

  bzero(&settingbuf, sizeof(Syslog_Setting));

  if (strcasecmp(SETTINGCLASSNAME,clsname)==0) {
    enumhdl = Syslog_Setting_Begin_Enum();
    if (enumhdl == NULL) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not do syslog setting enumeration");
      return st;
    } else {
      while (Syslog_Setting_Next_Enum(enumhdl,&settingbuf)) {
        // build setting object path from buffer 
        op = makeSettingPath(_broker,
                             clsname,
                             CMGetCharPtr(CMGetNameSpace(cop,NULL)),
                             &settingbuf);
        if (CMIsNullObject(op)) {
          CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                               "Could not construct object path");
          break;
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
        CMAddKey(opRef,"setting",&op,CMPI_ref);
        CMAddKey(opRef,"context",&cop,CMPI_ref);
        CMReturnObjectPath(rslt,opRef);
      } // end while 
      Syslog_Setting_End_Enum(enumhdl);
    } // if else 	      
  } // if clsname == SETTINGCLASSNAME 

  else if (strcasecmp(clsname, CONFIGURATIONCLASSNAME)==0) {
    confbufp = (Syslog_Configuration *)buf;	  
    // build configuration object path
    op = makeConfPath(_broker,
                      clsname,
	              CMGetCharPtr(CMGetNameSpace(cop,NULL)),
	              confbufp);
    if (CMIsNullObject(op)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not construct object path");
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
    CMAddKey(opRef,"setting",&cop,CMPI_ref);
    CMAddKey(opRef,"context",&op,CMPI_ref);
    CMReturnObjectPath(rslt,opRef);
  } // else if clsname == CONFIGURATIONCLASSNAME 

  else {
   char mesg[1024];
   sprintf(mesg, "--- unsupported class %s\n",CMGetCharPtr(CMGetClassName(cop,NULL)));
   syslog_debug(stderr, mesg);
  } // else unsupported class 

  return st;
}

static CMPIStatus returnRef( const CMPIContext *ctx,
                            const CMPIResult *rslt,
                            const CMPIObjectPath *cop,
                            char *clsname,
                            Syslog_Configuration *buf )
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIObjectPath *op = NULL;
  CMPIObjectPath *opRef = NULL;
  CMPIInstance *inRef = NULL;
  Syslog_Setting settingbuf;
  Syslog_Configuration *confbufp = NULL;
  void *enumhdl = NULL;

  syslog_debug(stderr, "--- returnRef()\n");

  bzero(&settingbuf, sizeof(Syslog_Setting));
  if (strcasecmp(SETTINGCLASSNAME,clsname)==0) {
    enumhdl = Syslog_Setting_Begin_Enum();
    if (enumhdl == NULL) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not do syslog setting enumeration");
      return st;
    } else {
      while (Syslog_Setting_Next_Enum(enumhdl,&settingbuf)) {
        // build setting object path from buffer 
        op = makeSettingPath(_broker,
                             clsname,
			     CMGetCharPtr(CMGetNameSpace(cop,NULL)),
                             &settingbuf);
        if (CMIsNullObject(op)) {
          CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                               "Could not construct object path");
          break;
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
	// make reference object instance
        inRef = CMNewInstance(_broker,opRef,NULL);
        if (CMIsNullObject(inRef)) {
          CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                               "Could not construct object instance");
          return st;
        }

        CMSetProperty(inRef,"setting",&op,CMPI_ref);
        CMSetProperty(inRef,"context",&cop,CMPI_ref);
        CMReturnInstance(rslt,inRef);

      } // end while 
      Syslog_Setting_End_Enum(enumhdl);
    } // else 
  } // if clsname == SETTINGCLASSNAME 

  else if (strcasecmp(clsname, CONFIGURATIONCLASSNAME)==0) {
    confbufp = (Syslog_Configuration *)buf;	  
    // build configuration object path from buffer
    op = makeConfPath(_broker,
	              clsname,
                      CMGetCharPtr(CMGetNameSpace(cop,NULL)),
		      confbufp);
    if (CMIsNullObject(op)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
			    "Could not construct object path");
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
    // make reference object instance
    inRef = CMNewInstance(_broker,opRef,NULL);
    if (CMIsNullObject(inRef)) {
      CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                           "Could not construct object instance");
      return st;
    }

    CMSetProperty(inRef,"setting",&cop,CMPI_ref);
    CMSetProperty(inRef,"context",&op,CMPI_ref);
    CMReturnInstance(rslt,inRef);
  } // else if clsname == CONFIGURATIONCLASSNAME 

  else {
    char mesg[1024];
    sprintf(mesg, "--- unsupported class %s\n",CMGetCharPtr(CMGetClassName(cop,NULL)));
    syslog_debug(stderr, mesg);
  } // else unsupported class 

  return st;
}

static CMPIStatus associatorHelper( const CMPIContext *ctx,
                                   const CMPIResult *rslt,
                                   const CMPIObjectPath *cop,
                                   int associators, 
                                   int names )
{
  CMPIStatus st = {CMPI_RC_OK,NULL};
  CMPIString *clsname;
  CMPIData data;
  void *enumhdl;
  Syslog_Configuration filebuf;

  syslog_debug(stderr,"--- associatorHelper()\n");

  //  check if the object path belongs to a supported class
  clsname = CMGetClassName(cop,NULL);
  if (clsname) {
    if (strcasecmp(SETTINGCLASSNAME,CMGetCharPtr(clsname))==0) {
      // we have the Setting and can return the Configuration 
      data = CMGetKey(cop,"Name",NULL);
      enumhdl = Syslog_Conf_Begin_Enum();
      if (enumhdl == NULL) {
        CMSetStatusWithChars(_broker, &st, CMPI_RC_ERR_FAILED,
                             "Could not begin syslog config enumeration");
        return st;
      } 
      else {
        if (Syslog_Conf_Next_Enum(enumhdl,&filebuf)) {
          if (names) {
            if (associators) {
              st = returnAssocName(ctx, rslt, cop, CONFIGURATIONCLASSNAME, &filebuf);
            } 
	    else {
              st = returnRefName(ctx, rslt, cop, CONFIGURATIONCLASSNAME, &filebuf);
            }
          } // if names, return paths
	  else {
	    if (associators) {
	      st = returnAssoc(ctx, rslt, cop, CONFIGURATIONCLASSNAME, &filebuf);
	    }
            else {
              st = returnRef(ctx, rslt, cop, CONFIGURATIONCLASSNAME, &filebuf);
            }
          } // if !names, return instances
        } // syslog_conf_next_enum()
        Syslog_Conf_End_Enum(enumhdl);
      }
    } // if clsname == SETTINGCLASSNAME 

    else if (strcasecmp(CONFIGURATIONCLASSNAME,CMGetCharPtr(clsname))==0) {
      // we have the Configuration and can return the Settings 
      data = CMGetKey(cop,"Name",NULL);
      enumhdl = Syslog_Conf_Begin_Enum();
      if (Syslog_Conf_Next_Enum(enumhdl,&filebuf)) {
        if (names) {
          if (associators) {
            st = returnAssocName(ctx, rslt, cop, SETTINGCLASSNAME, &filebuf);
          } 
	  else {
            st = returnRefName(ctx, rslt, cop, SETTINGCLASSNAME, &filebuf);
          }
        } // if names, return paths 
	else {
          if (associators) {
            st = returnAssoc(ctx, rslt, cop, SETTINGCLASSNAME, &filebuf);
          }
          else {
            st = returnRef(ctx, rslt, cop, SETTINGCLASSNAME, &filebuf);
          }
        } // if !names, return instances
      } // syslog_conf_next_enum()
      Syslog_Conf_End_Enum(enumhdl);
    } // else if clsname == CONFIGURATIONCLASSNAME 
                                                                                
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

CMPIStatus Syslog_SettingContextCleanup( CMPIInstanceMI *mi, 
                                        const CMPIContext *ctx,
					CMPIBoolean terminate) 
{
  CMReturn(CMPI_RC_OK);
}

/* Instance MI Functions */

CMPIStatus Syslog_SettingContextEnumInstanceNames( CMPIInstanceMI *mi, 
                                                  const CMPIContext *ctx, 
                                                  const CMPIResult *rslt, 
                                                  const CMPIObjectPath *ref) 
{
  syslog_debug(stderr, "--- Syslog_SettingContextEnumInstanceNames()\n");
  CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);	
}

CMPIStatus Syslog_SettingContextEnumInstances( CMPIInstanceMI *mi, 
                                              const CMPIContext *ctx, 
                                              const CMPIResult *rslt, 
                                              const CMPIObjectPath *ref, 
                                              const char **properties) 
{
  syslog_debug(stderr, "--- Syslog_SettingContextEnumInstances()\n");
  CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);	
}

CMPIStatus Syslog_SettingContextGetInstance( CMPIInstanceMI *mi, 
                                            const CMPIContext *ctx, 
                                            const CMPIResult *rslt, 
                                            const CMPIObjectPath *cop, 
                                            const char **properties) 
{
  syslog_debug(stderr, "--- Syslog_SettingContextGetInstance()\n");
  CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);	
}

CMPIStatus Syslog_SettingContextCreateInstance( CMPIInstanceMI *mi, 
                                               const CMPIContext *ctx, 
                                               const CMPIResult *rslt, 
                                               const CMPIObjectPath *cop, 
                                               const CMPIInstance *ci) 
{
  syslog_debug(stderr, "--- Syslog_SettingContextCreateInstance()\n");
  CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus Syslog_SettingContextSetInstance( CMPIInstanceMI *mi, 
                                            const CMPIContext *ctx, 
                                            const CMPIResult *rslt, 
                                            const CMPIObjectPath *cop,
                                            const CMPIInstance *ci, 
                                            const char **properties) 
{
  syslog_debug(stderr, "--- Syslog_SettingContextSetInstance()\n");
  CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus Syslog_SettingContextDeleteInstance( CMPIInstanceMI *mi, 
                                               const CMPIContext *ctx, 
                                               const CMPIResult *rslt, 
                                               const CMPIObjectPath *cop) 
{ 
  syslog_debug(stderr, "--- Syslog_SettingContextDeleteInstance()\n");
  CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus Syslog_SettingContextExecQuery( CMPIInstanceMI *mi, 
                                          const CMPIContext *ctx, 
                                          const CMPIResult *rslt, 
                                          const CMPIObjectPath *cop, 
                                          const char *lang, 
                                          const char *query) 
{
  syslog_debug(stderr, "--- Syslog_SettingContextExecQuery()\n");
  CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

/* Association MI Cleanup */

CMPIStatus Syslog_SettingContextAssociationCleanup( CMPIAssociationMI *mi,
                                                   const CMPIContext *ctx,
						   CMPIBoolean terminate)
{
  CMReturn(CMPI_RC_OK);
}

/* Association MI Functions */
                                                                                
CMPIStatus Syslog_SettingContextAssociators( CMPIAssociationMI *mi,
                                            const CMPIContext *ctx,
                                            const CMPIResult *rslt,
                                            const CMPIObjectPath *cop,
                                            const char *assocClass,
                                            const char *resultClass,
                                            const char *role,
                                            const char *resultRole,
                                            const char **propertyList)
{
  syslog_debug(stderr, "--- Syslog_SettingContextAssociators()\n");
  return associatorHelper(ctx, rslt, cop, 1, 0);
}

CMPIStatus Syslog_SettingContextAssociatorNames( CMPIAssociationMI *mi,
                                                const CMPIContext *ctx,
                                                const CMPIResult *rslt,
                                                const CMPIObjectPath *cop,
                                                const char *assocClass,
                                                const char *resultClass,
                                                const char *role,
                                                const char *resultRole)
{
  syslog_debug(stderr, "--- Syslog_SettingContextAssociatorNames()\n");
  return associatorHelper(ctx, rslt, cop, 1, 1);
}
                                                                                

CMPIStatus Syslog_SettingContextReferences( CMPIAssociationMI *mi,
                                           const CMPIContext *ctx,
                                           const CMPIResult *rslt,
                                           const CMPIObjectPath *cop,
                                           const char *assocClass,
                                           const char *role,
                                           const char **propertyList)
{
  syslog_debug(stderr, "--- Syslog_SettingContextReferences()\n");
  return associatorHelper(ctx, rslt, cop, 0, 0);
}

CMPIStatus Syslog_SettingContextReferenceNames( CMPIAssociationMI *mi,
                                               const CMPIContext *ctx,
                                               const CMPIResult *rslt,
                                               const CMPIObjectPath *cop,
                                               const char *assocClass,
                                               const char *role)
{
  syslog_debug(stderr, "--- Syslog_SettingContextReferenceNames()\n");
  return associatorHelper(ctx, rslt, cop, 0, 1);
}

/* Instance MI Factory */

CMInstanceMIStub( Syslog_SettingContext,
                  Syslog_SettingContext,
                  _broker,
                  CMNoHook);

/* Association MI Factory */                                          
                                                                                
CMAssociationMIStub( Syslog_SettingContext,
                     Syslog_SettingContext,
                     _broker,
		     CMNoHook);

