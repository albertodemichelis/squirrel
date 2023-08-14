#include "analyser.h"
#include <stdarg.h>
#include <cctype>
#include <unordered_set>

namespace SQCompilation {

const Expr *deparen(const Expr *e) {
  if (!e) return nullptr;

  if (e->op() == TO_PAREN)
    return deparen(static_cast<const UnExpr *>(e)->argument());
  return e;
}

const Expr *skipUnary(const Expr *e) {
  if (!e) return nullptr;

  if (e->op() == TO_INC) {
    return skipUnary(static_cast<const IncExpr *>(e)->argument());
  }

  if (TO_NOT <= e->op() && e->op() <= TO_DELETE) {
    return skipUnary(static_cast<const UnExpr *>(e)->argument());
  }

  return e;
}

const Statement *unwrapBody(Statement *stmt) {

  if (stmt == nullptr)
    return stmt;

  if (stmt->op() != TO_BLOCK)
    return stmt;

  auto &stmts = stmt->asBlock()->statements();

  if (stmts.size() != 1)
    return nullptr;

  return unwrapBody(stmts[0]);
}

static Expr *unwrapExprStatement(Statement *stmt) {
  return stmt->op() == TO_EXPR_STMT ? static_cast<ExprStatement *>(stmt)->expression() : nullptr;
}

static const SQChar *enumFqn(Arena *arena, const SQChar *enumName, const SQChar *cname) {
  int32_t l1 = strlen(enumName);
  int32_t l2 = strlen(cname);
  int32_t l = l1 + 1 + l2 + 1;
  SQChar *r = (SQChar *)arena->allocate(l);
  snprintf(r, l, "%s.%s", enumName, cname);
  return r;
}

static int32_t strhash(const SQChar *s) {
  int32_t r = 0;
  while (*s) {
    r *= 31;
    r += *s;
    ++s;
  }

  return r;
}

struct StringHasher {
  int32_t operator()(const SQChar *s) const {
    return strhash(s);
  }
};

struct StringEqualer {
  int32_t operator()(const SQChar *a, const SQChar *b) const {
    return strcmp(a, b) == 0;
  }
};

StaticAnalyser::StaticAnalyser(SQCompilationContext &ctx)
  : _ctx(ctx) {

}

class NodeEqualChecker {

  template<typename N>
  bool cmpNodeVector(const ArenaVector<N *> &lhs, const ArenaVector<N *> &rhs) const {
    if (lhs.size() != rhs.size())
      return false;

    for (int32_t i = 0; i < lhs.size(); ++i) {
      if (!check(lhs[i], rhs[i]))
        return false;
    }

    return true;
  }

  bool cmpId(const Id *l, const Id* r) const {
    return strcmp(l->id(), r->id()) == 0;
  }

  bool cmpLiterals(const LiteralExpr *l, const LiteralExpr *r) const {
    if (l->kind() != r->kind())
      return false;

    switch (l->kind())
    {
    case LK_STRING: return strcmp(l->s(), r->s()) == 0;
    default: return l->raw() == r->raw();
    }
  }

  bool cmpBinary(const BinExpr *l, const BinExpr *r) const {
    return check(l->lhs(), r->lhs()) && check(l->rhs(), r->rhs());
  }

  bool cmpUnary(const UnExpr *l, const UnExpr *r) const {
    return check(l->argument(), r->argument());
  }

  bool cmpTernary(const TerExpr *l, const TerExpr *r) const {
    return check(l->a(), r->a()) && check(l->b(), r->b()) && check(l->c(), r->c());
  }

  bool cmpBlock(const Block *l, const Block *r) const {
    if (l->isRoot() != r->isRoot())
      return false;

    if (l->isBody() != r->isBody())
      return false;

    return cmpNodeVector(l->statements(), r->statements());
  }

  bool cmpIf(const IfStatement *l, const IfStatement *r) const {
    if (!check(l->condition(), r->condition()))
      return false;

    if (!check(l->thenBranch(), r->thenBranch()))
      return false;

    return check(l->elseBranch(), r->elseBranch());
  }

  bool cmpWhile(const WhileStatement *l, const WhileStatement *r) const {
    if (!check(l->condition(), r->condition()))
      return false;

    return check(l->body(), r->body());
  }

  bool cmpDoWhile(const DoWhileStatement *l, const DoWhileStatement *r) const {
    if (!check(l->body(), r->body()))
      return false;

    return check(l->condition(), r->condition());
  }

  bool cmpFor(const ForStatement *l, const ForStatement *r) const {
    if (!check(l->initializer(), r->initializer()))
      return false;

    if (!check(l->condition(), r->condition()))
      return false;

    if (!check(l->modifier(), r->modifier()))
      return false;

    return check(l->body(), r->body());
  }

  bool cmpForeach(const ForeachStatement *l, const ForeachStatement *r) const {
    if (!check(l->idx(), r->idx()))
      return false;

    if (!check(l->val(), r->val()))
      return false;

    if (!check(l->container(), r->container()))
      return false;

    return check(l->body(), r->body());
  }

  bool cmpSwitch(const SwitchStatement *l, const SwitchStatement *r) const {
    if (!check(l->expression(), r->expression()))
      return false;

    const auto &lcases = l->cases();
    const auto &rcases = r->cases();

    if (lcases.size() != rcases.size())
      return false;

    for (int32_t i = 0; i < lcases.size(); ++i) {
      const auto &lc = lcases[i];
      const auto &rc = rcases[i];

      if (!check(lc.val, rc.val))
        return false;

      if (!check(lc.stmt, rc.stmt))
        return false;
    }

    return check(l->defaultCase().stmt, r->defaultCase().stmt);
  }

  bool cmpTry(const TryStatement *l, const TryStatement *r) const {
    if (!check(l->tryStatement(), r->tryStatement()))
      return false;

    if (!check(l->exceptionId(), r->exceptionId()))
      return false;

    return check(l->catchStatement(), r->catchStatement());
  }

  bool cmpTerminate(const TerminateStatement *l, const TerminateStatement *r) const {
    return check(l->argument(), r->argument());
  }

  bool cmpReturn(const ReturnStatement *l, const ReturnStatement *r) const {
    return l->isLambdaReturn() == r->isLambdaReturn() && cmpTerminate(l, r);
  }

  bool cmpExprStmt(const ExprStatement *l, const ExprStatement *r) const {
    return check(l->expression(), r->expression());
  }

  bool cmpComma(const CommaExpr *l, const CommaExpr *r) const {
    return cmpNodeVector(l->expressions(), r->expressions());
  }

  bool cmpIncExpr(const IncExpr *l, const IncExpr *r) const {
    if (l->form() != r->form())
      return false;

    if (l->diff() != r->diff())
      return false;

    return check(l->argument(), r->argument());
  }

  bool cmpDeclExpr(const DeclExpr *l, const DeclExpr *r) const {
    return check(l->declaration(), r->declaration());
  }

  bool cmpCallExpr(const CallExpr *l, const CallExpr *r) const {
    if (l->isNullable() != r->isNullable())
      return false;

    if (!check(l->callee(), r->callee()))
      return false;

    return cmpNodeVector(l->arguments(), r->arguments());
  }

  bool cmpArrayExpr(const ArrayExpr *l, const ArrayExpr *r) const {
    return cmpNodeVector(l->initialziers(), r->initialziers());
  }

  bool cmpGetField(const GetFieldExpr *l, const GetFieldExpr *r) const {
    if (l->isNullable() != r->isNullable())
      return false;

    if (strcmp(l->fieldName(), r->fieldName()))
      return false;

    return check(l->receiver(), r->receiver());
  }

  bool cmpGetTable(const GetTableExpr *l, const GetTableExpr *r) const {
    if (l->isNullable() != r->isNullable())
      return false;

    if (!check(l->key(), r->key()))
      return false;

    return check(l->receiver(), r->receiver());
  }

  bool cmpValueDecl(const ValueDecl *l, const ValueDecl *r) const {
    if (!check(l->expression(), r->expression()))
      return false;

    return strcmp(l->name(), r->name()) == 0;
  }

  bool cmpVarDecl(const VarDecl *l, const VarDecl *r) const {
    return l->isAssignable() == r->isAssignable() && cmpValueDecl(l, r);
  }

  bool cmpConst(const ConstDecl *l, const ConstDecl *r) const {
    if (l->isGlobal() != r->isGlobal())
      return false;

    if (strcmp(l->name(), r->name()))
      return false;

    return cmpLiterals(l->value(), r->value());
  }

  bool cmpDeclGroup(const DeclGroup *l, const DeclGroup *r) const {
    return cmpNodeVector(l->declarations(), r->declarations());
  }

  bool cmpDestructDecl(const DestructuringDecl *l, const DestructuringDecl *r) const {
    return l->type() == r->type() && check(l->initiExpression(), r->initiExpression());
  }

  bool cmpFunction(const FunctionDecl *l, const FunctionDecl *r) const {
    if (l->isVararg() != r->isVararg())
      return false;

    if (l->isLambda() != r->isLambda())
      return false;

    if (strcmp(l->name(), r->name()))
      return false;

    if (!cmpNodeVector(l->parameters(), r->parameters()))
      return false;

    return check(l->body(), r->body());
  }

  bool cmpTable(const TableDecl *l, const TableDecl *r) const {
    const auto &lmems = l->members();
    const auto &rmems = r->members();

    if (lmems.size() != rmems.size())
      return false;

    for (int32_t i = 0; i < lmems.size(); ++i) {
      const auto &lm = lmems[i];
      const auto &rm = rmems[i];

      if (!check(lm.key, rm.key))
        return false;

      if (!check(lm.value, rm.value))
        return false;

      if (lm.isStatic != rm.isStatic)
        return false;
    }

    return true;
  }

  bool cmpClass(const ClassDecl *l, const ClassDecl *r) const {
    if (!check(l->classBase(), r->classBase()))
      return false;

    if (!check(l->classKey(), r->classKey()))
      return false;

    return cmpTable(l, r);
  }

  bool cmpEnumDecl(const EnumDecl *l, const EnumDecl *r) const {
    if (l->isGlobal() != r->isGlobal())
      return false;

    if (strcmp(l->name(), r->name()))
      return false;

    const auto &lcs = l->consts();
    const auto &rcs = r->consts();

    if (lcs.size() != rcs.size())
      return false;

    for (int32_t i = 0; i < lcs.size(); ++i) {
      const auto &lc = lcs[i];
      const auto &rc = rcs[i];

      if (strcmp(lc.id, lc.id))
        return false;

      if (!cmpLiterals(lc.val, rc.val))
        return false;
    }

    return true;
  }

public:

  bool check(const Node *lhs, const Node *rhs) const {

    if (lhs == rhs)
      return true;

    if (!lhs || !rhs)
      return false;

    if (lhs->op() != rhs->op())
      return false;

    switch (lhs->op())
    {
    case TO_BLOCK:      return cmpBlock((const Block *)lhs, (const Block *)rhs);
    case TO_IF:         return cmpIf((const IfStatement *)lhs, (const IfStatement *)rhs);
    case TO_WHILE:      return cmpWhile((const WhileStatement *)lhs, (const WhileStatement *)rhs);
    case TO_DOWHILE:    return cmpDoWhile((const DoWhileStatement *)lhs, (const DoWhileStatement *)rhs);
    case TO_FOR:        return cmpFor((const ForStatement *)lhs, (const ForStatement *)rhs);
    case TO_FOREACH:    return cmpForeach((const ForeachStatement *)lhs, (const ForeachStatement *)rhs);
    case TO_SWITCH:     return cmpSwitch((const SwitchStatement *)lhs, (const SwitchStatement *)rhs);
    case TO_RETURN:
      return cmpReturn((const ReturnStatement *)lhs, (const ReturnStatement *)rhs);
    case TO_YIELD:
    case TO_THROW:
      return cmpTerminate((const TerminateStatement *)lhs, (const TerminateStatement *)rhs);
    case TO_TRY:
      return cmpTry((const TryStatement *)lhs, (const TryStatement *)rhs);
    case TO_BREAK:
    case TO_CONTINUE:
    case TO_EMPTY:
      return true;
    case TO_EXPR_STMT:
      return cmpExprStmt((const ExprStatement *)lhs, (const ExprStatement *)rhs);

      //case TO_STATEMENT_MARK:
    case TO_ID:         return cmpId((const Id *)lhs, (const Id *)rhs);
    case TO_COMMA:      return cmpComma((const CommaExpr *)lhs, (const CommaExpr *)rhs);
    case TO_NULLC:
    case TO_ASSIGN:
    case TO_OROR:
    case TO_ANDAND:
    case TO_OR:
    case TO_XOR:
    case TO_AND:
    case TO_NE:
    case TO_EQ:
    case TO_3CMP:
    case TO_GE:
    case TO_GT:
    case TO_LE:
    case TO_LT:
    case TO_IN:
    case TO_INSTANCEOF:
    case TO_USHR:
    case TO_SHR:
    case TO_SHL:
    case TO_MUL:
    case TO_DIV:
    case TO_MOD:
    case TO_ADD:
    case TO_SUB:
    case TO_NEWSLOT:
    case TO_INEXPR_ASSIGN:
    case TO_PLUSEQ:
    case TO_MINUSEQ:
    case TO_MULEQ:
    case TO_DIVEQ:
    case TO_MODEQ:
      return cmpBinary((const BinExpr *)lhs, (const BinExpr *)rhs);
    case TO_NOT:
    case TO_BNOT:
    case TO_NEG:
    case TO_TYPEOF:
    case TO_RESUME:
    case TO_CLONE:
    case TO_PAREN:
    case TO_DELETE:
      return cmpUnary((const UnExpr *)lhs, (const UnExpr *)rhs);
    case TO_LITERAL:
      return cmpLiterals((const LiteralExpr *)lhs, (const LiteralExpr *)rhs);
    case TO_BASE:
    case TO_ROOT:
      return true;
    case TO_INC:
      return cmpIncExpr((const IncExpr *)lhs, (const IncExpr *)rhs);
    case TO_DECL_EXPR:
      return cmpDeclExpr((const DeclExpr *)lhs, (const DeclExpr *)rhs);
    case TO_ARRAYEXPR:
      return cmpArrayExpr((const ArrayExpr *)lhs, (const ArrayExpr *)rhs);
    case TO_GETFIELD:
      return cmpGetField((const GetFieldExpr *)lhs, (const GetFieldExpr *)rhs);
    case TO_SETFIELD:
      assert(0); return false;
    case TO_GETTABLE:
      return cmpGetTable((const GetTableExpr *)lhs, (const GetTableExpr *)rhs);
    case TO_SETTABLE:
      assert(0); return false;
    case TO_CALL:
      return cmpCallExpr((const CallExpr *)lhs, (const CallExpr *)rhs);
    case TO_TERNARY:
      return cmpTernary((const TerExpr *)lhs, (const TerExpr *)rhs);
      //case TO_EXPR_MARK:
    case TO_VAR:
      return cmpVarDecl((const VarDecl *)lhs, (const VarDecl *)rhs);
    case TO_PARAM:
      return cmpValueDecl((const ValueDecl *)lhs, (const ValueDecl *)rhs);
    case TO_CONST:
      return cmpConst((const ConstDecl *)lhs, (const ConstDecl *)rhs);
    case TO_DECL_GROUP:
      return cmpDeclGroup((const DeclGroup *)lhs, (const DeclGroup *)rhs);
    case TO_DESTRUCT:
      return cmpDestructDecl((const DestructuringDecl *)lhs, (const DestructuringDecl *)rhs);
    case TO_FUNCTION:
    case TO_CONSTRUCTOR:
      return cmpFunction((const FunctionDecl *)lhs, (const FunctionDecl *)rhs);
    case TO_CLASS:
      return cmpClass((const ClassDecl *)lhs, (const ClassDecl *)rhs);
    case TO_ENUM:
      return cmpEnumDecl((const EnumDecl *)lhs, (const EnumDecl *)rhs);
    case TO_TABLE:
      return cmpTable((const TableDecl *)lhs, (const TableDecl *)rhs);
    default:
      assert(0);
      return false;
    }
  }
};

enum ReturnTypeBits
{
  RT_NOTHING = 1 << 0,
  RT_NULL = 1 << 1,
  RT_BOOL = 1 << 2,
  RT_NUMBER = 1 << 3,
  RT_STRING = 1 << 4,
  RT_TABLE = 1 << 5,
  RT_ARRAY = 1 << 6,
  RT_CLOSURE = 1 << 7,
  RT_FUNCTION_CALL = 1 << 8,
  RT_UNRECOGNIZED = 1 << 9,
  RT_THROW = 1 << 10,
  RT_CLASS = 1 << 11,
};

class FunctionReturnTypeEvaluator {

  void checkLiteral(const LiteralExpr *l);
  void checkDeclaration(const DeclExpr *de);

  bool checkNode(const Statement *node);

