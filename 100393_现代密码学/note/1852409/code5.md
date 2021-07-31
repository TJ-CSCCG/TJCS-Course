# 5

上回的最后讲到了书上的 2.3

对于移位密码，是完善保密的。



![1](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-1.png)

如果每个密钥匙用的概率都相等，都是 $\dfrac{1}{K}$ ，那么计算原来明文概率和已知明文概率是否相等即可。

![2](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-2.png)

高亮部分就是完善保密性的核心。先验概率等于概率。

对于每一个 x，乘上能够把它转换成特定密钥的 ki，得到 y 。



得到加密成 y 的概率是：
$$
Pr(y) = \sum Pr(x) Pr(k_i) = \dfrac{1}{|K|} \sum_{x \in P} Pr(x) = \dfrac{1}{|K|}
$$


正着证需要知道一个基本事实：

|K| >= |C| |C| >= |P|，后者和完善保密性没有关系，这个是作为密码的必要的东西。总不能让两个不同的明文加密成相同的密文。

给定了一个 y 用不同的密钥来解密，就解密成不同的明文，由于明文空间小于密文空间，那么就得到不同的明文。

如果 K 不大于 C ，用不同密钥解密明文，那就解密不出来这么多的明文。同样的，对于一个明文 P，加密出来的密文就不会是 C 个，因为 K 要小于 C，不可能一个密钥加密出来的密文有一个以上。

所以给定 C 中的一个值，一定可以找到一个密文是无法从密钥加密得来的。那么肯定存在一个 x \in P , st. $P_r(x|y) = 0 = P_r(x)$。但是这个不可能。因为 $P_r(x) \ne 0$。

所以必须：

**对于每一个明文 x 和对应的一个密文 y 都只要有一个密钥 k 使之 e(x) = y**

对于一个明文来说，用密钥加密他，所有的密文数要等于密钥数。（这里假定密钥们没有重复）（因为假设 |C| = |K|）



假设明文空间是

![3](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-3.png)

用 Ki 定义密钥的名字。对于每一个明文，只有一把密钥，将其加密为 yi

因为完善保密，所以第一个等式可以约分为：$Pr(y|x_i) = Pr(y)$。

而 $Pr(y|x_i) = Pr(K = K_i)$。

所以 $Pr(y) = Pr(K_i)$。

所以 y 也是 $\dfrac{1}{|K|}$。



![4](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-4.png)

一次一密。维吉尼亚算法。



## 熵

![5](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-5.png)

X 是一个随机变量。



## 凸函数

![6](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-6.png)

![7](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-7.png)



## 琴生不等式

![7](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-8.png)



## 熵的性质

熵是非负数。当且仅当某事件变成确定事件的时候，熵为 0 。

![9](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-9.png)

所有的概率相等时，熵取最大的 log_2 n。

![10](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-10.png)

![11](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-11.png)

![12](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-12.png)

![13](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-13.png)

等号当且仅当 $p_i q_j$ 都相等时成立 。

![14](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-14.png)

log2 26 = 4.7



语言熵没有告诉词句之间有什么相关性，语言之间是有关系的。

语言有一定的冗余度——有一些词不写出来也会被推出来。

![15](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-15.png)

若把 HL = 1.25，如果语言文字充分多，可以把原来的语言压缩成之前的 25% 。



![16](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-16.png)

存在一个明文，使得有密钥可以把明文加密成密文，所有这样的密钥构成了一个集合，这个密钥集合中只有一个是真的密钥，剩下的都是伪密钥。



![17](https://github.com/TJ-CSCCG/TJCS-Images/raw/TJCS-Course/100393_现代密码学/note/1852409/img/5-17.png)