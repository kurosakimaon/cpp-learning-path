// ============================================================
//  stl_algorithms.cpp - STL算法与迭代器
// ============================================================
//  徒弟啊，容器存数据，算法操作数据，迭代器是它们之间的桥梁。
//  STL算法库就是一堆高度优化的通用算法，别自己瞎写循环了。
//  今天咱把常用算法一个个过一遍，再把迭代器那点事儿讲透。
//  记住：不要重复造轮子，STL算法都是大师写的，比你写的快多了。
// ============================================================

#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <numeric>
#include <functional>
#include <string>
#include <iterator>
#include <ctime>
#include <cstdlib>

using namespace std;

// ============================================================
// 一、迭代器的分类
// ============================================================
// 徒弟，迭代器是STL的核心概念之一。
// 它就像一个指针，指向容器里的元素，但比指针更通用。
//
// 迭代器分5类，功能依次增强：
//
// 1. 输入迭代器（Input Iterator）
//    - 只能读，只能往前走（++），不能重复读同一个元素
//    - 比如：istream_iterator
//
// 2. 输出迭代器（Output Iterator）
//    - 只能写，只能往前走（++）
//    - 比如：ostream_iterator, back_inserter
//
// 3. 前向迭代器（Forward Iterator）
//    - 可读可写，只能往前走，可以重复读
//    - 比如：forward_list 的迭代器
//
// 4. 双向迭代器（Bidirectional Iterator）
//    - 可读可写，前后都能走（++/--）
//    - 比如：list, map, set 的迭代器
//
// 5. 随机访问迭代器（Random Access Iterator）
//    - 可读可写，随便跳（+n/-n/[]/比较大小）
//    - 比如：vector, deque, array 的迭代器，原生指针
//
// 算法要求什么迭代器，就只能用支持那种迭代器的容器。
// 比如 sort 需要随机访问迭代器，所以 list 不能用 std::sort，
// 只能用它自己的成员函数 sort()。
// ============================================================

void iteratorCategories()
{
    cout << "============================================================" << endl;
    cout << "  一、迭代器的五种类型" << endl;
    cout << "============================================================" << endl;
    cout << endl;
    cout << "  输入迭代器  ←  最" << endl;
    cout << "  输出迭代器  ←  弱" << endl;
    cout << "     │" << endl;
    cout << "  前向迭代器  ←  单向可重复" << endl;
    cout << "     │" << endl;
    cout << "  双向迭代器  ←  可前可后" << endl;
    cout << "     │" << endl;
    cout << "  随机访问    ←  最强，随便跳" << endl;
    cout << endl;
    cout << "  各容器对应的迭代器类型：" << endl;
    cout << "    vector/deque/array  → 随机访问迭代器" << endl;
    cout << "    list/map/set        → 双向迭代器" << endl;
    cout << "    forward_list        → 前向迭代器" << endl;
    cout << "    istream_iterator    → 输入迭代器" << endl;
    cout << "    ostream_iterator    → 输出迭代器" << endl;
    cout << endl;
    cout << "  算法对迭代器的要求：" << endl;
    cout << "    sort/partial_sort   → 随机访问" << endl;
    cout << "    reverse             → 双向" << endl;
    cout << "    find/for_each       → 输入（单向即可）" << endl;
}

// 辅助函数：打印vector
void printVec(const vector<int>& vec, const string& name = "vector")
{
    cout << "  " << name << ": ";
    for (int x : vec) cout << x << " ";
    cout << endl;
}

// ============================================================
// 二、排序类算法
// ============================================================
// sort：快速排序变种（introsort），平均O(n log n)，不稳定
// stable_sort：稳定排序（实现多用归并）；有额外内存 O(n log n)，内存不足时 O(n log² n)
// partial_sort：部分排序，前k个有序，O(n log k)
// nth_element：第n小的元素就位，左边都不大于它，右边都不小于它
//               （重复元素可能分布在两侧，不保证严格大小关系）
// ============================================================

