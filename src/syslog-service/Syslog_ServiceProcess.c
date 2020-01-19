/*
 * Syslog_ServiceProcess.c
 *
 * © Copyright IBM Corp. 2003, 2007, 2009
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
#include <libgen.h>
#include <stdio.h>

#include "cmpiOSBase_Common.h"
#include "OSBase_Common.h"

#include "Syslog_ServiceUtils.h"
#include "util/syslogserviceutil.h"

#define LOCALCLASSNAME "Syslog_ServiceProcess"
#define ASSOCSERVICE "Service"
#define ASSOCPROCESS "Process"
#define SERVICECLASSNAME "Syslog_Service"
#define PROCESSCLASSNAME "Linux_UnixProcess"

static const CMPIBroker *_broker;

#ifdef CMPI_VER_100
#define Syslog_ServiceProcessSetInstance Syslog_ServiceProcessModifyInstance
#endif

/* ---------------------------------------------------------------------------*/
/* _syslog_makePath_UnixProcess()                                             */
/* ---------------------------------------------------------------------------*/

CMPIObjectPath * _syslog_makePath_UnixProcess( const CMPIObjectPath *cop,
					       char *pid,
					       CMPIStatus *rc) {
  CMPIObjectPath * op = NULL;
 
  syslog_debug(stderr,"--- _syslog_makePath_UnixProcess()\n");

  op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)), 
			"Linux_UnixProcess", rc );
  if( CMIsNullObject(op) ) { 
    CMSetStatusWithChars( _broker, rc, 
			  CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." ); 
    syslog_debug(stderr,"--- _syslog_makePath_UnixProcess() failed: could not create object path\n");
    return op; 
  }

  /* the sblim-cmpi-base package offers some tool methods to get common
   * system data 
  */

  CMAddKey(op, "CSCreationClassName", CSCreationClassName, CMPI_chars);
  CMAddKey(op, "CSName", get_system_name(), CMPI_chars);
  CMAddKey(op, "OSCreationClassName", OSCreationClassName, CMPI_chars);
  CMAddKey(op, "OSName", get_os_name(), CMPI_chars);
  CMAddKey(op, "CreationClassName", "Linux_UnixProcess", CMPI_chars);
  CMAddKey(op, "Handle", pid, CMPI_chars);
  
  syslog_debug(stderr,"--- _syslog_makePath_UnixProcess() exited\n");
  return op;                
}


/* ---------------------------------------------------------------------------*/
/* _syslog_makePath_ServiceProcess()                                          */
/* ---------------------------------------------------------------------------*/

CMPIObjectPath * _syslog_makePath_ServiceProcess( const CMPIObjectPath *cop,
						  const CMPIObjectPath *service,
						  const CMPIObjectPath *process,
						  CMPIStatus *rc) {
  CMPIObjectPath *op = NULL;
 
  syslog_debug(stderr,"--- _syslog_makePath_ServiceProcess()\n");

  op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)), 
			LOCALCLASSNAME, rc);
  if( CMIsNullObject(op) ) { 
    CMSetStatusWithChars( _broker, rc, 
			  CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." ); 
    syslog_debug(stderr,"--- _syslog_makePath_ServiceProcess() failed: could not create object path\n");
    return op; 
  }

  CMAddKey(op, ASSOCSERVICE, (CMPIValue*)&(service), CMPI_ref);
  CMAddKey(op, ASSOCPROCESS, (CMPIValue*)&(process), CMPI_ref);

  syslog_debug(stderr,"--- _syslog_makePath_ServiceProcess() exited\n");
  return op;                
}


/* ---------------------------------------------------------------------------*/
/* _syslog_makeInst_ServiceProcess()                                          */
/* ---------------------------------------------------------------------------*/

CMPIInstance * _syslog_makeInst_ServiceProcess( const CMPIObjectPath *cop,
						const CMPIObjectPath *service,
						const CMPIObjectPath *process,
						CMPIStatus *rc) {
  CMPIObjectPath *op = NULL;
  CMPIInstance   *ci = NULL;
 
  syslog_debug(stderr,"--- _syslog_makeInst_ServiceProcess()\n");

  op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,rc)), 
			LOCALCLASSNAME, rc);
  if( CMIsNullObject(op) ) { 
    CMSetStatusWithChars( _broker, rc, 
			  CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed." ); 
    syslog_debug(stderr,"--- _syslog_makeInst_ServiceProcess() failed: could not create object path\n");
    return ci; 
  }
  ci = CMNewInstance(_broker,op,NULL);
  if (CMIsNullObject(ci)) {
    CMSetStatusWithChars(_broker, rc, CMPI_RC_ERR_FAILED,
			 "Create CMPIInstance failed.");
    syslog_debug(stderr,"--- _syslog_makeInst_ServiceProcess() failed: could not create instance\n");
    return ci;
  }
      
  CMSetProperty(ci, ASSOCSERVICE, (CMPIValue*)&(service), CMPI_ref);
  CMSetProperty(ci, ASSOCPROCESS, (CMPIValue*)&(process), CMPI_ref);

  syslog_debug(stderr,"--- _syslog_makeInst_ServiceProcess() exited\n");
  return ci;                
}


