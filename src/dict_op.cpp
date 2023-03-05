#include <fstream>
#include "dict_op.h"
#include "utils.h"
#include "log_utils.h"

namespace optrie {

void PatternDict::get(const std::string& pat,
                      std::shared_ptr<WordSet>& words, size_t& min_len, size_t& max_len) const {
  auto w_iter = _pat_words_map.find(pat);
  auto l_iter = _pat_length_range.find(pat);
  if (w_iter == _pat_words_map.end() || l_iter == _pat_length_range.end()) {
    throw std::runtime_error("Dict " + pat + " does not exist");
  } else {
    words = w_iter->second;
    min_len  = l_iter->second.first;
    max_len  = l_iter->second.second;
  }
}

void PatternDict::load(const std::vector<std::string>& dict_files) {
  std::string line;
  std::ifstream fi;
  for (auto& dict_file : dict_files) {
    fi.open(dict_file);
    if (!fi.is_open()) {
      throw std::runtime_error("Failed to open template dict file: " + dict_file);
    }
    std::string pat_name;
    while (std::getline(fi, line)) {
      line = trim(line);
      if (line.empty() || line[0] == '#') {
        continue;
      }
      if (line.find("[D:") == 0) {
        pat_name = line;
      } else if (!pat_name.empty()) {
        auto iter = _pat_words_map.find(pat_name);
        std::shared_ptr<WordSet> words = nullptr;
        if (iter != _pat_words_map.end()) {
          words = iter->second;
        } else {
          words = std::make_shared<WordSet>();
          _pat_words_map[pat_name] = words;
        }
        words->emplace(to_lower(utf8_to_wstring(line)));
      }
    }
    LOG_INFO("Parse optrie dict [%s] done", dict_file.c_str());
    fi.close();
  }

  LOG_INFO("All dict parsed");
  for (auto iter : _pat_words_map) {
    auto pat_name = iter.first;
    size_t min_len = 1<<20, max_len = 0;
    for (auto& word : *iter.second) {
      size_t len = word.length();
      min_len = min(min_len, len);
      max_len = max(max_len, len);
    }
    _pat_length_range[pat_name] = {min_len, max_len};
    LOG_INFO("Pattern: %s, size: %zu, length range: [%zu, %zu]",
             pat_name.c_str(), iter.second->size(), min_len, max_len);
  }
}

void DictOpNode::init(const PatternDict& pat_dic) {
  size_t min_len, max_len;
  pat_dic.get(expr, _words, min_len, max_len);
  set_max_len(max_len);
  set_min_len(min_len);
}

MatchIterator DictOpNode::match(const std::wstring& s, size_t start) const {
  size_t max_len = min(_max_len, relu(s.length() - start - _child_min_len));
  size_t min_len = max(_min_len, relu(s.length() - start - _child_max_len));
  return MatchIterator(this, s, start, max_len, min_len, -1);
}

bool DictOpNode::match_next(const std::wstring& s, size_t start, size_t length) const {
  return _words->find(s.substr(start, length)) != _words->end();
}

}  // namespace optrie