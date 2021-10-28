#include "sqmodules.h"
#include "path.h"

#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdblob.h>
#include <sqstdio.h>
#include <sqstdsystem.h>
#include <sqstddatetime.h>
#include <sqstdaux.h>
#include <sqdirect.h>

#include <string.h>
#include <assert.h>

#ifdef _WIN32
#  include <io.h>
#else
#  include <unistd.h>
#  define _access access
#endif

const char* SqModules::__main__ = "__main__";
const char* SqModules::__fn__ = nullptr;

static SQInteger persist_state(HSQUIRRELVM vm)
{
  // arguments:
  // 1 - module 'this', 2 - slot id, 3 - intitializer, 4 - state storage
  assert(sq_gettype(vm, 4) == OT_TABLE);

  const SQChar *slotName;
  sq_getstring(vm, 2, &slotName);

  sq_push(vm, 2); // slot id @ 5
  if (SQ_SUCCEEDED(sq_get_noerr(vm, 4)))
    return 1;

  HSQOBJECT mState;
  sq_getstackobj(vm, 4, &mState);

  sq_push(vm, 3); // initializer
  sq_push(vm, 1); // module 'this'
  if (SQ_FAILED(sq_call(vm, 1, SQTrue, SQTrue)))
    return sq_throwerror(vm, _SC("Failed to call initializer"));

  SQObjectType tp = sq_gettype(vm, -1);
  bool isMutable = tp==OT_TABLE || tp==OT_ARRAY || tp==OT_USERDATA || tp==OT_CLASS || tp==OT_INSTANCE;
  if (!isMutable)
  {
    const SQChar *slotName = nullptr;
    if (sq_getstring(vm, 2, &slotName))
      return sqstd_throwerrorf(vm, _SC("Persist '%s' is not mutable, probably an error"), slotName);
    else
      return sq_throwerror(vm, _SC("Persist is not mutable, probably an error"));
  }

  HSQOBJECT res;
  sq_getstackobj(vm, -1, &res);
  sq_addref(vm, &res);
  sq_pop(vm, 1); // initializer closure

  sq_push(vm, 2); // slot id
  sq_pushobject(vm, res); // initializer result
  sq_newslot(vm, 4, false);

  sq_pushobject(vm, res);
  sq_release(vm, &res);

  return 1;
}


static SQInteger keepref(HSQUIRRELVM vm)
{
  // arguments: this, object, ref holder
  sq_push(vm, 2); // push object
  sq_arrayappend(vm, 3); // append to ref holder
  sq_push(vm, 2); // push object again
  return 1;
}

void SqModules::resolveFileName(const char *requested_fn, std::string &res)
{
  res.clear();

  std::vector<char> scriptPathBuf;

  // try relative path first
  if (!runningScripts.empty())
  {
    const std::string &curFile = runningScripts.back();
    scriptPathBuf.resize(curFile.length()+2);
    dd_get_fname_location(&scriptPathBuf[0], curFile.c_str());
    dd_append_slash_c(&scriptPathBuf[0]);
    size_t locationLen = strlen(&scriptPathBuf[0]);
    size_t reqFnLen = strlen(requested_fn);
    scriptPathBuf.resize(locationLen + reqFnLen + 1);
    strcpy(&scriptPathBuf[locationLen], requested_fn);
    dd_simplify_fname_c(&scriptPathBuf[0]);
    scriptPathBuf.resize(strlen(&scriptPathBuf[0])+1);
    bool exists = _access(&scriptPathBuf[0], 0) == 0;
    if (exists)
      res.insert(res.end(), scriptPathBuf.begin(), scriptPathBuf.end());
  }

  if (res.empty())
  {
    scriptPathBuf.resize(strlen(requested_fn)+1);
    strcpy(&scriptPathBuf[0], requested_fn);
    dd_simplify_fname_c(&scriptPathBuf[0]);
    res.insert(res.end(), scriptPathBuf.begin(), scriptPathBuf.end());
  }
}


bool SqModules::checkCircularReferences(const char* resolved_fn, const char *)
{
  for (const std::string& scriptFn : runningScripts)
    if (scriptFn == resolved_fn)
      return false;
  return true;
}


