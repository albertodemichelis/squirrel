#ifndef  _SQCOMPILATIONCONTEXT_H_
#define _SQCOMPILATIONCONTEXT_H_ 1

#include <string>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include "squirrel.h"
#include "sqvm.h"
#include "sqstate.h"
#include "squtils.h"
#include "arena.h"


#define DIAGNOSTICS \
  DEF_DIAGNOSTIC(LITERAL_OVERFLOW, ERROR, LEX, -1, "", "%s constant overflow"), \
  DEF_DIAGNOSTIC(LITERAL_UNDERFLOW, ERROR, LEX, -1, "", "%s constant underflow"), \
  DEF_DIAGNOSTIC(FP_EXP_EXPECTED, ERROR, LEX, -1, "", "exponent expected"), \
  DEF_DIAGNOSTIC(MALFORMED_NUMBER, ERROR, LEX, -1, "", "malformed number"), \
  DEF_DIAGNOSTIC(HEX_DIGITS_EXPECTED, ERROR, LEX, -1, "", "expected hex digits after '0x'"), \
  DEF_DIAGNOSTIC(HEX_TOO_MANY_DIGITS, ERROR, LEX, -1, "", "too many digits for an Hex number"), \
  DEF_DIAGNOSTIC(OCTAL_NOT_SUPPORTED, ERROR, LEX, -1, "", "leading 0 is not allowed, octal numbers are not supported"), \
  DEF_DIAGNOSTIC(TOO_LONG_LITERAL, ERROR, LEX, -1, "", "constant too long"), \
  DEF_DIAGNOSTIC(EMPTY_LITERAL, ERROR, LEX, -1, "", "empty constant"), \
  DEF_DIAGNOSTIC(UNRECOGNISED_ESCAPER, ERROR, LEX, -1, "", "unrecognised escaper char"), \
  DEF_DIAGNOSTIC(NEWLINE_IN_CONST, ERROR, LEX, -1, "", "newline in a constant"), \
  DEF_DIAGNOSTIC(UNFINISHED_STRING, ERROR, LEX, -1, "", "unfinished string"), \
  DEF_DIAGNOSTIC(HEX_NUMBERS_EXPECTED, ERROR, LEX, -1, "", "hexadecimal number expected"), \
  DEF_DIAGNOSTIC(UNEXPECTED_CHAR, ERROR, LEX, -1, "", "unexpected character '%s'"), \
  DEF_DIAGNOSTIC(INVALID_TOKEN, ERROR, LEX, -1, "", "invalid token '%s'"), \
  DEF_DIAGNOSTIC(LEX_ERROR_PARSE, ERROR, LEX, -1, "", "error parsing %s"), \
  DEF_DIAGNOSTIC(BRACE_ORDER, ERROR, LEX, -1, "", "in brace order"), \
  DEF_DIAGNOSTIC(NO_PARAMS_IN_STRING_TEMPLATE, ERROR, LEX, -1, "", "no params collected for string interpolation"), \
  DEF_DIAGNOSTIC(COMMENT_IN_STRING_TEMPLATE, ERROR, LEX, -1, "", "comments inside interpolated string are not supported"), \
  DEF_DIAGNOSTIC(EXPECTED_LEX, ERROR, LEX, -1, "", "expected %s"), \
  DEF_DIAGNOSTIC(MACRO_RECURSION, ERROR, LEX, -1, "", "recursion in reader macro"), \
  DEF_DIAGNOSTIC(INVALID_CHAR, ERROR, LEX, -1, "", "Invalid character"), \
  DEF_DIAGNOSTIC(TRAILING_BLOCK_COMMENT, ERROR, LEX, -1, "", "missing \"*/\" in comment"), \
  DEF_DIAGNOSTIC(GLOBAL_CONSTS_ONLY, ERROR, SYNTAX, -1, "", "global can be applied to const and enum only"), \
  DEF_DIAGNOSTIC(END_OF_STMT_EXPECTED, ERROR, SYNTAX, -1, "", "end of statement expected (; or lf)"), \
  DEF_DIAGNOSTIC(EXPECTED_TOKEN, ERROR, SYNTAX, -1, "", "expected '%s'"), \
  DEF_DIAGNOSTIC(UNSUPPORTED_DIRECTIVE, ERROR, SYNTAX, -1, "", "unsupported directive '%s'"), \
  DEF_DIAGNOSTIC(EXPECTED_LINENUM, ERROR, SYNTAX, -1, "", "expected line number after #pos:"), \
  DEF_DIAGNOSTIC(EXPECTED_COLNUM, ERROR, SYNTAX, -1, "", "expected column number after #pos:<line>:"), \
  DEF_DIAGNOSTIC(TOO_BIG_AST, ERROR, SYNTAX, -1, "", "AST too big. Consider simplifing it"), \
  DEF_DIAGNOSTIC(INCORRECT_INTRA_ASSIGN, ERROR, SYNTAX, -1, "", ": intra-expression assignment can be used only in 'if', 'for', 'while' or 'switch'"), \
  DEF_DIAGNOSTIC(ASSIGN_INSIDE_FORBIDEN, ERROR, SYNTAX, -1, "", "'=' inside '%s' is forbidden"), \
  DEF_DIAGNOSTIC(BROKEN_SLOT_DECLARATION, ERROR, SYNTAX, -1, "", "cannot break deref/or comma needed after [exp]=exp slot declaration"), \
  DEF_DIAGNOSTIC(ROOT_TABLE_FORBIDDEN, ERROR, SYNTAX, -1, "", "Access to root table is forbidden"), \
  DEF_DIAGNOSTIC(UNINITIALIZED_BINDING, ERROR, SEMA, -1, "", "Binding '%s' must be initialized"), \
  DEF_DIAGNOSTIC(SAME_FOREACH_KV_NAMES, ERROR, SEMA, -1, "", "foreach() key and value names are the same: '%s'"), \
  DEF_DIAGNOSTIC(SCALAR_EXPECTED, ERROR, SYNTAX, -1, "", "scalar expected : %s"), \
  DEF_DIAGNOSTIC(VARARG_WITH_DEFAULT_ARG, ERROR, SYNTAX, -1, "", "function with default parameters cannot have variable number of parameters"), \
  DEF_DIAGNOSTIC(LOOP_CONTROLER_NOT_IN_LOOP, ERROR, SEMA, -1, "", "'%s' has to be in a loop block"), \
  DEF_DIAGNOSTIC(ASSIGN_TO_EXPR, ERROR, SEMA, -1, "", "can't assign to expression"), \
  DEF_DIAGNOSTIC(BASE_NOT_MODIFIABLE, ERROR, SEMA, -1, "", "'base' cannot be modified"), \
  DEF_DIAGNOSTIC(ASSIGN_TO_BINDING, ERROR, SEMA, -1, "", "can't assign to binding '%s' (probably declaring using 'local' was intended, but 'let' was used)"), \
  DEF_DIAGNOSTIC(LOCAL_SLOT_CREATE, ERROR, SEMA, -1, "", "can't 'create' a local slot"), \
  DEF_DIAGNOSTIC(CANNOT_INC_DEC, ERROR, SEMA, -1, "", "can't '++' or '--' %s"), \
  DEF_DIAGNOSTIC(VARNAME_CONFLICTS, ERROR, SEMA, -1, "", "Variable name %s conflicts with function name"), \
  DEF_DIAGNOSTIC(INVALID_ENUM, ERROR, SEMA, -1, "", "invalid enum [no '%s' field in '%s']"), \
  DEF_DIAGNOSTIC(UNKNOWN_SYMBOL, ERROR, SEMA, -1, "", "Unknown variable [%s]"), \
  DEF_DIAGNOSTIC(CANNOT_EVAL_UNARY, ERROR, SEMA, -1, "", "cannot evaluate unary-op"), \
  DEF_DIAGNOSTIC(DUPLICATE_KEY, ERROR, SEMA, -1, "", "duplicate key '%s'"), \
  DEF_DIAGNOSTIC(INVALID_SLOT_INIT, ERROR, SEMA, -1, "", "Invalid slot initializer '%s' - no such variable/constant or incorrect expression"), \
  DEF_DIAGNOSTIC(CANNOT_DELETE, ERROR, SEMA, -1, "", "can't delete %s"), \
  DEF_DIAGNOSTIC(CONFLICTS_WITH, ERROR, SEMA, -1, "", "%s name '%s' conflicts with %s"), \
  DEF_DIAGNOSTIC(LOCAL_CLASS_SYNTAX, ERROR, SEMA, -1, "", "cannot create a local class with the syntax (class <local>)"), \
  DEF_DIAGNOSTIC(INVALID_CLASS_NAME, ERROR, SEMA, -1, "", "invalid class name or context"), \
  DEF_DIAGNOSTIC(INC_DEC_NOT_ASSIGNABLE, ERROR, SEMA, -1, "", "argument of inc/dec operation is not assiangable"), \
  DEF_DIAGNOSTIC(TOO_MANY_SYMBOLS, ERROR, SEMA, -1, "", "internal compiler error: too many %s"), \
  DEF_DIAGNOSTIC(AND_OR_PAREN, WARNING, SYNTAX, 202, "and-or-paren", "Priority of the '&&' operator is higher than that of the '||' operator. Perhaps parentheses are missing?"), \
  DEF_DIAGNOSTIC(BITWISE_BOOL_PAREN, WARNING, SYNTAX, 203, "bitwise-bool-paren", "Result of bitwise operation used in boolean expression. Perhaps parentheses are missing?") 

