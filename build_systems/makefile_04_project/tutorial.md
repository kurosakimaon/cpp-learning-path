# 示例 4：完整项目结构 —— 老师傅的结业课

徒弟，恭喜你走到了最后一节课。这节课啊，咱把前面学的所有东西串起来，做一个真正能用在项目上的 `Makefile` 模板。

这个 `Makefile` 你可以直接拿去用，以后做新项目，改改目录和目标名就行。

## 一、这节课学什么？

1. 标准项目目录结构

   ```text
   src/     源文件
   include/ 头文件
   obj/     中间文件
   bin/     可执行文件
   lib/     库文件
   ```

   源码和生成物分离，干净整洁。

2. 递归发现源文件

   用 `$(shell find ...)` 递归找出所有 `.cpp` 子目录再多也不怕。

3. 自动依赖生成（重头戏！）

   - `-MMD -MP` 编译器选项，编译的时候顺手生成 `.d` 依赖文件
   - 改了头文件，`make` 自动知道哪些文件需要重编
   - 这是专业 `Makefile` 的标配。

4. 顺序依赖（order-only prerequisite）

   目录用 `|` 隔开的顺序依赖，目录变了不触发重编。

5. 完整的目标集合

   `all` / `clean` / `run` / `debug` / `rebuild` / `install` / `dist` / `info` / `help`
   一个正经项目该有的目标都有了。

6. Debug / Release 模式切换

   ```text
   make MODE=debug    调试版
   make MODE=release  发布版
   ```

7. 静态库和动态库的概念

   什么是 `.a` 什么是 `.so`，怎么生成怎么用。

## 二、项目目录结构

```text
makefile_04_project/
├── Makefile              ← 就是它
├── tutorial.md          ← 你正在看的这个
├── include/              ← 头文件（对外暴露的接口）
│   ├── calculator.h
│   ├── logger.h
│   ├── string_helper.h
│   └── application.h
├── src/                  ← 源文件（实现细节）
│   ├── main.cpp          ← 程序入口
│   ├── app/
│   │   └── application.cpp
│   ├── math/
│   │   └── calculator.cpp
│   └── utils/
│       ├── logger.cpp
│       └── string_helper.cpp
├── obj/                  ← 编译中间文件（自动生成）
│   ├── main.o
│   ├── main.d            ← 自动生成的依赖文件
│   ├── app/
│   │   ├── application.o
│   │   └── application.d
│   └── ...
└── bin/
    └── myapp             ← 最终可执行文件（Windows 下是 myapp.exe）
```

### 为什么要这样组织？

- 源码和生成物分离：清理的时候直接删 `obj/` 和 `bin/` 就行
- 头文件和源文件分离：`include/` 是对外的接口，`src/` 是内部实现
- 分模块：按功能分子目录（`app`, `math`, `utils`），大项目也不乱

## 三、怎么用这个示例？

进入 `makefile_04_project` 目录，然后试试：

1. 看看有哪些可用目标

   ```bash
   $ make help
   ```

2. 看看构建信息（变量都啥值）

   ```bash
   $ make info
   ```

   —— 这招对理解 `Makefile` 特别有帮助，建议多看看

3. 编译（Debug 模式，默认）

   ```bash
   $ make
   ```

   看看 `obj/` 目录和 `bin/` 目录是不是都生成了？
   看看 `obj/` 下是不是有 `.o` 和 `.d` 文件？

4. 运行

   ```bash
   $ make run
   ```

   实际运行的是 `bin/myapp`（Windows 上是 `bin/myapp.exe`）。

5. 试试增量编译

   ```bash
   $ make          # 先编一次
   # 改一下 include/logger.h（比如加个空行）
   $ make
   ```

   看看哪些 `.o` 被重新编译了？
   —— 自动依赖生成在起作用！只有包含了 `logger.h` 的才重编

   再试试改 `include/calculator.h`，看看哪些重编？

