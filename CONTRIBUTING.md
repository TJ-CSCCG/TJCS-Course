# :cloud: ​贡献方法

## :ship: 精神

```Chinese
青年朋友们！一代人有一代人的长征，一代人有一代人的担当。
建成社会主义现代化强国，实现中华民族伟大复兴，是一场接力跑。我们有决心为青年跑出一个好成绩，也期待现在的青年一代将来跑出更好的成绩。

衷心希望新时代中国青年积极拥抱新时代、奋进新时代，
让青春在为祖国、为人民、为民族、为人类的奉献中焕发出更加绚丽的光彩！
```

## :heart: 大致流程

1. `fork` 本项目；
2. `clone` 所 `fork` 到个人的仓库到本地；
3. 将本仓库作为个人 `fork` 的本地仓库的 **上游仓库** (git remote add upstream ...)；
4. 进行你的贡献；
5. 同步上游仓库代码，个人处理冲突；
6. `push` 本地仓库的修改到 `GitHub`；
7. 在 `fork` 的仓库里点击 `pull request`。

## :anchor: 标准

### 1. 仓库中命名格式

* 文件夹：[6位课号]\_[课程名]，如：102109\_数字逻辑。**（禁止：10210901\_数字逻辑、数字逻辑、102109——数字逻辑、102109\_数逻）**
* 文件名：任意，但尽量体现文件内容和价值。

### 2. 仓库中文件夹一般目录结构

本仓库中 **仅包含与课程有关** 的内容，大体目录结构如下：

```repo
----|
    |----README.md (summary)
    |
    |----book/
    |    |----README.md(links to books)
    |
    |----doc/
    |    |----README.md(desc doc folder)
    |    |----...(folders of various doc)
    |
    |----template/
    |    |----README.md(desc template folder)
    |    |----...(templates)
    |
    |----tools/
         |----README.md(directions or links)
         |----...(folders containing tools)
```

### 3. 注意

* `book` 文件夹中仅有 `README.md` 一个文件，不应上传 `pdf`；
* `doc` 文件夹规则要求较少，应尽量组织结构，并写 `README`；
* `template` 文件夹中只能放模板。模板本属于 `doc`，但为快速找到模板，故独立设置文件夹；
* `tools` 文件夹可上传个人开发的各种工具（或在 `README.md` 写上 :link: 链接），应写 / 修改好 `tools/README.md` 以及提交工具目录中的 `README`。

### 4. Markdown 规范

#### i. 遵守 `markdownlint`

[markdownlint 标准解读](https://github.com/DavidAnson/markdownlint)

在保证观感的前提下，尽量遵守 `markdownlint`。

有关的 `markdownlint` 的配置如下：

```json
"markdownlint.config": {
    "MD001": true,
    "MD002": true,
    "MD004": {
      "//": "unordered list style",
      "style": "asterisk",
    },
    "MD005": true,
    "MD007": {
      "//": "unordered list indent tab",
      "indent": 2
    },
    "MD012": {
      "maximum": 1
    },
    "MD013": {
      "//": "line length",
      "line_length": 150,
      "code_block_line_length": 200,
      "heading_line_length": 100
    },
    "MD022": {
      "lines_above": 1,
      "lines_below": 1
    },
    "MD023": true,
    "MD024": {
      "siblings_only": true,
      "allow_different_nesting": false
    },
    "MD025": {
      "level": 1
    },
    "MD026": {
      "punctuation": ".,;:!?。，；：！？"
    },
    "MD027": true,
    "MD031": true,
    "MD033": {
      "//": "html elements allowed",
      "allowed_elements": ["b", "h", "br", "span", "href", "font", "u"]
    },
    "MD035": {
      "//": "sep line style",
      "style": "consistent",
    },
    "MD036": true,
    "MD037": true,
    "MD039": true,
    "MD040": true,
    "MD041": true,
    "MD042": true,
    "MD046": {
      "style": "fenced",
    },
    "MD047": true
}
```

#### ii. 遵守 `中文文案排版指北`

[chinese-copywriting-guidelines 说明](https://github.com/sparanoid/chinese-copywriting-guidelines)

## :hammer_and_wrench: 推荐使用

* `Visual Studio Code` + `markdownlint`
* 记事本（大雾）
* 配合预览功能，登录 `GitHub` 直接修改需要修改的文件

（注：极不建议使用 `Typora` 作为编辑器）

## :warning: 注意

需要遵守以下约定：

* 若未经著作权人许可，未向其支付报酬时，应满足以下要求：
  * 上传文件时，应指明作者姓名、作品名称；
  * 上传文件时，不得侵犯著作权人享有的其他权利；
  * 上传文件可为 **个人学习、研究、欣赏** 而上传；
  * 上传文件可为 **介绍、评论某一作品或者说明某一问题** 而所做的适当引用；
  * 上传文件可为学校 **课堂教学或科学研究、翻译或者少量复制**，供 **教学** 使用（但不得发行）
* 若经著作权人许可，则需与著作权人达成协议或约定，依照协议、约定或其他有关规定提供补偿。

本仓库使用者不应将资料用于商业用途。

