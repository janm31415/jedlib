#pragma once

#include "namespace.h"

#include <string>
#include <stdint.h>
#include <vector>

JEDLIB_BEGIN

uint16_t ascii_to_utf16(unsigned char ch);
uint16_t ascii437_to_utf16(unsigned char ch);

bool remove_quotes(std::string& cmd);
bool remove_quotes(std::wstring& cmd);

void remove_whitespace(std::string& cmd);
void remove_whitespace(std::wstring& cmd);

std::vector<std::wstring> break_string(std::string in);

std::wstring convert_string_to_wstring(const std::string& str);
std::string convert_wstring_to_string(const std::wstring& str);

JEDLIB_END