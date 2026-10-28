# C++ 进阶学习之路 · C++ Learning Path

> 一套面向**零基础到资深**的 C++ 系统化学习资料库。
> 从语法基础、内存模型、现代特性，到并发编程、设计模式、工程实践，一条路走到底。

![C++](https://img.shields.io/badge/C%2B%2B-11%20%7C%2014%20%7C%2017%20%7C%2020-00599C?logo=cplusplus)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)
![Build](https://img.shields.io/badge/build-CMake%20%7C%20Makefile-orange)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 📖 这是什么

这是一套**循序渐进**的 C++ 学习代码库。不是零散的代码片段，而是一条完整的成长路线：

- **每个模块（或构建示例）都有学习指南**（`tutorial.md`，Markdown 格式），师傅口吻，讲清"为什么学、怎么学、坑在哪"
- **示例注释详细**，多数示例末尾附"老师傅的话"和课后练习；只有测试演示会故意保留失败用例，依赖库的示例需先装好对应库
- **交互式可视化**帮你建立算法直觉，而不是死记硬背
- **分步图解**把移动语义、内存与智能指针、异常栈展开、内存序、虚函数、链接、并发、IPC、Asio 这些抽象机制，以及链表、动态数组、栈/队列、二叉搜索树、哈希表、堆这些数据结构拆成可翻页的动画，支持自动播放
- **一个完整实战项目**把零散知识串起来

> 师傅领进门，修行在个人。代码是敲出来的，不是看出来的。

---

## 🗺️ 模块地图

| 层次 | 模块 | 内容 |
|------|------|------|
| 🌱 **基础语法** | `beginner/` | 语法基础、函数、指针引用、结构体、类与对象、继承多态、运算符重载、模板入门 |
| 💎 **进阶与算法** | `intermediate/` · `algorithms/` · `exceptions/` | 内存模型与 RAII、STL 容器与算法、智能指针、编程规范、数据结构与排序算法、异常处理 |
| 🔮 **现代 C++** | `cpp_evolution/` · `cpp17/` | 先读 `cpp_evolution/` 建立 C++11→14→17→20 的版本脉络，再读 `cpp17/` 逐个深潜：类型推导、结构化绑定、`if constexpr`、折叠表达式、`optional/variant/any`、移动语义（两个模块分工不重复） |
| 🔥 **并发专题** | `concurrency/` | 线程基础、同步原语、竞态与死锁、原子操作与内存序、线程池、`async/future`、进程间通信 |
| 👑 **工程质量** | `patterns/` · `debugging/` · `testing/` · `performance/` | 设计模式、调试排错、单元测试、性能优化 |
| 🏆 **生态与实战** | `boost/` · `build_systems/` · `projects/` | 13 个 Boost 示例（13 个常用库）、Makefile/CMake 构建系统（附 [CMake 实操速查](build_systems/cmake_guide.html)）、综合实战项目 |

---

## 📂 目录结构

```
.
├── 学习路线图.html          # ⭐ 总目录，从这开始（浏览器打开）
├── CMakeLists.txt           # 顶层构建配置
├── .gitignore
│
├── beginner/                # 筑基期 · C++ 入门（10 个示例）
├── intermediate/            # 蜕变期 · STL 与内存（5 个示例）
├── algorithms/              # 算法与数据结构（2 个示例 + 2 个可视化页面）
├── visuals/                 # 图解专区（16 个分步图解 + 首页，共用样式与脚本）
├── cpp_evolution/           # 版本脉络 · C++11→14→17→20（4 个示例，现代 C++ 从这里先读）
├── cpp17/                   # 内功期 · 现代 C++ 特性深潜（8 个示例，首页卡片已排好 ①→⑧）
├── concurrency/             # 并发编程专题（7 个示例）
├── debugging/               # 调试与排错（3 个示例）
├── testing/                 # 单元测试 · Google Test（2 个示例）
├── exceptions/              # 异常与错误处理（1 个示例）
├── patterns/                # 设计模式（11 个示例）
├── performance/             # 性能优化（2 个示例）
├── build_systems/           # 构建系统 · Makefile + CMake（11 个示例 + 1 篇 CMake 实操速查）
├── boost/                   # Boost 库（13 个示例）
└── projects/
    └── todo_cli/            # 综合实战项目：命令行待办管理
```

模块目录的组织方式：
- 大多数模块目录下有 `index.html`（模块导航首页）、`tutorial.md`（学习指南）和 `*.cpp`（示例源码）
- `build_systems/` 没有顶层 `tutorial.md`，每个构建示例目录自带 `tutorial.md` 和完整工程；模块导航在 `build_systems/index.html`，CMake 的命令与预期输出对照在 `build_systems/cmake_guide.html`
- `projects/todo_cli/` 是分层工程，带 `index.html`、`tutorial.md`、`include/`、`src/`、`tests/`

---

## 🚀 快速开始

### 1️⃣ 先看路线图

用浏览器打开 [`学习路线图.html`](学习路线图.html) —— 这是整个项目的总目录，所有内容都可点击跳转。

### 2️⃣ 环境要求

| 项目 | 要求 |
|------|------|
| 编译器 | GCC 9+ / Clang 10+ / MSVC 2019+（支持 C++17）；C++20 示例需 GCC 11+ / Clang 12+ / MSVC 2019 16.11+ |
| 构建工具 | CMake 3.15+ 或 GNU Make（`cmake --install` 需要 3.15+） |
| Boost（可选） | 仅 `boost/` 目录需要，1.74+（用到了新版 Boost.Filesystem 的 `copy_options`） |

**Windows 推荐**：Visual Studio Community（勾选"C++ 桌面开发"）或 MinGW + VS Code
**Linux/macOS**：`sudo apt install build-essential cmake` / `brew install cmake`

> 说明：C++17 并行算法是否真正并行、需要链接什么库，取决于标准库实现和构建配置。GCC 的 libstdc++ 常见配置需要 TBB 后端（安装 TBB 并链接 `-ltbb`）；如果构建 libstdc++ 时未启用 TBB，`par` 策略会退化为串行执行，而不是报错。Clang 的 libc++ 通常也依赖 TBB。MSVC 提供执行策略重载，但并行执行支持并不完整，可能按串行执行。`cpp17/parallel_algorithms.cpp` 顶部的 `NO_PARALLEL` 只控制是否运行并行分支，不能替代安装/链接后端；以所用工具链文档为准。

### 3️⃣ 编译运行

**方式一：CMake（推荐）**

> 刚装好 CMake、不知道该敲哪条命令、不知道每条命令跑完应该看到什么，
> 直接看 [`build_systems/cmake_guide.html`](build_systems/cmake_guide.html)：命令速查 + 每一讲预期输出 + 常见报错对照。

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release

# 编译单个
cmake --build . --target beginner_01_basics

# 按阶段批量编译
cmake --build . --target all_beginner    # 全部入门示例
cmake --build . --target all_demos        # 全部（不含 Boost）
```

**方式二：单文件直接编译**

```bash
g++ -std=c++17 -Wall -Wextra beginner/01_basics.cpp -o demo
./demo
```

**方式三：Makefile 教程**

```bash
cd build_systems/makefile_01_basic
make        # 编译
make run    # 运行
make clean  # 清理
```

---

## 📊 内容规模

| 类别 | 数量 |
|------|------|
| 模块代码示例 | **68 个** |
| 构建系统示例 | 11 个（6 个 CMake + 5 个 Makefile 工程） |
| 完整实战项目 | 1 个（17 个文件：8 个头文件 + 8 个实现文件 + 1 个测试文件） |
| 交互式可视化页面 | 2 个（排序、数据结构） |
| 实操速查页面 | 1 个（`build_systems/cmake_guide.html`：CMake 命令 + 预期输出 + 报错对照） |
| 分步图解页面 | 16 个（移动语义、内存/RAII、智能指针、异常栈展开、内存序、vtable、链接、并发、IPC、Asio + 链表、动态数组、栈/队列、BST、哈希表、堆），支持自动播放 |
| 模块学习指南 | 12 篇（另有 11 篇构建教程 + 1 篇实战项目教程） |

---

## 📚 一种可行的学习顺序

```
beginner  →  intermediate  →  cpp_evolution  →  cpp17
                                      ↓
                          concurrency / algorithms
                                      ↓
                 build_systems  ← 先学会把多文件项目构建起来
                                      ↓
                patterns / debugging / testing / exceptions / performance
                                      ↓
                                  boost
                                      ↓
                            projects/todo_cli  ← 综合实战
```

> 现代 C++ 这一步为什么是两个模块：先在 `cpp_evolution/` 建立
> "哪个版本加了什么、为什么加"的版本地图，再去 `cpp17/`
> 按首页排好的 ①→⑧ 逐个特性深潜写法与坑。分工不重复，同一个特性不用读两遍。

> `build_systems/` 为什么提前：设计模式、单元测试、性能优化都要先把多文件工程
> 编出来才看得见效果。构建这块补上之后，后面的模块才不至于边学边卡。

**三条铁律：**

1. **不要跳级** —— 指针、内存、RAII 这三关必须过，不然后面越学越糊涂
2. **亲手敲代码** —— 看懂了不代表会写，每个示例都要改一改、跑一跑
3. **做完每章的练习** —— 多数示例末尾有"小练习"，那才是真正掌握的关键

---

## 📖 配套书单

demo 只是带你入门，要真正成为高手，经典书是绕不开的（详见路线图页面底部）：

- 《C++ Primer》—— 入门必读
- 《Effective C++》—— 55 条最佳实践
- 《深度探索 C++ 对象模型》—— 理解底层
- 《STL 源码剖析》—— 容器底层原理
- 《C++ 并发编程实战》—— 并发权威指南
- 《设计模式》（GoF）—— 设计模式圣经

---

## 🤝 贡献

欢迎提交 Issue 和 PR！如果发现错误、有更好的示例，或者想补充新的学习内容，都非常欢迎。

---

## 📄 License

[MIT](LICENSE) —— 自由使用、修改、分发。

---

<div align="center">

**师傅领进门，修行在个人。**

⭐ 如果这套资料对你有帮助，欢迎点个 Star

</div>
