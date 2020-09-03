# coding=utf-8

'''
本程序可以自动完成计组课设CPU前仿真的测试，设计用于31条指令集CPU，但理论上可不加修改复用于54条指令集
环境要求：Vivado与Mars，其中Mars必须是通过MIPS246.tongji.edu.cn下载的、可以自动生成result.txt的版本
使用说明：将本文件置于vivado工程目录下（与.xpr文件同级），新建testcases目录，将所有测试用汇编代码（后缀必须是.txt）放入testcases目录中，按说明配置好路径信息后，命令行调用python autotest.py即可
注意事项：本程序的原理是将前仿真中使用$fdisplay写出的数据与MARS产生的result.txt全文比对，所以testbench中的输出必须和MARS的格式完全一致。详情参考附件。
在助教下发的测试文件中，有两处需要微调：add测试文件中add $19,$19,$3指令会导致溢出，MARS会直接结束程序，而我们所涉及的31条指令集CPU不涉及异常处理，会继续执行，导致结果不同，因此需要将那条指令及之后的部分全部删除；jr测试文件末尾放了一条空指令占位，而参考tb中判定停机的条件是读到空指令，因此会导致自己的输出比MARS少一轮，建议删除jr末尾的指令，不影响正确性。如果有发现更好的解决方案，请与作者（ymy1020@tongji.edu.cn）联系
'''

# 已知bug：本文件所在路径中有空格可能会导致错误，与Vivado本身的bug有关

import os
import filecmp
import msvcrt

print("--------------------------------------------------------------------------------------------------")
print("本程序可以自动完成计组课设CPU前仿真的测试，设计用于31条指令集CPU，但理论上可不加修改复用于54条指令集")
print("--------------------------------------------------------------------------------------------------")
print("A.环境要求:  Vivado与Mars")
print("             其中Mars必须是通过MIPS246.tongji.edu.cn下载的、可以自动生成result.txt的版本")
print("B.使用说明:  将本文件置于vivado工程目录下（与.xpr文件同级），新建testcases目录")
print("             将所有测试用汇编代码（后缀必须是.txt）放入testcases目录中")
print("             按说明配置好路径信息后，命令行调用python autotest.py即可")
print("C.注意事项:  本程序的原理是将前仿真中使用$fdisplay写出的数据与MARS产生的result.txt全文比对")
print("             所以testbench中的输出必须和MARS的格式完全一致，详情参考附件")
print("ATTENTION:   在助教下发的测试文件中，有两处需要微调：")
print("             add测试文件中add $19,$19,$3指令会导致溢出，MARS会直接结束程序")
print("             而我们所涉及的31条指令集CPU不涉及异常处理，会继续执行，导致结果不同")
print("             因此需要将那条指令及之后的部分全部删除！")
print("             jr测试文件末尾放了一条空指令占位，而参考tb中判定停机的条件是读到空指令")
print("             因此会导致自己的输出比MARS少一轮，建议删除jr末尾的指令，不影响正确性")
print("             如果有发现更好的解决方案，请与作者(ymy1020@tongji.edu.cn)联系")
print("--------------------------------------------------------------------------------------------------")
print("接下来，请按照指示顺序操作：")

