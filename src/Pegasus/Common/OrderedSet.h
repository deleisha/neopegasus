//%2006////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2006 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; Symantec Corporation; The Open Group.
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
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_OrderedSet_h
#define Pegasus_OrderedSet_h

#include <cstring>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CharSet.h>

PEGASUS_NAMESPACE_BEGIN

/* generateCIMNameTag generates a nameTag from a CIMName 
   A nameTag can be used as hashcode for hash tables.
   It is generated by entangling the bit values of a first
   and last letter of a CIMName. The entangling is done using
   a one access table in class CharSet to avoid algorithm overhead 
   as this part is performance critical. */

inline Uint32 generateCIMNameTag(const CIMName& name)
{
    if (name.isNull())
        return 0;
    const String& str = name.getString();
    const Uint16* p = reinterpret_cast<const Uint16*> (str.getChar16Data());
    Uint32 n = str.size();
    return
        (Uint32(CharSet::toUpperHash(p[0] & 0xFF)) << 1) | 
        Uint32(CharSet::toUpperHash(p[n-1] & 0xFF));
}

/** OrderedSet maintains an "ordered set" of named elements. It provides
    the following operations:

        1. Accessing the i-th element -- O(1)
        2. Appending a new element -- O(1)
        3. Removing the i-th element -- O(n)
        4. Inserting an element -- O(n)
        5. Finding a named element -- O(c), where c is a small constant.

    Requirements on template argument T:

        - Must have a sole member called "_rep" of type R.

    Requirements on template argument R:

        - Must be derived from Sharable.
        - Must have a public function CIMName getName() returning the name.
        - Must have a public function Uint32 getNameTag() returning the nameTag

    This container was designed to hold the following object types:

        - CIMProperty
        - CIMQualifier
        - CIMParameter
        - CIMMethod
 
    This container holds a hash table for a fast access via name
    and an array of nodes for ordered access.
    The hash table is implemented using a normal array with a fixed size.
    The array of nodes is implemented using the Buffer class to allow for
    dynamic growth of it without the need to implement just another linked list
   
   
*/
template<class T, class R, Uint32 N>
class OrderedSet
{
public:

    OrderedSet();

    ~OrderedSet();

    void clear();

    Uint32 size() const
    {
        return _size;
    }

    void reserveCapacity(Uint32 capacity)
    {
        _array.reserveCapacity(capacity * sizeof(Node));
    }

    Uint32 find(const CIMName& name, Uint32 nameTag) const;

    T& operator[](Uint32 i);

    const T& operator[](Uint32 i) const;

    void append(const T& x);

    void insert(Uint32 index, const T& x);

    void remove(Uint32 index);

private:

    OrderedSet(const OrderedSet& x);
    OrderedSet& operator=(const OrderedSet& x);

    struct Node
    {
        R* rep;
        Uint32 index;
        Node* next;
    };

    void _reorganize();

    /* Ordered list of nodes implemented as Buffer
       to inherit dynamic growth capability. Used
       for index based access
    */
    Buffer _array;

    /* Fast access hash table of nodes implemented
       as static array. Used for name based lookups
    */
    Node* (*_table)[N];

    Uint32 _size;
};

template<class T, class R, Uint32 N>
inline OrderedSet<T, R, N>::OrderedSet() : _array(64), _table(0), _size(0)
{
    /* Do not place code in this constructor
       to avoid construction overhead for empty OrderedSets
    */
}

template<class T, class R, Uint32 N>
inline OrderedSet<T, R, N>::~OrderedSet()
{
    // only do something in case we have members
    // avoids creation of an empty buffer getting build
    if (_size > 0)
    {
        Node* data = (Node*) _array.getData();
    
        for (Uint32 i = 0; i < _size; i++)
        {
            data[i].rep->decreaseOwnerCount();
            Dec(data[i].rep);
        }
    }
    free(_table);
}

template<class T, class R, Uint32 N>
inline void OrderedSet<T, R, N>::clear()
{
    /* Both, the dynamic, ordered list of elements(_array) and
       the hash table need to be cleaned up */
    if (_table)
        memset((*_table), 0, sizeof(Node*) * N);
    if (_size > 0)
    {
        Node* data = (Node*) _array.getData();
    
        for (Uint32 i = 0; i < _size; i++)
        {
            data[i].rep->decreaseOwnerCount();
            Dec(data[i].rep);
        }
        _size = 0;
        _array.clear();
    }
}

