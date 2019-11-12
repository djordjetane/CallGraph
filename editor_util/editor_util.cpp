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
    std::ofstream outfile(new_filename, std::fstream::out);
    if(!outfile.is_open())
    {
        std::cerr << "Failed to open file\n";
        exit(1);
    }
    outfile.write(buffer, buffer_size);

    outfile.close();   
}

static char action_string[11];

void FBAction_to_string(const FBAction& action)
{
    switch (action)
    {
    case NEW:
        strcpy(action_string, "NEW");
        break;
    case OPEN:
        strcpy(action_string, "OPEN");
        break;
    case SAVE:
        strcpy(action_string, "SAVE AS...");
        break;
    default:
        break;
    }
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
    
        // filter extension .cpp .hpp .h
        else
        {
            if(fs::path(path).extension() == ".cpp" || fs::path(path).extension() == ".hpp" || fs::path(path).extension() == ".h")
                res.push_back(path);
        }
    }

    std::sort(std::begin(res)+1, std::end(res));

    return res;
}

void draw_filebrowser(const FBAction& action, std::string& filename, bool& write, bool& is_clicked_OPEN)
{
    static char new_name[64] = "";
    ImGui::SetNextWindowSize(ImVec2(500, 400));
    FBAction_to_string(action);
    if(ImGui::Begin(action_string, &is_clicked_OPEN))
    {
        if(!fs::is_directory(filename))
        {
            // getting name of file
            int pos = filename.find_last_of("/");
            if(pos == filename.npos)
                pos = 0;
            else
                pos++;
            std::string name = filename.substr(pos);
            strcpy(new_name, name.c_str());
            filename = fs::path(filename).parent_path();
            ImGui::Text(("[D] " + filename + "\n\n").c_str());
        }
        else
        {
            ImGui::Text(("[D] " + filename + "\n\n").c_str());
            std::vector<std::string> files = get_directory_files(filename);
            
            for(auto& file : files)
            {
                // getting name of file
                int pos = file.find_last_of("/");
                if(pos == file.npos)
                    pos = 0;
                else
                    pos++;
                std::string name = file.substr(pos);
                if(ImGui::Selectable(name.c_str()))
                {
                    if(file == "<= BACK")
                        filename = fs::canonical(filename).parent_path();
                    else    
                        filename = fs::canonical(fs::path(file));
                    if(fs::is_regular_file(filename))
                    {
                        strcpy(new_name, name.c_str());
                    }
                }
            }
        }
            ImGui::InputText("###input_filename", new_name, 64);
            ImGui::SameLine();
            ImGui::Text(".cpp, .hpp, .h");
            if(ImGui::Button("OK"))
            {
                if(fs::is_directory(filename))
                {
                    if(strcmp(new_name, "") == 0)
                    {
                        ImGui::Text("Please enter file name\n");
                    }
                    else
                    {
                        filename.append("/");
                        filename.append(new_name);
                        is_clicked_OPEN = false;
                        write = true;
                    }
                    
                }
                else
                {
                    ImGui::Text("File already exists\n");
                }
            }
            if(ImGui::Button("Cancel"))
            {
                is_clicked_OPEN = false;
            }
        

        ImGui::End();
    }
}