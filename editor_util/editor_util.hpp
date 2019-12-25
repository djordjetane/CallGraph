#ifndef EDITOR_UTIL__HPP
#define EDITOR_UTIL__HPP

#include <vector>
#include <string>
#include "../imgui_util/imgui.h"


void save(const char* new_filename, const std::string& buffer);
std::vector<std::string> get_directory_files(const std::string& pathname);
void draw_filebrowser(const char* action, std::string& filename, bool& write, bool& is_clicked_OPEN);

#endif //EDITOR_UTIL__HPP