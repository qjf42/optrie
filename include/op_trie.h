#ifndef __OP_TRIE_OP_TRIE_H__
#define __OP_TRIE_OP_TRIE_H__

#include "op.h"
#include "dict_op.h"
#include "literal_op.h"
#include "wildcard_op.h"

namespace optrie {

// 算子工厂👻
class OpNodeFactory {
 public:
  OpNodeFactory(std::shared_ptr<PatternDict> pat_dic) : _pat_dic(pat_dic) {}

  /**
   * 工厂方法，根据expr构造算子节点，支持一下几种expr
   * 1. [D:dict_name] -> 词典算子
   * 2. [W:min-max] -> 模糊匹配，min可省略，默认为0
   * 3. 其他 -> 明文（完全）匹配
   */
  std::shared_ptr<OpNode> get(const std::string& expr);

 private:
  std::shared_ptr<PatternDict> _pat_dic;  // 词典算子用的词典
};

struct MatchResult {
  // 是否匹配
  bool matched;
  // 匹配置信度
  double score;
  // 匹配的分组
  std::map<std::string, std::wstring> groups;
  // 匹配的模板额外信息
  std::map<std::string, std::string> extra;
  // 匹配的模板
  std::string tpl;
};

// 算子匹配树
class OpTrie {
 public:
  OpTrie() : _root(std::make_shared<RootOpNode>()), _pat_dic(std::make_shared<PatternDict>()) {}

  ~OpTrie() = default;

  /**
   * 初始化，加载模板和词典
   * Params:
   *    template_files: 模板文件
   *    dict_files: 词典算子用到的词典文件
   */
  OpTrie& load(const std::vector<std::string>& template_files,
               const std::vector<std::string>& dict_files);

  /**
   * 模板匹配
   * Returns: bool, 是否匹配成功
   * Params:
   *    s: std::wstring
   *    extra: {k: v}
   *    groups: {k: matched value}
   */
  MatchResult match(const std::wstring& s) const;

  // 显示树结构，及一些辅助信息
  void show() const;

 private:
  std::shared_ptr<RootOpNode> _root;      // 根节点（不做匹配）
  std::shared_ptr<PatternDict> _pat_dic;  // 词典匹配算子的词典

  // 加载词典匹配算子的词典
  void load_pat_dict(const std::vector<std::string>& dict_files);

  // 加载模板，构建树
  void load_templates(const std::vector<std::string>& template_files);

  // 优化剪枝
  void optimize();

  // 回溯匹配（递归调用）
  bool match_dfs(std::shared_ptr<OpNode> cur_node, const std::wstring& s, size_t start,
                 std::vector<OpResult>& matched_results) const;
};

}  // namespace optrie

#endif  // __OP_TRIE_OP_TRIE_H__
