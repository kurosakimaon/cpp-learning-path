# ============================================================
#  Makefile 第5讲：静态库与动态库
#  老师傅手把手教你做库、链接库
# ============================================================
#
#  这一讲我们做两个库：
#    libmathutils.a / libmathutils.so   数学计算库
#    libstringutils.a / libstringutils.so 字符串处理库
#
#  然后写一个主程序，链接这两个库。
#
#  用法：
#    make static    ← 编译静态库版本
#    make shared    ← 编译动态库版本
#    make all       ← 两种都编译
#    make run       ← 运行静态库版本
#    make run_shared ← 运行动态库版本
#    make clean
# ============================================================


# ============================================================
#  一、基本配置
# ============================================================

CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -g
AR       := ar         # 静态库打包工具
ARFLAGS  := rcs        # ar 的参数：r替换 c创建 s建索引


# ============================================================
#  二、源文件与目标文件
# ============================================================

# 数学库的源文件和目标文件
MATH_SRCS := math_utils.cpp
MATH_OBJS := math_utils.o

# 字符串库的源文件和目标文件
STRING_SRCS := string_utils.cpp
STRING_OBJS := string_utils.o

# 主程序
MAIN_SRC := main.cpp
MAIN_OBJ := main.o


# ============================================================
#  三、库文件名字
# ============================================================
#
#  【命名规则】
#  Linux 下库文件名字都要以 lib 开头：
#    静态库：libxxx.a
#    动态库：libxxx.so
#
#  为什么要 lib 开头？
#  因为 -lxxx 链接的时候，编译器会自动去找 libxxx.a 或 libxxx.so
#  比如 -lmathutils → 找 libmathutils.a / libmathutils.so
#
#  这是约定俗成的规矩，别乱起名。

# 静态库
STATIC_MATH_LIB   := libmathutils.a
STATIC_STRING_LIB := libstringutils.a

# 动态库
SHARED_MATH_LIB   := libmathutils.so
SHARED_STRING_LIB := libstringutils.so


# ============================================================
#  四、默认目标
# ============================================================

.PHONY: all static shared clean run run_shared

all: static shared
	@echo ""
	@echo "========== 全部编译完成 =========="
	@echo "静态库: $(STATIC_MATH_LIB), $(STATIC_STRING_LIB)"
	@echo "动态库: $(SHARED_MATH_LIB), $(SHARED_STRING_LIB)"
	@echo "可执行文件: app_static, app_shared"
	@echo "====================================="


# ============================================================
#  五、静态库版本
# ============================================================
#
#  【什么是静态库？】
#  静态库就是把一堆 .o 文件打包成一个文件。
#  链接的时候，把用到的代码整个复制到可执行文件里。
#
#  【怎么制作静态库？】
#  用 ar 命令（archive 归档的意思）：
#    ar rcs libxxx.a file1.o file2.o file3.o
#
#  参数解释：
#    r - replace，如果库已存在就替换里面的成员
#    c - create，创建库（不输出提示信息）
#    s - 建立索引，加快链接速度（相当于 ranlib）
#
#  【怎么链接静态库？】
#  方法1：直接把 .a 文件写在 g++ 命令后面
#    g++ main.o libmathutils.a -o app
#
#  方法2：用 -lxxx -Ldir 方式（更常用）
#    g++ main.o -L. -lmathutils -o app
#    -L. 表示库文件搜索路径是当前目录
#    -lmathutils 表示链接 libmathutils.a 或 .so

static: app_static

# 数学静态库
$(STATIC_MATH_LIB): $(MATH_OBJS)
	$(AR) $(ARFLAGS) $@ $^
	@echo "✅ 静态库 $@ 生成完成"

# 字符串静态库
$(STATIC_STRING_LIB): $(STRING_OBJS)
	$(AR) $(ARFLAGS) $@ $^
	@echo "✅ 静态库 $@ 生成完成"

# 链接静态库生成可执行文件
app_static: $(MAIN_OBJ) $(STATIC_MATH_LIB) $(STATIC_STRING_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN_OBJ) \
		-L. -lmathutils -lstringutils
	@echo "✅ 静态链接可执行文件 $@ 生成完成"


# ============================================================
#  六、动态库版本
# ============================================================
#
#  【什么是动态库？】
#  动态库（也叫共享库）不把代码复制到可执行文件里，
#  只是在可执行文件里记个"引用"。
#  程序运行的时候，操作系统才去加载动态库。
#
#  【怎么做动态库？】
#  两步：
#    1. 编译 .o 的时候加 -fPIC（位置无关代码）
#    2. 链接的时候加 -shared
#
#  什么是 -fPIC？
#    Position Independent Code，位置无关代码。
#    动态库要被加载到内存的不同位置，所以代码里不能有
#    绝对地址，都要用相对地址。-fPIC 就是干这个的。
#
#  【怎么链接动态库？】
#  跟静态库语法一样：-lxxx -Ldir
#  编译器会自动优先找 .so，找不到再找 .a
#  （可以用 -static 强制静态链接）
#
#  【运行时找不到动态库怎么办？】
#  这是新手最常踩的坑！编译的时候用 -L 指定了路径，
#  但运行的时候操作系统不知道去哪找 .so 文件。
#
#  解决方法（三种）：
#    1. 设置 LD_LIBRARY_PATH 环境变量（临时用）
#       export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
#
#    2. 把库安装到系统目录 /usr/lib 或 /usr/local/lib（正式发布）
#       sudo cp libxxx.so /usr/local/lib
#       sudo ldconfig
#
#    3. 编译时加 -Wl,-rpath,/path/to/lib（把库路径写进可执行文件）
#       g++ main.o -L. -lxxx -Wl,-rpath,. -o app
#       -Wl 的意思是把后面的参数传给链接器 ld
#
#  咱们这个例子用第 3 种方法，最方便。

