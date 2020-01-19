/*
 * Syslog_LogUtils.h
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
 * Description: CIM Linux Syslog Log Provider
 */


#ifndef SYSLOGLOGUTILS_H
#define SYSLOGLOGUTILS_H

#include <cmpidt.h>

#include "util/sysloglogutil.h"

CMPIObjectPath *makeMessageLogPath(const CMPIBroker *broker, 
                                   const char * classname,
         	                   const char * namespace, 
                                   Syslog_MessageLog *svc);

CMPIObjectPath *makeLogRecordPath(const CMPIBroker *broker, 
                                  const char * classname,
                                  const char * namespace, 
                                  Syslog_LogRecord *svc);

CMPIInstance *makeMessageLogInstance(const CMPIBroker *broker, 
                                     const char * classname,
                                     const char * namespace, 
                                     Syslog_MessageLog *svc);

CMPIInstance *makeLogRecordInstance(const CMPIBroker *broker, 
                                    const char * classname,
                                    const char * namespace, 
                                    Syslog_LogRecord *svc);


#endif
