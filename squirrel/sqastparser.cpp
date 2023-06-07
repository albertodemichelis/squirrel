#include "sqpcheader.h"
#ifndef NO_COMPILER
#include "sqopcodes.h"
#include "sqstring.h"
#include "sqfuncproto.h"
#include "sqcompiler.h"
#include "sqastparser.h"
#include "sqcompiler.h"
#include <stdarg.h>


SQParser::SQParser(SQVM *v, SQLEXREADFUNC rg, SQUserPointer up, const SQChar* sourcename, Arena *astArena, bool raiseerror, bool lineinfo)
    : _lex(_ss(v))
    , _astArena(astArena)
{
    _vm=v;
    _lex.Init(_ss(v), rg, up,ThrowError,this);
    _sourcename = sourcename;
    _raiseerror = raiseerror;
    _compilererror[0] = _SC('\0');
    _expression_context = SQE_REGULAR;
    _lang_features = _ss(v)->defaultLangFeatures;
}


void SQParser::Error(const SQChar *s, ...)
{
    va_list vl;
    va_start(vl, s);
    scvsprintf(_compilererror, MAX_COMPILER_ERROR_LEN, s, vl);
    va_end(vl);
    longjmp(_errorjmp,1);
}


void SQParser::ProcessDirective()
{
    const SQChar *sval = _lex._svalue;

    if (scstrncmp(sval, _SC("pos:"), 4) == 0) {
        sval += 4;
        if (!scisdigit(*sval))
            Error(_SC("expected line number after #pos:"));
        SQChar * next = NULL;
        _lex._currentline = scstrtol(sval, &next, 10);
        if (!next || *next != ':')
            Error(_SC("expected ':'"));
        next++;
        if (!scisdigit(*next))
            Error(_SC("expected column number after #pos:<line>:"));
        _lex._currentcolumn = scstrtol(next, NULL, 10);

        return;
    }

    SQInteger setFlags = 0, clearFlags = 0;
    bool applyToDefault = false;
    if (scstrncmp(sval, _SC("default:"), 8) == 0) {
        applyToDefault = true;
        sval += 8;
    }

    if (scstrcmp(sval, _SC("strict")) == 0)
        setFlags = LF_STRICT;
    else if (scstrcmp(sval, _SC("relaxed")) == 0)
        clearFlags = LF_STRICT;
    else if (scstrcmp(sval, _SC("strict-bool")) == 0)
        setFlags = LF_STRICT_BOOL;
    else if (scstrcmp(sval, _SC("relaxed-bool")) == 0)
        clearFlags = LF_STRICT_BOOL;
    else if (scstrcmp(sval, _SC("no-root-fallback")) == 0)
        setFlags = LF_EXPLICIT_ROOT_LOOKUP;
    else if (scstrcmp(sval, _SC("implicit-root-fallback")) == 0)
        clearFlags = LF_EXPLICIT_ROOT_LOOKUP;
    else if (scstrcmp(sval, _SC("no-func-decl-sugar")) == 0)
        setFlags = LF_NO_FUNC_DECL_SUGAR;
    else if (scstrcmp(sval, _SC("allow-func-decl-sugar")) == 0)
        clearFlags = LF_NO_FUNC_DECL_SUGAR;
    else if (scstrcmp(sval, _SC("no-class-decl-sugar")) == 0)
        setFlags = LF_NO_CLASS_DECL_SUGAR;
    else if (scstrcmp(sval, _SC("allow-class-decl-sugar")) == 0)
        clearFlags = LF_NO_CLASS_DECL_SUGAR;
    else if (scstrcmp(sval, _SC("no-plus-concat")) == 0)
        setFlags = LF_NO_PLUS_CONCAT;
    else if (scstrcmp(sval, _SC("allow-plus-concat")) == 0)
        clearFlags = LF_NO_PLUS_CONCAT;
    else if (scstrcmp(sval, _SC("explicit-this")) == 0)
        setFlags = LF_EXPLICIT_THIS;
    else if (scstrcmp(sval, _SC("implicit-this")) == 0)
        clearFlags = LF_EXPLICIT_THIS;
    else if (scstrcmp(sval, _SC("forbid-root-table")) == 0)
        setFlags = LF_FORBID_ROOT_TABLE;
    else if (scstrcmp(sval, _SC("allow-root-table")) == 0)
        clearFlags = LF_FORBID_ROOT_TABLE;
    else if (scstrcmp(sval, _SC("disable-optimizer")) == 0)
        setFlags = LF_DISABLE_OPTIMIZER;
    else if (scstrcmp(sval, _SC("enable-optimizer")) == 0)
        clearFlags = LF_DISABLE_OPTIMIZER;
    else
        Error(_SC("unsupported directive"));

    _lang_features = (_lang_features | setFlags) & ~clearFlags;
    if (applyToDefault)
        _ss(_vm)->defaultLangFeatures = (_ss(_vm)->defaultLangFeatures | setFlags) & ~clearFlags;
}


void SQParser::Lex()
{
    _token = _lex.Lex();
    while (_token == TK_DIRECTIVE)
    {
        bool endOfLine = (_lex._prevtoken == _SC('\n'));
        ProcessDirective();
        _token = _lex.Lex();
        if (endOfLine)
            _lex._prevtoken = _SC('\n');
    }
}