SqModules::CompileScriptResult SqModules::compileScript(const char *resolved_fn, const char *requested_fn,
                                                        const HSQOBJECT *bindings,
                                                        SqObjPtr &script_closure, std::string &out_err_msg)
{
  script_closure.release();
  FILE* f = fopen(resolved_fn, "r");
  if (!f)
  {
    out_err_msg = std::string("Script file not found: ") + requested_fn +" / " + resolved_fn;
    return CompileScriptResult::FileNotFound;
  }

  std::vector<char> buf;

#ifdef _WIN32
  long len = _filelength(_fileno(f));
#else
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  if (len < 0)
  {
    fclose(f);
    out_err_msg = std::string("Cannot read script file: ") + requested_fn +" / " + resolved_fn;
    return CompileScriptResult::FileNotFound;
  }

  fseek(f, 0, SEEK_SET);
#endif

  buf.resize(len+1);
  fread(&buf[0], 1, len, f);
  buf[len] = 0;
  fclose(f);

  if (SQ_FAILED(sq_compilebuffer(sqvm, &buf[0], len, resolved_fn, true, bindings)))
  {
    out_err_msg = std::string("Failed to compile file: ") + requested_fn +" / " + resolved_fn;
    return CompileScriptResult::CompilationFailed;
  }

  script_closure.attachToStack(sqvm, -1);
  sq_pop(sqvm, 1);

  return CompileScriptResult::Ok;
}


SqModules::SqObjPtr SqModules::setupStateStorage(const char* resolved_fn)
{
  for (const Module &prevMod : prevModules)
    if (dd_fname_equal(prevMod.fn.c_str(), resolved_fn))
      return prevMod.stateStorage;

  HSQOBJECT ho;
  sq_newtable(sqvm);
  sq_getstackobj(sqvm, -1, &ho);
  SqObjPtr res(sqvm, ho);
  sq_poptop(sqvm);
  return res;
}

SqModules::Module * SqModules::findModule(const char * resolved_fn)
{
  for (Module &m : modules)
    if (dd_fname_equal(resolved_fn, m.fn.c_str()))
      return &m;

  return nullptr;
}


void SqModules::bindRequireApi(HSQOBJECT bindings)
{
  sq_pushobject(sqvm, bindings);

  sq_pushstring(sqvm, _SC("require"), 7);
  sq_pushuserpointer(sqvm, this);
  sq_newclosure(sqvm, sqRequire<true>, 1);
  sq_setparamscheck(sqvm, 2, _SC(".s"));
  sq_rawset(sqvm, -3);

  sq_pushstring(sqvm, _SC("require_optional"), 16);
  sq_pushuserpointer(sqvm, this);
  sq_newclosure(sqvm, sqRequire<false>, 1);
  sq_setparamscheck(sqvm, 2, _SC(".s"));
  sq_rawset(sqvm, -3);

  sq_poptop(sqvm); // bindings
}


bool SqModules::requireModule(const char *requested_fn, bool must_exist, const char *__name__,
                              SqObjPtr &exports, std::string &out_err_msg)
{
  out_err_msg.clear();
  exports.release();

  std::string resolvedFn;
  resolveFileName(requested_fn, resolvedFn);

  if (!checkCircularReferences(resolvedFn.c_str(), requested_fn))
  {
    out_err_msg = "Circular references error";
    // ^ will not be needed because of fatal() but keep it for the case of future changes
    return false;
  }

  if (Module * found = findModule(resolvedFn.c_str()))
  {
    exports = found->exports;
    return true;
  }

  HSQUIRRELVM vm = sqvm;
  SQInteger prevTop = sq_gettop(sqvm);
  (void)prevTop;


  sq_newtable(vm);
  HSQOBJECT hBindings;
  sq_getstackobj(vm, -1, &hBindings);
  SqObjPtr bindingsTbl(vm, hBindings); // add ref
  SqObjPtr stateStorage = setupStateStorage(resolvedFn.c_str());

  assert(sq_gettop(vm) == prevTop+1); // bindings table

  sq_pushstring(vm, "persist", 7);
  sq_pushobject(vm, stateStorage.o);
  sq_newclosure(vm, persist_state, 1);
  sq_setparamscheck(vm, 3, ".sc");
  sq_rawset(vm, -3);

  sq_newarray(vm, 0);
  SqObjPtr refHolder;
  refHolder.attachToStack(vm, -1);
  sq_poptop(vm);

  sq_pushstring(vm, "keepref", 7);
  sq_pushobject(vm, refHolder.o);
  sq_newclosure(vm, keepref, 1);
  sq_rawset(vm, -3);


  sq_pushstring(vm, "__name__", 8);
  sq_pushstring(vm, __name__, -1);
  sq_rawset(vm, -3);

  assert(sq_gettop(vm) == prevTop+1); // bindings table

  bindRequireApi(hBindings);

  assert(sq_gettop(vm) == prevTop+1); // bindings table
  sq_poptop(vm); //bindings table


  SqObjPtr scriptClosure;
  CompileScriptResult res = compileScript(resolvedFn.c_str(), requested_fn, &hBindings, scriptClosure, out_err_msg);
  if (!must_exist && res == CompileScriptResult::FileNotFound)
  {
    exports.release();
    return true;
  }
  if (res != CompileScriptResult::Ok)
    return false;

  if (__name__ == __fn__)
    __name__ = resolvedFn.c_str();

  size_t rsIdx = runningScripts.size();
  runningScripts.emplace_back(resolvedFn.c_str());


  sq_pushobject(vm, scriptClosure.o);
  sq_newtable(vm);

  assert(sq_gettype(vm, -1) == OT_TABLE);
  SqObjPtr objThis;
  objThis.attachToStack(vm, -1);


  SQRESULT callRes = sq_call(vm, 1, true, true);

  (void)rsIdx;
  assert(runningScripts.size() == rsIdx+1);
  runningScripts.pop_back();

  if (SQ_FAILED(callRes))
  {
    out_err_msg = std::string("Failed to run script ") + requested_fn + " / " + resolvedFn;
    sq_pop(vm, 1); // clojure, no return value on error
    return false;
  }

  HSQOBJECT hExports;
  sq_getstackobj(vm, -1, &hExports);
  exports = SqObjPtr(vm, hExports);

  sq_pop(vm, 2); // retval + closure

  assert(sq_gettop(vm) == prevTop);

  Module module;
  module.exports = exports;
  module.fn = resolvedFn;
  module.stateStorage = stateStorage;
  module.moduleThis = objThis;
  module.refHolder = refHolder;
  module.__name__ = __name__;

  modules.push_back(module);

  return true;
}


