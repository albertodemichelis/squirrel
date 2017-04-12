/*  see copyright notice in squirrel.h */
#ifndef _SQSTDIO_H_
#define _SQSTDIO_H_

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

extern SQUIRREL_API_VAR const struct tagSQRegClass _sqstd_stream_decl;
#define SQSTD_STREAM_TYPE_TAG ((SQUserPointer)(SQHash)&_sqstd_stream_decl)
extern SQUIRREL_API_VAR const struct tagSQRegClass _std_blob_decl;
#define SQSTD_BLOB_TYPE_TAG ((SQUserPointer)(SQHash)&_std_blob_decl)
extern SQUIRREL_API_VAR const struct tagSQRegClass _sqstd_file_decl;
#define SQSTD_FILE_TYPE_TAG ((SQUserPointer)(SQHash)&_sqstd_file_decl)
extern SQUIRREL_API_VAR const struct tagSQRegClass _sqstd_popen_decl;
#define SQSTD_POPEN_TYPE_TAG ((SQUserPointer)(SQHash)&_sqstd_popen_decl)
extern SQUIRREL_API_VAR const struct tagSQRegClass _sqstd_textreader_decl;
#define SQSTD_TEXTREADER_TYPE_TAG ((SQUserPointer)(SQHash)&_sqstd_textreader_decl)
extern SQUIRREL_API_VAR const struct tagSQRegClass _sqstd_textwriter_decl;
#define SQSTD_TEXTWRITER_TYPE_TAG ((SQUserPointer)(SQHash)&_sqstd_textwriter_decl)
extern SQUIRREL_API_VAR const struct tagSQRegClass _sqstd_streamreader_decl;
#define SQSTD_STREAMREADER_TYPE_TAG ((SQUserPointer)(SQHash)&_sqstd_streamreader_decl)

#define SQ_SEEK_CUR 0
#define SQ_SEEK_END 1
#define SQ_SEEK_SET 2

typedef struct SQFILE_tag *SQFILE;	// SQFILE represents SQStream

SQUIRREL_API SQInteger sqstd_fread(SQUserPointer, SQInteger, SQFILE);
SQUIRREL_API SQInteger sqstd_fwrite(const SQUserPointer, SQInteger, SQFILE);
SQUIRREL_API SQInteger sqstd_fseek(SQFILE , SQInteger , SQInteger);
SQUIRREL_API SQInteger sqstd_ftell(SQFILE);
SQUIRREL_API SQInteger sqstd_fflush(SQFILE);
SQUIRREL_API SQInteger sqstd_feof(SQFILE);
SQUIRREL_API SQInteger sqstd_fclose(SQFILE);
SQUIRREL_API void sqstd_frelease(SQFILE);

SQUIRREL_API SQInteger __sqstd_stream_releasehook(SQUserPointer p, SQInteger SQ_UNUSED_ARG(size));

SQUIRREL_API SQFILE sqstd_fopen(const SQChar *,const SQChar *);
SQUIRREL_API SQRESULT sqstd_createfile(HSQUIRRELVM v, SQUserPointer file, SQBool own);
SQUIRREL_API SQRESULT sqstd_getfile(HSQUIRRELVM v, SQInteger idx, SQUserPointer *file);

SQUIRREL_API SQFILE sqstd_blob(SQInteger size);

SQUIRREL_API SQFILE sqstd_textreader(SQFILE stream,SQBool owns,const SQChar *encoding,SQBool guess);
SQUIRREL_API SQFILE sqstd_textwriter(SQFILE stream,SQBool owns,const SQChar *encoding);

typedef struct tagSQSRDR *SQSRDR;
SQUIRREL_API SQSRDR sqstd_streamreader(SQFILE stream,SQBool owns,SQInteger buffer_size);
SQUIRREL_API SQInteger sqstd_srdrmark(SQSRDR srdr,SQInteger readAheadLimit);
SQUIRREL_API SQInteger sqstd_srdrreset(SQSRDR srdr);
SQUIRREL_API const SQChar *sqstd_guessencoding_srdr(SQSRDR srdr);
SQUIRREL_API SQFILE sqstd_textreader_srdr(SQSRDR srdr,SQBool owns_close,SQBool owns_release,const SQChar *encoding,SQBool guess);

//compiler helpers
SQUIRREL_API SQRESULT sqstd_compilestream(HSQUIRRELVM v,SQFILE stream,const SQChar *sourcename,SQBool raiseerror);
SQUIRREL_API SQRESULT sqstd_writeclosurestream(HSQUIRRELVM vm,SQFILE stream);
SQUIRREL_API SQRESULT sqstd_readclosurestream(HSQUIRRELVM vm,SQFILE stream);
SQUIRREL_API SQRESULT sqstd_loadstream(HSQUIRRELVM v,SQFILE stream,const SQChar *filename,SQBool printerror,SQInteger buf_size,const SQChar *encoding,SQBool guess);
SQUIRREL_API SQRESULT sqstd_loadfile(HSQUIRRELVM v,const SQChar *filename,SQBool printerror);
SQUIRREL_API SQRESULT sqstd_dofile(HSQUIRRELVM v,const SQChar *filename,SQBool retval,SQBool printerror);
SQUIRREL_API SQRESULT sqstd_writeclosuretofile(HSQUIRRELVM v,const SQChar *filename);

SQUIRREL_API SQRESULT sqstd_register_iolib(HSQUIRRELVM v);
SQUIRREL_API SQRESULT sqstd_register_squirrelio(HSQUIRRELVM v);
//SQUIRREL_API SQRESULT sqstd_register_textreader(HSQUIRRELVM v);
//SQUIRREL_API SQRESULT sqstd_register_streamreader(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*_SQSTDIO_H_*/

