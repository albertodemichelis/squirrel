local base_vec = {
    function _add(n) {
        return { x=x+n.x, y=y+n.y, z=z+n.z }
    }

    function _sub(n) {
        return { x=x-n.x, y=y-n.y, z=z-n.z }
    }

    function _div(n) {
        return { x=x/n.x, y=y/n.y, z=z/n.z }
    }

    function _mul(n) {
        return { x=x*n.x, y=y*n.y, z=z*n.z }
    }

    function _modulo(n) {
        return { x=x%n, y=y%n, z=z%n }
    }
    
    function _typeof() {
      return "vector"
    }
    
    function _get(key) {
        if(key==100) {
            return test_field
        }
    }

    function _set(key,val) {
        if (key==100) {
            test_field=val
            return test_field
        }
    }

    test_field="nothing"
}

let function vector(_x,_y,_z) {
    return {x=_x,y=_y,z=_z }.setdelegate(base_vec)
}

////////////////////////////////////////////////////////////

let v1=vector(1.5,2.5,3.5)
let v2=vector(1.5,2.5,3.5)

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

println(v1[100])
v1[100]="set SUCCEEDED"
println(v1[100])

if(typeof v1=="vector")
    println("<SUCCEEDED>")
else
    println("<FAILED>")
