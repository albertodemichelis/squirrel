/*  see copyright notice in squirrel.h */
#ifndef _SQSTD_AUXLIB_H_
#define _SQSTD_AUXLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

SQUIRREL_API void sqstd_seterrorhandlers(HSQUIRRELVM v);
SQUIRREL_API void sqstd_printcallstack(HSQUIRRELVM v);

typedef struct tagSQRegMember {
    const SQChar *name;
    HSQMEMBERHANDLE *phandle;
} SQRegMember;

typedef struct tagSQRegClass {
	const struct tagSQRegClass *base_class;
    const SQChar *reg_name;
    const SQChar *name;
    const SQRegMember *members;
	const SQRegFunction	*methods;
	const SQRegFunction	*globals;
} SQRegClass;

SQUIRREL_API SQInteger sqstd_registerclass(HSQUIRRELVM v,const SQRegClass *decl);
SQUIRREL_API SQInteger sqstd_registermembers(HSQUIRRELVM v,const SQRegMember *membs);
SQUIRREL_API SQInteger sqstd_registerfunctions(HSQUIRRELVM v,const SQRegFunction *fcts);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* _SQSTD_AUXLIB_H_ */
