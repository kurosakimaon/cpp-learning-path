# ============================================================
# 示例 1：Makefile 入门 —— 最基础的写法
# 老师傅带徒弟，一行一行讲明白
# ============================================================
#
# 【什么是 Makefile？】
# 徒弟啊，Makefile 就是个"施工图纸"。你告诉 make 工具：
#   - 要造什么东西（目标）
#   - 用什么材料造（依赖）
#   - 怎么造（命令）
# make 工具就会照着图纸干活，而且只干需要干的活——
# 如果"材料"比"成品"旧，它就不瞎忙活了。
#
# 【规则的基本语法】
# 目标: 依赖文件1 依赖文件2 ...
# 	命令1
# 	命令2
# 	...
#
# 注意！注意！注意！
# 命令前面必须是 TAB 键缩进，不能是空格！
# 这是新手最容易踩的坑，踩过的人能从天安门排到八达岭。
# ============================================================


# ---- 可执行文件后缀 ----
# Windows 上 g++ 会自动生成 main.exe，目标名也得跟着带 .exe。
# 用 uname 判断平台：某些 make/终端环境里 OS 变量不一定存在。
UNAME_S := $(shell uname -s 2>/dev/null)
ifeq ($(OS),Windows_NT)
    EXE := .exe
else ifneq (,$(findstring MINGW,$(UNAME_S)))
    EXE := .exe
else ifneq (,$(findstring MSYS,$(UNAME_S)))
    EXE := .exe
else ifneq (,$(findstring CYGWIN,$(UNAME_S)))
    EXE := .exe
else
    EXE :=
endif
TARGET := main$(EXE)


# ---- 第一条规则：最终目标 ----
# 目标名叫 main（Windows 上是 main.exe），得有 main.cpp 这个依赖
# 怎么生成？用 g++ 编译
$(TARGET): main.cpp
	g++ -o $@ main.cpp
#   ^
#   |
#   这里是 TAB，不是空格！！！
#
# 【解释】
# main       —— 目标（target）：我们想要生成的东西
# main.cpp   —— 依赖（prerequisite）：生成目标需要的材料
# g++ 那一行 —— 命令（recipe）：怎么从依赖生成目标
#
# 【make 的工作原理】
# 当你敲 make 命令时，make 会：
#   1. 找到 Makefile 文件（默认找 Makefile、makefile、GNUmakefile）
#   2. 执行第一条规则（也就是 main 这条）
#   3. 比较时间戳：
#      - 如果 main 不存在 → 必须编译
#      - 如果 main.cpp 比 main 新 → 重新编译（说明源码改了）
#      - 如果 main 比 main.cpp 新 → 啥也不干（已经是最新的了）
#
# 这就是 make 最核心的智慧："时间戳比较"。
# 大项目里几百个文件，每次只编译改了的，省老鼻子时间了。


# ---- 第二条规则：清理 ----
# 目标叫 clean，没有依赖（就是纯粹要执行一些命令）
# 作用是删除编译出来的可执行文件，让项目回到"干净"状态
clean:
	rm -f main main.exe
#
# 【解释】
# clean 这个目标比较特殊：
#   - 它不生成叫 clean 的文件
#   - 它就是执行一些清理命令
#   - 所以你得敲 make clean 才会执行它
#
# 【新手警告】
# 徒弟你记着：如果当前目录下刚好有个叫 clean 的文件，
# make 一看"clean 文件存在，又没有依赖，那肯定是最新的"，
# 就不执行命令了！这就是为什么后面要学 .PHONY 伪目标。
# 咱先卖个关子，第二个示例再讲。
