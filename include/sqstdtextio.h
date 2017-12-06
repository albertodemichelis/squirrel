/*  see copyright notice in sqstdtextio.cpp */
#ifndef _SQSTDTEXTIO_H_
#define _SQSTDTEXTIO_H_

#ifdef __cplusplus
extern "C" {
#endif

extern SQUIRREL_API_VAR const struct tagSQRegClass _sqstd_textreader_decl;
#define SQSTD_TEXTREADER_TYPE_TAG ((SQUserPointer)(SQHash)&_sqstd_textreader_decl)
extern SQUIRREL_API_VAR const struct tagSQRegClass _sqstd_textwriter_decl;
#define SQSTD_TEXTWRITER_TYPE_TAG ((SQUserPointer)(SQHash)&_sqstd_textwriter_decl)

SQUIRREL_API SQFILE sqstd_textreader(SQSTREAM stream,SQBool owns,const SQChar *encoding,SQBool guess);
SQUIRREL_API SQFILE sqstd_textwriter(SQSTREAM stream,SQBool owns,const SQChar *encoding);

SQUIRREL_API const SQChar *sqstd_guessencoding_srdr(SQSRDR srdr);
SQUIRREL_API SQFILE sqstd_textreader_srdr(SQSRDR srdr,SQBool owns_close,SQBool owns_release,const SQChar *encoding,SQBool guess);

SQUIRREL_API SQRESULT sqstd_register_textiolib(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // _SQSTDTEXTIO_H_
