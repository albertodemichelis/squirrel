/* see copyright notice in squirrel.h */
#include <squirrel.h>
#include <sqstdaux.h>
#include <assert.h>

void sqstd_printcallstack(HSQUIRRELVM v)
{
    SQPRINTFUNCTION pf = sq_geterrorfunc(v);
    if(pf) {
        SQStackInfos si;
        SQInteger i;
        SQFloat f;
        const SQChar *s;
        SQInteger level=1; //1 is to skip this function that is level 0
        const SQChar *name=0;
        SQInteger seq=0;
        pf(v,_SC("\nCALLSTACK\n"));
        while(SQ_SUCCEEDED(sq_stackinfos(v,level,&si)))
        {
            const SQChar *fn=_SC("unknown");
            const SQChar *src=_SC("unknown");
            if(si.funcname)fn=si.funcname;
            if(si.source)src=si.source;
            pf(v,_SC("*FUNCTION [%s()] %s line [%d]\n"),fn,src,si.line);
            level++;
        }
        level=0;
        pf(v,_SC("\nLOCALS\n"));

        for(level=0;level<10;level++){
            seq=0;
            while((name = sq_getlocal(v,level,seq)))
            {
                seq++;
                switch(sq_gettype(v,-1))
                {
                case OT_NULL:
                    pf(v,_SC("[%s] NULL\n"),name);
                    break;
                case OT_INTEGER:
                    sq_getinteger(v,-1,&i);
                    pf(v,_SC("[%s] %d\n"),name,i);
                    break;
                case OT_FLOAT:
                    sq_getfloat(v,-1,&f);
                    pf(v,_SC("[%s] %.14g\n"),name,f);
                    break;
                case OT_USERPOINTER:
                    pf(v,_SC("[%s] USERPOINTER\n"),name);
                    break;
                case OT_STRING:
                    sq_getstring(v,-1,&s);
                    pf(v,_SC("[%s] \"%s\"\n"),name,s);
                    break;
                case OT_TABLE:
                    pf(v,_SC("[%s] TABLE\n"),name);
                    break;
                case OT_ARRAY:
                    pf(v,_SC("[%s] ARRAY\n"),name);
                    break;
                case OT_CLOSURE:
                    pf(v,_SC("[%s] CLOSURE\n"),name);
                    break;
                case OT_NATIVECLOSURE:
                    pf(v,_SC("[%s] NATIVECLOSURE\n"),name);
                    break;
                case OT_GENERATOR:
                    pf(v,_SC("[%s] GENERATOR\n"),name);
                    break;
                case OT_USERDATA:
                    pf(v,_SC("[%s] USERDATA\n"),name);
                    break;
                case OT_THREAD:
                    pf(v,_SC("[%s] THREAD\n"),name);
                    break;
                case OT_CLASS:
                    pf(v,_SC("[%s] CLASS\n"),name);
                    break;
                case OT_INSTANCE:
                    pf(v,_SC("[%s] INSTANCE\n"),name);
                    break;
                case OT_WEAKREF:
                    pf(v,_SC("[%s] WEAKREF\n"),name);
                    break;
                case OT_BOOL:{
                    SQBool bval;
                    sq_getbool(v,-1,&bval);
                    pf(v,_SC("[%s] %s\n"),name,bval == SQTrue ? _SC("true"):_SC("false"));
                             }
                    break;
                default: assert(0); break;
                }
                sq_pop(v,1);
            }
        }
    }
}

static SQInteger _sqstd_aux_printerror(HSQUIRRELVM v)
{
    SQPRINTFUNCTION pf = sq_geterrorfunc(v);
    if(pf) {
        const SQChar *sErr = 0;
        if(sq_gettop(v)>=1) {
            if(SQ_SUCCEEDED(sq_getstring(v,2,&sErr)))   {
                pf(v,_SC("\nAN ERROR HAS OCCURRED [%s]\n"),sErr);
            }
            else{
                pf(v,_SC("\nAN ERROR HAS OCCURRED [unknown]\n"));
            }
            sqstd_printcallstack(v);
        }
    }
    return 0;
}

