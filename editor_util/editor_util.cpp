#include "editor_util.hpp"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <array>

namespace fs = std::filesystem;

void save(const char* new_filename, const char* buffer, const int buffer_size)
{
    std::ofstream outfile(new_filename, std::ios::out | std::ios::binary);
    outfile.write(buffer, buffer_size);

    outfile.close();   
}

std::vector<std::string> get_directory_files(const std::string& pathname)
{   
    std::string path(pathname);
    std::vector<std::string> res;
    res.push_back("<= BACK");
    for(auto& entry : fs::directory_iterator(pathname))
    {   
        path = fs::canonical(entry).string();
        if(fs::is_directory(path))
        {
            res.push_back(fs::canonical(path));
        }
    
        size_t pos = path.find_last_of('.');
        if(pos == path.npos)
            continue;
        std::string extension = path.substr(pos);
        if(extension == ".cpp" || extension == ".c" || extension == ".h" || extension == ".hpp")
        {   
            pos = path.find_last_of('/');
            res.push_back(path.substr(pos+1));
        }
    }

    std::sort(std::begin(res)+1, std::end(res));

    return res;
}


static bool open_prompt = false;
void draw_save(std::string& filename, const char* buffer, const int buffer_size, bool& bt_save)
{
    ImGui::SetNextWindowSize(ImVec2(500, 400));
    
    if(ImGui::Begin("Save As...", &bt_save))
    {   
        ImGui::Text(("[D] " + filename + "\n\n").c_str());
            std::vector<std::string> files = get_directory_files(filename);
            
            for(auto& file : files)
            {
                if(ImGui::Selectable(file.c_str()))
                {
                    if(file == "<= BACK")
                        filename = fs::canonical(filename).parent_path();
                    else    
                        filename = fs::canonical(fs::path(file));
                    if(!fs::is_directory(filename))
                    {
                        if(!fs::is_empty(filename))
                        {
                            
                        }

                        else
                        {
                            save(filename.c_str(), buffer, buffer_size);
                        }

                        bt_save = false;
                    }
                    
                }
            }

        ImGui::End();
    }
}


//FIXIT what():  filesystem error: cannot make canonical path: No such file or directory [imstb_textedit.h]
void draw_open(std::string& filename, char*src_code_buffer, u_int32_t buffer_size, bool& is_clicked_OPEN)
{
    ImGui::SetNextWindowSize(ImVec2(500, 400));
            ImGui::Begin("Open", &is_clicked_OPEN);

            ImGui::Text(("[D] " + filename + "\n\n").c_str());
            std::vector<std::string> files = get_directory_files(filename);
            
            for(auto& file : files)
            {
                if(ImGui::Selectable(file.c_str()))
                {
                    if(file == "<= BACK")
                        filename = fs::canonical(filename).parent_path();
                    else    
                        filename = fs::canonical(fs::path(file));
                    if(!fs::is_directory(filename))
                    {
                        std::ifstream in_file(filename);
                        std::string _str;
                        strcpy(src_code_buffer, "");
                        while(std::getline(in_file, _str))
                        {
                            strcat(src_code_buffer, _str.c_str());
                            strcat(src_code_buffer, "\n");
                        }
                        is_clicked_OPEN = false;
                    }
                    
                }
            }

    ImGui::End();
}
