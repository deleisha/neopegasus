//%2003////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002  BMC Software, Hewlett-Packard Development
// Company, L. P., IBM Corp., The Open Group, Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L. P.;
// IBM Corp.; EMC Corporation, The Open Group.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE INCLUDED IN
// ALL COPIES OR SUBSTANTIAL PORTIONS OF THE SOFTWARE. THE SOFTWARE IS PROVIDED
// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//==============================================================================
//
// Author:      Adrian Schuur, schuur@de.ibm.com
//
// Modified By:	Seema Gupta (gseema@in.ibm.com) for PEP135
//
//%/////////////////////////////////////////////////////////////////////////////

#include "CMPI_Version.h"

#include "CMPIProviderManager.h"

#include "CMPI_Object.h"
#include "CMPI_ContextArgs.h"
#include "CMPI_Instance.h"
#include "CMPI_ObjectPath.h"
#include "CMPI_Result.h"
#include "CMPI_SelectExp.h"

#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/Destroyer.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/MessageLoader.h> //l10n
#include <Pegasus/Common/Constants.h>

#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Server/CIMServer.h>

#include <Pegasus/ProviderManager2/ProviderType.h>
#include <Pegasus/ProviderManager2/ProviderName.h>
#include <Pegasus/ProviderManager2/CMPI/CMPIProvider.h>
#include <Pegasus/ProviderManager2/ProviderManagerService.h>
//#include <Pegasus/ProviderManager2/Default/OperationResponseHandler.h>

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
#include <Pegasus/ProviderManager2/ProviderManagerzOS_inline.h>
#endif

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

int _cmpi_trace=0;

#define DDD(x) if (_cmpi_trace) x;

CMPIProviderManager::IndProvTab    CMPIProviderManager::provTab;
CMPIProviderManager::IndSelectTab  CMPIProviderManager::selxTab;
CMPIProviderManager::ProvRegistrar CMPIProviderManager::provReg;

class CMPIPropertyList {
   char **props;
   int pCount;
  public:
   CMPIPropertyList(CIMPropertyList &propertyList) {
      if (!propertyList.isNull()) {
        Array<CIMName> p=propertyList.getPropertyNameArray();
        pCount=p.size();
        props=(char**)malloc((1+pCount)*sizeof(char*));
        for (int i=0; i<pCount; i++) {
           props[i]=strdup(p[i].getString().getCString());
        }
        props[pCount]=NULL;
      }
      else props=NULL;
   }
   ~CMPIPropertyList() {
      if (props) {
         for (int i=0; i<pCount; i++)
            free(props[i]);
         free(props);
      }
   }
   char **getList() {
      return props;
   }
};

CMPIProviderManager::CMPIProviderManager(Mode m)
{
   mode=m;
   if (getenv("CMPI_TRACE")) _cmpi_trace=1;
   else _cmpi_trace=0;
   _repository=ProviderManagerService::getRepository();
}

CMPIProviderManager::~CMPIProviderManager(void)
{
}

Boolean CMPIProviderManager::insertProvider(const ProviderName & name,
            const String &ns, const String &cn)
{
    String key(ns+String("::")+cn+String("::")+CIMValue(name.getCapabilitiesMask()).toString());
    return provReg.insert(key,name);
}


Message * CMPIProviderManager::processMessage(Message * request)
{
      PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "CMPIProviderManager::processMessage()");

    Message * response = 0;

    // pass the request message to a handler method based on message type
    switch(request->getType())
    {
    case CIM_GET_INSTANCE_REQUEST_MESSAGE:
        response = handleGetInstanceRequest(request);

        break;
    case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
        response = handleEnumerateInstancesRequest(request);

        break;
    case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
        response = handleEnumerateInstanceNamesRequest(request);

        break;
    case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
        response = handleCreateInstanceRequest(request);

        break;
    case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
        response = handleModifyInstanceRequest(request);

        break;
    case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
        response = handleDeleteInstanceRequest(request);

        break;
    case CIM_EXEC_QUERY_REQUEST_MESSAGE:
        response = handleExecQueryRequest(request);

        break;
    case CIM_ASSOCIATORS_REQUEST_MESSAGE:
        response = handleAssociatorsRequest(request);

        break;
    case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
        response = handleAssociatorNamesRequest(request);

        break;
    case CIM_REFERENCES_REQUEST_MESSAGE:
        response = handleReferencesRequest(request);

        break;
    case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
        response = handleReferenceNamesRequest(request);

        break;
    case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
        response = handleInvokeMethodRequest(request);

        break;
    case CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleCreateSubscriptionRequest(request);

        break;
/*    case CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleModifySubscriptionRequest(request);

        break;
*/  case CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleDeleteSubscriptionRequest(request);

        break;
    case CIM_ENABLE_INDICATIONS_REQUEST_MESSAGE:
        response = handleEnableIndicationsRequest(request);

        break;
    case CIM_DISABLE_INDICATIONS_REQUEST_MESSAGE:
        response = handleDisableIndicationsRequest(request);

        break;
/*    case CIM_EXPORT_INDICATION_REQUEST_MESSAGE:
        response = handleExportIndicationRequest(request);
        break;
*/
    case CIM_DISABLE_MODULE_REQUEST_MESSAGE:
        response = handleDisableModuleRequest(request);

        break;
    case CIM_ENABLE_MODULE_REQUEST_MESSAGE:
        response = handleEnableModuleRequest(request);

        break;
    case CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE:
        response = handleStopAllProvidersRequest(request);

        break;
    case CIM_INITIALIZE_PROVIDER_REQUEST_MESSAGE:
	response = handleInitializeProviderRequest(request);

	break;
    default:
        response = handleUnsupportedRequest(request);

        break;
    }

    PEG_METHOD_EXIT();

    return(response);
}

