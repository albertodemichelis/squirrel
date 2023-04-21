let class BaseVector {
    constructor(...)     {
        if (vargv.len() >= 3) {
            this.x = vargv[0]
            this.y = vargv[1]
            this.z = vargv[2]
        }
    }

    x = 0
    y = 0
    z = 0
}

let class Vector3 extends BaseVector {
    function _add(other) {
        local cls = this.getclass()
        if (other instanceof cls)
            return cls(this.x+other.x, this.y+other.y, this.z+other.z)
        else
            throw "wrong parameter"
    }

    function Print() {
        println($"{this.x}, {this.y}, {this.z}")
    }
}

let v0 = Vector3(1,2,3)
let v1 = Vector3(11,12,13)
let v2 = v0 + v1
v2.Print()
