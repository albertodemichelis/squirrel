#include "sqpcheader.h"
#include <sqconfig.h>

#if SQ_VAR_TRACE_ENABLED == 1

#include "squtils.h"
#include "sqvm.h"
#include "vartrace.h"

#define VT_FLAG_STRING (1<<0)
#define VT_FLAG_ELLIPSIS (1<<1)

HSQUIRRELVM VarTrace::vm = NULL;
volatile bool VarTrace::enabled = true;

bool VarTrace::isStacksEqual(int a, int b)
{
  for (int i = 0; i < VAR_TRACE_STACK_DEPTH; i++)
  {
    if (history[a].stack[i].line == STACK_NOT_INITIALIZED && history[b].stack[i].line == STACK_NOT_INITIALIZED)
      return true;

    if (history[a].stack[i].line != history[b].stack[i].line ||
      history[a].stack[i].fileName != history[b].stack[i].fileName)
    {
      return false;
    }
  }

  return true;
}

void VarTrace::saveStack(const SQObject & value, HSQUIRRELVM var_vm)
{
  if (!enabled)
    return;

  setCnt++;

  if (!var_vm)
    var_vm = vm;

  if (!var_vm)
    return;

  SQStackInfos si;
  SQInteger level = 0;
  history[pos].count = 1;

  const int valSize = sizeof(history[pos].val);

  history[pos].val[valSize - 2] = 0;

  SQObjectPtr obj(value);
  SQObjectPtr res;
  if (var_vm->ToString(obj, res))
  {
    const SQChar *valueAsString = sq_objtostring(&res);
    if (valueAsString)
    {
      strncpy(history[pos].val, valueAsString, valSize);

      history[pos].flags = (history[pos].val[valSize - 2] != 0 ? VT_FLAG_ELLIPSIS : 0) |
        (sq_isstring(value) ? VT_FLAG_STRING : 0);

      history[pos].val[valSize - 1] = 0;
    }
  }


  while (SQ_SUCCEEDED(sq_stackinfos(var_vm, level, &si)))
  {
    const SQChar *src = _SC("unknown");
    if (si.source)
      src = si.source;
    history[pos].stack[level].fileName = src;
    history[pos].stack[level].line = si.line;
    level++;
    if (level >= VAR_TRACE_STACK_DEPTH)
      break;
  }

  if (level < VAR_TRACE_STACK_DEPTH)
    history[pos].stack[level].line = STACK_NOT_INITIALIZED;

  int prevPos = (pos - 1 + VAR_TRACE_STACK_DEPTH) % VAR_TRACE_STACK_DEPTH;

  if (memcmp(&history[pos].val, &history[prevPos].val, sizeof(history[prevPos].val)) == 0 &&
      isStacksEqual(pos, prevPos))
  {
    history[pos].stack[0].line = STACK_NOT_INITIALIZED;
    history[prevPos].count++;
  }
  else
  {
    pos++;
    if (pos >= VAR_TRACE_STACK_HISTORY)
      pos = 0;
  }
}

#define VT_MAX(a, b) ((a) > (b) ? (a) : (b))
#define VT_APRINTF(...) { written += scsprintf(buf + written, VT_MAX(size - written, 0), __VA_ARGS__); }

void VarTrace::printStack(SQChar * buf, int size)
{
  size--;
  int written = 0;
  *buf = 0;

  for (int h = 0; h < VAR_TRACE_STACK_HISTORY; h++)
  {
    int historyPos = (-h + pos + VAR_TRACE_STACK_HISTORY * 2 - 1) % VAR_TRACE_STACK_HISTORY;
    HistoryRecord & hist = history[historyPos];

    bool stackPresent = hist.stack[0].line != STACK_NOT_INITIALIZED;
    bool showCount = history[historyPos].count > 1 && stackPresent;

    if (size > written)
      VT_APRINTF(_SC("%d:"), h);

    if (showCount)
      VT_APRINTF(_SC(" x%d"), history[historyPos].count);


    if (stackPresent)
      VT_APRINTF((history[historyPos].flags & VT_FLAG_STRING) ? _SC(" value='%s%s'") : _SC(" value=%s%s"),
        history[historyPos].val,
        (history[historyPos].flags & VT_FLAG_ELLIPSIS) ? _SC("...") : _SC(""));

    VT_APRINTF(_SC("\n"));

    for (int i = 0; i < VAR_TRACE_STACK_DEPTH; i++)
    {
      if (hist.stack[i].line == STACK_NOT_INITIALIZED)
        break;

      if (size > written)
        VT_APRINTF(_SC("  %s:%d\n"), hist.stack[i].fileName, hist.stack[i].line);
    }

    VT_APRINTF(_SC("\n"));
  }

  VT_APRINTF(_SC("set counter = %d\n"), setCnt);
}

#endif
