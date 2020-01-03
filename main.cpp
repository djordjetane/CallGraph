// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#include "imgui_util/imgui.h"
#include "imgui_util/glfw_opengl3/imgui_impl_glfw.h"
#include "imgui_util/glfw_opengl3/imgui_impl_opengl3.h"
#include "imgui_util/misc/cpp/imgui_stdlib.cpp"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include "editor_util/editor_util.hpp"
#include "editor_util/TextEditor.h"
#include "graph.hpp"

#include "clang_interface.h"
namespace fs = std::filesystem;

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


//GraphGui::GraphGui graph(nullptr);

namespace gui {
class MainWindow {
    const char* glsl_version;
    GLFWwindow* window;
    bool err;

public:
    GLFWwindow* Window()
    {
        return window;
    }
    MainWindow() {
        // Setup window
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
        {
            exit(EXIT_FAILURE);
        }

        // Decide GL+GLSL versions
    #if __APPLE__
        // GL 3.2 + GLSL 150
        glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    #else
        // GL 3.0 + GLSL 130
        glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    #endif

        // Create window with graphics context

        auto monitor = glfwGetPrimaryMonitor();
        auto mode = glfwGetVideoMode(monitor);

        window = glfwCreateWindow(mode->width, mode->height, "CallGraph", NULL, NULL);
        if (window == NULL)
        {
            exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        // Initialize OpenGL loader
    #if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
        err = gl3wInit() != 0;
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
        err = glewInit() != GLEW_OK;
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
        err = gladLoadGL() == 0;
    #else
        err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
    #endif
        if (err)
        {
            fprintf(stderr, "Failed to initialize OpenGL loader!\n");
            exit(EXIT_FAILURE);
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    ~MainWindow() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

class SourceCodePanel {
    ImGuiIO &io;
    TextEditor editor;
    MainWindow& main_window;    
    std::string filename;
    std::string restore_filename = "";
    bool is_clicked_NEW = false;
    bool is_clicked_OPEN = false;
    bool bt_Save = false;        
    bool unsaved = true;
    bool should_build_callgraph = false;
    bool *show_source_code_window;
public:
    SourceCodePanel(ImGuiIO& io, MainWindow& main_window, bool *p_open) : io(io), main_window(main_window), show_source_code_window(p_open) {
        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
    }
    TextEditor& Editor() { return editor; }
    auto SecondsSinceLastTextChange() const
    {
        return editor.SecondsSinceLastTextChange();
    }

    auto IsTextChanged() const
    {
        return editor.IsTextChanged();
    }
    bool ShouldBuildCallgraph() const
    {
        return should_build_callgraph;
    }
    void CallGraphBuilt() {
        should_build_callgraph = false;
    }
    const std::string SourceCode() const
    {
        return editor.GetText();
    }
    void Draw() {
        //*******************
        // KEY EVENTS
        //*******************
        // NEW
        if(io.KeysDown['N'] && io.KeyCtrl)
        {
            is_clicked_NEW = true;
        }
        // OPEN
        if(io.KeysDown['O'] && io.KeyCtrl)
        {
            is_clicked_OPEN = true;
        }
        // SAVE
        if(io.KeysDown['S'] && io.KeyCtrl)
        {
            bt_Save = true;
        }
        // SAVE AS
        if(io.KeysDown['S'] && io.KeyShift && io.KeyCtrl)
        {
            bt_Save = true;
            unsaved = true;
            if(!filename.empty())
                restore_filename = filename;
            filename.clear();

        }
        // EXIT
        if(io.KeysDown['Q'] && io.KeyCtrl)
        {
            glfwSetWindowShouldClose(main_window.Window(), GLFW_TRUE);
        }
        //*******************
        //SOURCE CODE WINDOW
        //*******************
        {

            if(!ImGui::Begin("SOURCE CODE", show_source_code_window, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse))
            {
                ImGui::End();
                return;
            }

            if(ImGui::BeginMenuBar())
            {
                if(ImGui::BeginMenu("File"))
                {
                    if(ImGui::MenuItem("New", "Ctrl+N"))
                    {
                        is_clicked_NEW = true;
                    }
                    if(ImGui::MenuItem("Open", "Ctrl+O"))
                    {
                        is_clicked_OPEN = true;
                    }
                    if(ImGui::MenuItem("Save", "Ctrl+S"))
                    {
                        bt_Save = true;
                    }
                    if(ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                    {
                        bt_Save = true;
                        unsaved = true;
                        if(!filename.empty())
                            restore_filename = filename;
                        filename.clear();                        
                    }
                    if(ImGui::MenuItem("Exit", "Ctrl+Q"))
                    {
                        glfwSetWindowShouldClose(main_window.Window(), GLFW_TRUE);
                    }
                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu("Edit"))
                {
                    if(ImGui::MenuItem("Undo", "Ctrl+Z"))
                    {
                        editor.Undo();
                    }
                    if(ImGui::MenuItem("Redo", "Ctrl+Y"))
                    {
                        editor.Redo();
                    }
                    if(ImGui::MenuItem("Cut", "Ctrl+X"))
                    {
                        editor.Cut();
                    }
                    if(ImGui::MenuItem("Copy", "Ctrl+C"))
                    {
                        editor.Copy();
                    }
                    if(ImGui::MenuItem("Paste", "Ctrl+V"))
                    {
                        editor.Paste();
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            // Writing file name instead of absoulte path
            auto position = filename.find_last_of('/');
            ImGui::Text("%s", filename.c_str() + (position == std::string::npos ? 0 : position + 1));
            ImGui::SameLine();
            
            if(!unsaved)
                unsaved = editor.IsTextChanged();

            ImGui::Text("%s", (unsaved ? "*" : ""));            

            editor.Render("Source Code Editor");                        

            ImGui::End();
        }
        //*******************
        //NEW BUTTON WINDOW
        //*******************        
        if(is_clicked_NEW)
        {            
            static std::string file = ".";
            file = fs::canonical(file);
            static bool write = false;

            draw_filebrowser("NEW", file, write, is_clicked_NEW);
            if(write)
            {
                filename = file;
                std::ofstream output_stream(filename);                
                if(!output_stream.is_open())
                {
                    std::cerr << "Failed to create file\n";
                    exit(1);
                }

                output_stream << editor.GetText();

                output_stream.close();
                is_clicked_NEW = false;
                write = false;
            }
        }

        //*******************
        //OPEN BUTTON WINDOW
        //*******************        
        if(is_clicked_OPEN)
        {            
            static bool write = false;
            static std::string file = ".";
            file = fs::canonical(file).string();

            draw_filebrowser("OPEN", file, write, is_clicked_OPEN); //  editor_util/editor_util.hpp
            if(write && fs::is_regular_file(file))
            {
                filename = file;
                std::ifstream in_file(filename);
                std::string _str;

                std::string buffer = "";                

                while(std::getline(in_file, _str))
                {                    
                    buffer.append(_str);
                    buffer.append("\n");
                }            

                //graph.BuildCallgraphFromSource(buffer);
                should_build_callgraph = true;
                editor.SetText(buffer);

                file = ".";
                write = false;
            }
        }

        //*******************
        //SAVE BUTTON WINDOW
        //*******************
        static bool save_prompt = false;
        static std::string file = ".";
        static bool write = false;        
        if(bt_Save)
        {                        
            file = fs::canonical(file);
            if(!unsaved) //IGNORE SAVE EVENT
            {}
            else if(!filename.empty())
            {
               save(filename.c_str(), editor.GetText());
               bt_Save = false;
               unsaved = false;
            }
            else
            {
                draw_filebrowser("SAVE", file, write, bt_Save);                
                if(write && (!fs::exists(file) || fs::is_regular_file(file)))
                {
                    if(!fs::exists(file))
                    {
                        filename = file;
                        
                        std::ofstream output_stream(filename);                
                        if(!output_stream.is_open())
                        {
                            std::cerr << "Failed to create file\n";
                            exit(1);
                        }   

                        //output_stream << editor.GetText();
                        
                        save(filename.c_str(), editor.GetText());

                        output_stream.close();
                        unsaved = false;
                        write = false;
                    }
                    else if(fs::is_empty(file))
                    {
                        filename = file;
                        save(filename.c_str(), editor.GetText());
                        unsaved = false;
                        write = false;
                    }
                    else
                    {
                        save_prompt = true;
                    }                    
                }
                if(!bt_Save && filename == "")
                    filename = restore_filename;            
            }
        }

        if(save_prompt)
        {
            ImGui::SetNextWindowSize(ImVec2(200, 90));
            if(ImGui::Begin("###save_prompt", &save_prompt))
            {

                ImGui::Text("Do you want to overwrite?");

                if(ImGui::Button("OK"))
                {
                    save_prompt = false;
                    bt_Save = false;
                    filename = file;
                    save(filename.c_str(), editor.GetText());
                    file = ".";
                    unsaved = false;
                    write = false;
                }
                ImGui::SameLine();
                if(ImGui::Button("Cancel"))
                {
                    save_prompt = false;
                    bt_Save = false;
                    file = ".";
                    write = false;
                }

                ImGui::End();
            }
        }

        if(editor.IsTextChanged()) {
            should_build_callgraph = true;
        }
    }
};

class WindowsToggleMenu {
private:
public:
    bool show_source_code_window = true;
    bool show_callgraph_window = true;
    bool show_ast_dump_window = false;
    bool show_function_list_window = false;

    void Draw() {
        ImGui::Begin("Windows toggle menu", __null, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::Checkbox("Source code", &show_source_code_window); ImGui::SameLine(150);
        ImGui::Checkbox("Callgraph", &show_callgraph_window); ImGui::SameLine(300);
        ImGui::Checkbox("AST dump", &show_ast_dump_window); ImGui::SameLine(450);
        ImGui::Checkbox("Function list", &show_function_list_window); ImGui::SameLine(600);

        ImGui::End();
    }

};

class FunctionListFilteringWindow {
private:
    ImGuiTextFilter filter;
    const clang_interface::CallGraph::NodesList* functions{nullptr};
    clang_interface::FunctionDecl* last_cliked{nullptr};
public:
    clang_interface::FunctionDecl* LastClickedFunction() const
    {
        return last_cliked;
    }
    void SetFunctionsList(const clang_interface::CallGraph::NodesList* func)
    {
        functions = func;
        last_cliked = nullptr;
    }
    void Draw() {
        ImGui::Begin("Functions Filtering List", __null, ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Filter usage:\n"
                    "  \"\"         display all lines\n"
                    "  \"xxx\"      display lines containing \"xxx\"\n"
                    "  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
                    "  \"-xxx\"     hide lines containing \"xxx\"");
        filter.Draw();

        if(functions) {
            for(const auto& function : *functions) {
                if(filter.PassFilter(function->NameAsString().c_str())) {
                    bool open = ImGui::TreeNode(function->NameAsString().c_str());
                    bool clicked = ImGui::IsItemClicked();

                    if(open)
                    {
                        ImGui::Text("Return type: %s", function->ReturnTypeAsString().c_str());
                        if(function->HasParams()) {
                            ImGui::Text("Params: ");
                            for(auto param = function->ParamBegin(); param != function->ParamEnd(); ++param)
                            {
                                ImGui::Text("\t%s %s", param->TypeAsString().c_str(), param->NameAsString().c_str());
                            }

                        } else {
                            ImGui::Text("Params: None");
                        }
                        ImGui::TreePop();
                    }
                    if(!open && clicked)
                    {
                        last_cliked = function.get();
                    }
                }
            }
        }

        ImGui::End();
    }
};

class FunctionASTDumpWindow {
private:
    clang_interface::FunctionDecl* function{nullptr};
public:
    void SetFunction(clang_interface::FunctionDecl* func)
    {
        function = func;
    }
    void Clear()
    {
        function = nullptr;
    }
    void Draw() {
        ImGui::Begin("Function AST Dump", __null, ImGuiWindowFlags_HorizontalScrollbar);
        if(function)
        {
            ImGui::Text("%s", function->ASTDump().c_str());
        }
        else
        {
            ImGui::Text("None");
        }
        ImGui::End();
    }
};

};


int main(int, char**)
{
    gui::MainWindow main_window;
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("imgui_util/misc/fonts/Cousine-Regular.ttf", 15.0f);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    gui::WindowsToggleMenu windows_toggle_menu;

    gui::SourceCodePanel source_code_panel(io, main_window, &windows_toggle_menu.show_source_code_window);

    clang_interface::ASTUnit ast_unit;
    clang_interface::CallGraph call_graph;
    gui::FunctionListFilteringWindow functions_filtering_window;
    gui::FunctionASTDumpWindow function_ast_dump_window;

    GraphGui::GraphGui graph(&io, &source_code_panel.Editor());
    while (!glfwWindowShouldClose(main_window.Window()))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        windows_toggle_menu.Draw();

        if(io.KeyShift && io.KeyCtrl && io.KeysDown['F'])
        {
            graph.focus_node(source_code_panel.Editor().GetSelectedText());
        }

        if(source_code_panel.SecondsSinceLastTextChange() == 2 && source_code_panel.ShouldBuildCallgraph())
        {
            function_ast_dump_window.Clear();
            auto new_ast_unit = clang_interface::BuildASTFromSource(source_code_panel.SourceCode());
            call_graph = clang_interface::ExtractCallGraphFromAST(new_ast_unit);
            ast_unit = std::move(new_ast_unit);
            graph.BuildCallGraph(call_graph);
            source_code_panel.CallGraphBuilt();
            functions_filtering_window.SetFunctionsList(&call_graph.nodes);

        }

        if(windows_toggle_menu.show_source_code_window) {
            source_code_panel.Draw();
        }

        if(windows_toggle_menu.show_function_list_window) {
            functions_filtering_window.Draw();
        }

        if(windows_toggle_menu.show_ast_dump_window) {
            function_ast_dump_window.SetFunction(functions_filtering_window.LastClickedFunction());
            function_ast_dump_window.Draw();
        }

        if(windows_toggle_menu.show_callgraph_window) {
            graph.draw(functions_filtering_window.LastClickedFunction());
        }
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(main_window.Window(), &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(main_window.Window());
    }


    return 0;
}