void _sqstd_compiler_error(HSQUIRRELVM v,const SQChar *sErr,const SQChar *sSource,SQInteger line,SQInteger column)
{
    SQPRINTFUNCTION pf = sq_geterrorfunc(v);
    if(pf) {
        pf(v,_SC("%s line = (%d) column = (%d) : error %s\n"),sSource,line,column,sErr);
    }
}

void sqstd_seterrorhandlers(HSQUIRRELVM v)
{
    sq_setcompilererrorhandler(v,_sqstd_compiler_error);
    sq_newclosure(v,_sqstd_aux_printerror,0);
    sq_seterrorhandler(v);
}

SQInteger sqstd_registerfunctions(HSQUIRRELVM v,const SQRegFunction *fcts)
{
														// table
	if( fcts != 0) {
		const SQRegFunction	*f = fcts;
		while( f->name != 0) {
			SQBool bstatic = SQFalse;
			const SQChar *typemask = f->typemask;
			if( typemask && (typemask[0] == _SC('-'))) {	// if first char of typemask is '-', make static function
				typemask++;
				bstatic = SQTrue;
			}
			sq_pushstring(v,f->name,-1);				// table, method_name
			sq_newclosure(v,f->f,0);					// table, method_name, method_fct
			sq_setparamscheck(v,f->nparamscheck,typemask);
			sq_newslot(v,-3,bstatic);					// table
			f++;
		}
	}
    return SQ_OK;
}

SQInteger sqstd_registermembers(HSQUIRRELVM v,const SQRegMember *membs)
{
    if( membs != 0) {
		const SQRegMember	*m = membs;
		SQInteger top = sq_gettop(v);
		while( m->name != 0) {
            SQBool bstatic = SQFalse;
			SQBool handle_only= SQFalse;
            const SQChar *name = m->name;
			while( (*name == _SC('-')) || (*name == _SC('&')) ) {
				if( *name == '-') bstatic = SQTrue;
				else if( *name == '&') handle_only = SQTrue;
				name++;
			}
			if( !handle_only) {
				sq_pushstring(v,name,-1);					// class, name
				sq_pushnull(v);								// class, name, value
				sq_pushnull(v);								// class, name, value, attribute
				sq_newmember(v,-4,bstatic);					// class, [name, value, attribute] - bay be a bug (name, value, attribute not poped)
				sq_settop(v,top);							// class                           - workaround
			}
			if( m->phandle != 0) {
				sq_pushstring(v,name,-1);				// class, name
				sq_getmemberhandle(v,-2, m->phandle);	// class
			}
			m++;
		}
    }
	return SQ_OK;
}

SQInteger sqstd_registerclass(HSQUIRRELVM v,const SQRegClass *decl)
{															// root
    sq_pushregistrytable(v);								// root, registry
    sq_pushstring(v,decl->reg_name,-1);						// root, registry, reg_name
    if(SQ_FAILED(sq_get(v,-2))) {
															// root, registry
        sq_pop(v,1);										// root
		if( decl->base_class != 0) {
			sqstd_registerclass(v,decl->base_class);			// root, base_class
			sq_newclass(v,SQTrue);							// root, new_class
		}
		else {
			sq_newclass(v,SQFalse);							// root, new_class
		}
        sq_settypetag(v,-1,(SQUserPointer)(SQHash)decl);
		sqstd_registermembers(v,decl->members);
		sqstd_registerfunctions(v,decl->methods);
		
		sq_pushregistrytable(v);							// root, new_class, registry
		sq_pushstring(v,decl->reg_name,-1);					// root, new_class, registry, reg_name
		sq_push(v,-3);										// root, new_class, registry, reg_name, new_class
        sq_newslot(v,-3,SQFalse);							// root, new_class, registry
        sq_poptop(v);                                       // root, new_class
		if( decl->name != 0) {
			sq_pushstring(v,decl->name,-1);					// root, new_class, class_name
			sq_push(v,-2);									// root, new_class, class_name, new_class
			sq_newslot(v,-4,SQFalse);						// root, new_class
		}
		if( decl->globals) {
			sq_push(v,-2);									// root, new_class, root
			sqstd_registerfunctions(v,decl->globals);
			sq_poptop(v);									// root, new_class
		}
    }
    else {
															// root, registry, reg_class
		sq_remove(v,-2);									// root, reg_class
    }
    return SQ_OK;
}