  bool checkBlock(const Block *b);
  bool checkIf(const IfStatement *stmt);
  bool checkLoop(const LoopStatement *loop);
  bool checkSwitch(const SwitchStatement *swtch);
  bool checkReturn(const ReturnStatement *ret);
  bool checkTry(const TryStatement *trstmt);
  bool checkThrow(const ThrowStatement *thrw);

public:

  unsigned flags;

  unsigned compute(const Statement *n, bool &r) {
    flags = 0;
    r = checkNode(n);
    if (!r)
      flags |= RT_NOTHING;

    return flags;
  }
};

bool FunctionReturnTypeEvaluator::checkNode(const Statement *n) {
  switch (n->op())
  {
  case TO_RETURN: return checkReturn(static_cast<const ReturnStatement *>(n));
  case TO_THROW: return checkThrow(static_cast<const ThrowStatement *>(n));
  case TO_FOR: case TO_FOREACH: case TO_WHILE: case TO_DOWHILE:
    return checkLoop(static_cast<const LoopStatement *>(n));
  case TO_IF: return checkIf(static_cast<const IfStatement *>(n));
  case TO_SWITCH: return checkSwitch(static_cast<const SwitchStatement *>(n));
  case TO_BLOCK: return checkBlock(static_cast<const Block *>(n));
  case TO_TRY: return checkTry(static_cast<const TryStatement *>(n));
  default:
    return false;
  }
}

void FunctionReturnTypeEvaluator::checkLiteral(const LiteralExpr *lit) {
  switch (lit->kind())
  {
  case LK_STRING: flags |= RT_STRING; break;
  case LK_NULL: flags |= RT_NULL; break;
  case LK_BOOL: flags |= RT_BOOL; break;
  default: flags |= RT_NUMBER; break;
  }
}

void FunctionReturnTypeEvaluator::checkDeclaration(const DeclExpr *de) {
  const Decl *decl = de->declaration();

  switch (decl->op())
  {
  case TO_CLASS: flags |= RT_CLASS; break;
  case TO_FUNCTION: flags |= RT_CLOSURE; break;
  case TO_TABLE: flags |= RT_TABLE; break;
  default:
    break;
  }
}

bool FunctionReturnTypeEvaluator::checkReturn(const ReturnStatement *ret) {

  const Expr *arg = deparen(ret->argument());

  if (arg == nullptr) {
    flags |= RT_NOTHING;
    return true;
  }

  switch (arg->op())
  {
  case TO_LITERAL:
    checkLiteral(static_cast<const LiteralExpr *>(arg));
    break;
  case TO_OROR:
  case TO_ANDAND:
  case TO_NE:
  case TO_EQ:
  case TO_GE:
  case TO_GT:
  case TO_LE:
  case TO_LT:
  case TO_INSTANCEOF:
  case TO_IN:
  case TO_NOT:
    flags |= RT_BOOL;
    break;
  case TO_ADD:
  case TO_SUB:
  case TO_MUL:
  case TO_DIV:
  case TO_MOD:
  case TO_NEG:
  case TO_BNOT:
  case TO_3CMP:
  case TO_AND:
  case TO_OR:
  case TO_XOR:
  case TO_SHL:
  case TO_SHR:
  case TO_USHR:
  case TO_INC:
    flags |= RT_NUMBER;
    break;
  case TO_CALL:
    flags |= RT_FUNCTION_CALL;
    break;
  case TO_DECL_EXPR:
    checkDeclaration(static_cast<const DeclExpr *>(arg));
    break;
  case TO_ARRAYEXPR:
    flags |= RT_ARRAY;
    break;
  default:
    flags |= RT_UNRECOGNIZED;
    break;
  }

  return true;
}

bool FunctionReturnTypeEvaluator::checkThrow(const ThrowStatement *thrw) {
  flags |= RT_THROW;
  return true;
}

bool FunctionReturnTypeEvaluator::checkIf(const IfStatement *ifStmt) {
  bool retThen = checkNode(ifStmt->thenBranch());
  bool retElse = false;
  if (ifStmt->elseBranch()) {
    retElse = checkNode(ifStmt->elseBranch());
  }

  return retThen && retElse;
}

bool FunctionReturnTypeEvaluator::checkLoop(const LoopStatement *loop) {
  checkNode(loop->body());
  return false;
}

bool FunctionReturnTypeEvaluator::checkBlock(const Block *block) {
  bool allReturns = false;
  
  for (const Statement *stmt : block->statements()) {
    allReturns |= checkNode(stmt);
  }

  return allReturns;
}

bool FunctionReturnTypeEvaluator::checkSwitch(const SwitchStatement *swtch) {
  bool allReturns = true;

  for (auto &c : swtch->cases()) {
    allReturns &= checkNode(c.stmt);
  }

  if (swtch->defaultCase().stmt) {
    allReturns &= checkNode(swtch->defaultCase().stmt);
  }

  return allReturns;
}

bool FunctionReturnTypeEvaluator::checkTry(const TryStatement *stmt) {
  bool retTry = checkNode(stmt->tryStatement());
  bool retCatch = checkNode(stmt->catchStatement());
  return retTry && retCatch;
}

class PredicateCheckerVisitor : public Visitor {
  bool deepCheck;
  bool result;
  const Node *checkee;
protected:
  NodeEqualChecker equalChecker;

  PredicateCheckerVisitor(bool deep) : deepCheck(deep) {}

  virtual bool doCheck(const Node *checkee, Node *n) const = 0;

public:
  void visitNode(Node *n) {
    if (doCheck(checkee, n)) {
      result = true;
      return;
    }

    if (deepCheck)
      n->visitChildren(this);
  }

  bool check(const Node *toCheck, Node *tree) {
    result = false;
    checkee = toCheck;
    tree->visit(this);
    return result;
  }
};

class CheckModificationVisitor : public PredicateCheckerVisitor {
protected:
  bool doCheck(const Node *checkee, Node *n) const {
    enum TreeOp op = n->op();

    if (op == TO_ASSIGN || op == TO_INEXPR_ASSIGN || (TO_PLUSEQ <= op && op <= TO_MODEQ)) {
      BinExpr *bin = static_cast<BinExpr *>(n);
      return equalChecker.check(checkee, bin->lhs());
    }

    if (op == TO_INC) {
      IncExpr *inc = static_cast<IncExpr *>(n);
      return equalChecker.check(checkee, inc->argument());
    }

    return false;
  }
public:
  CheckModificationVisitor() : PredicateCheckerVisitor(false) {}
};

class ExistsChecker : public PredicateCheckerVisitor {
protected:

  bool doCheck(const Node *checkee, Node *n) const {
    return equalChecker.check(checkee, n);
  }

public:

  ExistsChecker() : PredicateCheckerVisitor(true) {}
};

class ModificationChecker : public Visitor {
  bool result;
public:

  void visitNode(Node *n) {
    enum TreeOp op = n->op();

    switch (op)
    {
    case TO_INC:
    case TO_ASSIGN:
    case TO_INEXPR_ASSIGN:
    case TO_PLUSEQ:
    case TO_MINUSEQ:
    case TO_MULEQ:
    case TO_DIVEQ:
    case TO_MODEQ:
      result = true;
      return;
    default:
      n->visitChildren(this);
      break;
    }
  }

  bool check(Node *n) {
    result = false;
    n->visit(this);
    return result;
  }
};

static bool isBinaryArith(const Expr *expr) {
  return TO_OROR <= expr->op() && expr->op() <= TO_SUB;
}

static bool isAssignExpr(const Expr *expr) {
  enum TreeOp op = expr->op();
  return op == TO_ASSIGN || op == TO_INEXPR_ASSIGN || (TO_PLUSEQ <= op && op <= TO_MODEQ);
}

class ConditionalExitFinder : public Visitor {
  bool _firstLevel;
public:
  bool hasBreak;
  bool hasContinue;
  bool hasReturn;
  bool hasThrow;
  ConditionalExitFinder(bool firstLevel)
    : _firstLevel(firstLevel)
    , hasBreak(false)
    , hasContinue(false)
    , hasReturn(false)
    , hasThrow(false) {}

  void visitReturnStatement(ReturnStatement *stmt) {
    if (!hasReturn)
      hasReturn = !_firstLevel;
  }

  void visitThrowStatement(ThrowStatement *stmt) {
    if (!hasThrow)
      hasThrow = !_firstLevel;
  }

  void visitBreakStatement(BreakStatement *stmt) {
    if (!hasBreak)
      hasBreak = !_firstLevel;
  }

  void visitContinueStatement(ContinueStatement *stmt) {
    if (!hasContinue)
      hasContinue = !_firstLevel;
  }

  void visitIfStatement(IfStatement *stmt) {
    bool old = _firstLevel;
    _firstLevel = true;
    Visitor::visitIfStatement(stmt);
    _firstLevel = old;
  }
};

static bool isSuspiciousNeighborOfNullCoalescing(enum TreeOp op) {
  return (op == TO_3CMP || op == TO_ANDAND || op == TO_OROR || op == TO_IN || /*op == TO_NOTIN ||*/ op == TO_EQ || op == TO_NE || op == TO_LE ||
    op == TO_LT || op == TO_GT || op == TO_GE || op == TO_NOT || op == TO_BNOT || op == TO_AND || op == TO_OR ||
    op == TO_XOR || op == TO_DIV || op == TO_MOD || op == TO_INSTANCEOF || /*op == TO_QMARK ||*/ op == TO_NEG ||
    op == TO_ADD || op == TO_MUL || op == TO_SHL || op == TO_SHR || op == TO_USHR);
}

static bool isSuspiciousTernaryConditionOp(enum TreeOp op) {
  return op == TO_ADD || op == TO_SUB || op == TO_MUL || op == TO_DIV || op == TO_MOD ||
    op == TO_AND || op == TO_OR || op == TO_SHL || op == TO_SHR || op == TO_USHR || op == TO_3CMP;
}

static bool isSuspiciousSameOperandsBinaryOp(enum TreeOp op) {
  return op == TO_EQ || op == TO_LE || op == TO_LT || op == TO_GE || op == TO_GT || op == TO_NE ||
    op == TO_ANDAND || op == TO_OROR || op == TO_SUB || op == TO_3CMP || op == TO_DIV || op == TO_MOD ||
    op == TO_OR || op == TO_AND || op == TO_XOR || op == TO_SHL || op == TO_SHR || op == TO_USHR;
}

static bool isBlockTerminatorStatement(enum TreeOp op) {
  return op == TO_RETURN || op == TO_THROW || op == TO_BREAK || op == TO_CONTINUE;
}

static bool isBooleanResultOperator(enum TreeOp op) {
  return op == TO_OROR || op == TO_ANDAND || op == TO_NE || op == TO_EQ || (TO_GE <= op && op <= TO_IN) || op == TO_NOT;
}

static bool isArithOperator(enum TreeOp op) {
  return op == TO_OROR || op == TO_ANDAND
    || (TO_3CMP <= op && op <= TO_LT)
    || (TO_USHR <= op && op <= TO_SUB)
    || (TO_PLUSEQ <= op && op <= TO_MODEQ)
    || op == TO_BNOT || op == TO_NEG || op == TO_INC;
}

static bool isDivOperator(enum TreeOp op) {
  return op == TO_DIV || op == TO_MOD || op == TO_DIVEQ || op == TO_MODEQ;
}

bool isPureArithOperator(enum TreeOp op) {
  return TO_USHR <= op && op <= TO_SUB || TO_PLUSEQ <= op && op <= TO_MODEQ;
}

bool isRelationOperator(enum TreeOp op) {
  return TO_3CMP <= op && op <= TO_LT;
}

bool isBitwiseOperator(enum TreeOp op) {
  return op == TO_OR || op == TO_AND || op == TO_XOR;
}

bool isBoolCompareOperator(enum TreeOp op) {
  return op == TO_NE || op == TO_EQ || TO_GE <= op && op <= TO_LT;
}

bool isCompareOperator(enum TreeOp op) {
  return TO_NE <= op && op <= TO_LT;
}

bool isShiftOperator(enum TreeOp op) {
  return op == TO_SHL || op == TO_SHR || op == TO_USHR;
}

bool isHigherShiftPriority(enum TreeOp op) {
  return TO_MUL <= op && op <= TO_SUB;
}

bool looksLikeBooleanExpr(const Expr *e) {
  if (isBooleanResultOperator(e->op()))
    return true;

  if (e->op() == TO_LITERAL) {
    return e->asLiteral()->kind() == LK_BOOL;
  }

  return false;
}

static const char *terminatorOpToName(enum TreeOp op) {
  switch (op)
  {
  case TO_BREAK: return "break";
  case TO_CONTINUE: return "continue";
  case TO_RETURN: return "return";
  case TO_THROW: return "throw";
  default:
    assert(0);
    return "<unkown terminator>";
  }
}

static const SQChar *function_can_return_string[] =
{
  "subst",
  "concat",
  "tostring",
  "toupper",
  "tolower",
  "slice",
  "trim",
  "join",
  "format",
  "replace",
  nullptr
};

static const SQChar *function_should_return_bool_prefix[] =
{
  "has",
  "Has",
  "have",
  "Have",
  "should",
  "Should",
  "need",
  "Need",
  "is",
  "Is",
  "was",
  "Was",
  "will",
  "Will",
  nullptr
};

static const SQChar *function_should_return_something_prefix[] =
{
  "get",
  "Get",
  nullptr
};

static const SQChar *function_result_must_be_utilized[] =
{
  "__merge",
  "indexof",
  "findindex",
  "findvalue",
  "len",
  "reduce",
  "tostring",
  "tointeger",
  "tofloat",
  "slice",
  "tolower",
  "toupper",
  nullptr
};

static const SQChar *function_can_return_null[] =
{
  "indexof",
  "findindex",
  "findvalue",
  nullptr
};

static bool hasPrefix(const SQChar *str, const SQChar *prefix, unsigned &length) {
  unsigned i = 0;

  for (;;) {
    SQChar c = str[i];
    SQChar p = prefix[i];

    if (!p) {
      length = i;
      return true;
    }

    if (!c) {
      return false;
    }

    if (c != p)
      return false;

    ++i;
  }
}

static bool hasAnyPrefix(const SQChar *str, const SQChar *prefixes[]) {
  for (int32_t i = 0; prefixes[i]; ++i) {
    unsigned length = 0;
    if (hasPrefix(str, prefixes[i], length)) {
      SQChar c = str[length];
      if (!c || c == '_' || c != tolower(c)) {
        return true;
      }
    }
  }

  return false;
}

static bool nameLooksLikeResultMustBeBoolean(const SQChar *funcName) {
  if (!funcName)
    return false;

  return hasAnyPrefix(funcName, function_should_return_bool_prefix);
}

static bool nameLooksLikeFunctionMustReturnResult(const SQChar *funcName) {
  if (!funcName)
    return false;

  bool nameInList = nameLooksLikeResultMustBeBoolean(funcName) ||
    hasAnyPrefix(funcName, function_should_return_something_prefix);

  if (!nameInList)
    if ((strstr(funcName, "_ctor") || strstr(funcName, "Ctor")) && strstr(funcName, "set") != funcName)
      nameInList = true;

  return nameInList;
}

static bool nameLooksLikeResultMustBeUtilised(const SQChar *name) {
  return hasAnyPrefix(name, function_result_must_be_utilized) || nameLooksLikeResultMustBeBoolean(name);
}

static bool nameLooksLikeResultMustBeString(const SQChar *name) {
  return hasAnyPrefix(name, function_can_return_string);
}

static bool canFunctionReturnNull(const SQChar *n) {
  return hasAnyPrefix(n, function_can_return_null);
}

static const SQChar rootName[] = "::";
static const SQChar baseName[] = "base";
static const SQChar thisName[] = "this";

enum ValueRefState {
  VRS_UNDEFINED,
  VRS_EXPRESSION,
  VRS_INITIALIZED,
  VRS_MULTIPLE,
  VRS_UNKNOWN,
  VRS_PARTIALLY,
  VRS_DECLARED,
  VRS_NUM_OF_STATES
};

enum SymbolKind {
  SK_EXCEPTION,
  SK_FUNCTION,
  SK_METHOD,
  SK_FIELD,
  SK_CLASS,
  SK_TABLE,
  SK_VAR,
  SK_BINDING,
  SK_CONST,
  SK_ENUM,
  SK_ENUM_CONST,
  SK_PARAM,
  SK_FOREACH
};

static const char *symbolContextName(enum SymbolKind k) {
  switch (k)
  {
  case SK_EXCEPTION: return "exception";
  case SK_FUNCTION: return "function";
  case SK_METHOD: return "method";
  case SK_FIELD: return "field";
  case SK_CLASS: return "class";
  case SK_TABLE: return "table";
  case SK_VAR: return "variable";
  case SK_BINDING: return "let";
  case SK_CONST: return "const";
  case SK_ENUM: return "enum";
  case SK_ENUM_CONST: return "enum const";
  case SK_PARAM: return "parameter";
  case SK_FOREACH: return "foreach var";
  default: return "<unknown>";
  }
}

enum ValueBoundKind {
  VBK_UNKNOWN,
  VBK_INTEGER,
  VRK_FLOAT
};

struct ValueBound {
  enum ValueBoundKind kind;

