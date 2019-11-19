/*translation of the methcall test from The Great Computer Language Shootout
*/

local system = require("system")

local class Toggle {
    bool=null

    function constructor(startstate) {
        bool = startstate
    }

    function value() {
        return bool
    }

    function activate() {
        bool = !bool
        return this
    }
}


local class NthToggle extends Toggle {
    count_max=null
    count=0

    function constructor(start_state, max_counter) {
        base.constructor(start_state)
        count_max = max_counter
    }

    function activate() {
        ++count
        if (count >= count_max) {
          base.activate()
          count = 0
        }
        return this
    }
}



local function main() {
    local n = vargv.len()!=0?vargv[0].tointeger():1

    local val = 1
    local toggle = Toggle(val)
    local i = n;
    while(i--) {
      val = toggle.activate().value()

    }
    print(toggle.value() ? "true\n" : "false\n")

    val = 1
    local ntoggle = NthToggle(val, 3)
    i = n
    while(i--) {
      val = ntoggle.activate().value()
    }
    print(ntoggle.value() ? "true\n" : "false\n")

}
local start=system.clock()
main()
print("TIME="+(system.clock()-start)+"\n");
