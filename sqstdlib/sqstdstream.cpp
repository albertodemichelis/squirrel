/* see copyright notice in squirrel.h */
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <squirrel.h>
#include <sqstdaux.h>
#include <sqstdio.h>
#include <sqstdblob.h>
#include "sqstdstream.h"

// basic stream API

SQInteger sqstd_fread(void* buffer, SQInteger size, SQFILE file)
{
	SQStream *self = (SQStream *)file;
	return self->Read( buffer, size);
}

SQInteger sqstd_fwrite(const SQUserPointer buffer, SQInteger size, SQFILE file)
{
	SQStream *self = (SQStream *)file;
	return self->Write( buffer, size);
}

SQInteger sqstd_fseek(SQFILE file, SQInteger offset, SQInteger origin)
{
	SQStream *self = (SQStream *)file;
	return self->Seek( offset, origin);
}

SQInteger sqstd_ftell(SQFILE file)
{
	SQStream *self = (SQStream *)file;
	return self->Tell();
}

SQInteger sqstd_fflush(SQFILE file)
{
	SQStream *self = (SQStream *)file;
	return self->Flush();
}

SQInteger sqstd_feof(SQFILE file)
{
	SQStream *self = (SQStream *)file;
	return self->EOS();
}

SQInteger sqstd_fclose(SQFILE file)
{
	SQStream *self = (SQStream *)file;
	SQInteger r = self->Close();
	return r;
}

void sqstd_frelease(SQFILE file)
{
	SQStream *self = (SQStream *)file;
	self->Close();
	self->_Release();
}

SQInteger __sqstd_stream_releasehook(SQUserPointer p, SQInteger SQ_UNUSED_ARG(size))
{
	sqstd_frelease( (SQFILE)p);
    return 1;
}

#define SETUP_STREAM(v) \
    SQStream *self = NULL; \
    if(SQ_FAILED(sq_getinstanceup(v,1,(SQUserPointer*)&self,(SQUserPointer)((SQUnsignedInteger)SQSTD_STREAM_TYPE_TAG)))) \
        return sq_throwerror(v,_SC("invalid type tag")); \
    if(!self || !self->IsValid())  \
        return sq_throwerror(v,_SC("the stream is invalid"));

SQInteger _stream_readblob(HSQUIRRELVM v)
{
    SETUP_STREAM(v);
    SQUserPointer data,blobp;
    SQInteger size,res;
    sq_getinteger(v,2,&size);

    data = sq_getscratchpad(v,size);
    res = self->Read(data,size);
    if(res <= 0)
        return sq_throwerror(v,_SC("no data left to read"));
    blobp = sqstd_createblob(v,res);
    memcpy(blobp,data,res);
    return 1;
}

SQInteger _stream_readline(HSQUIRRELVM v)
{
    SETUP_STREAM(v);
	SQChar *buf = NULL;
	SQInteger buf_len = 0;
	SQInteger buf_pos = 0;
	SQInteger res;
	SQChar c = 0;

	while(1)
	{
		if( buf_pos >= buf_len)
		{
			buf_len += 1024;
			buf = sq_getscratchpad(v,buf_len);
		}
		
		res = self->Read( &c, sizeof(c));

		if( res != sizeof(c))
			break;
		else {
			buf[buf_pos] = c;
			buf_pos++;
			if( c == _SC('\n'))
				break;
		}
	}
	
	sq_pushstring(v,buf,buf_pos);
	
	return 1;
}

#define SAFE_READN(ptr,len) { \
    if(self->Read(ptr,len) != len) return sq_throwerror(v,_SC("io error")); \
    }
