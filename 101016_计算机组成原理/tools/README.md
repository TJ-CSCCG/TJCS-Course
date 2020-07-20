# Mars Vivado 前仿真自动测试工具

**（Author：[Maoyao233](https://github.com/Maoyao233)）**



## 环境要求

* 软件要求：

  * Vivado 2016.2
  * Mars（必须是通过 [MIPS246.tongji.edu.cn]([http://mips246.tongji.edu.cn](http://mips246.tongji.edu.cn/)) 下载）

  **Mars 必须可以自动生成 `result.txt` 的版本！**



## 使用说明

1. 将本文件置于vivado工程目录下（与.xpr文件同级）

2. 新建testcases目录，将所有测试用汇编代码（后缀必须是.txt）放入testcases目录中

3. 按说明配置好路径信息后，命令行调用：

   ```bash
   python autotest.py
   ```

    即可。



## 注意事项

本程序的原理是将前仿真中使用 `$fdisplay` 写出的数据与 `MARS` 产生的 `result.txt` 全文比对，所以 `testbench` 中的输出必须和 `MARS` 的格式完全一致。详情参考附件。



在助教下发的测试文件中，有两处需要微调：

* `add` 测试文件中：

  ```assembly
  add $19,$19,$3
  ```

  指令会导致溢出，`MARS` 会直接结束程序。

  而我们所涉及的 **31条指令集CPU** 不涉及异常处理，会继续执行，导致结果不同。因此需要将那条指令及之后的部分全部删除；

* `jr` 测试文件末尾放了一条空指令占位，而参考 `testbench` 中判定停机的条件是读到空指令，因此会导致自己的输出比 `MARS` 少一轮。

  建议删除 `jr`末尾的指令，不影响正确性。

  （如果有发现更好的解决方案，请与作者联系）