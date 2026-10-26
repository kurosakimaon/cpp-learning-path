# CMake 入门教程 - 第五讲

> 现代 CMake —— 基于目标的写法

## 一、什么是"现代 CMake"？

徒弟啊，学 CMake 你得知道，CMake 有"新""旧"两种写法。

### 老派 CMake（传统写法）

主要特征：用全局变量。比如：

- `include_directories(...)` —— 全局加头文件路径
- `link_libraries(...)` —— 全局链接库
- `add_definitions(...)` —— 全局加宏定义
- `set(CMAKE_CXX_FLAGS "...")` —— 全局改编译选项

问题：全局的东西一多，就乱。A 库加的路径影响了 B 库，调试起来老费劲了。

### 现代 CMake（基于目标）

主要特征：一切围绕目标（target）。比如：

- `target_include_directories(...)` —— 给某个目标加头文件路径
- `target_link_libraries(...)` —— 给某个目标链接库
- `target_compile_definitions(...)` —— 给某个目标加宏定义
- `target_compile_options(...)` —— 给某个目标加编译选项
- `target_compile_features(...)` —— 给某个目标指定 C++ 特性

好处：

1. 每个目标的依赖清清楚楚，不污染全局
2. 依赖关系可以自动传递（PUBLIC）
3. 封装性好，内部实现不对外暴露
4. 可复用性强，别人用你的库也方便

简单说：老派是"全局一锅炖"，现代是"目标各管各"。

现在业界都推荐用现代 CMake 的写法。你新写项目，直接上现代写法就行，别学老派的了。

## 二、PUBLIC / PRIVATE / INTERFACE —— 核心中的核心

这三个关键字是现代 CMake 的灵魂，必须搞懂。

咱用最通俗的话讲：

假设 A 链接了 B（`target_link_libraries(A B)`）：

- `PRIVATE`：B 的这个东西，只有 B 自己用，A 不用管。比如 B 内部的头文件路径、B 自己的编译选项。
- `PUBLIC`：B 的这个东西，B 自己要用，A 也得用。会从 B 传递给 A。比如 B 的公共头文件路径、B 要求的 C++ 标准。
- `INTERFACE`：B 自己不用，但 A 用了 B 就需要。比如 header-only 库，它自己不用编译，但用它的人需要它的头文件路径。

### 提醒：别把两处关键字混为一谈

上面说的是"B 自己的属性"怎么传给 A。而 A 写 `target_link_libraries(A PRIVATE B)` 时，`PRIVATE`/`PUBLIC` 决定的是：A 会不会把 B 继续传给"链接 A 的人"。两处都叫 PUBLIC/PRIVATE，但作用对象不同。

### 口诀

- 自己用、别人也用 → `PUBLIC`
- 自己用、别人不用 → `PRIVATE`
- 自己不用、别人用 → `INTERFACE`

### 举几个实际例子

例 1：头文件路径

- 库的公共头文件目录 → `PUBLIC`（用户也要用）
- 库的内部头文件目录 → `PRIVATE`（用户不用）

例 2：编译选项

- 警告选项（`-Wall`）→ `PRIVATE`（我自己开警告，不强迫用户）
- C++ 标准（`-std=c++17`）→ `PUBLIC`（用我的库也得是 C++17）

例 3：链接的库

- 如果 B 库的头文件里用到了 C 库的类型 → `PUBLIC`
- 如果 B 库只在 `.cpp` 里用了 C 库 → `PRIVATE`

### 为什么这么设计？

因为"依赖传递"很重要。你想啊，如果 B 是个库，A 用了 B。那 A 编译的时候，需不需要知道 B 的头文件在哪？需要！所以 B 的头文件路径得"传"给 A。这就是 `PUBLIC` 的意义。

那 B 内部用了什么辅助库、开了什么警告，A 需要知道吗？不需要！那就是 `PRIVATE`。

## 三、target_* 命令一览

现代 CMake 的 `target_*` 命令有一大家子，常用的这些：

- `target_include_directories` —— 头文件搜索路径
- `target_link_libraries` —— 链接的库
- `target_compile_definitions` —— 宏定义（`-D`）
- `target_compile_options` —— 编译选项
- `target_compile_features` —— C++ 特性（比如 `cxx_std_17`）
- `target_sources` —— 源文件（比较少直接用）
- `target_link_directories` —— 库搜索路径（尽量别用，用 `find_package`）
- `target_link_options` —— 链接选项

它们的用法都差不多：

```cmake
target_xxx(目标名  <PUBLIC|PRIVATE|INTERFACE>  参数...)
```

都支持 `PUBLIC` / `PRIVATE` / `INTERFACE`。都支持 generator expressions。

统一的接口，是不是很整齐？

