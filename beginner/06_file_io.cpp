// ============================================================
// 06_file_io.cpp - 文件I/O与流
// 编译命令：g++ -std=c++17 06_file_io.cpp -o 06_file_io
// ============================================================

#include <iostream>
#include <fstream>   // 文件流头文件（ifstream/ofstream/fstream）
#include <sstream>   // 字符串流头文件（stringstream）
#include <string>
#include <vector>
using namespace std;

int main()
{
    // ============================================================
    // 一、C++ 流的概念
    // ============================================================
    // 老师傅比喻：流（stream）就像水管里的水流，数据像水一样"流"过去。
    // 你要输出数据，就往流里"倒"；你要输入数据，就从流里"接"。
    // cout 是输出流（往屏幕流），cin 是输入流（从键盘流）。
    // 文件流就是和文件之间建立的"水管"，数据可以流向文件，也可以从文件流出来。
    cout << "===== 1. C++ 流的概念 =====" << endl;
    cout << "C++ 的 I/O 库是基于流（stream）设计的：" << endl;
    cout << "  - istream：输入流（cin 是它的代表）" << endl;
    cout << "  - ostream：输出流（cout 是它的代表）" << endl;
    cout << "  - ifstream：输入文件流（从文件读）" << endl;
    cout << "  - ofstream：输出文件流（往文件写）" << endl;
    cout << "  - fstream：文件流（可读可写）" << endl;
    cout << "  - stringstream：字符串流（和字符串打交道）" << endl;
    cout << endl;

    // ============================================================
    // 二、文本文件写入（ofstream）
    // ============================================================
    cout << "===== 2. 文本文件写入 =====" << endl;

    // 创建输出文件流对象，打开文件
    // 如果文件不存在，会自动创建；如果已存在，默认会覆盖
    ofstream outFile("demo_text.txt");

    // 老师傅提醒：打开文件后一定要检查是否成功！
    if (!outFile.is_open()) {
        cout << "文件打开失败！" << endl;
        return 1;  // 非正常退出
    }

    // 写入文件，用法和 cout 几乎一样
    outFile << "Hello, 文件I/O！" << endl;
    outFile << "这是第二行" << endl;
    outFile << "整数：" << 123 << "，浮点数：" << 3.14 << endl;
    outFile << "姓名：张三，年龄：18，成绩：92.5" << endl;

    // 关闭文件（析构函数会自动关，但手动关是个好习惯）
    outFile.close();
    cout << "文件写入完成，文件名：demo_text.txt" << endl;
    cout << endl;

    // ============================================================
    // 三、文本文件读取（ifstream）
    // ============================================================
    cout << "===== 3. 文本文件读取 =====" << endl;

    ifstream inFile("demo_text.txt");

    if (!inFile) {  // 也可以直接用 ifstream 对象判断，等价于 !is_open()
        cout << "文件打开失败！" << endl;
        return 1;
    }

    // 读取方式一：逐词读取（用 >>，遇到空格/换行就停）
    cout << "--- 逐词读取 ---" << endl;
    string word;
    int count = 0;
    while (inFile >> word) {  // 读到文件末尾时，inFile 会变成"假"
        cout << "第" << (++count) << "个词：" << word << endl;
    }

    // 读完了，要回到文件开头才能重新读
    inFile.clear();  // 清除文件结束标志（eofbit）
    inFile.seekg(0); // 把读指针移到文件开头（后面详细讲）

    // 读取方式二：逐行读取（用 getline，读整行包括空格）
    cout << endl << "--- 逐行读取 ---" << endl;
    string line;
    int lineNum = 0;
    while (getline(inFile, line)) {
        cout << "第" << (++lineNum) << "行：" << line << endl;
    }

    inFile.close();
    cout << endl;

    // ============================================================
    // 四、读写状态检查
    // ============================================================
    cout << "===== 4. 读写状态检查 =====" << endl;

    ifstream testFile("demo_text.txt");

    // 常见的状态位检查
    cout << "good() = " << testFile.good() << "（一切正常）" << endl;
    cout << "eof() = " << testFile.eof() << "（是否到了文件末尾）" << endl;
    cout << "fail() = " << testFile.fail() << "（读写操作是否失败）" << endl;
    cout << "bad() = " << testFile.bad() << "（是否发生严重错误）" << endl;

    // 老师傅说：while (inFile >> x) 这种写法
    // 本质上是在判断流的状态是否"好"
    // 如果读到文件末尾或者出错了，循环就停止

    // 读完整个文件再看状态
    string temp;
    while (testFile >> temp) {
        // 把文件读完
    }
    cout << endl << "读完文件后：" << endl;
    cout << "eof() = " << testFile.eof() << "（已经到末尾了）" << endl;
    cout << "fail() = " << testFile.fail() << "（读到末尾后再读，fail也会变真）" << endl;

    testFile.close();
    cout << endl;

    // ============================================================
    // 五、文件打开模式
    // ============================================================
    cout << "===== 5. 文件打开模式 =====" << endl;
    cout << "打开文件时可以指定模式，多个模式用 | 组合：" << endl;
    cout << "  ios::in      - 只读（ifstream默认）" << endl;
    cout << "  ios::out     - 只写（ofstream默认）" << endl;
    cout << "  ios::app     - 追加模式（在文件末尾写，不覆盖）" << endl;
    cout << "  ios::trunc   - 截断模式（清空原有内容，默认行为）" << endl;
    cout << "  ios::binary  - 二进制模式" << endl;
    cout << "  ios::ate     - 打开后指针移到文件末尾" << endl;
    cout << endl;

    // 演示追加模式
    ofstream appendFile("demo_text.txt", ios::app);
    if (appendFile.is_open()) {
        appendFile << "--- 这是追加的一行 ---" << endl;
        appendFile.close();
        cout << "已在 demo_text.txt 末尾追加一行" << endl;
    }

    // 演示 fstream（可读可写）
    fstream rwFile("demo_rw.txt", ios::in | ios::out | ios::trunc);
    if (rwFile.is_open()) {
        rwFile << "第一行" << endl;
        rwFile << "第二行" << endl;

        // 写完后要把指针移回去才能读
        rwFile.seekg(0);  // 读指针移到开头

        string s;
        while (getline(rwFile, s)) {
            cout << "  从rw文件读出：" << s << endl;
        }
        rwFile.close();
    }
    cout << endl;

    // ============================================================
    // 六、二进制文件读写
    // ============================================================
    cout << "===== 6. 二进制文件读写 =====" << endl;
    cout << "文本文件存的是字符，二进制文件存的是原始字节" << endl;
    cout << "二进制读写用 read() 和 write() 函数" << endl;
    cout << endl;

    // 二进制写入
    ofstream binOut("demo_binary.dat", ios::binary);
    if (binOut.is_open()) {
        // 写一个整数
        int num = 12345;
        binOut.write(reinterpret_cast<char*>(&num), sizeof(num));
        // reinterpret_cast<char*> 把地址转成 char*，这是C++的强制类型转换
        // 老师傅说：reinterpret_cast 是"重新解释"这块内存的类型，比较底层

        // 写一个结构体
        struct Person {
            char name[20];
            int age;
            double height;
        };
        Person p = {"张三", 25, 1.78};
        binOut.write(reinterpret_cast<char*>(&p), sizeof(p));

        // 写一个数组
        double arr[] = {1.1, 2.2, 3.3, 4.4, 5.5};
        binOut.write(reinterpret_cast<char*>(arr), sizeof(arr));

        binOut.close();
        cout << "二进制文件写入完成：demo_binary.dat" << endl;
    }

    // 二进制读取
    ifstream binIn("demo_binary.dat", ios::binary);
    if (binIn.is_open()) {
        int num;
        binIn.read(reinterpret_cast<char*>(&num), sizeof(num));
        cout << "读出的整数：" << num << endl;

        struct Person {
            char name[20];
            int age;
            double height;
        };
        Person p;
        binIn.read(reinterpret_cast<char*>(&p), sizeof(p));
        cout << "读出的Person：name=" << p.name
             << ", age=" << p.age
             << ", height=" << p.height << endl;

        double arr[5];
        binIn.read(reinterpret_cast<char*>(arr), sizeof(arr));
        cout << "读出的数组：";
        for (double v : arr) {
            cout << v << " ";
        }
        cout << endl;

        binIn.close();
    }

    cout << "老师傅提醒：二进制文件不能用记事本打开看（乱码）" << endl;
    cout << "而且跨平台可能有问题（字节序、结构体对齐等）" << endl;
    cout << endl;

    // ============================================================
    // 七、文件定位（seekg / seekp）
    // ============================================================
    cout << "===== 7. 文件定位 =====" << endl;
    cout << "文件里有个"指针"，指向当前读写的位置" << endl;
    cout << "  - seekg / tellg：操作读指针（get）" << endl;
    cout << "  - seekp / tellp：操作写指针（put）" << endl;
    cout << endl;

    // 先准备一个文件
    ofstream posOut("demo_seek.txt");
    posOut << "0123456789ABCDEFGHIJ";
    posOut.close();

    ifstream posIn("demo_seek.txt");
    if (posIn.is_open()) {
        char c;

        // tellg() 获取当前读指针的位置
        cout << "初始位置：" << posIn.tellg() << endl;

        // 从开头往后移5个字节
        posIn.seekg(5, ios::beg);  // ios::beg 表示从开头算
        posIn.get(c);
        cout << "seekg(5, beg) 后，字符是：" << c << "（第6个字符，下标5）" << endl;

        // 从当前位置往后移3个字节
        posIn.seekg(3, ios::cur);  // ios::cur 表示从当前位置算
        posIn.get(c);
        cout << "seekg(3, cur) 后，字符是：" << c << endl;

        // 从末尾往前移2个字节
        posIn.seekg(-2, ios::end);  // ios::end 表示从末尾算
        posIn.get(c);
        cout << "seekg(-2, end) 后，字符是：" << c << "（倒数第二个字符）" << endl;

        posIn.close();
    }
    cout << endl;

    // ============================================================
    // 八、stringstream 的使用
    // ============================================================
    cout << "===== 8. stringstream 的使用 =====" << endl;
    cout << "stringstream 把字符串当"流"来操作，可以从字符串里读，也可以往里写" << endl;
    cout << "特别适合：字符串分割、类型转换、字符串拼接等场景" << endl;
    cout << endl;

    // 8.1 用 stringstream 读（从字符串中提取数据）
    string data = "张三 18 92.5";
    istringstream iss(data);  // 输入字符串流

    string name;
    int age;
    double score;
    iss >> name >> age >> score;  // 就像从cin读一样，自动按空格分割

    cout << "从字符串解析：" << endl;
    cout << "  姓名：" << name << endl;
    cout << "  年龄：" << age << endl;
    cout << "  成绩：" << score << endl;

    // 8.2 用 stringstream 写（拼接成字符串）
    ostringstream oss;
    oss << "姓名：" << name
        << "，年龄：" << age
        << "，成绩：" << score;
    string result = oss.str();  // 用 str() 获取拼接好的字符串
    cout << "拼接结果：" << result << endl;

    // 8.3 用 stringstream 做类型转换
    cout << endl << "类型转换：" << endl;
    string numStr = "12345";
    int numVal;
    istringstream(numStr) >> numVal;  // 一行搞定字符串转int
    cout << "字符串 \"" << numStr << "\" 转成整数：" << numVal << endl;
    cout << "加100 = " << numVal + 100 << "（证明真的是数字了）" << endl;

    // 数字转字符串也可以
    ostringstream oss2;
    oss2 << 3.14159;
    string piStr = oss2.str();
    cout << "数字 3.14159 转成字符串：\"" << piStr << "\"" << endl;

    // 8.4 字符串分割（按空格分割最方便）
    cout << endl << "字符串分割：" << endl;
    string sentence = "C++ is a powerful language";
    istringstream iss2(sentence);
    vector<string> words;
    string w;
    while (iss2 >> w) {
        words.push_back(w);
    }
    cout << "分割出 " << words.size() << " 个单词：" << endl;
    for (size_t i = 0; i < words.size(); i++) {
        cout << "  [" << i << "] " << words[i] << endl;
    }

    cout << endl;

    // ============================================================
    // 老师傅的话：RAII思想初体验
    // ============================================================
    cout << "========================================" << endl;
    cout << "  老师傅的话：RAII思想初体验" << endl;
    cout << "========================================" << endl;
    cout << "  你有没有发现？我们写了很多 open，但有时候忘了 close" << endl;
    cout << "  但程序好像也没出什么问题？" << endl;
    cout << endl;
    cout << "  这就是C++的 RAII 机制在起作用！" << endl;
    cout << "  RAII = Resource Acquisition Is Initialization" << endl;
    cout << "  翻译过来就是：资源获取即初始化" << endl;
    cout << endl;
    cout << "  简单说：" << endl;
    cout << "    - 对象创建时获取资源（打开文件）" << endl;
    cout << "    - 对象销毁时释放资源（关闭文件）" << endl;
    cout << "    - 你不用手动关，析构函数会自动帮你关" << endl;
    cout << endl;
    cout << "  那为什么还要手动 close？" << endl;
    cout << "    1. 显式关闭更清晰，是个好习惯" << endl;
    cout << "    2. 有时候你需要立即释放资源" << endl;
    cout << "    3. 但即使你忘了，程序也不会泄漏资源" << endl;
    cout << endl;
    cout << "  RAII 是C++最核心的思想之一" << endl;
    cout << "  后面学智能指针、容器等等，都是基于这个思想" << endl;
    cout << "  现在你只要记住：" << endl;
    cout << "    文件对象出了作用域，文件会自动关闭" << endl;
    cout << "    这比C语言的fopen/fclose安全多了" << endl;
    cout << "========================================" << endl;
    cout << endl;

    // ============================================================
    // 小练习
    // ============================================================
    cout << "========================================" << endl;
    cout << "  小练习（动手写写看）" << endl;
    cout << "========================================" << endl;
    cout << "  1. 写一个程序，把1到100写入到一个文本文件中" << endl;
    cout << "     每行一个数，然后再读出来求和" << endl;
    cout << "  2. 统计一个文本文件中有多少行、多少个单词、多少个字符" << endl;
    cout << "     （类似Linux的wc命令）" << endl;
    cout << "  3. 用二进制方式把一个int数组写入文件，再读出来验证" << endl;
    cout << "  4. 写一个函数，把字符串按逗号分割成vector<string>" << endl;
    cout << "     （提示：用 getline + ',' 作为分隔符）" << endl;
    cout << "  5. 实现一个简单的学生成绩管理系统：" << endl;
    cout << "     - 学生信息（姓名、学号、成绩）存在文件里" << endl;
    cout << "     - 程序启动时从文件加载" << endl;
    cout << "     - 可以添加、查询、修改学生信息" << endl;
    cout << "     - 退出时保存回文件" << endl;
    cout << "  6. 用 stringstream 实现一个简单的计算器" << endl;
    cout << "     输入 \"12 + 34\" 这种格式的字符串，输出结果" << endl;
    cout << "========================================" << endl;

    return 0;
}
