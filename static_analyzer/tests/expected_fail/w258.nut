//expect:w258

::Aaa <- class {
  function requestUpdateEventLb(x) {}
  canRequestEventLb = false
  leaderboardsRequestStack = []

  function updateEventLb(requestData, id) {
    local requestAction = (@(requestData, id) function () {
      local taskId = ::requestUpdateEventLb(requestData)
      if (taskId < 0)
        return

      canRequestEventLb = false
      ::add_bg_task_cb(taskId, (@(requestData, id) function() {
        ::handleLbRequest(requestData, id)

        if (leaderboardsRequestStack.len())
          ::array_shift(leaderboardsRequestStack).fn()
        else
          canRequestEventLb = true
      })(requestData, id).bindenv(this))
    })(requestData, id).bindenv(this)

    if (canRequestEventLb)
      return requestAction()

    if (id)
      foreach (index, request in leaderboardsRequestStack)
        if (id == request)
          leaderboardsRequestStack.remove(index)

    leaderboardsRequestStack.append({fn = requestAction, id = id})
  }



  function updateEventLbSelfRow(requestData, id) {
    local requestAction = (@(requestData, id) function () {
      local taskId = ::requestEventLbSelfRow(requestData)
      if (taskId < 0)
        return

      canRequestEventLb = false
      ::add_bg_task_cb(taskId, (@(requestData, id) function() {
        ::handleLbSelfRowRequest(requestData, id)

        if (leaderboardsRequestStack.len())
          ::array_shift(leaderboardsRequestStack).fn()
        else
          canRequestEventLb = true
      })(requestData, id).bindenv(this))
    })(requestData, id).bindenv(this)

    if (canRequestEventLb)
      return requestAction()

    if (id)
      foreach (index, request in leaderboardsRequestStack)
        if (id == request)
          leaderboardsRequestStack.remove(index)

    leaderboardsRequestStack.append({fn = requestAction, id = id})
  }
}