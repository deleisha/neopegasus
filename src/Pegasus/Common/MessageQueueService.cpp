//%////-*-c++-*-////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software, Hewlett-Packard Company, IBM,
// The Open Group, Tivoli Systems
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
// Author: Mike Day (mdday@us.ibm.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include "MessageQueueService.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h> //l10n

PEGASUS_NAMESPACE_BEGIN

 
cimom *MessageQueueService::_meta_dispatcher = 0;
AtomicInt MessageQueueService::_service_count = 0;
AtomicInt MessageQueueService::_xid(1);
Mutex MessageQueueService::_meta_dispatcher_mutex;

static struct timeval create_time = {0, 1};
static struct timeval destroy_time = {300, 0}; 
static struct timeval deadlock_time = {0, 0};

ThreadPool *MessageQueueService::_thread_pool = 0;

DQueue<MessageQueueService> MessageQueueService::_polling_list(true);

Thread* MessageQueueService::_polling_thread = 0;

ThreadPool *MessageQueueService::get_thread_pool(void)
{
   return _thread_pool;
}

void unload_idle_providers(void);

PEGASUS_THREAD_RETURN PEGASUS_THREAD_CDECL  MessageQueueService::kill_idle_threads(void *parm)
{

   static struct timeval now, last = {0,0};
   gettimeofday(&now, NULL);
   int dead_threads = 0;
   
   if( now.tv_sec - last.tv_sec > 120 )
   {
      gettimeofday(&last, NULL);
      try 
      {
	 dead_threads =  MessageQueueService::_thread_pool->kill_dead_threads();
      }
      catch(...)
      {

      }
   }
   exit_thread((PEGASUS_THREAD_RETURN)dead_threads);
   return (PEGASUS_THREAD_RETURN)dead_threads;
}


void MessageQueueService::force_shutdown(Boolean destroy_flag)
{
   return;
   
#ifdef MESSAGEQUEUESERVICE_DEBUG
	//l10n
   MessageLoaderParms parms("Common.MessageQueueService.FORCING_SHUTDOWN",
			    "Forcing shutdown of CIMOM Message Router");
   PEGASUS_STD(cout) << MessageLoader::getMessage(parms) << PEGASUS_STD(endl);
#endif


   MessageQueueService *svc;
   int counter = 0;   
   _polling_list.lock();
   svc = _polling_list.next(0);
   
   while(svc != 0)
   {
#ifdef MESSAGEQUEUESERVICE_DEBUG
   		//l10n - reuse same MessageLoaderParms to avoid multiple creates
      	parms.msg_id = "Common.MessageQueueService.STOPPING_SERVICE";
   	  	parms.default_msg = "Stopping $0"; 
   	  	parms.arg0 = svc->getQueueName();
   	  	PEGASUS_STD(cout) << MessageLoader::getMessage(parms) << PEGASUS_STD(endl);
#endif
      							
      _polling_sem.signal();
      svc->_shutdown_incoming_queue();
      counter++;
      _polling_sem.signal();
      svc = _polling_list.next(svc);
   }
   _polling_list.unlock();

   _polling_sem.signal();

   MessageQueueService::_stop_polling = 1;
   
   if(destroy_flag == true) 
   {

      svc = _polling_list.remove_last();
      while(svc)
      {
	 delete svc;
	 svc = _polling_list.remove_last();
      }
      
   }
}


PEGASUS_THREAD_RETURN PEGASUS_THREAD_CDECL MessageQueueService::polling_routine(void *parm)
{
   Thread *myself = reinterpret_cast<Thread *>(parm);
   DQueue<MessageQueueService> *list = reinterpret_cast<DQueue<MessageQueueService> *>(myself->get_parm());
   while ( _stop_polling.value()  == 0 ) 
   {
      _polling_sem.wait();
      if(_stop_polling.value() != 0 )
      {
         break;
      }
      
      list->lock();
      MessageQueueService *service = list->next(0);
      while(service != NULL)
      {
	 if(service->_incoming.count() > 0 )
	 {
	    _thread_pool->allocate_and_awaken(service, _req_proc);
	 }
	 service = list->next(service);
      }
      list->unlock();
      if(_check_idle_flag.value() != 0 )
      {
	 _check_idle_flag = 0;
	 Thread th(kill_idle_threads, 0, true);
	 th.run();
      }
   }
   myself->exit_self( (PEGASUS_THREAD_RETURN) 1 );
   return(0);
}


