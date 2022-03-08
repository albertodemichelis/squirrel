//expect:w285

local regions = ::x ? [] : {}
return regions ?? 123
