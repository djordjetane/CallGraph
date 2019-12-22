#include "graph.hpp"

namespace GraphGui {

Function::Function(const Function& f)
        : name(f.name), id(f.id)
        {}

Function::Function(const std::string &_name, size_t _id)
    : name(_name), id(_id)
    {}

Node::Node()
{
    init();
}

Node::Node(ParserFunctionInfo* _function)
    : function(_function)
{
    init();
}

void Node::init()
{
    position = current_node_size;
    size = current_node_size;
    number_of_active_parents = 0;
    depth = 0;
    show_children = false;
}

void Node::show_neighbours()
{
    show_children = true;
    for(unsigned i=0; i<neighbors.size(); i++)
    {
        Node* neighbor = neighbors.at(i);
        neighbor->number_of_active_parents++;
    }
}

void Node::hide_neighbours()
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

void Node::show_info()
{
    ImGui::Text("%s", function->signature.c_str());
}

void Node::draw(ImGuiWindow* window, const ImU32& line_color, size_t line_thickness)
{
    if(number_of_active_parents == 0)
        return; 

    ImVec2 real_position = ImVec2(position.x + scroll_x, position.y + scroll_y);

    ImGui::SetNextWindowPos(real_position);
    ImGui::SetNextWindowSize(size);
    ImGui::BeginChild(function->signature.c_str(), size, false);
    //show_info();

    ImVec2 position = ImVec2(real_position.x + 50.f, real_position.y + 20.f);

    ImU32 col32Node = ImColor(0.f, 247.f/255.f, 1.f);
    ImU32 col32Text = ImColor(1.f, 1.f, 1.f);

    window->DrawList->AddCircleFilled(position, 25.f, col32Node, 256);
    window->DrawList->AddText(ImVec2(position.x - 25.f, position.y + 30.f), col32Text, function->signature.c_str());

    ImGuiWindow* node_window = ImGui::GetCurrentWindow();

    bool is_clicked = ImGui::IsMouseClicked(0);
    bool is_hovering = ImGui::IsWindowHovered();

    if(is_clicked && is_hovering)
        last_clicked_node = this;

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
        start_position.x += current_node_size.x-5;
        start_position.y += current_node_size.y/2;

        for(unsigned i=0; i<neighbors.size(); i++)
        {
            Node* neighbor = neighbors.at(i);
            if(!neighbor->number_of_active_parents)
                continue;

            ImVec2 end_position = ImVec2(neighbor->position.x + scroll_x, 
                                            neighbor->position.y + scroll_y);
            end_position.x += 5;
            end_position.y += current_node_size.y/2;

            window->DrawList->AddBezierCurve(start_position,
                ImVec2(start_position.x+current_node_size.x/2, start_position.y),
                ImVec2(start_position.x, end_position.y),
                end_position,
                line_color,
                line_thickness);
            //Drawing triangles for arrow end
            if(start_position.x+current_node_size.x/2 <= end_position.x)   
                window->DrawList->AddTriangleFilled(ImVec2(end_position.x + 10.f, end_position.y)
                                                , ImVec2(end_position.x, end_position.y + 5.f)
                                                , ImVec2(end_position.x, end_position.y - 5.f), line_color);
            else
            {
                window->DrawList->AddTriangleFilled(ImVec2(start_position.x - 10.f, start_position.y)
                                                , ImVec2(start_position.x, start_position.y + 5.f)
                                                , ImVec2(start_position.x, start_position.y - 5.f), line_color);
            }
            
        }
    }
    ImGui::End();
    if(ImGui::IsWindowFocused())
        refresh_nodes = true;
}

GraphGui::GraphGui(ParserFunctionCallGraph& call_graph, ImGuiIO& io, TextEditor& editor)
{
    io_pointer = &io;
    editor_pointer = &editor;

    int index = 0;
    int main_function_index = 0;
    for(const auto& e : call_graph.nodes)
    {
        nodes.emplace_back(std::make_unique<Node>());
        nodes.back()->function = e.get();

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

void GraphGui::set_window(ImGuiWindow* new_window) 
{
    window = new_window;
}

void GraphGui::draw()
{
    key_input_check();
    layers.clear();
    layers.resize(nodes.size(), 0);

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

void GraphGui::calculate_depth(Node* node) 
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

void GraphGui::refresh()
{
    for(auto& node: nodes)
        ImGui::SetWindowFocus(node->function->signature.c_str());
    refresh_nodes = false;
}

void GraphGui::key_input_check()
{
    ImVec2 screen_position = io_pointer->MousePos;

    if(screen_position.x < window->Pos.x || 
        screen_position.y < window->Pos.y)
        return;
    if(screen_position.x > (window->Pos.x + window->Size.x) ||
        screen_position.y > (window->Pos.y + window->Size.y))
        return;

    if(ImGui::IsKeyPressed('W') || io_pointer->KeysDown[io_pointer->KeyMap[ImGuiKey_UpArrow]])
    {
        scroll_y -= SCROLL_SPEED;
    }
    if(ImGui::IsKeyPressed('S') || io_pointer->KeysDown[io_pointer->KeyMap[ImGuiKey_DownArrow]])
    {
        scroll_y += SCROLL_SPEED;
    }
    if(ImGui::IsKeyPressed('A') || io_pointer->KeysDown[io_pointer->KeyMap[ImGuiKey_LeftArrow]])
    {
        scroll_x -= SCROLL_SPEED;
    }
    if(ImGui::IsKeyPressed('D') || io_pointer->KeysDown[io_pointer->KeyMap[ImGuiKey_RightArrow]])
    {
        scroll_x += SCROLL_SPEED;
    }

    if((last_clicked_node != nullptr) 
        && io_pointer->KeyShift 
        && io_pointer->KeyCtrl 
        && io_pointer->KeysDown['T'])
    {
        std::vector<std::string> buffer_lines = editor_pointer->GetTextLines();
        long unsigned row = 0, col = 0; 
        for(auto begin = buffer_lines.begin(); begin != buffer_lines.end() && col == 0; begin++, row++)
        {
            col = begin->find(last_clicked_node->function->signature);
            if(col == begin->npos)
                col = 0;
        }
        row--;

        //Finds first ocurance not declaration (#FIXIT)

        if(row != 0 || col != 0)
            editor_pointer->SetSelection(TextEditor::Coordinates(row, col)
                            , TextEditor::Coordinates(row, col + last_clicked_node->function->signature.length()));

        // Temporary (#TODO)
        last_clicked_node = nullptr;
    }

    current_node_size.x *= (100.0f - 3*io_pointer->MouseWheel)/100;
    current_node_size.y *= (100.0f - 3*io_pointer->MouseWheel)/100;
    current_node_size.x = std::max(NODE_MIN_SIZE_X, current_node_size.x);
    current_node_size.y = std::max(NODE_MIN_SIZE_Y, current_node_size.y);
    node_distance_x = 1.5*current_node_size.x;
    node_distance_y = 1.5*current_node_size.y;
    for(auto& node: nodes)
        node->set_size(current_node_size);
}

void GraphGui::focus_node(std::string node_signature)
{
    for(const auto& e : nodes)
        if(e->function->signature == node_signature)
        {
            if(e->number_of_active_parents == 0)
                continue; 
                
            int wx = window->Pos.x;
            int wy = window->Pos.y;
            int wx_mid = window->Size.x/2;
            int wy_mid = window->Size.y/2;

            int x = e->position.x - wx;
            int y = e->position.y - wy;

            scroll_x = wx_mid - x - e->size.x/2;
            scroll_y = wy_mid - y - e->size.x/2;
            
            break;
        }
}


} // namespace GraphGui
