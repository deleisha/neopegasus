//%////-*-c++-*-////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001 The Open group, BMC Software, Tivoli Systems, IBM
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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_MessageQueue_h
#define Pegasus_MessageQueue_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/IPC.h>
#include <Pegasus/Common/Thread.h>

PEGASUS_NAMESPACE_BEGIN

/** The MessageQueue class represents a queue abstraction and is used by
    modules to exchange messages. Methods are provided for enqueuing,
    dequeuing, removing, iterating messages. Some methods are virtual and
    may be overriden but subclasses to modify the behavior.

    <h1>A Word about Queue Ids</h1>

    You may pass a specific queue id to the MessageQueue() constructor. The
    default is to call MessageQueue::getNextQueueId() to obtain one. Only pass
    queue ids generated by calling MessageQueue::getNextQueueId() to this
    constructor. Otherwise, you might end up with two queues with the same 
    queue id.

    A technique we encourage, is to declare global queue ids like this:

    <pre>
    extern const Uint32 GROCERY_QUEUE_ID;
    </pre>

    And then define them like this:

    <pre>
    const Uint32 GROCERY_QUEUE_ID = MessageQueue::getNextQueueId();
    </pre>

    And then pass them to the constructor of MessageQueue (from the derived
    class). In this way you will secure a unique constant identifier by which
    you may refer to a queue later on.

    <h1>A Word about using the find() Methods</h1>

    There are two find() methods. One that takes a queue id and one that
    takes a name. The time complexity of the former is O(1); whereas, the
    time complexity of the latter is O(n). Therefore, use the queue id form
    since it is more efficient.
*/
class PEGASUS_COMMON_LINKAGE MessageQueue
{
   public:

      /** This constructor places this object on a queue table which is
	  maintained by this class. Each message queue has a queue-id (which
	  may be obtained by calling getQueueId()). The queue-id may be passed
	  to lookupQueue() to obtain a pointer to the corresponding queue).

	  @param queueId the queue id to be used by this object. ONLY PASS IN
	  QUEUE IDS WHICH WERE GENERATED USING MessageQueue::getNextQueueId().
	  Otherwise, you might end up with more than one queue with the same
	  queue id.
      */
      MessageQueue(
	 const char *name = 0,
	 Boolean async = false,
	 Uint32 queueId = MessageQueue::getNextQueueId());
      
      /** Removes this queue from the queue table. */
      virtual ~MessageQueue();

      /** Enques a message (places it at the back of the queue).
	  @param message pointer to message to be enqueued.
	  @exception throws NullPointer exception if message parameter is null.
      */
      virtual void enqueue(Message* message) throw(IPCException);


      /** allows a caller to determine if this message queue is asynchronous or 
	  not .
      */
      virtual Boolean isAsync(void) { return _async; }
    

      /** Dequeues a message (removes it from the front of the queue).
	  @return pointer to message or zero if queue is empty.
      */
      virtual Message* dequeue() throw(IPCException);

      /** Removes the given message from the queue.
	  @param message to be removed.
	  @exception throws NullPointer if message parameter is null.
	  @exception throws NoSuchMessageOnQueue is message paramter is not
	  on this queue.
      */
      virtual void remove(Message* message) throw(IPCException);

      /** Find the message with the given type.
	  @parameter type type of message to be found.
	  @return pointer to message if found; null otherwise.
      */
      virtual Message* findByType(Uint32 type) throw(IPCException);

      /** Const version of findByType(). */
      virtual const Message* findByType(Uint32 type) const throw(IPCException);

      /** Find the message with the given key.
	  @parameter key key of message to be found.
	  @return pointer to message if found; null otherwise.
      */
      virtual Message* findByKey(Uint32 key) throw(IPCException);

      /** Const version of findByKey(). */
      virtual const Message* findByKey(Uint32 key) const throw(IPCException);

      /** Finds the messages with the given type and key.
	  @param type type of message to be found.
	  @param type key of message to be found.
	  @return pointer to message if found; null otherwise.
      */
      virtual Message* find(Uint32 type, Uint32 key) throw(IPCException);

      /** Const version of find(). */
      virtual const Message* find(Uint32 type, Uint32 key) const throw(IPCException);

      /** Returns pointer to front message. */
      Message* front() { return _front; }

      /** Const version of front(). */
      const Message* front() const { return _front; }

      /** Returns pointer to back message. */
      Message* back() { return _back; }

      /** Const version of back(). */
      const Message* back() const { return _back; }

      /** Returns true if there are no messages on the queue. */
      Boolean isEmpty() const { return _front == 0; }

      /** Returns the number of messages on the queue. */
      Uint32 getCount() const { return _count; }

      /** Retrieve the queue id for this queue. */
      Uint32 getQueueId() const { return _queueId; }

      Uint32 get_capabilities(void) const 
      {
	 return _capabilities;
      }
      

      /** Prints the contents of this queue by calling the print() method
	  of each message.
	  @param os stream onto which the output is placed.
      */
      void print(PEGASUS_STD(ostream)& os) const throw(IPCException);

      /** Lock this queue. */
      virtual void lock() throw(IPCException);

      /** Unlock this queue. */
      virtual void unlock();

      /** Provide a string name for this queue to be used by the print method.
       */
      virtual const char* getQueueName() const;

      /** This method is called after a message has been enqueued. This default
	  implementation does nothing. Derived classes may override this to
	  take some action each time a message is enqueued (for example, this
	  method could handle the incoming message in the thread of the caller
	  of enqueue()).
      */
      virtual void handleEnqueue();

      /** This method <b>may</b> be called prior to enqueueing an message.
	  the message queue can inform the caller that it does not want
	  to handle the message by returning false **/

      virtual Boolean messageOK(const Message *msg) { return true ;}
      
      /** Lookup a message queue from a queue id. Note this is an O(1) operation.
       */
      static MessageQueue* lookup(Uint32 queueId) throw(IPCException);

      /** Lookup a message given a queue name. NOte this is an O(N) operation.
       */
      static  MessageQueue* lookup(const char *name) throw(IPCException);

      /** Get the next available queue id. It always returns a non-zero
	  queue id an monotonically increases and finally wraps (to one)
	  after reaching the maximum unsigned 32 bit integer.
      */
      static Uint32 getNextQueueId() throw(IPCException);

   protected:
      Mutex _mut;
      Uint32 _queueId;
      char *_name;
      Uint32 _capabilities;
      
   private:


      Uint32 _count;
      Message* _front;
      Message* _back;
      Boolean _async;
    
};

inline const Message* MessageQueue::findByType(Uint32 type) const
   throw(IPCException)
{
   return ((MessageQueue*)this)->findByType(type);
}

inline const Message* MessageQueue::findByKey(Uint32 key) const
   throw(IPCException)
{
   return ((MessageQueue*)this)->findByKey(key);
}

inline const Message* MessageQueue::find(Uint32 type, Uint32 key) const
   throw(IPCException)
{
   return ((MessageQueue*)this)->find(type, key);
}

class NoSuchMessageOnQueue : public Exception
{
   public:
      NoSuchMessageOnQueue() : Exception("No such message on this queue") { }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_MessageQueue_h */
