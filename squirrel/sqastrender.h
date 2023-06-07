#pragma once

#ifdef _DEBUG_DUMP

#include <iostream>

class RenderVisitor : public Visitor {


    const char* treeopToStr(enum TreeOp op) {
        switch (op)
        {
        case TO_NULLC: return " ?? ";
        case TO_ASSIGN: return " = ";
        case TO_OROR: return " || ";
        case TO_ANDAND: return " && ";
        case TO_OR: return " | ";
        case TO_XOR: return " ^ ";
        case TO_AND: return " & ";
        case TO_NE: return " != ";
        case TO_EQ: return " == ";
        case TO_3CMP: return " <=> ";
        case TO_GE: return " >= ";
        case TO_GT: return " > ";
        case TO_LE: return " <= ";
        case TO_LT: return " < ";
        case TO_IN: return " IN ";
        case TO_INSTANCEOF: " INSTANCEOF ";
        case TO_USHR: return " >>> ";
        case TO_SHR: return " >> ";
        case TO_SHL: return " << ";
        case TO_MUL: return " * ";
        case TO_DIV: return " / ";
        case TO_MOD: return " % ";
        case TO_ADD: return " + ";
        case TO_SUB: return " - ";
        case TO_NOT: return " ! ";
        case TO_BNOT: return " ~ ";
        case TO_NEG: return " - ";
        case TO_TYPEOF: return "TYPEOF ";
        case TO_RESUME: return "RESUME ";
        case TO_CLONE: return "CLONE ";
        case TO_DELETE: return "DELETE ";
        case TO_INEXPR_ASSIGN: return " := ";
        case TO_NEWSLOT: return " <- ";
        case TO_PLUSEQ: return " += ";
        case TO_MINUSEQ: return " -= ";
        case TO_MULEQ: return " *= ";
        case TO_DIVEQ: return " /= ";
        case TO_MODEQ: return " %= ";
        default: return "<UNKNOWN>";
        }
    }

    void indent(int ind) {
        for (int i = 0; i < ind; ++i) _out << ' ';
    }

public:
    RenderVisitor(std::ostream &output) : _out(output), _indent(0) {}

    std::ostream &_out;
    SQInteger _indent;

    void render(Node *n) {
        n->visit(this);
        _out << std::endl;
    }

    virtual void visitUnExpr(UnExpr *expr) {
        if (expr->op() == TO_PAREN) {
            _out << "(";
        }
        else {
            _out << treeopToStr(expr->op());
        }
        expr->argument()->visit(this);
        if (expr->op() == TO_PAREN) {
            _out << ")";
        }
    }
    virtual void visitBinExpr(BinExpr *expr) {
        expr->_lhs->visit(this);
        _out << treeopToStr(expr->op());
        expr->_rhs->visit(this);
    }
    virtual void visitTerExpr(TerExpr *expr) {
        expr->a()->visit(this);
        _out << " ? ";
        expr->b()->visit(this);
        _out << " : ";
        expr->c()->visit(this);
    }
    virtual void visitCallExpr(CallExpr *expr) {
        expr->callee()->visit(this);
        _out << "(";
        for (SQUnsignedInteger i = 0; i < expr->arguments().size(); ++i) {
            if (i) _out << ", ";
            expr->arguments()[i]->visit(this);
        }
        _out << ")";

    }
    virtual void visitId(Id *id) { _out << id->id(); }

    virtual void visitGetFieldExpr(GetFieldExpr *expr) {
        expr->receiver()->visit(this);
        if (expr->isNullable()) _out << '?';
        _out << '.';
        _out << expr->fieldName();
    }
    virtual void visitSetFieldExpr(SetFieldExpr *expr) {
        expr->receiver()->visit(this);
        if (expr->isNullable()) _out << '?';
        _out << '.';
        _out << expr->fieldName();
        _out << " = ";
        expr->value()->visit(this);
    }
    virtual void visitGetTableExpr(GetTableExpr *expr) {
        expr->receiver()->visit(this);
        if (expr->isNullable()) _out << '?';
        _out << '[';
        expr->key()->visit(this);
        _out << ']';
    }
    virtual void visitSetTableExpr(SetTableExpr *expr) {
        expr->receiver()->visit(this);
        if (expr->isNullable()) _out << '?';
        _out << '[';
        expr->key()->visit(this);
        _out << ']';
        _out << " = ";
        expr->value()->visit(this);
    }
    virtual void visitBaseExpr(BaseExpr *expr) { _out << "base"; }
    virtual void visitRootExpr(RootExpr *expr) { _out << "::"; }
    virtual void visitLiteralExpr(LiteralExpr *expr) {
        switch (expr->kind())
        {
        case LK_STRING: _out << '"' << expr->s() << '"'; break;
        case LK_FLOAT: _out << expr->f(); break;
        case LK_INT: _out << expr->i(); break;
        case LK_BOOL: _out << expr->b(); break;
        case LK_NULL: _out << "#NULL"; break;
        default: assert(0);
            break;
        };
    }
    virtual void visitIncExpr(IncExpr *expr) {

        const char *op = expr->diff() < 0 ? "--" : "++";

        if (expr->form() == IF_PREFIX) _out << op;
        expr->argument()->visit(this);
        if (expr->form() == IF_POSTFIX) _out << op;
    }

