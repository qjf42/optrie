# OpTrie
一个简单的字符串模板匹配工具

## 安装
### pip
`pip install optrie`

- 二进制兼容性manylinux2.17
- windows和macos暂未测试

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
# 注释，#必须为首字符
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
from optrie import OpTrie
# 加载模板和词典，可以有多个
m = OpTrie().load(['sample.tpl'], ['sample.dic'])
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