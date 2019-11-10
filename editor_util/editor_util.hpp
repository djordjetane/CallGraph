#ifndef EDITOR_UTIL__HPP
#define EDITOR_UTIL__HPP

#include <vector>
#include <string>
#include "../imgui_util/imgui.h"

void save(const char* new_filename, const char* buffer, const int buffer_size);
std::vector<std::string> get_directory_files(const std::string& pathname);
void draw_save(const char* filename, const char* buffer, const int buffer_size, bool& bt_save);
void draw_open(std::string& filename, char*src_code_buffer, u_int32_t buffer_size, bool& is_clicked_OPEN);

#endif //EDITOR_UTIL__HPP