#include "editor_util.hpp"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <climits>
#include <cstdlib>
#include "cstring"

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
    res.push_back("<-BACK");
    for(auto& entry : fs::directory_iterator(path))
    {   
        path = entry.path().string();
        if(fs::is_directory(entry))
        {
            size_t pos = path.find_last_of('/');
            res.push_back(path.substr(pos+1));
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

void draw_save(const char* filename, const char* buffer, const int buffer_size, bool& bt_save)
{
    ImGui::SetNextWindowSize(ImVec2(500, 400));
    
    if(bt_save && ImGui::Begin("Save As...", &bt_save))
    {   
        fs::path abs_path = fs::absolute(fs::path(filename));
        std::string real_path = abs_path.string();
        if(strcmp(filename, ".") == 0)
            real_path.pop_back();
        std::vector<std::string> files = get_directory_files(real_path);

        ImGui::Text(("[D] " + real_path + "\n\n").c_str());
        int i = 0;
        int selected = -1;
        for(const auto& file : files)
        {
            if(ImGui::Selectable(file.c_str()))
            {
                if(fs::is_directory(fs::path(file)))
                {
                    std::cout << file << std::endl;
                }
                selected = i;
            }
            i++;
        }

        if(ImGui::Button("OK"))
        {
            if(selected != -1)
                save(files[selected].c_str(), buffer, buffer_size);
            bt_save = false;
        }

        ImGui::End();
    }
}

void draw_open(std::string& filename, char*src_code_buffer, u_int32_t buffer_size, bool& is_clicked_OPEN)
{
    ImGui::SetNextWindowSize(ImVec2(500, 400));
            ImGui::Begin("Open", &is_clicked_OPEN);
            
            fs::path abs_path = fs::absolute(filename);
            if(filename == ".")
                filename = abs_path.string().substr(0, abs_path.string().size()-1);

            ImGui::Text(("[D] " + filename + "\n\n").c_str());
            std::vector<std::string> files = get_directory_files(filename);
            for(const auto& file : files)
            {
                if(ImGui::Selectable(file.c_str()))
                {
                    std::string real_name = filename.substr(filename.find_last_of('/')+1);
                    std::cout << real_name << std::endl;
                    if(file == "<-BACK")
                    {
                            filename.pop_back();
                            filename = filename.substr(0, filename.find_last_of('/'));
                    }

                    else if(fs::is_directory(fs::path(file)))
                        {
                            if(real_name == file)
                                continue;
                            filename = fs::absolute(fs::path(file));
                        }
                    else
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