Boolean CMPIProviderManager::hasActiveProviders()
{
     return providerManager.hasActiveProviders();
}

void CMPIProviderManager::unloadIdleProviders()
{
     providerManager.unloadIdleProviders();
}


#define CHARS(cstring) (char*)(strlen(cstring)?(const char*)cstring:NULL)


#define HandlerIntroBase(type,type1,message,request,response,handler,respType) \
    CIM##type##RequestMessage * request = \
        dynamic_cast<CIM##type##RequestMessage *>(const_cast<Message *>(message)); \
    PEGASUS_ASSERT(request != 0); \
    CIM##type##ResponseMessage * response = \
        new CIM##type##ResponseMessage( \
        request->messageId, \
        CIMException(), \
        request->queueIds.copyAndPop() \
        respType \
    PEGASUS_ASSERT(response != 0); \
    response->setKey(request->getKey()); \
    response->setHttpMethod(request->getHttpMethod()); \
    type1##ResponseHandler handler(request, response);

#define VOIDINTRO );
#define NOVOIDINTRO(type) ,type);
#define METHODINTRO ,CIMValue(), Array<CIMParamValue>(), request->methodName );


#define HandlerIntroVoid(type,message,request,response,handler) \
     HandlerIntroBase(type,type,message,request,response,handler,VOIDINTRO)

#define HandlerIntroMethod(type,message,request,response,handler) \
     HandlerIntroBase(type,type,message,request,response,handler,METHODINTRO)

#define HandlerIntroInd(type,message,request,response,handler) \
     HandlerIntroBase(type,Operation,message,request,response,handler,VOIDINTRO)

#define HandlerIntroInit(type,message,request,response,handler) \
     HandlerIntroBase(type,Operation,message,request,response,handler,VOIDINTRO)

#define HandlerIntro(type,message,request,response,handler,respType) \
     HandlerIntroBase(type,type,message,request,response,handler,NOVOIDINTRO(respType))

#define HandlerCatch(handler) \
    catch(CIMException & e)  \
    { PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, \
                "Exception: " + e.getMessage()); \
        handler.setStatus(e.getCode(), e.getContentLanguages(), e.getMessage()); \
    } \
    catch(Exception & e) \
    { PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, \
                "Exception: " + e.getMessage()); \
        handler.setStatus(CIM_ERR_FAILED, e.getContentLanguages(), e.getMessage()); \
    } \
    catch(...) \
    { PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4, \
                "Exception: Unknown"); \
        handler.setStatus(CIM_ERR_FAILED, "Unknown error."); \
    }



