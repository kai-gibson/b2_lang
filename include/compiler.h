#ifndef COMPILER_H
#define COMPILER_H

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include <stdexcept>
#include <string>

namespace compiler {
inline void emit_object_file(llvm::Module& module,
                             const std::string& filename) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  auto target_triple = llvm::sys::getDefaultTargetTriple();
  module.setTargetTriple(target_triple);

  std::string error;
  auto* target = llvm::TargetRegistry::lookupTarget(target_triple, error);
  if (!target) throw std::runtime_error("Failed to find target: " + error);

  auto* machine = target->createTargetMachine(
      target_triple, "generic", "", llvm::TargetOptions{}, llvm::Reloc::PIC_);

  module.setDataLayout(machine->createDataLayout());

  std::error_code ec;
  llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);
  if (ec) throw std::runtime_error("Failed to open file: " + ec.message());

  llvm::legacy::PassManager pass;
  if (machine->addPassesToEmitFile(pass, dest, nullptr,
                                   llvm::CodeGenFileType::ObjectFile))
    throw std::runtime_error("Target machine cannot emit object file");

  pass.run(module);
  dest.flush();
}

inline void link(const std::string& obj_file, const std::string& out_file) {
  auto cmd = "clang " + obj_file + " -o " + out_file;
  if (std::system(cmd.c_str()) != 0) throw std::runtime_error("Linking failed");
}
}  // namespace compiler

#endif  // COMPILER_H