Semaphore MessageQueueService::_polling_sem(0);
AtomicInt MessageQueueService::_stop_polling(0);
AtomicInt MessageQueueService::_check_idle_flag(0);


MessageQueueService::MessageQueueService(const char *name, 
					 Uint32 queueID, 
					 Uint32 capabilities, 
					 Uint32 mask) 
   : Base(name, true,  queueID),
     
     _mask(mask),
     _die(0),
     _incoming(true, 0),
     _callback(true),
     _incoming_queue_shutdown(0),
     _callback_ready(0),
     _req_thread(_req_proc, this, false),
     _callback_thread(_callback_proc, this, false)

{ 

   _capabilities = (capabilities | module_capabilities::async);
   
   _default_op_timeout.tv_sec = 30;
   _default_op_timeout.tv_usec = 100;

   _meta_dispatcher_mutex.lock(pegasus_thread_self());
   
   if( _meta_dispatcher == 0 )
   {
      PEGASUS_ASSERT( _service_count.value() == 0 );
      _meta_dispatcher = new cimom();
      if (_meta_dispatcher == NULL )
      {
	 _meta_dispatcher_mutex.unlock();
	 throw NullPointer();
      }
      _thread_pool = new ThreadPool(0, "MessageQueueService", 0, 0,
				    create_time, destroy_time, deadlock_time);  
      
      _polling_thread = new Thread(polling_routine, 
			           reinterpret_cast<void *>(&_polling_list), 
			           false);
      _polling_thread->run();
   }
   _service_count++;

   if( false == register_service(name, _capabilities, _mask) )
   {
      _meta_dispatcher_mutex.unlock();
      //l10n
      //throw BindFailedException("MessageQueueService Base Unable to register with  Meta Dispatcher");
      MessageLoaderParms parms("Common.MessageQueueService.UNABLE_TO_REGISTER",
			       "MessageQueueService Base Unable to register with  Meta Dispatcher");
      						   
      throw BindFailedException(parms);
   }
   
   _polling_list.insert_last(this);
   
   _meta_dispatcher_mutex.unlock();
//   _callback_thread.run();
   
//   _req_thread.run();
}


MessageQueueService::~MessageQueueService(void)
{
   _die = 1;

   if (_incoming_queue_shutdown.value() == 0 )
   {
      _shutdown_incoming_queue();
   }
   _callback_ready.signal();
   
   _meta_dispatcher_mutex.lock(pegasus_thread_self());
   _service_count--;
   if (_service_count.value() == 0 )
   {

      _stop_polling++;
      _polling_sem.signal();
      _polling_thread->join();
      delete _polling_thread;
      _polling_thread = 0;
      _meta_dispatcher->_shutdown_routed_queue();
      delete _meta_dispatcher;
      _meta_dispatcher = 0;

      delete _thread_pool;
      _thread_pool = 0;
   }
   _meta_dispatcher_mutex.unlock();
   _polling_list.remove(this);
   // Clean up in case there are extra stuff on the queue. 
  while (_incoming.count())
  {
	delete _incoming.remove_first();
  }
} 

void MessageQueueService::_shutdown_incoming_queue(void)
{
   
   
   if (_incoming_queue_shutdown.value() > 0 )
      return ;
   AsyncIoctl *msg = new AsyncIoctl(get_next_xid(),
				    0, 
				    _queueId, 
				    _queueId, 
				    true, 
				    AsyncIoctl::IO_CLOSE, 
				    0, 
				    0);

   msg->op = get_op();
   msg->op->_flags |= ASYNC_OPFLAGS_FIRE_AND_FORGET;
   msg->op->_flags &= ~(ASYNC_OPFLAGS_CALLBACK | ASYNC_OPFLAGS_SAFE_CALLBACK 
		   | ASYNC_OPFLAGS_SIMPLE_STATUS);
   msg->op->_state &= ~ASYNC_OPSTATE_COMPLETE;

   msg->op->_op_dest = this;
   msg->op->_request.insert_first(msg);
   
   _incoming.insert_last_wait(msg->op);

}



void MessageQueueService::enqueue(Message *msg) throw(IPCException)
{
   PEG_METHOD_ENTER(TRC_MESSAGEQUEUESERVICE, "MessageQueueService::enqueue()");

   Base::enqueue(msg);

   PEG_METHOD_EXIT();
}


