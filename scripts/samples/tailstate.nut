local state1, state2, state3

state1 = function state1() {
    suspend("state1")
    return state2()
}

state2 = function state2() {
    suspend("state2")
    return state3()
}

state3 = function state3() {
    suspend("state3")
    return state1()
}

let statethread = newthread(state1)

println(statethread.call())

for (local i = 0; i < 10000; i++)
    println(statethread.wakeup())
