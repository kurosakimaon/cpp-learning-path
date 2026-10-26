# Makefile 第5讲：静态库与动态库

> 老师傅手把手教你做库、用库

## 一、这一讲学什么？

徒弟啊，库（library）是软件开发里非常重要的概念。简单说，就是把一堆编译好的代码打包起来，给别人用。

这一讲咱要搞清楚：

1. 什么是静态库？什么是动态库？区别在哪？
2. 怎么制作静态库（`.a` 文件）？
3. 怎么制作动态库（`.so` 文件）？
4. 怎么链接库？`-L` 和 `-l` 是啥意思？
5. 动态库运行时找不到怎么办？`LD_LIBRARY_PATH` 是啥？
6. 静态链接和动态链接生成的可执行文件有啥不一样？

## 二、静态库 vs 动态库，大白话讲清楚

想象你在做一道菜，需要用到"秘制酱料"。

### 静态库 = 把酱料直接倒进菜里

- 菜做好了，酱料就在菜里了
- 你端菜给别人，不用额外带酱料
- 但菜会比较"多"（体积大）
- 想换酱料？那得重做这道菜

### 动态库 = 酱料单独放一个小碟子

- 菜里没有酱料，旁边放个小碟子
- 你端菜给别人，得连碟子一起端
- 菜本身比较"少"（体积小）
- 想换酱料？换碟子就行，不用重做菜
- 好几道菜可以共享同一碟酱料（省空间）

### 对比总结

| 特性 | 静态库 (`.a`) | 动态库 (`.so`) |
| --- | --- | --- |
| 代码何时进入程序 | 链接时复制进可执行文件 | 链接时只登记依赖，加载时由动态链接器映射 |
| 可执行文件大小 | 大（含库代码） | 小（只有引用） |
| 运行时依赖 | 无（库代码已在程序里；系统库另算） | 需要 `.so` 文件 |
| 多程序共享 | 不能（各自一份） | 可以（共享库代码页） |
| 库升级 | 重新编译程序 | 换 `.so` 就行（需 ABI 兼容） |
| 启动速度 | 快（不用加载库） | 稍慢（要加载并重定位） |
| 部署简单程度 | 简单（一个文件） | 麻烦（要带库） |
| 适用场景 | 小库、不想折腾 | 大库、多程序共享 |

### 平台后缀说明

上表以 Linux 为例。Windows 下：

- MSVC：静态库 `.lib`，动态库 `.dll` + 导入库 `.lib`
- MinGW：静态库 `.a`，动态库 `.dll` + 导入库 `.dll.a`

本讲后面的命令以 Linux 为主，示例 `Makefile` 已分别处理 Linux/macOS/Windows 的库后缀和链接方式。

## 三、制作静态库详细步骤

就三步：

1. 把源文件编译成 `.o` 目标文件

   ```bash
   g++ -c math_utils.cpp -o math_utils.o
   ```

2. 用 `ar` 命令把 `.o` 打包成 `.a`

   ```bash
   ar rcs libmathutils.a math_utils.o
   ```

3. 完成！`libmathutils.a` 就是静态库了

### ar 命令参数

```bash
ar rcs libxxx.a file1.o file2.o ...
   r - replace，库已存在就替换里面的成员
   c - create，库不存在就创建（不打印警告）
   s - 建立索引，加快链接速度
       （不加 s 也可以，之后单独运行 ranlib libxxx.a 也行）
```

### 查看静态库内容

```bash
ar t libmathutils.a        ← 看里面有哪些 .o 文件
ar -x libmathutils.a       ← 把 .o 解压出来
```

## 四、制作动态库详细步骤

也是三步，但多了两个关键选项：

1. 编译 `.o` 时加 `-fPIC`

   ```bash
   g++ -c -fPIC math_utils.cpp -o math_utils.o
   ```

2. 链接时加 `-shared`

   ```bash
   g++ -shared -o libmathutils.so math_utils.o
   ```

3. 完成！`libmathutils.so` 就是动态库了

### -fPIC 是什么？

Position Independent Code，位置无关代码。

为啥动态库需要这个？
因为动态库在运行时才加载，加载到内存的哪个位置是不确定的。如果代码里用了绝对地址，加载到不同位置就会出错。