bool SqModules::reloadModule(const char *fn, bool must_exist, const char *__name__, SqObjPtr &exports, std::string &out_err_msg)
{
  assert(prevModules.empty());

  modules.swap(prevModules);
  modules.clear(); // just in case

  std::string errMsg;
  bool res = requireModule(fn, must_exist, __name__, exports, out_err_msg);

  prevModules.clear();

  return res;
}


bool SqModules::reloadAll(std::string &full_err_msg)
{
  assert(prevModules.empty());
  full_err_msg.clear();

  modules.swap(prevModules);
  modules.clear(); // just in case

  bool res = true;
  SqObjPtr exportsTmp;
  std::string errMsg;
  for (const Module &prev : prevModules)
  {
    if (!requireModule(prev.fn.c_str(), true, prev.__name__.c_str(), exportsTmp, errMsg))
    {
      res = false;
      full_err_msg += prev.fn + ": " + errMsg + "\n";
    }
  }

  prevModules.clear();

  return res;
}


bool SqModules::addNativeModule(const char *module_name, const SqObjPtr &exports)
{
  if (!module_name || !*module_name)
    return false;
  auto ins = nativeModules.insert({std::string(module_name), exports});
  return ins.second; // false if already registered
}


template<bool must_exist> SQInteger SqModules::sqRequire(HSQUIRRELVM vm)
{
  SQUserPointer selfPtr = nullptr;
  if (SQ_FAILED(sq_getuserpointer(vm, -1, &selfPtr)) || !selfPtr)
    return sq_throwerror(vm, "No module manager");

  SqModules *self = reinterpret_cast<SqModules *>(selfPtr);
  assert(self->sqvm == vm);

  const char *fileName = nullptr;
  SQInteger fileNameLen = -1;
  sq_getstringandsize(vm, 2, &fileName, &fileNameLen);

  if (strcmp(fileName, "squirrel.native_modules")==0)
  {
    sq_newarray(vm, self->nativeModules.size());
    SQInteger idx = 0;
    for (auto &nm : self->nativeModules)
    {
      sq_pushinteger(vm, idx);
      sq_pushstring(vm, nm.first.data(), -1);
      sq_set(vm, -3);
      ++idx;
    }
    return 1;
  }

  auto nativeIt = self->nativeModules.find(fileName);
  if (nativeIt != self->nativeModules.end())
  {
    sq_pushobject(vm, nativeIt->second.o);
    return 1;
  }

  SqObjPtr exports;
  std::string errMsg;
  if (!self->requireModule(fileName, must_exist, __fn__, exports, errMsg))
    return sq_throwerror(vm, errMsg.c_str());

  sq_pushobject(vm, exports.o);
  return 1;
}


void SqModules::registerBaseLibNativeModule(const char *name, RegFunc reg_func)
{
  HSQOBJECT hModule;
  sq_newtable(sqvm);
  sq_getstackobj(sqvm, -1, &hModule);
  reg_func(sqvm);
  bool regRes = addNativeModule(name, SqObjPtr(sqvm, hModule));
  assert(regRes);
  sq_pop(sqvm, 1);
}


void SqModules::registerBaseLibs()
{
  registerBaseLibNativeModule("math", sqstd_register_mathlib);
  registerBaseLibNativeModule("string", sqstd_register_stringlib);
  registerBaseLibNativeModule("blob", sqstd_register_bloblib);
}


void SqModules::registerSystemLib()
{
  registerBaseLibNativeModule("system", sqstd_register_systemlib);
}

void SqModules::registerIoLib()
{
  registerBaseLibNativeModule("io", sqstd_register_iolib);
}

void SqModules::registerDateTimeLib()
{
  registerBaseLibNativeModule("datetime", sqstd_register_datetimelib);
}
