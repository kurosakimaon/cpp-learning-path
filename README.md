# C++ 进阶学习之路 · C++ Learning Path

> 一套面向**零基础到资深**的 C++ 系统化学习资料库。
> 从语法基础、内存模型、现代特性，到并发编程、设计模式、工程实践，一条路走到底。

![C++](https://img.shields.io/badge/C%2B%2B-11%20%7C%2014%20%7C%2017-00599C?logo=cplusplus)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)
![Build](https://img.shields.io/badge/build-CMake%20%7C%20Makefile-orange)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 📖 这是什么

这是一套**循序渐进**的 C++ 学习代码库。不是零散的代码片段，而是一条完整的成长路线：

- **每个模块都有学习指南**（`tutorial.txt`），师傅口吻，讲清"为什么学、怎么学、坑在哪"
- **每个示例都能独立编译运行**，注释详细，末尾附"老师傅的话"和课后练习
- **交互式可视化**帮你建立算法直觉，而不是死记硬背
- **一个完整实战项目**把零散知识串起来

> 师傅领进门，修行在个人。代码是敲出来的，不是看出来的。

---

## 🗺️ 六大阶段

| 阶段 | 模块 | 内容 |
|------|------|------|
| 🌱 **筑基期** | `beginner/` | 语法基础、函数、指针引用、结构体、类与对象、继承多态、运算符重载、模板入门 |
| 💎 **蜕变期** | `intermediate/` · `algorithms/` | 内存模型与 RAII、STL 容器与算法、智能指针、编程规范、数据结构与排序算法 |
| 🔮 **内功期** | `cpp17/` · `cpp_evolution/` | 类型推导、结构化绑定、`if constexpr`、折叠表达式、`optional/variant/any`、移动语义；C++11→14→17→20 演进 |
| 🔥 **并发专题** | `concurrency/` | 线程基础、同步原语、竞态与死锁、原子操作与内存序、线程池、`async/future`、进程间通信 |
| 👑 **化境期** | `patterns/` · `debugging/` · `testing/` · `exceptions/` · `performance/` | 11 种设计模式、调试排错、单元测试、异常安全、性能优化 |
| 🏆 **大师期** | `boost/` · `build_systems/` · `projects/` | 13 个 Boost 库、Makefile/CMake 构建系统、综合实战项目 |

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
├── cpp17/                   # 内功期 · C++17 现代特性（8 个示例）
├── cpp_evolution/           # C++ 11/14/17/20 版本演进（4 个示例）
├── concurrency/             # 并发编程专题（7 个示例）
├── debugging/               # 调试与排错（3 个示例）
├── testing/                 # 单元测试 · Google Test（2 个示例）
├── exceptions/              # 异常与错误处理（1 个示例）
├── patterns/                # 设计模式（11 个示例）
├── performance/             # 性能优化（2 个示例）
├── build_systems/           # 构建系统 · Makefile + CMake（10 个示例）
├── boost/                   # Boost 库（13 个示例）
└── projects/
    └── todo_cli/            # 综合实战项目：命令行待办管理
```

每个模块目录下都有：
- `index.html` —— 模块导航首页（含学习建议）
- `tutorial.txt` —— 模块学习指南
- `*.cpp` —— 示例源码

---

## 🚀 快速开始

### 1️⃣ 先看路线图

用浏览器打开 [`学习路线图.html`](学习路线图.html) —— 这是整个项目的总目录，所有内容都可点击跳转。

### 2️⃣ 环境要求

| 项目 | 要求 |
|------|------|
| 编译器 | GCC 7+ / Clang 5+ / MSVC 2017+（支持 C++17） |
| 构建工具 | CMake 3.14+ 或 GNU Make |
| Boost（可选） | 仅 `boost/` 目录需要，1.65+ |

**Windows 推荐**：Visual Studio Community（勾选"C++ 桌面开发"）或 MinGW + VS Code
**Linux/macOS**：`sudo apt install build-essential cmake` / `brew install cmake`

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
cmake --build . --target all              # 全部（不含 Boost）
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
| 代码示例 | **68 个** |
| 构建系统示例 | 10 个 |
| 完整实战项目 | 1 个（19 个源文件） |
| 交互式可视化页面 | 2 个 |
| 模块学习指南 | 13 篇 |

---

## 📚 推荐学习顺序

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
3. **做完每章的练习** —— 每个文件末尾都有"小练习"，那才是真正掌握的关键

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