## 四、Generator Expressions（生成器表达式）

这玩意儿刚开始看挺吓人的，全是尖括号 `$<$<...>:...>`。其实理解了就那么回事。

### 为什么要有生成器表达式？

CMake 执行分两个阶段：

1. 配置阶段（Configure）：读 `CMakeLists.txt`，生成构建文件
2. 构建阶段（Build）：真正调用编译器编译

有些信息配置阶段不知道，构建阶段才知道。最典型的是多配置生成器：VS 的一个工程里同时有 Debug、Release 等多个配置，具体编哪个要等构建时才知道。

普通变量在配置阶段就展开了，没法表达这种"按配置取值"的条件。所以就有了生成器表达式——构建阶段才展开。

（编译器 ID、指针大小这类信息在配置阶段通常已经能确定，但写成生成器表达式后，同一份目标定义也能在生成阶段按配置/平台自动展开，不用手写一堆 `if`。）

### 常用的生成器表达式

1. 条件判断：

   - `$<CONFIG:Debug>` → Debug 配置时为 1，否则 0
   - `$<CXX_COMPILER_ID:GNU>` → 编译器是 GCC 时为 1

2. 条件取值（类似三目运算符）：

   - `$<IF:条件,真值,假值>`

3. 带条件的参数（最常用！）：

   ```cmake
   $<$<CONFIG:Debug>:-g>
   ```

   意思是：如果是 Debug，就加 `-g`，否则什么都不加

   ```cmake
   $<$<CXX_COMPILER_ID:GNU>:-Wall>
   ```

   意思是：如果编译器是 GCC，就加 `-Wall`

4. 路径相关：

   - `$<BUILD_INTERFACE:路径>` → 构建时用这个路径
   - `$<INSTALL_INTERFACE:路径>` → 安装后用这个路径

### 语法解析

```cmake
$<$<CONFIG:Debug>:-g>
```

外层 `$<A:B>` 表示：如果 A 为真，整个表达式的值是 B，否则是空。A 是 `$<CONFIG:Debug>`，也就是判断是不是 Debug。B 是 `-g`。

所以合起来就是：Debug 时为 `-g`，否则为空。

嵌套多了看起来眼晕，其实就是层层套娃。写多了就习惯了。

## 五、ALIAS 目标（别名目标）

```cmake
add_library(mathlib::mathlib ALIAS mathlib)
```

这行代码干啥的？给 `mathlib` 起个别名 `mathlib::mathlib`。

为啥要起带两个冒号的名字？这是现代 CMake 的惯例。`find_package` 找出来的库，目标名都是 `xxx::xxx` 格式的，比如 `Boost::boost`, `Threads::Threads`, `Qt5::Core` 等。

我们自己的库也用这种命名方式，好处是：

1. 风格统一，一看就知道是 CMake 目标
2. 子目录里用 `add_subdirectory` 和用 `find_package`，目标名一样，代码不用改
3. 防止命名冲突，命名空间式的名字

推荐你也养成这个习惯，库目标都起个别名。

## 六、怎么玩这个例子？

```bash
cd cmake_05_modern
mkdir build && cd build
cmake ..
cmake --build .
./app/stats_app
# Windows + MinGW/Ninja：app\stats_app.exe
# Windows + Visual Studio：app\Debug\stats_app.exe
```

试试下面这些：

1. 尝试在 `app/main.cpp` 里包含 `internal_utils.h`（就是代码里注释掉的那行）看看会不会报错——应该会，因为 `internal_utils.h` 是 `PRIVATE` 的。
2. 试试 Debug 和 Release 两种构建

   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Debug
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

   看看编译出来的程序大小有没有区别。

## 七、老师傅的经验之谈

1. 新项目一律用现代 CMake 写法。别再用 `include_directories`、`link_libraries` 这种全局命令了。老项目能改也尽量改过来。
2. `PUBLIC`/`PRIVATE`/`INTERFACE` 刚开始可能有点绕，多写几次就有感觉了。拿不准的时候就想想："用我这个库的人需不需要这个东西？"需要 → PUBLIC，不需要 → PRIVATE。
3. 生成器表达式虽然语法丑，但真的好用。特别是处理多编译器、多配置的情况。先掌握最常用的几个就行。
4. 目标名字用 `xxx::xxx` 格式，跟 `find_package` 风格一致。这是行业惯例。
5. 现代 CMake 的核心思想："每个目标自己管自己的属性，需要传递的标记为 PUBLIC。"就这么简单。

---

第五讲就到这儿。现代 CMake 的内容很多，咱讲的这些是最核心的，掌握了就能写得有模有样了。下一讲咱来个大的——完整项目结构，把之前学的东西全都串起来。
