local arr=["one","two","three"]

::print("FOREACH\n")

foreach(i,val in arr) {
    ::print($"index [{i}]={val}\n")
}

::print("FOR\n")

local i
for(i=0;i<arr.len();++i){
    ::print($"index [{i}]={arr[i]}\n")
}

::print("WHILE\n")

i=0
while(i<arr.len()) {
    ::print($"index [{i}]={arr[i]}\n")
    ++i
}
::print("DO WHILE\n");

i=0
do {
    ::print($"index [{i}]={arr[i]}\n")
    ++i
}while(i<arr.len())