6. Release 模式编译

   ```bash
   $ make clean
   $ make MODE=release
   $ make MODE=release info
   ```

   看看 `CXXFLAGS` 有啥不一样？

7. 其他目标

   ```bash
   $ make rebuild    # 清理后重编
   $ make debug      # gdb 调试（需要装 gdb）
   $ make dist       # 打包成 tar.gz
   ```

## 四、老师傅的经验之谈

### 经验 1：自动依赖是必须的

手动写头文件依赖？那是石器时代的做法。用 `-MMD -MP`，编译的时候顺手就生成了，准确又省事。不会自动依赖，别说你会写 `Makefile`。

### 经验 2：目录用顺序依赖 |

普通依赖的话，目录的修改时间一变（比如往里加文件），所有目标都得重编，那不乱套了？目录只要存在就行，用顺序依赖 `|` 才是正确姿势。

### 经验 3：make help 和 make info 很有用

自己写的 `Makefile`，过俩月你也记不清有哪些目标。整个 `help` 目标，别人拿到你的项目也知道怎么用。`info` 目标调试 `Makefile` 特好使，变量展开对不对一看便知。

### 经验 4：$(shell find) 的跨平台问题

Windows CMD 里的 `find` 是文本搜索工具，不是 Unix 的 `find`。如果你在 Windows 上用 MinGW/MSYS2 或者 WSL，就有 Unix `find`。纯 Windows + nmake 那套咱不推荐，直接上 WSL 或者 MinGW 吧。

### 经验 5：不要过度设计

`Makefile` 够用就行，别写得太复杂。太复杂的 `Makefile`，除了你没人敢改，过俩月你自己也看不懂了。保持简洁，保持清晰。

### 坑 1：VPATH 和 vpath（不建议新手用）

有些教程会讲 `VPATH`，用来搜索源文件。那玩意儿坑多，容易出莫名其妙的问题。咱这种"明确指定路径 + 模式规则"的写法更靠谱。

### 坑 2：.d 文件第一次编译没有

第一次编译的时候还没有 `.d` 文件，没关系，`-include` 会忽略不存在的文件。反正第一次所有 `.o` 都没有，都会被编译，编译完 `.d` 文件就有了。

### 坑 3：删除头文件后 make 报错

如果你删了一个头文件，`make` 可能会报：

```text
"No rule to make target 'xxx.h', needed by 'xxx.d'."
```

这就是 `-MP` 选项要解决的问题。`-MP` 会给每个头文件生成一个空规则，删了头文件也不报错，直接重新编译就行。所以 `-MMD` 和 `-MP` 是黄金搭档，缺一不可。

## 五、进阶方向（学有余力可以研究）

1. CMake / Meson / Bazel 等构建系统

   `Makefile` 是基础，但大项目现在更多用 CMake 这些高级工具。它们能自动生成 `Makefile`（或 Ninja 文件），跨平台能力更强。但 `Makefile` 的原理是根，根儿扎牢了，学啥都快。

2. 单元测试集成

   加个 `test` 目标，自动跑单元测试。

3. 代码覆盖率、静态检查

   加个 `coverage` 目标、`lint` 目标。

4. 多平台适配

   用 `ifeq ($(shell uname), Darwin)` 判断 Mac/Linux/Windows。

5. 并行编译

   `make -j4` 用 4 个并行任务（进程）编译，大项目快很多。咱这个 `Makefile` 天然支持并行，直接用就行。

## 六、毕业寄语

徒弟啊，`Makefile` 的五个示例学完了，从最基础的一条规则，到现在这个完整的项目级 `Makefile`，不容易。

但你要记住：

`Makefile` 只是工具，是为项目服务的。不要为了炫技把 `Makefile` 写得花里胡哨。能解决问题、清晰易懂、方便维护，才是好的 `Makefile`。

编程这行当，工具层出不穷，但底层原理是相通的。把基础打牢，比啥都强。

师傅领进门，修行在个人。以后的路，还得你自己走。加油！

—— 你的老师傅