/* ---------------------------------------------------------------------------*/
/* associationHelper()                                                        */
/* ---------------------------------------------------------------------------*/

static CMPIStatus associationHelper( const CMPIContext *ctx,
				     const CMPIResult *rslt,
				     const CMPIObjectPath *cop,
				     int associators,
				     int names)
{
  CMPIStatus      rc         = {CMPI_RC_OK, NULL};
  CMPIObjectPath *op         = NULL;
  CMPIObjectPath *service_op = NULL;
  CMPIObjectPath *syslogd_op = NULL;
  CMPIObjectPath *klogd_op   = NULL;
  CMPIInstance   *ci         = NULL;
  CMPIInstance   *service_ci = NULL;
  CMPIString     *clsname;
  CMPIData        data;
  Syslog_Service  srv;
  void           *hdl        = NULL;
  char            pid[64];

  syslog_debug(stderr,"--- associationHelper()\n");

  //  Check if the object path belongs to a supported class
  clsname = CMGetClassName(cop,NULL);
  if (clsname) {

    // ---------------------------------------------------------------------
    // we have a Process
    if (strcasecmp(PROCESSCLASSNAME,CMGetCharPtr(clsname))==0) {
      data = CMGetKey(cop,"Handle",&rc);
      if(rc.rc!=CMPI_RC_OK) return rc;

      hdl = Syslog_Service_Begin_Enum();
      if(!hdl) {
        CMSetStatusWithChars(_broker, &rc, CMPI_RC_ERR_FAILED,
                             "Could not begin syslog service enumeration.");
        return rc;
      }

      if (Syslog_Service_Next_Enum(hdl,&srv)) {

	// syslog Service is not running
	if(srv.syslogd==0 || srv.klogd==0 ) return rc;

	// incoming Process is one of the syslog processes
	if( srv.syslogd==atoi(CMGetCharPtr(data.value.string)) ||
	    srv.klogd==atoi(CMGetCharPtr(data.value.string))    ) {
	  
	  if( (associators==1 && names==1) || associators==0 ) { 
	    service_op = makeServicePath(_broker, SERVICECLASSNAME,
					 CMGetCharPtr(CMGetNameSpace(cop,NULL)),
					 &srv);
	    if (CMIsNullObject(service_op)) {
	      CMSetStatusWithChars(_broker, &rc, CMPI_RC_ERR_FAILED,
				   "Could not construct Syslog_Service object path");
	      return rc;
	    }
	  }

	  if(associators==1) {
	    // return instance or objectpath of class Syslog_Service
	    if(names==1) {
	      // associatorNames()
	      CMReturnObjectPath(rslt,service_op);
	    }
	    else {
	      // associators()
	      service_ci = makeServiceInstance(_broker, SERVICECLASSNAME, 
					       CMGetCharPtr(CMGetNameSpace(cop,NULL)),
					       &srv);
	      if (CMIsNullObject(service_ci)) {
		CMSetStatusWithChars(_broker, &rc, CMPI_RC_ERR_FAILED,
				     "Could not construct Syslog_Service instance");
		return rc;
	      }
	      CMReturnInstance(rslt,service_ci);
	    }
	  }
	  else {
	    // return instance or objectpath of association 
	    // Syslog_ServiceProcess
	    if(names==1) { 
	      // referenceNames()
	      op = _syslog_makePath_ServiceProcess(cop, service_op, cop, &rc);	 
	      if (CMIsNullObject(op)) {
		CMSetStatusWithChars(_broker, &rc, CMPI_RC_ERR_FAILED,
				     "Could not construct Syslog_ServiceProcess object path");
		return rc;
	      }
	      CMReturnObjectPath(rslt,op);
	    }
	    else {
	      // references()
	      ci = _syslog_makeInst_ServiceProcess(cop, service_op, cop, &rc);
	      if (CMIsNullObject(ci)) {
		CMSetStatusWithChars(_broker, &rc, CMPI_RC_ERR_FAILED,
				     "Could not construct Syslog_ServiceProcess instance");
		return rc;
	      }	
	      CMReturnInstance(rslt,ci);
	    }
	  }
	}
      }
      Syslog_Service_End_Enum(hdl);
    }

    // ---------------------------------------------------------------------
    //  we have a Service
    else if (strcasecmp(SERVICECLASSNAME,CMGetCharPtr(clsname))==0) {

      data = CMGetKey(cop,"Name",&rc);
      if(rc.rc!=CMPI_RC_OK) return rc;

      // Service instance is not syslog
      if(strcasecmp(CMGetCharPtr(data.value.string),"syslog")!=0)
	return rc;

      hdl = Syslog_Service_Begin_Enum();
      if(!hdl) {
        CMSetStatusWithChars(_broker, &rc, CMPI_RC_ERR_FAILED,
                             "Could not begin syslog service enumeration.");
        return rc;
      }

      if (Syslog_Service_Next_Enum(hdl,&srv)) {

	memset(pid,0,sizeof(pid));
	sprintf(pid, "%i", srv.syslogd);
	syslogd_op = _syslog_makePath_UnixProcess(cop, pid, &rc);
	if (CMIsNullObject(syslogd_op)) {
	  CMSetStatusWithChars(_broker, &rc, CMPI_RC_ERR_FAILED,
			       "Could not construct Linux_UnixProcess syslogd object path");
	  return rc;
	}
	memset(pid,0,sizeof(pid));
	sprintf(pid, "%i", srv.klogd);
	klogd_op = _syslog_makePath_UnixProcess(cop, pid, &rc);
	if (CMIsNullObject(klogd_op)) {
	  CMSetStatusWithChars(_broker, &rc, CMPI_RC_ERR_FAILED,
			       "Could not construct Linux_UnixProcess klogd object path");
	  return rc;
	}

	if(associators==1) {
	  // return instance or objectpath of class Linux_UnixProcess

	  if(names==1) {
	    // associatorNames() 
	    CMReturnObjectPath(rslt,syslogd_op);    
	    CMReturnObjectPath(rslt,klogd_op);
	  }
	  else {
	    // associators()
	    ci = CBGetInstance(_broker, ctx, syslogd_op, NULL, &rc);
	    CMReturnInstance(rslt,ci);
	    ci = CBGetInstance(_broker, ctx, klogd_op, NULL, &rc);
	    CMReturnInstance(rslt,ci);
	  }
	}
	else {
	  // return instance or objectpath of association 
	  // Syslog_ServiceProcess
	  if(names==1) { 
	    // referenceNames()	      
	    op = _syslog_makePath_ServiceProcess(cop, cop, syslogd_op, &rc);	 
	    if (CMIsNullObject(op)) {
	      CMSetStatusWithChars(_broker, &rc, CMPI_RC_ERR_FAILED,
				   "Could not construct Syslog_ServiceProcess object path");
	      return rc;
	    }
	    CMReturnObjectPath(rslt,op);
      
	    op = _syslog_makePath_ServiceProcess(cop, cop, klogd_op, &rc);	 
	    if (CMIsNullObject(op)) {
	      CMSetStatusWithChars(_broker, &rc, CMPI_RC_ERR_FAILED,
				   "Could not construct Syslog_ServiceProcess object path");
	      return rc;
	    }
	    CMReturnObjectPath(rslt,op);
	  }
	  else {
	    // references()
	    ci = _syslog_makeInst_ServiceProcess(cop, cop, syslogd_op, &rc);
	    if (CMIsNullObject(ci)) {
	      CMSetStatusWithChars(_broker, &rc, CMPI_RC_ERR_FAILED,
				   "Could not construct Syslog_ServiceProcess instance");
	      return rc;
	    }	
	    CMReturnInstance(rslt,ci);

	    ci = _syslog_makeInst_ServiceProcess(cop, cop, klogd_op, &rc);
	    if (CMIsNullObject(ci)) {
	      CMSetStatusWithChars(_broker, &rc, CMPI_RC_ERR_FAILED,
				   "Could not construct Syslog_ServiceProcess instance");
	      return rc;
	    }	
	    CMReturnInstance(rslt,ci);
	  }
	}
      }
      Syslog_Service_End_Enum(hdl);
    }
  }        
  return rc;
}


