//expect:w282

local a = ::fn()
while (a := ::fn() || ::fn2())
  ::print(a)
