/* see copyright notice in squirrel.h */
#include <squirrel.h>
#include "sqstdmodule.h"
#include <sqstdimport.h>
#include <string>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__unix)
#include <dlfcn.h>
#endif

typedef SQRESULT (*SQMODULELOAD)(HSQUIRRELVM v, HSQAPI api);

static HSQAPI sqapi = NULL;

static HSQAPI newapi() {
    HSQAPI sq = (HSQAPI)sq_malloc(sizeof(sq_api));

    /*vm*/
    sq->open = sq_open;
    sq->newthread = sq_newthread;
    sq->seterrorhandler = sq_seterrorhandler;
    sq->close = sq_close;
    sq->setforeignptr = sq_setforeignptr;
    sq->getforeignptr = sq_getforeignptr;
    sq->setprintfunc = sq_setprintfunc;
    sq->getprintfunc = sq_getprintfunc;
    sq->suspendvm = sq_suspendvm;
    sq->wakeupvm = sq_wakeupvm;
    sq->getvmstate = sq_getvmstate;

    /*compiler*/
    sq->compile = sq_compile;
    sq->compilebuffer = sq_compilebuffer;
    sq->enabledebuginfo = sq_enabledebuginfo;
    sq->notifyallexceptions = sq_notifyallexceptions;
    sq->setcompilererrorhandler = sq_setcompilererrorhandler;

    /*stack operations*/
    sq->push = sq_push;
    sq->pop = sq_pop;
    sq->poptop = sq_poptop;
    sq->remove = sq_remove;
    sq->gettop = sq_gettop;
    sq->settop = sq_settop;
    sq->reservestack = sq_reservestack;
    sq->cmp = sq_cmp;
    sq->move = sq_move;

    /*object creation handling*/
    sq->newuserdata = sq_newuserdata;
    sq->newtable = sq_newtable;
    sq->newarray = sq_newarray;
    sq->newclosure = sq_newclosure;
    sq->setparamscheck = sq_setparamscheck;
    sq->bindenv = sq_bindenv;
    sq->pushstring = sq_pushstring;
    sq->pushfloat = sq_pushfloat;
    sq->pushinteger = sq_pushinteger;
    sq->pushbool = sq_pushbool;
    sq->pushuserpointer = sq_pushuserpointer;
    sq->pushnull = sq_pushnull;
    sq->gettype = sq_gettype;
    sq->getsize = sq_getsize;
    sq->getbase = sq_getbase;
    sq->instanceof = sq_instanceof;
    sq->tostring = sq_tostring;
    sq->tobool = sq_tobool;
    sq->getstring = sq_getstring;
    sq->getinteger = sq_getinteger;
    sq->getthread = sq_getthread;
    sq->getbool = sq_getbool;
    sq->getstringandsize = sq_getstringandsize;
    sq->getuserpointer = sq_getuserpointer;
    sq->getuserdata = sq_getuserdata;
    sq->settypetag = sq_settypetag;
    sq->gettypetag = sq_gettypetag;
    sq->setreleasehook = sq_setreleasehook;
    sq->getscratchpad = sq_getscratchpad;
    sq->getclosureinfo = sq_getclosureinfo;
    sq->setnativeclosurename = sq_setnativeclosurename;
    sq->setinstanceup = sq_setinstanceup;
    sq->getinstanceup = sq_getinstanceup;
    sq->setclassudsize = sq_setclassudsize;
    sq->newclass = sq_newclass;
    sq->createinstance = sq_createinstance;
    sq->setattributes = sq_setattributes;
    sq->getattributes = sq_getattributes;
    sq->getclass = sq_getclass;
    sq->weakref = sq_weakref;
    sq->getdefaultdelegate = sq_getdefaultdelegate;

    /*object manipulation*/
    sq->pushroottable = sq_pushroottable;
    sq->pushregistrytable = sq_pushregistrytable;
    sq->pushconsttable = sq_pushconsttable;
    sq->setroottable = sq_setroottable;
    sq->setconsttable = sq_setconsttable;
    sq->newslot = sq_newslot;
    sq->deleteslot = sq_deleteslot;
    sq->set = sq_set;
    sq->get = sq_get;
    sq->rawset = sq_rawset;
    sq->rawget = sq_rawget;
    sq->rawdeleteslot = sq_rawdeleteslot;
    sq->arrayappend = sq_arrayappend;
    sq->arraypop = sq_arraypop;
    sq->arrayresize = sq_arrayresize;
    sq->arrayreverse = sq_arrayreverse;
    sq->arrayremove = sq_arrayremove;
    sq->arrayinsert = sq_arrayinsert;
    sq->setdelegate = sq_setdelegate;
    sq->getdelegate = sq_getdelegate;
    sq->clone = sq_clone;
    sq->setfreevariable = sq_setfreevariable;
    sq->next = sq_next;
    sq->getweakrefval = sq_getweakrefval;
    sq->clear = sq_clear;

    /*calls*/
    sq->call = sq_call;
    sq->resume = sq_resume;
    sq->getlocal = sq_getlocal;
    sq->getfreevariable = sq_getfreevariable;
    sq->throwerror = sq_throwerror;
    sq->reseterror = sq_reseterror;
    sq->getlasterror = sq_getlasterror;

    /*raw object handling*/
    sq->getstackobj = sq_getstackobj;
    sq->pushobject = sq_pushobject;
    sq->addref = sq_addref;
    sq->release = sq_release;
    sq->resetobject = sq_resetobject;
    sq->objtostring = sq_objtostring;
    sq->objtobool = sq_objtobool;
    sq->objtointeger = sq_objtointeger;
    sq->objtofloat = sq_objtofloat;
    sq->getobjtypetag = sq_getobjtypetag;

    /*GC*/
    sq->collectgarbage = sq_collectgarbage;

    /*serialization*/
    sq->writeclosure = sq_writeclosure;
    sq->readclosure = sq_readclosure;

    /*mem allocation*/
    sq->malloc = sq_malloc;
    sq->realloc = sq_realloc;
    sq->free = sq_free;

    /*debug*/
    sq->stackinfos = sq_stackinfos;
    sq->setdebughook = sq_setdebughook;

    return sq;
}

