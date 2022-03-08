/*
*Random number function from The Great Computer Language shootout
*converted to a generator func
*/

let function gen_random(max) {
    local last=42
    local IM = 139968
    local IA = 3877
    local IC = 29573
    for (;;) {  //loops forever
        last = (last * IA + IC) % IM
        yield (max * last / IM)
    }
}

let randtor = gen_random(100)

println("RAND NUMBERS")

for (local i=0;i<10;i+=1)
    println($"> {resume randtor}")

println("FIBONACCI")

let function fiboz(n) {
    local prev=0
    local curr=1
    yield 1

    for (local i=0;i<n-1;++i) {
        let res=prev+curr
        prev=curr
        curr=res
        yield curr
    }
    return prev+curr
}

foreach (val in fiboz(10)) {
    println($"> {val}")
}
