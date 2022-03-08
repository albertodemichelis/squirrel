#include "module_exports.h"

#include <map>
#include <algorithm>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#  include <process.h>
#  define NULL_FILE_NAME "nul"
#else
#  include <unistd.h>
#  define NULL_FILE_NAME "/dev/null"
#endif

using namespace std;

namespace moduleexports
{
  string csq_exe = "csq";

  static int tmp_cnt = 0;

  //  module_file_name ("" = root), identifier, parents
  map< string, map<string, vector<string> > > module_content; // module content
  map< string, map<string, vector<string> > > module_to_root; // root table for each module


  //  identifier, parents
  map<string, vector<string> > root;

  const char * dump_sorted_module_code =
    #include "dump_sorted_module.nut.inl"
    ""
    ;

  bool module_export_collector(CompilationContext & ctx, int line, int col, const char * module_name) // nullptr for roottable
  {
    string moduleNameKey = ctx.fileDir + "#";
    moduleNameKey += module_name ? module_name : "";

    auto foundModuleRoot = module_to_root.find(moduleNameKey);
    if (foundModuleRoot != module_to_root.end())
    {
      if (!module_name)
        root = foundModuleRoot->second;
      else
        root.insert(foundModuleRoot->second.begin(), foundModuleRoot->second.end());

      return true;
    }

    if (module_name && strchr(module_name, '\"'))
    {
      ctx.error(71, "Export collector: Invalid module name.", line, col);
      return false;
    }

    char uniq[16] = { 0 };

#if defined(_WIN32)
    snprintf(uniq, sizeof(uniq), "%d", _getpid());
#else
    snprintf(uniq, sizeof(uniq), "%d", getpid());
#endif

    char nutFileName[512] = { 0 };
    char outputFileName[512] = { 0 };
    snprintf(nutFileName, sizeof(nutFileName), "%s%s~nut%s.%d.tmp", ctx.fileDir.c_str(),
      ctx.fileDir.empty() ? "" : "/", uniq, tmp_cnt);
    snprintf(outputFileName, sizeof(outputFileName), "~out%s.%d.tmp", uniq, tmp_cnt);
    tmp_cnt++;

    //tmpnam(nutFileName);
    //tmpnam(outputFileName);

    FILE * fnut = fopen(nutFileName, "wt");
    if (!fnut)
    {
      CompilationContext::setErrorLevel(ERRORLEVEL_FATAL);
      ctx.error(70, "Export collector: Cannot create temporary file.", line, col);
      return false;
    }

    if (module_name)
      fprintf(fnut, "local function dump_table() { return require(\"%s\"); }\n", module_name);

    fprintf(fnut, "%s", dump_sorted_module_code);
    fclose(fnut);

    if (system((csq_exe + " \"" + nutFileName + "\" > " + outputFileName).c_str()))
    {
      if (system((csq_exe + " --version > " NULL_FILE_NAME).c_str()))
      {
        CompilationContext::setErrorLevel(ERRORLEVEL_FATAL);
        ctx.error(72, string("Export collector: '" + csq_exe +
          "' not found. You may disable warnings -w242 and -w246 to continue.").c_str(),
          line, col);
        remove(outputFileName);
        remove(nutFileName);
        return false;
      }

      ctx.error(73, "Export collector: code of module executed with errors:\n", line, col);
      system((csq_exe + " --dont-print-table " + nutFileName).c_str());
      remove(outputFileName);
      remove(nutFileName);
      return false;
    }

    remove(nutFileName);

    FILE * fout = fopen(outputFileName, "rt");
    if (!fout)
    {
      CompilationContext::setErrorLevel(ERRORLEVEL_FATAL);
      ctx.error(73, "Export collector: cannot open results of module execution.", line, col);
      return false;
    }


    map<string, vector<string> > moduleContent;
    map<string, vector<string> > moduleRoot;

    string emptyString = string();

    char buffer[255] = { 0 };
    bool isError = false;
    while (fgets(buffer, sizeof(buffer) - 1, fout) != NULL)
    {
      bool isRoot = !strncmp(buffer, ".R. ", 4) && !module_name;
      bool isAddRoot = !strncmp(buffer, ".A. ", 4) && module_name;
      bool isModule = !strncmp(buffer, ".M. ", 4) && module_name;
      isError |= !strncmp(buffer, ".E. ", 4);

      map<string, vector<string> > & addTo = isModule ? moduleContent : moduleRoot;

      if (isRoot || isAddRoot || isModule)
      {
        const char * s = buffer + 4 - 1;
        vector<string> names;
        do
        {
          s++;
          const char * dot = strchr(s, '.');
          if (dot)
            names.push_back(string(s, dot - s));
          else
          {
            const char * nline = strchr(s, '\n');
            if (!nline)
              names.push_back(string(s));
            else
              names.push_back(string(s, nline - s));
          }
          s = dot;
        } while (s);


        string & parent = names[0];
        string & child = names.size() > 1 ? names[1] : emptyString;

        auto it = addTo.find(parent);
        if (it == addTo.end())
        {
          vector<string> str;
          str.push_back(child);
          addTo.insert(make_pair(parent, str));
        }
        else
        {
          if (find(it->second.begin(), it->second.end(), child) == it->second.end())
            it->second.push_back(child);
        }
      }
    }

    if (!module_name)
      root = moduleRoot;
    else
      root.insert(moduleRoot.begin(), moduleRoot.end());

    module_content.insert(make_pair(moduleNameKey, moduleContent));
    module_to_root.insert(make_pair(moduleNameKey, moduleRoot));

    fclose(fout);
    remove(outputFileName);

    if (isError && moduleContent.empty() && moduleRoot.empty())
    {
      CompilationContext::setErrorLevel(ERRORLEVEL_FATAL);
      ctx.error(74, (string("Export collector: failed to require '") +
        (module_name ? module_name : "<null>") + "'.").c_str(), line, col);
      return false;
    }

    return true;
  }


  bool is_identifier_present_in_root(const char * name)
  {
    if (!name || !name[0])
      return false;

    auto it = root.find(name);
    return (it != root.end());
  }


  bool is_identifier_present_in_module(const char * module_name, const char * ident_name)
  {
    if (!module_name || !module_name[0] || !ident_name || !ident_name[0])
      return false;

    auto it = module_content.find(module_name);
    if (it == module_content.end())
      return false;

    auto inModuleIt = it->second.find(ident_name);
    if (inModuleIt == it->second.end())
      return false;

    return true;
  }


  bool get_module_export(const char * module_name, vector<string> & out_exports)
  {
    out_exports.clear();

    if (!module_name)
      return false;

    auto it = module_content.find(module_name);
    if (it == module_content.end())
      return false;

    for (auto && e : it->second)
      out_exports.push_back(e.first);

    return true;
  }

} // namespace
