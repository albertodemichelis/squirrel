//expect:w285

local regions = ::unlock?.meta.regions ?? [::unlock?.meta.region] ?? []
return regions