static SQRESULT importbin(HSQUIRRELVM v, const SQChar *modulename) {
    SQMODULELOAD modload = 0;
#if defined(_WIN32)
    HMODULE mod;
    mod = GetModuleHandle(modulename);
    if(mod == NULL) {
        mod = LoadLibrary(modulename);
        if(mod == NULL) {
            return SQ_ERROR;
        }
    }

    modload = (SQMODULELOAD)GetProcAddress(mod, "sqmodule_load");
    if(modload == NULL) {
        FreeLibrary(mod);
        return SQ_ERROR;
    }
#elif defined(__unix)
    std::basic_string<SQChar> library(modulename);
    library += _SC(".so");
    void *mod = dlopen(library.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NOLOAD);
    if(mod == NULL) {
        mod = dlopen(library.c_str(), RTLD_NOW | RTLD_LOCAL);
        if(mod == NULL) {
            return SQ_ERROR;
        }
    }

    modload = (SQMODULELOAD)dlsym(mod, "sqmodule_load");
    if(modload == NULL) {
        dlclose(mod);
        return SQ_ERROR;
    }
#endif
    if(sqapi == NULL) {
        sqapi = newapi();
    }

    SQRESULT res = modload(v, sqapi);
    return res;
}

SQRESULT import(HSQUIRRELVM v) {
    const SQChar *modulename;
    HSQOBJECT table;
    SQRESULT res;

    sq_getstring(v, -2, &modulename);
    sq_getstackobj(v, -1, &table);
    sq_addref(v, &table);

    sq_settop(v, 0);
    sq_pushobject(v, table);

    res = importbin(v, modulename);

    sq_settop(v, 0);
    sq_pushobject(v, table);
    sq_release(v, &table);

    return res;
}

static SQInteger base_import(HSQUIRRELVM v) {
    SQInteger args = sq_gettop(v);
    switch(args) {
    case 2:
        sq_pushroottable(v);
        break;
    case 3:
        break;
    default:
        return SQ_ERROR;
    }

    import(v);
    return 1;
}

SQRESULT sqstd_register_importlib(HSQUIRRELVM v) {
    sq_pushroottable(v);

    sq_pushstring(v, _SC("import"), -1);
    sq_newclosure(v, &base_import, 0);
    sq_newslot(v, -3, SQFalse);

    sq_pop(v, 1);
    return SQ_OK;
}