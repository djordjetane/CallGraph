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

namespace GraphGui {

const ImVec2 NODE_DEFAULT_POSITION(100, 100);
const ImVec2 NODE_DEFAULT_SIZE(100, 50);
const int UNSET_DEPTH = -1;

struct Function {
    std::string name;
    size_t id;
    // args...

    Function(const Function& f)
    {
        name = f.name;
        id = f.id;   
    }

    Function(const std::string &_name, size_t _id)
    {
        name = _name;
        id = _id;
    }

};

struct Node {
    ImVec2 position;
    ImVec2 size;
    Function function;
    std::vector<Node*> neighbors;
    int depth;

    Node(Function _function)
        : function(_function)
    {
        position = NODE_DEFAULT_SIZE;
        size = NODE_DEFAULT_SIZE;
        depth = UNSET_DEPTH;
    }

    inline void set_position(ImVec2 _position) {position = _position;}
    inline void set_depth(int _depth) {depth = _depth;}
    inline void add_edge(Node* node) { neighbors.push_back(node);}

    void draw(ImGuiWindow* window, size_t line_thickness)
    {
        ImGui::SetNextWindowPos(position);
        ImGui::SetNextWindowSize(size);
        ImGui::Begin(function.name.c_str());
        ImGuiWindow* w = ImGui::GetCurrentWindow();

        ImVec2 start_position = w->Pos;
        start_position.x += NODE_DEFAULT_SIZE.x-5;
        start_position.y += NODE_DEFAULT_SIZE.y/2;

        /* TMP_BEGIN */
        for(unsigned i=0; i<neighbors.size(); i++)
        {
            Node* neighbor = neighbors.at(i);
            ImVec2 end_position = neighbor->position;
            end_position.x += 5;
            end_position.y += NODE_DEFAULT_SIZE.y/2;

            window->DrawList->AddLine(start_position, 
                                      end_position, 
                                      IM_COL32(255, 1, 1, 100), line_thickness);
        }
        /* TMP_END */
        ImGui::End();
    }
};

struct GraphGui {
    ImGuiWindow* window;
    size_t size;
    std::vector<Node*> nodes;
    std::vector<size_t> layers;

    // constants
    const int top_distance = 40;
    const int left_distance = 25;
    const int node_distance_x = 175;
    const int node_distance_y = 100;
    const int node_line_thickness = 5;
    
    GraphGui(ImGuiWindow* _window)
    {
        window = _window;

        // tmp hard-coded graph:
        Function f1 = Function("A", 0);
        Function f2 = Function("B", 1);
        Function f3 = Function("C", 2);
        Function f4 = Function("D", 3);
        Function f5 = Function("E", 4);
        Function f6 = Function("F", 5);
        Function f7 = Function("G", 6);

        Node* n1 = new Node(f1);
        Node* n2 = new Node(f2);
        Node* n3 = new Node(f3);
        Node* n4 = new Node(f4);
        Node* n5 = new Node(f5);
        Node* n6 = new Node(f6);
        Node* n7 = new Node(f7);

        nodes.push_back(n1);
        nodes.push_back(n2);
        nodes.push_back(n3);
        nodes.push_back(n4);
        nodes.push_back(n5);
        nodes.push_back(n6);
        nodes.push_back(n7);

        n1->add_edge(n2);
        n2->add_edge(n3);
        n2->add_edge(n4);
        n1->add_edge(n5);
        n1->add_edge(n6);
        n6->add_edge(n4);
        n4->add_edge(n7);

        size = 7;
        layers.resize(size, 0);

        calc_depth(n1);
    }

    void draw()
    {
        for(Node* node: nodes)
        {
            node->set_position(ImVec2(left_distance + window->Pos.x + node->depth*node_distance_x,
                                      top_distance + window->Pos.y + layers.at(node->depth)*node_distance_y));
            layers.at(node->depth)++;
        }

        for(Node* node: nodes)
            node->draw(window, node_line_thickness);
    }

    void _calc_depth(Node* node, int depth, std::vector<bool> &visited) 
    {
        // #TODO: zameniti rekurziju sa stekom
        if(visited.at(node->function.id))
            return;
        visited.at(node->function.id) = true;

        node->set_depth(depth);
        for(Node* neighbor: node->neighbors)
            if(!visited.at(neighbor->function.id))
                _calc_depth(neighbor, depth+1, visited);
    }
    void calc_depth(Node* node) 
    { 
        std::vector<bool> visited(size, false);
        _calc_depth(node, 0, visited);
    }
};

} // GraphGui

#endif