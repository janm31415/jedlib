#pragma once

#include "namespace.h"
#include <filesystem>
#include <string>
#include <vector>

JEDLIB_BEGIN

std::filesystem::path get_executable_path();
std::filesystem::path get_executable_folder();

std::string get_file_in_executable_path(const std::string& filename);

bool file_exists(const std::string& filename);
bool is_directory(const std::string& directory);

std::vector<std::string> get_files_from_directory(const std::string& d, bool include_subfolders);
std::vector<std::string> get_subdirectories_from_directory(const std::string& d, bool include_subfolders);
std::string get_filename(const std::string& path);
std::string get_extension(const std::string& filename);

JEDLIB_END
