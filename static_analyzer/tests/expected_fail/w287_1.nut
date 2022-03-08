//expect:w287

local curVal = ::a
local x = ::g_chat.rooms.len()

return (curVal < 0 || curVal > x)
