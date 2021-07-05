.. compiler_directives:


=========================
Complier directives
=========================

.. index::
    single: Complier directives

Quirrel has a way to for flexibly customize language features.

This allows to smoothly change compiler, while keeping backward compatibility if needed.

Example
::

   #default:strict

   local function foo() {
     #relaxed-bool
     if (123)
       print("bar")
   }


Prefix directive with 'default:' to apply it to entire VM, otherwise directive applies to clojure in where it is declared.


=============================
List of Complier directives
=============================

----------------
Strict booleans
----------------

::  

    #strict-bool

Force booleans in conditional expressions, raise an error on non-boolean types.

Non-boolean in conditional expressions can lead to errors.
Because all programming languages with such feature behave differently, for example Python treats empty arrays and strings as false,
but JS doesn't; PHP convert logical expression to Boolean, etc.
Another reason is that due to dynamic nature of Quirrel those who read code can't know what exactly author meant with it.
::

   if (some_var) //somevar can be null, 0, 0.0 or object

or

::

   local foo = a() || b() && c //what author means? What will be foo?



----------------------------
Implicit bool expressions
----------------------------

::

    #relaxed-bool

Original Squirrel 3.1 behavior (treat boolean false, null, 0 and 0.0 as false, all other values as true)


------------------------------------
Disable root fallback
------------------------------------

::

    #no-root-fallback

Require :: to access root
::

   print(1) //error
   ::print(1) //prints 1

Implicit root fallback is error prone and a bit slower due to more lookups.

------------------------
Enable root fallback
------------------------

::

     #implicit-root-fallback

Allow to search for variable in root table

------------------------------------
Force explicit access to 'this'
------------------------------------

::

     #explicit-this

Require to explicitly specify 'this' or :: to access fields of function environment ('this') or root table.
This behavior is similar to one in Python.
Identifiers must be known local variables, free variables or constants.
For unknown names a compilation error is thrown.

------------------------------------
Allow implicit access to 'this'
------------------------------------

::

     #implicit-this

Original Squirrel 3.1 behavior.
For identifiers not known as local or free variables or constants/enums fallback code performing get/set operations
in 'this' or root table will be generated.


----------------------------------------
Disable function declaration sugar
----------------------------------------

::

    #no-func-decl-sugar

Don't allow to implicitly add functions to 'this' as fields.
This also error prone (and functions can be unintentionally added to roottable).
Root functions can be added explicitely via slot creation
::

    ::foo <- function(){}

Local functions can be added as
::

    local function foo(){}

----------------------------------------------
Allow function declaration sugar
----------------------------------------------

::

    #allow-func-decl-sugar

Allow to implicitly add functions to 'this'


-----------------------------------------------
Disable class declaration sugar
-----------------------------------------------

::

    #no-class-decl-sugar

The same directive as #no-func-decl-sugar but for classes.
Local classes can be added with:
::

    local class Foo{}

------------------------------------------------
Allow class declaration sugar
------------------------------------------------

::

   #allow-class-decl-sugar

Allow implicitly add classes to 'this'


----------------------------------------------
Disable implicit string concatenation
----------------------------------------------

::

  #no-plus-concat

Throws error on string concatenation with +.
It is slower for multple strings, use concat or join instead.
It is also not safe and can cause errors, as + is supposed to be at least Associative operation, and usually Commutative too.
But + for string concatenation is not associative, e.g.

Example:
::

   local a = 1
   local b = 2
   local c = "3"
   (a + b) + c != a + (b + c) // "33" != "123"

This actually happens especially on reduce of arrays and alike.

----------------------------------------------
Enable plus string concatenation
----------------------------------------------

::

   #allow-plus-concat

Allow using plus operator '+' to concatenate strings.

------------------
#strict
------------------

::

   #strict

Enable all extra checks/restrictions


------------------
#relaxed
------------------

::

   #relaxed

Disable all extra checks/restrictions

