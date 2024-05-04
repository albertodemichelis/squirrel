
#include "squirrel.h"
#include <sqstdblob.h>
#include <sqstdsystem.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdaux.h>

typedef SQRESULT (*libsptrlist_t)(HSQUIRRELVM v);

static const char* const libsstrlist[] =
{
    "string",
    "blob",
    "io",
    "system",
    "math",
};

static const libsptrlist_t libsptrlist[] =
{
    sqstd_register_stringlib,    
    sqstd_register_bloblib,
    sqstd_register_iolib,
    sqstd_register_systemlib,
    sqstd_register_mathlib,
};

SQInteger sq_include(HSQUIRRELVM v)
{
    const SQChar* incl;
    if (SQ_SUCCEEDED(sq_getstring(v, 2, &incl)))
    {
        for (size_t i = 0; (sizeof(libsstrlist) / sizeof(libsstrlist[0])) > i; i++)
        {

            if (strcmp(incl, libsstrlist[i]) == 0)
            {
#if _DEBUG
                sq_getprintfunc(v)(v, "%s lib loaded!", incl);
#endif
                sq_pushroottable(v);
                return libsptrlist[i](v);
            }
        }
#if _DEBUG
        sq_getprintfunc(v)(v, "Unknown lib: %s", incl);
#endif
        return SQ_ERROR;

    }
    return SQ_ERROR;
}

SQRESULT sqstd_register_includelib(HSQUIRRELVM v)
{
    sq_pushstring(v, "include", -1);
    sq_newclosure(v, sq_include, 0);
    return sq_newslot(v, -3, 0);
}