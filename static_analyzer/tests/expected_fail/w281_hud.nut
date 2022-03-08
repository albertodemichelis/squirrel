//expect:w281

local tab = {
  watch = (::alertWatched ? ::alertWatched : []).extend(::titleWatched ? ::titleWatched : [])
}

return tab
