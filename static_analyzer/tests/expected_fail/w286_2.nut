//expect:w286

local fn1 = @() true

return fn1 || ::fn2 //-const-in-bool-expr