SQInteger _stream_readn(HSQUIRRELVM v)
{
    SETUP_STREAM(v);
    SQInteger format;
    sq_getinteger(v, 2, &format);
    switch(format) {
    case 'l': {
        SQInteger i;
        SAFE_READN(&i, sizeof(i));
        sq_pushinteger(v, i);
              }
        break;
    case 'i': {
        SQInt32 i;
        SAFE_READN(&i, sizeof(i));
        sq_pushinteger(v, i);
              }
        break;
    case 's': {
        short s;
        SAFE_READN(&s, sizeof(short));
        sq_pushinteger(v, s);
              }
        break;
    case 'w': {
        unsigned short w;
        SAFE_READN(&w, sizeof(unsigned short));
        sq_pushinteger(v, w);
              }
        break;
    case 'c': {
        char c;
        SAFE_READN(&c, sizeof(char));
        sq_pushinteger(v, c);
              }
        break;
    case 'b': {
        unsigned char c;
        SAFE_READN(&c, sizeof(unsigned char));
        sq_pushinteger(v, c);
              }
        break;
    case 'f': {
        float f;
        SAFE_READN(&f, sizeof(float));
        sq_pushfloat(v, f);
              }
        break;
    case 'd': {
        double d;
        SAFE_READN(&d, sizeof(double));
        sq_pushfloat(v, (SQFloat)d);
              }
        break;
    default:
        return sq_throwerror(v, _SC("invalid format"));
    }
    return 1;
}

SQInteger _stream_writeblob(HSQUIRRELVM v)
{
    SQUserPointer data;
    SQInteger size;
    SETUP_STREAM(v);
    if(SQ_FAILED(sqstd_getblob(v,2,&data)))
        return sq_throwerror(v,_SC("invalid parameter"));
    size = sqstd_getblobsize(v,2);
    if(self->Write(data,size) != size)
        return sq_throwerror(v,_SC("io error"));
    sq_pushinteger(v,size);
    return 1;
}

SQInteger _stream_print(HSQUIRRELVM v)
{
    const SQChar *data;
    SQInteger size;
    SETUP_STREAM(v);
    sq_getstring(v,2,&data);
    size = sq_getsize(v,2) * sizeof(*data);
    if(self->Write((const void*)data,size) != size)
        return sq_throwerror(v,_SC("io error"));
    sq_pushinteger(v,size);
    return 1;
}

SQInteger _stream_writen(HSQUIRRELVM v)
{
    SETUP_STREAM(v);
    SQInteger format, ti;
    SQFloat tf;
    sq_getinteger(v, 3, &format);
    switch(format) {
    case 'l': {
        SQInteger i;
        sq_getinteger(v, 2, &ti);
        i = ti;
        self->Write(&i, sizeof(SQInteger));
              }
        break;
    case 'i': {
        SQInt32 i;
        sq_getinteger(v, 2, &ti);
        i = (SQInt32)ti;
        self->Write(&i, sizeof(SQInt32));
              }
        break;
    case 's': {
        short s;
        sq_getinteger(v, 2, &ti);
        s = (short)ti;
        self->Write(&s, sizeof(short));
              }
        break;
    case 'w': {
        unsigned short w;
        sq_getinteger(v, 2, &ti);
        w = (unsigned short)ti;
        self->Write(&w, sizeof(unsigned short));
              }
        break;
    case 'c': {
        char c;
        sq_getinteger(v, 2, &ti);
        c = (char)ti;
        self->Write(&c, sizeof(char));
                  }
        break;
    case 'b': {
        unsigned char b;
        sq_getinteger(v, 2, &ti);
        b = (unsigned char)ti;
        self->Write(&b, sizeof(unsigned char));
              }
        break;
    case 'f': {
        float f;
        sq_getfloat(v, 2, &tf);
        f = (float)tf;
        self->Write(&f, sizeof(float));
              }
        break;
    case 'd': {
        double d;
        sq_getfloat(v, 2, &tf);
        d = tf;
        self->Write(&d, sizeof(double));
              }
        break;
    default:
        return sq_throwerror(v, _SC("invalid format"));
    }
    return 0;
}

