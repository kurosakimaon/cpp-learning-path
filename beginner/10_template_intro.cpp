// ============================================================================
// 10_template_intro.cpp - 模板入门与STL初步
// 编译命令：g++ -std=c++17 10_template_intro.cpp -o 10_template_intro
// ============================================================================

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

// ============================================================================
// 一、为什么需要模板（泛型编程思想）
// ============================================================================
// 徒弟啊，你想想，我要写一个"求两个数中较大值"的函数。
// int 版本写一个，double 版本写一个，string 版本再写一个...
// 逻辑一模一样，就是类型不一样，你说烦不烦？
//
// 模板就是解决这个问题的——写一份代码，能适配各种类型。
// 就像做月饼，模板（模具）只有一个，但你可以放不同的馅料。
//
// 这就叫"泛型编程"：编写与类型无关的通用代码。
//
// C++ 的模板分两种：
//   1. 函数模板
//   2. 类模板
// ============================================================================

// ============================================================================
// 二、函数模板
// ============================================================================
// 语法：template <typename T>
//       返回类型 函数名(参数列表) { ... }
//
// typename 也可以写成 class，两者是一样的（历史遗留问题）。
// 现代C++推荐用 typename，语义更清晰。
// ============================================================================

// --- 求两个数的较大值 ---
template <typename T>
T myMax(T a, T b) {
    return (a > b) ? a : b;
}

// --- 交换两个数 ---
template <typename T>
void mySwap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

// --- 多个模板参数的例子 ---
template <typename T1, typename T2>
void printTwo(T1 a, T2 b) {
    cout << "第一个：" << a << "，第二个：" << b << endl;
}

// ============================================================================
// 三、类模板
// ============================================================================
// 类模板和函数模板类似，就是整个类是模板。
// 最典型的就是 vector<T>，你可以有 vector<int>、vector<string>...
// ============================================================================

// --- 自己写一个简单的数组模板类 ---
template <typename T>
class MyArray {
private:
    T*      data;       // 数据指针
    int     size;       // 当前元素个数
    int     capacity;   // 容量

public:
    // 构造函数
    MyArray(int cap = 10) : size(0), capacity(cap) {
        data = new T[capacity];
    }

    // 析构函数
    ~MyArray() {
        delete[] data;
    }

    // 这个类拥有裸指针，默认拷贝是"浅拷贝"：
    // 两个对象会指向同一块内存，析构时 delete[] 两次 → 未定义行为，
    // 常见表现是崩溃或堆损坏。
    // 正确做法是写深拷贝（重新 new[] 再复制），这里先禁用拷贝，
    // 避免示例本身教出双重释放的 bug。
    MyArray(const MyArray&) = delete;
    MyArray& operator=(const MyArray&) = delete;

    // 添加元素
    void push_back(const T& value) {
        if (size >= capacity) {
            // 扩容（简单起见，这里就不写扩容逻辑了，意思意思）
            cout << "数组满了！" << endl;
            return;
        }
        data[size++] = value;
    }

    // 获取元素
    T& operator[](int index) {
        return data[index];
    }

    const T& operator[](int index) const {
        return data[index];
    }

    // 获取大小
    int getSize() const { return size; }

    // 获取容量
    int getCapacity() const { return capacity; }

    // 打印所有元素
    void printAll() const {
        for (int i = 0; i < size; i++) {
            cout << data[i] << " ";
        }
        cout << endl;
    }
};

// ============================================================================
// 四、模板的特化
// ============================================================================
// 模板是通用的，但有时候某些特殊类型需要特殊处理。
// 这时候就用"模板特化"——对特定的类型写一个专门的版本。
//
// 分两种：
//   1. 全特化：所有模板参数都指定具体类型
//   2. 偏特化：只指定部分模板参数（类模板才有，函数模板没有）
// ============================================================================

// --- 普通版本的比较函数 ---
template <typename T>
bool isEqual(T a, T b) {
    cout << "【通用版本】";
    return a == b;
}

// --- 特化版本：针对 const char* 类型 ---
// 因为 const char* 直接用 == 比较的是地址，不是内容
template <>
bool isEqual<const char*>(const char* a, const char* b) {
    cout << "【const char* 特化版本】";
    return strcmp(a, b) == 0;
}

// --- 类模板特化的例子 ---
template <typename T>
class Printer {
public:
    void print(const T& value) {
        cout << "【通用Printer】值是：" << value << endl;
    }
};

