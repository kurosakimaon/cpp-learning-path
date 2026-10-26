# CMake 入门教程 - 第二讲

> 多文件项目怎么搞

## 一、项目结构

徒弟啊，这一讲咱看看多文件项目怎么组织。

咱这个例子的文件结构：

```text
cmake_02_multi_file/
├── main.cpp        主程序
├── utils.h         工具函数声明
├── utils.cpp       工具函数实现
├── math.h          数学函数声明
├── math.cpp        数学函数实现
├── CMakeLists.txt  CMake 配置
└── tutorial.md     就是你正在看的这个
```

## 二、头文件和源文件那点事儿

C/C++ 里为什么要有 `.h` 和 `.cpp` 分开？

简单说：

- `.h` 文件（头文件）：放声明（函数长啥样、类有哪些成员）
- `.cpp` 文件（源文件）：放实现（函数具体干了啥）

为什么要分开？

1. 接口与实现分离。别人想用你的代码，只需要看 `.h` 文件就知道怎么调用，不用管你内部怎么实现的。
2. 加快编译。改了实现（`.cpp`），只需要重新编这个 `.cpp`；改了接口（`.h`），所有包含它的文件都得重新编。

### 老师傅小知识

`#include "xxx.h"` 本质上就是把 `xxx.h` 的内容原封不动地插入到这一行。就这么简单，没什么魔法。

## 三、include_directories 是干啥的？

你写 `#include "utils.h"` 的时候，编译器去哪儿找这个文件？

搜索顺序大概是这样的：

1. 当前源文件所在的目录
2. 你通过 `-I` 参数（也就是 CMake 里的 `include_directories`）指定的那些目录
3. 系统默认的头文件目录（比如 `/usr/include`）

所以如果你的头文件放在别的文件夹里，就得用 `include_directories` 告诉编译器一声。

举个例子：如果你把所有头文件都放在 `include/` 子目录里，那你就得写：

```cmake
include_directories(include)
```

## 四、set 命令和变量

CMake 里用 `set` 定义变量，用 `${变量名}` 引用变量。

```cmake
set(SOURCES main.cpp utils.cpp math.cpp)
add_executable(demo ${SOURCES})
```

变量名一般习惯用大写，不过 CMake 是大小写不敏感的吗？哎，这个得说清楚：

- CMake 命令名不区分大小写（`add_executable` 和 `ADD_EXECUTABLE` 一样）
- 但是变量名是区分大小写的！`${SOURCES}` 和 `${sources}` 不一样
- 所以最好统一风格，变量名都用大写，省得搞混

> **老师傅提醒**
>
> 除了自己定义的变量，CMake 还有一大堆内置变量。比如：
>
> - `CMAKE_CURRENT_SOURCE_DIR` 当前 `CMakeLists.txt` 所在的源码目录
> - `CMAKE_CURRENT_BINARY_DIR` 当前 `CMakeLists.txt` 对应的构建目录
> - `PROJECT_NAME` 项目名
> - `CMAKE_BUILD_TYPE` 构建类型（Debug/Release）
>
> 这些以后慢慢接触，不用急着记。

## 五、file(GLOB) 的坑（重点！）

`file(GLOB)` 这个命令新手最爱用，因为省事：

```cmake
file(GLOB SOURCES "*.cpp")
```

一把把所有 `.cpp` 文件都收进来，不用一个个列。

但是！老师傅我必须给你敲个警钟：

> 正式项目里别用 `file(GLOB)`！

为啥？最大的坑：新增文件不会自动被检测到！

举个场景：

1. 你有 5 个 `.cpp` 文件，用 `file(GLOB)` 收集起来了
2. 你 `cmake ..` 配置了一下，CMake 记下了这 5 个文件
3. 你又加了第 6 个 `.cpp` 文件
4. 你直接 `make`，结果第 6 个文件没被编译！
5. 链接时报错 "undefined reference"，你找了半天 bug
6. 最后发现是忘了重新 `cmake ..`

这种事儿太常见了，新手踩坑率 100%。

那什么时候能用？

- 小 demo，自己玩的项目
- 你能保证每次加文件都记得重新 `cmake`
- 或者用 CMake 3.12+ 的 `CONFIGURE_DEPENDS` 参数（但也不完美）

正式项目怎么办？

老老实实把源文件一个个列出来。文件多怎么了？多就多点，至少不会出莫名其妙的问题。而且代码审查的时候，加了哪些文件一目了然。

## 六、编译运行步骤

跟第一讲一样，外部构建：

```bash
cd cmake_02_multi_file
mkdir build
cd build
cmake ..
cmake --build .      # 或者 make
```

然后运行：

```bash
./demo                       # Linux/macOS
demo.exe                     # Windows + MinGW/Ninja（在 build 目录下）
Debug\demo.exe               # Windows + Visual Studio（在 build 目录下）
```

看看输出对不对，验证一下多文件编译是不是正常工作了。

## 七、老师傅的经验之谈

1. 源文件列表老老实实写，别贪省事用 GLOB。省那几分钟时间，将来踩坑可能花几小时去找。
2. 头文件保护（`#ifndef ... #define ... #endif`）一定要写，不然头文件被重复包含了会报一堆莫名其妙的错。
3. `include_directories` 是全局的，所有目标都能用到。小项目无所谓，大项目里推荐用 `target_include_directories`，那个是针对单个目标的，更干净。下一讲咱就会讲到。
4. 变量名用大写，这是约定俗成的写法，别人看你的 `CMakeLists.txt` 也顺眼。

---

第二讲就到这儿。把多文件编译搞明白，下一讲咱说说怎么把代码做成库，给别人调用。
