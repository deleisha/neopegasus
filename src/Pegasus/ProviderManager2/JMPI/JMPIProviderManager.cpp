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
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include "JMPIProviderManager.h"

#include "JMPIImpl.h"

#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/Destroyer.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/MessageLoader.h> //l10n

#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/ProviderManager2/ProviderName.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProvider.h>
#include <Pegasus/ProviderManager2/JMPI/JMPILocalProviderManager.h>
#include <Pegasus/ProviderManager2/ProviderManagerService.h>

#include <Pegasus/Server/ProviderRegistrationManager/ProviderRegistrationManager.h>


PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

static JMPILocalProviderManager providerManager;

int _jmpi_trace=0;

#define DDD(x) if (_jmpi_trace) x;

JMPIProviderManager::IndProvTab    JMPIProviderManager::provTab;
JMPIProviderManager::IndSelectTab  JMPIProviderManager::selxTab;
JMPIProviderManager::ProvRegistrar JMPIProviderManager::provReg;

JMPIProviderManager::JMPIProviderManager(Mode m)
{
   mode=m;
   if (getenv("JMPI_TRACE")) _jmpi_trace=1;
   else _jmpi_trace=0;
   String repositoryRootPath =
	ConfigManager::getHomedPath(
	ConfigManager::getInstance()->getCurrentValue("repositoryDir"));
   _repository = new CIMRepository(repositoryRootPath);
}

JMPIProviderManager::~JMPIProviderManager(void)
{
    delete _repository;
}

Boolean JMPIProviderManager::insertProvider(const ProviderName & name, 
            const String &ns, const String &cn)
{
    String key(ns+String("::")+cn+String("::")+CIMValue(name.getCapabilitiesMask()).toString());
    return provReg.insert(key,name);
}
         
	    
Message * JMPIProviderManager::processMessage(Message * request) throw()
{
      PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "JMPIProviderManager::processMessage()");

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
/*    case CIM_EXEC_QUERY_REQUEST_MESSAGE:
        response = handleExecuteQueryRequest(request);

*/        break;
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
/*    case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
        response = handleInvokeMethodRequest(request);

        break;
    case CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleCreateSubscriptionRequest(request);

        break;
    case CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleModifySubscriptionRequest(request);

        break;
    case CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleDeleteSubscriptionRequest(request);

        break;
    case CIM_ENABLE_INDICATIONS_REQUEST_MESSAGE:
        response = handleEnableIndicationsRequest(request);

        break;
    case CIM_DISABLE_INDICATIONS_REQUEST_MESSAGE:
        response = handleDisableIndicationsRequest(request);

        break;
    case CIM_CONSUME_INDICATION_REQUEST_MESSAGE:
        response = handleConsumeIndicationRequest(request);
        break;

    case CIM_DISABLE_MODULE_REQUEST_MESSAGE:
        response = handleDisableModuleRequest(request);

        break;
    case CIM_ENABLE_MODULE_REQUEST_MESSAGE:
        response = handleEnableModuleRequest(request);

        break;
    case CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE:
        response = handleStopAllProvidersRequest(request);

        break; */
    default:
        std::cout<<"*** Unsupported Request ??"<<request->getType()<<std::endl;
        asm("int $3");
        response = handleUnsupportedRequest(request);

        break;
    }

    PEG_METHOD_EXIT();

    return(response);
}

void JMPIProviderManager::unload_idle_providers(void)
{
     providerManager.unload_idle_providers();
}

#define STRDUPA(s,o) \
   if (s) { \
      o=(const char*)alloca(strlen(s)); \
      strcpy((char*)(o),(s)); \
   } \
   else o=NULL;

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