PEGASUS_THREAD_RETURN PEGASUS_THREAD_CDECL MessageQueueService::_callback_proc(void *parm)
{
   Thread *myself = reinterpret_cast<Thread *>(parm);
   MessageQueueService *service = reinterpret_cast<MessageQueueService *>(myself->get_parm());
   AsyncOpNode *operation = 0;
   
   while ( service->_die.value() == 0 ) 
   {
      service->_callback_ready.wait();
      
      service->_callback.lock();
      operation = service->_callback.next(0);
      while( operation != NULL)
      {
	 if( ASYNC_OPSTATE_COMPLETE & operation->read_state())
	 {
	    operation = service->_callback.remove_no_lock(operation);
	    PEGASUS_ASSERT(operation != NULL);
	    operation->_thread_ptr = myself;
	    operation->_service_ptr = service;
	    service->_handle_async_callback(operation);
	    break;
	 }
	 operation = service->_callback.next(operation);
      }
      service->_callback.unlock();
   }
   myself->exit_self( (PEGASUS_THREAD_RETURN) 1 );
   return(0);
}


PEGASUS_THREAD_RETURN PEGASUS_THREAD_CDECL MessageQueueService::_req_proc(void * parm)
{
   MessageQueueService *service = reinterpret_cast<MessageQueueService *>(parm);
   // pull messages off the incoming queue and dispatch them. then 
   // check pending messages that are non-blocking
   AsyncOpNode *operation = 0;
   
   if ( service->_die.value() == 0 ) 
    {
	 try 
	 {
	    operation = service->_incoming.remove_first();
	 }
	 catch(ListClosed & )
	 {
	    operation = 0;
	    
	    return(0);
	 }
	 if( operation )
	 {
	    operation->_service_ptr = service;
	    service->_handle_incoming_operation(operation);
	 }
    }

   return(0);
}

Uint32 MessageQueueService::get_pending_callback_count(void)
{
   return _callback.count();
}



void MessageQueueService::_sendwait_callback(AsyncOpNode *op, 
					     MessageQueue *q, 
					     void *parm)
{
   op->_client_sem.signal();
}


// callback function is responsible for cleaning up all resources
// including op, op->_callback_node, and op->_callback_ptr
void MessageQueueService::_handle_async_callback(AsyncOpNode *op)
{
   if( op->_flags & ASYNC_OPFLAGS_SAFE_CALLBACK )
   {

      Message *msg = op->get_request();
      if( msg && ( msg->getMask() & message_mask::ha_async))
      {
	 if(msg->getType() == async_messages::ASYNC_LEGACY_OP_START )
	 {
	    AsyncLegacyOperationStart *wrapper = 
	       static_cast<AsyncLegacyOperationStart *>(msg);
	    msg = wrapper->get_action();
	    delete wrapper;
	 }
	 else if (msg->getType() == async_messages::ASYNC_MODULE_OP_START)
	 {
	    AsyncModuleOperationStart *wrapper = 
	       static_cast<AsyncModuleOperationStart *>(msg);
	    msg = wrapper->get_action();
	    delete wrapper;
	 }
	 else if (msg->getType() == async_messages::ASYNC_OP_START)
	 {
	    AsyncOperationStart *wrapper = 
	       static_cast<AsyncOperationStart *>(msg);
	    msg = wrapper->get_action();
	    delete wrapper;
	 }
	 delete msg;
      }

      msg = op->get_response();
      if( msg && ( msg->getMask() & message_mask::ha_async))
      {
	 if(msg->getType() == async_messages::ASYNC_LEGACY_OP_RESULT )
	 {
	    AsyncLegacyOperationResult *wrapper = 
	       static_cast<AsyncLegacyOperationResult *>(msg);
	    msg = wrapper->get_result();
	    delete wrapper;
	 }
	 else if (msg->getType() == async_messages::ASYNC_MODULE_OP_RESULT)
	 {
	    AsyncModuleOperationResult *wrapper = 
	       static_cast<AsyncModuleOperationResult *>(msg);
	    msg = wrapper->get_result();
	    delete wrapper;
	 }
      }
      void (*callback)(Message *, void *, void *) = op->__async_callback;
      void *handle = op->_callback_handle;
      void *parm = op->_callback_parameter;
      op->release();
      return_op(op);
      callback(msg, handle, parm);
   }
   else if( op->_flags & ASYNC_OPFLAGS_CALLBACK )
   {
      // note that _callback_node may be different from op 
      // op->_callback_response_q is a "this" pointer we can use for 
      // static callback methods
      op->_async_callback(op->_callback_node, op->_callback_response_q, op->_callback_ptr);
   }
}


