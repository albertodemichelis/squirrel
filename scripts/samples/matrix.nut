/*
*
* Original Javascript version by David Hedbor(http://www.bagley.org/~doug/shootout/)
*
*/
const SIZE=30

local function mkmatrix(rows, cols) {
  local count = 1
  local m = ::array(rows)
  for (local i = 0; i < rows; ++i) {
    m[i] = ::array(cols)
    for (local j = 0; j < cols; ++j) {
      ++count
      m[i][j] = count
    }
  }
  return m
}

local function mmult(rows, cols,  m1, m2, m3) {
  for (local i = 0; i < rows; ++i) {
    for (local j = 0; j < cols; ++j) {
      local val = 0
      for (local k = 0; k < cols; ++k) {
        val += m1[i][k] * m2[k][j]
      }
      m3[i][j] = val
    }
  }
  return m3
}

local n = vargv.len()!=0?vargv[0].tointeger():1

local m1 = mkmatrix(SIZE, SIZE)
local m2 = mkmatrix(SIZE, SIZE)
local mm = mkmatrix(SIZE, SIZE)

for (local i = 0; i < n; i+=1) {
  mmult(SIZE, SIZE, m1, m2, mm)
}

print(mm[0][0]+" "+mm[2][3]+" "+mm[3][2]+" "+mm[4][4])