/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/

CMPIStatus Syslog_ServiceProcessCleanup( CMPIInstanceMI *mi, 
                                        const CMPIContext *ctx,
					CMPIBoolean terminate) 
{
  CMReturn(CMPI_RC_OK);
}

CMPIStatus Syslog_ServiceProcessEnumInstanceNames( CMPIInstanceMI *mi, 
                                                  const CMPIContext *ctx, 
                                                  const CMPIResult *rslt, 
                                                  const CMPIObjectPath *ref) 
{
  syslog_debug(stderr, "--- Syslog_ServiceProcessEnumInstanceNames()\n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );	
}

CMPIStatus Syslog_ServiceProcessEnumInstances( CMPIInstanceMI *mi, 
                                              const CMPIContext *ctx, 
                                              const CMPIResult *rslt, 
                                              const CMPIObjectPath *ref, 
                                              const char **properties) 
{
  syslog_debug(stderr, "--- Syslog_ServiceProcessEnumInstances()\n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );	
}

CMPIStatus Syslog_ServiceProcessGetInstance( CMPIInstanceMI *mi, 
                                            const CMPIContext *ctx, 
                                            const CMPIResult *rslt, 
                                            const CMPIObjectPath *cop, 
                                            const char **properties) 
{
  syslog_debug(stderr, "--- Syslog_ServiceProcessGetInstance()\n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );	
}

CMPIStatus Syslog_ServiceProcessCreateInstance( CMPIInstanceMI *mi, 
                                               const CMPIContext *ctx, 
                                               const CMPIResult *rslt, 
                                               const CMPIObjectPath *cop, 
                                               const CMPIInstance *ci) 
{
  syslog_debug(stderr, "--- Syslog_ServiceProcessCreateInstance()\n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_ServiceProcessSetInstance( CMPIInstanceMI *mi, 
                                            const CMPIContext *ctx, 
                                            const CMPIResult *rslt, 
                                            const CMPIObjectPath *cop,
                                            const CMPIInstance *ci, 
                                            const char **properties) 
{
  syslog_debug(stderr, "--- Syslog_ServiceProcessSetInstance()\n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_ServiceProcessDeleteInstance( CMPIInstanceMI *mi, 
                                               const CMPIContext *ctx, 
                                               const CMPIResult *rslt, 
                                               const CMPIObjectPath *cop) 
{ 
  syslog_debug(stderr, "--- Syslog_ServiceProcessDeleteInstance()\n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus Syslog_ServiceProcessExecQuery( CMPIInstanceMI *mi, 
                                          const CMPIContext *ctx, 
                                          const CMPIResult *rslt, 
                                          const CMPIObjectPath *cop, 
                                          const char *lang, 
                                          const char *query) 
{
  syslog_debug(stderr, "--- Syslog_ServiceProcessExecQuery()\n");
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}


/* ---------------------------------------------------------------------------*/
/*                      Association Provider Interface                        */
/* ---------------------------------------------------------------------------*/

CMPIStatus Syslog_ServiceProcessAssociationCleanup( CMPIAssociationMI *mi,
                                                    const CMPIContext *ctx,
						    CMPIBoolean terminate)
{
  syslog_debug(stderr, "--- Syslog_ServiceProcessAssociationCleanup()\n");
  CMReturn(CMPI_RC_OK);
}
                                              
CMPIStatus Syslog_ServiceProcessAssociators( CMPIAssociationMI *mi,
                                            const CMPIContext *ctx,
                                            const  CMPIResult *rslt,
                                            const CMPIObjectPath *cop,
                                            const char *assocClass,
                                            const char *resultClass,
                                            const char *role,
                                            const char *resultRole,
                                            const char **propertyList)
{
  CMPIStatus      rc = {CMPI_RC_OK, NULL};
  CMPIObjectPath *op = NULL;

  syslog_debug(stderr, "--- Syslog_ServiceProcessAssociators()\n");

  if(assocClass) {
    op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
			  LOCALCLASSNAME, &rc );
    if(!op) {
      CMSetStatusWithChars( _broker, &rc,
			    CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed.");
      syslog_debug(stderr, "--- Syslog_ServiceProcessAssociators() failed: Create CMPIObjectPath failed.\n");
      return rc;
    }
  }

  if( (!assocClass) || (CMClassPathIsA(_broker,op,assocClass,&rc)==1) ) {

    rc = associationHelper(ctx, rslt, cop, 1, 0);

    if(rc.rc!=CMPI_RC_OK) {
      syslog_debug(stderr, "--- Syslog_ServiceProcessAssociators() failed\n");
      return rc;
    }
  }
  
  CMReturnDone(rslt);
  syslog_debug(stderr, "--- Syslog_ServiceProcessAssociators() exited\n");
  CMReturn(CMPI_RC_OK);
}

CMPIStatus Syslog_ServiceProcessAssociatorNames( CMPIAssociationMI *mi,
                                                const CMPIContext *ctx,
                                                const CMPIResult *rslt,
                                                const CMPIObjectPath *cop,
                                                const char *assocClass,
                                                const char *resultClass,
                                                const char *role,
                                                const char *resultRole )
{
  CMPIStatus      rc = {CMPI_RC_OK, NULL};
  CMPIObjectPath *op = NULL;

  syslog_debug(stderr, "--- Syslog_ServiceProcessAssociatorNames()\n");

  if(assocClass) {
    op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
			  LOCALCLASSNAME, &rc );
    if(!op) {
      CMSetStatusWithChars( _broker, &rc,
			    CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed.");
      syslog_debug(stderr, "--- Syslog_ServiceProcessAssociatorNames() failed: Create CMPIObjectPath failed.\n");
      return rc;
    }
  }

  if( (!assocClass) || (CMClassPathIsA(_broker,op,assocClass,&rc)==1) ) {

    rc = associationHelper(ctx, rslt, cop, 1, 1);

    if(rc.rc!=CMPI_RC_OK) {
      syslog_debug(stderr, "--- Syslog_ServiceProcessAssociatorNames() failed\n");
      return rc;
    }
  }
  
  CMReturnDone(rslt);
  syslog_debug(stderr, "--- Syslog_ServiceProcessAssociatorNames() exited\n");
  CMReturn(CMPI_RC_OK);
}

CMPIStatus Syslog_ServiceProcessReferences( CMPIAssociationMI *mi,
                                           const CMPIContext *ctx,
                                           const CMPIResult *rslt,
                                           const CMPIObjectPath *cop,
                                           const char *assocClass,
                                           const char *role,
                                           const char **propertyList )
{
  CMPIStatus      rc = {CMPI_RC_OK, NULL};
  CMPIObjectPath *op = NULL;

  syslog_debug(stderr, "--- Syslog_ServiceProcessReferences()\n");

  if(assocClass) {
    op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
			  LOCALCLASSNAME, &rc );
    if(!op) {
      CMSetStatusWithChars( _broker, &rc,
			    CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed.");
      syslog_debug(stderr, "--- Syslog_ServiceProcessReferences() failed: Create CMPIObjectPath failed.\n");
      return rc;
    }
  }

  if( (!assocClass) || (CMClassPathIsA(_broker,op,assocClass,&rc)==1) ) {

    rc = associationHelper( ctx, rslt, cop, 0, 0);

    if(rc.rc!=CMPI_RC_OK) {
      syslog_debug(stderr, "--- Syslog_ServiceProcessReferences() failed\n");
      return rc;
    }
  }
  
  CMReturnDone(rslt);
  syslog_debug(stderr, "--- Syslog_ServiceProcessReferences() exited\n");
  CMReturn(CMPI_RC_OK);
}

CMPIStatus Syslog_ServiceProcessReferenceNames( CMPIAssociationMI *mi,
                                               const CMPIContext *ctx,
                                               const CMPIResult *rslt,
                                               const CMPIObjectPath *cop,
                                               const char *assocClass,
                                               const char *role)
{
  CMPIStatus      rc = {CMPI_RC_OK, NULL};
  CMPIObjectPath *op = NULL;

  syslog_debug(stderr, "--- Syslog_ServiceProcessReferenceNames()\n");

  if(assocClass) {
    op = CMNewObjectPath( _broker, CMGetCharPtr(CMGetNameSpace(cop,&rc)),
			  LOCALCLASSNAME, &rc );
    if(!op) {
      CMSetStatusWithChars( _broker, &rc,
			    CMPI_RC_ERR_FAILED, "Create CMPIObjectPath failed.");
      syslog_debug(stderr, "--- Syslog_ServiceProcessReferenceNames() failed: Create CMPIObjectPath failed.\n");
      return rc;
    }
  }

  if( (!assocClass) || (CMClassPathIsA(_broker,op,assocClass,&rc)==1) ) {

    rc = associationHelper( ctx, rslt, cop, 0, 1);

    if(rc.rc!=CMPI_RC_OK) {
      syslog_debug(stderr, "--- Syslog_ServiceProcessReferenceNames() failed\n");
      return rc;
    }
  }
  
  CMReturnDone(rslt);
  syslog_debug(stderr, "--- Syslog_ServiceProcessReferenceNames() exited\n");
  CMReturn(CMPI_RC_OK);
}


/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( Syslog_ServiceProcess,
                  Syslog_ServiceProcess,
                  _broker,
                  CMNoHook);
                                                
CMAssociationMIStub( Syslog_ServiceProcess,
                     Syslog_ServiceProcess,
                     _broker,
                     CMNoHook);