// 特化：针对 string 类型，打印时加个引号
template <>
class Printer<string> {
public:
    void print(const string& value) {
        cout << "【string特化Printer】值是：\"" << value << "\"" << endl;
    }
};

// ============================================================================
// 五、STL 是什么
// ============================================================================
// 徒弟啊，STL 就是 C++ 给你准备好的"兵器库"。
// STL = Standard Template Library（标准模板库）
//
// STL 有三大件：
//   1. 容器（Container）：装东西的，比如 vector、list、map、set...
//   2. 算法（Algorithm）：操作数据的，比如 sort、find、copy...
//   3. 迭代器（Iterator）：连接容器和算法的桥梁，像指针一样遍历容器
//
// 为什么牛逼？—— 容器和算法是分开的，通过迭代器连接。
// 也就是说，一个 sort 算法，可以对 vector 排序，也可以对 deque 排序，
// 只要这个容器提供了相应的迭代器就行。
//
// 今天咱们先学最常用的 vector 和几个基础算法。
// ============================================================================

// ============================================================================
// 六、vector 的基本用法（最常用的容器）
// ============================================================================
// vector 就是"动态数组"，可以自动扩容，比原生数组好用多了。
// 记住几个常用操作就行：push_back、size、[]、at、begin、end...
// ============================================================================

void demo_vector() {
    cout << "=== vector 基本用法 ===" << endl;

    // 创建一个空的 int 向量
    vector<int> v;

    // 添加元素
    v.push_back(10);
    v.push_back(30);
    v.push_back(20);
    v.push_back(50);
    v.push_back(40);

    cout << "添加5个元素后：" << endl;
    cout << "  大小：" << v.size() << endl;
    cout << "  容量：" << v.capacity() << endl;

    // 用 [] 访问（和数组一样）
    cout << "  第3个元素：" << v[2] << endl;

    // 用 at() 访问（会做边界检查，越界抛异常）
    cout << "  第2个元素（at方式）：" << v.at(1) << endl;

    // 遍历方式1：下标遍历
    cout << "  下标遍历：";
    for (size_t i = 0; i < v.size(); i++) {
        cout << v[i] << " ";
    }
    cout << endl;

    // 遍历方式2：范围for（C++11以后，最推荐）
    cout << "  范围for遍历：";
    for (int num : v) {
        cout << num << " ";
    }
    cout << endl;

    // 遍历方式3：迭代器遍历（后面会讲，先感受一下）
    cout << "  迭代器遍历：";
    for (vector<int>::iterator it = v.begin(); it != v.end(); ++it) {
        cout << *it << " ";
    }
    cout << endl;

    // 其他常用操作
    v.pop_back();  // 删除最后一个元素
    cout << "  pop_back 后大小：" << v.size() << endl;

    v.insert(v.begin() + 1, 99);  // 在第2个位置插入99
    cout << "  insert 后：";
    for (int num : v) cout << num << " ";
    cout << endl;

    v.erase(v.begin() + 2);  // 删除第3个元素
    cout << "  erase 后：";
    for (int num : v) cout << num << " ";
    cout << endl;

    v.clear();  // 清空
    cout << "  clear 后大小：" << v.size() << "，是否为空：" << (v.empty() ? "是" : "否") << endl;

    cout << endl;
}

// ============================================================================
// 七、算法入门（sort, find, for_each）
// 八、迭代器的概念
// ============================================================================
// 徒弟，迭代器你就把它当成"智能指针"就行。
//   *it     取元素
//   ++it    指向下一个
//   it++    也是指向下一个（前置效率更高）
//   it1 == it2  比较是否指向同一个位置
//
// 迭代器有五种类型，你先不用管，知道怎么用就行。
//
// STL 算法都在 <algorithm> 头文件里，都是函数模板。
// 算法的特点：前两个参数一般都是迭代器，表示操作的范围 [first, last)
// 注意是左闭右开区间！—— 包含 first，不包含 last。
// ============================================================================

// 自定义一个打印函数，给 for_each 用
void printInt(int n) {
    cout << n << " ";
}

// 自定义一个判断函数，给 find_if 用（找偶数）
bool isEven(int n) {
    return n % 2 == 0;
}

