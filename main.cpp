// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#include "imgui_util/imgui.h"
#include "imgui_util/glfw_opengl3/imgui_impl_glfw.h"
#include "imgui_util/glfw_opengl3/imgui_impl_opengl3.h"
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
#include "graph.cpp"

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

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "CallGraph", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    static bool is_clicked_NEW = false;
    static bool is_clicked_OPEN = false;
    static bool bt_Save = false;
    static bool key_event_new = false;
    static bool key_event_open = false;
    static bool key_event_save = false;
    static bool unsaved = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static char filename[255];
    strcpy(filename, "");
    static size_t buffer_size = 1 << 20;
    static char* src_code_buffer = (char*)malloc(buffer_size);
    const size_t alloc_step = 1 << 8;
    strcpy(src_code_buffer, "");

    // Main loop
    while (!glfwWindowShouldClose(window))
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

        //*******************
        //SOURCE CODE WINDOW
        //*******************
        {   
            ImGui::SetNextWindowPos(ImVec2(15, 11));
            ImGui::SetNextWindowSize(ImVec2(450, 700));
            ImGui::Begin("SOURCE CODE", __null, ImGuiWindowFlags_MenuBar);
            
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
                    if(ImGui::MenuItem("Save As..."))
                    {
                        bt_Save = true;
                    }
                    ImGui::EndMenu();
                }
                
                ImGui::EndMenuBar();
            }

            if(strlen(src_code_buffer) == buffer_size)
            {
                buffer_size += alloc_step;
                src_code_buffer = (char*)realloc(src_code_buffer, buffer_size);
                if(src_code_buffer == NULL)
                {
                    std::cerr << "Realloc failed\n";
                    exit(1);
                }
            }
            size_t written = strlen(src_code_buffer);
            ImGui::InputTextMultiline("###input_src", src_code_buffer, buffer_size, ImVec2(420, 630), ImGuiInputTextFlags_AllowTabInput);
            if(written == strlen(src_code_buffer))
                unsaved = true;
            else
                unsaved = false;

            ImGui::End();
        }

        //*******************
        //GENERATED GRAPH WINDOW
        //*******************
        {
            ImGui::SetNextWindowPos(ImVec2(500, 11));
            ImGui::SetNextWindowSize(ImVec2(800, 800));
            ImGui::Begin("GENERATED CALLGRAPH");

            /*
            // PROTOTIP 
            test::Graph test_graph(123);
            test::Node* n1 = new test::Node(ImVec2(520, 20), 100, "Node1");
            test::Node* n2 = new test::Node(ImVec2(530, 20), 101, "Node2");
            test::Node* n3 = new test::Node(ImVec2(540, 20), 102, "Node3");
            test::Node* n4 = new test::Node(ImVec2(540, 20), 103, "Node4");
            n1->add_edge(n2);
            n2->add_edge(n3);
            n2->add_edge(n4);

            ImGuiWindow* wwindow = ImGui::GetCurrentWindow();
            test_graph.nodes.push_back(n1);
            test_graph.nodes.push_back(n2);
            test_graph.nodes.push_back(n3);
            test_graph.nodes.push_back(n4);
            test_graph.draw(wwindow); 
            */

            GraphGui::GraphGui graph(ImGui::GetCurrentWindow());
            graph.draw();

            ImGui::End();
        }

        //*******************
        //NEW BUTTON WINDOW
        //*******************
        is_clicked_NEW |= key_event_new;
        if(is_clicked_NEW)
        {
            key_event_new = false;
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
            //file.pop_back(); file.pop_back();

            //std::cout << file << std::endl;
            draw_filebrowser(OPEN, file, write, is_clicked_OPEN); //  editor_util/editor_util.hpp
            if(write && fs::is_regular_file(file))
            {
                if(fs::file_size(file) > buffer_size)
                {
                    src_code_buffer = (char*)realloc(src_code_buffer, (size_t)fs::file_size(file) + alloc_step);
                    if(src_code_buffer == NULL)
                    {
                        std::cerr << "Realloc failed\n";
                        exit(1);
                    }
                    buffer_size = (size_t)fs::file_size(file);
                }
                strcpy(filename, file.c_str());
                std::ifstream in_file(filename);
                std::string _str;

                strcpy(src_code_buffer, "");

                while(std::getline(in_file, _str))
                {
                    strcat(src_code_buffer, _str.c_str());
                    strcat(src_code_buffer, "\n");
                }

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
            else if(strcmp(filename, "") != 0)
            {
               save(filename, src_code_buffer, strlen(src_code_buffer));
               bt_Save = false;
               unsaved = false; 
            }
            else
            {
                draw_filebrowser(SAVE, file, write, bt_Save);
                    //draw_save(file, src_code_buffer, strlen(src_code_buffer), bt_Save); //  editor_util/editor_util.hpp
                if(write && (!fs::exists(file) || fs::is_regular_file(file)))
                {
                    if(!fs::exists(file))
                    {
                        strcpy(filename, file.c_str());
                        if(creat(filename, 0644) == -1)
                        {
                            std::cerr << "Failed to create file\n";
                            exit(1);
                        }
                        save(filename, src_code_buffer, strlen(src_code_buffer));
                        unsaved = false;
                        write = false;
                    }
                    else if(fs::is_empty(file))
                    {
                        strcpy(filename, file.c_str());
                        save(filename, src_code_buffer, strlen(src_code_buffer));
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
                    strcpy(filename, file.c_str());
                    save(filename, src_code_buffer, strlen(src_code_buffer));
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
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    free(src_code_buffer);

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
