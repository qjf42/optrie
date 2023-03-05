#include "wildcard_op.h"

namespace optrie {

void WildcardOpNode::init() {
  size_t min_len = 0, max_len = 0;
  std::vector<std::string> range;
  split(expr.substr(3, expr.length() - 4), '-', range);
  if (range.size() == 1) {
    max_len = std::stoul(range[0]);
  } else if (range.size() == 2) {
    min_len = std::stoul(range[0]);
    max_len = std::stoul(range[1]);
  } else {
    throw std::runtime_error("Invalid range " + expr);
  }
  assert(max_len >= min_len);
  set_max_len(max_len);
  set_min_len(min_len);
}

MatchIterator WildcardOpNode::match(const std::wstring& s, size_t start) const {
  size_t max_len = min(_max_len, relu(s.length() - start - _child_min_len));
  size_t min_len = max(_min_len, relu(s.length() - start - _child_max_len));
  return MatchIterator(this, s, start, min_len, max_len, 1);
}

bool WildcardOpNode::match_next(const std::wstring& s, size_t start, size_t length) const {
  return true;
}

}  // namespace optrie