Message * CMPIProviderManager::handleGetInstanceRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleGetInstanceRequest");

    HandlerIntro(GetInstance,message,request,response,handler,CIMInstance());

    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CmpiProviderManager::handleGetInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->instanceName.getClassName(),
            request->instanceName.getKeyBindings());

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        // resolve provider name
        ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
        ProviderName name = _resolveProviderName(pidc);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getRemoteProvider(name.getLocation(), name.getLogicalName());
	}
	else {
        // get cached or load new provider module
           ph = providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());
        }

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));
        // forward request
        CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.getInstance: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::getInstance"<<endl);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        CMPIPropertyList props(request->propertyList);

        CMPIFlags flgs=0;
        if (request->includeQualifiers) flgs|=CMPI_FLAG_IncludeQualifiers;
        if (request->includeClassOrigin) flgs|=CMPI_FLAG_IncludeClassOrigin;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMGetInstanceRequestMessage * req = dynamic_cast<CIMGetInstanceRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif

        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->getInstance
        (pr.miVector.instMI,&eCtx,&eRes,&eRef,props.getList());

        STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif
        if (rc.rc!=CMPI_RC_OK)
           throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleEnumerateInstancesRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleEnumerateInstanceRequest");

    HandlerIntro(EnumerateInstances,message,request,response,
                 handler,Array<CIMInstance>());
    try {
      Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleEnumerateInstancesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->className.getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->className);

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        // resolve provider name
        ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
        ProviderName name = _resolveProviderName(pidc);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getRemoteProvider(name.getLocation(), name.getLogicalName());
	}
	else {
        // get cached or load new provider module
           ph = providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());
        }

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        CIMPropertyList propertyList(request->propertyList);

        // forward request
        CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.enumerateInstances: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::enumerateInstances"<<endl);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        CMPIPropertyList props(propertyList);

        CMPIFlags flgs=0;
        if (request->includeQualifiers) flgs|=CMPI_FLAG_IncludeQualifiers;
        if (request->includeClassOrigin) flgs|=CMPI_FLAG_IncludeClassOrigin;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMEnumerateInstancesRequestMessage * req = dynamic_cast<CIMEnumerateInstancesRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif

        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->enumInstances
        (pr.miVector.instMI,&eCtx,&eRes,&eRef,props.getList());

        STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
        throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);

        STAT_PMS_PROVIDEREND;
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleEnumerateInstanceNamesRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleEnumerateInstanceNamesRequest");

    HandlerIntro(EnumerateInstanceNames,message,request,response,
                 handler,Array<CIMObjectPath>());
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleEnumerateInstanceNamesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->className.getString());

       // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->className);

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        // resolve provider name
        ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
        ProviderName name = _resolveProviderName(pidc);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getRemoteProvider(name.getLocation(), name.getLogicalName());
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());
        }

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));
        CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.enumerateInstanceNames: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::enumerateInstanceNames"<<endl);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMEnumerateInstanceNamesRequestMessage * req = dynamic_cast<CIMEnumerateInstanceNamesRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif
        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->enumInstanceNames(pr.miVector.instMI,&eCtx,&eRes,&eRef);

        STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
           throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);


    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleCreateInstanceRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleCreateInstanceRequest");

    HandlerIntro(CreateInstance,message,request,response,
                 handler,CIMObjectPath());
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleCreateInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->newInstance.getPath().getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->newInstance.getPath().getClassName(),
            request->newInstance.getPath().getKeyBindings());
	request->newInstance.setPath(objectPath);

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        // resolve provider name
        ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
        ProviderName name = _resolveProviderName(pidc);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getRemoteProvider(name.getLocation(), name.getLogicalName());
	}
	else {
        // get cached or load new provider module
           ph = providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());
        }

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));
        // forward request
        CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.createInstance: " +
            ph.GetProvider().getName());

        DDD(cerr<<"--- CMPIProviderManager::createInstances"<<endl);
        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_InstanceOnStack eInst(request->newInstance);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMCreateInstanceRequestMessage * req = dynamic_cast<CIMCreateInstanceRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif

        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->createInstance
           (pr.miVector.instMI,&eCtx,&eRes,&eRef,&eInst);

        STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
           throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleModifyInstanceRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleModifyInstanceRequest");

    HandlerIntroVoid(ModifyInstance,message,request,response,
                 handler);
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleModifyInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->modifiedInstance.getPath().getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->modifiedInstance.getPath ().getClassName(),
            request->modifiedInstance.getPath ().getKeyBindings());

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        // resolve provider name
        ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
        ProviderName name = _resolveProviderName(pidc);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getRemoteProvider(name.getLocation(), name.getLogicalName());
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());
        }

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));
        // forward request
        CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.modifyInstance: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::modifyInstance"<<endl);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_InstanceOnStack eInst(request->modifiedInstance);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        CMPIPropertyList props(request->propertyList);

        CMPIFlags flgs=0;
        if (request->includeQualifiers) flgs|=CMPI_FLAG_IncludeQualifiers;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMModifyInstanceRequestMessage * req = dynamic_cast<CIMModifyInstanceRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif

        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->setInstance
        (pr.miVector.instMI,&eCtx,&eRes,&eRef,&eInst,props.getList());

        STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
           throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleDeleteInstanceRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleDeleteInstanceRequest");

    HandlerIntroVoid(DeleteInstance,message,request,response,
                 handler);
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleDeleteInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->instanceName.getClassName(),
            request->instanceName.getKeyBindings());

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        // resolve provider name
        ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
        ProviderName name = _resolveProviderName(pidc);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getRemoteProvider(name.getLocation(), name.getLogicalName());
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());
        }

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));
        // forward request
        CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.deleteInstance: " + pr.getName());

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMDeleteInstanceRequestMessage * req = dynamic_cast<CIMDeleteInstanceRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif

        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->deleteInstance
           (pr.miVector.instMI,&eCtx,&eRes,&eRef);

        STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
           throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleExecQueryRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleExecQueryRequest");

    HandlerIntro(ExecQuery,message,request,response,
                 handler,Array<CIMObject>());

    try {
      Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::ExecQueryRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->className.getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->className);

        Boolean remote=false;

        CMPIProvider::OpProviderHolder ph;

        // resolve provider name
        ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
        ProviderName name = _resolveProviderName(pidc);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getRemoteProvider(name.getLocation(), name.getLogicalName());
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());
        }

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        // forward request
        CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.execQuery: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::execQuery"<<endl);

        const char **props=NULL;

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);
        const CString queryLan=request->queryLanguage.getCString();
        const CString query=request->query.getCString();

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMExecQueryRequestMessage * req = dynamic_cast<CIMExecQueryRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif

        STAT_GETSTARTTIME;

        rc=pr.miVector.instMI->ft->execQuery
           (pr.miVector.instMI,&eCtx,&eRes,&eRef,CHARS(queryLan),CHARS(query));

        STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
           throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);

        STAT_PMS_PROVIDEREND;
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleAssociatorsRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleAssociatorsRequest");

    HandlerIntro(Associators,message,request,response,
                 handler,Array<CIMObject>());
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleAssociatorsRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->objectName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->objectName.getClassName());

        objectPath.setKeyBindings(request->objectName.getKeyBindings());

        CIMObjectPath assocPath(
            System::getHostName(),
            request->nameSpace,
            request->assocClass.getString());

        Boolean remote=false;

        CMPIProvider::OpProviderHolder ph;

        // resolve provider name
        ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
        ProviderName name = _resolveProviderName(pidc);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getRemoteProvider(name.getLocation(), name.getLogicalName());
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());
        }

       // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        // forward request
        CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.associators: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::associators"<<" role: >"<<request->role<<"< aCls "<<
        request->assocClass<<endl);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);
        const CString aClass=request->assocClass.getString().getCString();
        const CString rClass=request->resultClass.getString().getCString();
        const CString rRole=request->role.getCString();
        const CString resRole=request->resultRole.getCString();

        CMPIPropertyList props(request->propertyList);

        CMPIFlags flgs=0;
        if (request->includeQualifiers) flgs|=CMPI_FLAG_IncludeQualifiers;
        if (request->includeClassOrigin) flgs|=CMPI_FLAG_IncludeClassOrigin;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMAssociatorsRequestMessage * req = dynamic_cast<CIMAssociatorsRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif

        STAT_GETSTARTTIME;

        rc=pr.miVector.assocMI->ft->associators(
                         pr.miVector.assocMI,&eCtx,&eRes,&eRef,CHARS(aClass),
                         CHARS(rClass),CHARS(rRole),CHARS(resRole),props.getList());

        STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
           throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleAssociatorNamesRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleAssociatorNamesRequest");

    HandlerIntro(AssociatorNames,message,request,response,
                 handler,Array<CIMObjectPath>());
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleAssociatorNamesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->objectName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->objectName.getClassName());

        objectPath.setKeyBindings(request->objectName.getKeyBindings());

        CIMObjectPath assocPath(
            System::getHostName(),
            request->nameSpace,
            request->assocClass.getString());

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        // resolve provider name
        ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
        ProviderName name = _resolveProviderName(pidc);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getRemoteProvider(name.getLocation(), name.getLogicalName());
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());
        }

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        // forward request
        CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.associatorNames: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::associatorNames"<<" role: >"<<request->role<<"< aCls "<<
           request->assocClass<<endl);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);
        const CString aClass=request->assocClass.getString().getCString();
        const CString rClass=request->resultClass.getString().getCString();
        const CString rRole=request->role.getCString();
        const CString resRole=request->resultRole.getCString();

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMAssociatorNamesRequestMessage * req = dynamic_cast<CIMAssociatorNamesRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif

        STAT_GETSTARTTIME;

        rc=pr.miVector.assocMI->ft->associatorNames(
                         pr.miVector.assocMI,&eCtx,&eRes,&eRef,CHARS(aClass),
                         CHARS(rClass),CHARS(rRole),CHARS(resRole));

        STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
           throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleReferencesRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "CMPIProviderManager::handleReferencesRequest");

    HandlerIntro(References,message,request,response,
                 handler,Array<CIMObject>());
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleReferencesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->objectName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->objectName.getClassName());

        objectPath.setKeyBindings(request->objectName.getKeyBindings());

        CIMObjectPath resultPath(
            System::getHostName(),
            request->nameSpace,
            request->resultClass.getString());

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        // resolve provider name
        ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
        ProviderName name = _resolveProviderName(pidc);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getRemoteProvider(name.getLocation(), name.getLogicalName());
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());
        }

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));
        // forward request
        CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.references: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::references"<<" role: >"<<request->role<<"< aCls "<<
        request->resultClass<<endl);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);
        const CString rClass=request->resultClass.getString().getCString();
        const CString rRole=request->role.getCString();

        CMPIPropertyList props(request->propertyList);

        CMPIFlags flgs=0;
        if (request->includeQualifiers) flgs|=CMPI_FLAG_IncludeQualifiers;
        if (request->includeClassOrigin) flgs|=CMPI_FLAG_IncludeClassOrigin;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMReferencesRequestMessage * req = dynamic_cast<CIMReferencesRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif

        STAT_GETSTARTTIME;

        rc=pr.miVector.assocMI->ft->references(
                         pr.miVector.assocMI,&eCtx,&eRes,&eRef,
                         CHARS(rClass),CHARS(rRole),props.getList());

        STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
            throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleReferenceNamesRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleReferenceNamesRequest");

    HandlerIntro(ReferenceNames,message,request,response,
                 handler,Array<CIMObjectPath>());
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleReferenceNamesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->objectName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->objectName.getClassName());

        objectPath.setKeyBindings(request->objectName.getKeyBindings());

        CIMObjectPath resultPath(
            System::getHostName(),
            request->nameSpace,
            request->resultClass.getString());

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        // resolve provider name
        ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
        ProviderName name = _resolveProviderName(pidc);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getRemoteProvider(name.getLocation(), name.getLogicalName());
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());
        }

        // convert arguments
        OperationContext context;

		context.insert(request->operationContext.get(IdentityContainer::NAME));
		context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
	    context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));
        CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.referenceNames: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::referenceNames"<<" role: >"<<request->role<<"< aCls "<<
           request->resultClass<<endl);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);
        const CString rClass=request->resultClass.getString().getCString();
        const CString rRole=request->role.getCString();

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMReferenceNamesRequestMessage * req = dynamic_cast<CIMReferenceNamesRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif

        STAT_GETSTARTTIME;

        rc=pr.miVector.assocMI->ft->referenceNames(
                         pr.miVector.assocMI,&eCtx,&eRes,&eRef,
                         CHARS(rClass),CHARS(rRole));

        STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
           throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleInvokeMethodRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleInvokeMethodRequest");

    HandlerIntroMethod(InvokeMethod,message,request,response,
                 handler);
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleInvokeMethodRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->instanceName.getClassName(),
            request->instanceName.getKeyBindings());

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        // resolve provider name
        ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
        ProviderName name = _resolveProviderName(pidc);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getRemoteProvider(name.getLocation(), name.getLogicalName());
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());
        }

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        CIMObjectPath instanceReference(request->instanceName);

        // ATTN: propagate namespace
        instanceReference.setNameSpace(request->nameSpace);

        // forward request
        CMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.invokeMethod: " + pr.getName());

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ObjectPathOnStack eRef(objectPath);
        CMPI_ResultOnStack eRes(handler,&pr.broker);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);
        CMPI_ArgsOnStack eArgsIn(request->inParameters);
        Array<CIMParamValue> outArgs;
        CMPI_ArgsOnStack eArgsOut(outArgs);
        CString mName=request->methodName.getString().getCString();

        CMPIFlags flgs=0;
        eCtx.ft->addEntry(&eCtx,CMPIInvocationFlags,(CMPIValue*)&flgs,CMPI_uint32);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMInvokeMethodRequestMessage * req = dynamic_cast<CIMInvokeMethodRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif
        STAT_GETSTARTTIME;

        rc=pr.miVector.methMI->ft->invokeMethod(
           pr.miVector.methMI,&eCtx,&eRes,&eRef,CHARS(mName),&eArgsIn,&eArgsOut);

        STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
           throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);

       for (int i=0,s=outArgs.size(); i<s; i++)
           handler.deliverParamValue(outArgs[i]);
       handler.complete();
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