`-fPIC` 生成的代码全部用相对地址，加载到任何位置都能正常运行。

静态库为啥不需要？
如果静态库只链接进普通（非 PIE）的可执行文件，代码地址在链接时就固定了，确实可以不用 `-fPIC`。

但要注意：如果静态库会被链接进动态库，或者可执行文件启用了 PIE（很多 Linux 发行版现在默认开 PIE），那静态库里的 `.o` 也必须用 `-fPIC` 编译，否则链接时可能报：

```text
relocation R_X86_64_32S ... can not be used when making a PIE object
```

所以做通用静态库时，加 `-fPIC` 更省心（代价是极小的性能损失）。

### 查看动态库信息

```bash
file libmathutils.so      ← 看文件类型
ldd libmathutils.so       ← 看它依赖哪些其他库
nm -D libmathutils.so     ← 看导出了哪些符号（函数/变量）
```

## 五、链接库的方法

链接库用 `-l`（小写的 L）参数：

```bash
g++ main.o -lmathutils -o app
```

编译器会自动去找 `libmathutils.a` 或 `libmathutils.so`。
（这就是为啥库名要以 `lib` 开头的原因！）

### 库搜索路径 -L

如果库不在系统默认路径（`/lib`, `/usr/lib`），就要用 `-L` 告诉编译器去哪找：

```text
g++ main.o -L. -lmathutils -o app
              ↑
              当前目录
```

`-L` 和 `-l` 是一对好搭档：

- `-L` 指定搜索目录（去哪里找）
- `-l` 指定库名字（找哪个）

### 链接顺序很重要！

如果库 A 依赖库 B，那 A 要写在 B 前面：

```text
g++ main.o -lA -lB -o app   ✓ 正确
g++ main.o -lB -lA -o app   ✗ 可能报错
```

为啥？因为链接器是从左往右找，遇到未定义的符号才去后面的库里找。写反了可能找不到。

### 静态链接 vs 动态链接

默认情况下，同一个目录里既有 `.a` 又有 `.so`，编译器会优先选动态库。

如果想强制静态链接：

```bash
g++ main.o -static -lmathutils -o app
```

（或者直接写 `.a` 文件路径也行）

## 六、动态库的运行时问题（重点！）

徒弟，这是新手最容易踩的坑，师傅给你讲清楚。

### 现象

编译好好的，一运行就报错：

```text
error while loading shared libraries: libxxx.so:
cannot open shared object file: No such file or directory
```

### 原因

`-L` 是给编译器（编译时）用的，告诉编译器去哪找库。但程序运行的时候，操作系统的动态链接器（`ld.so`）不知道你的库在哪。编译器没把库的路径告诉可执行文件。

### 三种解决方法

#### 方法一：设置 `LD_LIBRARY_PATH` 环境变量（临时用，开发调试）

```bash
export LD_LIBRARY_PATH=/path/to/libs:$LD_LIBRARY_PATH
./app
```

特点：简单，不用改程序。但只对当前终端有效。
适合开发调试的时候用。
注意：这是 Linux 的变量；macOS 用 `DYLD_LIBRARY_PATH`，Windows 则是把库所在目录加进 `PATH`。

#### 方法二：安装到系统目录（正式发布）

```bash
sudo cp libxxx.so /usr/local/lib/
sudo ldconfig
./app
```

特点：一劳永逸，所有程序都能用。
但需要 root 权限，适合安装第三方库。
`ldconfig` 是 Linux 的工具；macOS 一般用 `/usr/local/lib` 的默认搜索路径，Windows 则把 DLL 放到可执行文件同目录或系统目录。

#### 方法三：编译时加 `-rpath`（推荐，开发/测试用）

```bash
g++ main.o -L. -lxxx -Wl,-rpath,. -o app
```

什么意思？

```text
-Wl,xxx  把 xxx 传给链接器 ld
-rpath   运行时库搜索路径
```

就是把库的路径"写进"可执行文件里，运行的时候直接去那找。

特点：不用设环境变量，不用装系统目录。但路径是写死的，移动了库的位置就不行了。

