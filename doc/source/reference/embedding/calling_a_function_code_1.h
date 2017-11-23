sq_pushroottable(v);
sq_pushstring(v,"foo",-1);
sq_get(v,-2); //get the function from the root table
sq_pushroottable(v); //'this' (function environment object)
sq_pushinteger(v,1);
sq_pushfloat(v,2.0);
sq_pushstring(v,"three",-1);
sq_call(v,4,SQFalse,SQFalse);
sq_pop(v,2); //pops the roottable and the function