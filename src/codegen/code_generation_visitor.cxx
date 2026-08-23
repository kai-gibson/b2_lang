#include "codegen/code_generation_visitor.h"

#include <llvm/IR/Verifier.h>

#include <fstream>

#include "codegen/operation_map.h"
#include "compiler.h"

CodegenVisitor::CodegenVisitor() {
  llvm_context = std::make_unique<llvm::LLVMContext>();
  llvm_module = std::make_unique<llvm::Module>("my_module", *llvm_context);
  llvm_builder = std::make_unique<llvm::IRBuilder<>>(*llvm_context);

  auto* printf_type =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(*llvm_context),
                              {llvm::PointerType::get(*llvm_context, 0)}, true);

  llvm::Function::Create(printf_type, llvm::Function::ExternalLinkage, "printf",
                         *llvm_module);
}

auto CodegenVisitor::emit(ASTNode& expr) -> llvm::Value* {
  expr.accept(*this);
  return result;
}

auto CodegenVisitor::get_llvm_type(Type& type) -> llvm::Type* {
  switch (type.type_id) {
    case TypeId::Bool:
      return llvm::Type::getInt1Ty(*llvm_context);
    case TypeId::Int8:
      return llvm::Type::getInt8Ty(*llvm_context);
    case TypeId::Int16:
      return llvm::Type::getInt16Ty(*llvm_context);
    case TypeId::Int32:
      return llvm::Type::getInt32Ty(*llvm_context);
    case TypeId::Int64:
      return llvm::Type::getInt64Ty(*llvm_context);
    case TypeId::UInt8:
      return llvm::Type::getInt8Ty(*llvm_context);
    case TypeId::UInt16:
      return llvm::Type::getInt16Ty(*llvm_context);
    case TypeId::UInt32:
      return llvm::Type::getInt32Ty(*llvm_context);
    case TypeId::UInt64:
      return llvm::Type::getInt64Ty(*llvm_context);
    case TypeId::Float32:
      return llvm::Type::getFloatTy(*llvm_context);
    case TypeId::Float64:
      return llvm::Type::getDoubleTy(*llvm_context);
    case TypeId::String:
    case TypeId::UserDefined:
      throw std::runtime_error(
          std::format("Datatype {} not implemented", type.identifier));
    case TypeId::IntLiteral:
    case TypeId::FloatLiteral:
      throw std::runtime_error(std::format(
          "Datatype {} should not survive until codegen", type.identifier));
  }
}

void CodegenVisitor::compile() {
  if (llvm::verifyModule(*llvm_module, &llvm::errs()))
    throw std::runtime_error("LLVM IR errors detected. See output");

  compiler::emit_object_file(*llvm_module, "output.o");
  compiler::link("output.o", "output");
}

void CodegenVisitor::output_llvm(const std::string_view filename) {
  this->llvm_module->print(llvm::errs(), nullptr);

  auto outfile = std::format("{}.ll", filename);
  std::ofstream ofs(outfile);
  std::error_code EC;
  llvm::raw_fd_ostream file(outfile, EC, llvm::sys::fs::OF_None);

  if (!EC) {
    this->llvm_module->print(file, nullptr);
    file.close();
  } else {
    llvm::errs() << "Error opening file: " << EC.message() << "\n";
  }
}

void CodegenVisitor::visit(VariableDeclarationStatement& stmt) {
  auto* exists = named_values[stmt.name];
  if (exists) {
    throw std::runtime_error(
        "Cannot declare same variable twice; use `set x = 1` to modify "
        "value");
  }

  auto r = emit(*stmt.value);

  if (!stmt.declaration_type) {
    throw std::runtime_error("Unable to resolve variable type");
  }

  auto type = get_llvm_type(*stmt.declaration_type);
  auto* alloca = llvm_builder->CreateAlloca(type, nullptr, stmt.name);
  llvm_builder->CreateStore(r, alloca);
  named_values[stmt.name] = alloca;
  result = r;
  return;
}

void CodegenVisitor::visit(VariableAssignmentStatement& stmt) {
  auto* variable = named_values[stmt.name];
  if (!variable) {
    throw std::runtime_error(
        std::format("Cannot assign to non-existent variable: {}", stmt.name));
  }

  auto r = emit(*stmt.value);

  llvm_builder->CreateStore(r, variable);
  result = r;
  return;
}