Expr* SQParser::Expect(SQInteger tok)
{
    if(_token != tok) {
        if(_token == TK_CONSTRUCTOR && tok == TK_IDENTIFIER) {
            //do nothing
        }
        else {
            const SQChar *etypename;
            if(tok > 255) {
                switch(tok)
                {
                case TK_IDENTIFIER:
                    etypename = _SC("IDENTIFIER");
                    break;
                case TK_STRING_LITERAL:
                    etypename = _SC("STRING_LITERAL");
                    break;
                case TK_INTEGER:
                    etypename = _SC("INTEGER");
                    break;
                case TK_FLOAT:
                    etypename = _SC("FLOAT");
                    break;
                default:
                    etypename = _lex.Tok2Str(tok);
                }
                Error(_SC("expected '%s'"), etypename);
            }
            Error(_SC("expected '%c'"), tok);
        }
    }
    Expr *ret = NULL;
    switch(tok)
    {
    case TK_IDENTIFIER:
        ret = newId(_lex._svalue);
        break;
    case TK_STRING_LITERAL:
        ret = newStringLiteral(_lex._svalue);
        break;
    case TK_INTEGER:
        ret = newNode<LiteralExpr>(_lex._nvalue);
        ret->setLinePos(_lex._currentline); ret->setColumnPos(_lex._currentcolumn);
        break;
    case TK_FLOAT:
        ret = newNode<LiteralExpr>(_lex._fvalue);
        ret->setLinePos(_lex._currentline); ret->setColumnPos(_lex._currentcolumn);
        break;
    }
    Lex();
    return ret;
}


void SQParser::OptionalSemicolon()
{
    if(_token == _SC(';')) { Lex(); return; }
    if(!IsEndOfStatement()) {
        Error(_SC("end of statement expected (; or lf)"));
    }
}


RootBlock* SQParser::parse()
{
    RootBlock *rootBlock = newNode<RootBlock>(arena());

    if(setjmp(_errorjmp) == 0) {
        Lex();
        rootBlock->setStartLine(_lex._currentline);
        while(_token > 0){
            rootBlock->addStatement(parseStatement());
            if(_lex._prevtoken != _SC('}') && _lex._prevtoken != _SC(';')) OptionalSemicolon();
        }
        rootBlock->setEndLine(_lex._currentline);
    }
    else {
        if(_raiseerror && _ss(_vm)->_compilererrorhandler) {
            _ss(_vm)->_compilererrorhandler(_vm, _compilererror, _sourcename ? _sourcename : _SC("unknown"),
                _lex._currentline, _lex._currentcolumn);
        }
        _vm->_lasterror = SQString::Create(_ss(_vm), _compilererror, -1);
        return NULL;
    }

    return rootBlock;
}


Block* SQParser::parseStatements()
{
    Block *result = newNode<Block>(arena());
    while(_token != _SC('}') && _token != TK_DEFAULT && _token != TK_CASE) {
        Statement *stmt = parseStatement();
        result->addStatement(stmt);
        if(_lex._prevtoken != _SC('}') && _lex._prevtoken != _SC(';')) OptionalSemicolon();
    }
    return result;
}


Statement* SQParser::parseStatement(bool closeframe)
{
    Statement *result = NULL;
    SQInteger l = _lex._currentline, c = _lex._currentcolumn;

    switch(_token) {
        case _SC(';'):  result = newNode<EmptyStatement>(); Lex(); break;
    case TK_IF:     result = parseIfStatement();          break;
    case TK_WHILE:  result = parseWhileStatement();       break;
    case TK_DO:     result = parseDoWhileStatement();     break;
    case TK_FOR:    result = parseForStatement();         break;
    case TK_FOREACH:result = parseForEachStatement();     break;
    case TK_SWITCH: result = parseSwitchStatement();      break;
    case TK_LOCAL:
    case TK_LET:
        result = parseLocalDeclStatement(_token == TK_LOCAL);
        break;
    case TK_RETURN:
    case TK_YIELD: {
        SQOpcode op;
        if(_token == TK_RETURN) {
            op = _OP_RETURN;
        }
        else {
            op = _OP_YIELD;
        }
        Lex();

        Expr *arg = NULL;
        if(!IsEndOfStatement()) {
            arg = Expression(SQE_RVALUE);
        }

        result = op == _OP_RETURN ? static_cast<TerminateStatement *>(newNode<ReturnStatement>(arg)) : newNode<YieldStatement>(arg);
        break;
    }
    case TK_BREAK:
        result = newNode<BreakStatement>(nullptr);
        Lex();
        break;
    case TK_CONTINUE:
        result = newNode<ContinueStatement>(nullptr);
        Lex();
        break;
    case TK_FUNCTION:
        if (!(_lang_features & LF_NO_FUNC_DECL_SUGAR))
            result = parseFunctionStatement();
        else
            Error(_SC("Syntactic sugar for declaring functions as fields is disabled"));
        break;
    case TK_CLASS:
        if (!(_lang_features & LF_NO_CLASS_DECL_SUGAR))
            result = parseClassStatement();
        else
            Error(_SC("Syntactic sugar for declaring classes as fields is disabled"));
        break;
    case TK_ENUM:
        result = parseEnumStatement(false);
        break;
    case _SC('{'):
        Lex();
        result = parseStatements();
        Expect(_SC('}'));
        break;
    case TK_TRY:
        result = parseTryCatchStatement();
        break;
    case TK_THROW:
        Lex();
        result = newNode<ThrowStatement>(Expression(SQE_RVALUE));
        break;
    case TK_CONST:
        result = parseConstStatement(false);
        break;
    case TK_GLOBAL:
        Lex();
        if (_token == TK_CONST)
            result = parseConstStatement(true);
        else if (_token == TK_ENUM)
            result = parseEnumStatement(true);
        else
            Error(_SC("global can be applied to const and enum only"));
        break;
    default:
        result = newNode<ExprStatement>(Expression(SQE_REGULAR));
        break;
    }
    result->setColumnPos(c);
    result->setLinePos(l);
    assert(result);
    return result;
}


