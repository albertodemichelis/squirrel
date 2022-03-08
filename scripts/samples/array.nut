/*
*
* Original Javascript version by David Hedbor(http://www.bagley.org/~doug/shootout/)
*
*/
local n, i, k

if (vargv.len()!=0) {
  n = ::max(1, vargv[0].tointeger())
} else {
  n = 1
}

let x = array(n)
let y = array(n)

for (i = 0; i < n; i+=1) {
  x[i] = i + 1
  y[i] = 0
}

for (k = 0 ; k < n; k+=1) {
  for (i = n-1; i >= 0; i-=1) {
    y[i] = y[i]+ x[i]
  }
}

println($"{y[0]} {y[n-1]}")

