#ifndef GRAPH_GUI
#define GRAPH_GUI

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
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
#include <cmath>
#include "clang_interface.h"
#include "TextEditor.h"

namespace GraphGui {

// name constants
const static unsigned DISPLAY_NAME_LENGHT = 11;

// scroll values and constants
static float scroll_x = 0;
static float scroll_y = 0;
const static float SCROLL_SPEED = 10;
const static float NODE_MIN_SIZE_Y = 60;
const static float NODE_MIN_SIZE_X = NODE_MIN_SIZE_Y;

// focus value
static bool refresh_nodes = false;

static ImVec2 current_node_size(NODE_MIN_SIZE_X, NODE_MIN_SIZE_Y);

struct Node {
    ImVec2 position;
    ImVec2 size;
    clang_interface::FunctionDecl* function;
    std::vector<Node*> neighbors;
    char display_name[DISPLAY_NAME_LENGHT];
    
    int depth;
    bool show_children;
    size_t number_of_active_parents;

    void init();
    void set_display_name();
    Node();
    Node(clang_interface::FunctionDecl* _function);

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

// Last clicked node
static Node* last_clicked_node = nullptr;
static Node* hovered_node = nullptr;
static Node* root = nullptr;

class GraphGui {

private:
    ImGuiWindow* window;
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<size_t> layers;
    ImGuiIO* io_pointer;
    TextEditor* editor_pointer;

    // constants
    int top_distance = 40;
    int left_distance = 25;
    int node_distance_x = 175;
    int node_distance_y = 100;
    int node_line_thickness = 5;
    ImU32 node_line_color = IM_COL32(255, 165, 0, 100);


public:
    GraphGui(ImGuiIO* io, TextEditor *editor)
        : io_pointer(io)
        , editor_pointer(editor)
    {}
    void BuildCallGraph(clang_interface::CallGraph& call_graph);
    void set_window(ImGuiWindow* new_window);
    void draw(clang_interface::FunctionDecl* function);
    void calculate_depth(Node* node);
    void refresh();
    void key_input_check();
    
    void focus_node(const std::string& node_signature);
    void draw_node_info_window();
    void graph_init();
};

} // namespace GraphGui

#endif
