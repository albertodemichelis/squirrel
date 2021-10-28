/*
  This is not a production code, but a demonstration of concept of modules
*/


#pragma once

#include <squirrel.h>
#include <vector>
#include <string>
#include <unordered_map>

class SqModules
{
public:
  struct SqObjPtr
  {
    HSQUIRRELVM vm = nullptr;
	  HSQOBJECT o = {OT_NULL, 0};
    ~SqObjPtr() { sq_release(vm, &o); }
    SqObjPtr() : vm(nullptr) { sq_resetobject(&o); }
    SqObjPtr(HSQUIRRELVM vm_, HSQOBJECT ho) : vm(vm_), o(ho) { sq_addref(vm, &o); }
    SqObjPtr(const SqObjPtr &ptr) : vm(ptr.vm), o(ptr.o) { sq_addref(vm, &o); }
    SqObjPtr(SqObjPtr &&ptr) { sq_release(vm, &o); vm=ptr.vm; o=ptr.o; sq_resetobject(&ptr.o); }
    SqObjPtr& operator=(const SqObjPtr &ptr) { vm = ptr.vm; o = ptr.o; sq_addref(vm, &o); return *this; }
    SqObjPtr& operator=(SqObjPtr &&ptr) { sq_release(vm, &o); vm=ptr.vm; o=ptr.o; sq_resetobject(&ptr.o); return *this; }

    void attachToStack(HSQUIRRELVM vm_, SQInteger idx)
    {
      sq_release(vm_, &o);
      vm = vm_;
      sq_getstackobj(vm_, idx, &o);
      sq_addref(vm_, &o);
    }

    void release() { sq_release(vm, &o); sq_resetobject(&o); }
  };

  struct Module
  {
    std::string fn;
    std::string  __name__;
    SqObjPtr exports, stateStorage, refHolder, moduleThis;
  };

public:
  SqModules(HSQUIRRELVM vm)
    : sqvm(vm)
  {}

  HSQUIRRELVM getVM() { return sqvm; }

  // File name may be:
  // 1) relative to currently running script
  // 2) relative to base path
  // Note: accessing files outside of current base paths (via ../../)
  // can mess things up (breaking load module once rule)
  // __name__ is put to module this. Use __fn__ constant to use resolved filename or custom string to override
  bool  requireModule(const char *fn, bool must_exist, const char *__name__, SqObjPtr &exports, std::string &out_err_msg);
  // This can also be used for initial module execution
  bool  reloadModule(const char *fn, bool must_exist, const char *__name__, SqObjPtr &exports, std::string &out_err_msg);

  bool  reloadAll(std::string &err_msg);

  bool  addNativeModule(const char *module_name, const SqObjPtr &exports);

  void  registerBaseLibs();
  void  registerSystemLib();
  void  registerIoLib();
  void  registerDateTimeLib();

private:
  // Script API
  //   require(file_name, must_exist=true)
  template<bool must_exist> static SQInteger sqRequire(HSQUIRRELVM vm);

  void  bindRequireApi(HSQOBJECT bindings);

  void  resolveFileName(const char *fn, std::string &res);
  bool  checkCircularReferences(const char *resolved_fn, const char *orig_fn);
  enum class CompileScriptResult
  {
    Ok,
    FileNotFound,
    CompilationFailed
  };
  CompileScriptResult compileScript(const char *resolved_fn, const char *orig_fn, const HSQOBJECT *bindings,
                                    SqObjPtr &script_closure, std::string &out_err_msg);
  SqObjPtr  setupStateStorage(const char *resolved_fn);
  Module * findModule(const char * resolved_fn);

  typedef SQInteger (*RegFunc)(HSQUIRRELVM);
  void  registerBaseLibNativeModule(const char *name, RegFunc);


public:
  static const char *__main__, *__fn__;

private:
  std::vector<Module>  modules;
  std::vector<Module>  prevModules;

  std::unordered_map<std::string, SqObjPtr> nativeModules;
  std::vector<std::string>  runningScripts;
  HSQUIRRELVM sqvm = nullptr;
};
