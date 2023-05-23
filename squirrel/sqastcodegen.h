#pragma once

#include "sqast.h"
#include "sqcompiler.h"
#include "sqopcodes.h"
#include <setjmp.h>

struct SQFuncState;

class CodegenVisitor : public Visitor {

    SQFuncState *_fs;
    SQFuncState *_childFs;
    SQVM *_vm;

    SQScope _scope;
    SQObjectPtrVec _scopedconsts;

    bool _donot_get;

    SQInteger _num_initial_bindings;
    SQInteger _lang_features;

    bool _lineinfo;
    bool _raiseerror;
    const SQChar *_sourceName;

    jmp_buf _errorjmp;

    Arena *_arena;

    SQChar _compilererror[MAX_COMPILER_ERROR_LEN];
    Node *_errorNode;

    SQInteger _last_pop = -1;

    SQObjectPtr _constVal;

public:
    CodegenVisitor(Arena *arena, const HSQOBJECT *bindings, SQVM *vm, const SQChar *sourceName, SQInteger lang_fuatures, bool lineinfo, bool raiseerror);

    bool generate(RootBlock *root, SQObjectPtr &out);

    static void ThrowError(void *ud, const SQChar *s) {
        CodegenVisitor *c = (CodegenVisitor *)ud;
        c->error(NULL, s);
    }

private:
    void error(Node *n, const SQChar *s, ...);

    void CheckDuplicateLocalIdentifier(Node *n, SQObject name, const SQChar *desc, bool ignore_global_consts);
    bool CheckMemberUniqueness(ArenaVector<Expr *> &vec, Expr *obj);

    void Emit2ArgsOP(SQOpcode op, SQInteger p3 = 0);

    void EmitLoadConstInt(SQInteger value, SQInteger target);

    void EmitLoadConstFloat(SQFloat value, SQInteger target);

    void ResolveBreaks(SQFuncState *funcstate, SQInteger ntoresolve);
    void ResolveContinues(SQFuncState *funcstate, SQInteger ntoresolve, SQInteger targetpos);

    void EmitDerefOp(SQOpcode op);

    void generateTableDecl(TableDecl *tableDecl);

    void checkClassKey(Expr *key);

    SQTable* GetScopedConstsTable();

    void emitUnaryOp(SQOpcode op, UnExpr *arg);

    void emitDelete(UnExpr *argument);

    void emitSimpleBin(SQOpcode op, Expr *lhs, Expr *rhs, SQInteger op3 = 0);

    void emitJpmArith(SQOpcode op, Expr *lhs, Expr *rhs);

    void emitCompoundArith(SQOpcode op, SQInteger opcode, Expr *lvalue, Expr *rvalue);

    bool isLValue(Expr *expr);

    void emitNewSlot(Expr *lvalue, Expr *rvalue);

    void emitAssign(Expr *lvalue, Expr * rvalue, bool inExpr);

    void emitFieldAssign(bool isLiteral);

    bool CanBeDefaultDelegate(const SQChar *key);

    bool canBeLiteral(AccessExpr *expr);

    void MoveIfCurrentTargetIsLocal();

    bool IsConstant(const SQObject &name, SQObject &e);

    bool IsLocalConstant(const SQObject &name, SQObject &e);

    bool IsGlobalConstant(const SQObject &name, SQObject &e);

    SQObject selectLiteral(LiteralExpr *lit);

    void maybeAddInExprLine(Expr *expr);

    void addLineNumber(Statement *stmt);

    void visitNoGet(Node *n);
    void visitForceGet(Node *n);

    void selectConstant(SQInteger target, const SQObject &constant);

public:

    void visitBlock(Block *block) override;

    void visitIfStatement(IfStatement *ifStmt) override;

    void visitWhileStatement(WhileStatement *whileLoop) override;

    void visitDoWhileStatement(DoWhileStatement *doWhileLoop) override;

    void visitForStatement(ForStatement *forLoop) override;

    void visitForeachStatement(ForeachStatement *foreachLoop) override;

    void visitSwitchStatement(SwitchStatement *swtch) override;

    void visitTryStatement(TryStatement *tryStmt) override;

    void visitBreakStatement(BreakStatement *breakStmt) override;

    void visitContinueStatement(ContinueStatement *continueStmt) override;

    void visitTerminateStatement(TerminateStatement *terminator) override;

    void visitReturnStatement(ReturnStatement *retStmt) override;

    void visitYieldStatement(YieldStatement *yieldStmt) override;

    void visitThrowStatement(ThrowStatement *throwStmt) override;

    void visitExprStatement(ExprStatement *stmt) override;

    void visitTableDecl(TableDecl *tableDecl) override;

    void visitClassDecl(ClassDecl *klass) override;

    void visitParamDecl(ParamDecl *param) override;

    void visitVarDecl(VarDecl *var) override;

    void visitDeclGroup(DeclGroup *group) override;

    void visitDesctructingDecl(DestructuringDecl *destruct) override;

    void visitFunctionDecl(FunctionDecl *func) override;

    void visitConstDecl(ConstDecl *decl) override;

    void visitEnumDecl(EnumDecl *enums) override;

    void visitCallExpr(CallExpr *call) override;

    void visitBaseExpr(BaseExpr *base) override;

    void visitRootExpr(RootExpr *expr) override;

    void visitLiteralExpr(LiteralExpr *lit) override;

    void visitArrayExpr(ArrayExpr *expr) override;

    void visitUnExpr(UnExpr *unary) override;

    void visitGetFieldExpr(GetFieldExpr *expr) override;

    void visitGetTableExpr(GetTableExpr *expr) override;

    void visitBinExpr(BinExpr *expr) override;

    void visitTerExpr(TerExpr *expr) override;

    void visitIncExpr(IncExpr *expr) override;

    void visitId(Id *id) override;

    void visitCommaExpr(CommaExpr *expr) override;

};