void sortAlgorithms()
{
    cout << "\n=== 2. 排序类算法 ===" << endl;

    vector<int> vec = {5, 2, 9, 1, 5, 6, 3, 7, 4, 8};
    printVec(vec, "原始数据");

    // 2.1 sort - 快速排序（默认升序）
    vector<int> v1 = vec;
    sort(v1.begin(), v1.end());
    printVec(v1, "sort 升序");

    // 降序：用 greater<int>() 谓词
    vector<int> v2 = vec;
    sort(v2.begin(), v2.end(), greater<int>());
    printVec(v2, "sort 降序");

    // 用 lambda 自定义排序规则
    vector<int> v3 = vec;
    sort(v3.begin(), v3.end(), [](int a, int b) {
        // 按个位数排序
        return (a % 10) < (b % 10);
    });
    printVec(v3, "sort 按个位数");

    // 2.2 stable_sort - 稳定排序
    // 什么叫稳定？两个相等的元素，排序后相对位置不变
    cout << "\n  2.2 stable_sort 稳定排序" << endl;
    vector<pair<int, string>> people = {
        {20, "张三"}, {18, "李四"}, {20, "王五"}, {22, "赵六"}, {18, "钱七"}
    };
    cout << "  原始（年龄, 姓名）：";
    for (auto& p : people) cout << p.first << p.second << " ";
    cout << endl;

    stable_sort(people.begin(), people.end(),
        [](const pair<int, string>& a, const pair<int, string>& b) {
            return a.first < b.first;
        });
    cout << "  stable_sort按年龄：";
    for (auto& p : people) cout << p.first << p.second << " ";
    cout << endl;
    cout << "  （同龄人的相对顺序保持不变，这就是稳定）" << endl;

    // 2.3 partial_sort - 部分排序
    cout << "\n  2.3 partial_sort 部分排序" << endl;
    vector<int> v4 = vec;
    // 只让前5个元素有序（最小的5个）
    partial_sort(v4.begin(), v4.begin() + 5, v4.end());
    printVec(v4, "partial_sort 前5个有序");

    // 2.4 nth_element - 找第n小的元素
    cout << "\n  2.4 nth_element 找第n小" << endl;
    vector<int> v5 = vec;
    nth_element(v5.begin(), v5.begin() + 3, v5.end());
    printVec(v5, "nth_element 第4小就位（索引3）");
    cout << "  第4小的元素是: " << v5[3] << endl;
    cout << "  左边的都<=它，右边的都>=它，但两边都不一定有序" << endl;
}

// ============================================================
// 三、查找类算法
// ============================================================
// find：顺序查找，O(n)
// find_if：按条件查找
// binary_search：二分查找，要求有序，O(log n)，只返回bool
// lower_bound：第一个 >= value 的位置
// upper_bound：第一个 > value 的位置
// equal_range：lower_bound + upper_bound，返回一对迭代器
// ============================================================

