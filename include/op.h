#ifndef __OP_TRIE_OP_H__
#define __OP_TRIE_OP_H__

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <cassert>

#include "utils.h"

namespace optrie {

static size_t MAX_LEN = 64;

void set_max_match_len(size_t len);

class OpNode;

// 每个节点的匹配结果
struct OpResult {
  OpResult(size_t start, size_t length, const OpNode* op)
      : start(start), length(length), op(op) {}

  size_t start;      // 字符串起始位置
  size_t length;     // 匹配长度
  const OpNode* op;  // 算子
};

// 每个节点的匹配结果
class MatchIterator {
 public:
  MatchIterator(const OpNode* node, const std::wstring& s, size_t pos_start,
                size_t len_start, size_t len_end, int64_t len_step)
      : _op(node), _s(s),
        _pos_start(pos_start),
        _len_start(static_cast<int64_t>(len_start)),
        _len_end(static_cast<int64_t>(len_end)),
        _len_step(len_step) {}

  /**
   * 迭代至下一个hit的结果
   * Returns: 存在下一个匹配时返回true，否则返回false
   * Params:
   *    length: 存在下一个匹配时改为匹配的片段长度，否则不变
   */
  bool next(size_t& length);

  // 打印（stdout）debug信息
  void debug_info() const;

 private:
  const OpNode* _op;  // 算子
  std::wstring _s;    // 要匹配的字符串（TODO, 改指针，减少拷贝）

  size_t _pos_start;   // 要匹配的起始位置
  int64_t _len_start;  // 长度范围起始
  int64_t _len_end;    // 长度范围结束
  int64_t _len_step;   // 长度范围步长，±1
};

// 算子节点（基类）
class OpNode {
  friend class MatchIterator;

 public:
  OpNode(const std::string& expr)
      : expr(expr), tpl(""), score(0.0), is_end(false),
        _parent(nullptr), _max_len(MAX_LEN), _min_len(0), _child_max_len(0), _child_min_len(0) {}

  /**
   * 匹配，返回一个迭代器（w/ next方法）
   * Returns: MatchIterator, 迭代器对象
   * Params:
   *    s: std::wstring
   *    start: 起始位置
   */
  virtual MatchIterator match(const std::wstring& s, size_t start) const = 0;

  /**
   * 根据expr拿子节点
   * Returns: bool，子节点是否存在
   * Params:
   *    expr: 模板中抽取的节点表达式
   *    child: 如果子节点存在，child=子节点指针，否则不变
   */
  bool get_child(const std::string& expr, std::shared_ptr<OpNode>& child) const;

  // 增加子节点
  // 需要先确认同expr表达式的节点不存在
  void add_child(std::shared_ptr<OpNode> child);

  inline void set_parent(std::shared_ptr<OpNode> parent) {
    _parent = parent;
  }

  // (向下)修正max_len
  inline void set_max_len(size_t max_len) {
    _max_len = min(_max_len, max_len);
  }

  // (向上)修正min_len
  inline void set_min_len(size_t min_len) {
    _min_len = max(_min_len, min_len);
  }

  // 计算子树能匹配的长度范围，方便回溯匹配时做剪枝
  void update_child_min_max_len();

  // 判断剩余字符串长度能否塞进至少一个子树中
  inline bool can_fit_in_children(size_t length) const {
    return length >= _child_min_len && length <= _child_max_len;
  }

  inline const std::map<std::string, std::string>& get_extra() const {
    return _extra;
  }

  inline void set_extra(const std::map<std::string, std::string>& extra) {
    _extra = extra;
  }

  inline const std::map<std::string, size_t>& get_extractors() const {
    return _extractors;
  }

  inline void set_extractors(const std::map<std::string, size_t>& extractors) {
    _extractors = extractors;
  }

  // 级联显示当前节点及子孙节点的信息
  void show(size_t depth = 0) const;

  std::string expr;                               // 表达式
  std::string tpl;                                // 叶子节点对应的模板
  double score;                                   // 置信度
  bool is_end;                                    // 是否可以终止匹配
  std::vector<std::shared_ptr<OpNode>> children;  // 子节点

 protected:
  virtual bool match_next(const std::wstring& s, size_t start, size_t length) const = 0;

  std::shared_ptr<OpNode> _parent;                               // 父节点
  std::map<std::string, std::shared_ptr<OpNode>> _children_map;  // 子节点map，方便构建时查询

  size_t _max_len;        // 当前节点支持的最大长度
  size_t _min_len;        // 当前节点支持的最小长度
  size_t _child_max_len;  // 当前节点子树支持的最大长度
  size_t _child_min_len;  // 当前节点子树支持的最小长度

  std::map<std::string, std::string> _extra;   // 每个模板的额外payload，如分类
  std::map<std::string, size_t> _extractors;  // 需要抽取的节点映射，{key: 子节点顺序（正序，对应于OpResult列表顺序）}
};

// ROOT节点（不做匹配）
class RootOpNode : public OpNode {
 public:
  RootOpNode() : OpNode("ROOT") {
    init();
  }
  ~RootOpNode() {}

  virtual MatchIterator match(const std::wstring& s, size_t start) const {
    throw std::runtime_error("U SHOULD NOT BE HERE!");
  }

 private:
  inline void init() {
    set_max_len(0);
  }

  virtual bool match_next(const std::wstring& s, size_t start, size_t end) const {
    return false;
  }
};

}  // namespace optrie

#endif  // __OP_TRIE_OP_H__
