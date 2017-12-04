/* see copyright notice in sqtool.h */
#include <stddef.h>
#include <squirrel.h>
#include <sqstdaux.h>
#include <sqstdio.h>
#include <sqstdstreamreader.h>
#include <sqstdtextio.h>

#define IO_BUFFER_SIZE 2048

static SQInteger _sqstd_SQLEXREADFUNC(SQUserPointer user)
{
	SQSTREAM s = (SQSTREAM)user;
	unsigned char c;
	if( sqstd_sread( &c, 1, s) == 1)
		return c;
	return 0;
}

// sq api stream

SQRESULT sqstd_compilestream(HSQUIRRELVM v,SQSTREAM stream,const SQChar *sourcename,SQBool raiseerror)
{
	return sq_compile(v,_sqstd_SQLEXREADFUNC,(SQUserPointer)stream,sourcename,raiseerror);
}

SQRESULT sqstd_writeclosuretostream(HSQUIRRELVM vm,SQSTREAM stream)
{
	return sq_writeclosure(vm,sqstd_STREAMWRITEFUNC,(SQUserPointer)stream);
}

SQRESULT sqstd_readclosurestream(HSQUIRRELVM vm,SQSTREAM stream)
{
	return sq_readclosure(vm,sqstd_STREAMREADFUNC,(SQUserPointer)stream);
}

SQRESULT sqstd_loadstreamex(HSQUIRRELVM v,SQSTREAM stream, const SQChar *sourcename,SQBool printerror, SQInteger buf_size, const SQChar *encoding, SQBool guess)
{
	if( buf_size < 0) buf_size = IO_BUFFER_SIZE;
	if( encoding == NULL) encoding = _SC("UTF-8");
	SQSRDR srdr = sqstd_streamreader(stream,SQFalse,buf_size);
	if( srdr != NULL) {
		if( SQ_SUCCEEDED(sqstd_srdrmark(srdr,16))) {
		    unsigned short us;
			if(sqstd_sread(&us,2,(SQSTREAM)srdr) != 2) {
				//probably an empty file
				us = 0;
			}
			sqstd_srdrreset(srdr);
			if(us == SQ_BYTECODE_STREAM_TAG) { //BYTECODE
				if(SQ_SUCCEEDED(sqstd_readclosurestream(v,(SQSTREAM)srdr))) {
					sqstd_srelease( (SQSTREAM)srdr);
					return SQ_OK;
				}
			}
			else { //SCRIPT
				SQSTREAM trdr = sqstd_textreader_srdr(srdr,SQFalse,SQFalse,encoding,guess);
				if( trdr != NULL) {
					if(SQ_SUCCEEDED(sqstd_compilestream(v,trdr,sourcename,printerror))){
						sqstd_srelease( trdr);
						sqstd_srelease( (SQSTREAM)srdr);
						return SQ_OK;
					}
					sqstd_srelease(trdr);
				}
			}
		}
		sqstd_srelease( (SQSTREAM)srdr);
	}
    return sq_throwerror(v,_SC("cannot load the file"));
}

SQRESULT sqstd_loadstream(HSQUIRRELVM v, SQSTREAM stream, const SQChar *sourcename, SQBool printerror)
{
    return sqstd_loadstreamex(v,stream,sourcename,printerror,-1,NULL,SQTrue);
}

SQRESULT sqstd_dostream(HSQUIRRELVM v, SQSTREAM stream, const SQChar *sourcename, SQBool retval, SQBool printerror)
{
    //at least one entry must exist in order for us to push it as the environment
    if(sq_gettop(v) == 0)
        return sq_throwerror(v,_SC("environment table expected"));

    if(SQ_SUCCEEDED(sqstd_loadstream(v,stream,sourcename,printerror))) {
        sq_push(v,-2);
        if(SQ_SUCCEEDED(sq_call(v,1,retval,SQTrue))) {
            sq_remove(v,retval?-2:-1); //removes the closure
            return 1;
        }
        sq_pop(v,1); //removes the closure
    }
    return SQ_ERROR;
}
