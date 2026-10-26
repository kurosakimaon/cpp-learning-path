# CMake 入门教程 - 第三讲

> 库的构建与链接

## 一、什么是库？

徒弟啊，咱先唠唠什么是库。

简单说，库就是一堆编译好了的代码，打包在一起，给别人用。

比如你写了一套数学函数，别人想用，总不能把你的 `.cpp` 文件都拷过去吧？太麻烦了。做成库的话，别人只要拿到库文件和头文件，就能用了。

库分两种：静态库和动态库。

## 二、静态库 vs 动态库

咱用大白话讲：

### 静态库（STATIC）

- 编译的时候，把库的代码完完整整复制到可执行文件里
- 结果：可执行文件比较大，但不依赖外部库文件
- 发布的时候，只要给一个 exe 就行，不用带库文件
- Linux 下后缀是 `.a`；Windows + MSVC 下是 `.lib`，MinGW 下是 `.a`

### 动态库（SHARED / 共享库）

- 编译的时候，只在可执行文件里记个"引用"
- 程序运行的时候，才去加载动态库
- 结果：可执行文件比较小，但运行时需要 `.so`/`.dll` 文件
- 好处：多个程序可以共享同一个库（省内存）
  库升级了不用重新编译程序（只要接口没变）
- Linux 下后缀是 `.so`，Windows 下是 `.dll`
  （Windows 链接时还需要一个导入库：MSVC 是 `.lib`，MinGW 是 `.dll.a`）

### 怎么选？

- 小项目、不想折腾 → 静态库，简单直接
- 大项目、多个程序共用 → 动态库，省空间
- 第三方库通常同时提供静态库和动态库，具体用哪种看发行版和安装选项（Qt 常用动态库，Boost 两种都有）

## 三、add_library 命令

创建库就用 `add_library`，跟 `add_executable` 很像：

```cmake
add_library(mathlib STATIC math_functions.cpp)
```

第一个参数是库的名字（随便起，后面链接的时候用这个名字）。第二个参数是库的类型：`STATIC` 或 `SHARED`。后面跟着源文件列表。

就这么简单。

## 四、add_subdirectory 命令

项目大了，代码要分目录放。每个子目录有自己的 `CMakeLists.txt`。根目录的 `CMakeLists.txt` 用 `add_subdirectory` 把子目录加进来。

```cmake
add_subdirectory(mathlib)
add_subdirectory(app)
```

CMake 会按顺序去处理这些子目录里的 `CMakeLists.txt`。

### 注意

虽然 `target_link_libraries` 可以引用后面才定义的库目标，先写 app 再写 mathlib 通常也能配置、链接成功，但建议还是按依赖顺序写：被依赖的库放前面，可读性更好。

真正有严格顺序要求的是"创建目标"和"给目标设属性"：必须先 `add_executable` / `add_library`，再对这个目标调用 `target_link_libraries`、`target_include_directories` 等命令。

## 五、target_link_libraries 命令

把库链接到可执行文件上：

```cmake
target_link_libraries(math_app PRIVATE mathlib)
```

意思是：`math_app` 这个目标，要链接 `mathlib` 这个库。

`PRIVATE` 是啥意思？咱先简单理解为"我自己用"就行，下几讲再深入讲 `PUBLIC`/`PRIVATE`/`INTERFACE` 的区别。

## 六、target_include_directories vs include_directories

这个很重要，徒弟你好好听。

### 老派写法：include_directories

```cmake
include_directories(/path/to/headers)
```

特点：全局生效。写了之后，这个 `CMakeLists.txt` 以及所有子目录里的所有目标，都能用上这些头文件路径。

问题：太"脏"了。有些目标根本不需要这些头文件，也被强行加上了。大项目里各种路径混在一起，乱得很。

### 现代写法：target_include_directories

```cmake
target_include_directories(mathlib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
```

特点：只针对 `mathlib` 这个目标。而且配合 `PUBLIC`/`PRIVATE`/`INTERFACE`，还能控制要不要把路径传递给链接这个库的人。

好处：

1. 干净——谁需要谁加，不污染全局
2. 智能——`PUBLIC` 的路径会自动传递给依赖者
3. 清晰——看一个目标的 CMake 代码就知道它需要啥

### 老师傅结论

新项目一律用 `target_include_directories`，别用 `include_directories` 了，那是老黄历了。

## 七、PUBLIC / PRIVATE / INTERFACE 初体验

这三个关键字是现代 CMake 的精髓，咱先混个脸熟：

- `PRIVATE`：只有我自己用，跟我链接的人不需要知道
- `PUBLIC`：我自己用，跟我链接的人也需要（要传出去）
- `INTERFACE`：我自己不用，但跟我链接的人需要

举个例子（数学库 `mathlib`）：

- `mathlib` 自己编译需要 `math_functions.h` → 自己要用
- 用 `mathlib` 的人（app）也需要 `math_functions.h` → 别人也要用
- 所以用 `PUBLIC`

再举个例子（假设 `mathlib` 内部用了一个内部头文件 `internal.h`）：

- `mathlib` 自己编译需要 `internal.h` → 自己要用
- 用 `mathlib` 的人不需要 `internal.h` → 别人不用
- 所以用 `PRIVATE`

### 口诀

- 自己用、别人也用 → `PUBLIC`
- 自己用、别人不用 → `PRIVATE`
- 自己不用、别人用 → `INTERFACE`

`INTERFACE` 啥时候用？比如一个只有头文件的库（header-only library），它自己没有源文件，不用编译，但用它的人需要包含它的头文件。这时候就用 `INTERFACE`。

## 八、编译运行

```bash
cd cmake_03_library
mkdir build
cd build
cmake ..
cmake --build .
```

运行：

```bash
./app/math_app              # Linux/macOS
app\math_app.exe            # Windows + MinGW/Ninja（在 build 目录下）
app\Debug\math_app.exe      # Windows + Visual Studio（在 build 目录下）
```

看看输出是不是正确。

## 九、老师傅的经验之谈

1. 能用静态库就先用静态库，简单，不容易出运行时找不到库的问题。等项目大了、有需要了再考虑动态库。
2. 用 `target_include_directories` 代替 `include_directories`。这是现代 CMake 的标志之一。
3. `target_link_libraries` 也要养成写 `PRIVATE`/`PUBLIC` 的习惯，别偷懒省略。省略了 CMake 默认行为有点复杂，容易搞混。
4. 子目录建议按依赖顺序写，被依赖的放前面，方便阅读；CMake 本身能在生成阶段解析后定义的库目标。
5. 库的名字和目录名可以一样也可以不一样，不过建议保持一致，省得找起来麻烦。

---

第三讲就到这儿。库的概念很重要，后面的例子里咱会大量用到。下一讲讲选项和配置，看看怎么让 CMake 项目更灵活。