shared: app_shared

# 动态库用的 .o 需要加 -fPIC 编译
# 为了不跟静态库的 .o 混，我们用不同的目标文件
MATH_OBJS_SHARED   := math_utils.pic.o
STRING_OBJS_SHARED := string_utils.pic.o

# 编译带 PIC 的目标文件
%.pic.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@
#
# 解释：
#   -fPIC   生成位置无关代码（动态库必须）
#   -c      只编译不链接
#   $<      依赖文件（.cpp）
#   $@      目标文件（.pic.o）

# 数学动态库
$(SHARED_MATH_LIB): $(MATH_OBJS_SHARED)
	$(CXX) -shared -o $@ $^
	@echo "✅ 动态库 $@ 生成完成"

# 字符串动态库
$(SHARED_STRING_LIB): $(STRING_OBJS_SHARED)
	$(CXX) -shared -o $@ $^
	@echo "✅ 动态库 $@ 生成完成"

# 链接动态库生成可执行文件
app_shared: $(MAIN_OBJ) $(SHARED_MATH_LIB) $(SHARED_STRING_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN_OBJ) \
		-L. -lmathutils -lstringutils \
		-Wl,-rpath,.
#		       ^
#		       |
#		       +-- 把当前目录写进可执行文件的库搜索路径里
#		           这样运行的时候就能找到 .so 了
	@echo "✅ 动态链接可执行文件 $@ 生成完成"


# ============================================================
#  七、通用编译规则（普通 .o，不带 PIC）
# ============================================================

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


# ============================================================
#  八、运行
# ============================================================

run: app_static
	@echo ""
	@echo "========== 运行静态链接版本 =========="
	./app_static

run_shared: app_shared
	@echo ""
	@echo "========== 运行动态链接版本 =========="
	./app_shared


# ============================================================
#  九、清理
# ============================================================

clean:
	rm -f *.o *.a *.so app_static app_shared
	@echo "清理完成！"


# ============================================================
#  十、查看库信息（调试用）
# ============================================================

.PHONY: info

info:
	@echo "========== 库文件信息 =========="
	@echo ""
	@echo "--- 静态库内容 ---"
	@if [ -f $(STATIC_MATH_LIB) ]; then \
		echo "$(STATIC_MATH_LIB):"; \
		ar t $(STATIC_MATH_LIB); \
		echo "  大小: $$(du -h $(STATIC_MATH_LIB) | cut -f1)"; \
	else \
		echo "$(STATIC_MATH_LIB): 未生成（先 make static）"; \
	fi
	@echo ""
	@if [ -f $(STATIC_STRING_LIB) ]; then \
		echo "$(STATIC_STRING_LIB):"; \
		ar t $(STATIC_STRING_LIB); \
		echo "  大小: $$(du -h $(STATIC_STRING_LIB) | cut -f1)"; \
	else \
		echo "$(STATIC_STRING_LIB): 未生成（先 make static）"; \
	fi
	@echo ""
	@echo "--- 动态库信息 ---"
	@if [ -f $(SHARED_MATH_LIB) ]; then \
		echo "$(SHARED_MATH_LIB):"; \
		echo "  大小: $$(du -h $(SHARED_MATH_LIB) | cut -f1)"; \
		file $(SHARED_MATH_LIB); \
	else \
		echo "$(SHARED_MATH_LIB): 未生成（先 make shared）"; \
	fi
	@echo ""
	@if [ -f $(SHARED_STRING_LIB) ]; then \
		echo "$(SHARED_STRING_LIB):"; \
		echo "  大小: $$(du -h $(SHARED_STRING_LIB) | cut -f1)"; \
		file $(SHARED_STRING_LIB); \
	else \
		echo "$(SHARED_STRING_LIB): 未生成（先 make shared）"; \
	fi
	@echo ""
	@echo "--- 可执行文件依赖 ---"
	@if [ -f app_static ]; then \
		echo "app_static (静态链接):"; \
		echo "  大小: $$(du -h app_static | cut -f1)"; \
		ldd app_static 2>&1 | head -5; \
	else \
		echo "app_static: 未生成"; \
	fi
	@echo ""
	@if [ -f app_shared ]; then \
		echo "app_shared (动态链接):"; \
		echo "  大小: $$(du -h app_shared | cut -f1)"; \
		echo "  依赖的库:"; \
		ldd app_shared; \
	else \
		echo "app_shared: 未生成"; \
	fi
	@echo "=================================="
