
#include <memory>
#include <variant>

struct ReturnStatement;
struct IntLiteralExpression;

using Node = std::variant<ReturnStatement, IntLiteralExpression>;

struct ReturnStatement {
  std::unique_ptr<Node> value;
};

struct IntLiteralExpression {
  int32_t value;
};

// class TypeChecker {
//  public:
//  private:
//   auto visit(FloatLiteralExpression* node) -> void {}
//   auto visit(BinaryExpression* node) -> void {}
//   auto visit(VariableExpression* node) -> void {}
//   auto visit(VariableAssignmentStatement* node) -> void {}
//   auto visit(VariableDeclarationStatement* node) -> void {}
//   auto visit(ShowStatement* node) -> void {}
//   auto visit(FunctionDeclaration* node) -> void {}
//   auto visit(FunctionCallExpression* node) -> void {}
//   auto visit(Program* node) -> void {}
//   auto visit(ReturnStatement* node) -> void {}
//   auto visit(TypeExpression* node) -> void {}
//   auto visit(IntLiteralExpression* node) -> void {}
//   auto visit(IfStatement* node) -> void {}
//
//   auto visit(std::unique_ptr<ASTNode>& node) -> void {
//     switch (node->kind()) {
//       case NodeKind::FloatLiteralExpression:
//         visit(static_cast<FloatLiteralExpression*>(node.get()));
//         break;
//       case NodeKind::BinaryExpression:
//         visit(static_cast<BinaryExpression*>(node.get()));
//         break;
//       case NodeKind::VariableExpression:
//         visit(static_cast<VariableExpression*>(node.get()));
//         break;
//       case NodeKind::VariableAssignmentStatement:
//         visit(static_cast<VariableAssignmentStatement*>(node.get()));
//         break;
//       case NodeKind::VariableDeclarationStatement:
//         visit(static_cast<VariableDeclarationStatement*>(node.get()));
//         break;
//       case NodeKind::ShowStatement:
//         visit(static_cast<ShowStatement*>(node.get()));
//         break;
//       case NodeKind::FunctionDeclaration:
//         visit(static_cast<FunctionDeclaration*>(node.get()));
//         break;
//       case NodeKind::FunctionCallExpression:
//         visit(static_cast<FunctionCallExpression*>(node.get()));
//         break;
//       case NodeKind::Program:
//         visit(static_cast<Program*>(node.get()));
//         break;
//       case NodeKind::ReturnStatement:
//         visit(static_cast<ReturnStatement*>(node.get()));
//         break;
//       case NodeKind::TypeExpression:
//         visit(static_cast<TypeExpression*>(node.get()));
//         break;
//       case NodeKind::IntLiteralExpression:
//         visit(static_cast<IntLiteralExpression*>(node.get()));
//         break;
//       case NodeKind::IfStatement:
//         visit(static_cast<IfStatement*>(node.get()));
//         break;
//     }
//   }
// };
