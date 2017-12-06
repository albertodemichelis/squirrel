.. _stdlib_stdiolib:

========================
The Input/Output library
========================

The i/o library implements basic input/output routines.

--------------
Squirrel API
--------------

++++++++++++++
Global Symbols
++++++++++++++


.. js:function:: dofile(path, [raiseerror])

    compiles a squirrel script or loads a precompiled one and executes it.
    returns the value returned by the script or null if no value is returned.
    if the optional parameter 'raiseerror' is true, the compiler error handler is invoked
    in case of a syntax error. If raiseerror is omitted or set to false, the compiler
    error handler is not invoked.
    When squirrel is compiled in Unicode mode the function can handle different character encodings,
    UTF8 with and without prefix and UCS-2 prefixed(both big endian an little endian).
    If the source stream is not prefixed UTF8 encoding is used as default.

.. js:function:: loadfile(path, [raiseerror])

    compiles a squirrel script or loads a precompiled one an returns it as as function.
    if the optional parameter 'raiseerror' is true, the compiler error handler is invoked
    in case of a syntax error. If raiseerror is omitted or set to false, the compiler
    error handler is not invoked.
    When squirrel is compiled in Unicode mode the function can handle different character encodings,
    UTF8 with and without prefix and UCS-2 prefixed(both big endian an little endian).
    If the source stream is not prefixed UTF8 encoding is used as default.

.. js:function:: writeclosuretofile(destpath, closure)

    serializes a closure to a bytecode file (destpath). The serialized file can be loaded
    using loadfile() and dofile().


.. js:data:: stderr

    File object bound on the os *standard error* stream

.. js:data:: stdin

    File object bound on the os *standard input* stream

.. js:data:: stdout

    File object bound on the os *standard output* stream


++++++++++++++
The file class
++++++++++++++

    The file class implements a stream on a operating system file.
    
    File class extends class stream and inherits all its methods.

.. js:class:: file(path, patten)

    It's constructor imitates the behaviour of the C runtime function fopen for eg. ::

        local myfile = file("test.xxx","wb+");

    creates a file with read/write access in the current directory.

--------------
C API
--------------

.. _sqstd_register_iolib:

.. c:function:: SQRESULT sqstd_register_iolib(HSQUIRRELVM v)

    :param HSQUIRRELVM v: the target VM
    :returns: an SQRESULT
    :remarks: The function aspects a table on top of the stack where to register the global library functions.

    initialize and register the io library in the given VM.

++++++++++++++
File Object
++++++++++++++

    The file object is represented by opaque structure SQFILE. SQFILE can be freely casted to SQSTREAM.

.. c:function:: SQFILE sqstd_fopen(const SQChar *filename ,const SQChar *mode)

    :param SQChar* filename: file name
    :param SQChar* mode: I/O mode
    :returns: a stream object representing file
    
    Opens file `filename` in mode `mode` and returns a file object bounded to opened file.
    
    File must be released by call to sqstd_fclose.

.. c:function:: SQInteger sqstd_fread(void* buffer, SQInteger size, SQInteger count, SQFILE file)

    :param void* buffer: buffer to read to
    :param SQInteger size: item size
    :param SQInteger count: items count
    :param SQFILE file: the file to read from
	:returns: return the number of items read. This number equals the number of bytes transferred only when `size` is 1.
    
    Reads `count` items of data, each `size` bytes long, from the file `file`, storing them at the location given by `buffer`.
	
.. c:function:: SQInteger sqstd_fwrite(const SQUserPointer buffer, SQInteger size, SQInteger count, SQFILE file)

    :param const void* buffer: buffer with data to be writen
    :param SQInteger size: item size
    :param SQInteger count: items count
    :param SQFILE stream: the stream to write to
	:returns: the number of items written. This number equals the number of bytes transferred only when `size` is 1.

    Writes `count` items of data, each `size` bytes long, to the file `file`, obtaining them from the location given by `buffer`.

.. c:function:: sqstd_fseek(SQFILE file, SQInteger offset, SQInteger origin)

    :param SQFILE file: the file
    :param SQInteger offset: offset in file relative to `origin`
    :param SQInteger origin: origin of `offset`
    :returns: 0 on success or non-zeto on failure.

    Sets position in the file.
    `origin` can be one of:

        +--------------+-------------------------------------------+
        |  SQ_SEEK_SET |  beginning of the stream                  |
        +--------------+-------------------------------------------+
        |  SQ_SEEK_CUR |  current location                         |
        +--------------+-------------------------------------------+
        |  SQ_SEEK_END |  end of the stream                        |
        +--------------+-------------------------------------------+

.. c:function:: SQInteger sqstd_ftell(SQFILE file)

    :param SQFILE file: the file
    :returns: the position in the stream or -1 on error.

.. c:function:: SQInteger sqstd_fflush(SQFILE file)

    :param SQFILE file: the file
    :returns: 0 on success or non-zeto on failure.

    Flushes the file.

