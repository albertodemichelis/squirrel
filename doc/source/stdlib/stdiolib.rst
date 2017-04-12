.. _stdlib_stdiolib:

========================
The Input/Output library
========================

the i/o library implements basic input/output routines.

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

.. js:function:: dofile(stream, [raiseerror,buffer_size,encoding,guess])

	Same as above function but operates on stream

.. js:function:: loadfile(path, [raiseerror])

    compiles a squirrel script or loads a precompiled one an returns it as as function.
    if the optional parameter 'raiseerror' is true, the compiler error handler is invoked
    in case of a syntax error. If raiseerror is omitted or set to false, the compiler
    error handler is not invoked.
    When squirrel is compiled in Unicode mode the function can handle different character encodings,
    UTF8 with and without prefix and UCS-2 prefixed(both big endian an little endian).
    If the source stream is not prefixed UTF8 encoding is used as default.

.. js:function:: loadfile(stream, [raiseerror,buffer_size,encoding,guess])

	Same as above function but operates on stream

.. js:function:: writeclosuretofile(destpath, closure)

    serializes a closure to a bytecode file (destpath). The serialized file can be loaded
    using loadfile() and dofile().

.. js:function:: writeclosuretofile(stream, closure)

	Same as above function but operates on stream

.. js:data:: stderr

    File object bound on the os *standard error* stream

.. js:data:: stdin

    File object bound on the os *standard input* stream

.. js:data:: stdout

    File object bound on the os *standard output* stream


++++++++++++++++
The stream class
++++++++++++++++

    The stream class is an abstract class generalizing sources or sinks of data.
    
    The stream class is the base class of: blob, file, streamreader, textreader and textwriter.

.. js:function:: stream.close()

    closes the stream.

.. js:function:: stream.eos()

    returns a non null value if the read/write pointer is at the end of the stream.

.. js:function:: stream.flush()

    flushes the stream. Return a value != null if succeeded, otherwise returns null

.. js:function:: stream.len()

    returns the length of the stream. If stream is not seekable result is -1

.. js:function:: stream.print(text)

    :param string text: a string to be writen
	
    writes a string to the stream.
	
.. note:: How text is encoded depends on squirrel configuration. (See textwriter and textwriter)

.. js:function:: stream.readblob(size)

    :param int size: number of bytes to read

    read n bytes from the stream and returns them as blob

.. js:function:: stream.readline()

    read a line of text from the stream and returns it as string
	
.. note:: How text is encoded depends on squirrel configuration. (See textwriter and textwriter)

.. js:function:: stream.readn(type)

    :param int type: type of the number to read

    reads a number from the stream according to the type parameter.

    `type` can have the following values:

+--------------+--------------------------------------------------------------------------------+----------------------+
| parameter    | return description                                                             |  return type         |
+==============+================================================================================+======================+
| 'l'          | processor dependent, 32bits on 32bits processors, 64bits on 64bits processors  |  integer             |
+--------------+--------------------------------------------------------------------------------+----------------------+
| 'i'          | 32bits number                                                                  |  integer             |
+--------------+--------------------------------------------------------------------------------+----------------------+
| 's'          | 16bits signed integer                                                          |  integer             |
+--------------+--------------------------------------------------------------------------------+----------------------+
| 'w'          | 16bits unsigned integer                                                        |  integer             |
+--------------+--------------------------------------------------------------------------------+----------------------+
| 'c'          | 8bits signed integer                                                           |  integer             |
+--------------+--------------------------------------------------------------------------------+----------------------+
| 'b'          | 8bits unsigned integer                                                         |  integer             |
+--------------+--------------------------------------------------------------------------------+----------------------+
| 'f'          | 32bits float                                                                   |  float               |
+--------------+--------------------------------------------------------------------------------+----------------------+
| 'd'          | 64bits float                                                                   |  float               |
+--------------+--------------------------------------------------------------------------------+----------------------+

