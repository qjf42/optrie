#ifndef __OP_TRIE_LITERAL_OP_H__
#define __OP_TRIE_LITERAL_OP_H__

#include "op.h"

namespace optrie {

// 字面（完全）匹配算子
class LiteralOpNode : public OpNode {
 public:
  LiteralOpNode(const std::string& expr) : OpNode(expr) {
    init();
  }

  virtual MatchIterator match(const std::wstring& s, size_t start) const;

 private:
  void init();

  virtual bool match_next(const std::wstring& s, size_t start, size_t length) const;

  std::wstring _w_expr;
};

}  // namespace optrie

#endif  // __OP_TRIE_LITERAL_OP_H__
