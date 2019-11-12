#ifndef EDITOR_UTIL__HPP
#define EDITOR_UTIL__HPP

#include <vector>
#include <string>
#include "../imgui_util/imgui.h"

enum FBAction
{
    NEW, OPEN, SAVE
};

void save(const char* new_filename, const char* buffer, const int buffer_size);
std::vector<std::string> get_directory_files(const std::string& pathname);
void draw_filebrowser(const FBAction& action, std::string& filename, bool& write, bool& is_clicked_OPEN);
void FBAction_to_string(const FBAction& action);

#endif //EDITOR_UTIL__HPP