void CodegenVisitor::visit(BinaryExpression& expr) {
  auto l = emit(*expr.lhs);
  auto r = emit(*expr.rhs);

  if (!l || !r) throw std::runtime_error("codegen failed on BinaryExpression");

  if (!expr.resolved_type) {
    throw std::runtime_error("Couldn't resolve type for binary expression");
  }

  // bool operations need to use type operations of the operand type
  std::unique_ptr<TypeOperations> builder;

  if (expr.resolved_type->type_id == TypeId::Bool) {
    if (!expr.operand_type) {
      throw std::runtime_error("Couldn't resolve type for boolean expression");
    }
    builder = build_type_operations(*expr.operand_type, *llvm_builder);
  } else {
    builder = build_type_operations(*expr.resolved_type, *llvm_builder);
  }

  result = builder->apply(expr.op, l, r);
}

void CodegenVisitor::visit(FloatLiteralExpression& expr) {
  result = llvm::ConstantFP::get(*llvm_context, llvm::APFloat(expr.value));
}

void CodegenVisitor::visit(IntLiteralExpression& expr) {
  int32_t num_bits{};
  switch (expr.resolved_type->type_id) {
    case TypeId::Int8:
    case TypeId::UInt8:
      num_bits = 8;
      break;
    case TypeId::Int16:
    case TypeId::UInt16:
      num_bits = 16;
      break;
    case TypeId::Int32:
    case TypeId::UInt32:
      num_bits = 32;
      break;
    case TypeId::Int64:
    case TypeId::UInt64:
      num_bits = 64;
      break;
    default:
      throw std::runtime_error(
          "Got a non integer type inside int literal codegen.");
  }

  bool is_signed{};
  switch (expr.resolved_type->type_id) {
    case TypeId::Int8:
    case TypeId::Int16:
    case TypeId::Int32:
    case TypeId::Int64:
      is_signed = true;
      break;
    case TypeId::UInt8:
    case TypeId::UInt16:
    case TypeId::UInt32:
    case TypeId::UInt64:
      is_signed = false;
      break;
    default:
      throw std::runtime_error(
          "Got a non integer type inside int literal codegen.");
  }

  result = llvm::ConstantInt::get(
      *llvm_context,
      llvm::APInt(num_bits, expr.value.convert_to<uint64_t>(), is_signed));
}

void CodegenVisitor::visit(VariableExpression& expr) {
  auto* alloca = named_values[expr.name];
  if (!alloca)
    throw std::runtime_error(
        std::format("Unknown variable name: {}", expr.name));

  if (!expr.resolved_type) {
    throw std::runtime_error("Unable to resolve variable expression type");
  }

  auto type = get_llvm_type(*expr.resolved_type);
  result = llvm_builder->CreateLoad(type, alloca, expr.name);
}

void CodegenVisitor::visit(Program& expr) {
  for (auto& func : expr.functions) {
    result = emit(*func);
  }
}

void CodegenVisitor::visit(ShowStatement& stmt) {
  auto* fmt = llvm_builder->CreateGlobalStringPtr("%d\n", "fmt");

  auto* printf_fn = llvm_module->getFunction("printf");

  result = emit(*stmt.expr);
  llvm_builder->CreateCall(printf_fn->getFunctionType(), printf_fn,
                           {fmt, result});
}

void CodegenVisitor::visit(FunctionDeclaration& func_declaration) {
  named_values.clear();

  auto* func_type =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(*llvm_context), false);
  auto* func =
      llvm::Function::Create(func_type, llvm::Function::ExternalLinkage,
                             func_declaration.name, *llvm_module);

  this->current_function = func;
  auto* entry = llvm::BasicBlock::Create(*llvm_context, "entry", func);
  llvm_builder->SetInsertPoint(entry);

  for (auto& statement : func_declaration.statements) result = emit(*statement);
}

void CodegenVisitor::visit(ReturnStatement& ret) {
  auto value = emit(*ret.expr);

  llvm_builder->CreateRet(value);
}

void CodegenVisitor::visit(TypeExpression& ret) { (void)ret; }

void CodegenVisitor::visit(FunctionCallExpression& funccall) {
  auto* fn = llvm_module->getFunction(funccall.name);
  if (!fn)
    throw std::runtime_error(
        std::format("Function {} does not exist", funccall.name));

  result = llvm_builder->CreateCall(fn->getFunctionType(), fn, {});
}

void CodegenVisitor::visit(IfStatement& if_stmt) {
  auto cond = emit(*if_stmt.condition);

  auto* if_true =
      llvm::BasicBlock::Create(*llvm_context, "if.then", current_function);

  auto* if_end =
      llvm::BasicBlock::Create(*llvm_context, "if.end", current_function);

  llvm_builder->CreateCondBr(cond, if_true, if_end);

  llvm_builder->SetInsertPoint(if_true);

  for (const auto& stmt : if_stmt.body) {
    stmt->accept(*this);
  }

  llvm_builder->CreateBr(if_end);

  llvm_builder->SetInsertPoint(if_end);
}