SQInteger _stream_seek(HSQUIRRELVM v)
{
    SETUP_STREAM(v);
    SQInteger offset, origin = SQ_SEEK_SET;
    sq_getinteger(v, 2, &offset);
    if(sq_gettop(v) > 2) {
        SQInteger t;
        sq_getinteger(v, 3, &t);
        switch(t) {
            case 'b': origin = SQ_SEEK_SET; break;
            case 'c': origin = SQ_SEEK_CUR; break;
            case 'e': origin = SQ_SEEK_END; break;
            default: return sq_throwerror(v,_SC("invalid origin"));
        }
    }
    sq_pushinteger(v, self->Seek(offset, origin));
    return 1;
}

SQInteger _stream_tell(HSQUIRRELVM v)
{
    SETUP_STREAM(v);
    sq_pushinteger(v, self->Tell());
    return 1;
}

SQInteger _stream_len(HSQUIRRELVM v)
{
    SETUP_STREAM(v);
    sq_pushinteger(v, self->Len());
    return 1;
}

SQInteger _stream_flush(HSQUIRRELVM v)
{
    SETUP_STREAM(v);
    if(!self->Flush())
        sq_pushinteger(v, 1);
    else
        sq_pushnull(v);
    return 1;
}

SQInteger _stream_eos(HSQUIRRELVM v)
{
    SETUP_STREAM(v);
    if(self->EOS())
        sq_pushinteger(v, 1);
    else
        sq_pushnull(v);
    return 1;
}

static SQInteger _stream_close(HSQUIRRELVM v)
{
    SETUP_STREAM(v);
    sq_pushinteger(v, self->Close());
    return 1;
}

SQInteger _stream__cloned(HSQUIRRELVM v)
{
	 return sq_throwerror(v,_SC("this object cannot be cloned"));
}

static const SQRegFunction _stream_methods[] = {
    _DECL_STREAM_FUNC(readblob,2,_SC("xn")),
	_DECL_STREAM_FUNC(readline,1,_SC("x")),
    _DECL_STREAM_FUNC(readn,2,_SC("xn")),
    _DECL_STREAM_FUNC(writeblob,-2,_SC("xx")),
	_DECL_STREAM_FUNC(print,2,_SC("xs")),
    _DECL_STREAM_FUNC(writen,3,_SC("xnn")),
    _DECL_STREAM_FUNC(seek,-2,_SC("xnn")),
    _DECL_STREAM_FUNC(tell,1,_SC("x")),
    _DECL_STREAM_FUNC(len,1,_SC("x")),
    _DECL_STREAM_FUNC(eos,1,_SC("x")),
    _DECL_STREAM_FUNC(flush,1,_SC("x")),
    _DECL_STREAM_FUNC(close,1,_SC("x")),
    _DECL_STREAM_FUNC(_cloned,0,NULL),
    {NULL,(SQFUNCTION)0,0,NULL}
};

const SQRegClass _sqstd_stream_decl = {
	NULL,				// base_class
    _SC("std_stream"),	// reg_name
    _SC("stream"),		// name
	NULL,				// members
	_stream_methods,	// methods
	NULL,		// globals
};

SQRESULT sqstd_register_iolib(HSQUIRRELVM v)
{
    SQInteger top = sq_gettop(v);
	if(SQ_FAILED(sqstd_registerclass(v,&_sqstd_file_decl)))
	{
		return SQ_ERROR;
	}
	sq_poptop(v);

	if(SQ_FAILED(sqstd_registerclass(v,&_sqstd_streamreader_decl))) {
		return SQ_ERROR;
	}
 	sq_poptop(v);
    
	if(SQ_FAILED(sqstd_registerclass(v,&_sqstd_textreader_decl)))
	{
		return SQ_ERROR;
	}
 	sq_poptop(v);

	if(SQ_FAILED(sqstd_registerclass(v,&_sqstd_textwriter_decl)))
	{
		return SQ_ERROR;
	}
 	sq_poptop(v);

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