    virtual void visitArrayExpr(ArrayExpr *expr) {
        _out << "[";
        for (SQUnsignedInteger i = 0; i < expr->initialziers().size(); ++i) {
            if (i) _out << ", ";
            expr->initialziers()[i]->visit(this);
        }
        _out << "]";
    }

    virtual void visitCommaExpr(CommaExpr *expr) {
        for (SQUnsignedInteger i = 0; i < expr->expressions().size(); ++i) {
            if (i) _out << ", ";
            expr->expressions()[i]->visit(this);
        }
    }

    virtual void visitBlock(Block *block) {
        _out << "{" << std::endl;
        int cur = _indent;
        _indent += 2;
        ArenaVector<Statement *> &stmts = block->statements();

        for (auto stmt : stmts) {
            indent(_indent);
            stmt->visit(this);
            _out << std::endl;
        }

        indent(cur);
        _out << "}";
        _indent = cur;
    }
    virtual void visitIfStatement(IfStatement *ifstmt) {
        _out << "IF (";
        ifstmt->condition()->visit(this);
        _out << ")" << std::endl;
        indent(_indent);
        _out << "THEN ";
        _indent += 2;
        ifstmt->thenBranch()->visit(this);
        if (ifstmt->elseBranch()) {
            _out << std::endl;
            indent(_indent - 2);
            _out << "ELSE ";
            ifstmt->elseBranch()->visit(this);
        }
        _indent -= 2;
        _out << std::endl;
        indent(_indent);
        _out << "END_IF";

    }
    virtual void visitLoopStatement(LoopStatement *loop) {
        indent(_indent);
        loop->body()->visit(this);
    }
    virtual void visitWhileStatement(WhileStatement *loop) {
        _out << "WHILE (";
        loop->condition()->visit(this);
        _out << ")" << std::endl;
        _indent += 2;
        visitLoopStatement(loop);
        _indent -= 2;
        indent(_indent);
        _out << "END_WHILE";

    }
    virtual void visitDoWhileStatement(DoWhileStatement *loop) {
        _out << "DO";

        _indent += 2;
        visitLoopStatement(loop);
        _indent -= 2;

        indent(_indent);
        _out << "WHILE (" << std::endl;
        loop->condition()->visit(this);
        _out << ")";
    }
    virtual void visitForStatement(ForStatement *loop) {
        _out << "FOR (";

        if (loop->initializer()) loop->initializer()->visit(this);
        _out << "; ";

        if (loop->condition()) loop->condition()->visit(this);
        _out << "; ";

        if (loop->modifier()) loop->modifier()->visit(this);
        _out << ")" << std::endl;


        _indent += 2;
        visitLoopStatement(loop);
        _indent -= 2;

        _out << std::endl;

        indent(_indent);
        _out << "END_FOR" << std::endl;
    }
    virtual void visitForeachStatement(ForeachStatement *loop) {
        _out << "FOR_EACH ( {";

        if (loop->idx()) {
            visitId(loop->idx());
            _out << ", ";
        }

        visitId(loop->val());
        _out << "} in ";

        loop->container()->visit(this);
        _out << ")" << std::endl;

        _indent += 2;
        visitLoopStatement(loop);
        _indent -= 2;

        _out << std::endl;
        indent(_indent);
        _out << "END_FOREACH";
    }
    virtual void visitSwitchStatement(SwitchStatement *swtch) {
        _out << "SWITCH (";
        swtch->expression()->visit(this);
        _out << ")" << std::endl;
        int cur = _indent;
        _indent += 2;

        for (auto &c : swtch->cases()) {
            indent(cur);
            _out << "CASE ";
            c.val->visit(this);
            _out << ": ";
            c.stmt->visit(this);
            _out << std::endl;
        }

        if (swtch->defaultCase().stmt) {
            indent(cur);
            _out << "DEFAULT: ";
            swtch->defaultCase().stmt->visit(this);
            _out << std::endl;
        }

        _indent -= 2;
    }
    virtual void visitTryStatement(TryStatement *tr) {
        _out << "TRY ";
        //_indent += 2;
        tr->tryStatement()->visit(this);
        _out << std::endl;
        indent(_indent);
        _out << "CATCH (";
        visitId(tr->exceptionId());
        _out << ") ";
        tr->catchStatement()->visit(this);
        _out << std::endl;
        indent(_indent);
        _out << "END_TRY";
    }
    virtual void visitTerminateStatement(TerminateStatement *term) { if (term->argument()) term->argument()->visit(this); }
    virtual void visitReturnStatement(ReturnStatement *ret) {
        _out << "RETURN ";
        visitTerminateStatement(ret);
    }
    virtual void visitYieldStatement(YieldStatement *yld) {
        _out << "YIELD ";
        visitTerminateStatement(yld);
    }
    virtual void visitThrowStatement(ThrowStatement *thr) {
        _out << "THROW ";
        visitTerminateStatement(thr);
    }
    virtual void visitBreakStatement(BreakStatement *jmp) { _out << "BREAK"; }
    virtual void visitContinueStatement(ContinueStatement *jmp) { _out << "CONTINUE"; }
    virtual void visitExprStatement(ExprStatement *estmt) { estmt->expression()->visit(this); }
    virtual void visitEmptyStatement(EmptyStatement *empty) { _out << ";"; }

