#include "file_utils.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#endif

JEDLIB_BEGIN

std::filesystem::path get_executable_path()
{
#if defined(_WIN32)
  wchar_t buffer[MAX_PATH];
  DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  if (len == 0) {
    throw std::runtime_error("GetModuleFileNameW failed");
  }
  return std::filesystem::path(buffer);
  
#elif defined(__APPLE__)
  uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size); // get required size
  std::string buffer(size, '\0');
  if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
    throw std::runtime_error("_NSGetExecutablePath failed");
  }
  return std::filesystem::weakly_canonical(std::filesystem::path(buffer.c_str()));
  
#elif defined(__linux__)
  char buffer[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (len == -1) {
    throw std::runtime_error("readlink(/proc/self/exe) failed");
  }
  buffer[len] = '\0';
  return std::filesystem::path(buffer);
  
#else
#error Unsupported platform
#endif
}

std::filesystem::path get_executable_folder()
{
  return get_executable_path().parent_path();
}


std::string get_file_in_executable_path(const std::string& filename) {
  std::filesystem::path path = get_executable_folder() / filename;
  return path.string();
}

bool file_exists(const std::string& filename) {
  return std::filesystem::exists(filename);
  }

bool is_directory(const std::string& directory) {
  return std::filesystem::is_directory(directory);
  }

std::vector<std::string> get_subdirectories_from_directory(const std::string& d, bool include_subfolders)
  {
  namespace fs = std::filesystem;

  std::vector<std::string> directories;
  fs::path root(d);

  if (!fs::exists(root) || !fs::is_directory(root))
    return directories;

  if (include_subfolders)
    {
    for (const auto& entry : fs::recursive_directory_iterator(root))
      {
      if (entry.is_directory())
        {
        const auto name = entry.path().filename().string();
        if (!name.empty() && name.front() != '.')
          directories.push_back(entry.path().string());
        }
      }
    }
  else
    {
    for (const auto& entry : fs::directory_iterator(root))
      {
      if (entry.is_directory())
        {
        const auto name = entry.path().filename().string();
        if (!name.empty() && name.front() != '.')
          directories.push_back(entry.path().string());
        }
      }
    }

  return directories;
  }

std::string get_filename(const std::string& path)
  {
  return std::filesystem::path(path).filename().string();
  }

std::vector<std::string> get_files_from_directory(const std::string& d, bool include_subfolders)
  {
  namespace fs = std::filesystem;

  std::vector<std::string> files;
  fs::path root(d);

  if (!fs::exists(root) || !fs::is_directory(root))
    return files;

  if (include_subfolders)
    {
    for (const auto& entry : fs::recursive_directory_iterator(root))
      {
      if (entry.is_regular_file() || entry.is_symlink())
        files.push_back(entry.path().string());
      }
    }
  else
    {
    for (const auto& entry : fs::directory_iterator(root))
      {
      if (entry.is_regular_file() || entry.is_symlink())
        files.push_back(entry.path().string());
      }
    }

  return files;
  }
  
std::string get_extension(const std::string& filename) {
  std::string ext = std::filesystem::u8path(filename).extension().u8string();
  return ext;
}

// The path of 'p' relative to project root 'root' if 'p' lives inside it, else
// an empty string. Used both to decide whether an editor belongs to the project
// and to compute the path stored in the session file.
std::string relInProject(const std::string& root, const std::string& p)
  {
  if (root.empty() || p.empty())
    return {};
  std::error_code ec;
  std::filesystem::path rel = std::filesystem::path(p).lexically_relative(root);
  std::string s = rel.generic_string();
  if (s.empty() || s == "." || s == ".." || s.rfind("../", 0) == 0)
    return {}; // not inside the project root
  return s;
  }

JEDLIB_END
