# ============================================================
#  CMake 示例 03 - 库的构建与链接
#  根目录 CMakeLists.txt
#  老师傅注释版
# ============================================================

cmake_minimum_required(VERSION 3.10)

project(LibraryDemo)

# ============================================================
#  add_subdirectory：添加子目录
# ============================================================
#
# 这命令告诉 CMake："去子目录里找另一个 CMakeLists.txt 来处理"。
# 大项目就是这么组织的——每个模块一个子目录，各管各的。
#
# 这里先加 mathlib 再加 app，读起来和依赖方向一致。
# 注意：CMake 会在生成阶段解析目标，所以“库目标写在后面”
# 本身不会导致 "Target not found"；真正不能反的是
# add_executable / add_library 与 target_* 命令的顺序——
# 必须先创建目标，再给这个目标设置属性。
#
add_subdirectory(mathlib)   # 先构建数学库
add_subdirectory(app)       # 再构建应用程序


# ============================================================
#  老师傅的话：
#  这个例子虽小，但五脏俱全。
#
#  重点要理解三件事：
#
#  1. add_library 用来创建库，STATIC 是静态库，SHARED 是动态库
#  2. add_subdirectory 用来组织多目录项目
#  3. target_link_libraries 用来把库链接到目标上
#
#  另外要注意 target_include_directories 和 include_directories 的区别：
#    include_directories 是老派写法，全局生效
#    target_include_directories 是现代写法，针对单个目标
#
#  现代 CMake 推荐用 target_* 系列命令，好处是：
#    - 依赖关系清晰，谁需要什么一目了然
#    - 可以自动传递（PUBLIC 的话，链接者也能用上）
#    - 不会造成全局污染
#
#  下一讲咱说说怎么加编译选项、怎么用 option 控制编译开关。
# ============================================================
