#include "utils.h"

#include <algorithm>
#include <codecvt>
#include <locale>

namespace optrie {

size_t max(size_t x, size_t y) {
  return x < y ? y : x;
}

size_t min(size_t x, size_t y) {
  return x > y ? y : x;
}

size_t relu(int64_t x) {
  return static_cast<size_t>(x < 0 ? 0 : x);
}

void split(const std::string& str, char sep, std::vector<std::string>& result) {
  result.clear();
  std::string::size_type last = 0, pos = 0;
  while ((pos = str.find(sep, last)) != std::string::npos) {
    result.emplace_back(str.substr(last, pos - last));
    last = pos + 1;
  }
  result.emplace_back(str.substr(last));
}

std::string ltrim(const std::string& str) {
  auto pos = std::find_if(str.cbegin(), str.cend(), [](unsigned char chr) {
    return !std::isspace(chr);
  });
  return std::string(pos, str.cend());
}

std::string rtrim(const std::string& str) {
  auto pos = std::find_if(str.crbegin(), str.crend(), [](unsigned char chr) {
               return !std::isspace(chr);
             }).base();
  return std::string(str.cbegin(), pos);
}

std::string trim(const std::string& str) {
  return ltrim(rtrim(str));
}

using cvt = std::codecvt_utf8<wchar_t>;
std::wstring_convert<cvt, wchar_t> converter;

std::wstring utf8_to_wstring(const std::string& str) {
  return converter.from_bytes(str);
}

std::string wstring_to_utf8(const std::wstring& str) {
  return converter.to_bytes(str);
}

std::wstring to_lower(const std::wstring& str) {
  std::wstring ret;
  for (auto wch : str) {
    if (wch >= 65 && wch <= 90) {
      ret += static_cast<wchar_t>(wch + 32);
    } else {
      ret += wch;
    }
  }
  return ret;
}

void replace_all(std::string& s, const std::string& from, const std::string& to) {
  size_t pos = 0;
  while((pos = s.find(from, pos)) != std::string::npos) {
    s.replace(pos, from.length(), to);
    pos += to.length();
  }
}

}  // namespace optrie