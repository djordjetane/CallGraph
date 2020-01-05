#ifndef CLANG_INTERFACE_H
#define CLANG_INTERFACE_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Basic/SourceLocation.h"
#include "llvm/Support/raw_ostream.h"

#define DUMP(out, x) out << #x << ' ' << x << '\n'

namespace clang_interface {

class ASTUnit {
 private:
  std::unique_ptr<clang::ASTUnit> ast;

 public:
  ASTUnit() = default;
  explicit ASTUnit(std::unique_ptr<clang::ASTUnit> arg) : ast(std::move(arg)) {}

  auto& ASTContext() { return ast->getASTContext(); }
  const auto& ASTContext() const { return ast->getASTContext(); }
};

class ParamVarDecl {
 private:
  const clang::ParmVarDecl* decl{nullptr};
  std::string name;
  std::string type;
  clang::FullSourceLoc full_source_loc;
 public:
  ParamVarDecl() = default;
  explicit ParamVarDecl(const clang::ParmVarDecl* p, unsigned index)
      : decl(p),
        name(p->getNameAsString()),
        type(decl->getOriginalType().getAsString()) {}
  unsigned ID() const { return decl->getID(); }
  const std::string& NameAsString() const { return name; }
  const std::string& TypeAsString() const { return type; }
  operator bool() const { return decl; }
};

class FunctionDecl {
 private:
  const clang::FunctionDecl* decl{nullptr};
  std::string name;
  std::string return_type;
  std::vector<ParamVarDecl> params;
  std::string ast_dump;
  clang::FullSourceLoc full_source_loc;

 public:
  FunctionDecl() = default;
  explicit FunctionDecl(const clang::FunctionDecl* arg, clang::FullSourceLoc source_loc)
      : decl(arg),
        name(arg->getNameAsString()),
        return_type(arg->getReturnType().getAsString()),
        full_source_loc(source_loc) {
    unsigned i = 0;
    for (auto param = arg->param_begin(); param != arg->param_end(); ++param) {
      params.emplace_back(*param, ++i);
    }
    llvm::raw_string_ostream out(ast_dump);
    arg->dump(out);
    out.str();

  }
  const std::string& ASTDump() const { return ast_dump; }
  unsigned ID() const { return decl->getID(); }
  const std::string& NameAsString() const { return name; }
  const std::string& ReturnTypeAsString() const { return return_type; }

  const clang::FullSourceLoc& FullSourceLoc() const { return full_source_loc; }
	
  auto ParamBegin() const { return params.begin(); }
  auto ParamEnd() const { return params.end(); }

  bool HasParams() const { return ParamBegin() != ParamEnd(); }
  bool IsMain() const { return decl->isMain(); }
  operator bool() const { return decl; }
};

struct Edge {
  clang_interface::FunctionDecl* caller;
  clang_interface::FunctionDecl* callee;
};

struct CallGraph {
  using NodesList = std::vector<std::unique_ptr<FunctionDecl>>;
  using EdgesList = std::vector<Edge>;

  NodesList nodes;
  EdgesList edges;
};

std::ostream& operator<<(std::ostream&, const ParamVarDecl&);
std::ostream& operator<<(std::ostream&, const FunctionDecl&);
std::ostream& operator<<(std::ostream&, const Edge&);
std::ostream& operator<<(std::ostream&, const CallGraph&);

ASTUnit BuildASTFromSource(const std::string& source,
                           std::vector<std::string> compiler_args = {});
void AddEdge(CallGraph& call_graph, Edge edge);
std::optional<clang_interface::FunctionDecl> FindNodeWithId(
    const CallGraph& call_graph, unsigned id);
CallGraph ExtractCallGraphFromAST(ASTUnit& ast);
CallGraph ExtractCallGraphFromSource(const std::string& source);
// CallGraph ExtractCallGraphFromFile(const std::string& file_name);

};  // namespace clang_interface

#endif
