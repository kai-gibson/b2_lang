#ifndef TYPE_CHECK_VISITOR_H
#define TYPE_CHECK_VISITOR_H

#include "visitor.h"

class TypeCheckVisitor : public Visitor {
 public:
  TypeCheckVisitor() = default;
  void finalise();
  void visit(BinaryExpression& expr) override;
  void visit(FloatLiteralExpression& expr) override;
  void visit(VariableExpression& expr) override;
  void visit(Program& expr) override;
  void visit(VariableDeclarationStatement& stmt) override;
  void visit(VariableAssignmentStatement& stmt) override;
  void visit(ShowStatement& stmt) override;
  void visit(FunctionDeclaration& stmt) override;
  void visit(FunctionCallExpression& stmt) override;
  void visit(ReturnStatement& stmt) override;
  void visit(TypeExpression& stmt) override;
  void visit(IntLiteralExpression& stmt) override;
  void visit(IfStatement& stmt) override;

  // helper function to visit leaf nodes
  auto emit(ASTNode& node) -> Type {
    node.accept(*this);
    return result;
  }

 private:
  Type result;
  std::unordered_map<std::string, Type> variable_map;
  FunctionDeclaration* current_function{};
};

#endif  // TYPE_CHECK_VISITOR_H