  union {
    SQInteger i;
    SQFloat f;
  } v;
};

struct FunctionInfo {

  FunctionInfo(const FunctionDecl *d, const FunctionDecl *o) : declaration(d), owner(o) {}

  ~FunctionInfo() = default;

  struct Modifiable {
    const FunctionDecl *owner;
    const SQChar *name;
  };

  const FunctionDecl *owner;
  std::vector<Modifiable> modifible;
  const FunctionDecl *declaration;
  std::vector<const SQChar *> parameters;

  void joinModifiable(const FunctionInfo *other);
  void addModifiable(const SQChar *name, const FunctionDecl *o);

};

void FunctionInfo::joinModifiable(const FunctionInfo *other) {
  for (auto &m : other->modifible) {
    if (owner == m.owner)
      continue;

    addModifiable(m.name, m.owner);
  }
}

void FunctionInfo::addModifiable(const SQChar *name, const FunctionDecl *o) {
  for (auto &m : modifible) {
    if (m.owner == o && strcmp(name, m.name) == 0)
      return;
  }

  modifible.push_back({ o, name });
}

struct VarScope;

static ValueRefState mergeMatrix[VRS_NUM_OF_STATES][VRS_NUM_OF_STATES] = {
 // VRS_UNDEFINED  VRS_EXPRESSION  VRS_INITIALIZED  VRS_MULTIPLE   VRS_UNKNOWN    VRS_PARTIALLY  VRS_DECLARED
  { VRS_UNDEFINED, VRS_PARTIALLY,  VRS_PARTIALLY,   VRS_PARTIALLY, VRS_UNKNOWN,   VRS_PARTIALLY, VRS_PARTIALLY }, // VRS_UNDEFINED
  { VRS_PARTIALLY, VRS_EXPRESSION, VRS_MULTIPLE,    VRS_MULTIPLE,  VRS_MULTIPLE,  VRS_PARTIALLY, VRS_PARTIALLY }, // VRS_EXPRESSION
  { VRS_PARTIALLY, VRS_MULTIPLE,   VRS_INITIALIZED, VRS_MULTIPLE,  VRS_MULTIPLE,  VRS_PARTIALLY, VRS_PARTIALLY }, // VRS_INITIALIZED
  { VRS_PARTIALLY, VRS_MULTIPLE,   VRS_MULTIPLE,    VRS_MULTIPLE,  VRS_PARTIALLY, VRS_PARTIALLY, VRS_MULTIPLE  }, // VRS_MULTIPLE
  { VRS_UNKNOWN,   VRS_PARTIALLY,  VRS_UNKNOWN,     VRS_PARTIALLY, VRS_UNKNOWN,   VRS_PARTIALLY, VRS_PARTIALLY }, // VRS_UNKNOWN
  { VRS_PARTIALLY, VRS_PARTIALLY,  VRS_PARTIALLY,   VRS_PARTIALLY, VRS_PARTIALLY, VRS_PARTIALLY, VRS_PARTIALLY }, // VRS_PARTIALLY
  { VRS_PARTIALLY, VRS_MULTIPLE,   VRS_INITIALIZED, VRS_MULTIPLE,  VRS_PARTIALLY, VRS_PARTIALLY, VRS_DECLARED  }  // VRS_DECLARED
};

struct SymbolInfo {
  union {
    const Id *x;
    const FunctionDecl *f;
    const ClassDecl *k;
    const TableMember *m;
    const VarDecl *v;
    const TableDecl *t;
    const ParamDecl *p;
    const EnumDecl *e;
    const ConstDecl *c;
    const EnumConst *ec;
  } declarator;

  enum SymbolKind kind;

  bool declared;
  bool used;
  bool usedAfterAssign;

  const struct VarScope *ownedScope;

  SymbolInfo(enum SymbolKind k) : kind(k) {
    declared = true;
    used = usedAfterAssign = false;
    ownedScope = nullptr;
  }

  bool isConstant() const {
    return kind != SK_VAR;
  }

  const Node *extractPointedNode() const {
    switch (kind)
    {
    case SK_EXCEPTION:
      return declarator.x;
    case SK_FUNCTION:
      return declarator.f;
    case SK_METHOD:
    case SK_FIELD:
      return declarator.m->key;
    case SK_CLASS:
      return declarator.k;
    case SK_TABLE:
      return declarator.t;
    case SK_VAR:
    case SK_BINDING:
    case SK_FOREACH:
      return declarator.v;
    case SK_CONST:
      return declarator.c;
    case SK_ENUM:
      return declarator.e;
    case SK_ENUM_CONST:
      return declarator.ec->val;
    case SK_PARAM:
      return declarator.p;
    default:
      assert(0);
      return nullptr;
    }
  }

  const char *contextName() const {
    return symbolContextName(kind);
  }
};

struct ValueRef {

  SymbolInfo *info;
  enum ValueRefState state;
  const Expr *expression;

  ValueRef(SymbolInfo *i) : info(i) {
    assigned = false;
    lastAssign = nullptr;
    lowerBound.kind = upperBound.kind = VBK_UNKNOWN;
    flagsPositive = flagsNegative = 0;
  }

  bool hasValue() const {
    return state == VRS_EXPRESSION || state == VRS_INITIALIZED;
  }

  bool isConstant() const {
    return info->isConstant();
  }

  bool assigned;
  const Expr *lastAssign;

  ValueBound lowerBound, upperBound;
  unsigned flagsPositive, flagsNegative;

  void kill(enum ValueRefState k = VRS_UNKNOWN) {
    if (!isConstant()) {
      state = k;
      expression = nullptr;
      flagsPositive = 0;
      flagsNegative = 0;
      lowerBound.kind = upperBound.kind = VBK_UNKNOWN;
    }
  }

  void merge(const ValueRef *other) {

    assert(info == other->info);

    assigned &= other->assigned;
    lastAssign = other->lastAssign;

    if (state != other->state) {
      enum ValueRefState k = mergeMatrix[other->state][state];
      kill(k);
      return;
    }

    if (isConstant()) {
      assert(other->isConstant());
      return;
    }

    if (!NodeEqualChecker().check(expression, other->expression)) {
      kill(VRS_MULTIPLE);
    }
  }
};

class CheckerVisitor;

struct VarScope {

  VarScope(const FunctionDecl *o, struct VarScope *p = nullptr) : owner(o), parent(p) {}

  ~VarScope() {
    if (parent)
      parent->~VarScope();
    symbols.clear();
  }

  const FunctionDecl *owner;
  struct VarScope *parent;
  std::unordered_map<const SQChar *, ValueRef *, StringHasher, StringEqualer> symbols;


  void merge(const VarScope *other);
  VarScope *copy(Arena *a, bool forClosure = false) const;
  void copyFrom(const VarScope *other);

  VarScope *findScope(const FunctionDecl *own);
  void checkUnusedSymbols(CheckerVisitor *v);
};

void VarScope::copyFrom(const VarScope *other) {
  VarScope *l = this;
  const VarScope *r = other;

  while (l) {
    assert(l->owner == r->owner && "Scope corruption");

    auto &thisSymbols = l->symbols;
    auto &otherSymbols = r->symbols;
    auto it = otherSymbols.begin();
    auto ie = otherSymbols.end();

    while (it != ie) {
      thisSymbols[it->first] = it->second;
      ++it;
    }

    l = l->parent;
    r = r->parent;
  }
}

void VarScope::merge(const VarScope *other) {
  VarScope *l = this;
  const VarScope *r = other;

  while (l) {
    assert(l->owner == r->owner && "Scope corruption");

    auto &thisSymbols = l->symbols;
    auto &otherSymbols = r->symbols;
    auto it = otherSymbols.begin();
    auto ie = otherSymbols.end();
    auto te = thisSymbols.end();

    while (it != ie) {
      auto f = thisSymbols.find(it->first);
      if (f != te) {
        f->second->merge(it->second);
      }
      else {
        it->second->kill(VRS_PARTIALLY);
        thisSymbols[it->first] = it->second;
      }
      ++it;
    }

    l = l->parent;
    r = r->parent;
  }
}

VarScope *VarScope::findScope(const FunctionDecl *own) {
  VarScope *s = this;

  while (s) {
    if (s->owner == own) {
      return s;
    }
    s = s->parent;
  }

  return nullptr;
}

VarScope *VarScope::copy(Arena *a, bool forClosure) const {
  VarScope *parentCopy = parent ? parent->copy(a, forClosure) : nullptr;
  void *mem = a->allocate(sizeof VarScope);
  VarScope *thisCopy = new(mem) VarScope(owner, parentCopy);

  for (auto &kv : symbols) {
    const SQChar *k = kv.first;
    ValueRef *v = kv.second;
    ValueRef *vcopy = nullptr;

    if (v->isConstant()) {
      vcopy = v;
    }
    else {
      void *mem = a->allocate(sizeof ValueRef);
      vcopy = new(mem) ValueRef(v->info);
      if (forClosure) {
        // if we analyse closure we cannot rely on existed assignable values
        vcopy->state = VRS_UNKNOWN;
        vcopy->expression = nullptr;
        vcopy->flagsNegative = vcopy->flagsPositive = 0;
        vcopy->lowerBound.kind = vcopy->upperBound.kind = VBK_UNKNOWN;
      }
      else {
        memcpy(vcopy, v, sizeof ValueRef);
      }
    }
    thisCopy->symbols[k] = vcopy;
  }

  return thisCopy;
}

class CheckerVisitor : public Visitor {
  friend struct VarScope;

  SQCompilationContext &_ctx;

  NodeEqualChecker _equalChecker;

  bool isUpperCaseIdentifier(const Expr *expr);

  void report(const Node *n, enum DiagnosticsId id, ...);

  void checkKeyNameMismatch(const SQChar *keyName, const Expr *expr);

  void checkAlwaysTrueOrFalse(const Node *expr);

  void checkForeachIteratorInClosure(const Id *id, const ValueRef *v);
  void checkIdUsed(const Id *id, const Node *p, ValueRef *v);

  void checkAndOrPriority(const BinExpr *expr);
  void checkBitwiseParenBool(const BinExpr *expr);
  void checkCoalescingPriority(const BinExpr *expr);
  void checkAssignmentToItself(const BinExpr *expr);
  void checkGlobalVarCreation(const BinExpr *expr);
  void checkSameOperands(const BinExpr *expr);
  void checkAlwaysTrueOrFalse(const BinExpr *expr);
  void checkDeclarationInArith(const BinExpr *expr);
  void checkIntDivByZero(const BinExpr *expr);
  void checkPotentiallyNullableOperands(const BinExpr *expr);
  void checkBitwiseToBool(const BinExpr *expr);
  void checkCompareWithBool(const BinExpr *expr);
  void checkCopyOfExpression(const BinExpr *expr);
  void checkConstInBoolExpr(const BinExpr *expr);
  void checkShiftPriority(const BinExpr *expr);
  void checkCanReturnNull(const BinExpr *expr);
  void checkCompareWithContainer(const BinExpr *expr);
  void checkBoolToStrangePosition(const BinExpr *expr);
  void checkNewSlotNameMatch(const BinExpr *expr);
  void checkPlusString(const BinExpr *expr);
  void checkAlwaysTrueOrFalse(const TerExpr *expr);
  void checkTernaryPriority(const TerExpr *expr);
  void checkSameValues(const TerExpr *expr);
  void checkExtendToAppend(const CallExpr *callExpr);
  void checkAlreadyRequired(const CallExpr *callExpr);
  void checkCallNullable(const CallExpr *callExpr);
  void checkArguments(const CallExpr *callExpr);
  void checkBoolIndex(const GetTableExpr *expr);
  void checkNullableIndex(const GetTableExpr *expr);

  bool findIfWithTheSameCondition(const Expr * condition, const IfStatement * elseNode) {
    if (_equalChecker.check(condition, elseNode->condition())) {
      return true;
    }

    const Statement *elseB = unwrapBody(elseNode->elseBranch());

    if (elseB && elseB->op() == TO_IF) {
      return findIfWithTheSameCondition(condition, static_cast<const IfStatement *>(elseB));
    }

    return false;
  }

  const SQChar *normalizeParamName(const SQChar *name, SQChar *buffer = nullptr);
  int32_t normalizeParamNameLength(const SQChar *n);

  void checkDuplicateSwitchCases(SwitchStatement *swtch);
  void checkDuplicateIfBranches(IfStatement *ifStmt);
  void checkDuplicateIfConditions(IfStatement *ifStmt);

  bool onlyEmptyStatements(int32_t start, const ArenaVector<Statement *> &statements) {
    for (int32_t i = start; i < statements.size(); ++i) {
      Statement *stmt = statements[i];
      if (stmt->op() != TO_EMPTY)
        return false;
    }

    return true;
  }

  bool existsInTree(Expr *toSearch, Expr *tree) const {
    return ExistsChecker().check(toSearch, tree);
  }

  bool indexChangedInTree(Expr *index) const {
    return ModificationChecker().check(index);
  }

  bool checkModification(Expr *key, Node *tree) {
    return CheckModificationVisitor().check(key, tree);
  }

  void checkUnterminatedLoop(LoopStatement *loop);
  void checkVariableMismatchForLoop(ForStatement *loop);
  void checkEmptyWhileBody(WhileStatement *loop);
  void checkEmptyThenBody(IfStatement *stmt);
  void checkForgottenDo(const Block *block);
  void checkUnreachableCode(const Block *block);
  void checkAssigneTwice(const Block *b);
  void checkUnutilizedResult(const ExprStatement *b);
  void checkNullableContainer(const ForeachStatement *loop);
  void checkMissedBreak(const SwitchStatement *swtch);

  const SQChar *findSlotNameInStack(const Decl *);
  void checkFunctionReturns(FunctionDecl *func);

  void checkAccessNullable(const DestructuringDecl *d);
  void checkAccessNullable(const AccessExpr *acc);
  void checkEnumConstUsage(const GetFieldExpr *acc);

  enum StackSlotType {
    SST_NODE,
    SST_TABLE_MEMBER
  };

  struct StackSlot {
    enum StackSlotType sst;
    union {
      const Node *n;
      const struct TableMember *member;
    };
  };

  std::vector<StackSlot> nodeStack;

  struct VarScope *currentScope;

  FunctionInfo *makeFunctionInfo(const FunctionDecl *d, const FunctionDecl *o) {
    void *mem = arena->allocate(sizeof FunctionInfo);
    return new(mem) FunctionInfo(d, o);
  }

  ValueRef *makeValueRef(SymbolInfo *info) {
    void *mem = arena->allocate(sizeof ValueRef);
    return new (mem) ValueRef(info);
  }

  SymbolInfo *makeSymbolInfo(enum SymbolKind kind) {
    void *mem = arena->allocate(sizeof SymbolInfo);
    return new (mem) SymbolInfo(kind);
  }

  std::unordered_map<const FunctionDecl *, FunctionInfo *> functionInfoMap;

  std::unordered_set<const SQChar *, StringHasher, StringEqualer> requriedModules;

  Arena *arena;

  FunctionInfo *currectInfo;

  void declareSymbol(const SQChar *name, ValueRef *v);
  void pushFunctionScope(VarScope *functionScope, const FunctionDecl *decl);

  void applyCallToScope(const CallExpr *call);
  void applyBinaryToScope(const BinExpr *bin);
  void applyAssignmentToScope(const BinExpr *bin);
  void applyAssignEqToScope(const BinExpr *bin);

  int32_t computeNameLength(const GetFieldExpr *access);
  void computeNameRef(const GetFieldExpr *access, SQChar *b, int32_t &ptr, int32_t size);
  int32_t computeNameLength(const Expr *e);
  void computeNameRef(const Expr *lhs, SQChar *buffer, int32_t &ptr, int32_t size);
  const SQChar *computeNameRef(const Expr *lhs);

  ValueRef *findValueInScopes(const SQChar *ref);
  void applyKnownInvocationToScope(const ValueRef *ref);
  void applyUnknownInvocationToScope();

  const FunctionInfo *findFunctionInfo(const Expr *e, bool &isCtor);

  void setExpression(const Expr *lvalue, const Expr *rvalue, unsigned pf = 0, unsigned nf = 0);
  const ValueRef *findValueForExpr(const Expr *e);
  const Expr *maybeEval(const Expr *e);

  const SQChar *findFieldName(const Expr *e);

  bool isPotentiallyNullable(const Expr *e);
  bool couldBeString(const Expr *e);

  void visitBinaryBranches(const BinExpr *expr, const Expr *v, unsigned pf, unsigned nf);

  LiteralExpr trueValue, falseValue, nullValue;

public:

  CheckerVisitor(SQCompilationContext &ctx)
    : _ctx(ctx)
    , arena(ctx.arena())
    , currectInfo(nullptr)
    , currentScope(nullptr)
    , trueValue(true)
    , falseValue(false)
    , nullValue() {}

