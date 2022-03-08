//expect:w282

local invite_info = ::fn()
if (local uid := invite_info?.leader.id.tostring()!=null)
  ::addInviteByContact(::Contact(uid))
