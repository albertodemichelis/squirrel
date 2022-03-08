let string = require("string")

{
  let ex = string.regexp("[a-zA-Z]+")
  let s = "123 Test; strlen(str);"
  let res = ex.search(s)
  println(s.slice(res.begin,res.end)) //prints "Test"
}

{
  let ex = string.regexp(@"\m()");
  let s = "123 Test; doSomething(str, getTemp(), (a+(b/c)));"
  let res = ex.search(s)
  println(s.slice(res.begin,res.end)) //prints "(...)"
}