#ifndef GRAPH_GUI
#define GRAPH_GUI

#include "imgui_util/imgui.h"
#include "imgui_util/imgui_internal.h"
#include "imgui_util/glfw_opengl3/imgui_impl_glfw.h"
#include "imgui_util/glfw_opengl3/imgui_impl_opengl3.h"
#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <stack>
#include <utility>
#include <algorithm>
#include <queue>
#include "function_parser.hpp"

namespace GraphGui {

// scroll values
static float scroll_x = 0;
static float scroll_y = 0;
const static float SCROLL_SPEED = 10;

// focus value
static bool refresh_nodes = false;

const ImVec2 NODE_DEFAULT_POSITION(100, 100);
const ImVec2 NODE_DEFAULT_SIZE(100, 50);

struct Function {
    std::string name;
    size_t id;
    // args...

    Function(const Function& f)
        : name(f.name), id(f.id)
        {}

    Function(const std::string &_name, size_t _id)
        : name(_name), id(_id)
        {}

};

struct Node {
    ImVec2 position;
    ImVec2 size;
    ParserFunctionInfo* function;
    std::vector<Node*> neighbors;

    int depth;
    bool show_children;
    size_t number_of_active_parents;

    void init()
    {
        position = NODE_DEFAULT_SIZE;
        size = NODE_DEFAULT_SIZE;
        number_of_active_parents = 0;
        show_children = false;
    }

    Node() = default;
    Node(ParserFunctionInfo* _function)
        : function(_function)
    {
        init();
    }

    inline void set_position(ImVec2 new_position) {position = new_position;}
    inline void set_depth(int new_depth) {depth = new_depth;}
    inline void add_edge(Node* node) {neighbors.push_back(node);}

    void show_neighbours()
    {
        show_children = true;
        for(unsigned i=0; i<neighbors.size(); i++)
        {
            Node* neighbor = neighbors.at(i);
            neighbor->number_of_active_parents++;
        }
    }

    void hide_neighbours()
    {
        show_children = false;
        for(unsigned i=0; i<neighbors.size(); i++)
        {
            Node* neighbor = neighbors.at(i);
            neighbor->number_of_active_parents--;
            if(neighbor->number_of_active_parents == 0 && neighbor->show_children)
                neighbor->hide_neighbours();
        }
    }

    void draw(ImGuiWindow* window, const ImU32& line_color, size_t line_thickness)
    {
        if(number_of_active_parents == 0)
            return; 

        ImVec2 real_position = ImVec2(position.x - scroll_x, position.y - scroll_y);

        ImGui::SetNextWindowPos(real_position);
        ImGui::SetNextWindowSize(size);
        ImGui::Begin(function->signature.c_str());
        ImGuiWindow* node_window = ImGui::GetCurrentWindow();

        bool is_clicked = ImGui::IsMouseClicked(0);
        bool is_hovering = ImGui::IsWindowHovered();

        if(number_of_active_parents && is_clicked && is_hovering)
        {
            if(show_children)
                hide_neighbours();
            else
                show_neighbours();
        }

        if(show_children)
        {
            ImVec2 start_position = node_window->Pos;
            start_position.x += NODE_DEFAULT_SIZE.x-5;
            start_position.y += NODE_DEFAULT_SIZE.y/2;

            for(unsigned i=0; i<neighbors.size(); i++)
            {
                Node* neighbor = neighbors.at(i);
                if(!neighbor->number_of_active_parents)
                    continue;

                ImVec2 end_position = ImVec2(neighbor->position.x - scroll_x, 
                                             neighbor->position.y - scroll_y);
                end_position.x += 5;
                end_position.y += NODE_DEFAULT_SIZE.y/2;

                window->DrawList->AddBezierCurve(start_position,
                    ImVec2(start_position.x+NODE_DEFAULT_SIZE.x/2, start_position.y),
                    ImVec2(start_position.x, end_position.y),
                    end_position,
                    line_color,
                    line_thickness);
            }
        }
        ImGui::End();
        if(ImGui::IsWindowFocused())
            refresh_nodes = true;
    }
};

struct GraphGui {
    ImGuiWindow* window;
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<size_t> layers;

