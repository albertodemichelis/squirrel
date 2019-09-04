.. _builtin_functions:


==================
Built-in Functions
==================

.. index::
    single: Built-in Functions
    pair: Global Symbols; Built-in Functions


^^^^^^^^^^^^^^
Global Symbols
^^^^^^^^^^^^^^

.. js:function:: array(size,[fill])

creates and returns array of a specified size. If the optional parameter fill is specified its value will be used to fill the new array's slots. If the fill parameter is omitted, null is used instead.

.. js:function:: seterrorhandler(func)


sets the runtime error handler

.. js:function:: callee()


returns the currently running closure

.. js:function:: setdebughook(hook_func)


sets the debug hook

.. js:function:: enabledebuginfo(enable)

enable/disable the debug line information generation at compile time. enable != null enables. enable == null disables.

.. js:function:: getroottable()

returns the root table of the VM.

.. js:function:: setroottable(table)

sets the root table of the VM. And returns the previous root table.

.. js:function:: getconsttable()

returns the const table of the VM.

.. js:function:: setconsttable(table)

sets the const table of the VM; returns the previous const table.

.. js:function:: assert(exp, [message])

throws an exception if exp is null or false. Throws "assertion failed" string by default, or message if specified.
If message argument is function it is evaluated and return value is used as message text. This is to avoid
unnecessary string formatting when it is not needed.

.. js:function:: print(x)

prints x to the standard output

.. js:function:: error(x)

prints x in the standard error output

.. js:function:: compilestring(string,[buffername])

compiles a string containing a squirrel script into a function and returns it::

    local compiledscript=compilestring("::print(\"ciao\")");
    //run the script
    compiledscript();

.. js:function:: collectgarbage()

    Runs the garbage collector and returns the number of reference cycles found (and deleted). This function only works on garbage collector builds.

.. js:function:: resurrectunreachable()

Runs the garbage collector and returns an array containing all unreachable object found. If no unreachable object is found, null is returned instead. This function is meant to help debugging reference cycles. This function only works on garbage collector builds.

.. js:function:: type(obj)

return the 'raw' type of an object without invoking the metamethod '_typeof'.

.. js:function:: getstackinfos(level)

returns the stack informations of a given call stack level. returns a table formatted as follow: ::

    {
        func="DoStuff", //function name

        src="test.nut", //source file

        line=10,        //line number

        locals = {      //a table containing the local variables

            a=10,

            testy="I'm a string"
        }
    }

level = 0 is getstackinfos() itself! level = 1 is the current function, level = 2 is the caller of the current function, and so on. If the stack level doesn't exist the function returns null.

.. js:function:: newthread(threadfunc)

creates a new cooperative thread object(coroutine) and returns it

.. js:function:: min(x, y, [z], [w], ...)

returns minimal value of all arguments

.. js:function:: max(x, y, [z], [w], ...)

returns maximal value of all arguments

.. js:function:: clamp(x, min_val, max_val)

returns value limited by provided min-max range

creates a new cooperative thread object(coroutine) and returns it

.. js:data:: _versionnumber_

integer values describing the version of VM and compiler. e.g. for Squirrel 3.0.1 this value will be 301

.. js:data:: _version_

string values describing the version of VM and compiler.

.. js:data:: _charsize_

size in bytes of the internal VM representation for characters(1 for ASCII builds 2 for UNICODE builds).

.. js:data:: _intsize_

size in bytes of the internal VM representation for integers(4 for 32bits builds 8 for 64bits builds).

.. js:data:: _floatsize_

size in bytes of the internal VM representation for floats(4 for single precision builds 8 for double precision builds).

-----------------
Default delegates
-----------------

Except null and userdata every squirrel object has a default delegate containing a set of functions to manipulate and retrieve information from the object itself.

^^^^^^^^
Integer
^^^^^^^^

.. js:function:: integer.tofloat()

convert the number to float and returns it


.. js:function:: integer.tostring()

converts the number to string and returns it


.. js:function:: integer.tointeger()

dummy function; returns the value of the integer.


.. js:function:: integer.tochar()

returns a string containing a single character represented by the integer.


.. js:function:: integer.weakref()

dummy function; returns the integer itself.

^^^^^
Float
^^^^^

.. js:function:: float.tofloat()

returns the value of the float(dummy function)


.. js:function:: float.tointeger()

converts the number to integer and returns it


.. js:function:: float.tostring()

