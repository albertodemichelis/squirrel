.. _diff_from_original:

*******************************************
Differences from original Squirrel language
*******************************************

.. index::
    single: diff_from_original

------------
New features
------------

* Check for conflicting local variable, local functions, function arguments and constant names
* Null propagation, null-call and null-coalescing operators
* Destructuring assignment
* String interpolation
* Functions declared in expressions can be named now (local foo = function foo() {})
* Support local class declaration (local class C {})
* Support call()/acall() delegates for classes
* min(), max(), clamp() global functions added to base library
* string default delegates: .subst(), .replace(), .join(), .split(), .concat() methods added
* table default delegates: .map(), .each(), .findindex(), .findvalue(), .reduce(),
  __merge(), .__update() methods added
* array default delegates: .each(), .findvalue(), .filter_inplace()  methods added
* Support negative indexes in array.slice()
* Compiler directives for stricter and thus safer language
* Added C APIs not using stack pushes/pops
* Variable change tracking

----------------
Removed features
----------------

* Support for comma operator removed
* Class and member attribures removed
* Adding table/class methods with class::method syntaxic sugar disallowed
* # single line comments removed

----------------
Changes
----------------

* Constants and enums are now locally scoped by default
* 'global' keyword for declaring global constans end enum explicitly
* Arrays/strings .find() default delegate renamed to .indexof()
* getinfos() renamed to getfuncinfos() and now can be applied to callable objects
* array.append() can take multiple arguments
* changed arguments order for array.filter() to unify with other functions
