// dear imgui: standalone example application for GLFW + OpenGL 3, using
// programmable pipeline If you are new to dear imgui, see examples/README.txt
// and documentation at the top of imgui.cpp. (GLFW is a cross-platform general
// purpose library for handling windows, inputs, OpenGL/Vulkan graphics context
// creation, etc.)

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "clang_interface.h"
#include "graph.hpp"
#include "gui.hpp"
#include "keyboard.hpp"

int main(int, char**) {
  gui::MainWindow main_window;
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF("libs/imgui/misc/fonts/Cousine-Regular.ttf",
                               15.0f);

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  gui::WindowsToggleMenu windows_toggle_menu;

  gui::SourceCodePanel source_code_panel(
      io, main_window, &windows_toggle_menu.show_source_code_window);

  clang_interface::ASTUnit ast_unit;
  clang_interface::CallGraph call_graph;
  gui::FunctionListFilteringWindow functions_filtering_window(
      windows_toggle_menu.show_function_list_window);

  gui::FunctionASTDumpWindow function_ast_dump_window(
      windows_toggle_menu.show_ast_dump_window);

  gui::GraphGui graph(&io, &source_code_panel.Editor(),
                      windows_toggle_menu.show_callgraph_window);
  while (!glfwWindowShouldClose(main_window.Window())) {
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    windows_toggle_menu.Draw();

    if (io.KeyShift && io.KeyCtrl && io.KeysDown[keyboard::FKey]) {
      graph.focus_node(source_code_panel.Editor().GetSelectedText());
    }

    if (source_code_panel.SecondsSinceLastTextChange() == 2 &&
        source_code_panel.ShouldBuildCallgraph()) {
      function_ast_dump_window.Clear();
      std::string compiler_include_dir =
          "-I" + source_code_panel.DirectoryOfLastOpenedFile().string();
      auto new_ast_unit = clang_interface::BuildASTFromSource(
          source_code_panel.SourceCode(), {compiler_include_dir});
      call_graph = clang_interface::ExtractCallGraphFromAST(new_ast_unit);
      ast_unit = std::move(new_ast_unit);
      graph.BuildCallGraph(call_graph);
      source_code_panel.CallGraphBuilt();
      functions_filtering_window.SetFunctionsList(&call_graph.nodes);
    }

    if (windows_toggle_menu.show_source_code_window) {
      source_code_panel.Draw();
    }

    if (windows_toggle_menu.show_function_list_window) {
      functions_filtering_window.Draw();
    }

    if (windows_toggle_menu.show_ast_dump_window) {
      function_ast_dump_window.SetFunction(
          functions_filtering_window.LastClickedFunction());
      function_ast_dump_window.Draw();
    }

    if (windows_toggle_menu.show_callgraph_window) {
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
