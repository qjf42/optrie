#include "literal_op.h"
#include "utils.h"

namespace optrie {

void LiteralOpNode::init() {
    _w_expr = utf8_to_wstring(expr);
    set_max_len(_w_expr.length());
    set_min_len(_w_expr.length());
}

MatchIterator LiteralOpNode::match(const std::wstring& s, size_t start) const {
  size_t max_len = min(_max_len, max(0, s.length() - start - _child_min_len));
  size_t min_len = max(_min_len, max(0, s.length() - start - _child_max_len));
  return MatchIterator(this, s, start, min_len, max_len, 1);
}

bool LiteralOpNode::match_next(const std::wstring& s, size_t start, size_t length) const {
  // 不用校验长度，MatchIterator 确定了长度范围
  return s.substr(start, length) == _w_expr;
}

}  // namespace optrie