Message * JMPIProviderManager::handleGetInstanceRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleGetInstanceRequest");

    HandlerIntro(GetInstance,message,request,response,handler,CIMInstance());

    JNIEnv *env=NULL;
    try {
	Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleGetInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->instanceName.getClassName(),
            request->instanceName.getKeyBindings());

        ProviderName name(
            objectPath,
            ProviderType::INSTANCE);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider(name.getPhysicalName(),
	                                name.getLogicalName());
        OperationContext context;

	// forward request
	JMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.getInstance: " + pr.getName());

        DDD(std::cerr<<"--- JMPIProviderManager::getInstance"<<std::endl);

        JvmVector *jv;
        JNIEnv *env=JMPIjvm::attachThread(&jv);
        JMPIjvm::checkException(env);
        jobject jRef=env->NewObject(jv->CIMObjectPathClassRef,
	               jv->CIMObjectPathNewI,(jint)&objectPath);
        JMPIjvm::checkException(env);

        CIMClass cls=pr._cimom_handle->getClass(context,request->nameSpace,
	                       request->instanceName.getClassName(),
                               false,true,true,CIMPropertyList());
        JMPIjvm::checkException(env);
        jobject cc=env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewI,(jint)&cls);
        JMPIjvm::checkException(env);

	JMPIProvider::pm_service_op_lock op_lock(&pr);

	STAT_GETSTARTTIME;

        jmethodID id=env->GetMethodID((jclass)pr.jProviderClass,"getInstance",
           "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;Z)Lorg/pegasus/jmpi/CIMInstance;");
        JMPIjvm::checkException(env);
        jobject inst=env->CallObjectMethod((jobject)pr.jProvider,id,jRef,cc,true);
        
        STAT_PMS_PROVIDEREND;
	
        JMPIjvm::checkException(env);
        handler.processing();       
	if (inst) {
           CIMInstance *ci=((CIMInstance*)env->CallIntMethod(inst,JMPIjvm::jv.CIMInstanceCInst)); 
           handler.deliver(*ci);
        }
        handler.complete();
    } 
    HandlerCatch(handler);
    
    if (env) JMPIjvm::detachThread();
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleEnumerateInstancesRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleEnumerateInstanceRequest");

    HandlerIntro(EnumerateInstances,message,request,response,
                 handler,Array<CIMInstance>());
    JNIEnv *env=NULL;
    try {  
      Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleEnumerateInstancesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->className.getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->className);

        ProviderName name(
            objectPath,
            ProviderType::INSTANCE);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(),
               String::EMPTY);

	// convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        CIMPropertyList propertyList(request->propertyList);

        // forward request
	JMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.enumerateInstances: " + pr.getName());

        DDD(std::cerr<<"--- JMPIProviderManager::enumerateInstances"<<std::endl);

        
	JvmVector *jv;
        env=JMPIjvm::attachThread(&jv);

        jobject jRef=env->NewObject(jv->CIMObjectPathClassRef,
	               jv->CIMObjectPathNewI,(jint)&objectPath);        
	JMPIjvm::checkException(env);
	
        CIMClass cls=pr._cimom_handle->getClass(context,request->nameSpace,
	                       request->className,
                               false,true,true,CIMPropertyList());
	JMPIjvm::checkException(env);
        jobject jCc=env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewI,(jint)&cls);
        JMPIjvm::checkException(env);

	JMPIProvider::pm_service_op_lock op_lock(&pr);

	STAT_GETSTARTTIME;

        jmethodID id=env->GetMethodID((jclass)pr.jProviderClass,"enumInstances",
           "(Lorg/pegasus/jmpi/CIMObjectPath;ZLorg/pegasus/jmpi/CIMClass;Z)Ljava/util/Vector;");
        JMPIjvm::checkException(env);

        jobject jVec=env->CallObjectMethod((jobject)pr.jProvider,id,jRef,false,jCc,true);
        JMPIjvm::checkException(env);
        
        STAT_PMS_PROVIDEREND;
	
        handler.processing();       
	if (jVec) {
	   for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
              JMPIjvm::checkException(env);
	      jobject jInst=env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);
              JMPIjvm::checkException(env);
              CIMInstance inst=*((CIMInstance*)env->CallIntMethod(jInst,JMPIjvm::jv.CIMInstanceCInst)); 
              JMPIjvm::checkException(env);
	      
   //           const CIMObjectPath& op=inst.getPath();
   //           CIMObjectPath iop=inst.buildPath(cls);
   //           iop.setNameSpace(op.getNameSpace());
   //           inst.setPath(iop);
   
              handler.deliver(inst);
 	   }
        }
        handler.complete();
    }
    HandlerCatch(handler);
    
    if (env) JMPIjvm::detachThread();	
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleEnumerateInstanceNamesRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleEnumerateInstanceNamesRequest");

    HandlerIntro(EnumerateInstanceNames,message,request,response,
                 handler,Array<CIMObjectPath>()); 
    JNIEnv *env=NULL;
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleEnumerateInstanceNamesRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->className.getString());
       
       // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->className);

       // build an internal provider name from the request arguments
        ProviderName name(
            objectPath,
            ProviderType::INSTANCE);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName());

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

	JMPIProvider & pr=ph.GetProvider();

	PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.enumerateInstanceNames: " + pr.getName());

        DDD(std::cerr<<"--- JMPIProviderManager::enumerateInstanceNames"<<std::endl);

        
	JvmVector *jv;
        env=JMPIjvm::attachThread(&jv);

        jobject jRef=env->NewObject(jv->CIMObjectPathClassRef,
	               jv->CIMObjectPathNewI,(jint)&objectPath);        
	JMPIjvm::checkException(env);
	
        CIMClass cls=pr._cimom_handle->getClass(context,request->nameSpace,
	                       request->className,
                               false,true,true,CIMPropertyList());
        JMPIjvm::checkException(env);
        jobject jCc=env->NewObject(jv->CIMClassClassRef,jv->CIMClassNewI,(jint)&cls);
        JMPIjvm::checkException(env);

	JMPIProvider::pm_service_op_lock op_lock(&pr);

	STAT_GETSTARTTIME;

        jmethodID id=env->GetMethodID((jclass)pr.jProviderClass,"enumInstances",
           "(Lorg/pegasus/jmpi/CIMObjectPath;ZLorg/pegasus/jmpi/CIMClass;)Ljava/util/Vector;");
        JMPIjvm::checkException(env);

        jobject jVec=env->CallObjectMethod((jobject)pr.jProvider,id,jRef,true,jCc);
        JMPIjvm::checkException(env);
        
        STAT_PMS_PROVIDEREND;
	
        handler.processing();       
	if (jVec) {
	   for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
               JMPIjvm::checkException(env);
	      jobject jCop=env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);
              JMPIjvm::checkException(env);
              CIMObjectPath *cop=((CIMObjectPath*)env->CallIntMethod
	         (jCop,JMPIjvm::jv.CIMObjectPathCInst)); 
              JMPIjvm::checkException(env);
              handler.deliver(*cop);
 	   }
        }
        handler.complete();
    }
    HandlerCatch(handler);
    
    if (env) JMPIjvm::detachThread();	

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleCreateInstanceRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "JMPIProviderManager::handleCreateInstanceRequest");

    HandlerIntro(CreateInstance,message,request,response,
                 handler,CIMObjectPath());
    JNIEnv *env=NULL;
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleCreateInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->newInstance.getPath().getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->newInstance.getPath().getClassName(),
            request->newInstance.getPath().getKeyBindings());

        ProviderName name(
            objectPath,
            ProviderType::INSTANCE);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(),
	           String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
 	JMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.createInstance: " +
            ph.GetProvider().getName());

        DDD(std::cerr<<"--- JMPIProviderManager::createInstances"<<std::endl);
        
	JvmVector *jv;
        env=JMPIjvm::attachThread(&jv);

        jobject jRef=env->NewObject(jv->CIMObjectPathClassRef,
	               jv->CIMObjectPathNewI,(jint)&objectPath);        
	JMPIjvm::checkException(env);
        jobject jInst=env->NewObject(jv->CIMInstanceClassRef,
	               jv->CIMInstanceNewI,(jint)&request->newInstance);        
	JMPIjvm::checkException(env);

	JMPIProvider::pm_service_op_lock op_lock(&pr);

	STAT_GETSTARTTIME;

        jmethodID id=env->GetMethodID((jclass)pr.jProviderClass,"createInstance",
           "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMInstance;)Lorg/pegasus/jmpi/CIMObjectPath;");
        JMPIjvm::checkException(env);

        jobject jCop=env->CallObjectMethod((jobject)pr.jProvider,id,jRef,jInst);
        
        STAT_PMS_PROVIDEREND;
	
        JMPIjvm::checkException(env);
        handler.processing();       
	if (jCop) {
           CIMObjectPath cop=*((CIMObjectPath*)env->CallIntMethod(jCop,JMPIjvm::jv.CIMObjectPathCInst)); 
           handler.deliver(cop);
        }
        handler.complete();
    }
    HandlerCatch(handler);
    
    if (env) JMPIjvm::detachThread();
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleModifyInstanceRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "JMPIProviderManager::handleModifyInstanceRequest");

    HandlerIntroVoid(ModifyInstance,message,request,response,handler);
    JNIEnv *env=NULL;
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "DefaultProviderManager::handleModifyInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->modifiedInstance.getPath().getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->modifiedInstance.getPath ().getClassName(),
            request->modifiedInstance.getPath ().getKeyBindings());

        ProviderName name(
            objectPath,
            ProviderType::INSTANCE);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        CIMPropertyList propertyList(request->propertyList);

        // forward request
 	JMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.modifyInstance: " + pr.getName());

        DDD(std::cerr<<"--- JMPIProviderManager::modifyInstance"<<std::endl);

	JvmVector *jv;
        env=JMPIjvm::attachThread(&jv);

        jobject jRef=env->NewObject(jv->CIMObjectPathClassRef,
	               jv->CIMObjectPathNewI,(jint)&objectPath);        
	JMPIjvm::checkException(env);
        jobject jInst=env->NewObject(jv->CIMInstanceClassRef,
	               jv->CIMInstanceNewI,(jint)&request->modifiedInstance);        
	JMPIjvm::checkException(env);

	JMPIProvider::pm_service_op_lock op_lock(&pr);

	STAT_GETSTARTTIME;

        jmethodID id=env->GetMethodID((jclass)pr.jProviderClass,"setInstance",
           "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMInstance;)V");
        JMPIjvm::checkException(env);

        env->CallVoidMethod((jobject)pr.jProvider,id,jRef,jInst);
        
        STAT_PMS_PROVIDEREND;
	
        JMPIjvm::checkException(env);
        JMPIjvm::detachThread();
    }
    HandlerCatch(handler);
    
    if (env) JMPIjvm::detachThread();
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleDeleteInstanceRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "JMPIProviderManager::handleDeleteInstanceRequest");

    HandlerIntroVoid(DeleteInstance,message,request,response,
                 handler);
    JNIEnv *env=NULL;
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "DefaultProviderManager::handleDeleteInstanceRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->instanceName.getClassName(),
            request->instanceName.getKeyBindings());

        ProviderName name(
            objectPath,
            ProviderType::INSTANCE);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
 	JMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.deleteInstance: " + pr.getName());

        DDD(std::cerr<<"--- JMPIProviderManager::modifyInstance"<<std::endl);

	JvmVector *jv;
        env=JMPIjvm::attachThread(&jv);

        jobject jRef=env->NewObject(jv->CIMObjectPathClassRef,
	               jv->CIMObjectPathNewI,(jint)&objectPath);        
	JMPIjvm::checkException(env);

	JMPIProvider::pm_service_op_lock op_lock(&pr);

	STAT_GETSTARTTIME;

        jmethodID id=env->GetMethodID((jclass)pr.jProviderClass,"deleteInstance",
           "(Lorg/pegasus/jmpi/CIMObjectPath;)V");
        JMPIjvm::checkException(env);

        env->CallVoidMethod((jobject)pr.jProvider,id,jRef);
        
        STAT_PMS_PROVIDEREND;
	
        JMPIjvm::checkException(env);
        JMPIjvm::detachThread();
    }
    HandlerCatch(handler);
    
    if (env) JMPIjvm::detachThread();
    PEG_METHOD_EXIT();

    return(response);
}


