/*  see copyright notice in sqstdstreamreader.cpp */
#ifndef _SQSTDSTREAMREADER_H_
#define _SQSTDSTREAMREADER_H_

#ifdef __cplusplus
extern "C" {
#endif

extern SQUIRREL_API_VAR const struct tagSQRegClass _sqstd_streamreader_decl;
#define SQSTD_STREAMREADER_TYPE_TAG ((SQUserPointer)(SQHash)&_sqstd_streamreader_decl)

typedef struct tagSQSRDR *SQSRDR;

SQUIRREL_API SQSRDR sqstd_streamreader(SQSTREAM stream,SQBool owns,SQInteger buffer_size);
SQUIRREL_API SQInteger sqstd_srdrmark(SQSRDR srdr,SQInteger readAheadLimit);
SQUIRREL_API SQInteger sqstd_srdrreset(SQSRDR srdr);

SQUIRREL_API SQRESULT sqstd_register_streamreaderlib(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // _SQSTDSTREAMREADER_H_