Expr* SQParser::parseCommaExpr(SQExpressionContext expression_context)
{
    SQInteger l = _lex._currentline;
    SQInteger c = _lex._currentcolumn;
    Expr *expr = Expression(expression_context);

    if (_token == ',') {
        CommaExpr *cm = newNode<CommaExpr>(arena());
        cm->setColumnPos(c);
        cm->setLinePos(l);
        cm->addExpression(expr);
        expr = cm;
        while (_token == ',') {
            Lex();
            cm->addExpression(Expression(expression_context));
        }
    }

    return expr;
}


Expr* SQParser::Expression(SQExpressionContext expression_context)
{
    SQExpressionContext saved_expression_context = _expression_context;
    _expression_context = expression_context;

    SQInteger line = _lex._prevtoken == _SC('\n') ? _lex._lasttokenline : _lex._currentline;
    SQInteger c = _lex._currentcolumn;

    Expr *expr = LogicalNullCoalesceExp();

    if (_token == TK_INEXPR_ASSIGNMENT && (expression_context == SQE_REGULAR || expression_context == SQE_FUNCTION_ARG))
        Error(_SC(": intra-expression assignment can be used only in 'if', 'for', 'while' or 'switch'"));

    switch(_token)  {
    case _SC('='):
    case TK_INEXPR_ASSIGNMENT:
    case TK_NEWSLOT:
    case TK_MINUSEQ:
    case TK_PLUSEQ:
    case TK_MULEQ:
    case TK_DIVEQ:
    case TK_MODEQ: {
        SQInteger op = _token;
        Lex();
        Expr *e2 = Expression(SQE_RVALUE);

        switch (op) {
        case TK_NEWSLOT: expr = newNode<BinExpr>(TO_NEWSLOT, expr, e2); break;
        case TK_INEXPR_ASSIGNMENT:
        case _SC('='): //ASSIGN
            if (op == _SC('='))
                switch (expression_context)
                {
                case SQE_IF:
                    Error(_SC("'=' inside 'if' is forbidden"));
                    break;
                case SQE_LOOP_CONDITION:
                    Error(_SC("'=' inside loop condition is forbidden"));
                    break;
                case SQE_SWITCH:
                    Error(_SC("'=' inside switch is forbidden"));
                    break;
                case SQE_FUNCTION_ARG:
                    Error(_SC("'=' inside function argument is forbidden"));
                    break;
                case SQE_RVALUE:
                    Error(_SC("'=' inside expression is forbidden"));
                    break;
                case SQE_REGULAR:
                    break;
                }
            expr = newNode<BinExpr>(op == TK_INEXPR_ASSIGNMENT ? TO_INEXPR_ASSIGN : TO_ASSIGN, expr, e2);
            break;
        case TK_MINUSEQ: expr = newNode<BinExpr>(TO_MINUSEQ, expr, e2); break;
        case TK_PLUSEQ: expr = newNode<BinExpr>(TO_PLUSEQ, expr, e2); break;
        case TK_MULEQ: expr = newNode<BinExpr>(TO_MULEQ, expr, e2); break;
        case TK_DIVEQ: expr = newNode<BinExpr>(TO_DIVEQ, expr, e2); break;
        case TK_MODEQ: expr = newNode<BinExpr>(TO_MODEQ, expr, e2); break;
        }
    }
    break;
    case _SC('?'): {
        Consume('?');

        Expr *ifTrue = Expression(SQE_RVALUE);

        Expect(_SC(':'));

        Expr *ifFalse = Expression(SQE_RVALUE);

        expr = newNode<TerExpr>(expr, ifTrue, ifFalse);
    }
    break;
    }

    _expression_context = saved_expression_context;
    expr->setLinePos(line);
    expr->setColumnPos(c);
    return expr;
}


template<typename T> Expr* SQParser::BIN_EXP(T f, enum TreeOp top, Expr *lhs)
{
    _expression_context = SQE_RVALUE;

    Lex();

    Expr *rhs = (this->*f)();

    return newNode<BinExpr>(top, lhs, rhs);
}


Expr* SQParser::LogicalNullCoalesceExp()
{
    Expr *lhs = LogicalOrExp();
    for (;;) {
        if (_token == TK_NULLCOALESCE) {
            Lex();

            Expr *rhs = LogicalNullCoalesceExp();
            lhs = newNode<BinExpr>(TO_NULLC, lhs, rhs);
        }
        else return lhs;
    }
}


Expr* SQParser::LogicalOrExp()
{
    Expr *lhs = LogicalAndExp();
    for (;;) {
        if (_token == TK_OR) {
            Lex();

            Expr *rhs = LogicalOrExp();
            lhs = newNode<BinExpr>(TO_OROR, lhs, rhs);
        }
        else return lhs;
    }
}

Expr* SQParser::LogicalAndExp()
{
    Expr *lhs = BitwiseOrExp();
    for (;;) {
        switch (_token) {
        case TK_AND: {
            Lex();

            Expr *rhs = LogicalAndExp();
            lhs = newNode<BinExpr>(TO_ANDAND, lhs, rhs);
        }
        default:
            return lhs;
        }
    }
}

Expr* SQParser::BitwiseOrExp()
{
    Expr *lhs = BitwiseXorExp();
    for (;;) {
        if (_token == _SC('|')) {
            return BIN_EXP(&SQParser::BitwiseOrExp, TO_OR, lhs);
        }
        else return lhs;
    }
}

Expr* SQParser::BitwiseXorExp()
{
    Expr * lhs = BitwiseAndExp();
    for (;;) {
        if (_token == _SC('^')) {
            lhs = BIN_EXP(&SQParser::BitwiseAndExp, TO_XOR, lhs);
        }
        else return lhs;
    }
}