Message * JMPIProviderManager::handleAssociatorsRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "JMPIProviderManager::handleAssociatorsRequest");

    HandlerIntro(Associators,message,request,response,
                 handler,Array<CIMObject>());
    JNIEnv *env=NULL;
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleAssociatorsRequest - Host name: $0  Name space: $1  Class name: $2",
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

         ProviderName name(
            assocPath,
            ProviderType::ASSOCIATION);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

       // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
 	JMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.associators: " + pr.getName());

        DDD(std::cerr<<"--- JMPIProviderManager::associators"<<
	               " role: >"<<request->role<<"< aCls "<<
	   request->assocClass<<std::endl);

	JvmVector *jv;
        env=JMPIjvm::attachThread(&jv);

        jobject jRef=env->NewObject(jv->CIMObjectPathClassRef,
	               jv->CIMObjectPathNewI,(jint)&objectPath);        
	JMPIjvm::checkException(env);

        jstring rClass=env->NewStringUTF(request->resultClass.getString().getCString());
        jstring rRole=env->NewStringUTF(request->role.getCString());
        jstring resRole=env->NewStringUTF(request->resultRole.getCString());
	JMPIjvm::checkException(env);

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        jmethodID id=env->GetMethodID((jclass)pr.jProviderClass,"associators",
           "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;"
	   "Ljava/lang/String;ZZ[Ljava/lang/String;)Ljava/util/Vector;");
	   
        JMPIjvm::checkException(env);

        jobject jVec=env->CallObjectMethod((jobject)pr.jProvider,id,jRef,
	                 rClass,rRole,resRole,false,false,NULL);
        JMPIjvm::checkException(env);

        STAT_PMS_PROVIDEREND;

        handler.processing();       
	if (jVec) {
	   for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
              JMPIjvm::checkException(env);
	      jobject jInst=env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);
              JMPIjvm::checkException(env);
              CIMInstance inst=*((CIMInstance*)env->CallIntMethod(jInst,JMPIjvm::jv.CIMInstanceCInst)); 
              JMPIjvm::checkException(env);
	      
	      CIMClass cls=pr._cimom_handle->getClass(context,request->nameSpace,
	             inst.getClassName(),false,true,true,CIMPropertyList());
              const CIMObjectPath& op=inst.getPath();
              CIMObjectPath iop=inst.buildPath(cls);
              iop.setNameSpace(op.getNameSpace());
              inst.setPath(iop);
   
              handler.deliver(inst);
 	   }
        }
        handler.complete();
    }
    HandlerCatch(handler);
    
    if (env) JMPIjvm::detachThread();	
    
    PEG_METHOD_EXIT();

    return(response);
}
	


