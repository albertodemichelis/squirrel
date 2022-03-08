//expect:w202
local x = 4
local canExecute = (x == 10)
if (x == 1 || x == 2 || x == 3 && canExecute)
  ::execute(x)