Expr* SQParser::BitwiseAndExp()
{
    Expr *lhs = EqExp();
    for (;;) {
        if (_token == _SC('&')) {
            lhs = BIN_EXP(&SQParser::EqExp, TO_AND, lhs);
        }
        else return lhs;
    }
}

Expr* SQParser::EqExp()
{
    Expr *lhs = CompExp();
    for (;;) {
        switch (_token) {
        case TK_EQ: lhs = BIN_EXP(&SQParser::CompExp, TO_EQ, lhs); break;
        case TK_NE: lhs = BIN_EXP(&SQParser::CompExp, TO_NE, lhs); break;
        case TK_3WAYSCMP: lhs = BIN_EXP(&SQParser::CompExp, TO_3CMP, lhs); break;
        default: return lhs;
        }
    }
}

Expr* SQParser::CompExp()
{
    Expr *lhs = ShiftExp();
    for (;;) {
        switch (_token) {
        case _SC('>'): lhs = BIN_EXP(&SQParser::ShiftExp, TO_GT, lhs); break;
        case _SC('<'): lhs = BIN_EXP(&SQParser::ShiftExp, TO_LT, lhs); break;
        case TK_GE: lhs = BIN_EXP(&SQParser::ShiftExp, TO_GE, lhs); break;
        case TK_LE: lhs = BIN_EXP(&SQParser::ShiftExp, TO_LE, lhs); break;
        case TK_IN: lhs = BIN_EXP(&SQParser::ShiftExp, TO_IN, lhs); break;
        case TK_INSTANCEOF: lhs = BIN_EXP(&SQParser::ShiftExp, TO_INSTANCEOF, lhs); break;
        case TK_NOT: {
            SQInteger l = _lex._currentline, c = _lex._currentcolumn;
            Lex();
            if (_token == TK_IN) {
                lhs = BIN_EXP(&SQParser::ShiftExp, TO_IN, lhs);
                lhs = newNode<UnExpr>(TO_NOT, lhs);
                lhs->setColumnPos(c);
                lhs->setLinePos(l);
            }
            else
                Error(_SC("'in' expected "));
        }
        default: return lhs;
        }
    }
}

Expr* SQParser::ShiftExp()
{
    Expr *lhs = PlusExp();
    for (;;) {
        switch (_token) {
        case TK_USHIFTR: lhs = BIN_EXP(&SQParser::PlusExp, TO_USHR, lhs); break;
        case TK_SHIFTL: lhs = BIN_EXP(&SQParser::PlusExp, TO_SHL, lhs); break;
        case TK_SHIFTR: lhs = BIN_EXP(&SQParser::PlusExp, TO_SHR, lhs); break;
        default: return lhs;
        }
    }
}

Expr* SQParser::PlusExp()
{
    Expr *lhs = MultExp();
    for (;;) {
        switch (_token) {
        case _SC('+'): lhs = BIN_EXP(&SQParser::MultExp, TO_ADD, lhs); break;
        case _SC('-'): lhs = BIN_EXP(&SQParser::MultExp, TO_SUB, lhs); break;

        default: return lhs;
        }
    }
}

Expr* SQParser::MultExp()
{
    Expr *lhs = PrefixedExpr();
    for (;;) {
        switch (_token) {
        case _SC('*'): lhs = BIN_EXP(&SQParser::PrefixedExpr, TO_MUL, lhs); break;
        case _SC('/'): lhs = BIN_EXP(&SQParser::PrefixedExpr, TO_DIV, lhs); break;
        case _SC('%'): lhs = BIN_EXP(&SQParser::PrefixedExpr, TO_MOD, lhs); break;

        default: return lhs;
        }
    }
}

Expr* SQParser::PrefixedExpr()
{
    //if 'pos' != -1 the previous variable is a local variable
    SQInteger pos;
    Expr *e = Factor(pos);
    bool nextIsNullable = false;
    for(;;) {
        switch(_token) {
        case _SC('.'):
        case TK_NULLGETSTR: {
            if (_token == TK_NULLGETSTR || nextIsNullable)
            {
                nextIsNullable = true;
            }
            Lex();

            Id *id = (Id *)Expect(TK_IDENTIFIER);
            assert(id);
            e = newNode<GetFieldExpr>(e, id->id(), nextIsNullable); //-V522
            break;
        }
        case _SC('['):
        case TK_NULLGETOBJ: {
            if (_token == TK_NULLGETOBJ || nextIsNullable)
            {
                nextIsNullable = true;
            }
            if(_lex._prevtoken == _SC('\n')) Error(_SC("cannot break deref/or comma needed after [exp]=exp slot declaration"));
            Lex();
            Expr *key = Expression(SQE_RVALUE);
            Expect(_SC(']'));
            e = newNode<GetTableExpr>(e, key, nextIsNullable);
            break;
        }
        case TK_MINUSMINUS:
        case TK_PLUSPLUS:
            {
                nextIsNullable = false;
                if(IsEndOfStatement()) return e;
                SQInteger diff = (_token==TK_MINUSMINUS) ? -1 : 1;
                Lex();
                e = newNode<IncExpr>(e, diff, IF_POSTFIX);
            }
            return e;
        case _SC('('):
        case TK_NULLCALL: {
            SQInteger nullcall = (_token==TK_NULLCALL || nextIsNullable);
            nextIsNullable = !!nullcall;
            CallExpr *call = newNode<CallExpr>(arena(), e, nullcall);
            Lex();
            while (_token != _SC(')')) {
                call->addArgument(Expression(SQE_FUNCTION_ARG));
                if (_token == _SC(',')) {
                    Lex();
                }
            }
            Lex();
            e = call;
            break;
        }
        default: return e;
        }
    }
}

