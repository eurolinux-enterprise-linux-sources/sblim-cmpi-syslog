/*
 * Syslog_ConfUtils.c
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
#include <unistd.h>
#include <string.h>

#include "Syslog_ConfUtils.h"
#include "util/syslogconfutil.h"

CMPIObjectPath *makeConfPath( const CMPIBroker *broker, 
                             const char *classname, 
                             const char *namespace, 
                             Syslog_Configuration *svc)
{
  CMPIObjectPath *op = NULL;

  syslog_debug(stderr,"--- makeConfPath() \n");

  op = CMNewObjectPath(broker, 
		       (char*)namespace,
		       (char*)classname,
		       NULL);
  if (!CMIsNullObject(op)) {
    CMAddKey(op,"Name",svc->fname,CMPI_chars); 
  }

  return op;
}

CMPIInstance *makeConfInstance( const CMPIBroker *broker, 
                               const char *classname, 
                               const char *namespace, 
                               Syslog_Configuration *svc)
{
  CMPIInstance   *in = NULL;
  CMPIObjectPath *op = CMNewObjectPath(broker, 
                                       (char*)namespace,
                                       (char*)classname,
                                       NULL);
  
  syslog_debug(stderr, "--- makeConfInstance() \n");

  if (!CMIsNullObject(op)) {
    in = CMNewInstance(broker,op,NULL);
    if (!CMIsNullObject(in)) {
      CMSetProperty(in,"Name",svc->fname,CMPI_chars); 
    }
  }

  return in;
}

CMPIObjectPath *makeSettingPath( const CMPIBroker *broker, 
                                const char *classname, 
                                const char *namespace, 
                                Syslog_Setting *svc)
{
  CMPIObjectPath *op = NULL;

  syslog_debug(stderr,"--- makeSettingPath() \n");

  op = CMNewObjectPath(broker, 
                       (char*)namespace,
                       (char*)classname,
                       NULL);
  if (!CMIsNullObject(op)) {
    CMAddKey(op,"Facility",svc->facility,CMPI_chars); 
    CMAddKey(op,"Priority",svc->priority,CMPI_chars); 
    CMAddKey(op,"Target",svc->target,CMPI_chars); 
  }

  return op;
}

CMPIInstance *makeSettingInstance( const CMPIBroker *broker, 
                                  const char *classname, 
                                  const char *namespace, 
                                  Syslog_Setting *svc)
{
  CMPIInstance *in = NULL;
  CMPIObjectPath *op = CMNewObjectPath(broker, 
                                       (char*)namespace,
                                       (char*)classname,
                                       NULL);
  
  syslog_debug(stderr, "---  makeSettingInstance() \n");

  if (!CMIsNullObject(op)) {
    in = CMNewInstance(broker,op,NULL);
    if (!CMIsNullObject(in)) {
      CMSetProperty(in,"Facility",svc->facility,CMPI_chars); 
      CMSetProperty(in,"Priority",svc->priority,CMPI_chars); 
      CMSetProperty(in,"Target",svc->target,CMPI_chars); 
    }
  }

  return in;
}

