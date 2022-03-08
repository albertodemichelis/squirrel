let arr=["one","two","three"]

print("FOREACH\n")

foreach (i,val in arr) {
    println($"index [{i}]={val}")
}

println("FOR")

local i
for(i=0;i<arr.len();++i){
    println($"index [{i}]={arr[i]}")
}

println("WHILE")

i=0
while(i<arr.len()) {
    println($"index [{i}]={arr[i]}")
    ++i
}

println("DO WHILE");

i=0
do {
    println($"index [{i}]={arr[i]}")
    ++i
}while(i<arr.len())
