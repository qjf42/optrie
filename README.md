# OpTrie
一个简单的字符串模板匹配工具

## 使用场景
短文本模式匹配，如搜索query理解，用于识别意图和槽位，或是较短文本的分类和信息抽取，如
- `[地点][场所类型]` 匹配 `上海动物园`
- `[人名][不超过2个字][属性or关系]` 匹配 `马斯克的公司`


## 安装
`pip install optrie`
- 支持
    - Linux: 主要Linux发行版([pypa manylinux 2.17](https://github.com/pypa/manylinux))
    - MacOs
    - Windows暂未测试

### 源码安装
```bash
git clone https://github.com/qjf42/optrie
cd optrie
python setup.py install
```

## Usage
### 1. 准备词典
- 每一个[D:xxx]表示词典key
- 后续的每一行是其可以匹配的字符串
- 不同key的值集合可以有重合

```
[D:hi]
你好
hello
# 注释，#是首字符
[D:location]
上海
北京
[D:price]
房价
价格
```

### 2. 准备模板
每一行是一个模板，用tab分隔，可以有2~4项，分别为：
1. 模板项
    - `[D:xx]`必须包含在词典中
    - `[W:1]`用于模糊匹配，表示0-1个字符，`[W:2-3]`表示2-3个字符
    - 也可以直接用明文
2. 置信分
    - ps：目前匹配采用贪心策略，当匹配多个模板时，只取第一个，不保证分数最大
3. 模板关联信息（可选）
    - json, schema: {string => string|numeric}
4. 信息抽取（可选）
    - json, schema: {输出字段 => 抽取的词典项或模糊匹配项}

```
[D:hi][W:1]	1
[W:2-3][D:location][D:price]	0.9	{"catg": "housing"}	{"loc": "[D:location]"}
[W:1-2]月新番	1		{"month":"[W:1-2]"}
```

3. 匹配
```python
import optie
# 默认情况下，最多支持长度64的短文本（主要考虑到个别case性能），长文本最好手动切句
# 如果要调整，需要在构建树之前
# optrie.set_max_match_len(128)
# 加载模板和词典，可以有多个
m = optrie.OpTrie().load(['sample.tpl'], ['sample.dic'])
# 打印词典树
m.show()

res = m.match('你好')
res.matched     # True
res.template    # [D:hi][W:1]
res.score       # 1

res = m.match('查询上海房价')
res.matched     # True 
res.template    # "[W:2-3][D:location][D:price]"
res.score       # 0.9
res.extra_info  # {"catg": "housing"}
res.groups      # {"loc": "上海"}

res = m.match('深圳房价')
res.matched     # False
```

## 设计思路
- 满足模式匹配可以有很多方法，比如：把模式展开为正则，e.g `(上海|北京).{,2}(房价|价格)`，但是这种方法在模式和词典很大的情况下有巨大的维护成本，且遍历所有模式的正则也会比较慢
- 怎么匹配的？
    - 采用类似字典树（Trie）的思路
    - 每个节点表示一个匹配算子(op)，也就是上面配置的模板项，包括：词典算子、模糊匹配算子、明文，每个节点可以匹配固定长度或不定长的多个字符
    - 每个词典算子对应一个词库：一般情况下模板的变化较少，词典的人工维护较多，所以这里把词典和模板解耦，可以通过`load([], [要更新的词典])`热更新词典；另一个好处是，比起遍历展开所有可能的词，形成一个传统的字典树，可以明显降低树结构的复杂度和内存占用
    - 回溯法匹配
- 和传统字典树有什么区别？
    - 传统字典树每个节点只匹配一个字符，所以可以用贪心的算法，这里每个节点可以匹配的长度不一定是固定的，贪心不一定是最优解
- 支持多大规模的模板？
    - 还没测试过，希望大家帮忙反馈
    - 作为个人项目，满足小规模的调研场景应该是够了
- 支持C++吗？
    - 虽然是个python模块，但实现是C++和pybind11，所以把src目录下的py_module.cpp去掉后可以直接当lib用
- 支持正则吗？
    - 正则算子因为长度不好估计，比较麻烦，还没加，如果加上长度约束的话那跟其他算子是差不多的