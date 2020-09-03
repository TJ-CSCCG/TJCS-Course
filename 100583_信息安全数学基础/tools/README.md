# :hammer_and_wrench: 信息安全数学基础 工具

## 100583 信息安全数学基础

### 一、总述

提供 py 文件是为了压缩不必要的作业时间。

并且，有的时候程序一通过，就不会再回头看过。这样减少了对加解密过程、特殊算法的理解时间，作业便没有起到复习巩固的作用。

### 二、工具分述

#### 1. [ShiCrypto](https://github.com/skyleaworlder/ShiCrypto)：[skyleaworlder](https://github.com/skyleaworlder)

##### a. 介绍

其实蛮丢脸的。代码写的稀烂，也没顾及什么优化（不如说写的时候还根本不会）

不过写了点注释，使用方法也贴了。

代码尽量做到了一清二楚，每一步都能在课件里面找到依据，不存在什么机械降神般的代码块。

* **没有具体与作业题绑定在一起**；
* 写的时候，我尽量按照课件的布置来布置；
* 基本覆盖了所有烦了点的计算，比如 ECC 上点的计算。

##### b. 使用

运行环境：Python 3.7

[ShiCrypto V0.1-alpha.1](https://github.com/skyleaworlder/ShiCrypto/releases/tag/v0.1-alpha.1)

（但是这个版本里面没有 master 里的 **Wiener Attack** 和 **Fermat Factorizatioin**，而且 **Inverse** 中间比较鬼畜，没有展现出 **与课件一致** 的风格 …… 直接 clone 应该是更好的）