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

// scroll values and constants
static float scroll_x = 0;
static float scroll_y = 0;
const static float SCROLL_SPEED = 10;
const static float NODE_MIN_SIZE_Y = 30;
const static float NODE_MIN_SIZE_X = 2*NODE_MIN_SIZE_Y;

// focus value
static bool refresh_nodes = false;

static ImVec2 current_node_size(100, 50);

struct Function {
    std::string name;
    size_t id;
    // args...

    Function(const Function& f);
    Function(const std::string &_name, size_t _id);
};

struct Node {
    ImVec2 position;
    ImVec2 size;
    ParserFunctionInfo* function;
    std::vector<Node*> neighbors;
    
    int depth;
    bool show_children;
    size_t number_of_active_parents;

    void init();
    Node();
    Node(ParserFunctionInfo* _function);

    inline ImVec2 get_absolute_position() 
        {return ImVec2(scroll_x + position.x, scroll_y + position.y);}

    inline void set_position(ImVec2 new_position) {position = new_position;}
    inline void set_depth(int new_depth) {depth = new_depth;}
    inline void set_size(ImVec2 new_size) {size = new_size;}
    inline void add_edge(Node* node) {neighbors.push_back(node);}

    void show_neighbours();
    void hide_neighbours();
    void show_info();
    void draw(ImGuiWindow* window, const ImU32& line_color, size_t line_thickness);
};

struct GraphGui {
    ImGuiWindow* window;
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<size_t> layers;
    ImGuiIO* io_pointer;

    // constants
    int top_distance = 40;
    int left_distance = 25;
    int node_distance_x = 175;
    int node_distance_y = 100;
    int node_line_thickness = 5;
    ImU32 node_line_color = IM_COL32(255, 165, 0, 100);
    explicit GraphGui(ParserFunctionCallGraph& call_graph, ImGuiIO& io);
    
    GraphGui() = default;
	GraphGui(const GraphGui&) = default;
	GraphGui(GraphGui&&) = default;
	
	GraphGui& operator=(GraphGui&&) = default;
	GraphGui& operator=(const GraphGui&) = default;
	
	~GraphGui() = default;

    void set_window(ImGuiWindow* new_window);
    void draw();
    void calculate_depth(Node* node);
    void refresh();
    void key_input_check();

    void focus_node(std::string node_signature);
};

} // namespace GraphGui

#endif
