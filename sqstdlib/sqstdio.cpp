/* see copyright notice in squirrel.h */
#include <new>
#include <stdio.h>
#include <squirrel.h>
#include <sqstdaux.h>
#include <sqstdio.h>

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

SQInteger sqstd_fread(void* buffer, SQInteger size, SQInteger count, SQFILE file)
{
    SQFile *self = (SQFile*)file;
	SQInteger ret = self->Read( buffer, size * count);
    if( ret > 0) {
        ret %= size;
    }
    return ret;
}

SQInteger sqstd_fwrite(SQUserPointer buffer, SQInteger size, SQInteger count, SQFILE file)
{
    SQFile *self = (SQFile*)file;
	SQInteger ret = self->Write( buffer, size * count);
    if( ret > 0) {
        ret %= size;
    }
    return ret;
}

SQInteger sqstd_fclose(SQFILE file)
{
    sqstd_srelease( (SQSTREAM)file);
    return SQ_OK;
}

static SQInteger _file__typeof(HSQUIRRELVM v)
{
    sq_pushstring(v,_sqstd_file_decl.name,-1);
    return 1;
}

static SQInteger _file_constructor(HSQUIRRELVM v)
{
    bool owns = true;
    SQFile *f;
    if(sq_gettype(v,2) == OT_STRING && sq_gettype(v,3) == OT_STRING) {
        const SQChar *filename,*mode;
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
        sq_getuserpointer(v,2,(SQUserPointer*)&f);
        owns = !(sq_gettype(v,3) == OT_NULL);
    } else {
        return sq_throwerror(v,_SC("wrong parameter"));
    }

    if(SQ_FAILED(sq_setinstanceup(v,1,f))) {
        if( owns) {
            f->_Release();
        }
        return sq_throwerror(v, _SC("cannot create file instance"));
    }
    if( owns) {
        sq_setreleasehook(v,1,__sqstd_stream_releasehook);
    }
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

SQRESULT sqstd_createsqfile(HSQUIRRELVM v, SQFILE file,SQBool own)
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

SQRESULT sqstd_createfile(HSQUIRRELVM v, SQUserPointer file,SQBool own)
{
    SQFile *f = new (sq_malloc(sizeof(SQFile)))SQFile((FILE*)file,own);
    return sqstd_createsqfile(v,f,SQTrue);
}

SQUIRREL_API SQRESULT sqstd_getsqfile(HSQUIRRELVM v, SQInteger idx, SQFILE *file)
{
    SQFile *fileobj = NULL;
    if(SQ_SUCCEEDED(sq_getinstanceup(v,idx,(SQUserPointer*)&fileobj,(SQUserPointer)SQSTD_FILE_TYPE_TAG))) {
        *file = (SQFILE)fileobj;
        return SQ_OK;
    }
    return sq_throwerror(v,_SC("not a file"));
}

SQRESULT sqstd_opensqfile(HSQUIRRELVM v, const SQChar *filename ,const SQChar *mode)
{
    SQInteger top = sq_gettop(v);
    sq_pushregistrytable(v);                                // registry
    sq_pushstring(v,_sqstd_file_decl.reg_name,-1);          // registry, "file"
    if(SQ_SUCCEEDED(sq_get(v,-2))) {                        // registry, file_class
        sq_remove(v,-2); //removes the registry             // file_class
        sq_pushnull(v); // push fake this                   // file_class, fake_this
        sq_pushstring(v,filename,-1); //file name           // file_class, fake_this, filename
        sq_pushstring(v,mode,-1); //mode                    // file_class, fake_this, filename, mode
        if(SQ_SUCCEEDED( sq_call(v,3,SQTrue,SQFalse) )) {   // file_class, stream
            sq_remove(v,-2);                                // stream
            return SQ_OK;
        }
    }
    sq_settop(v,top);
    return SQ_ERROR;
}

// sq api file

SQRESULT sqstd_loadfile(HSQUIRRELVM v,const SQChar *filename,SQBool printerror)
{
    SQFILE file = sqstd_fopen(filename,_SC("rb"));
	if( file) {
		SQRESULT r = sqstd_loadstream(v,(SQSTREAM)file,filename,printerror);
        sqstd_fclose(file);
		return r;
	}
    return sq_throwerror(v,_SC("cannot open the file"));
}

SQRESULT sqstd_dofile(HSQUIRRELVM v,const SQChar *filename,SQBool retval,SQBool printerror)
{
    //at least one entry must exist in order for us to push it as the environment
    if(sq_gettop(v) == 0)
        return sq_throwerror(v,_SC("environment table expected"));

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
    if(SQ_SUCCEEDED(sqstd_writeclosuretostream(v,(SQSTREAM)file))) {
        sqstd_fclose(file);
        return SQ_OK;
    }
    sqstd_fclose(file);
    return SQ_ERROR; //forward the error
}

// bindings

static SQInteger _g_io_loadfile(HSQUIRRELVM v)
{
    const SQChar *filename;
    SQBool printerror = SQFalse;
    sq_getstring(v,2,&filename);
    if(sq_gettop(v) >= 3) {
        sq_getbool(v,3,&printerror);
    }
    if(SQ_SUCCEEDED(sqstd_loadfile(v,filename,printerror)))
        return 1;
    return SQ_ERROR; //propagates the error
}

static SQInteger _g_io_writeclosuretofile(HSQUIRRELVM v)
{
    const SQChar *filename;
    sq_getstring(v,2,&filename);
    if(SQ_SUCCEEDED(sqstd_writeclosuretofile(v,filename)))
        return 1;
    return SQ_ERROR; //propagates the error
}

static SQInteger _g_io_dofile(HSQUIRRELVM v)
{
    const SQChar *filename;
    SQBool printerror = SQFalse;
    sq_getstring(v,2,&filename);
    if(sq_gettop(v) >= 3) {
        sq_getbool(v,3,&printerror);
    }
    sq_push(v,1); //repush the this
    if(SQ_SUCCEEDED(sqstd_dofile(v,filename,SQTrue,printerror)))
        return 1;
    return SQ_ERROR; //propagates the error
}

#define _DECL_GLOBALIO_FUNC(name,nparams,typecheck) {_SC(#name),_g_io_##name,nparams,typecheck}
static const SQRegFunction iolib_funcs[]={
    _DECL_GLOBALIO_FUNC(loadfile,-2,_SC(".sb")),
    _DECL_GLOBALIO_FUNC(dofile,-2,_SC(".sb")),
    _DECL_GLOBALIO_FUNC(writeclosuretofile,3,_SC(".sc")),
    {NULL,(SQFUNCTION)0,0,NULL}
};

SQRESULT sqstd_register_iolib(HSQUIRRELVM v)
{
    SQInteger top = sq_gettop(v);
	if(SQ_FAILED(sqstd_registerclass(v,&_sqstd_file_decl)))
	{
		return SQ_ERROR;
	}
	sq_poptop(v);
    
	sqstd_registerfunctions(v,iolib_funcs);
    
    sq_pushstring(v,_SC("stdout"),-1);
    sqstd_createfile(v,stdout,SQFalse);
    sq_newslot(v,-3,SQFalse);
    sq_pushstring(v,_SC("stdin"),-1);
    sqstd_createfile(v,stdin,SQFalse);
    sq_newslot(v,-3,SQFalse);
    sq_pushstring(v,_SC("stderr"),-1);
    sqstd_createfile(v,stderr,SQFalse);
    sq_newslot(v,-3,SQFalse);
    
    sq_settop(v,top);
    return SQ_OK;
}
