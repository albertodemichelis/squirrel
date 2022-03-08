//expect:w287

local value = ::a
local cnt = ::listObj.childrenCount()

return (value >= 0) && (value <= cnt)