int LocateIndicationProviderNames(const CIMInstance& pInstance, const CIMInstance& pmInstance,
                                  String& providerName, String& location)
{
    Uint32 pos = pInstance.findProperty(CIMName ("Name"));
    pInstance.getProperty(pos).getValue().get(providerName);

    pos = pmInstance.findProperty(CIMName ("Location"));
    pmInstance.getProperty(pos).getValue().get(location);
    return 0;
}

Message * CMPIProviderManager::handleCreateSubscriptionRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleCreateSubscriptionRequest");

    HandlerIntroInd(CreateSubscription,message,request,response,
                 handler);
    try {
        const CIMObjectPath &x=request->subscriptionInstance.getPath();
        CIMInstance req_provider, req_providerModule;
        ProviderIdContainer pidc = (ProviderIdContainer)request->operationContext.get(ProviderIdContainer::NAME);
        req_provider = pidc.getProvider();
        req_providerModule = pidc.getModule();

        String providerName,providerLocation;
        LocateIndicationProviderNames(req_provider, req_providerModule,
        providerName,providerLocation);

        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleCreateSubscriptionRequest - Host name: $0  Name space: $1  Provider name(s): $2",
            System::getHostName(),
            request->nameSpace.getString(),
            providerName);

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        String fileName = _resolvePhysicalName(providerLocation);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getProvider("CMPIRProxyProvider", providerName);
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(fileName, providerName);
        }

        indProvRecord *prec=NULL;
        provTab.lookup(providerName,prec);
        if (prec) prec->count++;
        else {
           prec=new indProvRecord();
           provTab.insert(providerName,prec);
        }

        indSelectRecord *srec=new indSelectRecord();
        const CIMObjectPath &sPath=request->subscriptionInstance.getPath();
        selxTab.insert(sPath.toString(),srec);

        // convert arguments
        OperationContext *context=new OperationContext();
        context->insert(request->operationContext.get(IdentityContainer::NAME));
        context->insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context->insert(request->operationContext.get(ContentLanguageListContainer::NAME));
        context->insert(request->operationContext.get(SubscriptionInstanceContainer::NAME));
        context->insert(request->operationContext.get(SubscriptionLanguageListContainer::NAME));
        context->insert(request->operationContext.get(SubscriptionFilterConditionContainer::NAME));

        CIMObjectPath subscriptionName = request->subscriptionInstance.getPath();

        CMPIProvider & pr=ph.GetProvider();

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(*context);
        CMPI_SelectExp *eSelx=new CMPI_SelectExp(*context,
        request->query,
        request->queryLanguage);
        srec->eSelx=eSelx;
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.createSubscriptionRequest: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::createSubscriptionRequest"<<endl);

        for(Uint32 i = 0, n = request->classNames.size(); i < n; i++) {
            CIMObjectPath className(
                System::getHostName(),
                request->nameSpace,
                request->classNames[i]);
            eSelx->classNames.append(className);
        }
        CMPI_ObjectPathOnStack eRef(eSelx->classNames[0]);

        CIMPropertyList propertyList = request->propertyList;
        if (!propertyList.isNull()) {
           Array<CIMName> p=propertyList.getPropertyNameArray();
           int pCount=p.size();
           eSelx->props=(const char**)malloc((1+pCount)*sizeof(char*));
           for (int i=0; i<pCount; i++) {
              eSelx->props[i]=strdup(p[i].getString().getCString());
           }
           eSelx->props[pCount]=NULL;
        }

        Uint16 repeatNotificationPolicy = request->repeatNotificationPolicy;

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMCreateSubscriptionRequestMessage * req = dynamic_cast<CIMCreateSubscriptionRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif
        STAT_GETSTARTTIME;

        rc=pr.miVector.indMI->ft->activateFilter(
           pr.miVector.indMI,&eCtx,NULL,eSelx,
           CHARS(eSelx->classNames[0].getClassName().getString().getCString()),
           &eRef,false);

       STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
           throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleDeleteSubscriptionRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleDeleteSubscriptionRequest");

    HandlerIntroInd(DeleteSubscription,message,request,response,
                 handler);
    try {
        String providerName,providerLocation;

        CIMInstance req_provider, req_providerModule;
        ProviderIdContainer pidc = (ProviderIdContainer)request->operationContext.get(ProviderIdContainer::NAME);
        req_provider = pidc.getProvider();
        req_providerModule = pidc.getModule();

        LocateIndicationProviderNames(req_provider, req_providerModule,
           providerName,providerLocation);

        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "CMPIProviderManager::handleDeleteSubscriptionRequest - Host name: $0  Name space: $1  Provider name(s): $2",
            System::getHostName(),
            request->nameSpace.getString(),
            providerName);

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        String fileName = _resolvePhysicalName(providerLocation);

        if ((remote=pidc.isRemoteNameSpace())) {
           ph = providerManager.getProvider("CMPIRProxyProvider", providerName);
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(fileName, providerName);
        }


        indProvRecord *prec=NULL;
        provTab.lookup(providerName,prec);
        if (--prec->count<=0) {
           provTab.remove(providerName);
           prec=NULL;
        }

        indSelectRecord *srec=NULL;
        const CIMObjectPath &sPath=request->subscriptionInstance.getPath();
        String sPathString=sPath.toString();
        selxTab.lookup(sPathString,srec);

        CMPI_SelectExp *eSelx=srec->eSelx;
        CMPI_ObjectPathOnStack eRef(eSelx->classNames[0]);
        selxTab.remove(sPathString);

        // convert arguments
        OperationContext context;
        context.insert(request->operationContext.get(IdentityContainer::NAME));
        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));
        context.insert(request->operationContext.get(SubscriptionInstanceContainer::NAME));
        context.insert(request->operationContext.get(SubscriptionLanguageListContainer::NAME));

        CIMObjectPath subscriptionName = request->subscriptionInstance.getPath();

        CMPIProvider & pr=ph.GetProvider();

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.deleteSubscriptionRequest: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::deleteSubscriptionRequest"<<endl);

        if (remote) {
           CString info=pidc.getRemoteInfo().getCString();
           eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",(CMPIValue*)(const char*)info,CMPI_chars);
        }

        CMPIProvider::pm_service_op_lock op_lock(&pr);

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                CIMDeleteSubscriptionRequestMessage * req = dynamic_cast<CIMDeleteSubscriptionRequestMessage *>(const_cast<Message *>(message));
                int err_num=enablePThreadSecurity(req->userName);
                if (err_num!=0)
                {
                        // need a new CIMException for this
                        throw CIMException(CIM_ERR_ACCESS_DENIED,String(strerror(err_num)));
                }