Message * JMPIProviderManager::handleAssociatorNamesRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "JMPIProviderManager::handleAssociatorNamesRequest");

    HandlerIntro(AssociatorNames,message,request,response,
                 handler,Array<CIMObjectPath>());
    JNIEnv *env=NULL;
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleAssociatorNamesRequest - Host name: $0  Name space: $1  Class name: $2",
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
      
      ProviderName name(
            assocPath,
            ProviderType::ASSOCIATION);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
 	JMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.associatorNames: " + pr.getName());

        DDD(std::cerr<<"--- JMPIProviderManager::associatorNames"<<
	               " role: >"<<request->role<<"< aCls "<<
	   request->assocClass<<std::endl);

	JvmVector *jv;
        env=JMPIjvm::attachThread(&jv);

        jobject jRef=env->NewObject(jv->CIMObjectPathClassRef,
	               jv->CIMObjectPathNewI,(jint)&objectPath);        
	JMPIjvm::checkException(env);
        
        jstring rClass=env->NewStringUTF(request->resultClass.getString().getCString());
        jstring rRole=env->NewStringUTF(request->role.getCString());
        jstring resRole=env->NewStringUTF(request->resultRole.getCString());
	JMPIjvm::checkException(env);

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        jmethodID id=env->GetMethodID((jclass)pr.jProviderClass,"associatorNames",
           "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;"
	   "Ljava/lang/String;)Ljava/util/Vector;");
	   
        JMPIjvm::checkException(env);

        jobject jVec=env->CallObjectMethod((jobject)pr.jProvider,id,jRef,
	                 rClass,rRole,resRole);
        JMPIjvm::checkException(env);

        STAT_PMS_PROVIDEREND;

        handler.processing();       
	if (jVec) {
	   for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
               JMPIjvm::checkException(env);
	      jobject jCop=env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);
              JMPIjvm::checkException(env);
              CIMObjectPath *cop=((CIMObjectPath*)env->CallIntMethod
	         (jCop,JMPIjvm::jv.CIMObjectPathCInst)); 
              JMPIjvm::checkException(env);
              handler.deliver(*cop);
 	   }
        }
        handler.complete();
    }
    HandlerCatch(handler);
    
    if (env) JMPIjvm::detachThread();	
    
    PEG_METHOD_EXIT();

    return(response);
}


