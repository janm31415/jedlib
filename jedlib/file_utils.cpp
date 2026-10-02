#include "file_utils.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#endif

#include <charconv>

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
  return std::filesystem::exists(filename) && !std::filesystem::is_directory(filename);
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
  std::filesystem::path p = std::filesystem::u8path(path);
  return p.filename().u8string();
}

std::string get_folder(const std::string& filename_utf8)
{
  std::filesystem::path p = std::filesystem::u8path(filename_utf8);
  return p.parent_path().u8string();
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

bool parse_int(std::string_view s, int& value) {
  if (s.empty()) return false;
  auto* begin = s.data();
  auto* end = s.data() + s.size();
  auto [ptr, ec] = std::from_chars(begin, end, value);
  return ec == std::errc{} && ptr == end;
}

bool parse_int(const std::wstring& ws, int& value) {
  std::string s(ws.begin(), ws.end());
  return parse_int(s, value);
}

bool parse_location(std::string_view input, ParsedLocation& out) {
  if (input.empty())
    return false;
  
  while (!input.empty() && input.back() == ':')
    input.remove_suffix(1); // remove trailing ':'
  
  int line = 0, column = 0;
  auto last_colon = input.rfind(':');
  std::string path_part;
  
  if (last_colon != std::string_view::npos) {
    
    auto second_last_colon = input.rfind(':', last_colon - 1);
    if (second_last_colon != std::string_view::npos) {
      
      path_part = input.substr(0, second_last_colon);
      std::string_view line_part = input.substr(second_last_colon + 1,
                                                last_colon - second_last_colon - 1);
      std::string_view column_part = input.substr(last_colon + 1);
      
      if (!parse_int(line_part, line) || !parse_int(column_part, column))
        return false;
      
    }
    else {
      
      path_part = input.substr(0, last_colon);
      std::string_view line_part = input.substr(last_colon + 1);
      if (!parse_int(line_part, line))
        return false;
    }
    
  }
  else {
    path_part = input;
  }
  
  if (path_part.empty())
    return false;
  
  int dummy;
  if (parse_int(path_part, dummy)) // path should not be an integer
    return false;
  
  if (line < 0 || column < 0)
    return false;
  
  while (!path_part.empty() && path_part.front() == '"')
    path_part.erase(path_part.begin());
  
  if (!path_part.empty() && path_part.back() == '"')
    path_part.pop_back();
  
  if (path_part.empty())
    return false;
  
  // Optional: syntactic path validation
  std::filesystem::path p(path_part);
  
  out = { p, line, column };
  return true;
}

bool looks_like_path(std::wstring s) {
  auto isSep = [](wchar_t c) {
    return c == L'/' || c == L'\\';
  };
  
  bool hasDriveLetter =
  s.size() >= 3 &&
  std::iswalpha(s[0]) &&
  s[1] == L':' &&
  isSep(s[2]);
  
  if (hasDriveLetter) {
    s = s.substr(3);
  }
  
  if (!s.empty() && s.back() == L':')
    s.pop_back(); // remove trailing ':'
  int line = 0, column = 0;
  auto last_colon = s.rfind(L':');
  std::wstring path_part;
  
  if (last_colon != std::wstring::npos) {
    
    auto second_last_colon = s.rfind(L':', last_colon - 1);
    if (second_last_colon != std::wstring::npos) {
      
      path_part = s.substr(0, second_last_colon);
      std::wstring line_part = s.substr(second_last_colon + 1,
                                                last_colon - second_last_colon - 1);
      std::wstring column_part = s.substr(last_colon + 1);
      
      if (!parse_int(line_part, line) || !parse_int(column_part, column))
        return false;
      
      std::swap(s, path_part);
    }
    else {
      
      path_part = s.substr(0, last_colon);
      std::wstring line_part = s.substr(last_colon + 1);
      if (!parse_int(line_part, line))
        return false;
      std::swap(s, path_part);
    }
    
  }

  if (s.empty()) return hasDriveLetter;
    
  // Reject embedded NUL
  if (s.find(L'\0') != std::wstring::npos) return false;
  
  auto hasInvalidWindowsChar = [](wchar_t c) {
    switch (c) {
      case L'<': case L'>': case L'"': case L'|': case L'?': case L'*':
        return true;
      default:
        return false;
    }
  };
  
  const size_t n = s.size();
  
  // Strong path indicators
  bool startsWithSlash = isSep(s[0]);
  bool startsWithTilde = (s[0] == L'~');
  bool startsWithDot =
  (s == L"." || s == L"..") ||
  (n >= 2 && s[0] == L'.' && isSep(s[1])) ||
  (n >= 3 && s[0] == L'.' && s[1] == L'.' && isSep(s[2]));
  
  bool isUNC =
  n >= 5 &&
  isSep(s[0]) && isSep(s[1]) &&
  !isSep(s[2]);
  
  bool hasSeparator = false;
  bool hasExtension = false;
  
  for (size_t i = 0; i < n; ++i) {
    wchar_t c = s[i];
    
    if (isSep(c)) {
      hasSeparator = true;
    }
    
    if (hasInvalidWindowsChar(c)) {
      return false;
    }
    
    // Colon is only accepted as "C:" at position 1
    if (c == L':') {
      if (!(i == 1 && std::iswalpha(s[0]))) {
        return false;
      }
    }
  }
  
  // Check for trailing "file.ext" style extension in last component
  {
    size_t lastSep = s.find_last_of(L"/\\");
    size_t start = (lastSep == std::wstring::npos) ? 0 : lastSep + 1;
    size_t dot = s.find_last_of(L'.');
    if (dot != std::wstring::npos && dot > start && dot + 1 < n) {
      hasExtension = true;
    }
  }
  
  // Reject obvious plain text:
  // no root/drive/UNC, no separator, no dot-path, no extension, no tilde
  if (!startsWithSlash &&
      !startsWithTilde &&
      !startsWithDot &&
      !hasDriveLetter &&
      !isUNC &&
      !hasSeparator &&
      !hasExtension) {
    return false;
  }
  
  // Reject segments that are empty in the middle: "foo//bar" or "foo\\\bar"
  // but allow leading separators for root/UNC.
  for (size_t i = 1; i < n; ++i) {
    if (isSep(s[i]) && isSep(s[i - 1])) {
      // Allow UNC prefix "\\" or "//"
      if (!(i == 1 && isSep(s[0]) && isSep(s[1]))) {
        return false;
      }
    }
  }
  
  // For Windows paths, reject reserved chars in components.
  // For Unix these chars may be legal, but this is intentionally heuristic/strict.
  return true;
}

JEDLIB_END
