#pragma once

#include <iostream>
#include "sqast.h"

class TreeDumpVisitor : public Visitor {

    void indent(int ind) {
        for (int i = 0; i < ind; ++i) _out << "  ";
    }

    const char* treeopStr(enum TreeOp op) {
        switch (op)
        {
            case TO_NULLC: return "??";
            case TO_ASSIGN: return "=";
            case TO_OROR: return "||";
            case TO_ANDAND: return "&&";
            case TO_OR: return "|";
            case TO_XOR: return "^";
            case TO_AND: return "&";
            case TO_NE: return "!=";
            case TO_EQ: return "==";
            case TO_3CMP: return "<=>";
            case TO_GE: return ">=";
            case TO_GT: return ">";
            case TO_LE: return "<=";
            case TO_LT: return "<";
            case TO_USHR: return ">>>";
            case TO_SHR: return ">>";
            case TO_SHL: return "<<";
            case TO_MUL: return "*";
            case TO_DIV: return "/";
            case TO_MOD: return "%";
            case TO_ADD: return "+";
            case TO_SUB: return "-";
            case TO_NOT: return "!";
            case TO_BNOT: return "~";
            case TO_NEG: return "-";
            case TO_INEXPR_ASSIGN: return ":=";
            case TO_NEWSLOT: return "<-";
            case TO_PLUSEQ: return "+=";
            case TO_MINUSEQ: return "-=";
            case TO_MULEQ: return "*=";
            case TO_DIVEQ: return "/=";
            case TO_MODEQ: return "%=";
            default: return nullptr;
        }
    }

public:
    TreeDumpVisitor(std::ostream &output) : _out(output), _indent(0) {}

    std::ostream &_out;
    SQInteger _indent;

    virtual void visitNode(Node *node) override {
        TreeOp op = node->op();
        indent(_indent);
        _out << sq_tree_op_names[op];
        const char *opStr = treeopStr(op);
        if (opStr)
            _out << " (" << opStr << ")";
        if (op == TO_ID) {
            _out << " [" << static_cast<Id *>(node)->id() << "]";
        } else if (op == TO_LITERAL) {
            LiteralExpr *literal = static_cast<LiteralExpr *>(node);
            LiteralKind kind = literal->kind();
            _out << " [";
            switch (kind) {
                case LK_STRING: _out << '"' << literal->s() << '"'; break;
                case LK_INT: _out << literal->i(); break;
                case LK_FLOAT: _out << literal->f(); break;
                case LK_BOOL: _out << (literal->b() ? "true" : "false"); break;
            }
            _out << "]";
        }
        _out << std::endl;

        ++_indent;
        node->visitChildren(this);
        --_indent;
    }
};

