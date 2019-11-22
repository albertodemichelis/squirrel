local class BaseVector {
    constructor(...)
    {
        if (vargv.len() >= 3) {
            x = vargv[0]
            y = vargv[1]
            z = vargv[2]
        }
    }


    x = 0
    y = 0
    z = 0
}

local class Vector3 extends BaseVector {
    function _add(other)
    {
        local cls = this.getclass()
        if (other instanceof cls)
            return cls(x+other.x,y+other.y,z+other.z)
        else
            throw "wrong parameter";
    }
    function Print()
    {
        ::print($"{x}, {y}, {z}\n")
    }
}

local v0 = Vector3(1,2,3)
local v1 = Vector3(11,12,13)
local v2 = v0 + v1
v2.Print()

::FakeNamespace <- {
    Utils = {}
}

FakeNamespace.Utils.SuperClass <- class  {
    constructor() {
        ::print("FakeNamespace.Utils.SuperClass")
    }
}

local testy = FakeNamespace.Utils.SuperClass()