.. js:function:: stream.seek(offset [,origin])

    :param int offset: indicates the number of bytes from `origin`.
    :param int origin: origin of the seek

                        +--------------+-------------------------------------------+
                        |  'b'         |  beginning of the stream                  |
                        +--------------+-------------------------------------------+
                        |  'c'         |  current location                         |
                        +--------------+-------------------------------------------+
                        |  'e'         |  end of the stream                        |
                        +--------------+-------------------------------------------+

    Moves the read/write pointer to a specified location.

.. note:: If origin is omitted the parameter is defaulted as 'b'(beginning of the stream).

.. js:function:: stream.tell()

    returns the read/write pointer absolute position

.. js:function:: stream.writeblob(src)

    :param blob src: the source blob containing the data to be written

    writes a blob in the stream

.. js:function:: stream.writen(n, type)

    :param number n: the value to be written
    :param int type: type of the number to write

    writes a number in the stream formatted according to the `type` pamraeter

    `type` can have the following values:

+--------------+--------------------------------------------------------------------------------+
| parameter    | return description                                                             |
+==============+================================================================================+
| 'i'          | 32bits number                                                                  |
+--------------+--------------------------------------------------------------------------------+
| 's'          | 16bits signed integer                                                          |
+--------------+--------------------------------------------------------------------------------+
| 'w'          | 16bits unsigned integer                                                        |
+--------------+--------------------------------------------------------------------------------+
| 'c'          | 8bits signed integer                                                           |
+--------------+--------------------------------------------------------------------------------+
| 'b'          | 8bits unsigned integer                                                         |
+--------------+--------------------------------------------------------------------------------+
| 'f'          | 32bits float                                                                   |
+--------------+--------------------------------------------------------------------------------+
| 'd'          | 64bits float                                                                   |
+--------------+--------------------------------------------------------------------------------+


++++++++++++++
The file class
++++++++++++++

    The file class implements a stream on a operating system file.
    
    File class extends class stream.

.. js:class:: file(path, patten)

    It's constructor imitates the behaviour of the C runtime function fopen for eg. ::

        local myfile = file("test.xxx","wb+");

    creates a file with read/write access in the current directory.

++++++++++++++++++++++
The streamreader class
++++++++++++++++++++++

    The streamreader class implements an abstract read only stream.
    
    The streamreader class extends class stream.

.. js:class:: streamreader(source[,owns,buffer_size])

    :param stream source: stream to read from
    :param bool owns: if source stream will be closed when streamreader is closed. Default is false.
    :param int buffer_size: buffer size to be used while reading source stream. Default is 0 - no buffering.

    If successfully created textreader instance will reference the `source` stream.
	
.. js:function:: streamreader.mark(readAheadLimit)

    :param int readAheadLimit: Limit on the number of characters that may be read while still preserving the mark. After reading more than this many characters, attempting to reset the stream may fail.

    Marks the present position in the stream. Subsequent calls to reset() will attempt to reposition the stream to this point.

.. js:function:: streamreader.reset()

    If the stream has been marked, then attempt to reposition it at the mark. Return value is 0.
    If the stream has not been marked or readAheadLimit is reached, nothing is done. Return value is 1.

++++++++++++++++++++
The textreader class
++++++++++++++++++++

	The textreader class implements an abstract read only stream. It is used to read text with arbitrary encoding from a stream.

    The textreader class extends class stream.

.. js:class:: textreader(source[,owns,encoding,guess])

    :param stream source: stream to read from
    :param bool owns: if source stream will be closed when textreader is closed. Default is false.
    :param string encoding: encoding name. Default is "UTF-8".
    :param bool guess: try to guess encoding from BOM in source stream, in this case `encoding` is used as fallback. Default is false.

    If successfully created textreader instance will reference the `source` stream.
    
    Currently supported encodings are: ASCII; UTF-8; UTF-16, UTF-16BE, UCS-2BE; UCS-2, UCS-2LE, UTF-16LE. Encoding UCS-2 is supported only as alias for UTF-16.

++++++++++++++++++++
The textwriter class
++++++++++++++++++++

	The textwriter class implements an abstract write only stream. It is used to write text with arbitrary encoding to a stream.

    The textwriter class extends class stream.
    