void findAlgorithms()
{
    cout << "\n=== 3. 查找类算法 ===" << endl;

    vector<int> vec = {5, 2, 9, 1, 5, 6, 3, 7, 4, 8};
    printVec(vec, "原始数据");

    // 3.1 find - 顺序查找
    auto it = find(vec.begin(), vec.end(), 6);
    if (it != vec.end())
    {
        cout << "  find(6)：找到了，索引是 " << (it - vec.begin()) << endl;
    }
    else
    {
        cout << "  find(6)：没找到" << endl;
    }

    // 3.2 find_if - 按条件查找
    it = find_if(vec.begin(), vec.end(), [](int x) {
        return x > 7;  // 找第一个大于7的
    });
    if (it != vec.end())
    {
        cout << "  find_if(>7)：找到了 " << *it
             << "，索引 " << (it - vec.begin()) << endl;
    }

    // 3.3 binary_search - 二分查找（要求有序！）
    cout << "\n  3.3 binary_search 二分查找（需要先排序）" << endl;
    vector<int> sorted = vec;
    sort(sorted.begin(), sorted.end());
    printVec(sorted, "排序后");

    bool found = binary_search(sorted.begin(), sorted.end(), 5);
    cout << "  binary_search(5)：" << (found ? "找到了" : "没找到") << endl;

    // 3.4 lower_bound / upper_bound
    cout << "\n  3.4 lower_bound / upper_bound" << endl;
    auto low = lower_bound(sorted.begin(), sorted.end(), 5);
    auto high = upper_bound(sorted.begin(), sorted.end(), 5);
    // 注意：找不到时返回 end()，必须先判断再解引用！
    if (low != sorted.end()) {
        cout << "  lower_bound(5) 指向第一个 >=5 的元素: " << *low
             << " (索引 " << (low - sorted.begin()) << ")" << endl;
    } else {
        cout << "  lower_bound(5) 没找到 >=5 的元素（返回 end()）" << endl;
    }
    if (high != sorted.end()) {
        cout << "  upper_bound(5) 指向第一个 >5 的元素: " << *high
             << " (索引 " << (high - sorted.begin()) << ")" << endl;
    } else {
        cout << "  upper_bound(5) 没找到 >5 的元素（返回 end()）" << endl;
    }
    cout << "  等于5的元素有 " << (high - low) << " 个" << endl;

    // 3.5 equal_range = lower_bound + upper_bound
    auto range = equal_range(sorted.begin(), sorted.end(), 5);
    cout << "  equal_range(5)：范围大小 = " << (range.second - range.first) << endl;
}

// ============================================================
// 四、遍历与变换
// ============================================================
// for_each：遍历每个元素，执行操作
// transform：变换元素，结果写到另一个地方
// copy / copy_if：拷贝元素
// ============================================================

void transformAlgorithms()
{
    cout << "\n=== 4. 遍历与变换 ===" << endl;

    vector<int> vec = {1, 2, 3, 4, 5};
    printVec(vec, "原始数据");

    // 4.1 for_each - 遍历
    cout << "  4.1 for_each 遍历：" << endl;
    for_each(vec.begin(), vec.end(), [](int& x) {
        x *= 2;  // 每个元素乘2
    });
    printVec(vec, "for_each 每个*2后");

    // 4.2 transform - 变换
    cout << "\n  4.2 transform 变换：" << endl;
    vector<string> strs = {"apple", "banana", "cherry"};
    cout << "  原始字符串: ";
    for (auto& s : strs) cout << s << " ";
    cout << endl;

    vector<int> lengths;
    transform(strs.begin(), strs.end(), back_inserter(lengths),
              [](const string& s) {
                  return s.length();  // 把每个字符串转成长度
              });
    printVec(lengths, "transform 得到长度");

    // 两个序列做运算
    vector<int> a = {1, 2, 3, 4, 5};
    vector<int> b = {10, 20, 30, 40, 50};
    vector<int> c;
    transform(a.begin(), a.end(), b.begin(), back_inserter(c),
              [](int x, int y) {
                  return x + y;  // 对应位置相加
              });
    printVec(c, "transform 两个序列相加");

    // 4.3 copy / copy_if
    cout << "\n  4.3 copy / copy_if：" << endl;
    vector<int> src = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    vector<int> dst;

    copy(src.begin() + 2, src.begin() + 7, back_inserter(dst));
    printVec(dst, "copy 索引2~6");

    dst.clear();
    copy_if(src.begin(), src.end(), back_inserter(dst),
            [](int x) { return x % 2 == 0; });
    printVec(dst, "copy_if 偶数");
}

// ============================================================
// 五、删除与替换
// ============================================================
// remove / remove_if：移除元素（注意：并不真正删除，只是移到后面）
// replace / replace_if：替换元素
// 经典惯用法：erase-remove 惯用法（真正删除元素）
// ============================================================

