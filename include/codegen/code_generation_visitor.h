#ifndef CODE_GENERATION_VISITOR_H
#define CODE_GENERATION_VISITOR_H

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include "ast.h"
#include "visitor.h"

class CodegenVisitor : public Visitor {
 public:
  CodegenVisitor();
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

  auto emit(ASTNode& expr) -> llvm::Value*;
  void compile();
  void output_llvm(const std::string_view filename);
  auto get_llvm_type(Type& type) -> llvm::Type*;

  std::unique_ptr<llvm::LLVMContext> llvm_context;
  std::unique_ptr<llvm::Module> llvm_module;
  std::unique_ptr<llvm::IRBuilder<>> llvm_builder;

  std::unordered_map<std::string, llvm::Value*> named_values;
  llvm::Function* current_function = nullptr;

 private:
  llvm::Value* result;
};

#endif  // CODE_GENERATION_VISITOR_H
