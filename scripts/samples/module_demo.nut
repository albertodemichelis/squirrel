local foo = require("module_1.nut")
local {bar, baz} = require("module_2.nut")

::print($"foo() result = {foo()}\n")
::print($"bar = {bar}, baz = {baz}\n")
