//expect:w259

local numAnimations = ::a + ::b + ::c + ::d - (::a + ::b + ::c + ::d) * ::x + 123
local numTextAnimations = ::a + ::b + ::c + ::d - (::a + ::b + ::c + ::d) * ::x + 124

return [numAnimations, numTextAnimations]