void MessageQueueService::_handle_incoming_operation(AsyncOpNode *operation) 
//						     Thread *thread, 
//						     MessageQueue *queue)
{
   if ( operation != 0 )
   {
      
// ATTN: optimization 
// << Tue Feb 19 14:10:38 2002 mdd >>
      operation->lock();
            
      Message *rq = operation->_request.next(0);
     
// optimization <<< Thu Mar  7 21:04:05 2002 mdd >>>
// move this to the bottom of the loop when the majority of 
// messages become async messages. 

      // divert legacy messages to handleEnqueue
      if ((rq != 0) && (!(rq->getMask() & message_mask::ha_async)))
      {
	 rq = operation->_request.remove_first() ;
	 operation->unlock();
	 // delete the op node 
	 operation->release();
	 return_op( operation);

	 handleEnqueue(rq);
	 return;
      }

      if ( (operation->_flags & ASYNC_OPFLAGS_CALLBACK || 
	    operation->_flags & ASYNC_OPFLAGS_SAFE_CALLBACK) && 
	   (operation->_state & ASYNC_OPSTATE_COMPLETE))
      {
	 
	 operation->unlock();
	 _handle_async_callback(operation);
      }
      else 
      {
	 PEGASUS_ASSERT(rq != 0 );
	 operation->unlock();
	 _handle_async_request(static_cast<AsyncRequest *>(rq));
      }
   }
   return;
}

void MessageQueueService::_handle_async_request(AsyncRequest *req)
{
   if ( req != 0 )
   {
      req->op->processing();
   
      Uint32 type = req->getType();
      if( type == async_messages::HEARTBEAT )
	 handle_heartbeat_request(req);
      else if (type == async_messages::IOCTL)
	 handle_AsyncIoctl(static_cast<AsyncIoctl *>(req));
      else if (type == async_messages::CIMSERVICE_START)
	 handle_CimServiceStart(static_cast<CimServiceStart *>(req));
      else if (type == async_messages::CIMSERVICE_STOP)
	 handle_CimServiceStop(static_cast<CimServiceStop *>(req));
      else if (type == async_messages::CIMSERVICE_PAUSE)
	 handle_CimServicePause(static_cast<CimServicePause *>(req));
      else if (type == async_messages::CIMSERVICE_RESUME)
	 handle_CimServiceResume(static_cast<CimServiceResume *>(req));
      else if ( type == async_messages::ASYNC_OP_START)
	 handle_AsyncOperationStart(static_cast<AsyncOperationStart *>(req));
      else 
      {
	 // we don't handle this request message 
	 _make_response(req, async_results::CIM_NAK );
      }
   }
}


Boolean MessageQueueService::_enqueueResponse(
   Message* request, 
   Message* response)
   
{
   PEG_METHOD_ENTER(TRC_MESSAGEQUEUESERVICE,
                    "MessageQueueService::_enqueueResponse");

   if( request->getMask() & message_mask::ha_async)
   {
      if (response->getMask() & message_mask::ha_async )
      {
	 _completeAsyncResponse(static_cast<AsyncRequest *>(request), 
				static_cast<AsyncReply *>(response), 
				ASYNC_OPSTATE_COMPLETE, 0 );
         PEG_METHOD_EXIT();
	 return true;
      }
   }
   
   if(request->_async != 0 )
   {
      Uint32 mask = request->_async->getMask();
      PEGASUS_ASSERT(mask & (message_mask::ha_async | message_mask::ha_request ));
      
      AsyncRequest *async = static_cast<AsyncRequest *>(request->_async);
      AsyncOpNode *op = async->op;
      request->_async = 0;
      // the legacy request is going to be deleted by its handler
      // remove it from the op node 

      static_cast<AsyncLegacyOperationStart *>(async)->get_action();
      
            
      AsyncLegacyOperationResult *async_result = 
	 new AsyncLegacyOperationResult( 
	    async->getKey(),
	    async->getRouting(),
	    op,
	    response);
      _completeAsyncResponse(async,
			     async_result,
			     ASYNC_OPSTATE_COMPLETE, 
			     0);
      PEG_METHOD_EXIT();
      return true;
   }
   
   // ensure that the destination queue is in response->dest
   PEG_METHOD_EXIT();
   return SendForget(response);
   
}

