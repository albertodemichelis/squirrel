/*	see copyright notice in squirrel.h */
#ifndef _SQCOMPILER_H_
#define _SQCOMPILER_H_

struct SQVM;

enum SQKeywordsEnum {
    TK_FIRST_ENUM_TOKEN = 258,
    /*
    the above token is only for internal purposes
    like calculate total enum_tokens = TK_LAST_ENUM_TOKEN - TK_FIRST_ENUM_TOKEN -1
    */
    TK_IDENTIFIER,
    TK_STRING_LITERAL,
    TK_INTEGER,
    TK_FLOAT,
    TK_BASE,
    TK_DELETE,
    TK_EQ,
    TK_NE,
    TK_LE,
    TK_GE,
    TK_SWITCH,
    TK_ARROW,
    TK_AND,
    TK_OR,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_BREAK,
    TK_FOR,
    TK_DO,
    TK_NULL,
    TK_FOREACH,
    TK_IN,
    TK_NEWSLOT,
    TK_MODULO,
    TK_LOCAL,
    TK_CLONE,
    TK_FUNCTION,
    TK_RETURN,
    TK_TYPEOF,
    TK_UMINUS,
    TK_PLUSEQ,
    TK_MINUSEQ,
    TK_CONTINUE,
    TK_YIELD,
    TK_TRY,
    TK_CATCH,
    TK_THROW,
    TK_SHIFTL,
    TK_SHIFTR,
    TK_RESUME,
    TK_DOUBLE_COLON,
    TK_CASE,
    TK_DEFAULT,
    TK_THIS,
    TK_PLUSPLUS,
    TK_MINUSMINUS,
    TK_3WAYSCMP,
    TK_USHIFTR,
    TK_CLASS,
    TK_EXTENDS,
    TK_CONSTRUCTOR,
    TK_INSTANCEOF,
    TK_VARPARAMS,
    TK___LINE__,
    TK___FILE__,
    TK_TRUE,
    TK_FALSE,
    TK_MULEQ,
    TK_DIVEQ,
    TK_MODEQ,
    TK_ATTR_OPEN,
    TK_ATTR_CLOSE,
    TK_STATIC,
    TK_ENUM,
    TK_CONST,
    /*
    the next token is only for internal purposes
    like calculate total enum_tokens = TK_LAST_ENUM_TOKEN - TK_FIRST_ENUM_TOKEN -1
    */
    TK_LAST_ENUM_TOKEN
};

typedef void(*CompilerErrorFunc)(void *ud, const SQChar *s);
bool Compile(SQVM *vm, SQLEXREADFUNC rg, SQUserPointer up, const SQChar *sourcename, SQObjectPtr &out,
             bool raiseerror, bool lineinfo, bool show_warnings=false);
#endif //_SQCOMPILER_H_
