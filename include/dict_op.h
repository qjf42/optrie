#ifndef __OP_TRIE_DICT_OP_H__
#define __OP_TRIE_DICT_OP_H__

#include "op.h"

#include <set>

namespace optrie {

using WordSet = std::set<std::wstring>;

class PatternDict {
 public:
  void get(const std::string& pat,
           std::shared_ptr<WordSet>& words, size_t& min_len, size_t& max_len) const;

  void load(const std::vector<std::string>& dict_files);

 private:
  std::map<std::string, std::shared_ptr<WordSet>> _pat_words_map;     // {dict_type: {set of words}}
  std::map<std::string, std::pair<size_t, size_t>> _pat_length_range; // {dict_type: (min_len, max_len)}
};

// 字典匹配算子（表达式：[D:dict_name]）
class DictOpNode : public OpNode {
 public:
  DictOpNode(const std::string& expr, const PatternDict& pat_dic) : OpNode(expr) {
    init(pat_dic);
  }

  virtual MatchIterator match(const std::wstring& s, size_t start) const;

 private:

  void init(const PatternDict& pat_dic);

  virtual bool match_next(const std::wstring& s, size_t start, size_t length) const;

  std::shared_ptr<WordSet> _words;
};

}  // namespace optrie

#endif  // __OP_TRIE_DICT_OP_H__
