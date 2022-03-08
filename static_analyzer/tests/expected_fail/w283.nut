//expect:w283

let function fn(x) { //-declared-never-used
  return ::y.cc ?? x ?? null
}