    virtual void visitValueDecl(ValueDecl *decl) {
        if (decl->op() == TO_VAR) {
            _out << (((VarDecl *)decl)->isAssignable() ? "local " : "let ");
        }
        _out << decl->name();
        if (decl->expression()) {
            _out << " = ";
            decl->expression()->visit(this);
        }
    }
    virtual void visitTableDecl(TableDecl *tbl) {
        _out << "{" << std::endl;
        _indent += 2;

        for (auto &m : tbl->members()) {
            indent(_indent);
            if (m.isStatic) _out << "STATIC ";
            m.key->visit(this);
            _out << " <- ";
            m.value->visit(this);
            _out << std::endl;
        }

        _indent -= 2;
        indent(_indent);
        _out << "}";
    }
    virtual void visitClassDecl(ClassDecl *cls) {
        _out << "CLASS ";
        if (cls->classKey()) cls->classKey()->visit(this);
        if (cls->classBase()) {
            _out << " EXTENDS ";
            cls->classBase()->visit(this);
            _out << ' ';
        }
        visitTableDecl(cls);
    }
    virtual void visitFunctionDecl(FunctionDecl *f) {
        _out << "FUNCTION";
        if (f->isLambda()) {
            _out << " @ ";
        } else if (f->name()) {
            _out << " ";
            _out << f->name();
        }

        _out << '(';
        for (SQUnsignedInteger i = 0; i < f->parameters().size(); ++i) {
            if (i) _out << ", ";
            visitParamDecl(f->parameters()[i]);
        }

        if (f->isVararg()) {
            _out << ", ...";
        }
        _out << ") ";

        f->body()->visit(this);
    }
    virtual void visitConstructorDecl(ConstructorDecl *ctr) { visitFunctionDecl(ctr); }
    virtual void visitConstDecl(ConstDecl *cnst) {
        if (cnst->isGlobal()) _out << "G ";
        _out << cnst->name();
        _out << " = ";
        cnst->value()->visit(this);
    }
    virtual void visitEnumDecl(EnumDecl *enm) {
        _out << "ENUM ";
        _out << enm->name();
        _out << std::endl;
        _indent += 2;

        for (auto &c : enm->consts()) {
            indent(_indent);
            _out << c.id;
            _out << " = ";
            c.val->visit(this);
            _out << std::endl;
        }
        _indent -= 2;
        indent(_indent);
        _out << "END_ENUM";
    }

    virtual void visitDeclGroup(DeclGroup *grp) {
        for (SQUnsignedInteger i = 0; i < grp->declarations().size(); ++i) {
            if (i) _out << ", ";
            grp->declarations()[i]->visit(this);
        }
    }

    virtual void visitDesctructingDecl(DestructuringDecl *destruct) {
        _out << "{ ";
        for (SQUnsignedInteger i = 0; i < destruct->declarations().size(); ++i) {
            if (i) _out << ", ";
            destruct->declarations()[i]->visit(this);
        }
        _out << " } = ";
        destruct->initiExpression()->visit(this);
    }
};

#endif // _DEBUG_DUMP
