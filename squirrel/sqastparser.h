#pragma once

#include "sqpcheader.h"
#ifndef NO_COMPILER
#include <ctype.h>
#include <setjmp.h>
#include <algorithm>
#include "sqcompiler.h"
#include "sqlexer.h"
#include "sqvm.h"
#include "sqast.h"


class SQParser
{
    Arena *_astArena;

    template<typename N, typename ... Args>
    N *newNode(Args... args) {
        return new (arena()) N(args...);
    }

    SQChar *copyString(const SQChar *s) {
        size_t len = strlen(s);
        size_t memLen = (len + 1) * sizeof(SQChar);
        SQChar *buf = (SQChar *)arena()->allocate(memLen);
        memcpy(buf, s, memLen);
        return buf;
    }

    Id *newId(const SQChar *name) {
        Id *r = newNode<Id>(copyString(name));
        r->setLinePos(_lex._currentline);
        r->setColumnPos(_lex._currentcolumn);
        return r;
    }

    LiteralExpr *newStringLiteral(const SQChar *s) {
        LiteralExpr *r = newNode<LiteralExpr>(copyString(s));
        r->setLinePos(_lex._currentline);
        r->setColumnPos(_lex._currentcolumn);
        return r;
    }

    Arena *arena() { return _astArena; }

public:

    SQParser(SQVM *v, SQLEXREADFUNC rg, SQUserPointer up, const SQChar* sourcename, Arena *astArena, bool raiseerror, bool lineinfo);

    static void ThrowError(void *ud, const SQChar *s) {
        SQParser *c = (SQParser *)ud;
        c->Error(s);
    }
    void Error(const SQChar *s, ...);


    bool ProcessPosDirective();
    void Lex();

    void Consume(SQInteger tok) {
        assert(tok == _token);
        Lex();
    }

    Expr*   Expect(SQInteger tok);
    bool    IsEndOfStatement() {
        return ((_lex._prevtoken == _SC('\n')) || (_token == SQUIRREL_EOB) || (_token == _SC('}')) || (_token == _SC(';')))
            || (_token == TK_DIRECTIVE);
    }
    void    OptionalSemicolon();

    RootBlock*  parse();
    Block*      parseStatements();
    Statement*  parseStatement(bool closeframe = true);
    Expr*       parseCommaExpr(SQExpressionContext expression_context);
    Expr*       Expression(SQExpressionContext expression_context);

    template<typename T> Expr *BIN_EXP(T f, enum TreeOp top, Expr *lhs);

    Expr*   LogicalNullCoalesceExp();
    Expr*   LogicalOrExp();
    Expr*   LogicalAndExp();
    Expr*   BitwiseOrExp();
    Expr*   BitwiseXorExp();
    Expr*   BitwiseAndExp();
    Expr*   EqExp();
    Expr*   CompExp();
    Expr*   ShiftExp();
    Expr*   PlusExp();
    Expr*   MultExp();
    Expr*   PrefixedExpr();
    Expr*   Factor(SQInteger &pos);
    Expr*   UnaryOP(enum TreeOp op);

    void ParseTableOrClass(TableDecl *decl, SQInteger separator, SQInteger terminator);

    Decl* parseLocalDeclStatement(bool assignable);
    Statement* IfBlock();
    IfStatement* parseIfStatement();
    WhileStatement* parseWhileStatement();
    DoWhileStatement* parseDoWhileStatement();
    ForStatement* parseForStatement();
    ForeachStatement* parseForEachStatement();
    SwitchStatement* parseSwitchStatement();
    FunctionDecl* parseFunctionStatement();
    ClassDecl* parseClassStatement();
    LiteralExpr* ExpectScalar();
    ConstDecl* parseConstStatement(bool global);
    EnumDecl* parseEnumStatement(bool global);
    TryStatement* parseTryCatchStatement();
    Id* generateSurrogateFunctionName();
    DeclExpr* FunctionExp(SQInteger ftype, bool lambda = false);
    ClassDecl* ClassExp(Expr *key);
    Expr* DeleteExpr();
    Expr* PrefixIncDec(SQInteger token);
    FunctionDecl* CreateFunction(Id *name, bool lambda = false, bool ctor = false);
    Statement* parseDirectiveStatement();

private:
    SQInteger _token;
    const SQChar *_sourcename;
    SQLexer _lex;
    bool _raiseerror;
    SQExpressionContext _expression_context;
    SQUnsignedInteger _lang_features;
    SQChar _compilererror[MAX_COMPILER_ERROR_LEN];
    jmp_buf _errorjmp;
    SQVM *_vm;
};

#endif
