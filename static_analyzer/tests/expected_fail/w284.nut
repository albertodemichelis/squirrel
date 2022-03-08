//expect:w284

let function fn(x) {
  return ::y(x)
}

return fn(1) != null ? fn(1) : null
