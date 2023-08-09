//expect:w227

local class AAA { //-declared-never-used

  function fn() {
    local x = 6
    ::print(x)
  }

  x = 5
}
