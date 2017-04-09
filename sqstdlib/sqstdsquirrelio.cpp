/* see copyright notice in sqtool.h */
#include <stddef.h>
#include <squirrel.h>
#include <sqstdaux.h>
#include <sqstdio.h>

#define IO_BUFFER_SIZE 2048

static SQInteger _sqstd_SQLEXREADFUNC(SQUserPointer user)
{
	SQFILE s = (SQFILE)user;
	unsigned char c;
	if( sqstd_fread( &c, 1, s) == 1)
		return c;
	return 0;
}

static SQInteger _sqstd_SQWRITEFUNC(SQUserPointer user,SQUserPointer buf,SQInteger size)
{
	SQFILE s = (SQFILE)user;
    return sqstd_fwrite( buf, size, s);
}

static SQInteger _sqstd_SQREADFUNC(SQUserPointer user,SQUserPointer buf,SQInteger size)
{
	SQFILE s = (SQFILE)user;
    SQInteger ret;
    if( (ret = sqstd_fread( buf, size, s))!=0 ) return ret;
    return -1;
}

SQRESULT sqstd_compilestream(HSQUIRRELVM v,SQFILE stream,const SQChar *sourcename,SQBool raiseerror)
{
	return sq_compile(v,_sqstd_SQLEXREADFUNC,(SQUserPointer)stream,sourcename,raiseerror);
}

SQRESULT sqstd_writeclosurestream(HSQUIRRELVM vm,SQFILE stream)
{
	return sq_writeclosure(vm,_sqstd_SQWRITEFUNC,(SQUserPointer)stream);
}

SQRESULT sqstd_readclosurestream(HSQUIRRELVM vm,SQFILE stream)
{
	return sq_readclosure(vm,_sqstd_SQREADFUNC,(SQUserPointer)stream);
}

SQRESULT sqstd_loadstream(HSQUIRRELVM v,SQFILE stream,const SQChar *filename,SQBool printerror,SQInteger buf_size,const SQChar *encoding,SQBool guess)
{
	if( buf_size < 0) buf_size = IO_BUFFER_SIZE;
	if( encoding == NULL) encoding = _SC("UTF-8");
	SQSRDR srdr = sqstd_streamreader(stream,SQFalse,buf_size);
	if( srdr != NULL) {
		if( SQ_SUCCEEDED(sqstd_srdrmark(srdr,16))) {
		    unsigned short us;
			if(sqstd_fread(&us,2,(SQFILE)srdr) != 2) {
				//probably an empty file
				us = 0;
			}
			sqstd_srdrreset(srdr);
			if(us == SQ_BYTECODE_STREAM_TAG) { //BYTECODE
				if(SQ_SUCCEEDED(sqstd_readclosurestream(v,(SQFILE)srdr))) {
					sqstd_frelease( (SQFILE)srdr);
					return SQ_OK;
				}
			}
			else { //SCRIPT
				SQFILE trdr = sqstd_textreader_srdr(srdr,SQFalse,SQFalse,encoding,guess);
				if( trdr != NULL) {
					if(SQ_SUCCEEDED(sqstd_compilestream(v,(SQFILE)trdr,filename,printerror))){
						sqstd_frelease( trdr);
						sqstd_frelease( (SQFILE)srdr);
						return SQ_OK;
					}
					sqstd_frelease(trdr);
				}
			}
		}
		sqstd_frelease( (SQFILE)srdr);
	}
    return sq_throwerror(v,_SC("cannot load the file"));
}

// sq api

SQRESULT sqstd_loadfile(HSQUIRRELVM v,const SQChar *filename,SQBool printerror)
{
    SQFILE file = sqstd_fopen(filename,_SC("rb"));
	if( file) {
		SQRESULT r = sqstd_loadstream(v,file,filename,printerror,-1,NULL,SQTrue);
		sqstd_frelease( file);
		return r;
	}
    return sq_throwerror(v,_SC("cannot open the file"));
}

SQRESULT sqstd_dofile(HSQUIRRELVM v,const SQChar *filename,SQBool retval,SQBool printerror)
{
    if(SQ_SUCCEEDED(sqstd_loadfile(v,filename,printerror))) {
        sq_push(v,-2);
        if(SQ_SUCCEEDED(sq_call(v,1,retval,SQTrue))) {
            sq_remove(v,retval?-2:-1); //removes the closure
            return 1;
        }
        sq_pop(v,1); //removes the closure
    }
    return SQ_ERROR;
}