converts the number to string and returns it


.. js:function:: float.tochar()

returns a string containing a single character represented by the integer part of the float.


.. js:function:: float.weakref()

dummy function; returns the float itself.

^^^^
Bool
^^^^

.. js:function:: bool.tofloat()

returns 1.0 for true 0.0 for false


.. js:function:: bool.tointeger()

returns 1 for true 0 for false


.. js:function:: bool.tostring()

returns "true" for true and "false" for false


.. js:function:: bool.weakref()

dummy function; returns the bool itself.

^^^^^^
String
^^^^^^

.. js:function:: string.len()

returns the string length


.. js:function:: string.tointeger([base])

Converts the string to integer and returns it. An optional parameter base can be specified--if a base is not specified, it defaults to base 10.


.. js:function:: string.tofloat()

converts the string to float and returns it


.. js:function:: string.tostring()

returns the string (really, a dummy function)


.. js:function:: string.slice(start,[end])

returns a section of the string as new string. Copies from start to the end (not included). If start is negative the index is calculated as length + start, if end is negative the index is calculated as length + end. If end is omitted end is equal to the string length.


.. js:function:: string.find(substr,[startidx])

Searches a sub string (substr) starting from the index startidx and returns the index of its first occurrence. If startidx is omitted the search operation starts from the beginning of the string. The function returns null if substr is not found.


.. js:function:: string.tolower()

returns a lowercase copy of the string.


.. js:function:: string.toupper()

returns a uppercase copy of the string.


.. js:function:: string.weakref()

returns a weak reference to the object.

.. js:function:: string.subst(...)

This delegate is used to format strings. A format string can contain variable positional arguments and table keys.
As parameters, you can pass an arbitrary number of tables and arbitrary number of positional arguments. If the key is found in several tables,
then the most value from the leftmost table will be used.

Example: ::

"Score: {0}".subst(4200) => "Score: 4200"
"x={0} y={1} z={2}".subst(42, 45.53, -10.8) => "x=42 y=45.53 z=-10.8"
"Score: {score}".subst({score=4200}) => "Score: 4200"
"x={x} y={y} z={z}".subst({y=45.53, x=42, z=-10.8}) => "x=42 y=45.53 z=-10.8"
"Type: {type}, Health: {hp}".subst({hp=100, damage=5}, {isAir=true, type="helicopter"}) => "Type: helicopter, Health: 100"
"Type: {type}, Pos: x={0} y={1} z={2}".subst({isAir=true, type="helicopter"}, 42, 45.53, -10.8) => "Type: helicopter, Pos: x=42 y=45.53 z=-10.8"
"Score: {0}".subst() => "Score: {0}"
"Score: {score}".subst({}) => "Score: {score}"

.. js:function:: string.replace(from, to)

Replaces all occurrences of 'from' substring to 'to'

.. js:function:: string.join(arr)

Concatenate all items in provided array using string itself as separator.
Example: ::
", ".join(["a", "b", "c"]) // => "a, b, c"

.. js:function:: string.concat(...)

Concatenate all arguments using string itself as separator.
Example: ::
", ".concat("a", "b", "c") // => "a, b, c"

.. js:function:: string.split([sep])

Return a list of the words in the string, using sep as the delimiter string.
If sep is given, consecutive delimiters are not grouped together and are deemed to delimit empty strings
(for example, '1,,2'.split(',') returns ['1', '', '2']).
The sep argument may consist of multiple characters (for example, '1<>2<>3'.split('<>') returns ['1', '2', '3']).
Splitting an empty string with a specified separator returns [''].

If sep is not specified or is None, a different splitting algorithm is applied:
runs of consecutive whitespace are regarded as a single separator, and the result will contain no empty strings
at the start or end if the string has leading or trailing whitespace.
Consequently, splitting an empty string or a string consisting of just whitespace without providing a separator returns [].


^^^^^
Table
^^^^^

.. js:function:: table.len()

returns the number of slots contained in a table


.. js:function:: table.rawget(key)

tries to get a value from the slot 'key' without employing delegation


.. js:function:: table.rawset(key,val)

Sets the slot 'key' with the value 'val' without employing delegation. If the slot does not exists, it will be created. Returns table itself.


.. js:function:: table.rawdelete()

Deletes the slot key without employing delegation and returns its value. If the slot does not exists, returns null.


.. js:function:: table.rawin(key)