.. js:class:: textwriter(destination[,owns,encoding])

    :param stream destination: stream to write to
    :param bool owns: if destination stream will be closed when textwriter is closed. Default is false.
    :param string encoding: encoding name. Default is "UTF-8".
    
    If successfully created textwriter instance will reference the `destination` stream.

    For encodings see textreader.

--------------
C API
--------------

.. _sqstd_register_iolib:

.. c:function:: SQRESULT sqstd_register_iolib(HSQUIRRELVM v)

    :param HSQUIRRELVM v: the target VM
    :returns: an SQRESULT
    :remarks: The function aspects a table on top of the stack where to register the global library functions.

    initialize and register the io library in the given VM.

++++++++++++++++
The stream class
++++++++++++++++

    The stream object is represented by opaque structure SQFILE.

.. c:function:: SQInteger sqstd_fread(void* buffer, SQInteger size, SQFILE file)

    :param void* buffer: buffer to read to
    :param SQInteger size: size in bytes to read from the stream
    :param SQFILE stream: the stream to read from
	:returns: the number of bytes read or -1 on error
    
    Reads `size` bytes from `stream` and stores them to `buffer`.
	
.. c:function:: SQInteger sqstd_fwrite(const SQUserPointer buffer, SQInteger size, SQFILE stream)

    :param const void* buffer: buffer with data to be writen
    :param SQInteger size: size in bytes to write from to stream
    :param SQFILE stream: the stream to write to
	:returns: the number of bytes writen or -1 on error

    Writes `size` bytes stored in `buffer` to `stream`.

.. c:function:: sqstd_fseek(SQFILE stream, SQInteger offset, SQInteger origin)

    :param SQFILE stream: the stream
    :param SQInteger offset: offset in file relative to `origin`
    :param SQInteger origin: origin of `offset`
    :returns: 0 on success or -1 on error.

    Sets position in the stream.
    `origin` can be one of:

        +--------------+-------------------------------------------+
        |  SQ_SEEK_SET |  beginning of the stream                  |
        +--------------+-------------------------------------------+
        |  SQ_SEEK_CUR |  current location                         |
        +--------------+-------------------------------------------+
        |  SQ_SEEK_END |  end of the stream                        |
        +--------------+-------------------------------------------+

.. c:function:: SQInteger sqstd_ftell(SQFILE stream)

    :param SQFILE stream: the stream
    :returns: the position in the stream or -1 on error.

.. c:function:: SQInteger sqstd_fflush(SQFILE stream)

    :param SQFILE stream: the stream
    :returns: 0 on success or -1 on error.

    Flushes the stream

.. c:function:: SQInteger sqstd_feof(SQFILE stream)

    :param SQFILE stream: the stream
    :returns: non-zero if end of stream is reached, zero if not.
    
    Checks if end of stream was reached.
    
.. c:function:: SQInteger sqstd_fclose(SQFILE stream)

    :param SQFILE stream: the stream
    :returns: 0 on success or -1 on error.
    
    Closes the stream. Returns zero on success or non-zeto on failure.

.. c:function:: void sqstd_frelease(SQFILE stream)

    :param SQFILE stream: the stream

    Releases (frees) the stream object. All stream objects must be released.

++++++++++++++
File Object
++++++++++++++

.. c:function:: SQFILE sqstd_fopen(const SQChar *filename ,const SQChar *mode)

    :param const SQChar *filename: file name
    :param const SQChar *mode: I/O mode
    :returns: a stream object representing file
    
    Opens file `filename` in mode `mode` and returns a stream object bounded to opened file.
    
    Stream must be released by call to sqstd_frelease.

.. c:function:: SQRESULT sqstd_createfile( HSQUIRRELVM v, SQUserPointer file, SQBool owns)

    :param HSQUIRRELVM v: the target VM
    :param SQUserPointer file: the stream that will be represented by the file object
    :param SQBool owns: if different true the stream will be automatically closed when the newly create file object is destroyed.
    :returns: an SQRESULT

    creates a stream object bound to the FILE passed as parameter `file` and pushes it in the stack

