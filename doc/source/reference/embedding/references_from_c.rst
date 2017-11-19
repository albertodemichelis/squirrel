.. embedding_references_from_c:

========================================================
Mantaining references to Squirrel values from the C API
========================================================

Squirrel allows to reference values through the C API; the function sq_getstackobj() gets
a handle to a squirrel object(any type). The object handle can be used to control the lifetime
of an object by adding or removing references to it( see sq_addref() and sq_release()).
The object can be also re-pushed in the VM stack using sq_pushobject().

.. literalinclude:: references_from_c_code_1.h