Message * JMPIProviderManager::handleReferencesRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "JMPIProviderManager::handleReferencesRequest");

    HandlerIntro(References,message,request,response,
                 handler,Array<CIMObject>());
    JNIEnv *env=NULL;
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "DefaultProviderManager::handleReferencesRequest - Host name: $0  Name space: $1  Class name: $2",
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

        ProviderName name(
            resultPath,
            ProviderType::ASSOCIATION);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        // forward request
 	JMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.references: " + pr.getName());

        DDD(std::cerr<<"--- JMPIProviderManager::references"<<" role: >"<<request->role<<"< aCls "<<
	   request->resultClass<<std::endl);

	JvmVector *jv;
        env=JMPIjvm::attachThread(&jv);

        jobject jRef=env->NewObject(jv->CIMObjectPathClassRef,
	               jv->CIMObjectPathNewI,(jint)&objectPath);        
	JMPIjvm::checkException(env);

        jstring rRole=env->NewStringUTF(request->role.getCString());
	JMPIjvm::checkException(env);

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        jmethodID id=env->GetMethodID((jclass)pr.jProviderClass,"references",
           "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
	   "ZZ[Ljava/lang/String;)Ljava/util/Vector;");
	   
        JMPIjvm::checkException(env);

        jobject jVec=env->CallObjectMethod((jobject)pr.jProvider,id,jRef,
	                 rRole,false,false,NULL);
        JMPIjvm::checkException(env);

        STAT_PMS_PROVIDEREND;

        handler.processing();       
	if (jVec) {
	   for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
              JMPIjvm::checkException(env);
	      jobject jInst=env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);
              JMPIjvm::checkException(env);
              CIMInstance inst=*((CIMInstance*)env->CallIntMethod(jInst,JMPIjvm::jv.CIMInstanceCInst)); 
              JMPIjvm::checkException(env);
	      
	      CIMClass cls=pr._cimom_handle->getClass(context,request->nameSpace,
	             inst.getClassName(),false,true,true,CIMPropertyList());
              const CIMObjectPath& op=inst.getPath();
              CIMObjectPath iop=inst.buildPath(cls);
              iop.setNameSpace(op.getNameSpace());
              inst.setPath(iop);
   
              handler.deliver(inst);
 	   }
        }
        handler.complete();
    }
    HandlerCatch(handler);
    
    if (env) JMPIjvm::detachThread();	
    
    PEG_METHOD_EXIT();

    return(response);
}


Message * JMPIProviderManager::handleReferenceNamesRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleReferenceNamesRequest");

    HandlerIntro(ReferenceNames,message,request,response,
                 handler,Array<CIMObjectPath>());
    JNIEnv *env=NULL;
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleReferenceNamesRequest - Host name: $0  Name space: $1  Class name: $2",
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

        ProviderName name(
            resultPath,
            ProviderType::ASSOCIATION);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

  	JMPIProvider & pr=ph.GetProvider();

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.referenceNames: " + pr.getName());

	                 
        DDD(std::cerr<<"--- JMPIProviderManager::referenceNames"<<" role: >"<<request->role<<"< aCls "<<
	   request->resultClass<<std::endl);

	JvmVector *jv;
        env=JMPIjvm::attachThread(&jv);

        jobject jRef=env->NewObject(jv->CIMObjectPathClassRef,
	               jv->CIMObjectPathNewI,(jint)&objectPath);        
	JMPIjvm::checkException(env);
        
        jstring rRole=env->NewStringUTF(request->role.getCString());
	JMPIjvm::checkException(env);

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        jmethodID id=env->GetMethodID((jclass)pr.jProviderClass,"referenceNames",
           "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;)Ljava/util/Vector;");
	   
        JMPIjvm::checkException(env);

        jobject jVec=env->CallObjectMethod((jobject)pr.jProvider,id,jRef,rRole);
        JMPIjvm::checkException(env);

        STAT_PMS_PROVIDEREND;

        handler.processing();       
	if (jVec) {
	   for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize); i<m; i++) {
               JMPIjvm::checkException(env);
	      jobject jCop=env->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,i);
              JMPIjvm::checkException(env);
              CIMObjectPath *cop=((CIMObjectPath*)env->CallIntMethod
	         (jCop,JMPIjvm::jv.CIMObjectPathCInst)); 
              JMPIjvm::checkException(env);
              handler.deliver(*cop);
 	   }
        }
        handler.complete();
    }
    HandlerCatch(handler);
    
    if (env) JMPIjvm::detachThread();	
    
    PEG_METHOD_EXIT();

    return(response);
}


