/*translation of the list test from The Great Computer Language Shootout
*/

let function compare_arr(a1,a2) {
    foreach(i,val in a1)
        if(val!=a2[i])
          return null
    return 1
}

let function test() {
    let size=10000
    let l1 = array(size)
    for (local i=0; i<size; ++i)
      l1[i]=i
    let l2=clone l1
    let  l3=[]

    l2.reverse()
    while(l2.len()>0)
        l3.append(l2.pop())
    while(l3.len()>0)
        l2.append(l3.pop())
    l1.reverse()

    if (compare_arr(l1,l2))
        return l1.len()
    return null
}

let n = vargv.len()!=0?vargv[0].tointeger():1

for(local i=0;i<n;++i) {
    if(!test()) {
        println("failed")
        return
    }
}

println("oki doki")
