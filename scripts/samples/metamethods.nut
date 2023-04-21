let class Vector {
    x = 0
    y = 0
    z = 0
    constructor(x, y, z) {
        this.x = x
        this.y = y
        this.z = z
    }
    function _add(n) {
        return this.getclass()(this.x+n.x, this.y+n.y, this.z+n.z)
    }

    function _sub(n) {
        return this.getclass()(this.x-n.x, this.y-n.y, this.z-n.z)
    }

    function _div(n) {
        return this.getclass()(this.x/n.x, this.y/n.y, this.z/n.z)
    }

    function _mul(n) {
        return this.getclass()(this.x*n.x, this.y*n.y, this.z*n.z)
    }

    function _modulo(n) {
        return this.getclass()(this.x%n, this.y%n, this.z%n)
    }
    
    function _typeof() {
        return "Vector"
    }
}


////////////////////////////////////////////////////////////

let v1=Vector(1.5,2.5,3.5)
let v2=Vector(1.5,2.5,3.5)

local r=v1+v2

foreach(i,val in r) {
    println($"{i} = {val}")
}

r=v1*v2

foreach(i,val in r) {
    println($"{i} = {val}")
}

r=v1/v2

foreach(i,val in r) {
    println($"{i} = {val}")
}

r=v1-v2

foreach(i,val in r) {
    println($"{i} = {val}")
}

r=v1%2

foreach(i,val in r) {
    println($"{i} = {val}")
}

if (typeof v1=="Vector")
    println("<SUCCEEDED>")
else
    println("<FAILED>")