void removeReplaceAlgorithms()
{
    cout << "\n=== 5. 删除与替换 ===" << endl;

    vector<int> vec = {1, 2, 3, 2, 4, 2, 5, 2, 6};
    printVec(vec, "原始数据");

    // 5.1 remove - 移除元素（但不缩小容器！）
    cout << "\n  5.1 remove 的迷惑行为：" << endl;
    vector<int> v1 = vec;
    auto newEnd = remove(v1.begin(), v1.end(), 2);
    printVec(v1, "remove(2) 后（注意size没变！）");
    cout << "  remove 返回的新末尾位置索引: " << (newEnd - v1.begin()) << endl;
    cout << "  注意：remove 只是把要保留的元素移到前面，" << endl;
    cout << "       容器的size没变，后面的元素处于有效但未指定状态" << endl;

    // 5.2 erase-remove 惯用法（重点！）
    cout << "\n  5.2 erase-remove 惯用法（真正删除）：" << endl;
    vector<int> v2 = vec;
    v2.erase(remove(v2.begin(), v2.end(), 2), v2.end());
    printVec(v2, "erase+remove 后（真删了）");

    // remove_if + erase
    vector<int> v3 = vec;
    v3.erase(remove_if(v3.begin(), v3.end(),
                       [](int x) { return x % 2 == 0; }),
             v3.end());
    printVec(v3, "删除偶数后");

    // 5.3 replace / replace_if
    cout << "\n  5.3 replace 替换：" << endl;
    vector<int> v4 = vec;
    replace(v4.begin(), v4.end(), 2, 99);
    printVec(v4, "replace(2→99)");

    replace_if(v4.begin(), v4.end(),
               [](int x) { return x > 50; },
               0);
    printVec(v4, "replace_if(>50→0)");
}

// ============================================================
// 六、排列算法
// ============================================================
// next_permutation：下一个排列（按字典序）
// prev_permutation：上一个排列
// ============================================================

void permutationAlgorithms()
{
    cout << "\n=== 6. 排列算法 ===" << endl;

    string s = "abc";
    cout << "  字符串 \"" << s << "\" 的全排列：" << endl;

    // 必须先排序，才能用 next_permutation 生成全排列
    sort(s.begin(), s.end());
    int count = 0;
    do
    {
        cout << "    " << s;
        if (++count % 6 == 0) cout << endl;
    } while (next_permutation(s.begin(), s.end()));
    cout << "  共 " << count << " 种排列" << endl;

    cout << "\n  应用：生成全排列、组合、判断是否是下一个排列等" << endl;
}

// ============================================================
// 七、算术算法（numeric头文件）
// ============================================================
// accumulate：累加/累积
// inner_product：内积
// partial_sum：前缀和
// adjacent_difference：相邻差
// ============================================================

void numericAlgorithms()
{
    cout << "\n=== 7. 算术算法 ===" << endl;

    vector<int> vec = {1, 2, 3, 4, 5};
    printVec(vec, "原始数据");

    // 7.1 accumulate - 累加
    int sum = accumulate(vec.begin(), vec.end(), 0);
    cout << "  accumulate 累加和: " << sum << endl;

    // 自定义运算：累乘
    int product = accumulate(vec.begin(), vec.end(), 1,
                             [](int a, int b) { return a * b; });
    cout << "  accumulate 累乘: " << product << endl;

    // 7.2 inner_product - 内积（对应相乘再相加）
    cout << "\n  7.2 inner_product 内积：" << endl;
    vector<int> a = {1, 2, 3};
    vector<int> b = {4, 5, 6};
    int dot = inner_product(a.begin(), a.end(), b.begin(), 0);
    cout << "  [1,2,3] · [4,5,6] = " << dot
         << " (1*4 + 2*5 + 3*6 = 4+10+18=32)" << endl;

    // 7.3 partial_sum - 前缀和
    cout << "\n  7.3 partial_sum 前缀和：" << endl;
    vector<int> prefix;
    partial_sum(vec.begin(), vec.end(), back_inserter(prefix));
    printVec(prefix, "前缀和");

    // 7.4 adjacent_difference - 相邻差
    cout << "\n  7.4 adjacent_difference 相邻差：" << endl;
    vector<int> diff;
    adjacent_difference(vec.begin(), vec.end(), back_inserter(diff));
    printVec(diff, "相邻差");
}