/*
Message * JMPIProviderManager::handleInvokeMethodRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleInvokeMethodRequest");

    HandlerIntroMethod(InvokeMethod,message,request,response,
                 handler);
    try {
        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleInvokeMethodRequest - Host name: $0  Name space: $1  Class name: $2",
            System::getHostName(),
            request->nameSpace.getString(),
            request->instanceName.getClassName().getString());

        // make target object path
        CIMObjectPath objectPath(
            System::getHostName(),
            request->nameSpace,
            request->instanceName.getClassName(),
            request->instanceName.getKeyBindings());

       ProviderName name(
            objectPath,
            ProviderType::METHOD);

        // resolve provider name
        name = _resolveProviderName(name);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(name.getPhysicalName(), name.getLogicalName(), String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(IdentityContainer(request->userName));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        CIMObjectPath instanceReference(request->instanceName);

        // ATTN: propagate namespace
        instanceReference.setNameSpace(request->nameSpace);

        // forward request
 	JMPIProvider & pr=ph.GetProvider();

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

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.methMI->ft->invokeMethod(
           pr.miVector.methMI,&eCtx,&eRes,&eRef,CHARS(mName),&eArgsIn,&eArgsOut);

        STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc);

       for (int i=0,s=outArgs.size(); i<s; i++)
           handler.deliverParamValue(outArgs[i]);
       handler.complete();
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response); 
}
*//*
struct indProvRecord {
   indProvRecord() : enabled(false), count(1), handler(NULL) {}
   Boolean enabled;
   int count;
   EnableIndicationsResponseHandler* handler;
};

struct indSelectRecord {
   indSelectRecord() : eSelx(NULL) {}
   CMPI_SelectExp *eSelx;
};


typedef HashTable<String,indProvRecord*,EqualFunc<String>,HashFunc<String> > IndProvTab;
typedef HashTable<String,indSelectRecord*,EqualFunc<String>,HashFunc<String> > IndSelectTab;

IndProvTab provTab;
IndSelectTab selxTab;
*//*
int LocateIndicationProviderNames(const CIMInstance& pInstance, const CIMInstance& pmInstance,
                                  String& providerName, String& location)
{
    Uint32 pos = pInstance.findProperty(CIMName ("Name"));
    pInstance.getProperty(pos).getValue().get(providerName);

    pos = pmInstance.findProperty(CIMName ("Location"));
    pmInstance.getProperty(pos).getValue().get(location);
    return 0;
}

String JMPIProviderManager::getFilter(CIMInstance &subscription)
{
   try {
   CIMValue filterValue = subscription.getProperty (subscription.findProperty
        ("Filter")).getValue ();
   CIMObjectPath filterReference;
   filterValue.get(filterReference);
   CIMNamespaceName ns("root/PG_InterOp");

   _repository->read_lock ();
   CIMInstance filter=_repository->getInstance(ns,filterReference);
   _repository->read_unlock ();

   CIMValue queryValue = filter.getProperty (filter.findProperty
        ("Query")).getValue ();
   String query;
   queryValue.get(query);
   return query;
   }
   catch (CIMException &e) {
      _repository->read_unlock ();
      std::cout<<"??? JMPIProviderManager::getFilter"<<e.getCode()<<" "<<e.getMessage()<<" ???"<<std::endl;
      abort();
  }
   catch (...) {
      _repository->read_unlock ();
      std::cout<<"??? What Happend ???"<<std::endl;
      abort();
   }
}

Message * JMPIProviderManager::handleCreateSubscriptionRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "DefaultProviderManager::handleCreateSubscriptionRequest");

    HandlerIntroInd(CreateSubscription,message,request,response,
                 handler);
    try {
        const CIMObjectPath &x=request->subscriptionInstance.getPath();

	String providerName,providerLocation;
	LocateIndicationProviderNames(request->provider, request->providerModule,
	   providerName,providerLocation);

        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleCreateSubscriptionRequest - Host name: $0  Name space: $1  Provider name(s): $2",
            System::getHostName(),
            request->nameSpace.getString(),
            providerName);

        String fileName = resolveFileName(providerLocation);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(fileName, providerName, String::EMPTY);

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

        context->insert(IdentityContainer(request->userName));
        context->insert(SubscriptionInstanceContainer
            (request->subscriptionInstance));
        context->insert(SubscriptionFilterConditionContainer
            (request->condition, request->queryLanguage));

        context->insert(SubscriptionLanguageListContainer
            (request->acceptLanguages));
        context->insert(AcceptLanguageListContainer(request->acceptLanguages));
        context->insert(ContentLanguageListContainer(request->contentLanguages));

        CIMObjectPath subscriptionName = request->subscriptionInstance.getPath();

  	JMPIProvider & pr=ph.GetProvider();

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(*context);
        CMPI_SelectExp *eSelx=new CMPI_SelectExp(*context,
	   getFilter(request->subscriptionInstance),
	   request->queryLanguage);
	srec->eSelx=eSelx;
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.createSubscriptionRequest: " + pr.getName());

        DDD(std::cerr<<"--- JMPIProviderManager::createSubscriptionRequest"<<std::endl);

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

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.indMI->ft->activateFilter(
           pr.miVector.indMI,&eCtx,NULL,eSelx,
           CHARS(request->nameSpace.getString().getCString()),&eRef,false);

       STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc);
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleDeleteSubscriptionRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleDeleteSubscriptionRequest");

    HandlerIntroInd(DeleteSubscription,message,request,response,
                 handler);
    try {
	String providerName,providerLocation;
	LocateIndicationProviderNames(request->provider, request->providerModule,
	   providerName,providerLocation);

        Logger::put(Logger::STANDARD_LOG, System::CIMSERVER, Logger::TRACE,
            "JMPIProviderManager::handleDeleteSubscriptionRequest - Host name: $0  Name space: $1  Provider name(s): $2",
            System::getHostName(),
            request->nameSpace.getString(),
            providerName);

        String fileName = resolveFileName(providerLocation);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(fileName, providerName, String::EMPTY);


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

        context.insert(IdentityContainer(request->userName));
        context.insert(SubscriptionInstanceContainer
            (request->subscriptionInstance));

        context.insert(SubscriptionLanguageListContainer
            (request->acceptLanguages));
        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

        CIMObjectPath subscriptionName = request->subscriptionInstance.getPath();

  	JMPIProvider & pr=ph.GetProvider();

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.deleteSubscriptionRequest: " + pr.getName());

        DDD(std::cerr<<"--- JMPIProviderManager::deleteSubscriptionRequest"<<std::endl);

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        rc=pr.miVector.indMI->ft->deActivateFilter(
           pr.miVector.indMI,&eCtx,NULL,eSelx,
           CHARS(request->nameSpace.getString().getCString()),&eRef,prec==NULL);

       delete eSelx;

       STAT_PMS_PROVIDEREND;

        if (rc.rc!=CMPI_RC_OK)
	   throw CIMException((CIMStatusCode)rc.rc);
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleEnableIndicationsRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager:: handleEnableIndicationsRequest");

    HandlerIntroInd(EnableIndications,message,request,response,
                 handler);
    try {
        String providerName,providerLocation;
	LocateIndicationProviderNames(request->provider, request->providerModule,
	   providerName,providerLocation);

        indProvRecord *provRec;
        if (provTab.lookup(providerName,provRec)) {
           provRec->enabled=true;
           provRec->handler=new EnableIndicationsResponseHandler(request, response,
              request->provider, ProviderManagerService::providerManagerService);
        }

        String fileName = resolveFileName(providerLocation);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(fileName, providerName, String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

  	JMPIProvider & pr=ph.GetProvider();

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.EnableIndicationRequest: " + pr.getName());

        DDD(std::cerr<<"--- JMPIProviderManager::enableIndicationRequest"<<std::endl);

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        pr.miVector.indMI->ft->enableIndications(
           pr.miVector.indMI);

       STAT_PMS_PROVIDEREND;
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleDisableIndicationsRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager:: handleDisableIndicationsRequest");

    HandlerIntroInd(DisableIndications,message,request,response,
                 handler);
    try {
        String providerName,providerLocation;
	LocateIndicationProviderNames(request->provider, request->providerModule,
	   providerName,providerLocation);

        indProvRecord *provRec;
        if (provTab.lookup(providerName,provRec)) {
           provRec->enabled=false;
           if (provRec->handler) delete provRec->handler;
	   provRec->handler=NULL;
        }

        String fileName = resolveFileName(providerLocation);

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(fileName, providerName, String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(AcceptLanguageListContainer(request->acceptLanguages));
        context.insert(ContentLanguageListContainer(request->contentLanguages));

  	JMPIProvider & pr=ph.GetProvider();

	CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(context);
        CMPI_ThreadContext thr(&pr.broker,&eCtx);

        PEG_TRACE_STRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.DisableIndicationRequest: " + pr.getName());

        DDD(std::cerr<<"--- JMPIProviderManager::disableIndicationRequest"<<std::endl);

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        STAT_GETSTARTTIME;

        pr.miVector.indMI->ft->disableIndications(
           pr.miVector.indMI);

       STAT_PMS_PROVIDEREND;
    }
    HandlerCatch(handler);
    
    PEG_METHOD_EXIT();

    return(response);
}*/
//
// Provider module status
//
static const Uint16 _MODULE_OK       = 2;
static const Uint16 _MODULE_STOPPING = 9;
static const Uint16 _MODULE_STOPPED  = 10;

