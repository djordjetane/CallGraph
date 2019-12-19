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

//GraphGui::GraphGui graph(nullptr);

struct imgui
{
    const char* glsl_version;
    GLFWwindow* window;
    bool err;
};

imgui imgui_create()
{
    imgui result;
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) 
    {
        exit(EXIT_FAILURE);
    }
        

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    result.glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    result.glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    result.window = glfwCreateWindow(1280, 720, "CallGraph", NULL, NULL);
    if (result.window == NULL)
    {
        exit(EXIT_FAILURE);
    }
    
    glfwMakeContextCurrent(result.window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    result.err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    result.err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    result.err = gladLoadGL() == 0;
#else
    result.err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (result.err)
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
    ImGui_ImplGlfw_InitForOpenGL(result.window, true);
    ImGui_ImplOpenGL3_Init(result.glsl_version);
    return result;
}

void imgui_destroy(imgui* to_destroy)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(to_destroy->window);
    glfwTerminate();
}


struct LinuxCommands
{
	const std::string compile_template{"clang++-8 -g -O0 -emit-llvm {} -c -std=c++17"};
	std::string cleanup{"rm *.bc callgraph.dot"};
	std::string compile;
	std::string link{"llvm-link-8 *.bc -o single.bc"};
	std::string analyze{"opt-8 -analyze -std-link-opts -dot-callgraph single.bc"};
	std::string demangle{"cat callgraph.dot | c++filt | cat > out.txt"};
	
	void SetFileToAnalyze(const std::string& filename)
	{
		compile = compile_template;
		compile.replace(compile.find("{}"), 2, filename);
		std::cout << compile << '\n';
	}
	
	int RunCommands()
	{
		system(cleanup.c_str());
		system(compile.c_str());
		system(link.c_str());
		system(analyze.c_str());
		system(demangle.c_str());
		system(cleanup.c_str());
		return 0;
	}
};



int RunParserCommand(const LinuxCommands& commands)
{
	int result = 0;

	return result;
}

int main(int, char**)
{
    imgui gui = imgui_create();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("imgui_util/misc/fonts/Cousine-Regular.ttf", 15.0f);

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

    static std::string buffer;

    ParserFunctionCallGraph call_graph = ExtractCallGraphFromFile("out.txt");
    GraphGui::GraphGui graph(call_graph, io);
    // Main loop
	LinuxCommands commands;
    while (!glfwWindowShouldClose(gui.window))
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
            strcpy(filename, "");
        }
        // EXIT
        if(io.KeysDown['Q'] && io.KeyCtrl)
        {
            glfwSetWindowShouldClose(gui.window, GLFW_TRUE);
        }

        //*******************
        //GENERATED GRAPH WINDOW
        //*******************
        {
            ImGui::SetNextWindowPos(ImVec2(470, 10));
            ImGui::SetNextWindowSize(ImVec2(800, 705));
            ImGui::Begin("GENERATED CALLGRAPH", __null, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

            
            graph.set_window(ImGui::GetCurrentWindow());
            //PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect);
            graph.draw();

            ImGui::End();
            ImGui::PopClipRect();
        }

        //*******************
        //SOURCE CODE WINDOW
        //*******************
        {   
            ImGui::SetNextWindowPos(ImVec2(15, 10));
            ImGui::SetNextWindowSize(ImVec2(450, 705));
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
                        strcpy(filename, "");
                    }
                    if(ImGui::MenuItem("Exit", "Ctrl+Q"))
                    {
                        glfwSetWindowShouldClose(gui.window, GLFW_TRUE);
                    }
                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu("Edit"))
                {
                    if(ImGui::MenuItem("Undo", "Ctrl+Z"))
                    {}
                    if(ImGui::MenuItem("Redo", "Ctrl+Y"))
                    {}
                    if(ImGui::MenuItem("Cut", "Ctrl+X"))
                    {}
                    if(ImGui::MenuItem("Copy", "Ctrl+C"))
                    {}
                    if(ImGui::MenuItem("Paste", "Ctrl+V"))
                    {}

                    ImGui::EndMenu();
                }
                
                ImGui::EndMenuBar();
            }

            // Writing file name instead of absoulte path
            char* name = NULL;
            name = strrchr(filename, '/');
            if(name == NULL)
                name = &filename[0];
            else
                name++;
            
            ImGui::Text("%s", name);
            ImGui::SameLine();
            ImGui::Text("%s", (unsaved ? "*" : ""));

            size_t written = buffer.length(); // IF BUFFER HAS CHANGED VIA TEXT EDITOR

            ImGui::BeginChild("###line_numbers", ImVec2(430,615), ImGuiWindowFlags_NoCollapse);
                
            int number_of_lines = 0;
            number_of_lines = std::count(buffer.cbegin(), buffer.cend(), '\n');

            std::string line_nums;
            for(int i = 1; i <= number_of_lines+1; i++)
            {
                line_nums.append(std::to_string(i));
                line_nums.append("\n");
            }
            ImGui::TextUnformatted(line_nums.c_str());
            
            ImGui::SameLine();
            ImGui::InputTextMultiline("###input_src", &buffer, ImVec2(410, 590)
                                    , ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_Multiline);
            if(written != buffer.length())
                unsaved = true;

            ImGui::EndChild();
            ImGui::End();
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

            draw_filebrowser(NEW, file, write, is_clicked_NEW);
            if(write)
            {
                strcpy(filename, file.c_str());
                if(creat(filename, 0644) == -1)
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

            draw_filebrowser(OPEN, file, write, is_clicked_OPEN); //  editor_util/editor_util.hpp
            if(write && fs::is_regular_file(file))
            {
                strcpy(filename, file.c_str());
                std::ifstream in_file(filename);
                std::string _str;

                buffer.clear();

                while(std::getline(in_file, _str))
                {
                    buffer.append(_str);
                    buffer.append("\n");
                }
                
				commands.SetFileToAnalyze(filename);
				commands.RunCommands();
				call_graph = ExtractCallGraphFromFile("out.txt");
				graph = GraphGui::GraphGui(call_graph, io);
				
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
               save(filename, buffer);
               bt_Save = false;
               unsaved = false; 
            }
            else
            {
                draw_filebrowser(SAVE, file, write, bt_Save);
                    //draw_save(file, src_code_buffer, src_code_buffer.length, bt_Save); //  editor_util/editor_util.hpp
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
                        save(filename, buffer);
                        unsaved = false;
                        write = false;
                    }
                    else if(fs::is_empty(file))
                    {
                        strcpy(filename, file.c_str());
                        save(filename, buffer);
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
                    save(filename, buffer);
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
        glfwGetFramebufferSize(gui.window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(gui.window);
    }

    imgui_destroy(&gui);

    return 0;
}