// ============================================================
// 八、其他常用算法
// ============================================================

void otherAlgorithms()
{
    cout << "\n=== 8. 其他常用算法 ===" << endl;

    vector<int> vec = {5, 2, 9, 1, 5, 6};
    printVec(vec, "原始数据");

    // 8.1 min_element / max_element
    auto minIt = min_element(vec.begin(), vec.end());
    auto maxIt = max_element(vec.begin(), vec.end());
    cout << "  最小值: " << *minIt << "，最大值: " << *maxIt << endl;

    // 8.2 count / count_if
    int cnt = count(vec.begin(), vec.end(), 5);
    cout << "  count(5) = " << cnt << endl;
    cnt = count_if(vec.begin(), vec.end(), [](int x) { return x > 5; });
    cout << "  count_if(>5) = " << cnt << endl;

    // 8.3 all_of / any_of / none_of
    bool allPositive = all_of(vec.begin(), vec.end(),
                              [](int x) { return x > 0; });
    cout << "  all_of(>0) = " << (allPositive ? "true" : "false") << endl;

    bool hasEven = any_of(vec.begin(), vec.end(),
                          [](int x) { return x % 2 == 0; });
    cout << "  any_of(偶数) = " << (hasEven ? "true" : "false") << endl;

    bool noZero = none_of(vec.begin(), vec.end(),
                          [](int x) { return x == 0; });
    cout << "  none_of(==0) = " << (noZero ? "true" : "false") << endl;

    // 8.4 reverse / rotate
    cout << "\n  8.4 reverse / rotate：" << endl;
    vector<int> v1 = vec;
    reverse(v1.begin(), v1.end());
    printVec(v1, "reverse 反转");

    vector<int> v2 = vec;
    rotate(v2.begin(), v2.begin() + 2, v2.end());
    printVec(v2, "rotate 以索引2为轴旋转");

    // 8.5 unique - 去重（相邻重复）
    cout << "\n  8.5 unique 去重（配合erase真正删除）：" << endl;
    vector<int> v3 = {1, 1, 2, 2, 2, 3, 3, 4};
    printVec(v3, "原始");
    v3.erase(unique(v3.begin(), v3.end()), v3.end());
    printVec(v3, "unique后");
    cout << "  注意：unique只去相邻重复，所以通常先sort再unique" << endl;

    // 8.6 fill / generate
    cout << "\n  8.6 fill / generate：" << endl;
    vector<int> v4(5);
    fill(v4.begin(), v4.end(), 42);
    printVec(v4, "fill 全填42");

    int n = 0;
    generate(v4.begin(), v4.end(), [&n]() { return n++; });
    printVec(v4, "generate 依次生成");

    // 8.7 merge - 合并两个有序序列
    cout << "\n  8.7 merge 合并有序序列：" << endl;
    vector<int> a = {1, 3, 5, 7};
    vector<int> b = {2, 4, 6, 8};
    vector<int> merged;
    merge(a.begin(), a.end(), b.begin(), b.end(), back_inserter(merged));
    printVec(merged, "merge 合并后");
}

// ============================================================
// 九、lambda 与谓词
// ============================================================
// 谓词（Predicate）：返回bool的函数/函数对象/lambda
// 一元谓词：一个参数的谓词
// 二元谓词：两个参数的谓词
//
// lambda 语法：[捕获列表](参数列表) -> 返回类型 { 函数体 }
//   - 值捕获 [=]：所有外部变量值拷贝
//   - 引用捕获 [&]：所有外部变量引用
//   - 混合捕获 [x, &y]：x值捕获，y引用捕获
// ============================================================

