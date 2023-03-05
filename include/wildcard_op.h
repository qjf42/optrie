#ifndef __OP_TRIE_WILDCARD_OP_H__
#define __OP_TRIE_WILDCARD_OP_H__

#include "op.h"

namespace optrie {

// 模糊匹配（表达式：[W:min-max]，min可省略，默认为0）
class WildcardOpNode : public OpNode {
 public:
  WildcardOpNode(const std::string& expr) : OpNode(expr) {
    init();
  }

  virtual MatchIterator match(const std::wstring& s, size_t start) const;

 private:
  void init();

  bool match_next(const std::wstring& s, size_t start, size_t length) const;

};

}  // namespace optrie

#endif  // __OP_TRIE_WILDCARD_OP_H__
