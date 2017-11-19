.. _embedding_vm_initialization:

==============================
Virtual Machine Initialization
==============================

The first thing that a host application has to do, is create a virtual machine.
The host application can create any number of virtual machines through the function
*sq_open()*.
Every single VM that was created using *sq_open()* has to be released with the function *sq_close()* when it is no
longer needed.

.. literalinclude:: vm_initialization_code_1.h