Message * JMPIProviderManager::handleDisableModuleRequest(const Message * message) throw()
{
    // HACK
    ProviderRegistrationManager * _providerRegistrationManager = GetProviderRegistrationManager();

    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleDisableModuleRequest");

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

    //
    // get operational status
    //
    Array<Uint16> operationalStatus;

    if(!disableProviderOnly)
    {
        Uint32 pos2 = mInstance.findProperty(CIMName ("OperationalStatus"));

        if(pos2 != PEG_NOT_FOUND)
        {
            //
            //  ATTN-CAKG-P2-20020821: Check for null status?
            //
            mInstance.getProperty(pos2).getValue().get(operationalStatus);
        }

        //
        // update module status from OK to Stopping
        //
        for(Uint32 i=0, n = operationalStatus.size(); i < n; i++)
        {
            if(operationalStatus[i] == _MODULE_OK)
            {
                operationalStatus.remove(i);
            }
        }
        operationalStatus.append(_MODULE_STOPPING);

        if(_providerRegistrationManager->setProviderModuleStatus
            (moduleName, operationalStatus) == false)
        {
            //l10n
            //throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, "set module status failed.");
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
                "ProviderManager.JMPIProviderManager.SET_MODULE_STATUS_FAILED",
                "set module status failed."));
        }
    }

    // Unload providers
    Array<CIMInstance> _pInstances = request->providers;

    for(Uint32 i = 0, n = _pInstances.size(); i < n; i++)
    {
        /* temp disabled by Chip
        // get the provider file name and logical name
        Triad<String, String, String> triad =
            getProviderRegistrar()->_getProviderRegPair(_pInstances[i], mInstance);

        providerManager.unloadProvider(triad.first, triad.second);
        */
    }

    if(!disableProviderOnly)
    {
        // update module status from Stopping to Stopped
        for(Uint32 i=0, n = operationalStatus.size(); i < n; i++)
        {
            if(operationalStatus[i] == _MODULE_STOPPING)
            {
                operationalStatus.remove(i);
            }
        }

        operationalStatus.append(_MODULE_STOPPED);

        if(_providerRegistrationManager->setProviderModuleStatus
            (moduleName, operationalStatus) == false)
        {
            //l10n
            //throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            //"set module status failed.");
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
                "ProviderManager.JMPIProviderManager.SET_MODULE_STATUS_FAILED",
                "set module status failed."));
        }
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