void demo_algorithm() {
    cout << "=== STL 算法入门 ===" << endl;

    vector<int> v = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    cout << "原始数组：";
    for (int n : v) cout << n << " ";
    cout << endl;

    // --- sort 排序 ---
    cout << endl;
    cout << "【sort 排序】" << endl;

    sort(v.begin(), v.end());  // 默认从小到大
    cout << "  升序排序后：";
    for (int n : v) cout << n << " ";
    cout << endl;

    sort(v.begin(), v.end(), greater<int>());  // 从大到小
    cout << "  降序排序后：";
    for (int n : v) cout << n << " ";
    cout << endl;

    // 也可以只排一部分，比如排前5个
    sort(v.begin(), v.begin() + 5);
    cout << "  只排前5个：";
    for (int n : v) cout << n << " ";
    cout << endl;

    // --- find 查找 ---
    cout << endl;
    cout << "【find 查找】" << endl;

    // 先恢复成有序的
    sort(v.begin(), v.end());

    auto it = find(v.begin(), v.end(), 7);  // 找值为7的元素
    if (it != v.end()) {
        cout << "  找到了 7，下标是 " << (it - v.begin())
             << "（从 0 数），即第 " << (it - v.begin() + 1) << " 个" << endl;
    } else {
        cout << "  没找到 7" << endl;
    }

    // find_if：按条件找
    auto it2 = find_if(v.begin(), v.end(), isEven);
    if (it2 != v.end()) {
        cout << "  第一个偶数是：" << *it2 << endl;
    }

    // --- for_each 遍历 ---
    cout << endl;
    cout << "【for_each 遍历】" << endl;
    cout << "  用 for_each 打印：";
    for_each(v.begin(), v.end(), printInt);
    cout << endl;

    // 用 lambda 表达式（C++11的好东西，后面再细讲，先感受一下）
    cout << "  用 lambda 打印每个数x2：";
    for_each(v.begin(), v.end(), [](int n) {
        cout << n * 2 << " ";
    });
    cout << endl;

    // --- 其他常用算法 ---
    cout << endl;
    cout << "【其他常用算法】" << endl;

    int sum = 0;
    for_each(v.begin(), v.end(), [&sum](int n) { sum += n; });
    cout << "  所有元素之和：" << sum << endl;

    int maxVal = *max_element(v.begin(), v.end());
    int minVal = *min_element(v.begin(), v.end());
    cout << "  最大值：" << maxVal << "，最小值：" << minVal << endl;

    // reverse 反转
    reverse(v.begin(), v.end());
    cout << "  反转后：";
    for (int n : v) cout << n << " ";
    cout << endl;

    // count 计数
    int countOf5 = count(v.begin(), v.end(), 5);
    cout << "  5 出现了 " << countOf5 << " 次" << endl;

    int evenCount = count_if(v.begin(), v.end(), isEven);
    cout << "  偶数有 " << evenCount << " 个" << endl;

    cout << endl;
}

// ============================================================================
// 九、迭代器的深入一点
// ============================================================================
// 迭代器有五大类（了解就行，不用死记）：
//   1. 输入迭代器：只能读，往前走
//   2. 输出迭代器：只能写，往前走
//   3. 前向迭代器：读写，往前走
//   4. 双向迭代器：读写，前后走（比如 list）
//   5. 随机访问迭代器：读写，随便跳（比如 vector、deque）
//
// 不同的算法要求不同的迭代器类型。
// 比如 sort 要求随机访问迭代器，所以 list 不能直接用 sort，
// list 自己提供了成员函数 sort()。
// ============================================================================