void MessageQueueService::_make_response(Message *req, Uint32 code)
{
   cimom::_make_response(req, code);
}


void MessageQueueService::_completeAsyncResponse(AsyncRequest *request, 
						AsyncReply *reply, 
						Uint32 state, 
						Uint32 flag)
{
   PEG_METHOD_ENTER(TRC_MESSAGEQUEUESERVICE,
                    "MessageQueueService::_completeAsyncResponse");

   cimom::_completeAsyncResponse(request, reply, state, flag);

   PEG_METHOD_EXIT();
}


void MessageQueueService::_complete_op_node(AsyncOpNode *op,
					    Uint32 state, 
					    Uint32 flag, 
					    Uint32 code)
{
   cimom::_complete_op_node(op, state, flag, code);
}


Boolean MessageQueueService::accept_async(AsyncOpNode *op)
{
   if (_incoming_queue_shutdown.value() > 0 )
      return false;
   
// ATTN optimization remove the message checking altogether in the base 
// << Mon Feb 18 14:02:20 2002 mdd >>
   op->lock();
   Message *rq = op->_request.next(0);
   Message *rp = op->_response.next(0);
   op->unlock();
   
   if(  (rq != 0 && (true == messageOK(rq))) || (rp != 0 && ( true == messageOK(rp) )) &&  
	_die.value() == 0  )
   {
      _incoming.insert_last_wait(op);
      _polling_sem.signal();
      return true;
   }
   return false;
}

Boolean MessageQueueService::messageOK(const Message *msg)
{
   if (_incoming_queue_shutdown.value() > 0 )
      return false;
   return true;
}

void MessageQueueService::handle_heartbeat_request(AsyncRequest *req)
{
   // default action is to echo a heartbeat response 
   
   AsyncReply *reply = 
      new AsyncReply(async_messages::HEARTBEAT,
		     req->getKey(),
		     req->getRouting(),
		     0,
		     req->op, 
		     async_results::OK, 
		     req->resp,
		     false);
   _completeAsyncResponse(req, reply, ASYNC_OPSTATE_COMPLETE, 0 );
}


void MessageQueueService::handle_heartbeat_reply(AsyncReply *rep)
{ 
   ;
}
      
void MessageQueueService::handle_AsyncIoctl(AsyncIoctl *req)
{
   
   switch( req->ctl )
   {
      case AsyncIoctl::IO_CLOSE:
      {

	 MessageQueueService *service = static_cast<MessageQueueService *>(req->op->_service_ptr);

#ifdef MESSAGEQUEUESERVICE_DEBUG
	 PEGASUS_STD(cout) << service->getQueueName() << " Received AsyncIoctl::IO_CLOSE " << PEGASUS_STD(endl);
#endif
	 
	 // respond to this message. this is fire and forget, so we don't need to delete anything. 
	 // this takes care of two problems that were being found 
	 // << Thu Oct  9 10:52:48 2003 mdd >>
	  _make_response(req, async_results::OK);
	 // ensure we do not accept any further messages

	 // ensure we don't recurse on IO_CLOSE
	 if( _incoming_queue_shutdown.value() > 0 )
	    break;
	 
	 // set the closing flag 
	 service->_incoming_queue_shutdown = 1;
	 // empty out the queue
	 while( 1 )
	 {
	    AsyncOpNode *operation;
	    try 
	    {
	       operation = service->_incoming.remove_first();
	    }
	    catch(IPCException & )
	    {
	       break;
	    }
	    if( operation )
	    {
	       operation->_service_ptr = service;
	       service->_handle_incoming_operation(operation);
	    }
	    else
	       break;
	 } // message processing loop
	 
	 // shutdown the AsyncDQueue
	 service->_incoming.shutdown_queue();
	 return;
      }

      default:
	 _make_response(req, async_results::CIM_NAK);
   }
}