.. c:function:: SQRESULT sqstd_getfile(HSQUIRRELVM v, SQInteger idx, SQUserPointer* file)

    :param HSQUIRRELVM v: the target VM
    :param SQInteger idx: and index in the stack
    :param SQUserPointer* file: A pointer to a FILE handle that will store the result
    :returns: an SQRESULT

    retrieve the pointer of a FILE handle from an arbitrary
    position in the stack.


+++++++++++++++++++
Streamreader Object
+++++++++++++++++++

    The streamreader object is represented by opaque structure SQSRDR. SQSRDR can be freely casted to SQFILE.
    
.. c:function:: SQSRDR sqstd_streamreader( SQFILE source,SQBool owns,SQInteger buffer_size)

    :param SQFILE source: a stream to read from
    :param SQBool owns: tells if stream `source` will be closed when streamreader is closed
    :param SQInteger buffer_size: buffer size to be used while reading source stream.
    :returns: a streamreader object
    
    Creates a streamreader to read from stream `source`. If `buffer_size` is 0 no buffering is used.
    
    Streamreader must be released by call to sqstd_frelease.
    
.. note:: Releasing streamreader does NOT release the `source` stream.

.. c:function:: SQInteger sqstd_srdrmark(SQSRDR srdr,SQInteger readAheadLimit)

    :param SQSRDR srdr: streamreader object
    :param SQInteger readAheadLimit: read ahead limit

    Marks the present position in the stream. Subsequent calls to reset() will attempt to reposition the stream to this point.

.. c:function:: SQInteger sqstd_srdrreset(SQSRDR srdr)

    :param SQSRDR srdr: streamreader object
    :returns: zero on success, non-zero otherwise.

    Repositions streamreader to marked position.

+++++++++++++++++++
Textreader and Textwriter Objects
+++++++++++++++++++

.. c:function:: SQFILE sqstd_textreader(SQFILE source,SQBool owns,const SQChar *encoding,SQBool guess)

    :param SQFILE source: stream object to read from
    :param SQBool owns: tells if stream `source` will be closed when textreader is closed
    :param const SQChar *encoding: encoding name. If NULL default encoding "UTF-8" is used.
    :param SQBool guess: If non-zero - try to guess encoding by reading BOM from `source`.
    :returns: a stream representing textreader object
    
    Creates textreader to read from `source` stream.
    
    Textreader must be released by call to sqstd_frelease.
    
.. note:: Releasing textreader does NOT release the `source` stream.

.. c:function:: SQFILE sqstd_textwriter(SQFILE destination,SQBool owns,const SQChar *encoding)

    :param SQFILE destination: stream object to write to
    :param SQBool owns: tells if stream `destination` will be closed when textwriter is closed
    :param const SQChar *encoding: encoding name. If NULL default encoding "UTF-8" is used.
    :returns: a stream representing textwriter object

    Creates textwriter to write to `destination` stream.
    
    Textwriter must be released by call to sqstd_frelease.
    
.. note:: Releasing textwriter does NOT release the `destination` stream.

++++++++++++++++++++++++++++++++
Script loading and serialization
++++++++++++++++++++++++++++++++

.. c:function:: SQRESULT sqstd_loadfile(HSQUIRRELVM v, const SQChar* filename, SQBool printerror)

    :param HSQUIRRELVM v: the target VM
    :param SQChar* filename: path of the script that has to be loaded
    :param SQBool printerror: if true the compiler error handler will be called if a error occurs
    :returns: an SQRESULT

    Compiles a squirrel script or loads a precompiled one an pushes it as closure in the stack.
    When squirrel is compiled in Unicode mode the function can handle different character encodings,
    UTF8 with and without prefix and UCS-2 prefixed(both big endian an little endian).
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
    When squirrel is compiled in unicode mode the function can handle different character encodings,
    UTF8 with and without prefix and UCS-2 prefixed(both big endian an little endian).
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

