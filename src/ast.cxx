#include "ast.h"

#include "visitor.h"

inline void FloatLiteralExpression::accept(Visitor& v) { v.visit(*this); }
inline void BinaryExpression::accept(Visitor& v) { v.visit(*this); }
inline void VariableExpression::accept(Visitor& v) { v.visit(*this); }
inline void Program::accept(Visitor& v) { v.visit(*this); }
inline void VariableDeclarationStatement::accept(Visitor& v) { v.visit(*this); }
inline void VariableAssignmentStatement::accept(Visitor& v) { v.visit(*this); }
inline void ShowStatement::accept(Visitor& v) { v.visit(*this); }
inline void FunctionDeclaration::accept(Visitor& v) { v.visit(*this); }
inline void FunctionCallExpression::accept(Visitor& v) { v.visit(*this); }
inline void ReturnStatement::accept(Visitor& v) { v.visit(*this); }
inline void TypeExpression::accept(Visitor& v) { v.visit(*this); }
inline void IntLiteralExpression::accept(Visitor& v) { v.visit(*this); }
inline void IfStatement::accept(Visitor& v) { v.visit(*this); }
inline void BlockStatement::accept(Visitor& v) { v.visit(*this); }
inline void LoopStatement::accept(Visitor& v) { v.visit(*this); }
inline void BreakStatement::accept(Visitor& v) { v.visit(*this); }
inline void CycleStatement::accept(Visitor& v) { v.visit(*this); }
inline void PointerTypeExpression::accept(Visitor& v) { v.visit(*this); }
