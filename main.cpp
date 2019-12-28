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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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

struct imgui
{

};

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
        window = glfwCreateWindow(1280, 720, "CallGraph", NULL, NULL);
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
};


const static float EDITOR_GRAPH_RATIO = 0.35;  

int main(int, char**)
{
    gui::MainWindow main_window;
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("imgui_util/misc/fonts/Cousine-Regular.ttf", 15.0f);

    // Our state
    bool is_clicked_NEW = false;
    bool is_clicked_OPEN = false;
    bool bt_Save = false;
    bool key_event_new = false;
    bool key_event_open = false;
    bool key_event_save = false;
    bool unsaved = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::string filename;

    TextEditor editor;
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());

    std::string buffer;



    GraphGui::GraphGui graph(&io, &editor);


    while (!glfwWindowShouldClose(main_window.Window()))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

            ImGui::PushClipRect(ImVec2(100, 100), ImVec2(200, 200), true);

        //*******************
        // KEY EVENTS
        //*******************
        // NEW
        if(io.KeysDown['N'] && io.KeyCtrl)
        {
            key_event_new = true;
        }
        // OPEN
        if(io.KeysDown['O'] && io.KeyCtrl)
        {
            key_event_open = true;
        }
        // SAVE
        if(io.KeysDown['S'] && io.KeyCtrl)
        {
            key_event_save = true;
        }
        // SAVE AS
        if(io.KeysDown['S'] && io.KeyShift && io.KeyCtrl)
        {
            key_event_save = true;
            filename.clear();

        }
        // EXIT
        if(io.KeysDown['Q'] && io.KeyCtrl)
        {
            glfwSetWindowShouldClose(main_window.Window(), GLFW_TRUE);
        }

        float graph_size_x = io.DisplaySize.x*(1 - EDITOR_GRAPH_RATIO)-30;
        float graph_size_y = io.DisplaySize.y-20;
        float editor_size_x = io.DisplaySize.x*EDITOR_GRAPH_RATIO;
        float editor_size_y = io.DisplaySize.y-20;
        
        //*******************
        //GENERATED GRAPH WINDOW
        //*******************
        {
            ImGui::SetNextWindowPos(ImVec2(editor_size_x+25, 10));
            ImGui::SetNextWindowSize(ImVec2(graph_size_x, graph_size_y));
            ImGui::Begin("GENERATED CALLGRAPH", __null, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

            
            graph.set_window(ImGui::GetCurrentWindow());
            //PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect);
            graph.draw();
            //graph.focus_node(std::string("base::base()"));

            ImGui::End();
            ImGui::PopClipRect();
        }

        //*******************
        //SOURCE CODE WINDOW
        //*******************
        {   
            ImGui::SetNextWindowPos(ImVec2(15, 10));
            ImGui::SetNextWindowSize(ImVec2(editor_size_x, editor_size_y));
            //ImGui::SetNextWindowFocus();
            ImGui::Begin("SOURCE CODE", __null, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse);
            
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
            
            ImGui::Text("%s", filename.c_str());
            ImGui::SameLine();
            ImGui::Text("%s", (unsaved ? "*" : ""));

            size_t written = buffer.length(); // IF BUFFER HAS CHANGED VIA TEXT EDITOR

            editor.Render("Source Code Editor");
            buffer = editor.GetText();
            
            if(written != buffer.length())
                unsaved = true;

            
            ImGui::End();
        }

        //*******************
        //JUMP TO SELECTED NODE
        //*******************
        if(io.KeyShift && io.KeyCtrl && io.KeysDown['F'])
        {
            graph.focus_node(editor.GetSelectedText());
        }

        //*******************
        //NEW BUTTON WINDOW
        //*******************
        is_clicked_NEW |= key_event_new;
        if(is_clicked_NEW)
        {
            key_event_new = false;
            static std::string file = ".";
            file = fs::canonical(file);
            static bool write = false;

            draw_filebrowser("NEW", file, write, is_clicked_NEW);
            if(write)
            {
                filename = file;
                if(creat(filename.c_str(), 0644) == -1)
                {
                    std::cerr << "Failed to create file\n";
                    exit(1);
                }
                is_clicked_NEW = false;
                write = false;
            }
        }

        //*******************
        //OPEN BUTTON WINDOW
        //*******************
        is_clicked_OPEN |= key_event_open;
        if(is_clicked_OPEN)
        {
            key_event_open = false;
            static bool write = false;
            static std::string file = ".";
            file = fs::canonical(file).string();

            draw_filebrowser("OPEN", file, write, is_clicked_OPEN); //  editor_util/editor_util.hpp
            if(write && fs::is_regular_file(file))
            {
                filename = file;
                std::ifstream in_file(filename);
                std::string _str;

                buffer.clear();

                while(std::getline(in_file, _str))
                {
                    buffer.append(_str);
                    buffer.append("\n");
                }
                
                graph.BuildCallgraphFromSource(buffer);
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
        bt_Save |= key_event_save;
        if(bt_Save)
        {
            key_event_save = false;
            // file = ".";
            file = fs::canonical(file);
            if(!unsaved) //IGNORE SAVE EVENT
            {}
            else if(!filename.empty())
            {
               save(filename.c_str(), buffer);
               bt_Save = false;
               unsaved = false; 
            }
            else
            {
                draw_filebrowser("SAVE", file, write, bt_Save);
                    //draw_save(file, src_code_buffer, src_code_buffer.length, bt_Save); //  editor_util/editor_util.hpp
                if(write && (!fs::exists(file) || fs::is_regular_file(file)))
                {
                    if(!fs::exists(file))
                    {
                        filename = file;
                        if(creat(filename.c_str(), 0644) == -1)
                        {
                            std::cerr << "Failed to create file\n";
                            exit(1);
                        }
                        save(filename.c_str(), buffer);
                        unsaved = false;
                        write = false;
                    }
                    else if(fs::is_empty(file))
                    {
                        filename = file;
                        save(filename.c_str(), buffer);
                        unsaved = false;
                        write = false;
                    }
                    else
                    {
                        save_prompt = true;
                    }  
                }
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
                    save(filename.c_str(), buffer);
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
