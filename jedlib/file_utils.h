#pragma once

#include <filesystem>
#include <string>
#include <vector>

std::filesystem::path getExecutablePath();
std::filesystem::path getExecutableFolder();

std::string get_file_in_executable_path(const std::string& filename);

bool file_exists(const std::string& filename);
bool is_directory(const std::string& directory);

std::vector<std::string> get_files_from_directory(const std::string& d, bool include_subfolders);
std::vector<std::string> get_subdirectories_from_directory(const std::string& d, bool include_subfolders);
std::string get_filename(const std::string& path);