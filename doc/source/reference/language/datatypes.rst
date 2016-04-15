.. _datatypes_and_values:

=====================
Values and Data types
=====================

Squirrel is a dynamically typed language so variables do not have a type, although they
refer to a value that does have a type.
Squirrel basic types are integer, float, string, null, table, array, function, generator,
class, instance, bool, thread and userdata.

.. _userdata-index:

--------
Integer
--------

An Integer represents a 32 bits (or better) signed number.::

    local a = 123 //decimal
    local b = 0x0012 //hexadecimal
    local c = 075 //octal
    local d = 'w' //char code

--------
Float
--------

A float represents a 32 bits (or better) floating point number.::

    local a=1.0
    local b=0.234

--------
String
--------

Strings are an immutable sequence of characters to modify a string is necessary create a new one.

Squirrel's strings, behave like C or C++, are delimited by quotation marks(``"``) and can contain
escape sequences(``\t``, ``\a``, ``\b``, ``\n``, ``\r``, ``\v``, ``\f``, ``\\``, ``\"``, ``\'``, ``\0``,
``\x<hh>``, ``\u<hhhh>`` and ``\U<hhhhhhhh>``).

Verbatim string literals begin with ``@"`` and end with the matching quote.
Verbatim string literals also can extend over a line break. If they do, they
include any white space characters between the quotes: ::

    local a = "I'm a wonderful string\n"
    // has a newline at the end of the string
    local x = @"I'm a verbatim string\n"
    // the \n is copied in the string same as \\n in a regular string "I'm a verbatim string\n"

The only exception to the "no escape sequence" rule for verbatim
string literals is that you can put a double quotation mark inside a
verbatim string by doubling it: ::

    local multiline = @"
        this is a multiline string
        it will ""embed"" all the new line
        characters
    "

--------
Null
--------

The null value is a primitive value that represents the null, empty, or non-existent
reference. The type Null has exactly one value, called null.::

    local a = null

--------
Bool
--------

the bool data type can have only two. They are the literals ``true``
and ``false``. A bool value expresses the validity of a condition
(tells whether the condition is true or false).::

    local a = true;

--------
Table
--------

Tables are associative containers implemented as pairs of key/value (called a slot).::

    local t={}
    local test=
    {
        a=10
        b=function(a) { return a+1; }
    }

--------
Array
--------

Arrays are simple sequence of objects, their size is dynamic and their index starts always from 0.::

    local a  = ["I'm","an","array"]
    local b = [null]
    b[0] = a[2];

--------
Function
--------

Functions are similar to those in other C-like languages and to most programming
languages in general, however there are a few key differences (see below).

--------
Class
--------

Classes are associative containers implemented as pairs of key/value. Classes are created through
a 'class expression' or a 'class statement'. class members can be inherited from another class object
at creation time. After creation members can be added until a instance of the class is created.

--------------
Class Instance
--------------

Class instances are created by calling a *class object*. Instances, as tables, are
implemented as pair of key/value. Instances members cannot be dynamically added or removed; however the value of the members can be changed.



---------
Generator
---------

Generators are functions that can be suspended with the statement 'yield' and resumed
later (see :ref:`Generators <generators>`).

---------
Userdata
---------

Userdata objects are blobs of memory(or pointers) defined by the host application but
stored into Squirrel variables (See :ref:`Userdata and UserPointers <embedding_userdata_and_userpointers>`).


---------
Thread
---------

Threads are objects that represents a cooperative thread of execution, also known as coroutines.

--------------
Weak Reference
--------------

Weak References are objects that point to another (non-scalar) object but do not own a strong reference to it.
(See :ref:`Weak References <weak_references>`).