再提醒一句：`-Wl,-rpath,.` 里的 `"."` 是相对于"运行程序时的工作目录"，不是相对于可执行文件所在目录。换到别的目录运行就找不到库了。发布时更稳妥的写法是用 `$ORIGIN`（Linux）：

```bash
-Wl,-rpath,'$ORIGIN/lib'
```

它的意思是"可执行文件所在目录下的 lib 子目录"。

### 咱们这个例子用的是方法三

Linux 下 shell 命令写的是 `-Wl,-rpath,'$ORIGIN'`；如果这行出现在 `Makefile` 的配方里，`$` 是 make 的变量语法，必须写成 `-Wl,-rpath,'$$ORIGIN'`（make 会把 `$$` 还原成一个 `$`）。它的意思是把“可执行文件所在目录”加到运行时搜索路径里。
macOS 对应 `@loader_path`；Windows 不需要 rpath，DLL 和 exe 放在同一目录即可。

> 🎬 想看加载器怎么找库、`$ORIGIN` 怎么展开：打开[静态库与动态库图解](../../visuals/linking.html)。

## 七、动手试试

咱这个示例项目里有两个库：

- `libmathutils`：数学计算库
- `libstringutils`：字符串处理库

还有一个主程序 `main.cpp`，会用到这两个库。

试试下面这些命令，感受一下：

1. 编译静态库版本

   ```bash
   make static
   ./app_static
   ls -lh app_static    ← 看看多大
   ```

2. 编译动态库版本

   ```bash
   make shared
   ./app_shared
   ls -lh app_shared    ← 跟静态版比比大小
   ```

3. 查看库文件信息

   ```bash
   make info
   ```

4. 看看动态库依赖

   ```bash
   ldd app_shared
   ldd app_static
   ```

5. 试试去掉 rpath 会怎样

   （修改 `Makefile`，把 Linux 分支的 `RPATH_FLAGS` 清空，重新编译运行；Windows 上 DLL 与 exe 同目录，不受影响）

6. 试试用 `LD_LIBRARY_PATH`

   （去掉 rpath 之后，试试：`LD_LIBRARY_PATH=. ./app_shared`）

> **老师傅建议**
>
> 多动手试！光看不练假把式。
> 改改 `Makefile`，故意搞出点错误，看看报错信息是什么样的。
> 踩过坑、填过坑，知识才是你的。

## 八、老师傅的经验之谈

1. **默认用静态还是动态？**

   - 小项目、简单库 → 静态库，简单省事，不用管运行时依赖
   - 大项目、多程序共享 → 动态库，省空间、好升级
   - 第三方库 → 通常同时提供静态库和动态库（Qt 常用动态库；Boost 两种都有，Windows 上静态库很常见）

2. **库的命名**

   一定要 `lib` 开头！`-lxxx` 才能找到。Windows 下是 `.dll`，但也有类似的命名约定。

3. **动态库版本号**

   正式发布的动态库一般带版本号：

   ```text
   libfoo.so.1.2.3    完整版本
   libfoo.so.1        主版本号（兼容的）
   libfoo.so          软链接，指向最新版
   ```

   这是个约定，保证二进制兼容性。新手先不用管这么细，知道有这回事就行。

4. **符号可见性**

   默认情况下，动态库会导出所有函数和全局变量。但其实很多内部函数不需要导出。可以用 `-fvisibility=hidden` 把默认设为隐藏，然后用 `__attribute__((visibility("default")))` 标记需要导出的函数。这样库更小，加载更快，也更安全。（进阶话题，以后再研究）

5. **静态链接的坑**

   如果多个动态库都静态链接了同一个静态库，那内存里会有多份代码，浪费空间。而且如果静态库有全局变量，每个库各有一份，可能出问题。这个要注意。

6. **不要怕库**

   很多新手一提到库就发怵，觉得很复杂。其实本质很简单：库就是一堆编译好的 `.o` 打包起来，链接的时候把需要的拿过来用。理解了这个本质，什么库都不怕。

> 第5讲就到这儿。库的概念很重要，后面不管用什么
> 第三方库（Boost、Qt、OpenCV...），本质都是链接库。
> 把这一讲搞懂了，后面学什么都快。