returns true if the slot 'key' exists. the function has the same effect as the operator 'in' but does not employ delegation.


.. js:function:: table.weakref()

returns a weak reference to the object.


.. js:function:: table.tostring()

Tries to invoke the _tostring metamethod. If that fails, it returns "(table : pointer)".


.. js:function:: table.clear()

removes all the slots from the table. Returns table itself.


.. js:function:: table.setdelegate(table)

Sets the delegate of the table. To remove a delegate, 'null' must be passed to the function. The function returns the table itself (e.g. a.setdelegate(b) -- in this case 'a' is the return value).


.. js:function:: table.getdelegate()

returns the table's delegate or null if no delegate was set.


.. js:function:: table.filter(func(val, [key], [table_ref]))

Creates a new table with all values that pass the test implemented by the provided function. In detail, it creates a new table, invokes the specified function for each key-value pair in the original table; if the function returns 'true', then the value is added to the newly created table at the same key.

.. js:function:: table.keys()

returns an array containing all the keys of the table slots.

.. js:function:: table.values()

returns an array containing all the values of the table slots.


.. js:function:: table.map(func(slot_value, [slot_key], [table_ref]))

Creates a new table of the same size. For each element in the original table invokes the function 'func' and assigns the return value of the function to the corresponding slot of the newly created table.
Provided func can accept up to 3 arguments: slot value (required), slot key in table (optional), reference to table itself (optional).

.. js:function:: table.each(func(slot_value, [slot_key], [table_ref]))

Iterates a table and calls provided function for each element.

.. js:function:: table.search(func(slot_value, [slot_key], [table_ref]))

Performs a linear search calling provided function for each value in the table.
Returns the index of the value if it was found (callback returned true (non-false) value) or null otherwise.

.. js:function:: table.reduce(func(accumulator, slot_value, [slot_key], [table_ref]), [initializer])

Reduces a table to a single value (similar to array.reduce()).
For each table slot invokes the function 'func' passing the initial value
(or value from the previous callback call) and the value of the current element.
Callback function can also take optional parameters: key in table for current value and reference to table itself.
Iteration order is not determined.

.. js:function:: table.__merge(table_1, [table_2], [table_3], ...)

This delegate is used to create new table from old and given.
Arguments to merge fields from can be tables, classes and instances.

Example: ::

    local foo = {fizz=1}
    local bar = foo.__merge({buzz=2})
    => foo == {fizz=1}; bar={fizz=1, buzz=2}


.. js:function:: table.__update(table_1, [table_2], [table_3], ...)

This delegate is used to update new table with values from given ones.
In other words it mutates table with data from provided tables.

Example: ::

    local foo = {fizz=1}
    local bar = foo.__update({buzz=2})
    => foo == {fizz=1, bazz=2}; bar={fizz=1, buzz=2}


^^^^^^
Array
^^^^^^

.. js:function:: array.len()

returns the length of the array


.. js:function:: array.append(val)

appends the value 'val' at the end of the array. Returns array itself.


.. js:function:: array.push(val)

appends the value 'val' at the end of the array. Returns array itself.


.. js:function:: array.extend(array)

Extends the array by appending all the items in the given array. Returns array itself.


.. js:function:: array.pop()

removes a value from the back of the array and returns it.


.. js:function:: array.top()

returns the value of the array with the higher index


.. js:function:: array.insert(idx,val)

inserts the value 'val' at the position 'idx' in the array. Returns array itself.


.. js:function:: array.remove(idx)

removes the value at the position 'idx' in the array and returns its value.


.. js:function:: array.resize(size,[fill])

Resizes the array. If the optional parameter 'fill' is specified, its value will be used to fill the new array's slots when the size specified is bigger than the previous size. If the fill parameter is omitted, null is used instead. Returns array itself.


.. js:function:: array.sort([compare_func])

Sorts the array in-place. A custom compare function can be optionally passed. The function prototype as to be the following.::

    function custom_compare(a,b)
    {
        if(a>b) return 1
        else if(a<b) return -1
        return 0;
    }

a more compact version of a custom compare can be written using a lambda expression and the operator <=> ::

    arr.sort(@(a,b) a <=> b);

Returns array itself.

.. js:function:: array.reverse()

reverse the elements of the array in place. Returns array itself.


.. js:function:: array.slice(start,[end])

