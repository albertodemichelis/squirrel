.. _embedding_creating_a_c_function:

===================
Create a C function
===================

A native C function must have the following prototype: ::

    typedef SQInteger (*SQFUNCTION)(HSQUIRRELVM);

The parameters is an handle to the calling VM and the return value is an integer
respecting the following rules:

* 1 if the function returns a value
* 0 if the function does not return a value
* SQ_ERROR runtime error is thrown

In order to obtain a new callable squirrel function from a C function pointer, is necessary
to call sq_newclosure() passing the C function to it; the new Squirrel function will be
pushed in the stack.

When the function is called, the stackbase is the first parameter of the function and the
top is the last. In order to return a value the function has to push it in the stack and
return 1.

Function parameters are in the stack from position 1 ('this') to *n*.
*sq_gettop()* can be used to determinate the number of parameters.

If the function has free variables, those will be in the stack after the explicit parameters
an can be handled as normal parameters. Note also that the value returned by *sq_gettop()* will be
affected by free variables. *sq_gettop()* will return the number of parameters plus
number of free variables.

Here an example, the following function print the value of each argument and return the
number of arguments.

.. literalinclude:: creating_a_c_function_code_1.h

Here an example of how to register a function

.. literalinclude:: creating_a_c_function_code_2.h
