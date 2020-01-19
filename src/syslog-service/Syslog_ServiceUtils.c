/*
 * Syslog_ServiceUtils.c
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
#include <unistd.h>
#include <string.h>

#include "Syslog_ServiceUtils.h"
#include "util/syslogserviceutil.h"
static char sccn[] = "CIM_UnitaryComputerSystem";
static char ccn[] = "Syslog_Service";
static char csn[500] = "";


char *SystemCreationClassName()
{
  return sccn;
}

char *SystemName()
{
  if (*csn == 0)
    gethostname(csn,sizeof(csn));
  return csn;
}

char *CreationClassName()
{
  return ccn;
}

CMPIObjectPath *makeServicePath( const CMPIBroker *broker, 
                                const char *classname, 
                                const char *namespace, 
                                Syslog_Service *svc)
{
  CMPIObjectPath *op = NULL;

  syslog_debug(stderr,"--- makeServicePath() \n");

  op = CMNewObjectPath(broker, 
                       (char*)namespace,
                       (char*)classname,
                       NULL);
  if (!CMIsNullObject(op)) {
    CMAddKey(op,"CreationClassName",CreationClassName(),CMPI_chars); 
    CMAddKey(op,"SystemCreationClassName",SystemCreationClassName(),CMPI_chars); 
    CMAddKey(op,"SystemName",SystemName(),CMPI_chars); 
    CMAddKey(op,"Name",svc->svName,CMPI_chars); 
  }

  return op;
}

CMPIInstance *makeServiceInstance( const CMPIBroker *broker, 
		                  const char * classname, 
			          const char * namespace, 
				  Syslog_Service *svc)
{
  CMPIInstance *in = NULL;
  CMPIObjectPath *op = CMNewObjectPath(broker, 
                                       (char*)namespace,
                                       (char*)classname,
                                       NULL);
  
  syslog_debug(stderr,"--- makeServiceInstance() \n");

  if (!CMIsNullObject(op)) {
    in = CMNewInstance(broker,op,NULL);
    if (!CMIsNullObject(in)) {
      CMSetProperty(in,"CreationClassName",CreationClassName(),CMPI_chars);
      CMSetProperty(in,"SystemCreationClassName",SystemCreationClassName(),CMPI_chars);
      CMSetProperty(in,"SystemName",SystemName(),CMPI_chars);
      CMSetProperty(in,"Name",svc->svName,CMPI_chars);
      CMSetProperty(in,"Status",svc->svStatus,CMPI_chars);
      CMSetProperty(in,"Started",&svc->svStarted,CMPI_boolean);
    }
  }

  return in;
}