void MessageQueueService::handle_CimServiceStart(CimServiceStart *req)
{

#ifdef MESSAGEQUEUESERVICE_DEBUG
   PEGASUS_STD(cout) << getQueueName() << "received START" << PEGASUS_STD(endl);
#endif
   
   // clear the stoped bit and update
   _capabilities &= (~(module_capabilities::stopped));
   _make_response(req, async_results::OK);
   // now tell the meta dispatcher we are stopped 
   update_service(_capabilities, _mask);

}
void MessageQueueService::handle_CimServiceStop(CimServiceStop *req)
{
#ifdef MESSAGEQUEUESERVICE_DEBUG
   PEGASUS_STD(cout) << getQueueName() << "received STOP" << PEGASUS_STD(endl);
#endif
   // set the stopeed bit and update
   _capabilities |= module_capabilities::stopped;
   _make_response(req, async_results::CIM_STOPPED);
   // now tell the meta dispatcher we are stopped 
   update_service(_capabilities, _mask);
   
}
void MessageQueueService::handle_CimServicePause(CimServicePause *req)
{
   // set the paused bit and update
   _capabilities |= module_capabilities::paused;
   update_service(_capabilities, _mask);
   _make_response(req, async_results::CIM_PAUSED);
   // now tell the meta dispatcher we are stopped 
}
void MessageQueueService::handle_CimServiceResume(CimServiceResume *req)
{
   // clear the paused  bit and update
   _capabilities &= (~(module_capabilities::paused));
   update_service(_capabilities, _mask);
   _make_response(req, async_results::OK);
   // now tell the meta dispatcher we are stopped 
}
      
void MessageQueueService::handle_AsyncOperationStart(AsyncOperationStart *req)
{
   _make_response(req, async_results::CIM_NAK);
}

void MessageQueueService::handle_AsyncOperationResult(AsyncOperationResult *req)
{
   ;
}


void MessageQueueService::handle_AsyncLegacyOperationStart(AsyncLegacyOperationStart *req)
{
   // remove the legacy message from the request and enqueue it to its destination
   Uint32 result = async_results::CIM_NAK;
   
   Message *legacy = req->_act;
   if ( legacy != 0 )
   {
      MessageQueue* queue = MessageQueue::lookup(req->_legacy_destination);
      if( queue != 0 )
      {
	 if(queue->isAsync() == true )
	 {
	    (static_cast<MessageQueueService *>(queue))->handleEnqueue(legacy);
	 }
	 else 
	 {
	    // Enqueue the response:
	    queue->enqueue(req->get_action());
	 }
	 
	 result = async_results::OK;
      }
   }
   _make_response(req, result);
}

void MessageQueueService::handle_AsyncLegacyOperationResult(AsyncLegacyOperationResult *rep)
{
   ;
}

AsyncOpNode *MessageQueueService::get_op(void)
{
   AsyncOpNode *op = new AsyncOpNode();
   
   op->_state = ASYNC_OPSTATE_UNKNOWN;
   op->_flags = ASYNC_OPFLAGS_SINGLE | ASYNC_OPFLAGS_NORMAL;
   
   return op;
}

void MessageQueueService::return_op(AsyncOpNode *op)
{
   PEGASUS_ASSERT(op->read_state() & ASYNC_OPSTATE_RELEASED );
   delete op;
}


Boolean MessageQueueService::ForwardOp(AsyncOpNode *op, 
				       Uint32 destination)
{
   PEGASUS_ASSERT(op != 0 );
   op->lock();
   op->_op_dest = MessageQueue::lookup(destination);
   op->_flags |= (ASYNC_OPFLAGS_FIRE_AND_FORGET | ASYNC_OPFLAGS_FORWARD);
   op->_flags &= ~(ASYNC_OPFLAGS_CALLBACK);
   op->unlock();
   if ( op->_op_dest == 0 )
      return false;
      
   return  _meta_dispatcher->route_async(op);
}

 
Boolean MessageQueueService::SendAsync(AsyncOpNode *op, 
				       Uint32 destination,
				       void (*callback)(AsyncOpNode *, 
							MessageQueue *,
							void *),
				       MessageQueue *callback_response_q,
				       void *callback_ptr)
{ 
   PEGASUS_ASSERT(op != 0 && callback != 0 );
   
   // get the queue handle for the destination

   op->lock();
   op->_op_dest = MessageQueue::lookup(destination); // destination of this message
   op->_flags |= ASYNC_OPFLAGS_CALLBACK;
   op->_flags &= ~(ASYNC_OPFLAGS_FIRE_AND_FORGET);
   op->_state &= ~ASYNC_OPSTATE_COMPLETE;
   // initialize the callback data
   op->_async_callback = callback;   // callback function to be executed by recpt. of response
   op->_callback_node = op;          // the op node
   op->_callback_response_q = callback_response_q;  // the queue that will receive the response
   op->_callback_ptr = callback_ptr;   // user data for callback
   op->_callback_request_q = this;     // I am the originator of this request
   
   op->unlock();
   if(op->_op_dest == 0) 
      return false;
   
   return  _meta_dispatcher->route_async(op);
}


