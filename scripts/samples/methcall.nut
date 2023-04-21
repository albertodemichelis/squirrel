/*translation of the methcall test from The Great Computer Language Shootout
*/

let datetime = require("datetime")

let class Toggle {
    bool=null

    function constructor(startstate) {
        this.bool = startstate
    }

    function value() {
        return this.bool
    }

    function activate() {
        this.bool = !this.bool
        return this
    }
}


let class NthToggle extends Toggle {
    count_max=null
    count=0

    function constructor(start_state, max_counter) {
        base.constructor(start_state)
        this.count_max = max_counter
    }

    function activate() {
        ++this.count
        if (this.count >= this.count_max) {
          base.activate()
          this.count = 0
        }
        return this
    }
}



local function main() {
    let n = vargv.len()!=0?vargv[0].tointeger():1

    local val = 1
    let  toggle = Toggle(val)
    local i = n
    while(i--) {
      val = toggle.activate().value()
    }
    println(toggle.value() ? "true" : "false")

    val = 1
    local ntoggle = NthToggle(val, 3)
    i = n
    while(i--) {
      val = ntoggle.activate().value()
    }
    println(ntoggle.value() ? "true" : "false")

}

let start=datetime.clock()
main()
println("TIME="+(datetime.clock()-start))