Expr* SQParser::Factor(SQInteger &pos)
{
    Expr *r = NULL;

    SQInteger l = _lex._currentline, c = _lex._currentcolumn;

    switch(_token)
    {
    case TK_STRING_LITERAL:
        r = newStringLiteral(_lex._svalue);
        Lex();
        break;
    case TK_BASE:
        Lex();
        r = newNode<BaseExpr>();
        break;
    case TK_IDENTIFIER: r = newId(_lex._svalue); Lex(); break;
    case TK_CONSTRUCTOR: r = newNode<Id>(_SC("constructor")); Lex(); break;
    case TK_THIS: r = newNode<Id>(_SC("this")); Lex(); break;
    case TK_DOUBLE_COLON:  // "::"
        if (_lang_features & LF_FORBID_ROOT_TABLE)
            Error(_SC("Access to root table is forbidden"));
        _token = _SC('.'); /* hack: drop into PrefixExpr, case '.'*/
        r = newNode<RootExpr>();
        break;
    case TK_NULL:
        r = newNode<LiteralExpr>();
        Lex();
        break;
    case TK_INTEGER:
        r = newNode<LiteralExpr>(_lex._nvalue);
        Lex();
        break;
    case TK_FLOAT:
        r = newNode<LiteralExpr>(_lex._fvalue);
        Lex();
        break;
    case TK_TRUE: case TK_FALSE:
        r = newNode<LiteralExpr>((bool)(_token == TK_TRUE));
        Lex();
        break;
    case _SC('['): {
            Lex();
            ArrayExpr *arr = newNode<ArrayExpr>(arena());
            while(_token != _SC(']')) {
                SQInteger line = _lex._currentline;
                Expr *v = Expression(SQE_RVALUE);
                v->setLinePos(line);
                arr->addValue(v);
                if(_token == _SC(',')) Lex();
            }
            Lex();
            r = arr;
        }
        break;
    case _SC('{'): {
        Lex();
        TableDecl *t = newNode<TableDecl>(arena());
        ParseTableOrClass(t, _SC(','), _SC('}'));
        r = newNode<DeclExpr>(t);
        break;
    }
    case TK_FUNCTION:
        r = FunctionExp(_token);
        break;
    case _SC('@'):
        r = FunctionExp(_token,true);
        break;
    case TK_CLASS: {
        Lex();
        Decl *classDecl = ClassExp(NULL);
        classDecl->setContext(DC_EXPR);
        r = newNode<DeclExpr>(classDecl);
        break;
    }
    case _SC('-'):
        Lex();
        switch(_token) {
        case TK_INTEGER:
            r = newNode<LiteralExpr>(-_lex._nvalue);
            Lex();
            break;
        case TK_FLOAT:
            r = newNode<LiteralExpr>(-_lex._fvalue);
            Lex();
            break;
        default:
            r = UnaryOP(TO_NEG);
            break;
        }
        break;
    case _SC('!'):
        Lex();
        r = UnaryOP(TO_NOT);
        break;
    case _SC('~'):
        Lex();
        if(_token == TK_INTEGER)  {
            r = newNode<LiteralExpr>(~_lex._nvalue);
            Lex();
        }
        else {
            r = UnaryOP(TO_BNOT);
        }
        break;
    case TK_TYPEOF : Lex(); r = UnaryOP(TO_TYPEOF); break;
    case TK_RESUME : Lex(); r = UnaryOP(TO_RESUME); break;
    case TK_CLONE : Lex(); r = UnaryOP(TO_CLONE); break;
    case TK_MINUSMINUS :
    case TK_PLUSPLUS :
        r = PrefixIncDec(_token);
        break;
    case TK_DELETE : r = DeleteExpr(); break;
    case _SC('('):
        Lex();
        r = newNode<UnExpr>(TO_PAREN, Expression(_expression_context));
        Expect(_SC(')'));
        break;
    case TK___LINE__:
        r = newNode<LiteralExpr>(_lex._currentline);
        Lex();
        break;
    case TK___FILE__:
        r = newNode<LiteralExpr>(_sourcename);
        Lex();
        break;
    default: Error(_SC("expression expected"));
    }
    r->setColumnPos(c);
    r->setLinePos(l);
    return r;
}

Expr* SQParser::UnaryOP(enum TreeOp op)
{
    Expr *arg = PrefixedExpr();
    return newNode<UnExpr>(op, arg);
}

