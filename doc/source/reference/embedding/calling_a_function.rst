.. _embedding_calling_a_function:

==================
Calling a function
==================

To call a squirrel function it is necessary to push the function in the stack followed by the
parameters and then call the function sq_call.
The function will pop the parameters and push the return value if the last sq_call
parameter is > 0.

.. literalinclude:: calling_a_function_example_1.h

this is equivalent to the following Squirrel code::

    foo(1,2.0,"three");

If a runtime error occurs (or a exception is thrown) during the squirrel code execution
the sq_call will fail.
