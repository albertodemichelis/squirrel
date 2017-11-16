HSQOBJECT obj;

sq_resetobject(&obj) //initialize the handle
sq_getstackobj(v,-2,&obj); //retrieve an object handle from the pos -2
sq_addref(v,&obj); //adds a reference to the object

// do stuff

sq_pushobject(v,obj); //push the object in the stack
sq_release(v,&obj); //relese the object