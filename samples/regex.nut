local string = require("string")

local ex = string.regexp("[a-zA-Z]+")
local s = "123 Test; strlen(str);"
local res = ex.search(s)
::print(s.slice(res.begin,res.end)) //prints "Test"
::print("\n")

ex = string.regexp(@"\m()");
s = "123 Test; doSomething(str, getTemp(), (a+(b/c)));"
res = ex.search(s)
::print(s.slice(res.begin,res.end)) //prints "(...)"
::print("\n")