void demo_iterator_types() {
    cout << "=== 迭代器小知识 ===" << endl;
    cout << "vector 的迭代器是随机访问迭代器，可以这样跳：" << endl;

    vector<int> v = {10, 20, 30, 40, 50};
    auto it = v.begin();
    cout << "  begin(): " << *it << endl;

    it += 3;  // 直接跳3步（随机访问才支持）
    cout << "  it += 3: " << *it << endl;

    it -= 2;
    cout << "  it -= 2: " << *it << endl;

    cout << "  v.end() - v.begin() = " << (v.end() - v.begin()) << "（直接算距离）" << endl;

    // 反向迭代器
    cout << "  反向遍历：";
    for (auto rit = v.rbegin(); rit != v.rend(); ++rit) {
        cout << *rit << " ";
    }
    cout << endl;

    cout << endl;
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    cout << "==================== 10 - 模板入门与STL初步 ====================" << endl;
    cout << endl;

    // ---- 1. 函数模板演示 ----
    cout << "--- 1. 函数模板 ---" << endl;
    cout << "myMax(3, 7) = " << myMax(3, 7) << endl;
    cout << "myMax(3.14, 2.71) = " << myMax(3.14, 2.71) << endl;
    cout << "myMax(string(\"apple\"), string(\"banana\")) = "
         << myMax(string("apple"), string("banana")) << endl;

    int x = 10, y = 20;
    mySwap(x, y);
    cout << "mySwap 后：x = " << x << "，y = " << y << endl;

    printTwo(100, "hello");
    cout << endl;

    // ---- 2. 类模板演示 ----
    cout << "--- 2. 类模板（自定义MyArray） ---" << endl;
    MyArray<int> intArr(20);
    intArr.push_back(1);
    intArr.push_back(2);
    intArr.push_back(3);
    cout << "int 数组："; intArr.printAll();

    MyArray<string> strArr(10);
    strArr.push_back("hello");
    strArr.push_back("world");
    cout << "string 数组："; strArr.printAll();
    cout << endl;

    // ---- 3. 模板特化演示 ----
    cout << "--- 3. 模板特化 ---" << endl;
    cout << "比较两个 int：" << isEqual(5, 5) << endl;
    cout << "比较两个 const char*："
         << isEqual<const char*>("hello", "hello") << endl;
    // 注意：如果没有特化，比较 const char* 会比较地址而不是内容！

    Printer<int> p1;
    p1.print(123);

    Printer<string> p2;
    p2.print("你好");  // 用的是特化版本，会加引号
    cout << endl;

    // ---- 4. vector 演示 ----
    demo_vector();

    // ---- 5. STL 算法演示 ----
    demo_algorithm();

    // ---- 6. 迭代器演示 ----
    demo_iterator_types();

    cout << "=================== 程序结束 ===================" << endl;

    return 0;
}

// ============================================================================
// 老师傅的话
// ============================================================================
/*
 * 徒弟啊，今天讲的模板和STL，是C++的大杀器，你一定要熟练掌握。
 *
 * 【STL是C++的兵器库，要熟练掌握】
 *
 * 我年轻的时候，啥都自己写：自己写动态数组，自己写排序，自己写查找...
 * 写得累不说，还容易出bug。
 *
 * 后来用了STL，才发现：轮子人家早就造好了，而且比你造的好一百倍。
 *   - 要动态数组？用 vector
 *   - 要链表？用 list
 *   - 要键值对？用 map
 *   - 要排序？直接 sort
 *   - 要查找？直接 find
 *
 * 这些都是标准委员会那帮老家伙们反复打磨过的，
 * 正确性、效率、边界处理，都比你自己写的强。
 *
 * 【学习建议】
 *
 * 1. 先把 vector 用熟，这是最常用的容器，没有之一。
 * 2. 算法不用全记，常用的十几个记住就行，其他的用到再查。
 * 3. 迭代器是连接容器和算法的桥梁，理解了它就理解了STL的设计思想。
 * 4. 模板这东西，先会用，再深入。用多了自然就理解了。
 *
 * 【几个注意点】
 *
 * 1. vector 的 [] 不会做边界检查，越界是未定义行为
 *    （可能崩溃，也可能看起来没事，和数组一样）。
 *    怕出错就用 at()，会抛 std::out_of_range 异常。
 *
 * 2. 迭代器失效问题：对 vector 做插入、删除、扩容操作后，
 *    之前的迭代器可能就失效了，别再用了。
 *
 * 3. 算法的区间都是左闭右开 [first, last)，
 *    也就是说 first 指向的元素会处理，last 指向的不会处理。
 *    这个设计很巧妙，习惯了就觉得好用。
 *
 * 4. 模板是编译时实例化的，所以模板代码一般都写在头文件里。
 *    （这也是为什么你看不到 vector 的实现文件）
 *
 * 【口诀】
 *   模板编程真神奇，
 *   一份代码多类型，
 *   函数模板和类模板，
 *   特化处理特殊情，
 *   STL是兵器库，
 *   容器算法迭代器，
 *   vector最常用，
 *   sort find for_each，
 *   左闭右开要记清，
 *   自己别去造轮子。
 *
 * 【小练习】
 *   1. 用 vector 存 10 个学生的成绩，排序后输出。
 *   2. 写一个函数模板 myMin，求两个数的较小值。
 *   3. 写一个类模板 Stack（栈），实现 push、pop、top、empty 功能。
 *      （内部用 vector 或者动态数组都行）
 *   4. 用 for_each 配合 lambda，计算 vector 中所有奇数的和。
 *   5. 试试 sort 自定义排序规则：对一个 vector<string> 按字符串长度排序。
 *      （提示：写一个比较函数，或者用 lambda）
 *
 *  STL 这东西，用得越多越顺手。以后你写代码，
 *  第一反应应该是"STL里有没有现成的？"，而不是"我自己写一个"。
 *  记住：别重复造轮子！
 */
