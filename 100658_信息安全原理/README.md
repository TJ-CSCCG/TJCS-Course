# 信息安全原理

**（注：17 级及以前为计科信安必修课，18 级后改为信安必修课，学分变为 2，缩减了大作业数目）**

## 100658 信息安全原理

### 一、总述

#### 1. 教材

网络安全协议 -- 原理，结构与应用 第 2 版 高等教育出版社 寇晓蕤 王清贤

教材编得不好，但是对考试有用。如果对考试成绩有需求，还是建议买纸质二手书。

这本书内容几乎全部来源于 `RFC`，以下为作业 / 考试前可参考的一些标准：

* [L2TP : RFC-2661](https://tools.ietf.org/html/rfc2661)
* [IP : RFC-791](https://tools.ietf.org/html/rfc791), [TCP : RFC-793](https://tools.ietf.org/html/rfc793), [UDP : RFC-768](https://tools.ietf.org/html/rfc768)
* [Security Architecture for IP : RFC-2401](https://tools.ietf.org/html/rfc2401), [AH : RFC-2402](https://tools.ietf.org/html/rfc2402), [ESP : RFC-2406](https://tools.ietf.org/html/rfc2406), [ISAKMP : RFC-2408](https://tools.ietf.org/html/rfc2408)
* [IKEv1 : RFC-3947](https://tools.ietf.org/html/rfc3947), [IKEv2 : RFC-4306](https://tools.ietf.org/html/rfc4306)
* [SSLv3 : RFC-6101](https://tools.ietf.org/html/rfc6101), [TLSv1.0 : RFC-5077](https://tools.ietf.org/html/rfc5077)
* [SSH : RFC-4250](https://tools.ietf.org/html/rfc4250), [SSH-Authentication : RFC-4252](https://tools.ietf.org/html/rfc4252), [SSH-Transport Layer : RFC-4253](https://tools.ietf.org/html/rfc4253), [SSH-Connection : RFC-4254](https://tools.ietf.org/html/rfc4254)
* [PGP : RFC-1991](https://tools.ietf.org/html/rfc1991), [OpenPGP Msg : RFC-2440](https://tools.ietf.org/html/rfc2440), [OpenPGP Msg : RFC-4880](https://tools.ietf.org/html/rfc4880)
* [SMTP : RFC-5321](https://tools.ietf.org/html/rfc5321), [POP3 : RFC-1939](https://tools.ietf.org/html/rfc1939), [MIME : RFC-2045](https://tools.ietf.org/html/rfc2045)

**（注：这些标准不都是最新的，但是还是有用的）**

#### 2. 作业

17 级及之前一共 6 个大作业，包括 **L2TP 协议分析，OpenPGP 文件夹加密，手写简单 SSH，IPSec VPN 搭建，HTTPS 部署** 等。

作业时间并不宽松。

18 级调整了大作业数量，降低了作业难度，共 5 次作业：**L2TP 协议分析，IPSec VPN 搭建，IPSec 分析题，HTTPS 部署，OpenPGP 文件夹加密**。

21级作业与17、18级作业相仿。

#### 3. 课堂

老师喜欢到处乱走，上课不会抽人回答问题，自己做自己的事情也没关系。

可能会挑人少的课签到。

#### 4. 考试

“我在考前也觉得自己要挂了，但考的都是给出来的题。”——某 17 级同学。

本 `README` 撰写者考前也慌得很：

![慌死了慌死了](./img/复习盛况.png)

但是老师考试前会给往届的考试题目，甚至还可能布置下去供同学间讨论。每一次作业不仅要好好做，还要理解其理论知识，因为很有可能这就是考试题目。

17 / 18 级试卷与老师课上展示的往届试卷内容大题相仿。例如，18 级期末考试 5 道大题分别为：

* OSI 7 层各层能否 / 如何实现访问控制；
* SSL 协议体系结构与记录协议安全机制默写，分析记录协议中的某安全机制具体实现；
* IPSec 与 SSL 在抵抗中间人攻击方面的对应策略；
* IPSec 网关内主机到另一网关的通信如何屏蔽外部无线 AP 带来的安全风险；
* 基于 PGP、SMTP/POP3 的带大数据附件邮件处理、发送、接受、认证、还原以及端端密钥注册交换流程。

18 级与 17 级试卷题目不相同，但考察内容 / 知识点差别不大。

23学年（21级）考试题目包括以上：

* IPSec 网关内主机到另一网关的通信如何屏蔽外部无线 AP 带来的安全风险；

其他考点和题目和之前的往届试卷均有一定出路，切不可刻舟求剑！（血泪教训）

#### 5. 附注

该课程讲授内容主要为安全协议，但我校信安 / 计科专业在培养方案安排上存在极大缺陷，大三上学期及之前并未教授计算机网络。因此，我校学生在信安原理前大多数并未系统接触网络协议相关知识，即使有所了解，也大多是对应用层、传输层协议 `header/body` 各 `options` 的记忆，不存在系统的 `Security Consideration`。

本附注将一直保留，将删除于我校再次调整计科 / 信安专业培养方案，理解信安专业信安原理课程与计算机网络基础知识间依赖关系之时。

### 二、任课教师

#### 1. 99060 Tan

18 级及以前信息安全原理老师。

上课建议前几节课坐在前面，等适应了口音再逐渐往后排坐。老师声音比较小，且带口音，刚开学就坐在后面肯定听不懂内容。

**不发 PPT**，但是平常作业 / 考试前 **PPT** 或许有些重要。但是老师是无论如何都不会把课件传到群里的，会以各种理由推脱。因此，建议上课及时拍照。如果对内容感兴趣，可以录音、速录讲话内容。

老师水平还是有的，说话可能有时候有点气人，抽问时可能会一怼到底。别往心里去。

#### 2. 07104 Yang

17 级及以前信息安全原理老师。

因 18 级后计科专业不再必修信安原理，教学班缩减，所以 Yang 老师不上这门课了。（原本 Yang 老师的作业少一些的）