namespace SQCompilation {

enum DiagnosticsId {
#define DEF_DIAGNOSTIC(id, _, __, ___, _____, ______) DI_##id
  DIAGNOSTICS,
#undef DEF_DIAGNOSTIC
  DI_NUM_OF_DIAGNOSTICS
};

enum DiagnosticSeverity {
  DS_HINT,
  DS_WARNING,
  DS_ERROR
};

class SQCompilationContext
{
public:
  SQCompilationContext(SQVM *vm, Arena *a, const SQChar *sn, const char *code, size_t csize, bool raiseError);
  ~SQCompilationContext();

  jmp_buf &errorJump() { return _errorjmp; }

  void vreportDiagnostic(enum DiagnosticsId diag, int32_t line, int32_t pos, int32_t width, va_list args);
  void reportDiagnostic(enum DiagnosticsId diag, int32_t line, int32_t pos, int32_t width, ...);

  Arena *arena() const { return _arena; }

private:

  bool isDisabled(enum DiagnosticsId id, int line, int pos);

  void buildLineMap();

  const char *findLine(int lineNo);

  const SQChar *_sourceName;

  SQVM *_vm;

  const SQChar *_code;
  size_t _codeSize;

  jmp_buf _errorjmp;
  Arena *_arena;

  bool _raiseError;

  sqvector<const char *> _linemap;
};

} // namespace SQCompilation

#endif // ! _SQCOMPILATIONCONTEXT_H_
