.. _stdlib_stdbloblib:

==================
The Blob library
==================
The blob library implements binary data manipulations routines. The library is
based on `blob objects` that represent a buffer of arbitrary
binary data.

---------------
Squirrel API
---------------

+++++++++++++++
Global symbols
+++++++++++++++

.. js:function:: castf2i(f)

    casts a float to a int

.. js:function:: casti2f(n)

    casts a int to a float

.. js:function:: swap2(n)

    swap the byte order of a number (like it would be a 16bits integer)

.. js:function:: swap4(n)

    swap the byte order of an integer

.. js:function:: swapfloat(n)

    swaps the byteorder of a float

++++++++++++++++++
The blob class
++++++++++++++++++

The blob object is a buffer of arbitrary binary data. The object behaves like
a file stream, it has a read/write pointer and it automatically grows if data
is written out of his boundary.
A blob can also be accessed byte by byte through the `[]` operator.

    The blob class extends class stream.

.. js:class:: blob(size)

    :param int size: initial size of the blob

    returns a new instance of a blob class of the specified size in bytes

.. js:function:: blob.resize(size)

    :param int size: the new size of the blob in bytes

    resizes the blob to the specified `size`

.. js:function:: blob.swap2()

    swaps the byte order of the blob content as it would be an array of `16bits integers`

.. js:function:: blob.swap4()

    swaps the byte order of the blob content as it would be an array of `32bits integers`

------
C API
------

.. _sqstd_register_bloblib:

.. c:function:: SQRESULT sqstd_register_bloblib(HSQUIRRELVM v)

    :param HSQUIRRELVM v: the target VM
    :returns: an SQRESULT
    :remarks: The function aspects a table on top of the stack where to register the global library functions.

    initializes and registers the blob library in the given VM.

.. _sqstd_getblob:

.. c:function:: SQRESULT sqstd_getblob(HSQUIRRELVM v, SQInteger idx, SQUserPointer* ptr)

    :param HSQUIRRELVM v: the target VM
    :param SQInteger idx: and index in the stack
    :param SQUserPointer* ptr: A pointer to the userpointer that will point to the blob's payload
    :returns: an SQRESULT

    retrieve the pointer of a blob's payload from an arbitrary
    position in the stack.

.. _sqstd_getblobsize:

.. c:function:: SQInteger sqstd_getblobsize(HSQUIRRELVM v, SQInteger idx)

    :param HSQUIRRELVM v: the target VM
    :param SQInteger idx: and index in the stack
    :returns: the size of the blob at `idx` position

    retrieves the size of a blob's payload from an arbitrary
    position in the stack.

.. _sqstd_createblob:

.. c:function:: SQUserPointer sqstd_createblob(HSQUIRRELVM v, SQInteger size)

    :param HSQUIRRELVM v: the target VM
    :param SQInteger size:  the size of the blob payload that has to be created
    :returns: a pointer to the newly created blob payload

    creates a blob with the given payload size and pushes it in the stack.
