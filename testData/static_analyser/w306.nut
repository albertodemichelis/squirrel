//expect:w223

local x = 1
local y = 1
local z = 2
if (x == y != z)
  ::print("a")

if ((x == y) != z)
  ::print("b")

if (x == (y != z))
  ::print("c")