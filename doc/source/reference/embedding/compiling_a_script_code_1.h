typedef SQInteger (*SQLEXREADFUNC)(SQUserPointer userdata);

SQRESULT sq_compile(HSQUIRRELVM v,SQREADFUNC read,SQUserPointer p,
            const SQChar *sourcename,SQBool raiseerror);