template<class T, class R, Uint32 N>
inline Uint32 OrderedSet<T, R, N>::find(const CIMName& name,
                                        Uint32 nameTag) const
{
    /* Search based on a name means we first do a direct access
       using the hash table and then lookup the corresponding
       index in the dynamic, ordered list */
    if (_size > 0)
    {
        Uint32 code = nameTag % N;
        
        for (const Node* p = (*_table)[code]; p != 0; p = p->next)
        {
            if (p->rep->getNameTag() == nameTag)
            {
                if (name.equal(p->rep->getName()))
                    return p->index;
            }
        }
    }
    return PEG_NOT_FOUND;
}

template<class T, class R, Uint32 N>
inline void OrderedSet<T, R, N>::append(const T& x)
{
    /* To append an element to the OrderedSet we have to
       update the hash table((*_table)) and append the element to
       the dynamic, ordered list(_array)
    */       
    struct Layout
    {
        R* rep;
    };
    const Layout* layout = reinterpret_cast<const Layout*> (&x);

    Uint32 code = layout->rep->getNameTag() % N;

    // First write access to the OrderedSet
    // initialise array and hash table
    if (_size  == 0)
    {
        if (!_table)
        {
            _table = (Node* (*)[N]) malloc(sizeof(Node*) * N);
        }
        if (!_table)
            throw PEGASUS_STD(bad_alloc)();
        memset((*_table), 0, sizeof(Node*) * N);
    }

    // Check if Buffer capacity changes
    // this would relocate a good number of pointers
    // which following have to be _reorganized
    Boolean reOrg = (_array.capacity() < sizeof(Node) + _array.size());
    if (reOrg)
        reserveCapacity((_size + 1) << 1);

    // First append to _array(dynamic, ordered list):
    {
        Node node;
        node.rep = layout->rep;
        node.index = _size;
        node.next = (*_table)[code];

        _array.append((const char*) &node, sizeof(node));
    }
        
    // Now add to hash table
    {
        Node* data = (Node*) _array.getData();
        (*_table)[code] = &data[_size];
    }

    layout->rep->increaseOwnerCount();
    Inc(layout->rep);
    _size++;
    
    // Reorganize hash table to reflect state of dynamic, ordered list
    // if necessary
    if (reOrg)
        _reorganize();
}

template<class T, class R, Uint32 N>
inline void OrderedSet<T, R, N>::remove(Uint32 index)
{
    if (index >= _size)
        throw IndexOutOfBoundsException();

    // Remove node from array (dynamic, ordered list)
    {
        Node* node = (Node*) _array.getData() + index;
        node->rep->decreaseOwnerCount();
        Dec(node->rep);
        _array.remove(index * sizeof(Node), sizeof(Node));
        _size--;
    }

    // Reorganize hash table to reflect state of dynamic, ordered list
    _reorganize();
}

template<class T, class R, Uint32 N>
inline void OrderedSet<T, R, N>::insert(Uint32 index, const T& x)
{
    if (index > _size)
        throw IndexOutOfBoundsException();

    // First write access to the OrderedSet
    // initialise array and hash table
    if (_size  == 0)
    {
        // reserveCapacity(N);
        if (!_table)
        {
            _table = (Node* (*)[N]) malloc(sizeof(Node*) * N);
        }
        if (!_table)
            throw PEGASUS_STD(bad_alloc)();
        memset((*_table), 0, sizeof(Node*) * N);
    }

    struct Layout
    {
        R* rep;
    };
    const Layout* layout = reinterpret_cast<const Layout*> (&x);

    // Insert into the ordered list.
    {
        Node node;
        node.rep = layout->rep;
        node.index = _size;
        _array.insert(index * sizeof(Node), (const char*) &node, sizeof(node));
        layout->rep->increaseOwnerCount();
        Inc(layout->rep);
        _size++;
    }

    // Reorganize hash table to reflect state of dynamic, ordered list
    _reorganize();
}

template<class T, class R, Uint32 N>
inline T& OrderedSet<T, R, N>::operator[](Uint32 index)
{
    if (index >= _size)
        throw IndexOutOfBoundsException();

    const Node* node = (const Node*) _array.getData() + index;
    return *const_cast<T*>(reinterpret_cast<const T*>(node));
}

template<class T, class R, Uint32 N>
inline const T& OrderedSet<T, R, N>::operator[](Uint32 index) const
{
    return ((OrderedSet*)this)->operator[](index);
}

template<class T, class R, Uint32 N>
void OrderedSet<T, R, N>::_reorganize()
{
    /* Resets index values of the hash table to reflect
       current state of the dynamic, ordered list
    */
    memset((*_table), 0, sizeof(Node*) * N);
    Node* data = (Node*) _array.getData();

    for (Uint32 i = 0; i < _size; i++)
    {
        Node* node = &data[i];

        node->index = i;
        Uint32 code = node->rep->getNameTag() % N;
        node->next = (*_table)[code];
        (*_table)[code] = node;
    }
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_OrderedSet_h */