flag = True
while(flag):
    choice = input("请问是否使用已有的配置文件？(Y/N)：")
    print('\n')
    if choice == 'N' or choice == 'n':

        flag = False

        # 设置Mars所在的位置
        # marsPath="..\\Mars1.jar"
        marsPath = input("请输入Mars的绝对路径：")
        print("\n")

        # vivado工程名（即.xpr文件的名字）
        # projectName = "cpu_test"
        projectName = input("请输入 CPU 的 vivado 工程名 (即 .xpr 文件的名字)：")
        print('\n')

        # 用于IMEM所使用的的IP核的名字，注意不是自己实例化的名字
        # 例如 目录结构中显示的IP核为："instruction_memory - dist_mem_gen_0"，则此处为dist_mem_gen_0
        print("接下来要求填写 IMEM 中 IP Core 的名字")
        print("注：若目录结构中显示的IP核为：\"instruction_memory - dist_mem_gen_0\"，则此处为dist_mem_gen_0")
        # IPCoreName = "dist_mem_gen_0"
        IPCoreName = input("请输入 IP Core 的名字：")
        print('\n')

        # vivado.bat的位置，注意不是平时启动vivado的可执行文件，而应该去安装目录下找到bin目录
        print("接下来要求输入 vivado.bat 的所在位置")
        print("注：vivado.bat的位置，注意不是平时启动vivado的可执行文件，更不是快捷方式所在目录！")
        print("    而应该去安装目录下找到bin目录")
        print("    该目录一般为\"X:\\Xlinx\\Vivado\\2016.2\\bin\\vivado \" (X为驱动器编号，vivado 为批处理文件)")
        # vivadoPath = "D:\\Xilinx\\Vivado\\2016.2\\bin\\vivado"
        vivadoPath = input("请输入 vivado.bat 的所在位置：")
        print('\n配置文件已经生成，下次可直接沿用！\n')

        f = open("CONFIG_YS.txt", "w")
        f.write(marsPath + '\n')
        f.write(projectName + '\n')
        f.write(IPCoreName + '\n')
        f.write(vivadoPath + '\n')
        f.close()

    elif choice == 'Y' or choice == 'y':

        flag = False

        print("使用配置文件进行仿真...\n")
        f = open("CONFIG_YS.txt", "r")
        lines = f.readlines()
        marsPath = lines[0].strip('\n')
        projectName = lines[1].strip('\n')
        IPCoreName = lines[2].strip('\n')
        vivadoPath = lines[3].strip('\n')

    else:
        print("输入错误，请重新输入！\n")


def instertToHead(path, msg):
    with open(path, "r+") as f:
        old = f.read()
        f.seek(0)
        f.write(msg)
        f.write(old)


def makeVivadoTCL(file):
    with open("test.tcl".format(file), "w") as f:
        f.writelines(
            ["open_project {}.xpr\n".format(projectName),
             "set_property CONFIG.coefficient_file ../../../../testcases/{}.coe [get_ips {}]\n".format(
                file, IPCoreName),
             "generate_target {{simulation}} [get_ips {}]\n".format(
                 IPCoreName),
             "launch_simulation\n",
             "close_sim\n",
             "close_project\n"]
        )

print("测试开始，请耐心等待...\n如需中途退出，请多按几次Ctrl+C并等待片刻\n")

for file in os.listdir(".\\testcases"):
    if file.endswith(".txt"):
        print('Testing {}:\n'.format(file))
        file = os.path.splitext(file)[0]

        txtPath = ".\\testcases\\" + file + ".txt"
        coePath = ".\\testcases\\" + file + ".coe"

        # 获取coe文件
        os.system(
            "java -jar {} {} dump .text HexText {} >nul".format(marsPath, txtPath, coePath))
        instertToHead(
            coePath, "memory_initialization_radix=16;\nmemory_initialization_vector=\n")

        # 获取MARS的result.txt

        os.system(
            "java -jar {} {} >nul".format(marsPath, txtPath))

        makeVivadoTCL(file)

        os.system("{} -mode batch -source test.tcl >nul".format(vivadoPath))

        if not filecmp.cmp(".\\{}.sim\\sim_1\\behav\\output.txt".format(projectName), "result.txt"):
            print("文件.\\{}.sim\\sim_1\\behav\\output.txt与result.txt不一致，请对比检查！\n".format(
                projectName))
            print("是否要继续测试？[Y/N]")
            ch = msvcrt.getch().decode("utf-8").lower() 
            while ch != 'y' and ch != 'n':
                ch = msvcrt.getch().decode("utf-8").lower() 
            print(ch)
            if ch == 'n':
                break
        else:
            print("结果比对无误！\n")

os.system("del /Q test.tcl")
