/* see copyright notice in squirrel.h */
#include <new>
#include <stdio.h>
#include <squirrel.h>
#include <sqstdaux.h>
#include <sqstdio.h>
#include "sqstdstream.h"

//File
struct SQFile : public SQStream {
    SQFile() { _handle = NULL; _owns = false;}
    SQFile( FILE *file, bool owns) { _handle = file; _owns = owns;}
	
    bool Open(const SQChar *filename ,const SQChar *mode) {
        Close();
#ifndef SQUNICODE
        if( (_handle = fopen(filename,mode)) ) {
#else
        if( (_handle = _wfopen(filename,mode)) ) {
#endif
            _owns = true;
            return true;
        }
        return false;
    }
    SQInteger Close() {
		SQInteger r = 0;
        if( _handle && _owns) {
            r = fclose( _handle);
        }
        _handle = NULL;
        _owns = false;
		return r;
    }
	void _Release() {
		this->~SQFile();
		sq_free(this,sizeof(SQFile));
	}
    SQInteger Read(void *buffer,SQInteger size) {
	    return (SQInteger)fread( buffer, 1, size, _handle);
    }
    SQInteger Write(const void *buffer,SQInteger size) {
	    return (SQInteger)fwrite( buffer, 1, size, _handle);
    }
    SQInteger Flush() {
	    return fflush( _handle);
    }
    SQInteger Tell() {
	    return ftell( _handle);
    }
    SQInteger Len() {
        SQInteger prevpos=Tell();
        if( Seek(0,SQ_SEEK_END) == 0) {
			SQInteger size=Tell();
			Seek(prevpos,SQ_SEEK_SET);
			return size;
		}
		return -1;
    }
    SQInteger Seek(SQInteger offset, SQInteger origin)  {
		int realorigin;
		switch(origin) {
			case SQ_SEEK_CUR: realorigin = SEEK_CUR; break;
			case SQ_SEEK_END: realorigin = SEEK_END; break;
			case SQ_SEEK_SET: realorigin = SEEK_SET; break;
			default: return -1; //failed
		}
		return fseek( _handle, (long)offset, (int)realorigin);
    }
    bool IsValid() { return _handle?true:false; }
    bool EOS() { return feof( _handle)?true:false; }
    FILE *GetHandle() {return _handle;}
protected:
    FILE *_handle;
    bool _owns;
};

SQFILE sqstd_fopen(const SQChar *filename ,const SQChar *mode)
{
    SQFile *f;
	
    f = new (sq_malloc(sizeof(SQFile)))SQFile();
	f->Open( filename, mode);
	if( f->IsValid())
	{
		return (SQFILE)f;
	}
	else
	{
		f->_Release();
		return NULL;
	}
}

static SQInteger _file__typeof(HSQUIRRELVM v)
{
    sq_pushstring(v,_sqstd_file_decl.name,-1);
    return 1;
}

static SQInteger _file_constructor(HSQUIRRELVM v)
{
    const SQChar *filename,*mode;
    SQFile *f;
    if(sq_gettype(v,2) == OT_STRING && sq_gettype(v,3) == OT_STRING) {
        sq_getstring(v, 2, &filename);
        sq_getstring(v, 3, &mode);
	    f = new (sq_malloc(sizeof(SQFile)))SQFile();
		f->Open( filename, mode);
		if( !f->IsValid())
		{
			f->_Release();
			return sq_throwerror(v, _SC("cannot open file"));
		}
    } else if(sq_gettype(v,2) == OT_USERPOINTER) {
	    FILE *fh;
	    bool owns = true;
        sq_getuserpointer(v,2,(SQUserPointer*)&fh);
        owns = !(sq_gettype(v,3) == OT_NULL);
	    f = new (sq_malloc(sizeof(SQFile)))SQFile(fh,owns);
    } else {
        return sq_throwerror(v,_SC("wrong parameter"));
    }

    if(SQ_FAILED(sq_setinstanceup(v,1,f))) {
		f->_Release();
        return sq_throwerror(v, _SC("cannot create file instance"));
    }
    sq_setreleasehook(v,1,__sqstd_stream_releasehook);
    return 0;
}

//bindings
#define _DECL_FILE_FUNC(name,nparams,typecheck) {_SC(#name),_file_##name,nparams,typecheck}
static const SQRegFunction _file_methods[] = {
    _DECL_FILE_FUNC(constructor,3,_SC("x")),
    _DECL_FILE_FUNC(_typeof,1,_SC("x")),
    {NULL,(SQFUNCTION)0,0,NULL}
};

const SQRegClass _sqstd_file_decl = {
	&_sqstd_stream_decl,	// base_class
    _SC("std_file"),	// reg_name
    _SC("file"),		// name
	NULL,				// members
	_file_methods,		// methods
	NULL,				// globals
};

SQRESULT sqstd_createfile(HSQUIRRELVM v, SQUserPointer file,SQBool own)
{
    SQInteger top = sq_gettop(v);
    sq_pushregistrytable(v);
    sq_pushstring(v,_sqstd_file_decl.reg_name,-1);
    if(SQ_SUCCEEDED(sq_get(v,-2))) {
        sq_remove(v,-2); //removes the registry
        sq_pushroottable(v); // push the this
        sq_pushuserpointer(v,file); //file
        if(own){
            sq_pushinteger(v,1); //true
        }
        else{
            sq_pushnull(v); //false
        }
        if(SQ_SUCCEEDED( sq_call(v,3,SQTrue,SQFalse) )) {
            sq_remove(v,-2);
            return SQ_OK;
        }
    }
    sq_settop(v,top);
    return SQ_ERROR;
}

SQRESULT sqstd_getfile(HSQUIRRELVM v, SQInteger idx, SQUserPointer *file)
{
    SQFile *fileobj = NULL;
    if(SQ_SUCCEEDED(sq_getinstanceup(v,idx,(SQUserPointer*)&fileobj,(SQUserPointer)SQSTD_FILE_TYPE_TAG))) {
        *file = fileobj->GetHandle();
        return SQ_OK;
    }
    return sq_throwerror(v,_SC("not a file"));
}

// SQRESULT sqstd_register_fileiolib(HSQUIRRELVM v)
// {
//     SQInteger top = sq_gettop(v);
// 	if(SQ_FAILED(sqstd_registerclass(v,&_sqstd_file_decl)))
// 	{
// 		return SQ_ERROR;
// 	}
// 	sq_poptop(v);
//     sq_pushstring(v,_SC("stdout"),-1);
//     sqstd_createfile(v,stdout,SQFalse);
//     sq_newslot(v,-3,SQFalse);
//     sq_pushstring(v,_SC("stdin"),-1);
//     sqstd_createfile(v,stdin,SQFalse);
//     sq_newslot(v,-3,SQFalse);
//     sq_pushstring(v,_SC("stderr"),-1);
//     sqstd_createfile(v,stderr,SQFalse);
//     sq_newslot(v,-3,SQFalse);
//     
//     sq_settop(v,top);
//     return SQ_OK;
// }
