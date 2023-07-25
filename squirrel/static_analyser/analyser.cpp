#include "analyser.h"
#include <stdarg.h>



namespace SQCompilation {

StaticAnalyser::StaticAnalyser(SQCompilationContext &ctx)
  : _ctx(ctx) {

}


class ExpressionChecker : public Visitor
{
  SQCompilationContext &_ctx;


  void report(Node *n, enum DiagnosticsId id, ...);
public:
  ExpressionChecker(SQCompilationContext &ctx) : _ctx(ctx) {}

  void visitBinExpr(BinExpr *expr);
};

void ExpressionChecker::report(Node *n, enum DiagnosticsId id, ...) {
  va_list vargs;
  va_start(vargs, id);

  _ctx.vreportDiagnostic(id, n->lineStart(), n->columnStart(), n->columnEnd() - n->columnStart(), vargs);

  va_end(vargs);
}


void ExpressionChecker::visitBinExpr(BinExpr *expr) {
  Expr *l = expr->lhs();
  Expr *r = expr->rhs();
  
  if (expr->op() == TO_OROR) {
    if (l->op() == TO_ANDAND || r->op() == TO_ANDAND) {
      report(expr, DiagnosticsId::DI_AND_OR_PAREN);
    }
  }

  if (expr->op() == TO_OROR || expr->op() == TO_ANDAND) {
    if (l->op() == TO_AND || l->op() == TO_OR || r->op() == TO_AND || r->op() == TO_OR) {
      report(expr, DiagnosticsId::DI_BITWISE_BOOL_PAREN);
    }
  }

  expr->visitChildren(this);
}


void StaticAnalyser::runAnalysis(RootBlock *root)
{
  ExpressionChecker ec(_ctx);
  root->visit(&ec);
}

}