  ~CheckerVisitor();

  void visitNode(Node *n);


  void visitId(Id *id);
  void visitBinExpr(BinExpr *expr);
  void visitTerExpr(TerExpr *expr);
  void visitCallExpr(CallExpr *expr);
  void visitGetFieldExpr(GetFieldExpr *expr);
  void visitGetTableExpr(GetTableExpr *expr);

  void visitExprStatement(ExprStatement *stmt);

  void visitBlock(Block *b);
  void visitForStatement(ForStatement *loop);
  void visitForeachStatement(ForeachStatement *loop);
  void visitWhileStatement(WhileStatement *loop);
  void visitDoWhileStatement(DoWhileStatement *loop);
  void visitIfStatement(IfStatement *ifstmt);

  void visitSwitchStatement(SwitchStatement *swtch);

  void visitTryStatement(TryStatement *tryStmt);

  void visitFunctionDecl(FunctionDecl *func);

  void visitTableDecl(TableDecl *table);

  void visitParamDecl(ParamDecl *p);
  void visitVarDecl(VarDecl *decl);
  void visitConstDecl(ConstDecl *cnst);
  void visitEnumDecl(EnumDecl *enm);
  void visitDesctructingDecl(DestructuringDecl *decl);

  void analyse(RootBlock *root);
};

void VarScope::checkUnusedSymbols(CheckerVisitor *checker) {

  for (auto &s : symbols) {
    const SQChar *n = s.first;
    const ValueRef *v = s.second;

    if (strcmp(n, thisName) == 0) // skip 'this'
      continue;

    if (n[0] == '_')
      continue;

    SymbolInfo *info = v->info;

    if (!info->used) {
      checker->report(info->extractPointedNode(), DiagnosticsId::DI_DECLARED_NEVER_USED, info->contextName(), n);
      // TODO: add hint for param/exception name about silencing it with '_' prefix
    }
  }
}

CheckerVisitor::~CheckerVisitor() {
  for (auto &p : functionInfoMap) {
    p.second->~FunctionInfo();
  }
}

void CheckerVisitor::visitNode(Node *n) {
  nodeStack.push_back({ SST_NODE, n });
  Visitor::visitNode(n);
  nodeStack.pop_back();
}

void CheckerVisitor::report(const Node *n, enum DiagnosticsId id, ...) {
  va_list vargs;
  va_start(vargs, id);

  _ctx.vreportDiagnostic(id, n->lineStart(), n->columnStart(), n->columnEnd() - n->columnStart(), vargs);

  va_end(vargs);
}

bool CheckerVisitor::isUpperCaseIdentifier(const Expr *e) {

  const char *id = nullptr;

  if (e->op() == TO_GETFIELD) {
    id = e->asGetField()->fieldName();
  }
  else if (e->op() == TO_GETTABLE) {
    e = e->asGetTable()->key();
  }

  if (e->op() == TO_ID) {
    id = e->asId()->id();
  }
  else if (e->op() == TO_LITERAL && e->asLiteral()->kind() == LK_STRING) {
    id = e->asLiteral()->s();
  }

  if (!id)
    return false;

  while (*id) {
    if (*id >= 'a' && *id <= 'z')
      return false;
    ++id;
  }

  return true;
}

void CheckerVisitor::checkForeachIteratorInClosure(const Id *id, const ValueRef *v) {
  if (v->info->kind != SK_FOREACH)
    return;

  if (v->info->ownedScope->owner != currentScope->owner) {
    report(id, DiagnosticsId::DI_ITER_IN_CLOSURE, id->id());
  }
}

void CheckerVisitor::checkIdUsed(const Id *id, const Node *p, ValueRef *v) {
  const Expr *e = nullptr;
  if (p->op() == TO_EXPR_STMT)
    e = static_cast<const ExprStatement *>(p)->expression();
  else if (p->isExpression())
    e = p->asExpression();

  bool assgned = v->assigned;

  if (e && isAssignExpr(e)) {
    const BinExpr *bin = static_cast<const BinExpr *>(e);
    const Expr *lhs = bin->lhs();
    const Expr *rhs = bin->rhs();
    if (id == lhs) {
      if (!v->info->usedAfterAssign && assgned) {
        report(bin, DiagnosticsId::DI_REASSIGN_WITH_NO_USAGE);
      }
      v->assigned = true;
      v->lastAssign = bin;
    }
    if (e->op() == TO_ASSIGN || e->op() == TO_INEXPR_ASSIGN)
      return;
  }

  // it's usage
  v->info->used = true;
  if (assgned) {
    v->info->usedAfterAssign = true;
    v->assigned = e ? TO_PLUSEQ <= e->op() && e->op() <= TO_MODEQ : false;
  }

  if (v->state == VRS_PARTIALLY) {
    report(id, DiagnosticsId::DI_POSSIBLE_GARGABE, id->id());
  }
  else if (v->state == VRS_UNDEFINED) {
    report(id, DiagnosticsId::DI_UNITNIALIZED_VAR);
  }
}

void CheckerVisitor::checkAndOrPriority(const BinExpr *expr) {
  const Expr *l = expr->lhs();
  const Expr *r = expr->rhs();

  if (expr->op() == TO_OROR) {
    if (l->op() == TO_ANDAND || r->op() == TO_ANDAND) {
      report(expr, DiagnosticsId::DI_AND_OR_PAREN);
    }
  }
}

void CheckerVisitor::checkBitwiseParenBool(const BinExpr *expr) {
  const Expr *l = expr->lhs();
  const Expr *r = expr->rhs();

  if (expr->op() == TO_OROR || expr->op() == TO_ANDAND) {
    if (l->op() == TO_AND || l->op() == TO_OR || r->op() == TO_AND || r->op() == TO_OR) {
      report(expr, DiagnosticsId::DI_BITWISE_BOOL_PAREN);
    }
  }
}

void CheckerVisitor::checkCoalescingPriority(const BinExpr *expr) {
  const Expr *l = expr->lhs();
  const Expr *r = expr->rhs();

  if (expr->op() == TO_NULLC) {
    if (isSuspiciousNeighborOfNullCoalescing(l->op())) {
      report(l, DiagnosticsId::DI_NULL_COALSESSING_PRIOR, treeopStr(l->op()));
    }

    if (isSuspiciousNeighborOfNullCoalescing(r->op())) {
      report(r, DiagnosticsId::DI_NULL_COALSESSING_PRIOR, treeopStr(r->op()));
    }
  }
}

void CheckerVisitor::checkAssignmentToItself(const BinExpr *expr) {
  const Expr *l = expr->lhs();
  const Expr *r = expr->rhs();

  if (expr->op() == TO_ASSIGN) {
    auto *ll = deparen(l);
    auto *rr = deparen(r);

    if (_equalChecker.check(ll, rr)) {
      report(expr, DiagnosticsId::DI_ASG_TO_ITSELF);
    }
  }
}

void CheckerVisitor::checkGlobalVarCreation(const BinExpr *expr) {
  const Expr *l = expr->lhs();

  if (expr->op() == TO_NEWSLOT) {
    if (l->op() == TO_ID) {
      report(expr, DiagnosticsId::DI_GLOBAL_VAR_CREATE);
    }
  }
}

void CheckerVisitor::checkSameOperands(const BinExpr *expr) {
  const Expr *l = expr->lhs();
  const Expr *r = expr->rhs();

  if (isSuspiciousSameOperandsBinaryOp(expr->op())) {
    const Expr *ll = deparen(l);
    const Expr *rr = deparen(r);

    if (_equalChecker.check(ll, rr)) {
      if (ll->op() != TO_LITERAL || (ll->asLiteral()->kind() != LK_FLOAT && ll->asLiteral()->kind() != LK_INT)) {
        report(expr, DiagnosticsId::DI_SAME_OPERANDS, treeopStr(expr->op()));
      }
    }
  }
}

void CheckerVisitor::checkAlwaysTrueOrFalse(const Node *n) {
  const Node *cond = n->isExpression() ? maybeEval(n->asExpression()) : n;

  if (cond->op() == TO_LITERAL) {
    const LiteralExpr *l = cond->asExpression()->asLiteral();
    report(n, DiagnosticsId::DI_ALWAYS_T_OR_F, l->raw() ? "true" : "false");
  }
  else if (cond->op() == TO_ARRAYEXPR || cond->op() == TO_DECL_EXPR || cond->isDeclaration()) {
    report(n, DiagnosticsId::DI_ALWAYS_T_OR_F, "true");
  }
}

void CheckerVisitor::checkAlwaysTrueOrFalse(const TerExpr *expr) {
  checkAlwaysTrueOrFalse(expr->a());
}

void CheckerVisitor::checkTernaryPriority(const TerExpr *expr) {
  const Expr *cond = expr->a();
  const Expr *thenExpr = expr->b();
  const Expr *elseExpr = expr->c();

  if (isSuspiciousTernaryConditionOp(cond->op())) {
    const BinExpr *binCond = static_cast<const BinExpr *>(cond);
    const Expr *l = binCond->lhs();
    const Expr *r = binCond->rhs();
    bool isIgnore = cond->op() == TO_AND
      && (isUpperCaseIdentifier(l) || isUpperCaseIdentifier(r)
        || (r->op() == TO_LITERAL && r->asLiteral()->kind() == LK_INT));

    if (!isIgnore) {
      report(cond, DiagnosticsId::DI_TERNARY_PRIOR, treeopStr(cond->op()));
    }
  }
}

void CheckerVisitor::checkSameValues(const TerExpr *expr) {
  const Expr *ifTrue = maybeEval(expr->b())->asExpression();
  const Expr *ifFalse = maybeEval(expr->c())->asExpression();

  if (_equalChecker.check(ifTrue, ifFalse)) {
    report(expr, DiagnosticsId::DI_TERNARY_SAME_VALUES);
  }
}

void CheckerVisitor::checkAlwaysTrueOrFalse(const BinExpr *bin) {
  enum TreeOp op = bin->op();
  if (op != TO_ANDAND && op != TO_OROR)
    return;

  const Expr *lhs = maybeEval(bin->lhs())->asExpression();
  const Expr *rhs = maybeEval(bin->rhs())->asExpression();

  enum TreeOp cmpOp = lhs->op();
  if (cmpOp == rhs->op() && (cmpOp == TO_NE || cmpOp == TO_EQ)) {
    const char *constValue = nullptr;

    if (op == TO_ANDAND && cmpOp == TO_EQ)
      constValue = "false";

    if (op == TO_OROR && cmpOp == TO_NE)
      constValue = "true";

    if (!constValue)
      return;

    const BinExpr *lhsBin = static_cast<const BinExpr *>(lhs);
    const BinExpr *rhsBin = static_cast<const BinExpr *>(rhs);

    const Expr *lconst = lhsBin->rhs();
    const Expr *rconst = rhsBin->rhs();

    if (lconst->op() == TO_LITERAL || isUpperCaseIdentifier(lconst)) {
      if (rconst->op() == TO_LITERAL || isUpperCaseIdentifier(rconst)) {
        if (_equalChecker.check(lhsBin->lhs(), rhsBin->lhs())) {
          if (!_equalChecker.check(lconst, rconst)) {
            report(bin, DiagnosticsId::DI_ALWAYS_T_OR_F, constValue);
          }
        }
      }
    }
  }
  else if (lhs->op() == TO_NOT || rhs->op() == TO_NOT && (lhs->op() != rhs->op())) {
    const char *v = op == TO_OROR ? "true" : "false";
    if (lhs->op() == TO_NOT) {
      const UnExpr *u = static_cast<const UnExpr *>(lhs);
      if (_equalChecker.check(u->argument(), rhs)) {
        report(bin, DiagnosticsId::DI_ALWAYS_T_OR_F, v);
      }
    }
    if (rhs->op() == TO_NOT) {
      const UnExpr *u = static_cast<const UnExpr *>(rhs);
      if (_equalChecker.check(lhs, u->argument())) {
        report(bin, DiagnosticsId::DI_ALWAYS_T_OR_F, v);
      }
    }
  }
}

void CheckerVisitor::checkDeclarationInArith(const BinExpr *bin) {
  if (isArithOperator(bin->op())) {
    const Expr *lhs = maybeEval(bin->lhs());
    const Expr *rhs = maybeEval(bin->rhs());

    if (lhs->op() == TO_DECL_EXPR || lhs->op() == TO_ARRAYEXPR) {
      report(bin->lhs(), DiagnosticsId::DI_DECL_IN_EXPR);
    }

    if (rhs->op() == TO_DECL_EXPR || rhs->op() == TO_ARRAYEXPR) {
      report(bin->rhs(), DiagnosticsId::DI_DECL_IN_EXPR);
    }
  }
}

void CheckerVisitor::checkIntDivByZero(const BinExpr *bin) {
  if (isDivOperator(bin->op())) {
    const Expr *divided = maybeEval(bin->lhs());
    const Expr *divisor = maybeEval(bin->rhs());

    if (divisor->op() == TO_LITERAL) {
      const LiteralExpr *rhs = divisor->asLiteral();
      if (rhs->raw() == 0) {
        report(bin, DiagnosticsId::DI_DIV_BY_ZERO);
        return;
      }
      if (divided->op() == TO_LITERAL) {
        const LiteralExpr *lhs = divided->asLiteral();
        if (lhs->kind() == LK_INT && rhs->kind() == LK_INT) {
          if (rhs->i() == -1 && lhs->i() == MIN_SQ_INTEGER) {
            report(bin, DiagnosticsId::DI_INTEGER_OVERFLOW);
          }
          else if (lhs->i() % rhs->i() != 0) {
            report(bin, DiagnosticsId::DI_ROUND_TO_INT);
          }
        }
      }
    }
  }
}

void CheckerVisitor::checkPotentiallyNullableOperands(const BinExpr *bin) {

  bool isRelOp = isRelationOperator(bin->op());
  bool isArithOp = isPureArithOperator(bin->op());
  bool isAssign = bin->op() == TO_ASSIGN || bin->op() == TO_INEXPR_ASSIGN || bin->op() == TO_NEWSLOT;

  if (!isRelOp && !isArithOp && !isAssign)
    return;

  const Expr *lhs = skipUnary(maybeEval(skipUnary(bin->lhs())));
  const Expr *rhs = skipUnary(maybeEval(skipUnary(bin->rhs())));

  const char *opType = isRelOp ? "Comparison operation" : "Arithmetic operation";

  if (isPotentiallyNullable(lhs)) {
    if (isAssign) {
      if (lhs->op() != TO_ID) {
        report(bin->lhs(), DiagnosticsId::DI_NULLABLE_ASSIGNMENT);
      }
    }
    else {
      report(bin->lhs(), DiagnosticsId::DI_NULLABLE_OPERANDS, opType);
    }
  }

  if (isPotentiallyNullable(rhs) && !isAssign) {
    report(bin->rhs(), DiagnosticsId::DI_NULLABLE_OPERANDS, opType);
  }
}

void CheckerVisitor::checkBitwiseToBool(const BinExpr *bin) {
  if (!isBitwiseOperator(bin->op()))
    return;

  const Expr *lhs = maybeEval(bin->lhs());
  const Expr *rhs = maybeEval(bin->rhs());

  if (looksLikeBooleanExpr(lhs) || looksLikeBooleanExpr(rhs)) {
    report(bin, DiagnosticsId::DI_BITWISE_OP_TO_BOOL);
  }

}

void CheckerVisitor::checkCompareWithBool(const BinExpr *expr) {
  const Expr *l = expr->lhs();
  const Expr *r = expr->rhs();

  if (isBoolCompareOperator(expr->op()) && l->op() != TO_PAREN && r->op() != TO_PAREN) {
    const Expr *lhs = maybeEval(l);
    const Expr *rhs = maybeEval(r);

    if (isBoolCompareOperator(lhs->op()) || isBoolCompareOperator(rhs->op())) {
      bool warn = true;

      if (expr->op() == TO_EQ || expr->op() == TO_NE) {

        //function_should_return_bool_prefix

        if (nameLooksLikeResultMustBeBoolean(findFieldName(l)) ||
          nameLooksLikeResultMustBeBoolean(findFieldName(r)) ||
          nameLooksLikeResultMustBeBoolean(findFieldName(rhs)) ||
          nameLooksLikeResultMustBeBoolean(findFieldName(lhs))) {
          warn = false;
        }

        if (l->op() == TO_ID && r->op() == TO_ID) {
          warn = false;
        }
      }

      if (warn) {
        report(expr, DiagnosticsId::DI_COMPARE_WITH_BOOL);
      }
    }
  }
}

void CheckerVisitor::checkCopyOfExpression(const BinExpr *bin) {
  enum TreeOp op = bin->op();
  if (op != TO_OROR && op != TO_ANDAND && op != TO_OR)
    return;

  const Expr *lhs = bin->lhs();
  const Expr *cmp = bin->rhs();

  while (cmp->op() == op) {
    const BinExpr *binCmp = static_cast<const BinExpr *>(cmp);

    if (_equalChecker.check(lhs, binCmp->lhs()) || _equalChecker.check(lhs, binCmp->rhs())) {
      report(binCmp, DiagnosticsId::DI_COPY_OF_EXPR);
    }
    cmp = binCmp->rhs();
  }
}

void CheckerVisitor::checkConstInBoolExpr(const BinExpr *bin) {
  if (bin->op() != TO_OROR && bin->op() != TO_ANDAND)
    return;

  const Expr *lhs = skipUnary(maybeEval(skipUnary(bin->lhs())));
  const Expr *rhs = skipUnary(maybeEval(skipUnary(bin->rhs())));

  bool leftIsConst = lhs->op() == TO_LITERAL || lhs->op() == TO_DECL_EXPR || lhs->op() == TO_ARRAYEXPR || isUpperCaseIdentifier(lhs);
  bool rightIsConst = rhs->op() == TO_LITERAL || rhs->op() == TO_DECL_EXPR || rhs->op() == TO_ARRAYEXPR || isUpperCaseIdentifier(rhs);

  if (rightIsConst && bin->op() == TO_OROR) {
    if (rhs->op() != TO_LITERAL || rhs->asLiteral()->kind() != LK_BOOL || rhs->asLiteral()->b() != true) {
      rightIsConst = false;
    }
  }

  if (leftIsConst || rightIsConst) {
    report(bin, DiagnosticsId::DI_CONST_IN_BOOL_EXPR);
  }
}

void CheckerVisitor::checkShiftPriority(const BinExpr *bin) {
  if (isShiftOperator(bin->op())) {
    if (isHigherShiftPriority(bin->lhs()->op()) || isHigherShiftPriority(bin->rhs()->op())) {
      report(bin, DiagnosticsId::DI_SHIFT_PRIORITY);
    }
  }
}

void CheckerVisitor::checkCanReturnNull(const BinExpr *bin) {
  if (!isPureArithOperator(bin->op()) && !isRelationOperator(bin->op())) {
    return;
  }

  const Expr *l = skipUnary(maybeEval(skipUnary(bin->lhs())));
  const Expr *r = skipUnary(maybeEval(skipUnary(bin->rhs())));
  const CallExpr *c = nullptr;

  if (l->op() == TO_CALL)
    c = static_cast<const CallExpr *>(l);
  else if (r->op() == TO_CALL)
    c = static_cast<const CallExpr *>(r);

  if (!c)
    return;

  const Expr *callee = c->callee();
  bool isCtor = false;
  const FunctionInfo *info = findFunctionInfo(callee, isCtor);

  if (isCtor)
    return;

  const SQChar *funcName = nullptr;

  if (info) {
    funcName = info->declaration->name();
  } else if (callee->op() == TO_ID) {
    funcName = callee->asId()->id();
  }
  else if (callee->op() == TO_GETFIELD) {
    funcName = callee->asGetField()->fieldName();
  }

  if (funcName) {
    if (canFunctionReturnNull(funcName)) {
      report(c, DiagnosticsId::DI_FUNC_CAN_RET_NULL, funcName);
    }
  }
}

void CheckerVisitor::checkCompareWithContainer(const BinExpr *bin) {
  if (!isCompareOperator(bin->op()))
    return;

  const Expr *l = maybeEval(bin->lhs());
  const Expr *r = maybeEval(bin->rhs());

  if (l->op() == TO_ARRAYEXPR || r->op() == TO_ARRAYEXPR) {
    report(bin, DiagnosticsId::DI_CMP_WITH_CONTAINER, "array");
  }

  if (l->op() == TO_DECL_EXPR || r->op() == TO_DECL_EXPR) {
    report(bin, DiagnosticsId::DI_CMP_WITH_CONTAINER, "declaration");
  }
}

void CheckerVisitor::checkExtendToAppend(const CallExpr *expr) {
  const Expr *callee = expr->callee();
  const auto &args = expr->arguments();

  if (callee->op() == TO_GETFIELD) {
    if (args.size() > 0) {
      Expr *arg0 = args[0];
      if (arg0->op() == TO_ARRAYEXPR) {
        if (strcmp(callee->asGetField()->fieldName(), "extend") == 0) {
          report(expr, DiagnosticsId::DI_EXTEND_TO_APPEND);
        }
      }
    }
  }
}

void CheckerVisitor::checkBoolToStrangePosition(const BinExpr *bin) {
  if (bin->op() != TO_IN && bin->op() != TO_INSTANCEOF)
    return;

  const Expr *lhs = maybeEval(bin->lhs());
  const Expr *rhs = maybeEval(bin->rhs());

  if (looksLikeBooleanExpr(lhs) && bin->op() == TO_IN) {
    report(bin, DiagnosticsId::DI_BOOL_PASSED_TO_STRANGE, "in");
  }

  if (looksLikeBooleanExpr(rhs)) {
    report(bin, DiagnosticsId::DI_BOOL_PASSED_TO_STRANGE, bin->op() == TO_IN ? "in" : "instanceof");
  }
}

void CheckerVisitor::checkKeyNameMismatch(const SQChar *fieldName, const Expr *e) {

  if (!fieldName)
    return;

  if (e->op() != TO_DECL_EXPR)
    return;

  const Decl *decl = static_cast<const DeclExpr *>(e)->declaration();

  const SQChar *declName = nullptr;

  if (decl->op() == TO_FUNCTION) {
    const FunctionDecl *f = static_cast<const FunctionDecl *>(decl);
    if (!f->isLambda() && f->name()[0] != '(') {
      declName = f->name();
    }
  }

  if (declName) {
    if (strcmp(fieldName, declName)) {
      report(e, DiagnosticsId::DI_KEY_NAME_MISMATCH, fieldName, declName);
    }
  }
}

void CheckerVisitor::checkNewSlotNameMatch(const BinExpr *bin) {
  if (bin->op() != TO_NEWSLOT)
    return;

  const Expr *lhs = bin->lhs();
  const Expr *rhs = bin->rhs();

  if (lhs->op() == TO_GETFIELD)
    checkKeyNameMismatch(lhs->asGetField()->fieldName(), rhs);
}

void CheckerVisitor::checkPlusString(const BinExpr *bin) {
  if (bin->op() != TO_ADD && bin->op() != TO_PLUSEQ)
    return;

  const Expr *l = maybeEval(bin->lhs());
  const Expr *r = maybeEval(bin->rhs());

  bool sl = couldBeString(l);
  bool sr = couldBeString(r);

  if (sl && !sr || !sl && sr) {
    report(bin, DiagnosticsId::DI_PLUS_STRING);
  }
}

void CheckerVisitor::checkAlreadyRequired(const CallExpr *call) {
  if (call->arguments().size() != 1)
    return;

  const Expr *arg = maybeEval(call->arguments()[0]);

  if (arg->op() != TO_LITERAL)
    return;

  const LiteralExpr *l = arg->asLiteral();
  if (l->kind() != LK_STRING)
    return;


  const Expr *callee = call->callee();
  bool isCtor = false;
  const FunctionInfo *info = findFunctionInfo(callee, isCtor);

  if (isCtor)
    return;

  const SQChar *name = nullptr;

  if (info) {
    name = info->declaration->name();
  }
  else if (callee->op() == TO_ID) {
    name = callee->asId()->id();
  }
  else if (callee->op() == TO_GETFIELD) {
    const GetFieldExpr *g = callee->asGetField();
    if (g->receiver()->op() == TO_ROOT) {
      name = g->fieldName();
    }
  }

  if (!name)
    return;

  if (strcmp(name, "require"))
    return;

  const SQChar *moduleName = l->s();

  if (requriedModules.find(moduleName) != requriedModules.end()) {
    report(call, DiagnosticsId::DI_ALREADY_REQUIRED, moduleName);
  }
  else {
    requriedModules.insert(moduleName);
  }
}

void CheckerVisitor::checkCallNullable(const CallExpr *call) {
  const Expr *c = call->callee();

  if (isPotentiallyNullable(c) && !call->isNullable()) {
    report(call, DiagnosticsId::DI_ACCESS_POT_NULLABLE, c->op() == TO_ID ? c->asId()->id() : "expression", "function");
  }
}

int32_t CheckerVisitor::normalizeParamNameLength(const SQChar *name) {
  int32_t r = 0;

  while (*name) {
    if (*name != '_')
      ++r;
    ++name;
  }

  return r;
}

const SQChar *CheckerVisitor::normalizeParamName(const SQChar *name, SQChar *buffer) {

  if (!buffer) {
    int32_t nl = normalizeParamNameLength(name);
    buffer = (SQChar *)arena->allocate(nl + 1);
  }

  int32_t i = 0, j = 0;
  while (name[i]) {
    SQChar c = name[i++];
    if (c != '_') {
      buffer[j++] = std::tolower(c);
    }
  }

  buffer[j] = '\0';

  return buffer;
}

void CheckerVisitor::checkArguments(const CallExpr *callExpr) {
  bool dummy;
  const FunctionInfo *info = findFunctionInfo(callExpr->callee(), dummy);

  if (!info)
    return;

  const auto& params = info->parameters;
  const auto& args = callExpr->arguments();
  const FunctionDecl *decl = info->declaration;

  const size_t efferctiveParamSize = decl->isVararg() ? params.size() - 2 : params.size() - 1;

  if (efferctiveParamSize != args.size()) {
    if (!decl->isVararg() || args.size() <= params.size()) {
      report(callExpr, DiagnosticsId::DI_PARAM_COUNT_MISMATCH, decl->name());
    }
  }

  for (int i = 1; i < params.size(); ++i) {
    const SQChar *paramName = params[i];
    for (int j = 0; j < args.size(); ++j) {
      const Expr *arg = args[j];
      const SQChar *possibleArgName = nullptr;

      if (arg->op() == TO_ID)
        possibleArgName = arg->asId()->id();
      else if (arg->op() == TO_GETFIELD)
        possibleArgName = arg->asGetField()->fieldName();

      if (!possibleArgName)
        continue;

      int32_t argNL = normalizeParamNameLength(possibleArgName);
      SQChar *buffer = (SQChar *)malloc(argNL + 1);
      normalizeParamName(possibleArgName, buffer);

      if ((i - 1) != j) {
        if (strcmp(paramName, buffer) == 0) {
          report(arg, DiagnosticsId::DI_PARAM_POSITION_MISMATCH, paramName);
        }
      }

      free(buffer);
    }
  }
}

void CheckerVisitor::visitBinaryBranches(const BinExpr *expr, const Expr *v, unsigned pf, unsigned nf) {
  Expr *lhs = expr->lhs();
  Expr *rhs = expr->rhs();
  VarScope *trunkScope = currentScope;
  VarScope *branchScope = nullptr;

  lhs->visit(this);
  currentScope = branchScope = trunkScope->copy(arena);
  setExpression(lhs, v, pf, nf);
  rhs->visit(this);
  trunkScope->merge(branchScope);
  branchScope->~VarScope();
  currentScope = trunkScope;
}

void CheckerVisitor::visitId(Id *id) {
  const StackSlot &ss = nodeStack.back();
  const Node *parentNode = nullptr;

  if (ss.sst == SST_NODE)
    parentNode = ss.n;

  if (!parentNode)
    return;

  ValueRef *v = findValueInScopes(id->id());

  if (!v)
    return;

  checkForeachIteratorInClosure(id, v);
  checkIdUsed(id, parentNode, v);
}

void CheckerVisitor::visitBinExpr(BinExpr *expr) {
  checkAndOrPriority(expr);
  checkBitwiseParenBool(expr);
  checkCoalescingPriority(expr);
  checkAssignmentToItself(expr);
  //checkGlobalVarCreation(expr); // seems this check is no longer has any sense
  checkSameOperands(expr);
  checkAlwaysTrueOrFalse(expr);
  checkDeclarationInArith(expr);
  checkIntDivByZero(expr);
  checkPotentiallyNullableOperands(expr);
  checkBitwiseToBool(expr);
  checkCompareWithBool(expr);
  checkCopyOfExpression(expr);
  checkConstInBoolExpr(expr);
  checkShiftPriority(expr);
  checkCanReturnNull(expr);
  checkCompareWithContainer(expr);
  checkBoolToStrangePosition(expr);
  checkNewSlotNameMatch(expr);
  checkPlusString(expr);

  switch (expr->op())
  {
  case TO_NULLC:
    visitBinaryBranches(expr, &nullValue, RT_NULL, 0);
    break;
  case TO_ANDAND:
    visitBinaryBranches(expr, nullptr, 0, RT_NULL);
    break;
  case TO_OROR:
    visitBinaryBranches(expr, nullptr, RT_NULL, 0);
    break;
  case TO_PLUSEQ:
  case TO_MINUSEQ:
  case TO_MULEQ:
  case TO_DIVEQ:
  case TO_MODEQ:
  case TO_NEWSLOT:
  case TO_ASSIGN:
  case TO_INEXPR_ASSIGN:
    expr->rhs()->visit(this);
    expr->lhs()->visit(this);
    break;
  default:
    Visitor::visitBinExpr(expr);
    break;
  }

  applyBinaryToScope(expr);
}

void CheckerVisitor::visitTerExpr(TerExpr *expr) {
  checkTernaryPriority(expr);
  checkSameValues(expr);
  checkAlwaysTrueOrFalse(expr);

  expr->a()->visit(this);
  VarScope *trunkScope = currentScope;

  VarScope *ifTrueScope = trunkScope->copy(arena);
  currentScope = ifTrueScope;
  setExpression(expr->a(), nullptr, 0, RT_NULL);
  expr->b()->visit(this);

  VarScope *ifFalseScope = trunkScope->copy(arena);
  currentScope = ifFalseScope;
  setExpression(expr->a(), nullptr, RT_NULL, 0);
  expr->c()->visit(this);

  trunkScope->merge(ifTrueScope);
  trunkScope->merge(ifFalseScope);

  ifTrueScope->~VarScope();
  ifFalseScope->~VarScope();

  currentScope = trunkScope;
}

void CheckerVisitor::visitCallExpr(CallExpr *expr) {
  checkExtendToAppend(expr);
  checkAlreadyRequired(expr);
  checkCallNullable(expr);
  checkArguments(expr);

  applyCallToScope(expr);

  Visitor::visitCallExpr(expr);
}

void CheckerVisitor::checkBoolIndex(const GetTableExpr *expr) {
  const Expr *key = maybeEval(expr->key())->asExpression();

  if (isBooleanResultOperator(key->op())) {
    report(expr->key(), DiagnosticsId::DI_BOOL_AS_INDEX);
  }
}

void CheckerVisitor::checkNullableIndex(const GetTableExpr *expr) {
  const Expr *key = maybeEval(expr->key());

  if (isPotentiallyNullable(key)) {
    report(expr->key(), DiagnosticsId::DI_POTENTILLY_NULLABLE_INDEX);
  }
}

void CheckerVisitor::visitGetFieldExpr(GetFieldExpr *expr) {
  checkAccessNullable(expr);
  checkEnumConstUsage(expr);

  Visitor::visitGetFieldExpr(expr);
}

void CheckerVisitor::visitGetTableExpr(GetTableExpr *expr) {
  checkBoolIndex(expr);
  checkNullableIndex(expr);
  checkAccessNullable(expr);

  Visitor::visitGetTableExpr(expr);
}

void CheckerVisitor::checkDuplicateSwitchCases(SwitchStatement *swtch) {
  const auto &cases = swtch->cases();

  for (int32_t i = 0; i < int32_t(cases.size()) - 1; ++i) {
    for (int32_t j = i + 1; j < cases.size(); ++j) {
      const auto &icase = cases[i];
      const auto &jcase = cases[j];

      if (_equalChecker.check(icase.val, jcase.val)) {
        report(jcase.val, DiagnosticsId::DI_DUPLICATE_CASE);
      }
    }
  }
}

void CheckerVisitor::checkMissedBreak(const SwitchStatement *swtch) {
  auto &cases = swtch->cases();

  FunctionReturnTypeEvaluator rtEvaluator;

  for (auto &c : cases) {
    const Statement *stmt = c.stmt;
    bool r = false;
    unsigned f = rtEvaluator.compute(stmt, r);
    if (!r) {
      bool warn = false;
      const Statement *last = nullptr;
      if (stmt->op() != TO_BLOCK) {
        last = stmt;
        warn = stmt->op() == TO_BREAK;
      }
      else {
        auto &stmts = stmt->asBlock()->statements();
        if (!stmts.empty()) {
          last = stmts.back();
          warn = last->op() != TO_BREAK;
        }
      }

      if (warn) {
        assert(last);
        report(last, DiagnosticsId::DI_MISSED_BREAK);
      }
    }
  }
}

void CheckerVisitor::checkDuplicateIfBranches(IfStatement *ifStmt) {
  if (_equalChecker.check(ifStmt->thenBranch(), ifStmt->elseBranch())) {
    report(ifStmt->elseBranch(), DiagnosticsId::DI_THEN_ELSE_EQUAL);
  }
}

void CheckerVisitor::checkDuplicateIfConditions(IfStatement *ifStmt) {
  const Statement *elseB = unwrapBody(ifStmt->elseBranch());

  if (elseB && elseB->op() == TO_IF) {
    if (findIfWithTheSameCondition(ifStmt->condition(), static_cast<const IfStatement *>(elseB))) {
      report(ifStmt->condition(), DiagnosticsId::DI_DUPLICATE_IF_EXPR);
    }
  }
}

void CheckerVisitor::checkVariableMismatchForLoop(ForStatement *loop) {
  const SQChar *varname = nullptr;
  Node *init = loop->initializer();
  Expr *cond = loop->condition();
  Expr *mod = loop->modifier();

  if (init) {
    if (init->op() == TO_ASSIGN) {
      Expr *l = static_cast<BinExpr *>(init)->lhs();
      if (l->op() == TO_ID)
        varname = l->asId()->id();
    }

    if (init->op() == TO_VAR) {
      VarDecl *decl = static_cast<VarDecl *>(init);
      varname = decl->name();
    }
  }

  if (varname && cond) {
    if (isBinaryArith(cond)) {
      BinExpr *bin = static_cast<BinExpr *>(cond);
      Expr *l = bin->lhs();
      Expr *r = bin->rhs();
      bool idUsed = false;

      if (l->op() == TO_ID) {
        if (strcmp(l->asId()->id(), varname)) {
          if (r->op() != TO_ID || strcmp(r->asId()->id(), varname)) {
            report(cond, DiagnosticsId::DI_MISMATCH_LOOP_VAR);
          }
        }
      }
    }
  }

  if (varname && mod) {
    bool idUsed = false;
    if (isAssignExpr(mod)) {
      Expr *lhs = static_cast<BinExpr *>(mod)->lhs();
      if (lhs->op() == TO_ID) {
        if (strcmp(varname, lhs->asId()->id())) {
          report(mod, DiagnosticsId::DI_MISMATCH_LOOP_VAR);
        }
      }
    }

    if (!idUsed && mod->op() == TO_INC) {
      Expr *arg = static_cast<IncExpr *>(mod)->argument();
      if (arg->op() == TO_ID) {
        if (strcmp(varname, arg->asId()->id())) {
          report(mod, DiagnosticsId::DI_MISMATCH_LOOP_VAR);
        }
      }
    }
  }
}

void CheckerVisitor::checkUnterminatedLoop(LoopStatement *loop) {
  Statement *body = loop->body();

  ReturnStatement *retStmt = nullptr;
  ThrowStatement *throwStmt = nullptr;
  BreakStatement *breakStmt = nullptr;
  ContinueStatement *continueStmt = nullptr;

  switch (body->op())
  {
  case TO_BLOCK: {
    Block *b = static_cast<Block *>(body);
    for (Statement *stmt : b->statements()) {
      switch (stmt->op())
      {
      case TO_RETURN: retStmt = static_cast<ReturnStatement *>(stmt); break;
      case TO_THROW: throwStmt = static_cast<ThrowStatement *>(stmt); break;
      case TO_BREAK: breakStmt = static_cast<BreakStatement *>(stmt); break;
      case TO_CONTINUE: continueStmt = static_cast<ContinueStatement *>(stmt); break;
      default:
        break;
      }
    }
    break;
  }
  case TO_RETURN: retStmt = static_cast<ReturnStatement *>(body); break;
  case TO_THROW: throwStmt = static_cast<ThrowStatement *>(body); break;
  case TO_BREAK: breakStmt = static_cast<BreakStatement *>(body); break;
  case TO_CONTINUE: continueStmt = static_cast<ContinueStatement *>(body); break;
  default:
    //loop->visitChildren(this);
    return;
  }

  if (retStmt || throwStmt || breakStmt || continueStmt) {
    ConditionalExitFinder checker(false);
    body->visit(&checker);

    if (retStmt) {
      if (!checker.hasBreak && !checker.hasContinue && !checker.hasThrow && loop->op() != TO_FOREACH) {
        report(retStmt, DiagnosticsId::DI_UNCOND_TERMINATED_LOOP, "return");
      }
    }

    if (throwStmt) {
      if (!checker.hasBreak && !checker.hasContinue && !checker.hasReturn && loop->op() != TO_FOREACH) {
        report(throwStmt, DiagnosticsId::DI_UNCOND_TERMINATED_LOOP, "throw");
      }
    }

    if (continueStmt) {
      if (!checker.hasBreak && !checker.hasThrow && !checker.hasReturn) {
        report(continueStmt, DiagnosticsId::DI_UNCOND_TERMINATED_LOOP, "continue");
      }
    }

    if (breakStmt) {
      if (!checker.hasContinue && !checker.hasReturn && !checker.hasThrow && loop->op() != TO_FOREACH) {
        report(breakStmt, DiagnosticsId::DI_UNCOND_TERMINATED_LOOP, "break");
      }
    }
  }
}

void CheckerVisitor::checkEmptyWhileBody(WhileStatement *loop) {
  const Statement *body = unwrapBody(loop->body());

  if (body && body->op() == TO_EMPTY) {
    report(body, DiagnosticsId::DI_EMPTY_BODY, "while");
  }
}

void CheckerVisitor::checkEmptyThenBody(IfStatement *stmt) {
  const Statement *thenStmt = unwrapBody(stmt->thenBranch());

  if (thenStmt && thenStmt->op() == TO_EMPTY) {
    report(thenStmt, DiagnosticsId::DI_EMPTY_BODY, "then");
  }
}

void CheckerVisitor::checkAssigneTwice(const Block *b) {

  const auto &statements = b->statements();

  for (int32_t i = 0; i < int32_t(statements.size()) - 1; ++i) {
    Expr *iexpr = unwrapExprStatement(statements[i]);

    if (iexpr && iexpr->op() == TO_ASSIGN) {
      for (int32_t j = i + 1; j < statements.size(); ++j) {
        Expr *jexpr = unwrapExprStatement(statements[j]);

        if (jexpr && jexpr->op() == TO_ASSIGN) {
          BinExpr *iassgn = static_cast<BinExpr *>(iexpr);
          BinExpr *jassgn = static_cast<BinExpr *>(jexpr);

          if (_equalChecker.check(iassgn->lhs(), jassgn->lhs())) {
            Expr *firstAssignee = iassgn->lhs();

            bool ignore = existsInTree(firstAssignee, jassgn->rhs());
            if (!ignore && firstAssignee->op() == TO_GETTABLE) {
              GetTableExpr *getT = firstAssignee->asGetTable();
              ignore = indexChangedInTree(getT->key());
              if (!ignore) {
                for (int32_t m = i + 1; m < j; ++m) {
                  if (checkModification(getT->key(), statements[m])) {
                    ignore = true;
                    break;
                  }
                }
              }
            }

            if (!ignore) {
              report(jassgn, DiagnosticsId::DI_ASSIGNED_TWICE);
            }
          }
        }
      }
    }
  }
}

void CheckerVisitor::checkUnutilizedResult(const ExprStatement *s) {
  const Expr *e = s->expression();

  if (e->op() == TO_CALL) {
    const CallExpr *c = static_cast<const CallExpr *>(e);
    const Expr *callee = c->callee();
    bool isCtor = false;
    const FunctionInfo *info = findFunctionInfo(callee, isCtor);
    if (info) {
      const FunctionDecl *f = info->declaration;
      assert(f);

      if (f->op() == TO_CONSTRUCTOR) {
        report(e, DiagnosticsId::DI_NAME_LIKE_SHOULD_RETURN, "<constructor>");
      }
      else if (nameLooksLikeResultMustBeUtilised(f->name())) {
        report(e, DiagnosticsId::DI_NAME_LIKE_SHOULD_RETURN, f->name());
      }
    }
    else if (isCtor) {
      report(e, DiagnosticsId::DI_NAME_LIKE_SHOULD_RETURN, "<constructor>");
    }

    const SQChar *calleeName = nullptr;

    if (callee->op() == TO_ID) {
      calleeName = callee->asId()->id();
    }
    else if (callee->op() == TO_GETFIELD) {
      calleeName = callee->asGetField()->fieldName();
    }

    if (calleeName && nameLooksLikeResultMustBeUtilised(calleeName)) {
      report(e, DiagnosticsId::DI_NAME_LIKE_SHOULD_RETURN, calleeName);
    }
    return;
  }

  if (!isAssignExpr(e) && e->op() != TO_INC && e->op() != TO_NEWSLOT) {
    report(s, DiagnosticsId::DI_UNUTILIZED_EXPRESSION);
  }
}

void CheckerVisitor::checkForgottenDo(const Block *b) {
  const auto &statements = b->statements();

  for (int32_t i = 1; i < statements.size(); ++i) {
    Statement *prev = statements[i - 1];
    Statement *stmt = statements[i];

    if (stmt->op() == TO_WHILE && prev->op() == TO_BLOCK) {
      report(stmt, DiagnosticsId::DI_FORGOTTEN_DO);
    }
  }
}

void CheckerVisitor::checkUnreachableCode(const Block *block) {
  const auto &statements = block->statements();

  for (int32_t i = 0; i < int32_t(statements.size()) - 1; ++i) {
    Statement *stmt = statements[i];
    Statement *next = statements[i + 1];
    if (isBlockTerminatorStatement(stmt->op())) {
      if (next->op() != TO_BREAK) {
        if (!onlyEmptyStatements(i + 1, statements)) {
          report(next, DiagnosticsId::DI_UNREACHABLE_CODE, terminatorOpToName(stmt->op()));
          break;
        }
      }
    }
  }
}

void CheckerVisitor::visitExprStatement(ExprStatement *stmt) {
  checkUnutilizedResult(stmt);

  Visitor::visitExprStatement(stmt);
}

void CheckerVisitor::visitBlock(Block *b) {
  checkForgottenDo(b);
  checkUnreachableCode(b);
  checkAssigneTwice(b);

  VarScope *thisScope = currentScope;
  VarScope blockScope(thisScope ? thisScope->owner : nullptr, thisScope);
  currentScope = &blockScope;

  Visitor::visitBlock(b);

  blockScope.parent = nullptr;
  blockScope.checkUnusedSymbols(this);
  currentScope = thisScope;
}

void CheckerVisitor::visitForStatement(ForStatement *loop) {
  checkUnterminatedLoop(loop);
  checkVariableMismatchForLoop(loop);

  VarScope *trunkScope = currentScope;

  Node *init = loop->initializer();
  Expr *cond = loop->condition();
  Expr *mod = loop->modifier();

  VarScope *copyScope = trunkScope->copy(arena);
  VarScope loopScope(copyScope->owner, copyScope);
  currentScope = &loopScope;

  if (init) {
    init->visit(this);
  }

  if (cond) {
    cond->visit(this);
    setExpression(cond, nullptr, 0, RT_NULL);
  }

  loop->body()->visit(this);

  if (mod) {
    mod->visit(this);
  }

  trunkScope->merge(copyScope);
  loopScope.checkUnusedSymbols(this);

  currentScope = trunkScope;
  //if (cond)
  //  setExpression(cond, &falseValue, RT_NULL, 0);
}

void CheckerVisitor::checkNullableContainer(const ForeachStatement *loop) {
  if (isPotentiallyNullable(loop->container())) {
    report(loop->container(), DiagnosticsId::DI_POTENTIALLY_NULLABLE_CONTAINER);
  }
}

void CheckerVisitor::visitForeachStatement(ForeachStatement *loop) {
  checkUnterminatedLoop(loop);
  checkNullableContainer(loop);


  VarScope *trunkScope = currentScope;
  VarScope *copyScope = trunkScope->copy(arena);
  VarScope loopScope(trunkScope->owner, copyScope);
  currentScope = &loopScope;

  const VarDecl *idx = loop->idx();
  const VarDecl *val = loop->val();

  if (idx) {
    SymbolInfo *info = makeSymbolInfo(SK_FOREACH);
    ValueRef *v = makeValueRef(info);
    v->state = VRS_UNKNOWN;
    v->expression = nullptr;
    info->declarator.v = idx;
    info->ownedScope = &loopScope;
    declareSymbol(idx->name(), v);
  }

  if (val) {
    SymbolInfo *info = makeSymbolInfo(SK_FOREACH);
    ValueRef *v = makeValueRef(info);
    v->state = VRS_UNKNOWN;
    v->expression = nullptr;
    info->declarator.v = val;
    info->ownedScope = &loopScope;
    declareSymbol(val->name(), v);
  }

  StackSlot slot;
  slot.n = loop;
  slot.sst = SST_NODE;
  nodeStack.push_back(slot);

  loop->container()->visit(this);
  loop->body()->visit(this);

  nodeStack.pop_back();

  trunkScope->merge(copyScope);
  loopScope.checkUnusedSymbols(this);
  currentScope = trunkScope;
}

void CheckerVisitor::visitWhileStatement(WhileStatement *loop) {
  checkUnterminatedLoop(loop);
  checkEmptyWhileBody(loop);

  loop->condition()->visit(this);

  VarScope *trunkScope = currentScope;
  VarScope *loopScope = trunkScope->copy(arena);
  currentScope = loopScope;
  // TODO: set condition -- true

  setExpression(loop->condition(), nullptr, 0, RT_NULL);
  loop->body()->visit(this);

  trunkScope->merge(loopScope);
  loopScope->~VarScope();
  currentScope = trunkScope;
  // TODO: check if loop has no other exits than normal
  //setExpression(loop->condition(), &falseValue, RT_NULL, 0);
}

void CheckerVisitor::visitDoWhileStatement(DoWhileStatement *loop) {
  checkUnterminatedLoop(loop);

  VarScope *trunkScope = currentScope;
  VarScope *loopScope = trunkScope->copy(arena);
  currentScope = loopScope;

  loop->body()->visit(this);
  loop->condition()->visit(this);

  trunkScope->merge(loopScope);
  loopScope->~VarScope();
  currentScope = trunkScope;
  //setExpression(loop->condition(), &falseValue, RT_NULL, 0);
}

void CheckerVisitor::visitIfStatement(IfStatement *ifstmt) {
  checkEmptyThenBody(ifstmt);
  checkDuplicateIfConditions(ifstmt);
  checkDuplicateIfBranches(ifstmt);
  checkAlwaysTrueOrFalse(ifstmt->condition());

  ifstmt->condition()->visit(this);

  VarScope *trunkScope = currentScope;
  VarScope *thenScope = trunkScope->copy(arena);
  currentScope = thenScope;
  setExpression(ifstmt->condition(), nullptr, 0, RT_NULL);
  ifstmt->thenBranch()->visit(this);
  VarScope *elseScope = nullptr;
  if (ifstmt->elseBranch()) {
    currentScope = elseScope = trunkScope->copy(arena);
    setExpression(ifstmt->condition(), nullptr, RT_NULL, 0);
    ifstmt->elseBranch()->visit(this);
  }

  if (elseScope) {
    thenScope->merge(elseScope);
    elseScope->~VarScope();
    trunkScope->copyFrom(thenScope);
  }
  else {
    trunkScope->merge(thenScope);
  }
  thenScope->~VarScope();
  currentScope = trunkScope;
}

void CheckerVisitor::visitSwitchStatement(SwitchStatement *swtch) {
  checkDuplicateSwitchCases(swtch);
  checkMissedBreak(swtch);

  Expr *expr = swtch->expression();
  expr->visit(this);

  auto &cases = swtch->cases();
  VarScope *trunkScope = currentScope;

  std::vector<VarScope *> scopes;

  for (auto &c : cases) {
    c.val->visit(this);
    VarScope *caseScope = trunkScope->copy(arena);
    currentScope = caseScope;
    setExpression(expr, c.val);
    c.stmt->visit(this);
    scopes.push_back(caseScope);
  }

  if (swtch->defaultCase().stmt) {
    VarScope *defaultScope = trunkScope->copy(arena);
    currentScope = defaultScope;
    swtch->defaultCase().stmt->visit(this);
    scopes.push_back(defaultScope);
  }

  for (VarScope *s : scopes) {
    trunkScope->merge(s);
    s->~VarScope();
  }

  currentScope = trunkScope;
}

void CheckerVisitor::visitTryStatement(TryStatement *tryStmt) {

  Statement *t = tryStmt->tryStatement();
  Id *id = tryStmt->exceptionId();
  Statement *c = tryStmt->catchStatement();

  VarScope *trunkScope = currentScope;
  VarScope *tryScope = trunkScope->copy(arena);
  currentScope = tryScope;
  t->visit(this);

  VarScope *copyScope = trunkScope->copy(arena);
  VarScope catchScope(copyScope->owner, copyScope);
  currentScope = &catchScope;
  SymbolInfo *info = makeSymbolInfo(SK_EXCEPTION);
  ValueRef *v = makeValueRef(info);
  v->state = VRS_UNKNOWN;
  v->expression = nullptr;
  info->declarator.x = id;
  info->ownedScope = &catchScope;

  declareSymbol(id->id(), v);

  id->visit(this);
  c->visit(this);

  trunkScope->merge(tryScope);
  trunkScope->merge(copyScope);
  tryScope->~VarScope();
  //catchScope->~VarScope();
  currentScope = trunkScope;
}

const SQChar *CheckerVisitor::findSlotNameInStack(const Decl *decl) {
  auto it = nodeStack.rbegin();
  auto ie = nodeStack.rend();

  while (it != ie) {
    auto slot = *it;
    if (slot.sst == SST_NODE) {
      const Node *n = slot.n;
      if (n->op() == TO_NEWSLOT) {
        const BinExpr *bin = static_cast<const BinExpr *>(n);
        Expr *lhs = bin->lhs();
        Expr *rhs = bin->rhs();
        if (rhs->op() == TO_DECL_EXPR) {
          const DeclExpr *de = static_cast<const DeclExpr *>(rhs);
          if (de->declaration() == decl) {
            if (lhs->op() == TO_LITERAL) {
              if (lhs->asLiteral()->kind() == LK_STRING) {
                return lhs->asLiteral()->s();
              }
            }
          }
          return nullptr;
        }
      }
    }
    else {
      assert(slot.sst == SST_TABLE_MEMBER);
      Expr *lhs = slot.member->key;
      Expr *rhs = slot.member->value;
      if (rhs->op() == TO_DECL_EXPR) {
        const DeclExpr *de = static_cast<const DeclExpr *>(rhs);
        if (de->declaration() == decl) {
          if (lhs->op() == TO_LITERAL) {
            if (lhs->asLiteral()->kind() == LK_STRING) {
              return lhs->asLiteral()->s();
            }
          }
        }
        return nullptr;
      }
    }
    ++it;
  }

  return nullptr;
}

void CheckerVisitor::checkFunctionReturns(FunctionDecl *func) {
  const SQChar *name = func->name();

  if (!name || name[0] == '(') {
    name = findSlotNameInStack(func);
  }

  bool dummy;
  unsigned returnFlags = FunctionReturnTypeEvaluator().compute(func->body(), dummy);

  bool reported = false;

  if (returnFlags & ~(RT_BOOL | RT_UNRECOGNIZED | RT_FUNCTION_CALL)) {
    if (nameLooksLikeResultMustBeBoolean(name)) {
      report(func, DiagnosticsId::DI_NAME_RET_BOOL, name);
      reported = true;
    }
  }

  if (!!(returnFlags & RT_NOTHING) &&
    !!(returnFlags & (RT_NUMBER | RT_STRING | RT_TABLE | RT_CLASS | RT_ARRAY | RT_CLOSURE | RT_UNRECOGNIZED | RT_THROW)))
  {
    if ((returnFlags & RT_THROW) == 0)
      report(func, DiagnosticsId::DI_NOT_ALL_PATH_RETURN_VALUE);
    reported = true;
  }
  else if (returnFlags)
  {
    unsigned flagsDiff = returnFlags & ~(RT_THROW | RT_NOTHING | RT_NULL | RT_UNRECOGNIZED | RT_FUNCTION_CALL);
    if (flagsDiff)
    {
      bool powerOfTwo = !(flagsDiff == 0) && !(flagsDiff & (flagsDiff - 1));
      if (!powerOfTwo)
      {
        report(func, DiagnosticsId::DI_RETURNS_DIFFERENT_TYPES);
        reported = true;
      }
    }
  }

  if (!reported) {
    if (!!(returnFlags & RT_NOTHING) && nameLooksLikeFunctionMustReturnResult(name)) {
      report(func, DiagnosticsId::DI_NAME_EXPECTS_RETURN, name);
    }
  }
}

void CheckerVisitor::checkAccessNullable(const DestructuringDecl *dd) {
  const Expr *i = dd->initiExpression();
  const Expr *initializer = maybeEval(i);

  if (isPotentiallyNullable(initializer)) {
    bool allHasDefault = true;

    for (auto d : dd->declarations()) {
      if (d->initializer() == nullptr) {
        allHasDefault = false;
        break;
      }
    }

    if (!allHasDefault) {
      report(dd, DiagnosticsId::DI_ACCESS_POT_NULLABLE, i->op() == TO_ID ? i->asId()->id() : "expression", "container");
    }
  }
}

void CheckerVisitor::checkAccessNullable(const AccessExpr *acc) {
  const Expr *r = maybeEval(acc->receiver());

  if (isPotentiallyNullable(r) && !acc->isNullable()) {
    report(acc, DiagnosticsId::DI_ACCESS_POT_NULLABLE, "expression", "container");
  }
}

void CheckerVisitor::checkEnumConstUsage(const GetFieldExpr *acc) {

  const SQChar *receiverName = computeNameRef(acc->receiver());

  if (!receiverName)
    return;

  const ValueRef *enumV = findValueInScopes(receiverName);
  if (!enumV || enumV->info->kind != SK_ENUM)
    return;

  const SQChar *fqn = enumFqn(arena, receiverName, acc->fieldName());
  const ValueRef *constV = findValueInScopes(fqn);
  
  if (!constV) {
    // very suspeccious
    return;
  }

  constV->info->used = true;
}

const SQChar *tryExtractKeyName(const Expr *e) {
  if (e->op() == TO_ID)
    return e->asId()->id();

  if (e->op() == TO_LITERAL) {
    if (e->asLiteral()->kind() == LK_STRING)
      return e->asLiteral()->s();
  }

  return nullptr;
}

void CheckerVisitor::visitTableDecl(TableDecl *table) {
  for (auto &member : table->members()) {
    StackSlot slot;
    slot.sst = SST_TABLE_MEMBER;
    slot.member = &member;

    checkKeyNameMismatch(tryExtractKeyName(member.key), member.value);

    nodeStack.push_back(slot);
    member.key->visit(this);
    member.value->visit(this);
    nodeStack.pop_back();
  }
}

void CheckerVisitor::visitFunctionDecl(FunctionDecl *func) {
  VarScope *parentScope = currentScope;
  VarScope *copyScope = parentScope->copy(arena, true);
  VarScope functionScope(func, copyScope);

  SymbolInfo *info = makeSymbolInfo(SK_FUNCTION);
  ValueRef *v = makeValueRef(info);
  v->state = VRS_DECLARED;
  v->expression = nullptr;
  info->declarator.f = func;
  info->ownedScope = currentScope;
  info->used = true;

  declareSymbol(func->name(), v);

  pushFunctionScope(&functionScope, func);

  FunctionInfo *oldInfo = currectInfo;
  FunctionInfo *newInfo = functionInfoMap[func];

  currectInfo = newInfo;
  assert(newInfo);

  checkFunctionReturns(func);

  Visitor::visitFunctionDecl(func);

  if (oldInfo) {
    oldInfo->joinModifiable(newInfo);
  }

  functionScope.checkUnusedSymbols(this);

  currectInfo = oldInfo;
  currentScope = parentScope;
}

ValueRef *CheckerVisitor::findValueInScopes(const SQChar *ref) {
  if (!ref)
    return nullptr;

  VarScope *current = currentScope;
  VarScope *s = current;

  while (s) {
    auto &symbols = s->symbols;
    auto it = symbols.find(ref);
    if (it != symbols.end()) {
      return it->second;
    }

    s = s->parent;
  }

  return nullptr;
}

void CheckerVisitor::applyAssignmentToScope(const BinExpr *bin) {
  assert(bin->op() == TO_ASSIGN || bin->op() == TO_INEXPR_ASSIGN);

  const Expr *lhs = bin->lhs();
  const Expr *rhs = bin->rhs();

  if (lhs->op() != TO_ID)
    return;

  const SQChar *name = lhs->asId()->id();
  ValueRef *v = findValueInScopes(name);

  if (!v) {
    // TODO: what if declarator == null
    SymbolInfo *info = makeSymbolInfo(SK_VAR);
    v = makeValueRef(info);
    currentScope->symbols[name] = v;
    info->ownedScope = currentScope;
    info->declarator.v = nullptr;
    if (currectInfo) {
      currectInfo->addModifiable(name, info->ownedScope->owner);
    }
  }

  v->expression = rhs;
  v->state = VRS_EXPRESSION;
  v->flagsNegative = v->flagsPositive = 0;
  v->lowerBound.kind = v->upperBound.kind = VBK_UNKNOWN;
}

void CheckerVisitor::applyAssignEqToScope(const BinExpr *bin) {
  assert(TO_PLUSEQ <= bin->op() && bin->op() <= TO_MODEQ);

  const Expr *lhs = bin->lhs();
  const Expr *rhs = bin->rhs();

  const SQChar *name = computeNameRef(lhs);
  if (!name)
    return;

  ValueRef *v = findValueInScopes(name);

  if (v) {
    if (currectInfo) {
      currectInfo->addModifiable(name, v->info->ownedScope->owner);
    }
    v->kill();
  }
}

void CheckerVisitor::applyBinaryToScope(const BinExpr *bin) {
  switch (bin->op())
  {
  case TO_ASSIGN:
  case TO_INEXPR_ASSIGN:
    return applyAssignmentToScope(bin);
    //return applyNewslotToScope(bin);
  case TO_PLUSEQ:
  case TO_MINUSEQ:
  case TO_MULEQ:
  case TO_DIVEQ:
  case TO_MODEQ:
    return applyAssignEqToScope(bin);
  default:
    break;
  }
}

int32_t CheckerVisitor::computeNameLength(const Expr *e) {
  switch (e->op())
  {
  case TO_GETFIELD: return computeNameLength(e->asGetField());
  //case TO_GETTABLE: return computeNameLength(e->asGetTable());
  case TO_ID: return strlen(e->asId()->id());
  case TO_ROOT: return sizeof rootName;
  case TO_BASE: return sizeof baseName;
    // TODO:
  default:
    return -1;
  }
}

void CheckerVisitor::computeNameRef(const Expr *e, SQChar *b, int32_t &ptr, int32_t size) {
  switch (e->op())
  {
  case TO_GETFIELD: return computeNameRef(e->asGetField(), b, ptr, size);
  //case TO_GETTABLE: return computeNameRef(lhs->asGetTable());
  case TO_ID: {
    int32_t l = snprintf(&b[ptr], size - ptr, "%s", e->asId()->id());
    ptr += l;
    break;
  }
  case TO_ROOT:
    snprintf(&b[ptr], size - ptr, "%s", rootName);
    ptr += sizeof rootName;
    break;
  case TO_BASE:
    snprintf(&b[ptr], size - ptr, "%s", baseName);
    ptr += sizeof baseName;
    break;
  }
}

const SQChar *CheckerVisitor::computeNameRef(const Expr *lhs) {
  int32_t length = computeNameLength(lhs);
  if (length < 0)
    return nullptr;

  SQChar *b = (SQChar *)arena->allocate(length + 1);
  int32_t ptr = 0;
  computeNameRef(lhs, b, ptr, length + 1);
  assert(ptr = length);
  return b;
}

void CheckerVisitor::setExpression(const Expr *lvalue, const Expr *rvalue, unsigned pf, unsigned nf) {
  const SQChar *name = computeNameRef(lvalue);

  if (!name)
    return;

  ValueRef *v = findValueInScopes(name);

  if (v) {
    if (rvalue) {
      v->expression = rvalue;
      v->state = VRS_EXPRESSION;
    }

    v->flagsPositive |= pf;
    v->flagsPositive &= ~nf;
    v->flagsNegative |= nf;
    v->flagsNegative &= ~pf;
  }
}

const ValueRef *CheckerVisitor::findValueForExpr(const Expr *e) {
  e = deparen(e);
  const SQChar *n = computeNameRef(e);

  if (!n) {
    return nullptr;
  }

  return findValueInScopes(n);
}

bool CheckerVisitor::isPotentiallyNullable(const Expr *e) {

  const ValueRef *v = findValueForExpr(e);

  if (v) {
    if (v->flagsPositive & RT_NULL)
      return true;

    if (v->flagsNegative & RT_NULL)
      return false;
  }

  e = maybeEval(e);

  if (e->op() == TO_LITERAL) {
    const LiteralExpr *l = e->asLiteral();
    if (l->kind() == LK_NULL) {
      return true;
    }

    if (l->kind() == LK_BOOL) {
      return !l->b();
    }

    return false;
  }

  if (e->isAccessExpr()) {
    if (e->asAccessExpr()->isNullable()) {
      return true;
    }
  }

  if (e->op() == TO_CALL) {
    const CallExpr *call = static_cast<const CallExpr *>(e);
    if (call->isNullable()) {
      return true;
    }
  }

  if (e->op() == TO_NULLC) {
    return isPotentiallyNullable(static_cast<const BinExpr *>(e)->rhs());
  }

  v = findValueForExpr(e);

  if (v) {
    switch (v->state)
    {
    case VRS_EXPRESSION:
    case VRS_INITIALIZED:
      return isPotentiallyNullable(v->expression);
    default:
      return false;
    }
  }

  return false;
}

bool CheckerVisitor::couldBeString(const Expr *e) {
  if (!e)
    return false;

  if (e->op() == TO_LITERAL) {
    return e->asLiteral()->kind() == LK_STRING;
  }

  if (e->op() == TO_NULLC) {
    const BinExpr *b = static_cast<const BinExpr *>(e);
    if (b->rhs()->op() == TO_LITERAL && b->rhs()->asLiteral()->kind() == LK_STRING)
      return couldBeString(b->lhs());
  }

  if (e->op() == TO_CALL) {
    const SQChar *name = nullptr;
    const Expr *callee = static_cast<const CallExpr *>(e)->callee();

    if (callee->op() == TO_ID)
      name = callee->asId()->id();
    else if (callee->op() == TO_GETFIELD)
      name = callee->asGetField()->fieldName();

    if (name) {
      return nameLooksLikeResultMustBeString(name) || strcmp(name, "loc") == 0;
    }
  }

  return false;
}

const Expr *CheckerVisitor::maybeEval(const Expr *e) {
  e = deparen(e);
  const ValueRef *v = findValueForExpr(e);

  if (!v) {
    return e;
  }

  if (v->hasValue()) {
    return maybeEval(v->expression);
  }
  else {
    return e;
  }
}

const SQChar *CheckerVisitor::findFieldName(const Expr *e) {
  if (e->op() == TO_ID)
    return e->asId()->id();

  if (e->op() == TO_GETFIELD)
    return e->asGetField()->fieldName();

  const ValueRef *v = findValueForExpr(e);

  if (v && v->expression && v->expression->op() == TO_DECL_EXPR) {
    const Decl *d = static_cast<const DeclExpr *>(v->expression)->declaration();
    if (d->op() == TO_FUNCTION) {
      return static_cast<const FunctionDecl *>(d)->name();
    }
  }

  return "";
}

int32_t CheckerVisitor::computeNameLength(const GetFieldExpr *acc) {
  int32_t size = computeNameLength(acc->receiver());

  if (size < 0) {
    return size;
  }

  size += 1;
  size += strlen(acc->fieldName());
  return size;
}

void CheckerVisitor::computeNameRef(const GetFieldExpr *access, SQChar *b, int32_t &ptr, int32_t size) {
  computeNameRef(access->receiver(), b, ptr, size);
  b[ptr++] = '.';
  int32_t l = snprintf(&b[ptr], size - ptr, "%s", access->fieldName());
  ptr += l;
}

const FunctionInfo *CheckerVisitor::findFunctionInfo(const Expr *e, bool &isCtor) {
  const Expr *ee = maybeEval(e);

  if (ee->op() == TO_DECL_EXPR) {
    const Decl *decl = ee->asDeclExpr()->declaration();
    if (decl->op() == TO_FUNCTION || decl->op() == TO_CLASS) {
      return functionInfoMap[static_cast<const FunctionDecl *>(decl)];
    }
  }

  const SQChar *name = computeNameRef(ee);

  if (!name)
    return nullptr;

  const ValueRef *v = findValueInScopes(name);

  if (!v || !v->hasValue())
    return nullptr;

  const Expr *expr = maybeEval(v->expression);

  if (expr->op() != TO_DECL_EXPR)
    return nullptr;

  const Decl *decl = static_cast<const DeclExpr *>(expr)->declaration();

  if (decl->op() == TO_FUNCTION || decl->op() == TO_CONSTRUCTOR) {
    return functionInfoMap[static_cast<const FunctionDecl *>(decl)];
  }
  else if (decl->op() == TO_CLASS) {
    const ClassDecl *klass = static_cast<const ClassDecl *>(decl);
    isCtor = true;
    for (auto &m : klass->members()) {
      const Expr *me = m.value;
      if (me->op() == TO_DECL_EXPR) {
        const Decl *de = static_cast<const DeclExpr *>(m.value)->declaration();
        if (de->op() == TO_CONSTRUCTOR) {
          return functionInfoMap[static_cast<const FunctionDecl *>(de)];
        }
      }
    }
  }

  return nullptr;
}

void CheckerVisitor::applyKnownInvocationToScope(const ValueRef *value) {
  const FunctionInfo *info = nullptr;

  if (value->hasValue()) {
    const Expr *expr = maybeEval(value->expression);
    assert(expr != nullptr);

    if (expr->op() == TO_DECL_EXPR) {
      const Decl *decl = static_cast<const DeclExpr *>(expr)->declaration();
      if (decl->op() == TO_FUNCTION || decl->op() == TO_CONSTRUCTOR) {
        info = functionInfoMap[static_cast<const FunctionDecl *>(decl)];
      }
      else if (decl->op() == TO_CLASS) {
        const ClassDecl *klass = static_cast<const ClassDecl *>(decl);
        for (auto &m : klass->members()) {
          const Expr *me = m.value;
          if (me->op() == TO_DECL_EXPR) {
            const Decl *de = static_cast<const DeclExpr *>(m.value)->declaration();
            if (de->op() == TO_CONSTRUCTOR) {
              info = functionInfoMap[static_cast<const FunctionDecl *>(de)];
              break;
            }
          }
        }
      }
      else {
        applyUnknownInvocationToScope();
        return;
      }
    }
  }

  if (!info) {
    // probably it is class constructor
    return;
  }

  for (auto s : info->modifible) {
    VarScope *scope = currentScope->findScope(s.owner);
    if (!scope)
      continue;

    auto it = scope->symbols.find(s.name);
    if (it != currentScope->symbols.end()) {
      if (currectInfo) {
        currectInfo->addModifiable(it->first, it->second->info->ownedScope->owner);
      }
      it->second->kill();
    }
  }
}

void CheckerVisitor::applyUnknownInvocationToScope() {
  VarScope *s = currentScope;

  while (s) {
    auto &symbols = s->symbols;
    for (auto &sym : symbols) {
      if (currectInfo) {
        currectInfo->addModifiable(sym.first, sym.second->info->ownedScope->owner);
      }
      sym.second->kill();
    }
    s = s->parent;
  }
}

void CheckerVisitor::applyCallToScope(const CallExpr *call) {
  const Expr *callee = deparen(call->callee());

  if (callee->op() == TO_ID) {
    const Id *calleeId = callee->asId();
    //const NameRef ref(nullptr, calleeId->id());
    const ValueRef *value = findValueInScopes(calleeId->id());
    if (value) {
      applyKnownInvocationToScope(value);
    }
    else {
      // unknown invocation by pure Id points to some external
      // function which could not modify any scoped value
    }
  }
  else if (callee->op() == TO_GETFIELD) {
    const SQChar *ref = computeNameRef(callee);
    const ValueRef *value = findValueInScopes(ref);
    if (value) {
      applyKnownInvocationToScope(value);
    }
    else if (!ref || strncmp(ref, rootName, sizeof rootName)) {
      // we don't know what exactly is being called so assume the most conservative case
      applyUnknownInvocationToScope();
    }
  }
  else {
    // unknown invocation so everything could be modified
    applyUnknownInvocationToScope();
  }
}

void CheckerVisitor::pushFunctionScope(VarScope *functionScope, const FunctionDecl *decl) {

  FunctionInfo *info = functionInfoMap[decl];

  if (!info) {
    info = makeFunctionInfo(decl, currentScope->owner);
    functionInfoMap[decl] = info;
  }

  currentScope = functionScope;
}

void CheckerVisitor::declareSymbol(const SQChar *nameRef, ValueRef *v) {
  currentScope->symbols[nameRef] = v;
}

void CheckerVisitor::visitParamDecl(ParamDecl *p) {
  Visitor::visitParamDecl(p);

  const Expr *dv = p->defaultValue();

  SymbolInfo *info = makeSymbolInfo(SK_PARAM);
  ValueRef *v = makeValueRef(info);
  v->state = VRS_UNKNOWN;
  v->expression = nullptr;
  info->declarator.p = p;
  info->ownedScope = currentScope;
  info->used = p->isVararg();

  if (dv && dv->op() == TO_LITERAL) {
    if (dv->asLiteral()->kind() == LK_NULL) {
      v->flagsPositive |= RT_NULL;
    }
  }

  declareSymbol(p->name(), v);

  assert(currectInfo);
  currectInfo->parameters.push_back(normalizeParamName(p->name()));
}

void CheckerVisitor::visitVarDecl(VarDecl *decl) {
  Visitor::visitVarDecl(decl);

  SymbolInfo *info = makeSymbolInfo(decl->isAssignable() ? SK_VAR : SK_BINDING);
  ValueRef *v = makeValueRef(info);
  v->state = decl->initializer() ? VRS_INITIALIZED : VRS_UNDEFINED;
  v->expression = decl->initializer();
  info->declarator.v = decl;
  info->ownedScope = currentScope;

  declareSymbol(decl->name(), v);
}

void CheckerVisitor::visitConstDecl(ConstDecl *cnst) {

  SymbolInfo *info = makeSymbolInfo(SK_CONST);
  ValueRef *v = makeValueRef(info);
  v->state = VRS_INITIALIZED;
  v->expression = cnst->value();
  info->declarator.c = cnst;
  info->ownedScope = currentScope;

  declareSymbol(cnst->name(), v);

  Visitor::visitConstDecl(cnst);
}

void CheckerVisitor::visitEnumDecl(EnumDecl *enm) {

  SymbolInfo *info = makeSymbolInfo(SK_ENUM);
  ValueRef *ev = makeValueRef(info);
  ev->state = VRS_DECLARED;
  ev->expression = nullptr;
  info->declarator.e = enm;
  info->ownedScope = currentScope;
  declareSymbol(enm->name(), ev);

  for (auto &c : enm->consts()) {
    SymbolInfo *constInfo = makeSymbolInfo(SK_ENUM_CONST);
    ValueRef *cv = makeValueRef(constInfo);
    cv->state = VRS_INITIALIZED;
    cv->expression = c.val;
    constInfo->declarator.ec = &c;
    constInfo->ownedScope = currentScope;

    const SQChar *fqn = enumFqn(arena, enm->name(), c.id);
    declareSymbol(fqn, cv);

    c.val->visit(this);
  }
}

void CheckerVisitor::visitDesctructingDecl(DestructuringDecl *d) {

  checkAccessNullable(d);

  Visitor::visitDesctructingDecl(d);
}

void CheckerVisitor::analyse(RootBlock *root) {
  root->visit(this);
}

class NameShadowingChecker : public Visitor {
  SQCompilationContext _ctx;