void SQParser::ParseTableOrClass(TableDecl *decl, SQInteger separator, SQInteger terminator)
{
    NewObjectType otype = separator==_SC(',') ? NOT_TABLE : NOT_CLASS;

    while(_token != terminator) {
        SQInteger line = _lex._currentline;
        SQInteger c = _lex._currentcolumn;
        bool isstatic = false;
        //check if is an static
        if(otype == NOT_CLASS) {
            if(_token == TK_STATIC) {
                isstatic = true;
                Lex();
            }
        }

        switch (_token) {
        case TK_FUNCTION:
        case TK_CONSTRUCTOR: {
            SQInteger tk = _token;
            Lex();
            Id *funcName = tk == TK_FUNCTION ? (Id *)Expect(TK_IDENTIFIER) : newNode<Id>(_SC("constructor"));
            assert(funcName);
            LiteralExpr *key = newNode<LiteralExpr>(funcName->id()); //-V522
            key->setLinePos(line);
            key->setColumnPos(c);
            Expect(_SC('('));
            FunctionDecl *f = CreateFunction(funcName, false, tk == TK_CONSTRUCTOR);
            decl->addMember(key, f, isstatic);
        }
        break;
        case _SC('['): {
            Lex();

            Expr *key = Expression(SQE_RVALUE); //-V522
            assert(key);
            key->setLinePos(line);
            key->setColumnPos(c);
            Expect(_SC(']'));
            Expect(_SC('='));
            Expr *value = Expression(SQE_RVALUE);
            decl->addMember(key, value, isstatic);
            break;
        }
        case TK_STRING_LITERAL: //JSON
            if (otype == NOT_TABLE) { //only works for tables
                LiteralExpr *key = (LiteralExpr *)Expect(TK_STRING_LITERAL);  //-V522
                assert(key);
                key->setLinePos(line); //-V522
                key->setColumnPos(c); //-V522
                Expect(_SC(':'));
                Expr *expr = Expression(SQE_RVALUE);
                decl->addMember(key, expr, isstatic);
                break;
            }  //-V796
        default: {
            Id *id = (Id *)Expect(TK_IDENTIFIER);
            assert(id);
            LiteralExpr *key = newNode<LiteralExpr>(id->id()); //-V522
            key->setLinePos(line);
            key->setColumnPos(c);
            if ((otype == NOT_TABLE) &&
                (_token == TK_IDENTIFIER || _token == separator || _token == terminator || _token == _SC('[')
                    || _token == TK_FUNCTION)) {
                decl->addMember(key, id, isstatic);
            }
            else {
                Expect(_SC('='));
                Expr *expr = Expression(SQE_RVALUE);
                decl->addMember(key, expr, isstatic);
            }
        }
        }
        if (_token == separator) Lex(); //optional comma/semicolon
    }

    Lex();
}


Decl* SQParser::parseLocalDeclStatement(bool assignable)
{
    Lex();
    SQInteger l = _lex._currentline, c = _lex._currentcolumn;
    if (_token == TK_FUNCTION) {
        Lex();
        Id *varname = (Id *)Expect(TK_IDENTIFIER);
        Expect(_SC('('));
        FunctionDecl *f = CreateFunction(varname, false);
        f->setContext(DC_LOCAL);
        VarDecl *d = newNode<VarDecl>(varname->id(), newNode<DeclExpr>(f), assignable);
        d->setColumnPos(c); d->setLinePos(l);
        return d;
    } else if (_token == TK_CLASS) {
        Lex();
        Id *varname = (Id *)Expect(TK_IDENTIFIER);
        ClassDecl *cls = ClassExp(NULL);
        cls->setContext(DC_LOCAL);
        VarDecl *d = newNode<VarDecl>(varname->id(), newNode<DeclExpr>(cls), assignable);
        d->setColumnPos(c); d->setLinePos(l);
        return d;
    }

    DeclGroup *decls = NULL;
    DestructuringDecl  *dd = NULL;
    Decl *decl = NULL;
    SQInteger destructurer = 0;

    if (_token == _SC('{') || _token == _SC('[')) {
        destructurer = _token;
        Lex();
        decls = dd = newNode<DestructuringDecl>(arena(), destructurer == _SC('{') ? DT_TABLE : DT_ARRAY);
        dd->setColumnPos(c); dd->setLinePos(l);
    }

    do {
        l = _lex._currentline;
        c = _lex._currentcolumn;
        Id *varname = (Id *)Expect(TK_IDENTIFIER);
        assert(varname);
        VarDecl *cur = NULL;
        if(_token == _SC('=')) {
            Lex();
            Expr *expr = Expression(SQE_REGULAR);
            cur = newNode<VarDecl>(varname->id(), expr, assignable);
        }
        else {
            if (!assignable && !destructurer)
                Error(_SC("Binding '%s' must be initialized"), varname->id()); //-V522
            cur = newNode<VarDecl>(varname->id(), nullptr, assignable);
        }

        cur->setColumnPos(c); cur->setLinePos(l);

        if (decls) {
            decls->addDeclaration(cur);
        } else if (decl) {
            decls = newNode<DeclGroup>(arena());
            decls->addDeclaration(static_cast<VarDecl *>(decl));
            decls->addDeclaration(cur);
            decl = decls;
        } else {
            decl = cur;
        }

        if (destructurer) {
            if (_token == _SC(',')) {
                Lex();
                if (_token == _SC(']') || _token == _SC('}'))
                    break;
            }
            else if (_token == TK_IDENTIFIER)
                continue;
            else
                break;
        }
        else {
            if (_token == _SC(','))
                Lex();
            else
                break;
        }
    } while(1);

    if (destructurer) {
        Expect(destructurer==_SC('[') ? _SC(']') : _SC('}'));
        Expect(_SC('='));
        assert(dd);
        dd->setExpression(Expression(SQE_RVALUE)); //-V522
        return dd;
    } else {
        return decls ? static_cast<Decl*>(decls) : decl;
    }
}

Statement* SQParser::IfBlock()
{
    Statement *stmt = NULL;
    if (_token == _SC('{'))
    {
        Lex();
        stmt = parseStatements();
        Expect(_SC('}'));
    }
    else {
        stmt = parseStatement();
        Block *block = newNode<Block>(arena());
        block->addStatement(stmt);
        stmt = block;
        if (_lex._prevtoken != _SC('}') && _lex._prevtoken != _SC(';')) OptionalSemicolon();
    }

    return stmt;
}

IfStatement* SQParser::parseIfStatement()
{
    Consume(TK_IF);

    Expect(_SC('('));
    Expr *cond = Expression(SQE_IF);
    Expect(_SC(')'));

    Statement *thenB = IfBlock();
    Statement *elseB = NULL;
    if(_token == TK_ELSE){
        Lex();
        elseB = IfBlock();
    }

    return newNode<IfStatement>(cond, thenB, elseB);
}