    // constants
    int top_distance = 40;
    int left_distance = 25;
    int node_distance_x = 175;
    int node_distance_y = 100;
    int node_line_thickness = 5;
    ImU32 node_line_color = IM_COL32(255, 165, 0, 100);
    
    GraphGui() = default;
	GraphGui(const GraphGui&) = default;
	GraphGui(GraphGui&&) = default;
	
	GraphGui& operator=(GraphGui&&) = default;
	GraphGui& operator=(const GraphGui&) = default;
	
	~GraphGui() = default;
	
    explicit GraphGui(ParserFunctionCallGraph& call_graph)
    {
        int index = 0;
        int main_function_index = 0;
        for(const auto& e : call_graph.nodes)
        {
            nodes.emplace_back(std::make_unique<Node>());
            nodes.back()->function = e.get();
            // popraviti deo sa Node() = default ~ treba da poziva init();
            // dole je privremeno resenje
            nodes.back()->init();

            if(nodes.back()->function->signature == "main")
                main_function_index = index;
            index++;
        }

        // postavljamo da main ima nulti indeks
        swap(nodes.at(0), nodes.at(main_function_index));

        for(const auto[from, to] : call_graph.edges)
        {
            auto from_node = std::find_if(nodes.begin(), nodes.end(), [id = from->id](const auto& n) {
                return n->function->id == id;
            });

            auto to_node = std::find_if(nodes.begin(), nodes.end(), [id = to->id](const auto& n) {
                return n->function->id == id;
            });

            (*from_node)->add_edge((*to_node).get());
        }
        
        std::cout << "sup" << std::endl;
        for(const auto& e : nodes)
        {
            std::cout << e->function->signature << '\n';
            for(const auto& n: e->neighbors)
            {
                std::cout << '\t' << n->function->signature << '\n';
            }
        }
        std::cout << std::endl;

        layers.resize(nodes.size(), 0);
        nodes.at(0)->number_of_active_parents = 1;
        
        calculate_depth(nodes.front().get());
    }

    inline void set_window(ImGuiWindow* new_window) 
    {
        layers.clear();
        layers.resize(nodes.size(), 0);
        window = new_window;
    }

    void draw()
    {
        key_input_check();

        for(auto& node: nodes)
        {
            node->set_position(ImVec2(left_distance + window->Pos.x + node->depth*node_distance_x,
                                      top_distance + window->Pos.y + layers.at(node->depth)*node_distance_y));
            layers.at(node->depth)++;
        }

        for(auto& node: nodes)
            node->draw(window, node_line_color, node_line_thickness);

        if(refresh_nodes)
            refresh();

    }

    void calculate_depth(Node* node) 
    { 
        std::vector<bool> visited(nodes.size(), false);

        std::queue<std::pair<Node*, int> > s;
        s.push(std::make_pair(node, 0));
        while(!s.empty())
        {
            Node* node = s.front().first;
            int depth = s.front().second;
            s.pop();

            if(visited.at(node->function->id))
                continue;
            visited.at(node->function->id) = true;

            node->set_depth(depth);
            for(Node* neighbor: node->neighbors)
                s.push(std::make_pair(neighbor, depth+1));
        }
    }

    void refresh()
    {
        for(auto& node: nodes)
            ImGui::SetWindowFocus(node->function->signature.c_str());
        refresh_nodes = false;
    }

    void key_input_check()
    {
        if(ImGui::IsKeyPressed('W'))
        {
            scroll_y += SCROLL_SPEED;
        }
        if(ImGui::IsKeyPressed('S'))
        {
            scroll_y -= SCROLL_SPEED;
        }
        if(ImGui::IsKeyPressed('A'))
        {
            scroll_x += SCROLL_SPEED;
        }
        if(ImGui::IsKeyPressed('D'))
        {
            scroll_x -= SCROLL_SPEED;
        }
    }
};

} // GraphGui

#endif
