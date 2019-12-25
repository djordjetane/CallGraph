#ifndef PARSER_H
#define CLANG_INTERFACE_H

#include<vector>
#include<string>
#include<memory>
#include<optional>

#include "clang/AST/AST.h"
#include "clang/AST/Decl.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"


#define DUMP(out, x) out << #x << ' ' << x << '\n'

namespace clang_interface
{

class ASTUnit
{
private:
    std::unique_ptr<clang::ASTUnit> ast;
public:
    explicit ASTUnit(std::unique_ptr<clang::ASTUnit> arg) : ast(std::move(arg))
    {}

    auto& ASTContext()
    {
        return ast->getASTContext();
    }
    const auto& ASTContext() const
    {
        return ast->getASTContext();
    }
};

ASTUnit BuildASTFromSource(const std::string& source)
{
    ASTUnit ast(clang::tooling::buildASTFromCode(source, "source.cc"));
    return ast;
}

class ParamVarDecl
{
private:
    const clang::ParmVarDecl *decl{nullptr};
public:
    ParamVarDecl() = default;
    explicit ParamVarDecl(const clang::ParmVarDecl* p) : decl(p)
    {}

    unsigned ID() const
    {
        return decl->getID();
    }
    std::string NameAsString() const
    {
        return decl->getNameAsString();
    }
    std::string TypeAsString() const
    {
        return decl->getOriginalType().getAsString();
    }
   operator bool() const
   {
      return decl;
   }
};

class FunctionDecl
{
private:
    const clang::FunctionDecl* decl {nullptr};

public:
    FunctionDecl() = default;
    explicit FunctionDecl(const clang::FunctionDecl* arg) : decl(arg) {}
    unsigned ID() const
    {
        return decl->getID();
    }
    std::string NameAsString() const
    {
        return decl->getNameAsString();
    }
    std::string ReturnTypeAsString() const
    {
        return decl->getReturnType().getAsString();
    }

    auto ParamBegin() const
    {
        return decl->param_begin();
    }
    auto ParamEnd() const
    {
        return decl->param_end();
    }
   operator bool() const
   {
      return decl;
   }

};

struct Edge
{
   clang_interface::FunctionDecl caller;
   clang_interface::FunctionDecl callee;
};

struct CallGraph
{
    using NodesList = std::vector<FunctionDecl>;
    using EdgesList = std::vector<Edge>;

    NodesList nodes;
    EdgesList edges;
};

std::ostream& operator<<(std::ostream&, const ParamVarDecl&);
std::ostream& operator<<(std::ostream&, const FunctionDecl&);
std::ostream& operator<<(std::ostream&, const Edge&);
std::ostream& operator<<(std::ostream&, const CallGraph&);

void AddEdge(CallGraph &call_graph, Edge edge);
std::optional<clang_interface::FunctionDecl> FindNodeWithId(const CallGraph& call_graph, unsigned id);
CallGraph ExtractCallGraphFromAST(ASTUnit& ast);
CallGraph ExtractCallGraphFromSource(const std::string& source);
//CallGraph ExtractCallGraphFromFile(const std::string& file_name);

}; // namespace clang_interface

#endif
