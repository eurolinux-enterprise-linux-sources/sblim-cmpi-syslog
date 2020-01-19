/*
 * Syslog_ServiceUtils.h
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


#ifndef SYSLOGSERVICEUTILS_H
#define SYSLOGSERVICEUTILS_H

#include "util/syslogserviceutil.h"
#include <cmpidt.h>


CMPIObjectPath *makeServicePath(const CMPIBroker *broker, 
                                const char *classname, 
                                const char *namespace, 
                                Syslog_Service *svc);

CMPIInstance *makeServiceInstance(const CMPIBroker *broker, 
                                  const char *classname, 
                                  const char *namespace, 
                                  Syslog_Service *svc);
#endif
