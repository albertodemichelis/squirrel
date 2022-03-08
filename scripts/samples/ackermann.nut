/*
*
* Original Javascript version by David Hedbor(http://www.bagley.org/~doug/shootout/)
*
*/

local function Ack(M, N) {
    if (M == 0) return N + 1
    if (N == 0) return Ack(M-1, 1)
    return Ack(M - 1, Ack(M, (N-1)))
}

local n

if(vargv.len()!=0) {
  n = max(1, vargv[0].tointeger())
} else {
  n = 1
}

println($"n={n}")
println($"Ack(3,{n}):{Ack(3, n)}")
