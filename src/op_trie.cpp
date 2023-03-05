#include <fstream>
#include "op_trie.h"
#include "log_utils.h"
#include "nlohmann/json.hpp"

namespace optrie {

const static size_t MAX_MATCH_DEPTH = 16;

std::shared_ptr<OpNode> OpNodeFactory::get(const std::string& expr) {
  auto type = expr.substr(0, 3);
  std::shared_ptr<OpNode> op;
  if (type == "[D:") {
    op = std::make_shared<DictOpNode>(expr, *_pat_dic);
  } else if (type == "[W:") {
    op = std::make_shared<WildcardOpNode>(expr);
  } else {
    op = std::make_shared<LiteralOpNode>(expr);
  }
  return op;
}

MatchResult OpTrie::match(const std::wstring& s) const {
  MatchResult res;
  std::vector<OpResult> matched_results;
  if (match_dfs(_root, s, 0, matched_results)) {
    // last op
    auto op = matched_results.back().op;
    // extra
    res.extra = op->get_extra();
    // extractors of last op
    for (auto& pair : op->get_extractors()) {
      auto& op_res = matched_results[pair.second];
      res.groups[pair.first] = s.substr(op_res.start, op_res.length);
    }
    res.tpl = op->tpl;
    res.score = op->score;
    res.matched = true;
  } else {
    res.matched = false;
  }
  return res;
}

bool OpTrie::match_dfs(std::shared_ptr<OpNode> cur_node, const std::wstring& s, size_t start,
                       std::vector<OpResult>& matched_results) const {
  // assert(start <= s.length());
  if (start == s.length() && cur_node->is_end) {
    return true;
  }
  if (cur_node->can_fit_in_children(s.length() - start)) {
    for (auto& child : cur_node->children) {
      auto iter = child->match(s, start);
      // iter.debug();
      size_t matched_length;
      while (iter.next(matched_length)) {
        // LOG_DEBUG("Itering, matched_length: %zu ", matched_length);
        matched_results.emplace_back(start, matched_length, child.get());
        if (match_dfs(child, s, start + matched_length, matched_results)) {
          return true;
        }
        matched_results.pop_back();
      }
    }
  }
  return false;
}

OpTrie& OpTrie::load(const std::vector<std::string>& template_files,
                     const std::vector<std::string>& dict_files) {
  load_pat_dict(dict_files);
  load_templates(template_files);
  optimize();
  return *this;
}

void OpTrie::load_pat_dict(const std::vector<std::string>& dict_files) {
  _pat_dic->load(dict_files);
}

// 切分模板字符串，每个部分是"[]"包住，或者常量字符串
bool split_tpl(std::string& tpl, std::vector<std::string>& result) {
  result.clear();
  std::vector<std::string> tmp_result;
  // []转义
  const std::vector<std::string> lb{"\\[", "__L_B__", "["};
  const std::vector<std::string> rb{"\\]", "__R_B__", "]"};
  // tpl = replace_all(replace_all(tpl, lb[0], lb[1]), rb[0], rb[1]);
  replace_all(tpl, lb[0], lb[1]);
  replace_all(tpl, rb[0], rb[1]);
  std::string cur;
  bool left_on = false;
  for (size_t i = 0; i < tpl.size(); ++i) {
    auto ch = tpl[i];
    if (ch == '[') {
      if (left_on) {
        return false;
      } else {
        if (cur.length() > 0) {
          tmp_result.emplace_back(cur);
        }
      }
      cur = '[';
      left_on = true;
    } else if (ch == ']') {
      if (!left_on) {
        return false;
      } else {
        cur += ']';
        tmp_result.emplace_back(cur);
        cur.clear();
        left_on = false;
      }
    } else {
      cur += ch;
    }
  }
  if (left_on) {
    return false;
  } else {
    if (cur.length() > 0) {
      tmp_result.emplace_back(cur);
    }
  }
  for (auto& expr : tmp_result) {
    replace_all(expr, lb[1], lb[2]);
    replace_all(expr, rb[1], rb[2]);
    result.emplace_back(expr);
  }

  return true;
}


void parse_template(std::string& tpl, std::vector<std::string>& exprs,
                    const std::string& score_str, double& score,
                    const std::string& extra_str, std::map<std::string, std::string>& extra,
                    const std::string& extractor_str, std::map<std::string, size_t>& extractors) {
  // 1. 拆模板到节点表达式
  auto split_succ = split_tpl(tpl, exprs);
  if (!split_succ) {
    throw std::runtime_error("Invalid template: " + tpl);
  }
   // 限制递归匹配深度
  if (exprs.size() > MAX_MATCH_DEPTH) {
    throw std::runtime_error("Invalid template: " + tpl);
  }
  // 2. 解析score
  try {
    score = std::stod(score_str);
  } catch (const std::exception& e) {
    throw std::runtime_error("Invalid template score: " + score_str);
  }
  // 3. 解析extra
  if (trim(extra_str).length() > 0) {
    try {
      auto extra_json = nlohmann::json::parse(extra_str);
      if (!extra_json.is_object()) {
        throw std::runtime_error("extra is not an object");
      }
      for (auto& kv : extra_json.items()) {
        if (kv.value().is_string()) {
          extra[kv.key()] = kv.value();
        } else if (kv.value().is_number_integer()) {
          extra[kv.key()] = std::to_string(kv.value().get<int64_t>());
        } else if (kv.value().is_number_float()) {
          extra[kv.key()] = std::to_string(kv.value().get<double>());
        } else {
         throw std::runtime_error("extra value is not string");
        }
      }
    } catch (const std::exception& e) {
      throw std::runtime_error("invalid extra: '" + extra_str + "', " + e.what());
    }
  }
  // 4. 解析extractor
  if (trim(extractor_str).length() > 0) {
    try {
      auto extractor_json = nlohmann::json::parse(extractor_str);
      if (!extractor_json.is_object()) {
        throw std::runtime_error("extractor is not an object");
      }
      for (auto& kv : extractor_json.items()) {
        if (!kv.value().is_string()) {
          throw std::runtime_error("extractor value is not string");
        }
        auto ex = kv.value().get<std::string>();
        // hardcode 校验
        size_t op_order = 0;
        if (ex[ex.length()-2] == '#') {
          op_order = std::stoul(ex.substr(ex.length()-1)) - 1;
          ex = ex.substr(0, ex.length() - 2);
        }
        bool found = false;
        size_t op_i = 0;
        for (size_t i = 0; i < exprs.size(); ++i) {
          if (ex == exprs[i]) {
            if (op_i == op_order) {
              extractors[kv.key()] = i;
              found = true;
              break;
            }
            ++op_i;
          }
        }
        if (!found) {
          throw std::runtime_error("extractor not found in exprs");
        }
      }
    } catch (const std::exception& e) {
      throw std::runtime_error("Invalid template extractor: '" + extractor_str + "', " + e.what());
    }
  }
}

void OpTrie::load_templates(const std::vector<std::string>& template_files) {
  OpNodeFactory op_factory(_pat_dic);
  std::ifstream fi;
  std::string line;
  std::vector<std::string> fields;
  for (auto& template_file : template_files) {
    fi.open(template_file);
    if (!fi.is_open()) {
      throw std::runtime_error("failed to open template file: " + template_file);
    }
    while (std::getline(fi, line)) {
      line = trim(line);
      if (line.empty() || line[0] == '#') {
        continue;
      }
      split(line, '\t', fields);
      if (fields.size() < 2 || fields.size() > 4) {
        LOG_WARN("Invalid template line: %s", line.c_str());
      }
      std::vector<std::string> exprs;
      double score;
      std::map<std::string, std::string> extra;
      std::map<std::string, size_t> extractors;
      try {
        parse_template(fields[0], exprs,
                       fields[1], score,
                       (fields.size() > 2 ? fields[2] : ""), extra,
                       (fields.size() > 3 ? fields[3] : ""), extractors);
      } catch (const std::exception& e) {
        LOG_WARN("Failed to parse template line, %s", e.what());
      }

      std::shared_ptr<OpNode> op{_root}, next_op{nullptr};
      for (auto& expr : exprs) {
        if (!op->get_child(expr, next_op)) {
          next_op = op_factory.get(expr);
          next_op->set_parent(op);
          op->add_child(next_op);
        }
        op = next_op;
      }
      // last op
      op->is_end = true;
      op->score = score;
      op->tpl = fields[0];
      op->set_extra(extra);
      op->set_extractors(extractors);
    }
    LOG_INFO("Parse template %s done", template_file.c_str());
    fi.close();
  }
}

void OpTrie::optimize() {
  _root->update_child_min_max_len();
}

void OpTrie::show() const {
  _root->show();
}

}  // namespace optrie