Message * JMPIProviderManager::handleEnableModuleRequest(const Message * message) throw()
{
    // HACK
    ProviderRegistrationManager * _providerRegistrationManager = GetProviderRegistrationManager();

    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleEnableModuleRequest");

    CIMEnableModuleRequestMessage * request =
        dynamic_cast<CIMEnableModuleRequestMessage *>(const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    //
    // get module status
    //
    CIMInstance mInstance = request->providerModule;
    Array<Uint16> operationalStatus;
    Uint32 pos = mInstance.findProperty(CIMName ("OperationalStatus"));

    if(pos != PEG_NOT_FOUND)
    {
        //
        //  ATTN-CAKG-P2-20020821: Check for null status?
        //
        mInstance.getProperty(pos).getValue().get(operationalStatus);
    }

    // update module status from Stopped to OK
    for(Uint32 i=0, n = operationalStatus.size(); i < n; i++)
    {
        if(operationalStatus[i] == _MODULE_STOPPED)
        {
            operationalStatus.remove(i);
        }
    }

    operationalStatus.append(_MODULE_OK);

    //
    // get module name
    //
    String moduleName;

    Uint32 pos2 = mInstance.findProperty(CIMName ("Name"));

    if(pos2 != PEG_NOT_FOUND)
    {
        mInstance.getProperty(pos2).getValue().get(moduleName);
    }

    if(_providerRegistrationManager->setProviderModuleStatus
        (moduleName, operationalStatus) == false)
    {
        //l10n
        //throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, "set module status failed.");
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
            "ProviderManager.JMPIProviderManager.SET_MODULE_STATUS_FAILED",
            "set module status failed."));
    }

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

Message * JMPIProviderManager::handleStopAllProvidersRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleStopAllProvidersRequest");

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

Message * JMPIProviderManager::handleUnsupportedRequest(const Message * message) throw()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "JMPIProviderManager::handleUnsupportedRequest");

    PEG_METHOD_EXIT();

    // a null response implies unsupported or unknown operation
    return(0);
}

ProviderName JMPIProviderManager::_resolveProviderName(const ProviderName & providerName)
{
    ProviderName temp = findProvider(providerName);

    return(temp);
}

String JMPIProviderManager::resolveFileName(String fileName)
{
    String name;
    #if defined(PEGASUS_OS_TYPE_WINDOWS)
    name = fileName; // + String(".dll");
    #elif defined(PEGASUS_OS_HPUX) && defined(PEGASUS_PLATFORM_HPUX_PARISC_ACC)
    name = ConfigManager::getHomedPath(ConfigManager::getInstance()->getCurrentValue("providerDir"));
    name.append(String("/") + fileName); // + String(".sl"));
    #elif defined(PEGASUS_OS_HPUX) && !defined(PEGASUS_PLATFORM_HPUX_PARISC_ACC)
    name = ConfigManager::getHomedPath(ConfigManager::getInstance()->getCurrentValue("providerDir"));
    name.append(String("/") + fileName); // + String(".so"));
    #elif defined(PEGASUS_OS_OS400)
    name = filrName;
    #else
    name = ConfigManager::getHomedPath(ConfigManager::getInstance()->getCurrentValue("providerDir"));
    name.append(String("/") + fileName); // + String(".so"));
    #endif
    return name;
}

PEGASUS_NAMESPACE_END
