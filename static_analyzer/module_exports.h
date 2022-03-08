#pragma once

#include "compilation_context.h"

#include <vector>
#include <string>

namespace moduleexports
{
  extern std::string csq_exe;

  bool module_export_collector(CompilationContext & ctx, int line, int col, const char * module_name = nullptr); // nullptr for roottable
  bool is_identifier_present_in_root(const char * name);
  bool get_module_export(const char * module_name, std::vector<std::string> & out_exports);
}
