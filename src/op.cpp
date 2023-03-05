#include "op.h"
#include "log_utils.h"
#include "nlohmann/json.hpp"

namespace optrie {

void set_max_match_len(size_t len) {
  MAX_LEN = len;
}

bool MatchIterator::next(size_t& length) {
  while ((_len_end - _len_start) * _len_step >= 0) {
    auto len = _len_start;
    bool hit = _op->match_next(_s, _pos_start, len);
    _len_start += _len_step;
    if (hit) {
      length = len;
      return true;
    }
  }
  return false;
}

void MatchIterator::debug_info() const {
  LOG_DEBUG("Iterator of op[%s], length range(%ld, %ld, %ld)", _op->expr.c_str(), _len_start, _len_end, _len_step);
}

bool OpNode::get_child(const std::string& expr, std::shared_ptr<OpNode>& child) const {
  auto iter = _children_map.find(expr);
  if (iter == _children_map.end()) {
    return false;
  } else {
    child = iter->second;
    return true;
  }
}

void OpNode::add_child(std::shared_ptr<OpNode> child) {
  children.emplace_back(child);
  _children_map[child->expr] = child;
}

void OpNode::update_child_min_max_len() {
  if (children.size() > 0) {
    // 先更新子节点
    size_t child_min = 1 << 20, child_max = 0;
    for (auto& child : children) {
      child->update_child_min_max_len();
      child_max = max(child_max, child->_child_max_len + child->_max_len);
      child_min = min(child_min, child->_child_min_len + child->_min_len);
    }
    _child_max_len = min(MAX_LEN, child_max);
    _child_min_len = child_min;
  }
  // end可以视为某个特殊的空子节点
  if (is_end) {
    _child_min_len = 0;
  }
}

void OpNode::show(size_t depth) const {
  if (depth > 0) {
    std::cout << std::string(4 * depth - 1, ' ') << "└";
  }
  std::cout << expr << " (" << relu(depth - 1) << ")";
  std::cout << "\t[" << _min_len << ", " << _max_len << ", " << _child_min_len << ", " << _child_max_len << ']';
  if (is_end) {
    std::cout << "\t[END]";
    if (!_extra.empty()) {
      std::cout << "\textra:" << nlohmann::json(_extra);
    }
    if (!_extractors.empty()) {
      std::cout << "\textractors:" << nlohmann::json(_extractors);
    }
  }
  std::cout << std::endl;
  for (auto node : children) {
    node->show(depth + 1);
  }
}

}  // namespace optrie