void lambdaAndPredicate()
{
    cout << "\n=== 9. lambda与谓词 ===" << endl;

    vector<int> vec = {5, 2, 9, 1, 5, 6, 3, 7, 4, 8};
    printVec(vec, "原始数据");

    // 9.1 值捕获
    int threshold = 5;
    int cnt = count_if(vec.begin(), vec.end(),
                       [threshold](int x) { return x > threshold; });
    cout << "  值捕获 threshold=" << threshold
         << "，大于它的元素有 " << cnt << " 个" << endl;

    // 9.2 引用捕获（修改外部变量）
    int sum = 0;
    for_each(vec.begin(), vec.end(), [&sum](int x) { sum += x; });
    cout << "  引用捕获 sum，累加结果: " << sum << endl;

    // 9.3 混合捕获
    int factor = 2;
    int count = 0;
    for_each(vec.begin(), vec.end(), [factor, &count](int x) {
        if (x % factor == 0)
            ++count;
    });
    cout << "  factor值捕获，count引用捕获，偶数个数: " << count << endl;

    // 9.4 带状态的lambda（mutable）
    cout << "\n  9.4 mutable lambda：" << endl;
    int seed = 0;
    auto gen = [seed]() mutable { return seed++; };
    // mutable 允许修改值捕获的变量（修改的是副本，不影响外面）
    cout << "  gen() = " << gen() << endl;
    cout << "  gen() = " << gen() << endl;
    cout << "  外面的 seed 还是: " << seed << "（值捕获的副本）" << endl;

    // 9.5 自定义类型的排序
    cout << "\n  9.5 自定义类型排序：" << endl;
    struct Student
    {
        string name;
        int score;
    };
    vector<Student> students = {
        {"张三", 85}, {"李四", 92}, {"王五", 78}, {"赵六", 92}
    };

    // 按分数降序，分数相同按名字升序
    sort(students.begin(), students.end(),
         [](const Student& a, const Student& b) {
             if (a.score != b.score)
                 return a.score > b.score;  // 分数降序
             return a.name < b.name;       // 名字升序
         });

    cout << "  按分数降序、名字升序排序后：" << endl;
    for (const auto& s : students)
    {
        cout << "    " << s.name << ": " << s.score << endl;
    }
}

// ============================================================
// 十、迭代器适配器
// ============================================================
// insert迭代器：back_inserter, front_inserter, inserter
// 流迭代器：istream_iterator, ostream_iterator
// 反向迭代器：rbegin, rend
// 移动迭代器：make_move_iterator
// ============================================================

void iteratorAdapters()
{
    cout << "\n=== 10. 迭代器适配器 ===" << endl;

    vector<int> vec = {1, 2, 3, 4, 5};

    // 10.1 反向迭代器
    cout << "  10.1 反向迭代器：";
    for (auto it = vec.rbegin(); it != vec.rend(); ++it)
    {
        cout << *it << " ";
    }
    cout << endl;

    // 10.2 插入迭代器
    cout << "\n  10.2 插入迭代器：" << endl;
    vector<int> dst;
    copy(vec.begin(), vec.end(), back_inserter(dst));  // 尾插
    printVec(dst, "back_inserter 结果");

    list<int> lst;
    copy(vec.begin(), vec.end(), front_inserter(lst));  // 头插（顺序会反过来）
    cout << "  front_inserter 结果: ";
    for (int x : lst) cout << x << " ";
    cout << endl;

    vector<int> dst2 = {100, 200, 300};
    copy(vec.begin(), vec.end(),
         inserter(dst2, dst2.begin() + 1));  // 在指定位置插入
    printVec(dst2, "inserter 在位置1插入");

    // 10.3 流迭代器
    cout << "\n  10.3 ostream_iterator：" << endl;
    cout << "  用ostream_iterator输出vector: ";
    copy(vec.begin(), vec.end(), ostream_iterator<int>(cout, " "));
    cout << endl;
}