  std::vector<const Node *> nodeStack;

  void report(const Node *n, enum DiagnosticsId id, ...) {
    va_list vargs;
    va_start(vargs, id);

    _ctx.vreportDiagnostic(id, n->lineStart(), n->columnStart(), n->columnEnd() - n->columnStart(), vargs);

    va_end(vargs);
  }

  struct Scope;

  struct SymbolInfo {
    union {
      const Id *x;
      const FunctionDecl *f;
      const ClassDecl *k;
      const TableMember *m;
      const VarDecl *v;
      const TableDecl *t;
      const ParamDecl *p;
      const EnumDecl *e;
      const ConstDecl *c;
      const EnumConst *ec;
    } declaration;

    enum SymbolKind kind;

    const struct Scope *ownerScope;
    const SQChar *name;

    SymbolInfo(enum SymbolKind k) : kind(k) {}
  };

  struct Scope {
    NameShadowingChecker *checker;

    Scope(NameShadowingChecker *chk, const Decl *o) : checker(chk), owner(o), symbols() {
      parent = checker->scope;
      checker->scope = this;
    }

    ~Scope() {
      checker->scope = parent;
    }

    struct Scope *parent;
    std::unordered_map<const SQChar *, SymbolInfo *, StringHasher, StringEqualer> symbols;
    const Decl *owner;