Boolean MessageQueueService::SendAsync(Message *msg, 
				       Uint32 destination,
				       void (*callback)(Message *response, 
							void *handle, 
							void *parameter),
				       void *handle, 
				       void *parameter)
{
   if(msg == NULL)
      return false;
   if(callback == NULL)
      return SendForget(msg);
   AsyncOpNode *op = get_op();
   msg->dest = destination;
   if( NULL == (op->_op_dest = MessageQueue::lookup(msg->dest)))
   {
      op->release();
      return_op(op);
      return false;
   }
   op->_flags |= ASYNC_OPFLAGS_SAFE_CALLBACK;
   op->_flags &= ~(ASYNC_OPFLAGS_FIRE_AND_FORGET);
   op->_state &= ~ASYNC_OPSTATE_COMPLETE;
   op->__async_callback = callback;
   op->_callback_node = op;
   op->_callback_handle = handle;
   op->_callback_parameter = parameter;
   op->_callback_response_q = this;
   

   if( ! (msg->getMask() & message_mask::ha_async) )
   {
      AsyncLegacyOperationStart *wrapper = 
	 new AsyncLegacyOperationStart(get_next_xid(),
				       op, 
				       destination, 
				       msg, 
				       destination);
   }
   else 
   {
      op->_request.insert_first(msg);
      (static_cast<AsyncMessage *>(msg))->op = op;
   }
   
   _callback.insert_last(op);
   return _meta_dispatcher->route_async(op);
}


Boolean MessageQueueService::SendForget(Message *msg)
{

   
   AsyncOpNode *op = 0;
   Uint32 mask = msg->getMask();
   
   if (mask & message_mask::ha_async)
   {
      op = (static_cast<AsyncMessage *>(msg))->op ;
   }

   if( op == 0 )
   {
      op = get_op();
      op->_request.insert_first(msg);
      if (mask & message_mask::ha_async)
	 (static_cast<AsyncMessage *>(msg))->op = op;
   }
   op->_op_dest = MessageQueue::lookup(msg->dest);
   op->_flags |= ASYNC_OPFLAGS_FIRE_AND_FORGET;
   op->_flags &= ~(ASYNC_OPFLAGS_CALLBACK | ASYNC_OPFLAGS_SAFE_CALLBACK 
		   | ASYNC_OPFLAGS_SIMPLE_STATUS);
   op->_state &= ~ASYNC_OPSTATE_COMPLETE;
   if ( op->_op_dest == 0 )
   {
      op->release();
      return_op(op);
      return false;
   }
   
   // now see if the meta dispatcher will take it
   return  _meta_dispatcher->route_async(op);
}


AsyncReply *MessageQueueService::SendWait(AsyncRequest *request)
{
   if ( request == 0 )
      return 0 ;

   Boolean destroy_op = false;
   
   if (request->op == 0)
   {
      request->op = get_op();
      request->op->_request.insert_first(request);
      destroy_op = true;
   }
   
   request->block = false;
   request->op->_flags |= ASYNC_OPFLAGS_PSEUDO_CALLBACK;
   SendAsync(request->op, 
	     request->dest,
 	     _sendwait_callback,
	     this,
	     (void *)0);
   
   request->op->_client_sem.wait();
   request->op->lock();
   AsyncReply * rpl = static_cast<AsyncReply *>(request->op->_response.remove_first());
   rpl->op = 0;
   request->op->unlock();
   
   if( destroy_op == true)
   {
      request->op->lock();
      request->op->_request.remove(request);
      request->op->_state |= ASYNC_OPSTATE_RELEASED;
      request->op->unlock();
      return_op(request->op);
      request->op = 0;
   }
   return rpl;
}