#endif
        STAT_GETSTARTTIME;
        rc=pr.miVector.indMI->ft->deActivateFilter(
           pr.miVector.indMI,&eCtx,NULL,eSelx,
           CHARS(eSelx->classNames[0].getClassName().getString().getCString()),
           &eRef,prec==NULL);

       delete eSelx;

       STAT_PMS_PROVIDEREND;

#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
                disablePThreadSecurity(req->userName);
#endif

        if (rc.rc!=CMPI_RC_OK)
           throw CIMException((CIMStatusCode)rc.rc,
               rc.msg ? CMGetCharsPtr(rc.msg,NULL) : String::EMPTY);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleEnableIndicationsRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager:: handleEnableIndicationsRequest");

    HandlerIntroInd(EnableIndications,message,request,response,
                 handler);
    try {
        String providerName,providerLocation;
		CIMInstance req_provider, req_providerModule;
		ProviderIdContainer pidc = (ProviderIdContainer)request->operationContext.get(ProviderIdContainer::NAME);
		req_provider = pidc.getProvider();
		req_providerModule = pidc.getModule();

        LocateIndicationProviderNames(req_provider, req_providerModule,
           providerName,providerLocation);

        indProvRecord *provRec;
        if (provTab.lookup(providerName,provRec)) {
           provRec->enabled=true;
		   ProviderIdContainer pidc = request->operationContext.get(ProviderIdContainer::NAME);
           provRec->handler=new EnableIndicationsResponseHandler(
               request, response, req_provider, _indicationCallback); 
        }

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        String fileName = _resolvePhysicalName(providerLocation);

/*        if ((remote=_repository->isRemoteNameSpace(request->nameSpace,remoteInfo))) {
           ph = providerManager.getProvider("CMPIRProxyProvider", providerName);
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(fileName, providerName);
        } */
        ph = providerManager.getProvider(fileName, providerName);

        // convert arguments
        OperationContext context;

        context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        CMPIProvider & pr=ph.GetProvider();

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.EnableIndicationRequest: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::enableIndicationRequest"<<endl);

        CMPIProvider::pm_service_op_lock op_lock(&pr);
        ph.GetProvider().protect();

        STAT_GETSTARTTIME;

        pr.miVector.indMI->ft->enableIndications(
           pr.miVector.indMI);

       STAT_PMS_PROVIDEREND;
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleDisableIndicationsRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager:: handleDisableIndicationsRequest");

    HandlerIntroInd(DisableIndications,message,request,response,
                 handler);
    try {
        String providerName,providerLocation;
        CIMInstance req_provider, req_providerModule;
        ProviderIdContainer pidc = (ProviderIdContainer)request->operationContext.get(ProviderIdContainer::NAME);
        req_provider = pidc.getProvider();
        req_providerModule = pidc.getModule();

        LocateIndicationProviderNames(req_provider, req_providerModule ,
           providerName,providerLocation);

        indProvRecord *provRec;
        if (provTab.lookup(providerName,provRec)) {
           provRec->enabled=false;
           if (provRec->handler) delete provRec->handler;
           provRec->handler=NULL;
        }

        Boolean remote=false;
        CMPIProvider::OpProviderHolder ph;

        String fileName = _resolvePhysicalName(providerLocation);

/*        if ((remote=_repository->isRemoteNameSpace(request->nameSpace,remoteInfo))) {
           ph = providerManager.getProvider("CMPIRProxyProvider", providerName);
        }
        else {
        // get cached or load new provider module
           ph = providerManager.getProvider(fileName, providerName);
        } */
        ph = providerManager.getProvider(fileName, providerName);

        // convert arguments
        OperationContext context;

		context.insert(request->operationContext.get(AcceptLanguageListContainer::NAME));
	    context.insert(request->operationContext.get(ContentLanguageListContainer::NAME));

        CMPIProvider & pr=ph.GetProvider();

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.DisableIndicationRequest: " + pr.getName());

        DDD(cerr<<"--- CMPIProviderManager::disableIndicationRequest"<<endl);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        pr.miVector.indMI->ft->disableIndications(
           pr.miVector.indMI);

        ph.GetProvider().unprotect();

        STAT_PMS_PROVIDEREND;
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleDisableModuleRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleDisableModuleRequest");

    CIMDisableModuleRequestMessage * request =
        dynamic_cast<CIMDisableModuleRequestMessage *>(const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    // get provider module name
    String moduleName;
    CIMInstance mInstance = request->providerModule;
    Uint32 pos = mInstance.findProperty(CIMName ("Name"));

    if(pos != PEG_NOT_FOUND)
    {
        mInstance.getProperty(pos).getValue().get(moduleName);
    }

    Boolean disableProviderOnly = request->disableProviderOnly;

    Array<Uint16> operationalStatus;
    // Assume success.
    operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_STOPPED);

    //
    // Unload providers
    //
    Array<CIMInstance> _pInstances = request->providers;
    String physicalName=_resolvePhysicalName(request->providerModule.getProperty(
              request->providerModule.findProperty("Location")).getValue().toString());

    for(Uint32 i = 0, n = _pInstances.size(); i < n; i++)
    {
        providerManager.unloadProvider(_pInstances[i].getProperty(
                                          request->providerModule.findProperty
                                          ("Name")).getValue ().toString (),
                                       physicalName);
    }

    CIMDisableModuleResponseMessage * response =
        new CIMDisableModuleResponseMessage(
        request->messageId,
        CIMException(),
        request->queueIds.copyAndPop(),
        operationalStatus);

    PEGASUS_ASSERT(response != 0);

    // preserve message key
    response->setKey(request->getKey());

    //
    //  Set HTTP method in response from request
    //
    response->setHttpMethod (request->getHttpMethod ());

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleEnableModuleRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleEnableModuleRequest");

    CIMEnableModuleRequestMessage * request =
        dynamic_cast<CIMEnableModuleRequestMessage *>(const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    Array<Uint16> operationalStatus;
    operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_OK);

    CIMEnableModuleResponseMessage * response =
        new CIMEnableModuleResponseMessage(
        request->messageId,
        CIMException(),
        request->queueIds.copyAndPop(),
        operationalStatus);

    PEGASUS_ASSERT(response != 0);

    // preserve message key
    response->setKey(request->getKey());

    //  Set HTTP method in response from request
    response->setHttpMethod (request->getHttpMethod ());

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleStopAllProvidersRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleStopAllProvidersRequest");

    CIMStopAllProvidersRequestMessage * request =
        dynamic_cast<CIMStopAllProvidersRequestMessage *>(const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    CIMStopAllProvidersResponseMessage * response =
        new CIMStopAllProvidersResponseMessage(
        request->messageId,
        CIMException(),
        request->queueIds.copyAndPop());

    PEGASUS_ASSERT(response != 0);

    // preserve message key
    response->setKey(request->getKey());

    //  Set HTTP method in response from request
    response->setHttpMethod (request->getHttpMethod ());

    // tell the provider manager to shutdown all the providers
    providerManager.shutdownAllProviders();

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleInitializeProviderRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleInitializeProviderRequest");

    HandlerIntroInit(InitializeProvider,message,request,response,handler);

    try
    {
        // resolve provider name
	ProviderName name = _resolveProviderName(
	    request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        CMPIProvider::OpProviderHolder ph =
           providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());

    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleUnsupportedRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "CMPIProviderManager::handleUnsupportedRequest");

    PEG_METHOD_EXIT();

    // a null response implies unsupported or unknown operation
    return(0);
}

ProviderName CMPIProviderManager::_resolveProviderName(
    const ProviderIdContainer & providerId)
{
    String providerName;
    String fileName;
    String location;
    CIMValue genericValue;

    genericValue = providerId.getProvider().getProperty(
        providerId.getProvider().findProperty("Name")).getValue();
    genericValue.get(providerName);

    genericValue = providerId.getModule().getProperty(
        providerId.getModule().findProperty("Location")).getValue();
    genericValue.get(location);
    fileName = _resolvePhysicalName(location);

    ProviderName name(providerName, fileName, String::EMPTY, 0);
    name.setLocation(location);
    return name;
//    return ProviderName(providerName, fileName, interfaceName, 0);
}

PEGASUS_NAMESPACE_END
