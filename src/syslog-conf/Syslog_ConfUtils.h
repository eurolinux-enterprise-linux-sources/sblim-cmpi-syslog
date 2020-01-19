/*
 * Syslog_ConfUtils.h
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


#ifndef SYSLOGCONFUTILS_H
#define SYSLOGCONFUTILS_H

#include <cmpidt.h>

#include "util/syslogconfutil.h"

CMPIObjectPath *makeConfPath(const CMPIBroker *broker, 
                             const char *classname, 
                             const char *namespace, 
                             Syslog_Configuration *svc);

CMPIInstance *makeConfInstance(const CMPIBroker *broker, 
                               const char *classname, 
                               const char *namespace, 
                               Syslog_Configuration *svc);

CMPIObjectPath *makeSettingPath(const CMPIBroker *broker, 
                                const char *classname, 
                                const char *namespace, 
                                Syslog_Setting *svc);

CMPIInstance *makeSettingInstance(const CMPIBroker *broker, 
                                    const char *classname, 
                                    const char *namespace, 
                                    Syslog_Setting *svc);

#endif