Returns a section of the array as new array. Copies from start to the end (not included). If start is negative the index is calculated as length + start, if end is negative the index is calculated as length + end. If end is omitted end is equal to the array length.


.. js:function:: array.weakref()

returns a weak reference to the object.


.. js:function:: array.tostring()

returns the string "(array : pointer)".


.. js:function:: array.clear()

removes all the items from the array


.. js:function:: array.map(func(item_value, [item_index], [array_ref]))

Creates a new array of the same size. For each element in the original array invokes the function 'func' and assigns the return value of the function to the corresponding element of the newly created array.
Provided func can accept up to 3 arguments: array item value (required), array item index (optional), reference to array itself (optional).


.. js:function:: array.apply(func([item_value, [item_index], [array_ref]))

for each element in the array invokes the function 'func' and replace the original value of the element with the return value of the function.

.. js:function:: array.each(func(item_value, [item_index], [array_ref]))

Iterates an array and calls provided function for each element.

.. js:function:: array.reduce(func(prevval,curval,[index],[array_ref]), [initializer])

Reduces an array to a single value. For each element in the array invokes the function 'func' passing
the initial value (or value from the previous callback call) and the value of the current element.
Callback can optionally accept index of current value and reference to array itself.
The return value of the function is then used as 'prevval' for the next element.
If the optional initializer is present, it is placed before the items of the array in the calculation,
and serves as a default when the sequence is empty.
If initializer is not given then for sequence contains only one item, reduce() returns the first item,
and for empty sequence returns null.

Given an sequence with 2 or more elements (including initializer) calls the function with the first two elements as the parameters,
gets that result, then calls the function with that result and the third element, gets that result,
calls the function with that result and the fourth parameter and so on until all element have been processed.
Finally, returns the return value of the last invocation of func.


.. js:function:: array.filter(func(val, [index], [array_ref]))

Creates a new array with all elements that pass the test implemented by the provided function. In detail, it creates a new array, for each element in the original array invokes the specified function passing the index of the element and it's value; if the function returns 'true', then the value of the corresponding element is added on the newly created array.

.. js:function:: array.filter_inplace(func(val, [index], [array_ref]))

Similar to array.filter(), but modifies given array instead of creating new one.
It removes all elements for which provided function returns false.

.. js:function:: array.find(value)

Performs a linear search for the value in the array. Returns the index of the value if it was found null otherwise.

.. js:function:: array.search(func(item_value, [item_index], [array_ref]))

Performs a linear search calling provided function for each value in the array.
Returns the index of the value if it was found (callback returned true (non-false) value) or null otherwise.


^^^^^^^^
Function
^^^^^^^^

.. js:function:: function.call(_this,args...)

calls the function with the specified environment object('this') and parameters


.. js:function:: function.pcall(_this,args...)

calls the function with the specified environment object('this') and parameters, this function will not invoke the error callback in case of failure(pcall stays for 'protected call')


.. js:function:: function.acall(array_args)

calls the function with the specified environment object('this') and parameters. The function accepts an array containing the parameters that will be passed to the called function.Where array_args has to contain the required 'this' object at the [0] position.


.. js:function:: function.pacall(array_args)

calls the function with the specified environment object('this') and parameters. The function accepts an array containing the parameters that will be passed to the called function.Where array_args has to contain the required 'this' object at the [0] position. This function will not invoke the error callback in case of failure(pacall stays for 'protected array call')


.. js:function:: function.weakref()

returns a weak reference to the object.


.. js:function:: function.tostring()

returns the string "(closure : pointer)".


.. js:function:: function.setroot(table)

sets the root table of a closure


.. js:function:: function.getroot()

returns the root table of the closure


.. js:function:: function.bindenv(env)

clones the function(aka closure) and bind the environment object to it(table,class or instance). the this parameter of the newly create function will always be set to env. Note that the created function holds a weak reference to its environment object so cannot be used to control its lifetime.


.. js:function:: function.getfuncinfos()

returns a table containing informations about the function, like parameters, name and source name; ::

    //the data is returned as a table is in form
    //pure squirrel function
    {
      native = false
      name = "zefuncname"
      src = "/somthing/something.nut"
      parameters = ["a","b","c"]
      defparams = [1,"def"]
      varargs = 2
    }
    //native C function
    {
      native = true
      name = "zefuncname"
      paramscheck = 2
      typecheck = [83886082,83886384] //this is the typemask (see C defines OT_INTEGER,OT_FLOAT etc...)
    }



^^^^^
Class
^^^^^

.. js:function:: class.instance()

returns a new instance of the class. this function does not invoke the instance constructor. The constructor must be explicitly called (eg. class_inst.constructor(class_inst) ).


.. js:function:: class.rawin(key)

returns true if the slot 'key' exists. the function has the same effect as the operator 'in' but does not employ delegation.


.. js:function:: class.weakref()

returns a weak reference to the object.


.. js:function:: class.tostring()

returns the string "(class : pointer)".


.. js:function:: class.rawget(key)

tries to get a value from the slot 'key' without employing delegation


.. js:function:: class.rawset(key,val)

sets the slot 'key' with the value 'val' without employing delegation. If the slot does not exists, it will be created.


.. js:function:: class.newmember(key,val,[bstatic])

sets/adds the slot 'key' with the value 'val' and if present invokes the _newmember metamethod. If bstatic is true the slot will be added as static. If the slot does not exists , it will be created.


.. js:function:: class.rawnewmember(key,val,[bstatic])

sets/adds the slot 'key' with the value 'val'. If bstatic is true the slot will be added as static. If the slot does not exist, it will be created. It doesn't invoke any metamethod.

.. js:function:: class.getfuncinfos()

If class has _call() metamethod, get info about it (see function.getfuncinfos() for details).

.. js:function:: class.__merge(table_or_class_1, [table_or_class_2], [table_or_class_3], ...)

This delegate is used to create new class from old and given.
Arguments to merge fields from can be tables, classes and instances.

.. js:function:: class.__update(table_1, [table_2], [table_3], ...)

This delegate is used to update new table with values from given ones.
In other words it mutates table with data from provided tables.

^^^^^^^^^^^^^^
Class Instance
^^^^^^^^^^^^^^

.. js:function:: instance.getclass()

returns the class that created the instance.


.. js:function:: instance.rawin(key)

    :param key: ze key

returns true if the slot 'key' exists. the function has the same effect as the operator 'in' but does not employ delegation.


.. js:function:: instance.weakref()

returns a weak reference to the object.


.. js:function:: instance.tostring()

tries to invoke the _tostring metamethod, if failed. returns "(instance : pointer)".


.. js:function:: instance.rawget(key)

tries to get a value from the slot 'key' without employing delegation


.. js:function:: instance.rawset(key,val)

sets the slot 'key' with the value 'val' without employing delegation. If the slot does not exists, it will be created.

.. js:function:: instance.getfuncinfos()

If instance has _call() metamethod, get info about it (see function.getfuncinfos() for details).

^^^^^^^^^^^^^^
Generator
^^^^^^^^^^^^^^


.. js:function:: generator.getstatus()

returns the status of the generator as string : "running", "dead" or "suspended".


.. js:function:: generator.weakref()

returns a weak reference to the object.


.. js:function:: generator.tostring()

returns the string "(generator : pointer)".

^^^^^^^^^^^^^^
Thread
^^^^^^^^^^^^^^

.. js:function:: thread.call(...)

starts the thread with the specified parameters


.. js:function:: thread.wakeup([wakeupval])

wakes up a suspended thread, accepts a optional parameter that will be used as return value for the function that suspended the thread(usually suspend())


.. js:function:: thread.wakeupthrow(objtothrow,[propagateerror = true])

wakes up a suspended thread, throwing an exception in the awaken thread, throwing the object 'objtothrow'.


.. js:function:: thread.getstatus()

returns the status of the thread ("idle","running","suspended")


.. js:function:: thread.weakref()

returns a weak reference to the object.


.. js:function:: thread.tostring()

returns the string "(thread : pointer)".


.. js:function:: thread.getstackinfos(stacklevel)

returns the stack frame informations at the given stack level (0 is the current function 1 is the caller and so on).

^^^^^^^^^^^^^^
Weak Reference
^^^^^^^^^^^^^^

.. js:function:: weakreference.ref()

returns the object that the weak reference is pointing at; null if the object that was point at was destroyed.


.. js:function:: weakreference.weakref()

returns a weak reference to the object.


.. js:function:: weakreference.tostring()

returns the string "(weakref : pointer)".

^^^^^^^^^^^^^^
Userdata
^^^^^^^^^^^^^^

.. js:function:: userdata.getfuncinfos()

If userdata has _call() metamethod in delegate, get info about it (see function.getfuncinfos() for details).