.. c:function:: SQInteger sqstd_feof(SQFILE file)

    :param SQFILE file: the file
    :returns: non-zero if end of file is reached, zero if not.
    
    Checks if end of file was reached.
    
.. c:function:: SQInteger sqstd_fclose(SQFILE file)

    :param SQFILE file: the file
    :returns: 0 on success or non-zeto on failure.
    
    Closes and releases the file object. Returns zero on success or non-zeto on failure.
    All file object obtained by call to sqstd_fopen must be released by sqstd_fclose.


.. c:function:: SQRESULT sqstd_createfile( HSQUIRRELVM v, SQUserPointer file, SQBool owns)

    :param HSQUIRRELVM v: the target VM
    :param SQUserPointer file: the C FILE handle that will be represented by the file object
    :param SQBool owns: if true the stream will be automatically closed when the newly create file object is destroyed.
    :returns: an SQRESULT

    Creates a file object bound to the C FILE passed as parameter `file` and pushes it in the stack

.. c:function:: SQRESULT sqstd_getfile(HSQUIRRELVM v, SQInteger idx, SQUserPointer* file)

    :param HSQUIRRELVM v: the target VM
    :param SQInteger idx: and index in the stack
    :param SQUserPointer* file: A pointer to a C FILE handle that will store the result
    :returns: an SQRESULT

    Retrieve the pointer of a C FILE handle from an arbitrary position in the stack.

.. c:function:: SQRESULT sqstd_opensqfile(HSQUIRRELVM v, const SQChar *filename ,const SQChar *mode)

    :param HSQUIRRELVM v: the target VM
    :param SQChar* filename: file name
    :param SQChar* mode: I/O mode
    :returns: an SQRESULT
    
    Opens file `filename` in mode `mode`, creates a file object and pushes it in the stack

.. c:function:: SQRESULT sqstd_createsqfile(HSQUIRRELVM v, SQFILE file,SQBool own)

    :param HSQUIRRELVM v: the target VM
    :param SQFILE file: file
    :param SQBool own: if true the file will be automatically closed when the newly create file object is destroyed.
    :returns: an SQRESULT
    
    Creates a file object bound to the SQFILE `file` and pushes it in the stack

.. c:function:: SQRESULT sqstd_getsqfile(HSQUIRRELVM v, SQInteger idx, SQFILE *file)

    :param HSQUIRRELVM v: the target VM
    :param SQInteger idx: and index in the stack
    :param SQFILE* file: A pointer to a SQFILE that will store the result
    :returns: an SQRESULT

    Retrieve the pointer of a SQFILE handle from an arbitrary position in the stack.

++++++++++++++++++++++++++++++++
Script loading and serialization
++++++++++++++++++++++++++++++++

.. c:function:: SQRESULT sqstd_loadfile(HSQUIRRELVM v, const SQChar* filename, SQBool printerror)

    :param HSQUIRRELVM v: the target VM
    :param SQChar* filename: path of the script that has to be loaded
    :param SQBool printerror: if true the compiler error handler will be called if a error occurs
    :returns: an SQRESULT

    Compiles a squirrel script or loads a precompiled one an pushes it as closure in the stack.
    The function can handle different character encodings, UTF8 with and without prefix and UCS-2 prefixed(both big endian an little endian).
    If the source stream is not prefixed UTF8 encoding is used as default.

.. c:function:: SQRESULT sqstd_dofile(HSQUIRRELVM v, const SQChar* filename, SQBool retval, SQBool printerror)

    :param HSQUIRRELVM v: the target VM
    :param SQChar* filename: path of the script that has to be loaded
    :param SQBool retval: if true the function will push the return value of the executed script in the stack.
    :param SQBool printerror: if true the compiler error handler will be called if a error occurs
    :returns: an SQRESULT
    :remarks: the function expects a table on top of the stack that will be used as 'this' for the execution of the script. The 'this' parameter is left untouched in the stack.

    Compiles a squirrel script or loads a precompiled one and executes it.
    Optionally pushes the return value of the executed script in the stack.
    The function can handle different character encodings, UTF8 with and without prefix and UCS-2 prefixed(both big endian an little endian).
    If the source stream is not prefixed, UTF8 encoding is used as default. ::

        sq_pushroottable(v); //push the root table(were the globals of the script will are stored)
        sqstd_dofile(v, _SC("test.nut"), SQFalse, SQTrue);// also prints syntax errors if any

.. c:function:: SQRESULT sqstd_writeclosuretofile(HSQUIRRELVM v, const SQChar* filename)

    :param HSQUIRRELVM v: the target VM
    :param SQChar* filename: destination path of serialized closure
    :returns: an SQRESULT

    serializes the closure at the top position in the stack as bytecode in
    the file specified by the parameter filename. If a file with the
    same name already exists, it will be overwritten.