WhileStatement* SQParser::parseWhileStatement()
{
    Consume(TK_WHILE);

    Expect(_SC('('));
    Expr *cond = Expression(SQE_LOOP_CONDITION);
    Expect(_SC(')'));

    Statement *body = parseStatement();

    return newNode<WhileStatement>(cond, body);
}

DoWhileStatement* SQParser::parseDoWhileStatement()
{
    Consume(TK_DO); // DO

    Statement *body = parseStatement();

    Expect(TK_WHILE);

    Expect(_SC('('));
    Expr *cond = Expression(SQE_LOOP_CONDITION);
    Expect(_SC(')'));

    return newNode<DoWhileStatement>(body, cond);
}

ForStatement* SQParser::parseForStatement()
{
    Consume(TK_FOR);

    Expect(_SC('('));

    Node *init = NULL;
    if (_token == TK_LOCAL) init = parseLocalDeclStatement(true);
    else if (_token != _SC(';')) {
        init = parseCommaExpr(SQE_REGULAR);
    }
    Expect(_SC(';'));

    Expr *cond = NULL;
    if(_token != _SC(';')) {
        cond = Expression(SQE_LOOP_CONDITION);
    }
    Expect(_SC(';'));

    Expr *mod = NULL;
    if(_token != _SC(')')) {
        mod = parseCommaExpr(SQE_REGULAR);
    }
    Expect(_SC(')'));

    Statement *body = parseStatement();

    return newNode<ForStatement>(init, cond, mod, body);
}

ForeachStatement* SQParser::parseForEachStatement()
{
    Consume(TK_FOREACH);

    Expect(_SC('('));
    Id *valname = (Id *)Expect(TK_IDENTIFIER);
    assert(valname);

    Id *idxname = NULL;
    if(_token == _SC(',')) {
        idxname = valname;
        Lex();
        valname = (Id *)Expect(TK_IDENTIFIER);
        assert(valname);

        if (scstrcmp(idxname->id(), valname->id()) == 0) //-V522
            Error(_SC("foreach() key and value names are the same: %s"), valname->id());
    }
    else {
        //idxname = newNode<Id>(_SC("@INDEX@"));
    }

    Expect(TK_IN);

    Expr *contnr = Expression(SQE_RVALUE);
    Expect(_SC(')'));

    Statement *body = parseStatement();

    return newNode<ForeachStatement>(idxname, valname, contnr, body);
}

SwitchStatement* SQParser::parseSwitchStatement()
{
    Consume(TK_SWITCH);

    Expect(_SC('('));
    Expr *switchExpr = Expression(SQE_SWITCH);
    Expect(_SC(')'));

    Expect(_SC('{'));

    SwitchStatement *switchStmt = newNode<SwitchStatement>(arena(), switchExpr);

    while(_token == TK_CASE) {
        Consume(TK_CASE);

        Expr *cond = Expression(SQE_RVALUE);
        Expect(_SC(':'));

        Statement *caseBody = parseStatements();
        switchStmt->addCases(cond, caseBody);
    }

    if(_token == TK_DEFAULT) {
        Consume(TK_DEFAULT);
        Expect(_SC(':'));

        switchStmt->addDefault(parseStatements());
    }

    Expect(_SC('}'));

    return switchStmt;
}

FunctionDecl* SQParser::parseFunctionStatement()
{
    SQInteger l = _lex._currentline, c = _lex._currentcolumn;
    Consume(TK_FUNCTION);
    Id *funcName = (Id *)Expect(TK_IDENTIFIER);
    Expect(_SC('('));
    FunctionDecl *d = CreateFunction(funcName);
    d->setLinePos(l); d->setColumnPos(c);
    return d;
}

ClassDecl* SQParser::parseClassStatement()
{
    SQInteger l = _lex._currentline, c = _lex._currentcolumn;
    Consume(TK_CLASS);

    Expr *key = PrefixedExpr();

    ClassDecl *klass = ClassExp(key);
    klass->setContext(DC_SLOT);
    klass->setLinePos(l);
    klass->setColumnPos(c);

    return klass;
}

LiteralExpr* SQParser::ExpectScalar()
{
    LiteralExpr *ret = NULL;
    SQInteger l = _lex._currentline, c = _lex._currentcolumn;

    switch(_token) {
        case TK_INTEGER:
            ret = newNode<LiteralExpr>(_lex._nvalue);
            break;
        case TK_FLOAT:
            ret = newNode<LiteralExpr>(_lex._fvalue);
            break;
        case TK_STRING_LITERAL:
            ret = newStringLiteral(_lex._svalue);
            break;
        case TK_TRUE:
        case TK_FALSE:
            ret = newNode<LiteralExpr>((bool)(_token == TK_TRUE ? 1 : 0));
            break;
        case '-':
            Lex();
            switch(_token)
            {
            case TK_INTEGER:
                ret = newNode<LiteralExpr>(-_lex._nvalue);
            break;
            case TK_FLOAT:
                ret = newNode<LiteralExpr>(-_lex._fvalue);
            break;
            default:
                Error(_SC("scalar expected : integer, float"));
            }
            break;
        default:
            Error(_SC("scalar expected : integer, float, or string"));
    }
    Lex();
    ret->setLinePos(l);
    ret->setColumnPos(c);
    return ret;
}


ConstDecl* SQParser::parseConstStatement(bool global)
{
    SQInteger l = _lex._currentline, c = _lex._currentcolumn;
    Lex();
    Id *id = (Id *)Expect(TK_IDENTIFIER);

    Expect('=');
    LiteralExpr *valExpr = ExpectScalar();
    OptionalSemicolon();

    ConstDecl *d = newNode<ConstDecl>(id->id(), valExpr, global);
    d->setColumnPos(c);
    d->setLinePos(l);
    return d;
}


