.. _stdlib_stdimportlib:

==================
The Import library
==================

the import library implements one single function that allows you to use
the functions and methods of an external Squirrel package.

------------
Squirrel API
------------

++++++++++++++
Global Symbols
++++++++++++++

.. js:function:: import(modulename)

    retrieves a table with all the functions and classes exposed from the
    module with its given name.
    the module name passed in the argument must not have the extension passed
    with it (e.g. if the library for the module is called 'my_module.dll',
    just use 'import(my_module)').


------------
C API
------------

.. _sqstd_register_importlib:

.. c:function:: SQRESULT sqstd_register_importlib(HSQUIRRELVM v)

    :param HSQUIRRELVM v: the target VM
    :returns: an SQRESULT
    :remarks: The function aspects a table on top of the stack where to register the global library functions.

    initialize and register the import library in the give VM.
