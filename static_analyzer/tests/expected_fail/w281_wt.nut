//expect:w281

local getSeasonMainPrizesData = @() (::premiumUnlock.value?.meta.promo ?? [])
  .extend(::basicUnlock.value?.meta.promo ?? [])

return getSeasonMainPrizesData
