/*  see copyright notice in squirrel.h */
#ifndef _SQSTD_STREAM_H_
#define _SQSTD_STREAM_H_

#ifdef __cplusplus

struct SQStream {
    virtual void _Release() = 0;
    virtual SQInteger Read(void *buffer, SQInteger size) = 0;
    virtual SQInteger Write(const void *buffer, SQInteger size) = 0;
    virtual SQInteger Flush() = 0;
    virtual SQInteger Tell() = 0;
    virtual SQInteger Len() = 0;
    virtual SQInteger Seek(SQInteger offset, SQInteger origin) = 0;
    virtual bool IsValid() = 0;
    virtual bool EOS() = 0;
    virtual SQInteger Close() { return 0; };
};

extern "C" {
#endif

#define SQ_SEEK_CUR 0
#define SQ_SEEK_END 1
#define SQ_SEEK_SET 2

extern SQUIRREL_API_VAR const struct tagSQRegClass _sqstd_stream_decl;
#define SQSTD_STREAM_TYPE_TAG ((SQUserPointer)(SQHash)&_sqstd_stream_decl)

typedef void *SQSTREAM;

SQUIRREL_API SQInteger sqstd_sread(SQUserPointer, SQInteger, SQSTREAM);
SQUIRREL_API SQInteger sqstd_swrite(SQUserPointer, SQInteger, SQSTREAM);
SQUIRREL_API SQInteger sqstd_sseek(SQSTREAM , SQInteger , SQInteger);
SQUIRREL_API SQInteger sqstd_stell(SQSTREAM);
SQUIRREL_API SQInteger sqstd_sflush(SQSTREAM);
SQUIRREL_API SQInteger sqstd_seof(SQSTREAM);
SQUIRREL_API SQInteger sqstd_sclose(SQSTREAM);
SQUIRREL_API void sqstd_srelease(SQSTREAM);

SQUIRREL_API SQInteger __sqstd_stream_releasehook(SQUserPointer p, SQInteger SQ_UNUSED_ARG(size));

SQUIRREL_API SQInteger sqstd_STREAMWRITEFUNC(SQUserPointer user,SQUserPointer buf,SQInteger size);
SQUIRREL_API SQInteger sqstd_STREAMREADFUNC(SQUserPointer user,SQUserPointer buf,SQInteger size);

SQUIRREL_API SQRESULT sqstd_compilestream(HSQUIRRELVM v,SQSTREAM stream,const SQChar *sourcename,SQBool raiseerror);
SQUIRREL_API SQRESULT sqstd_writeclosuretostream(HSQUIRRELVM vm,SQSTREAM stream);
SQUIRREL_API SQRESULT sqstd_readclosurestream(HSQUIRRELVM vm,SQSTREAM stream);
SQUIRREL_API SQRESULT sqstd_loadstream(HSQUIRRELVM v, SQSTREAM stream, const SQChar *sourcename, SQBool printerror);
SQUIRREL_API SQRESULT sqstd_dostream(HSQUIRRELVM v, SQSTREAM stream, const SQChar *sourcename, SQBool retval, SQBool printerror);

SQUIRREL_API SQRESULT sqstd_register_streamlib(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*_SQSTD_STREAM_H_*/