// ============================================================
// 主函数
// ============================================================
int main()
{
    srand((unsigned)time(nullptr));

    cout << "============================================================" << endl;
    cout << "  STL算法与迭代器 - 中级进阶Demo" << endl;
    cout << "============================================================" << endl;

    // 一、迭代器分类
    iteratorCategories();

    // 二、排序算法
    sortAlgorithms();

    // 三、查找算法
    findAlgorithms();

    // 四、遍历与变换
    transformAlgorithms();

    // 五、删除与替换
    removeReplaceAlgorithms();

    // 六、排列算法
    permutationAlgorithms();

    // 七、算术算法
    numericAlgorithms();

    // 八、其他算法
    otherAlgorithms();

    // 九、lambda与谓词
    lambdaAndPredicate();

    // 十、迭代器适配器
    iteratorAdapters();

    // ============================================================
    // 老师傅的话
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "============================================================" << endl;
    cout << "  徒弟啊，STL算法这块儿，我跟你说几句实在的：" << endl;
    cout << endl;
    cout << "  1. 不要重复造轮子。STL算法都是高度优化的，" << endl;
    cout << "     比你自己写的循环快得多，也不容易出错。" << endl;
    cout << "     能用上算法的地方，就别自己写for循环。" << endl;
    cout << endl;
    cout << "  2. 了解迭代器的分类很重要。" << endl;
    cout << "     不是所有算法都能用在所有容器上，" << endl;
    cout << "     得看容器支持什么迭代器。" << endl;
    cout << "     比如sort需要随机访问，list就用不了std::sort。" << endl;
    cout << endl;
    cout << "  3. erase-remove 惯用法一定要记住。" << endl;
    cout << "     remove不真删，只是把元素移到后面，" << endl;
    cout << "     必须配合erase才能真正删除元素。" << endl;
    cout << "     这是新手最容易踩的坑之一。" << endl;
    cout << endl;
    cout << "  4. lambda 是 STL 算法的好搭档。" << endl;
    cout << "     C++11之前用函数对象，丑得很。" << endl;
    cout << "     有了lambda，写谓词、写自定义比较都方便多了。" << endl;
    cout << "     但也别滥用，复杂的逻辑还是抽成函数好。" << endl;
    cout << endl;
    cout << "  5. 算法命名是有规律的：" << endl;
    cout << "     - 带_if的：按条件（如find_if, remove_if）" << endl;
    cout << "     - 带_copy的：结果输出到别处（如remove_copy）" << endl;
    cout << "     - 带_n的：操作n个元素（如copy_n）" << endl;
    cout << "     掌握规律，很多算法不用记也能猜到。" << endl;
    cout << endl;
    cout << "  6. 性能方面：" << endl;
    cout << "     - 要有序且经常查找 → 先sort再binary_search" << endl;
    cout << "     - 只需找最值 → min_element/max_element（O(n)）" << endl;
    cout << "     - 找第k大/小 → nth_element（平均O(n)）" << endl;
    cout << "     - 别啥都sort，杀鸡不用牛刀。" << endl;
    cout << "============================================================" << endl;

    // ============================================================
    // 小练习
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  小练习（回去自己做）" << endl;
    cout << "============================================================" << endl;
    cout << "  1. 用STL算法实现：统计字符串中每个字母出现的次数" << endl;
    cout << "  2. 用STL算法实现：去除vector中重复的元素（保留第一个）" << endl;
    cout << "  3. 实现一个自定义的Person类，包含name和age，" << endl;
    cout << "     用 sort 按年龄降序排序，年龄相同按名字升序" << endl;
    cout << "  4. 用 partial_sort 和 nth_element 分别找前10大的元素，" << endl;
    cout << "     对比性能差异" << endl;
    cout << "  5. 用accumulate实现：计算vector中所有偶数的平方和" << endl;
    cout << "  6. 思考：为什么list有自己的sort成员函数，" << endl;
    cout << "     而不直接用std::sort？" << endl;
    cout << "============================================================" << endl;

    return 0;
}
