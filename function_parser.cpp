#include "function_parser.hpp"
#include<fstream>
#include<iostream>
#include<algorithm>
#include<vector>
#include<utility>
#include<unordered_map>

static bool StartsWith(const std::string& search_in, const char* this_characters)
{
    return search_in.rfind(this_characters, 0) == 0;
}

static std::string& LTrimInplace(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    return s;
}

using Edge = std::string;
using Node = std::string;

bool IsEdge(const std::string& line)
{
    return line.find("->") != std::string::npos;
}

bool IsNode(const std::string &line)
{
    return line.find("[") != std::string::npos;
}

std::pair<std::string, std::string> ExtractEdge(const std::string &e)
{
    auto pos = e.find("->");
    return {
        {e.begin(), e.begin() + pos - 1},
        {e.begin() + pos + 3, e.end() - 1}
    };
}

std::string ExtractFunctionSignature(const std::string& line)
{
    auto beg = line.find('"') + 2;
    auto end = line.rfind('"') - 1;
    std::string result(line.begin() + beg, line.begin() + end);
    return result;
}

std::string ExtractNodeAddress(const std::string& line)
{
    std::string result(line.begin(), line.begin() + line.find(' '));
    return result;
}

ParserFunctionCallGraph ExtractCallGraphFromFile(const char *file)
{
    std::ifstream stream{file};
    std::string line;
    ParserFunctionCallGraph result;
    std::unordered_map<std::string, ParserFunctionInfo::IdType> address_to_index;
    size_t id = 0;
    std::vector<std::pair<std::string, std::string>> edges;

    while(std::getline(stream, line))
    {
        LTrimInplace(line);
        if(IsNode(line))
        {
            auto function_signature = ExtractFunctionSignature(line);
            auto node_address = ExtractNodeAddress(line);
            //std::cout << function_signature << ' ' << node_address << '\n';
            result.nodes.emplace_back(std::make_unique<ParserFunctionInfo>(id, function_signature));
            address_to_index[node_address] = id;
            ++id;
            //std::cout << id - 1 << ' ' << function_signature << ' ' << node_address << '\n';
        }
        else if (IsEdge(line))
        {
            edges.emplace_back(ExtractEdge(line));
            //std::cout << edges.back().first << ' ' << edges.back().second << '\n';
        }
    }

    result.edges.reserve(edges.size());

    for(const auto& [from, to] : edges)
    {
        auto from_it = address_to_index.find(from);
        auto to_it = address_to_index.find(to);
        if(from_it != address_to_index.end() && to_it != address_to_index.end())
        {
            auto from_index = address_to_index.find(from)->second;
            auto to_index = address_to_index.find(to)->second;
            //std::cout << from_index << ' ' << to_index << '\n';
            result.edges.emplace_back(std::make_pair(result.nodes[from_index].get(), result.nodes[to_index].get()));
        }   
    }

    return result;
}