EnumDecl* SQParser::parseEnumStatement(bool global)
{
    SQInteger l = _lex._currentline, c = _lex._currentcolumn;
    Lex();
    Id *id = (Id *)Expect(TK_IDENTIFIER);

    EnumDecl *decl = newNode<EnumDecl>(arena(), id->id(), global);

    Expect(_SC('{'));

    SQInteger nval = 0;
    while(_token != _SC('}')) {
        Id *key = (Id *)Expect(TK_IDENTIFIER);
        LiteralExpr *valExpr = NULL;
        if(_token == _SC('=')) {
            // TODO1: should it behave like C does?
            // TODO2: should float and string literal be allowed here?
            Lex();
            valExpr = ExpectScalar();
        }
        else {
            valExpr = newNode<LiteralExpr>(nval++);
        }

        decl->addConst(key->id(), valExpr);

        if(_token == ',') Lex();
    }

    Lex();

    decl->setLinePos(l);
    decl->setColumnPos(c);

    return decl;
}


TryStatement* SQParser::parseTryCatchStatement()
{
    Consume(TK_TRY);

    Statement *t = parseStatement();

    Expect(TK_CATCH);

    Expect(_SC('('));
    Id *exid = (Id *)Expect(TK_IDENTIFIER);
    Expect(_SC(')'));

    Statement *c = parseStatement();

    return newNode<TryStatement>(t, exid, c);
}


Id* SQParser::generateSurrogateFunctionName()
{
    const SQChar * fileName = _sourcename ? _sourcename : _SC("unknown");
    int lineNum = int(_lex._currentline);

    const SQChar * rightSlash = std::max(scstrrchr(fileName, _SC('/')), scstrrchr(fileName, _SC('\\')));

    SQChar buf[MAX_FUNCTION_NAME_LEN];
    scsprintf(buf, MAX_FUNCTION_NAME_LEN, _SC("(%s:%d)"), rightSlash ? (rightSlash + 1) : fileName, lineNum);
    return newId(buf);
}


DeclExpr* SQParser::FunctionExp(SQInteger ftype, bool lambda)
{
    SQInteger l = _lex._currentline, c = _lex._currentcolumn;
    Lex();
    Id *funcName = (_token == TK_IDENTIFIER) ? (Id *)Expect(TK_IDENTIFIER) : generateSurrogateFunctionName();
    Expect(_SC('('));

    Decl *f = CreateFunction(funcName, lambda);
    f->setLinePos(l); f->setColumnPos(c);
    return  newNode<DeclExpr>(f);
}


ClassDecl* SQParser::ClassExp(Expr *key)
{
    Expr *baseExpr = NULL;
    if(_token == TK_EXTENDS) {
        Lex();
        baseExpr = Expression(SQE_RVALUE);
    }
    Expect(_SC('{'));
    ClassDecl *d = newNode<ClassDecl>(arena(), key, baseExpr);
    ParseTableOrClass(d, _SC(';'),_SC('}'));
    return d;
}


Expr* SQParser::DeleteExpr()
{
    Consume(TK_DELETE);
    Expr *arg = PrefixedExpr();
    return newNode<UnExpr>(TO_DELETE, arg);
}


Expr* SQParser::PrefixIncDec(SQInteger token)
{
    SQInteger diff = (token==TK_MINUSMINUS) ? -1 : 1;
    Lex();
    Expr *arg = PrefixedExpr();
    return newNode<IncExpr>(arg, diff, IF_PREFIX);
}


FunctionDecl* SQParser::CreateFunction(Id *name, bool lambda, bool ctor)
{
    FunctionDecl *f = ctor ? newNode<ConstructorDecl>(arena(), name->id()) : newNode<FunctionDecl>(arena(), name->id());
    f->setLinePos(_lex._currentline); f->setColumnPos(_lex._currentcolumn);

    f->addParameter(_SC("this"));

    SQInteger defparams = 0;

    while (_token!=_SC(')')) {
        if (_token == TK_VARPARAMS) {
            if(defparams > 0) Error(_SC("function with default parameters cannot have variable number of parameters"));
            f->addParameter(_SC("vargv"));
            f->setVararg();
            Lex();
            if(_token != _SC(')')) Error(_SC("expected ')'"));
            break;
        }
        else {
            Id *paramname = (Id *)Expect(TK_IDENTIFIER);
            Expr *defVal = NULL;
            if (_token == _SC('=')) {
                Lex();
                defVal = Expression(SQE_RVALUE);
                defparams++;
            }
            else {
                if (defparams > 0) Error(_SC("expected '='"));
            }

            f->addParameter(paramname->id(), defVal);

            if(_token == _SC(',')) Lex();
            else if(_token != _SC(')')) Error(_SC("expected ')' or ','"));
        }
    }

    Expect(_SC(')'));

    Block *body = NULL;
    SQInteger startLine = _lex._currentline;

    if (lambda) {
        SQInteger line = _lex._prevtoken == _SC('\n') ? _lex._lasttokenline : _lex._currentline;
        Expr *expr = Expression(SQE_REGULAR);
        ReturnStatement *retStmt = newNode<ReturnStatement>(expr);
        retStmt->setLinePos(line);
        retStmt->setIsLambda();
        body = newNode<Block>(arena());
        body->addStatement(retStmt);
    }
    else {
        if (_token != '{')
            Error(_SC("'{' expected"));
        body = (Block *)parseStatement(false);
    }
    SQInteger line2 = _lex._prevtoken == _SC('\n') ? _lex._lasttokenline : _lex._currentline;
    body->setStartLine(startLine);
    body->setEndLine(line2);

    f->setBody(body);

    f->setSourceName(_sourcename);
    f->setLambda(lambda);

    return f;
}


#endif
