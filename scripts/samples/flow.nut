if (min(100,200) > max(50,20))
    println("I'm useless statement just to show up the if/else")
else
    println("squirrel!!")

print("\n")

let function typy(obj) {
    switch(typeof obj) {
        case "integer":
        case "float":
            return "is a number"
        case "table":
        case "array":
            return "is a container"
        default:
            return "is other stuff"
    }
}

local a=1, b={}
local function c(a,b){return a+b}

println($"a {typy(a)}")
println($"b {typy(b)}")
println($"c {typy(c)}")
