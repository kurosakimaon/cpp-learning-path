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
- **一个完整实战项目**把零散知识串起来

> 师傅领进门，修行在个人。代码是敲出来的，不是看出来的。

---

## 🗺️ 模块地图

| 层次 | 模块 | 内容 |
|------|------|------|
| 🌱 **基础语法** | `beginner/` | 语法基础、函数、指针引用、结构体、类与对象、继承多态、运算符重载、模板入门 |
| 💎 **进阶与算法** | `intermediate/` · `algorithms/` · `exceptions/` | 内存模型与 RAII、STL 容器与算法、智能指针、编程规范、数据结构与排序算法、异常处理 |
| 🔮 **现代 C++** | `cpp17/` · `cpp_evolution/` | 类型推导、结构化绑定、`if constexpr`、折叠表达式、`optional/variant/any`、移动语义；C++11→14→17→20 演进 |
| 🔥 **并发专题** | `concurrency/` | 线程基础、同步原语、竞态与死锁、原子操作与内存序、线程池、`async/future`、进程间通信 |
| 👑 **工程质量** | `patterns/` · `debugging/` · `testing/` · `performance/` | 设计模式、调试排错、单元测试、性能优化 |
| 🏆 **生态与实战** | `boost/` · `build_systems/` · `projects/` | 13 个 Boost 库、Makefile/CMake 构建系统、综合实战项目 |

---

## 📂 目录结构

```
.
├── 学习路线图.html          # ⭐ 总目录，从这开始（浏览器打开）
├── CMakeLists.txt           # 顶层构建配置
├── .gitignore
├── common/                  # 公共工具头（历史示例用；当前目标按需包含）
│
├── beginner/                # 筑基期 · C++ 入门（10 个示例）
├── intermediate/            # 蜕变期 · STL 与内存（5 个示例）
├── algorithms/              # 算法与数据结构（2 个示例 + 2 个可视化页面）
├── cpp17/                   # 内功期 · C++17 现代特性（8 个示例）
├── cpp_evolution/           # C++ 11/14/17/20 版本演进（4 个示例）
├── concurrency/             # 并发编程专题（7 个示例）
├── debugging/               # 调试与排错（3 个示例）
├── testing/                 # 单元测试 · Google Test（2 个示例）
├── exceptions/              # 异常与错误处理（1 个示例）
├── patterns/                # 设计模式（11 个示例）
├── performance/             # 性能优化（2 个示例）
├── build_systems/           # 构建系统 · Makefile + CMake（11 个示例）
├── boost/                   # Boost 库（13 个示例）
└── projects/
    └── todo_cli/            # 综合实战项目：命令行待办管理
```

模块目录的组织方式：
- 大多数模块目录下有 `index.html`（模块导航首页）、`tutorial.md`（学习指南）和 `*.cpp`（示例源码）
- `build_systems/` 没有顶层 `tutorial.md`，每个构建示例目录自带 `tutorial.md` 和完整工程；模块导航在 `build_systems/index.html`
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
| 模块学习指南 | 12 篇（另有 11 篇构建教程 + 1 篇实战项目教程） |

---

## 📚 一种可行的学习顺序

```
beginner  →  intermediate  →  cpp17 / cpp_evolution
                                      ↓
                          concurrency / algorithms
                                      ↓
                patterns / debugging / testing / exceptions / performance
                                      ↓
                          boost / build_systems
                                      ↓
                            projects/todo_cli  ← 综合实战
```

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