SQRESULT sqstd_writeclosuretofile(HSQUIRRELVM v,const SQChar *filename)
{
    SQFILE file = sqstd_fopen(filename,_SC("wb+"));
    if(!file) return sq_throwerror(v,_SC("cannot open the file"));
    if(SQ_SUCCEEDED(sqstd_writeclosurestream(v,file))) {
        sqstd_frelease(file);
        return SQ_OK;
    }
    sqstd_frelease(file);
    return SQ_ERROR; //forward the error
}

// bindings

static SQInteger _g_stream_loadfile(HSQUIRRELVM v)
{
	SQInteger top = sq_gettop(v);
    SQBool printerror = SQFalse;
    if(top >= 3) {
        sq_getbool(v,3,&printerror);
    }
	if( sq_gettype(v,2) == OT_INSTANCE) {
		SQFILE file;
	    if( SQ_FAILED( sq_getinstanceup( v,2,(SQUserPointer*)&file,(SQUserPointer)SQSTD_STREAM_TYPE_TAG))) {
	        return sq_throwerror(v,_SC("invalid argument type"));
		}
		SQInteger buf_size = -1;
		const SQChar *encoding = NULL;
		SQBool guess = SQTrue;
		if(top >= 4) {
			sq_getinteger(v,4,&buf_size);
			if(top >= 5) {
				if( sq_gettype(v,5) == OT_STRING) {
					sq_getstring(v,5,&encoding);
				}
				if(top >= 6) {
					sq_getbool(v,6,&guess);
				}
			}
		}
		if(SQ_SUCCEEDED(sqstd_loadstream(v,file,_SC("stream"),printerror,buf_size,encoding,guess)))
			return 1;
	}
	else {
	    const SQChar *filename;
		sq_getstring(v,2,&filename);
		if(SQ_SUCCEEDED(sqstd_loadfile(v,filename,printerror)))
			return 1;
	}
    return SQ_ERROR; //propagates the error
}

static SQInteger _g_stream_dofile(HSQUIRRELVM v)
{
	if( SQ_SUCCEEDED(_g_stream_loadfile(v))) {
	    sq_push(v,1); //repush the this
        if(SQ_SUCCEEDED(sq_call(v,1,SQTrue,SQTrue))) {
            sq_remove(v,-2); //removes the closure
            return 1;
        }
        sq_pop(v,1); //removes the closure
	}
	return SQ_ERROR;
}

static SQInteger _g_stream_writeclosuretofile(HSQUIRRELVM v)
{
	if( sq_gettype(v,2) == OT_INSTANCE) {
		SQFILE file;
	    if( SQ_FAILED( sq_getinstanceup( v,2,(SQUserPointer*)&file,(SQUserPointer)SQSTD_STREAM_TYPE_TAG))) {
	        return sq_throwerror(v,_SC("invalid argument type"));
		}
		if(SQ_SUCCEEDED(sqstd_writeclosurestream(v,file)))
			return 1;
	}
	else {
		const SQChar *filename;
		sq_getstring(v,2,&filename);
		if(SQ_SUCCEEDED(sqstd_writeclosuretofile(v,filename)))
			return 1;
	}
    return SQ_ERROR; //propagates the error
}

#define _DECL_GLOBALSTREAM_FUNC(name,nparams,typecheck) {_SC(#name),_g_stream_##name,nparams,typecheck}
static const SQRegFunction _stream_funcs[]={
    _DECL_GLOBALSTREAM_FUNC(loadfile,-2,_SC(".s|xbio|sb")),
    _DECL_GLOBALSTREAM_FUNC(dofile,-2,_SC(".s|xbio|sb")),
    _DECL_GLOBALSTREAM_FUNC(writeclosuretofile,3,_SC(".s|xc")),
    {NULL,(SQFUNCTION)0,0,NULL}
};

SQRESULT sqstd_register_squirrelio(HSQUIRRELVM v)
{
	return sqstd_registerfunctions(v,_stream_funcs);
}
