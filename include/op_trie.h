#ifndef __OP_TRIE_OP_TRIE_H__
#define __OP_TRIE_OP_TRIE_H__

#include "op.h"
#include "dict_op.h"
#include "literal_op.h"
#include "wildcard_op.h"

namespace optrie {

// ç®å­å·¥åð»
class OpNodeFactory {
 public:
  OpNodeFactory(std::shared_ptr<PatternDict> pat_dic) : _pat_dic(pat_dic) {}

  /**
   * å·¥åæ¹æ³ï¼æ ¹æ®expræé ç®å­èç¹ï¼æ¯æä¸ä¸å ç§expr
   * 1. [D:dict_name] -> è¯å¸ç®å­
   * 2. [W:min-max] -> æ¨¡ç³å¹éï¼minå¯çç¥ï¼é»è®¤ä¸º0
   * 3. å¶ä» -> ææï¼å®å¨ï¼å¹é
   */
  std::shared_ptr<OpNode> get(const std::string& expr);

 private:
  std::shared_ptr<PatternDict> _pat_dic;  // è¯å¸ç®å­ç¨çè¯å¸
};

struct MatchResult {
  // æ¯å¦å¹é
  bool matched;
  // å¹éç½®ä¿¡åº¦
  double score;
  // å¹éçåç»
  std::map<std::string, std::wstring> groups;
  // å¹éçæ¨¡æ¿é¢å¤ä¿¡æ¯
  std::map<std::string, std::string> extra;
  // å¹éçæ¨¡æ¿
  std::string tpl;
};

// ç®å­å¹éæ 
class OpTrie {
 public:
  OpTrie() : _root(std::make_shared<RootOpNode>()), _pat_dic(std::make_shared<PatternDict>()) {}

  ~OpTrie() = default;

  /**
   * åå§åï¼å è½½æ¨¡æ¿åè¯å¸
   * Params:
   *    template_files: æ¨¡æ¿æä»¶
   *    dict_files: è¯å¸ç®å­ç¨å°çè¯å¸æä»¶
   */
  OpTrie& load(const std::vector<std::string>& template_files,
               const std::vector<std::string>& dict_files);

  /**
   * æ¨¡æ¿å¹é
   * Returns: bool, æ¯å¦å¹éæå
   * Params:
   *    s: std::wstring
   *    extra: {k: v}
   *    groups: {k: matched value}
   */
  MatchResult match(const std::wstring& s) const;

  // æ¾ç¤ºæ ç»æï¼åä¸äºè¾å©ä¿¡æ¯
  void show() const;

 private:
  std::shared_ptr<RootOpNode> _root;      // æ ¹èç¹ï¼ä¸åå¹éï¼
  std::shared_ptr<PatternDict> _pat_dic;  // è¯å¸å¹éç®å­çè¯å¸

  // å è½½è¯å¸å¹éç®å­çè¯å¸
  void load_pat_dict(const std::vector<std::string>& dict_files);

  // å è½½æ¨¡æ¿ï¼æå»ºæ 
  void load_templates(const std::vector<std::string>& template_files);

  // ä¼ååªæ
  void optimize();

  // åæº¯å¹éï¼éå½è°ç¨ï¼
  bool match_dfs(std::shared_ptr<OpNode> cur_node, const std::wstring& s, size_t start,
                 std::vector<OpResult>& matched_results) const;
};

}  // namespace optrie

#endif  // __OP_TRIE_OP_TRIE_H__