    SymbolInfo *findSymbol(const SQChar *name) const;
  };

  const Node *extractPointedNode(const SymbolInfo *info);

  SymbolInfo *newSymbolInfo(enum SymbolKind k) {
    void *mem = _ctx.arena()->allocate(sizeof SymbolInfo);
    return new(mem) SymbolInfo(k);
  }

  struct Scope *scope;

  void walkTableMembers(const TableDecl *t);

  void declareVar(enum SymbolKind k, const VarDecl *v);
  void declareSymbol(const SQChar *name, SymbolInfo *info);

public:
  NameShadowingChecker(SQCompilationContext &ctx) : _ctx(ctx), scope(nullptr) {}

  void visitNode(Node *n);

  void visitVarDecl(VarDecl *var);
  void visitFunctionDecl(FunctionDecl *f);
  void visitParamDecl(ParamDecl *p);
  void visitConstDecl(ConstDecl *c);
  void visitEnumDecl(EnumDecl *e);
  void visitClassDecl(ClassDecl *k);
  void visitTableDecl(TableDecl *t);

  void visitBlock(Block *block);
  void visitTryStatement(TryStatement *stmt);
  void visitForStatement(ForStatement *stmt);
  void visitForeachStatement(ForeachStatement *stmt);

  void analyse(RootBlock *root) {
    root->visit(this);
  }
};

const Node *NameShadowingChecker::extractPointedNode(const SymbolInfo *info) {
  switch (info->kind)
  {
  case SK_EXCEPTION:
    return info->declaration.x;
  case SK_FUNCTION:
    return info->declaration.f;
  case SK_METHOD:
  case SK_FIELD:
    return info->declaration.m->key;
  case SK_CLASS:
    return info->declaration.k;
  case SK_TABLE:
    return info->declaration.t;
  case SK_VAR:
  case SK_BINDING:
  case SK_FOREACH:
    return info->declaration.v;
  case SK_CONST:
    return info->declaration.c;
  case SK_ENUM:
    return info->declaration.e;
  case SK_ENUM_CONST:
    return info->declaration.ec->val;
  case SK_PARAM:
    return info->declaration.p;
  default:
    assert(0);
    return nullptr;
  }
}

void NameShadowingChecker::declareSymbol(const SQChar *name, SymbolInfo *info) {
  const SymbolInfo *existedInfo = scope->findSymbol(name);
  if (existedInfo) {
    bool warn = true;
    if (strcmp(name, "this") == 0) {
      warn = false;
    }

    if ((existedInfo->kind == SK_BINDING || existedInfo->kind == SK_VAR) && info->kind == SK_FUNCTION) {
      if (nodeStack.size() > 2) {
        const Node *ln = nodeStack[nodeStack.size() - 1];
        const Node *lln = nodeStack[nodeStack.size() - 2];
        if (ln->op() == TO_DECL_EXPR && static_cast<const DeclExpr *>(ln)->declaration() == info->declaration.f) {
          if (existedInfo->declaration.v == lln) {
            warn = false;
          }
        }
      }
    }

    if (existedInfo->kind == SK_METHOD && info->kind == SK_FUNCTION) {
      const Node *ln = nodeStack.back();
      if (existedInfo->declaration.m->value == ln) {
        warn = false;
      }
    }

    if (warn) {
      report(extractPointedNode(info), DiagnosticsId::DI_ID_HIDES_ID,
        symbolContextName(info->kind),
        info->name,
        symbolContextName(existedInfo->kind));
    }
  }

  scope->symbols[name] = info;
}

NameShadowingChecker::SymbolInfo *NameShadowingChecker::Scope::findSymbol(const SQChar *name) const {
  auto it = symbols.find(name);
  if (it != symbols.end()) {
    return it->second;
  }

  return parent ? parent->findSymbol(name) : nullptr;
}

void NameShadowingChecker::visitNode(Node *n) {
  nodeStack.push_back(n);
  n->visitChildren(this);
  nodeStack.pop_back();
}

void NameShadowingChecker::declareVar(enum SymbolKind k, const VarDecl *var) {
  SymbolInfo *info = newSymbolInfo(k);
  info->declaration.v = var;
  info->ownerScope = scope;
  info->name = var->name();
  declareSymbol(var->name(), info);
}

void NameShadowingChecker::visitVarDecl(VarDecl *var) {
  declareVar(var->isAssignable() ? SK_VAR : SK_BINDING, var);

  Visitor::visitVarDecl(var);
}

void NameShadowingChecker::visitParamDecl(ParamDecl *p) {
  SymbolInfo *info = newSymbolInfo(SK_PARAM);
  info->declaration.p = p;
  info->ownerScope = scope;
  info->name = p->name();
  declareSymbol(p->name(), info);

  Visitor::visitParamDecl(p);
}

void NameShadowingChecker::visitConstDecl(ConstDecl *c) {
  SymbolInfo *info = newSymbolInfo(SK_CONST);
  info->declaration.c = c;
  info->ownerScope = scope;
  info->name = c->name();
  declareSymbol(c->name(), info);

  Visitor::visitConstDecl(c);
}

void NameShadowingChecker::visitEnumDecl(EnumDecl *e) {
  SymbolInfo *info = newSymbolInfo(SK_ENUM);
  info->declaration.e = e;
  info->ownerScope = scope;
  info->name = e->name();
  declareSymbol(e->name(), info);

  for (auto &ec : e->consts()) {
    SymbolInfo *cinfo = newSymbolInfo(SK_ENUM_CONST);
    const SQChar *fqn = enumFqn(_ctx.arena(), e->name(), ec.id);

    info->declaration.ec = &ec;
    info->ownerScope = scope;
    info->name = fqn;

    declareSymbol(fqn, cinfo);
  }
}

void NameShadowingChecker::visitFunctionDecl(FunctionDecl *f) {
  Scope *p = scope;

  bool tableScope = p->owner && (p->owner->op() == TO_CLASS || p->owner->op() == TO_TABLE);

  if (!tableScope) {
    SymbolInfo *info = newSymbolInfo(SK_FUNCTION);
    info->declaration.f = f;
    info->ownerScope = p;
    info->name = f->name();
    declareSymbol(f->name(), info);
  }

  Scope funcScope(this, f);
  Visitor::visitFunctionDecl(f);
}

static bool isFunctionDecl(const Expr *e) {
  if (e->op() != TO_DECL_EXPR)
    return false;

  const Decl *d = static_cast<const DeclExpr *>(e)->declaration();

  return d->op() == TO_FUNCTION || d->op() == TO_CONSTRUCTOR;
}

void NameShadowingChecker::walkTableMembers(const TableDecl *t) {
  for (auto &m : t->members()) {
    if (m.key->op() == TO_LITERAL && m.key->asLiteral()->kind() == LK_STRING) {
      const SQChar *name = m.key->asLiteral()->s();
      bool isMethod = isFunctionDecl(m.value);
      SymbolInfo *info = newSymbolInfo(isMethod ? SK_METHOD : SK_FIELD);
      info->declaration.m = &m;
      info->name = name;
      info->ownerScope = scope;
      declareSymbol(name, info);
    }
  }
}

void NameShadowingChecker::visitTableDecl(TableDecl *t) {
  Scope tableScope(this, t);

  walkTableMembers(t);
  nodeStack.push_back(t);
  t->visitChildren(this);
  nodeStack.pop_back();
}

void NameShadowingChecker::visitClassDecl(ClassDecl *k) {
  Scope klassScope(this, k);

  walkTableMembers(k);
  nodeStack.push_back(k);
  k->visitChildren(this);
  nodeStack.pop_back();
}

void NameShadowingChecker::visitBlock(Block *block) {
  Scope blockScope(this, scope ? scope->owner : nullptr);
  Visitor::visitBlock(block);
}

void NameShadowingChecker::visitTryStatement(TryStatement *stmt) {

  nodeStack.push_back(stmt);

  stmt->tryStatement()->visit(this);

  Scope catchScope(this, scope->owner);

  SymbolInfo *info = newSymbolInfo(SK_EXCEPTION);
  info->declaration.x = stmt->exceptionId();
  info->ownerScope = scope;
  info->name = stmt->exceptionId()->id();
  declareSymbol(stmt->exceptionId()->id(), info);

  stmt->catchStatement()->visit(this);

  nodeStack.pop_back();
}

void NameShadowingChecker::visitForStatement(ForStatement *stmt) {
  assert(scope);
  Scope scope(this, scope->owner);

  Visitor::visitForStatement(stmt);
}

void NameShadowingChecker::visitForeachStatement(ForeachStatement *stmt) {

  assert(scope);
  Scope scope(this, scope->owner);

  VarDecl *idx = stmt->idx();
  if (idx) {
    declareVar(SK_FOREACH, idx);
    assert(idx->initializer() == nullptr);
  }
  VarDecl *val = stmt->val();
  if (val) {
    declareVar(SK_FOREACH, val);
    assert(val->initializer() == nullptr);
  }

  nodeStack.push_back(stmt);

  stmt->container()->visit(this);
  stmt->body()->visit(this);

  nodeStack.pop_back();
}

void StaticAnalyser::runAnalysis(RootBlock *root)
{
  CheckerVisitor(_ctx).analyse(root);
  NameShadowingChecker(_ctx).analyse(root);
}

}
