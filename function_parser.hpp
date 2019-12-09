#include<vector>
#include<map>
#include<string>
#include<memory>

struct ParserFunctionInfo
{
    using IdType = std::size_t;
    using SignatureInfoType = std::string;

    IdType id;
    SignatureInfoType signature;

    ParserFunctionInfo() = default;
    ParserFunctionInfo(IdType id, SignatureInfoType signature) : id(id), signature(std::move(signature)) {}
};

struct ParserFunctionCallGraph
{
    std::vector<std::pair<ParserFunctionInfo*, ParserFunctionInfo*>> edges;
    std::vector<std::unique_ptr<ParserFunctionInfo>> nodes;
};


ParserFunctionCallGraph ExtractCallGraphFromFile(const char *file);

