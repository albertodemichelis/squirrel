//expect:w256

const C = 1

let _t = {
  wrongName = function foo(_p) {},

  [C] = function bar(_p) {},

  "anotherWrongName" : function qux() {}
}