Boolean MessageQueueService::register_service(String name, 
					      Uint32 capabilities, 
					      Uint32 mask)

{
   RegisterCimService *msg = new RegisterCimService(get_next_xid(),
						    0, 
						    true, 
						    name, 
						    capabilities, 
						    mask,
						    _queueId);
   msg->dest = CIMOM_Q_ID;
   
   Boolean registered = false;
   AsyncReply *reply = static_cast<AsyncReply *>(SendWait( msg ));
   
   if ( reply != 0 )
   {
      if(reply->getMask() & message_mask:: ha_async)
      {
	 if(reply->getMask() & message_mask::ha_reply)
	 {
	    if(reply->result == async_results::OK || 
	       reply->result == async_results::MODULE_ALREADY_REGISTERED )
	       registered = true;
	 }
      }
      
      delete reply; 
   }
   delete msg;
   return registered;
}

Boolean MessageQueueService::update_service(Uint32 capabilities, Uint32 mask)
{
   
   
   UpdateCimService *msg = new UpdateCimService(get_next_xid(), 
						0, 
						true, 
						_queueId,
						_capabilities, 
						_mask);
   Boolean registered = false;

   AsyncMessage *reply = SendWait(msg);
   if (reply)
   {
      if(reply->getMask() & message_mask:: ha_async)
      {
	 if(reply->getMask() & message_mask::ha_reply)
	 {
	    if(static_cast<AsyncReply *>(reply)->result == async_results::OK)
	       registered = true;
	 }
      }
      delete reply;
   }
   delete msg;
   return registered;
}


Boolean MessageQueueService::deregister_service(void)
{

   _meta_dispatcher->deregister_module(_queueId);
   return true;
}


void MessageQueueService::find_services(String name, 
					Uint32 capabilities, 
					Uint32 mask, 
					Array<Uint32> *results)
{
   
   if( results == 0 )
      throw NullPointer();
    
   results->clear();
   
   FindServiceQueue *req = 
      new FindServiceQueue(get_next_xid(), 
			   0, 
			   _queueId, 
			   true, 
			   name, 
			   capabilities, 
			   mask);
   
   req->dest = CIMOM_Q_ID;
   
   AsyncMessage *reply = SendWait(req); 
   if(reply)
   {
      if( reply->getMask() & message_mask::ha_async)
      {
	 if(reply->getMask() & message_mask::ha_reply)
	 {
	    if(reply->getType() == async_messages::FIND_SERVICE_Q_RESULT)
	    {
	       if( (static_cast<FindServiceQueueResult *>(reply))->result == async_results::OK )
		  *results = (static_cast<FindServiceQueueResult *>(reply))->qids;
	    }
	 }
      }
      delete reply;
   }
   delete req;
   return ;
}

void MessageQueueService::enumerate_service(Uint32 queue, message_module *result)
{
   if(result == 0)
      throw NullPointer();
   
   EnumerateService *req 
      = new EnumerateService(get_next_xid(),
			     0, 
			     _queueId, 
			     true, 
			     queue);
   
   AsyncMessage *reply = SendWait(req);
   
   if (reply)
   {
      Boolean found = false;
      
      if( reply->getMask() & message_mask::ha_async)
      {
	 if(reply->getMask() & message_mask::ha_reply)
	 {
	    if(reply->getType() == async_messages::ENUMERATE_SERVICE_RESULT)
	    {
	       if( (static_cast<EnumerateServiceResponse *>(reply))->result == async_results::OK )
	       {
		  if( found == false)
		  {
		     found = true;
		     
		     result->put_name( (static_cast<EnumerateServiceResponse *>(reply))->name);
		     result->put_capabilities((static_cast<EnumerateServiceResponse *>(reply))->capabilities);
		     result->put_mask((static_cast<EnumerateServiceResponse *>(reply))->mask);
		     result->put_queue((static_cast<EnumerateServiceResponse *>(reply))->qid);
		  }
	       }
	    }
	 }
      }
      delete reply;
   }
   delete req;
   
   return;
}

Uint32 MessageQueueService::get_next_xid(void)
{
   static Mutex _monitor;
   Uint32 value;
   _monitor.lock(pegasus_thread_self());
   
   _xid++;
   value =  _xid.value();
   _monitor.unlock();
   return value;
   
}

PEGASUS_NAMESPACE_END
