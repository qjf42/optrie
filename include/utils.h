#ifndef __OP_TRIE_UTILS_H__
#define __OP_TRIE_UTILS_H__

#include <string>
#include <vector>


namespace optrie {

size_t max(size_t x, size_t y);

size_t min(size_t x, size_t y);

size_t relu(int64_t x);

void split(const std::string& str, char sep, std::vector<std::string>& result);

std::string ltrim(const std::string& str);

std::string rtrim(const std::string& str);

std::string trim(const std::string& str);

std::wstring utf8_to_wstring(const std::string& str);

std::string wstring_to_utf8(const std::wstring& str);

std::wstring to_lower(const std::wstring& str);

void replace_all(std::string& s, const std::string& from, const std::string& to);

}


#endif  // __OP_TRIE_UTILS_H__
