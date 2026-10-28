// ============================================================
//  stl_containers.cpp - STL容器详解
// ============================================================
//  徒弟啊，STL是C++的瑞士军刀，容器是STL的基石。
//  写C++代码，十有八九都在跟容器打交道。
//  光会用还不行，你得知道底层是啥原理，才能选对用好。
//  今天咱把常用容器一个个拆开看，从底层到性能，全给你讲明白。
// ============================================================

#include <iostream>
#include <chrono>
#include <vector>
#include <list>
#include <deque>
#include <array>
#include <forward_list>

#include <map>
#include <set>

#include <unordered_map>
#include <unordered_set>

#include <stack>
#include <queue>

#include <string>
#include <algorithm>
#include <ctime>
#include <cstdlib>

using namespace std;

// ============================================================
// 一、STL容器分类概览
// ============================================================
// 徒弟，STL容器分三大类，你先把这个框架记牢：
//
// 1. 顺序容器（Sequence Containers）
//    - 元素按顺序排列，位置由插入时机和位置决定
//    - vector, deque, list, array, forward_list
//
// 2. 关联容器（Associative Containers）
//    - 元素按关键字（key）有序排列，位置由key决定
//    - map, set, multimap, multiset  → 底层红黑树
//    - unordered_map, unordered_set → 底层哈希表
//
// 3. 容器适配器（Container Adapters）
//    - 基于其他容器改造，提供特定接口
//    - stack（栈）, queue（队列）, priority_queue（优先队列）
// ============================================================

void containerOverview()
{
    cout << "============================================================" << endl;
    cout << "  STL容器分类概览" << endl;
    cout << "============================================================" << endl;
    cout << endl;
    cout << "  ┌──────────────────────────────────────────────────┐" << endl;
    cout << "  │ 顺序容器（按位置排列）                           │" << endl;
    cout << "  │  vector    → 动态数组，连续内存，随机访问O(1)   │" << endl;
    cout << "  │  deque     → 双端队列，分段连续，两端增删O(1)   │" << endl;
    cout << "  │  list      → 双向链表，非连续，任意位置增删O(1) │" << endl;
    cout << "  │  array     → 固定大小数组，连续内存              │" << endl;
    cout << "  │  forward_list → 单向链表                        │" << endl;
    cout << "  ├──────────────────────────────────────────────────┤" << endl;
    cout << "  │ 关联容器（按键值排列）                           │" << endl;
    cout << "  │  [有序，红黑树底层]                              │" << endl;
    cout << "  │    map / multimap   → key-value，key唯一/可重复 │" << endl;
    cout << "  │    set / multiset   → 只有key，key唯一/可重复   │" << endl;
    cout << "  │  [无序，哈希表底层]                              │" << endl;
    cout << "  │    unordered_map/unordered_multimap             │" << endl;
    cout << "  │    unordered_set/unordered_multiset             │" << endl;
    cout << "  ├──────────────────────────────────────────────────┤" << endl;
    cout << "  │ 容器适配器（包装其他容器）                       │" << endl;
    cout << "  │  stack           → 后进先出 LIFO                │" << endl;
    cout << "  │  queue           → 先进先出 FIFO                │" << endl;
    cout << "  │  priority_queue  → 优先级队列（堆）             │" << endl;
    cout << "  └──────────────────────────────────────────────────┘" << endl;
}

// ============================================================
// 二、vector 深入剖析
// ============================================================
// vector是最常用的容器，没有之一。底层就是个动态数组。
//
// 核心特点：
//   - 连续内存存储 → 随机访问O(1)，缓存友好
//   - 尾部增删快（均摊O(1)），中间插入慢（O(n)）
//   - 动态扩容：不够了就重新分配更大的内存，把旧数据拷贝过去
//
// 扩容机制（重点！）：
//   - 当 size == capacity 时，再插入就会触发扩容
//   - 新容量 = 旧容量 * 增长因子（一般是2，VS是1.5）
//   - 扩容过程：分配新内存 → 拷贝/移动元素 → 释放旧内存 → 更新指针
//   - 扩容会导致迭代器、指针、引用全部失效！
//
// emplace_back vs push_back：
//   - push_back：先构造临时对象，再拷贝/移动到容器里
//   - emplace_back：直接在容器内存上构造，少一次拷贝/移动
//   - 对于简单类型差别不大，对于复杂对象emplace_back性能更好
// ============================================================

void vectorDemo_basic()
{
    cout << "\n=== 2.1 vector 基础操作 ===" << endl;

    vector<int> vec;
    cout << "  初始状态：size=" << vec.size()
         << ", capacity=" << vec.capacity() << endl;

    // 尾插
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i);
        cout << "  push_back(" << i << ") → size=" << vec.size()
             << ", capacity=" << vec.capacity() << endl;
    }

    // 随机访问
    cout << "  vec[3] = " << vec[3] << endl;
    cout << "  vec.at(5) = " << vec.at(5) << " (at会检查边界，越界抛异常)" << endl;

    // 头插和中间插（慢！）
    vec.insert(vec.begin(), -1);
    cout << "  头插 -1 后，vec[0] = " << vec[0] << endl;

    // 尾删
    vec.pop_back();
    cout << "  pop_back 后 size=" << vec.size() << endl;
}

void vectorDemo_capacity()
{
    cout << "\n=== 2.2 vector 扩容机制演示 ===" << endl;

    vector<int> vec;
    size_t prevCap = 0;

    cout << "  观察扩容过程（libstdc++ 每次翻倍，MSVC 约 1.5 倍）：" << endl;
    cout << "  ";
    for (int i = 0; i < 100; ++i)
    {
        vec.push_back(i);
        if (vec.capacity() != prevCap)
        {
            cout << "[" << vec.capacity() << "] ";
            prevCap = vec.capacity();
        }
    }
    cout << endl;

    // reserve 预分配空间，避免频繁扩容
    vector<int> vec2;
    vec2.reserve(100);  // 提前分配100个元素的空间
    cout << "\n  reserve(100) 后：size=" << vec2.size()
         << ", capacity=" << vec2.capacity() << endl;

    // shrink_to_fit 释放多余空间
    vec2.resize(10);
    cout << "  resize(10) 后：size=" << vec2.size()
         << ", capacity=" << vec2.capacity() << endl;
    vec2.shrink_to_fit();
    cout << "  shrink_to_fit 后：size=" << vec2.size()
         << ", capacity=" << vec2.capacity() << endl;
}

// 自定义类，观察构造和拷贝
class DemoObj
{
public:
    DemoObj(int v = 0) : value(v)
    {
        // cout << "  构造 DemoObj(" << value << ")" << endl;
    }
    DemoObj(const DemoObj& other) : value(other.value)
    {
        // cout << "  拷贝构造 DemoObj(" << value << ")" << endl;
    }
    DemoObj(DemoObj&& other) noexcept : value(other.value)
    {
        other.value = 0;
        // cout << "  移动构造 DemoObj(" << value << ")" << endl;
    }
    ~DemoObj()
    {
        // cout << "  析构 DemoObj(" << value << ")" << endl;
    }

    int value;
};

void vectorDemo_emplace()
{
    cout << "\n=== 2.3 emplace_back vs push_back ===" << endl;

    vector<DemoObj> vec;
    vec.reserve(10);  // 预分配，避免扩容干扰观察

    cout << "  push_back 传临时对象（会构造+移动/拷贝）：" << endl;
    vec.push_back(DemoObj(1));

    cout << "  emplace_back 传构造参数（直接在容器内存上构造）：" << endl;
    vec.emplace_back(2);  // 直接把参数传给构造函数

    cout << "  结论：对于自定义类型，emplace_back 更高效，" << endl;
    cout << "       省去了一次临时对象的构造和析构" << endl;
}

void vectorDemo_iteratorInvalidation()
{
    cout << "\n=== 2.4 迭代器失效（重要！）===" << endl;

    vector<int> vec = {1, 2, 3, 4, 5};
    auto it = vec.begin() + 2;  // 指向第3个元素

    cout << "  初始：*it = " << *it << endl;

    // 插入元素导致扩容 → 迭代器全部失效！
    // vec.reserve(100); // 如果预分配了足够空间，就不会扩容
    vec.insert(vec.begin(), 0);  // 头插，所有元素后移

    // cout << *it; // 错误！it已经失效了，访问是未定义行为！
    cout << "  insert 后，原来的迭代器失效了！不能再用了！" << endl;
    cout << "  正确做法：insert 会返回新的有效迭代器" << endl;

    auto newIt = vec.insert(vec.begin() + 3, 99);
    cout << "  insert 返回的迭代器指向新元素：" << *newIt << endl;

    cout << "\n  迭代器失效总结：" << endl;
    cout << "    - insert/push_back/emplace_back：如果触发扩容，全部失效" << endl;
    cout << "      没扩容的话，插入点及其之后的迭代器（含 end()）全部失效" << endl;
    cout << "    - erase：被删除元素及其之后的迭代器失效" << endl;
    cout << "    - pop_back：尾迭代器和最后一个元素的迭代器失效" << endl;
}

// ============================================================
// 三、list 双向链表
// ============================================================
// list底层是双向链表，每个节点存数据+前后指针。
//
// 特点：
//   - 不连续内存 → 不能随机访问，查找O(n)
//   - 任意位置插入删除都是O(1)（只要你有迭代器）
//   - 没有扩容问题，插入删除不会导致其他迭代器失效
//   - 额外开销大（每个节点两个指针），缓存不友好
//
// 适用场景：频繁在中间插入删除，且不需要随机访问
// ============================================================

void listDemo()
{
    cout << "\n=== 3. list 双向链表 ===" << endl;

    list<int> lst = {3, 1, 4, 1, 5, 9, 2, 6};

    cout << "  list内容：";
    for (int x : lst) cout << x << " ";
    cout << endl;

    // 首尾访问
    cout << "  front = " << lst.front() << ", back = " << lst.back() << endl;

    // 不能用 [] 或 +n，只能++ --
    auto it = lst.begin();
    ++it; ++it;  // 只能一步步走
    cout << "  走两步后，*it = " << *it << endl;

    // 任意位置插入 O(1)
    lst.insert(it, 100);  // 在it前面插入
    cout << "  insert(100) 后：";
    for (int x : lst) cout << x << " ";
    cout << endl;

    // list 自带 sort（因为通用sort需要随机访问迭代器）
    lst.sort();
    cout << "  sort 排序后：";
    for (int x : lst) cout << x << " ";
    cout << endl;

    // 反转
    lst.reverse();
    cout << "  reverse 反转后：";
    for (int x : lst) cout << x << " ";
    cout << endl;

    // 去重（相邻重复元素）
    lst.unique();
    cout << "  unique 去重后：";
    for (int x : lst) cout << x << " ";
    cout << endl;

    cout << "\n  list 特点：任意位置增删O(1)，但不能随机访问" << endl;
    cout << "  缓存不友好，节点多了遍历比vector慢很多" << endl;
}

// ============================================================
// 四、deque 双端队列
// ============================================================
// deque（double-ended queue）底层是一段段连续内存（中控器+缓冲区）。
// 看起来像连续的，实际上是多段拼起来的。
//
// 特点：
//   - 两端增删都是O(1)（均摊）
//   - 支持随机访问（但比vector慢一点，要算段号）
//   - 没有扩容导致的全量拷贝问题
//   - 中间插入删除慢（O(n)）
//   - 内存布局比vector复杂
//
// 适用场景：需要两端操作，又想随机访问
// （stack和queue的默认底层容器就是deque）
// ============================================================

void dequeDemo()
{
    cout << "\n=== 4. deque 双端队列 ===" << endl;

    deque<int> dq;

    // 两端都能加
    dq.push_back(10);
    dq.push_back(20);
    dq.push_front(5);
    dq.push_front(1);

    cout << "  push后内容：";
    for (int x : dq) cout << x << " ";
    cout << endl;

    // 随机访问
    cout << "  dq[2] = " << dq[2] << endl;

    // 两端都能删
    dq.pop_front();
    dq.pop_back();
    cout << "  pop后内容：";
    for (int x : dq) cout << x << " ";
    cout << endl;

    cout << "\n  deque 特点：两端增删O(1)，支持随机访问" << endl;
    cout << "  底层是分段连续内存，没有vector那种全量拷贝的扩容" << endl;
    cout << "  但随机访问比vector慢一点（要计算在哪个段）" << endl;
}

// ============================================================
// 五、map / multimap（红黑树底层）
// ============================================================
// map是键值对集合，key有序且唯一。底层是红黑树（一种自平衡二叉搜索树）。
//
// 红黑树的性质：
//   - 每个节点要么红要么黑
//   - 根节点是黑的
//   - 叶子节点（NIL）是黑的
//   - 红节点的孩子必须是黑的
//   - 从任一节点到其每个叶子的所有路径包含相同数目的黑节点
//
// 特点：
//   - 查找、插入、删除都是 O(log n)
//   - key 有序（中序遍历就是升序）
//   - key 唯一（multimap允许重复key）
//   - 插入删除后迭代器不失效（除了被删的那个）
//
// 适用场景：需要有序的key-value，查找插入删除频率差不多
// ============================================================

void mapDemo()
{
    cout << "\n=== 5. map 有序键值对（红黑树底层）===" << endl;

    map<string, int> scores;

    // 插入
    scores["张三"] = 95;      // []运算符，不存在就插入
    scores["李四"] = 88;
    scores.insert({"王五", 76});  // insert方式
    scores.emplace("赵六", 92);   // emplace方式

    cout << "  map内容（按key排序）：" << endl;
    for (const auto& pair : scores)
    {
        cout << "    " << pair.first << ": " << pair.second << endl;
    }

    // 查找
    auto it = scores.find("李四");
    if (it != scores.end())
    {
        cout << "  找到李四，分数: " << it->second << endl;
    }

    // [] 的副作用：如果key不存在，会插入一个默认值！
    // 所以查找最好用 find，不要用 []
    int s = scores["不存在的人"];  // 这会插入一条记录！默认值0
    cout << "  用[]访问不存在的key后，size = " << scores.size() << endl;
    cout << "  （所以查找一定要用 find！）" << endl;

    // 删除
    scores.erase("不存在的人");

    // lower_bound / upper_bound
    auto low = scores.lower_bound("李");  // 第一个 >= "李" 的
    auto high = scores.upper_bound("王"); // 第一个 > "王" 的
    cout << "  姓氏在李~王之间的人：" << endl;
    for (auto it = low; it != high; ++it)
    {
        cout << "    " << it->first << endl;
    }

    cout << "\n  map 特点：key有序，查找插入删除都是O(log n)" << endl;
    cout << "  底层红黑树，自平衡保证最坏情况也是O(log n)" << endl;
}

void multimapDemo()
{
    cout << "\n=== 5.1 multimap 允许重复key ===" << endl;

    multimap<string, string> courses;
    courses.insert({"张老师", "高等数学"});
    courses.insert({"张老师", "线性代数"});
    courses.insert({"李老师", "数据结构"});
    courses.insert({"李老师", "算法设计"});

    cout << "  multimap 内容：" << endl;
    for (const auto& pair : courses)
    {
        cout << "    " << pair.first << " → " << pair.second << endl;
    }

    // 查找某个key的所有值
    auto range = courses.equal_range("李老师");
    cout << "  李老师教的课：" << endl;
    for (auto it = range.first; it != range.second; ++it)
    {
        cout << "    " << it->second << endl;
    }
}

// ============================================================
// 六、set / multiset
// ============================================================
// set就是只有key没有value的map，底层也是红黑树。
// 用来存储不重复的有序集合。
// ============================================================

void setDemo()
{
    cout << "\n=== 6. set 有序集合 ===" << endl;

    set<int> s = {5, 2, 8, 1, 9, 2, 5};  // 重复的会自动去掉

    cout << "  set内容（自动排序+去重）：";
    for (int x : s) cout << x << " ";
    cout << endl;

    // 插入
    auto result = s.insert(3);
    cout << "  insert(3)，成功? " << (result.second ? "是" : "否")
         << "，值: " << *(result.first) << endl;

    result = s.insert(5);  // 已存在，插入失败
    cout << "  insert(5)，成功? " << (result.second ? "是" : "否") << endl;

    // 查找
    cout << "  count(8) = " << s.count(8) << " (0或1，因为key唯一)" << endl;
    cout << "  find(9) != end() ? " << (s.find(9) != s.end() ? "是" : "否") << endl;

    cout << "\n  set 特点：有序、去重，查找插入删除O(log n)" << endl;
    cout << "  常用于去重+排序、集合运算（交并差）" << endl;
}

// ============================================================
// 七、unordered_map / unordered_set（哈希表）
// ============================================================
// 无序容器，底层是哈希表（散列表）。
// 通过哈希函数把key映射到桶（bucket）里，理想情况查找O(1)。
//
// 哈希表原理：
//   - 数组 + 链表（或开放寻址）
//   - 哈希函数：hash(key) % bucket_count → 桶索引
//   - 哈希冲突：不同key映射到同一个桶 → 链地址法（挂链表）
//
// 特点：
//   - 平均情况查找插入删除都是 O(1)
//   - 最坏情况 O(n)（哈希冲突严重时）
//   - 元素无序
//   - 加载因子 = 元素数 / 桶数，超过阈值就rehash
//
// 适用场景：对顺序没要求，追求查找速度
// ============================================================

void unorderedMapDemo()
{
    cout << "\n=== 7. unordered_map 哈希表 ===" << endl;

    unordered_map<string, int> hashMap;

    hashMap["apple"] = 10;
    hashMap["banana"] = 20;
    hashMap["orange"] = 30;
    hashMap["grape"] = 15;

    cout << "  unordered_map 内容（无序）：" << endl;
    for (const auto& pair : hashMap)
    {
        cout << "    " << pair.first << ": " << pair.second << endl;
    }

    // 桶信息
    cout << "  桶的数量: " << hashMap.bucket_count() << endl;
    cout << "  加载因子: " << hashMap.load_factor() << endl;
    cout << "  最大加载因子: " << hashMap.max_load_factor() << endl;

    // 查看每个key在哪个桶
    cout << "\n  各个key所在的桶：" << endl;
    for (const auto& pair : hashMap)
    {
        cout << "    " << pair.first << " → 桶"
             << hashMap.bucket(pair.first) << endl;
    }

    // 查找
    auto it = hashMap.find("banana");
    if (it != hashMap.end())
    {
        cout << "\n  找到 banana，值: " << it->second << endl;
    }

    cout << "\n  unordered_map 特点：平均O(1)查找，无序" << endl;
    cout << "  哈希函数好、冲突少的时候，比map快很多" << endl;
    cout << "  但最坏情况是O(n)（全冲突），而且内存开销大" << endl;
}

// ============================================================
// 八、容器适配器
// ============================================================
// 适配器不是独立容器，是基于其他容器改造的，提供特定的接口。
//
// stack：默认基于deque，LIFO（后进先出）
// queue：默认基于deque，FIFO（先进先出）
// priority_queue：默认基于vector + 堆算法，最大堆
// ============================================================

void adapterDemo()
{
    cout << "\n=== 8. 容器适配器 ===" << endl;

    // 栈
    cout << "  8.1 stack (LIFO):" << endl;
    stack<int> stk;
    stk.push(1); stk.push(2); stk.push(3);
    cout << "    top = " << stk.top() << endl;
    stk.pop();
    cout << "    pop后 top = " << stk.top() << endl;

    // 队列
    cout << "  8.2 queue (FIFO):" << endl;
    queue<int> q;
    q.push(1); q.push(2); q.push(3);
    cout << "    front = " << q.front() << ", back = " << q.back() << endl;
    q.pop();
    cout << "    pop后 front = " << q.front() << endl;

    // 优先队列（最大堆，默认大顶堆）
    cout << "  8.3 priority_queue (最大堆):" << endl;
    priority_queue<int> pq;
    pq.push(3); pq.push(1); pq.push(4); pq.push(1); pq.push(5);
    cout << "    依次出队（从大到小）: ";
    while (!pq.empty())
    {
        cout << pq.top() << " ";
        pq.pop();
    }
    cout << endl;

    // 小顶堆
    priority_queue<int, vector<int>, greater<int>> minPq;
    minPq.push(3); minPq.push(1); minPq.push(4);
    cout << "    小顶堆，top = " << minPq.top() << endl;

    cout << "\n  适配器特点：接口受限，功能单一，语义明确" << endl;
    cout << "  需要什么功能就用什么适配器，代码更清晰" << endl;
}

// ============================================================
// 九、如何选择容器
// ============================================================
void howToChoose()
{
    cout << "\n============================================================" << endl;
    cout << "  九、容器选择指南" << endl;
    cout << "============================================================" << endl;
    cout << endl;
    cout << "  问：我该用哪个容器？" << endl;
    cout << "  答：默认用 vector！除非你有明确的理由不用它。" << endl;
    cout << endl;
    cout << "  选择决策树：" << endl;
    cout << endl;
    cout << "  需要 key-value 吗？" << endl;
    cout << "    ├─ 是 → 需要有序吗？" << endl;
    cout << "    │       ├─ 是 → map / multimap" << endl;
    cout << "    │       └─ 否 → 追求速度？unordered_map（注意哈希冲突）" << endl;
    cout << "    └─ 否 → 需要唯一元素吗？" << endl;
    cout << "            ├─ 是 → 需要有序？set / unordered_set" << endl;
    cout << "            └─ 否 → 默认 vector" << endl;
    cout << "                    ├─ 频繁两端增删 → deque" << endl;
    cout << "                    ├─ 频繁中间增删且不随机访问 → list" << endl;
    cout << "                    ├─ 大小固定 → array" << endl;
    cout << "                    └─ 其他情况 → vector" << endl;
    cout << endl;
    cout << "  经验法则：" << endl;
    cout << "    1. 不知道用啥就用 vector，大多数情况下它都是最优的" << endl;
    cout << "    2. 性能瓶颈不在容器选择上，别过早优化" << endl;
    cout << "    3. 真有性能问题了，先测再改，别想当然" << endl;
    cout << "    4. list 没你想的那么快，中间插入你得先找到位置（O(n)）" << endl;
    cout << "    5. vector 的缓存友好性吊打链表，数据量不大时vector遍历最快" << endl;
}

// ============================================================
// 十、性能对比（简易版）
// ============================================================

void performanceCompare()
{
    cout << "\n=== 10. 简易性能对比（100万条数据）===" << endl;
    // 说明：这是单次运行的粗略计时，不是严谨的 benchmark。
    // 用 steady_clock 量真实经过时间；clock() 量的是进程 CPU 时间，
    // 而且在 Windows 上精度往往只有几十毫秒，不适合这种短测试。

    const int N = 1000000;
    auto now = [] { return std::chrono::steady_clock::now(); };
    auto ms_since = [](std::chrono::steady_clock::time_point start) {
        return std::chrono::duration<double, std::milli>(
                   std::chrono::steady_clock::now() - start).count();
    };

    // vector 尾插
    vector<int> vec;
    auto t0 = now();
    for (int i = 0; i < N; ++i)
        vec.push_back(i);
    cout << "  vector push_back: " << ms_since(t0) << " ms" << endl;

    // list 尾插
    list<int> lst;
    t0 = now();
    for (int i = 0; i < N; ++i)
        lst.push_back(i);
    cout << "  list push_back:   " << ms_since(t0) << " ms" << endl;

    // deque 尾插
    deque<int> dq;
    t0 = now();
    for (int i = 0; i < N; ++i)
        dq.push_back(i);
    cout << "  deque push_back:  " << ms_since(t0) << " ms" << endl;

    // 遍历时重复几遍，避免耗时太短、计时噪声占主导
    long long sum = 0;
    t0 = now();
    for (int round = 0; round < 10; ++round)
        for (int x : vec) sum += x;
    cout << "  vector 遍历 x10: " << ms_since(t0)
         << " ms (sum=" << sum << ")" << endl;

    sum = 0;
    t0 = now();
    for (int round = 0; round < 10; ++round)
        for (int x : lst) sum += x;
    cout << "  list 遍历   x10: " << ms_since(t0)
         << " ms (sum=" << sum << ")" << endl;

    cout << "\n  在这台机器上，遍历速度 vector 通常明显快于 list，这就是缓存的力量；" << endl;
    cout << "  数据连续存放，CPU 缓存命中率高。具体倍数受机器和负载影响。" << endl;
}

// ============================================================
// 主函数
// ============================================================
int main()
{
    cout << "============================================================" << endl;
    cout << "  STL容器详解 - 中级进阶Demo" << endl;
    cout << "============================================================" << endl;

    // 一、容器分类
    containerOverview();

    // 二、vector
    vectorDemo_basic();
    vectorDemo_capacity();
    vectorDemo_emplace();
    vectorDemo_iteratorInvalidation();

    // 三、list
    listDemo();

    // 四、deque
    dequeDemo();

    // 五、map / multimap
    mapDemo();
    multimapDemo();

    // 六、set
    setDemo();

    // 七、unordered_map
    unorderedMapDemo();

    // 八、容器适配器
    adapterDemo();

    // 九、如何选择
    howToChoose();

    // 十、性能对比
    performanceCompare();

    // ============================================================
    // 老师傅的话
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "============================================================" << endl;
    cout << "  徒弟啊，容器这块儿水也不浅。我跟你说几句掏心窝子的：" << endl;
    cout << endl;
    cout << "  1. 了解容器的底层实现，才能选对用好。" << endl;
    cout << "     别光记API，得知道底层是什么数据结构，" << endl;
    cout << "     这样你才能预判各种操作的时间复杂度和行为。" << endl;
    cout << endl;
    cout << "  2. 默认用 vector。这不是我说的，是C++祖师爷说的。" << endl;
    cout << "     vector的缓存友好性太强了，大多数场景下它都是最快的。" << endl;
    cout << "     别听人瞎忽悠list插入快——插入之前你得先找到位置啊！" << endl;
    cout << "     找位置就要遍历，那就是O(n)了，跟vector移动元素差不多。" << endl;
    cout << endl;
    cout << "  3. map vs unordered_map：" << endl;
    cout << "     - 要有序、要范围查询 → map" << endl;
    cout << "     - 只查单个key，追求速度 → unordered_map" << endl;
    cout << "     但unordered_map最坏情况是O(n)，哈希函数很关键。" << endl;
    cout << "     自定义类型做key时，得自己提供哈希函数。" << endl;
    cout << endl;
    cout << "  4. 迭代器失效是个大坑。每次调用容器的修改函数，" << endl;
    cout << "     都要想想哪些迭代器会失效。" << endl;
    cout << "     特别是vector的insert/erase，很容易踩坑。" << endl;
    cout << endl;
    cout << "  5. emplace_back 比 push_back 香。" << endl;
    cout << "     C++11以后，能emplace就别push，省一次拷贝。" << endl;
    cout << "     简单类型差别不大，但复杂对象就明显了。" << endl;
    cout << "============================================================" << endl;

    // ============================================================
    // 小练习
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  小练习（回去自己做）" << endl;
    cout << "============================================================" << endl;
    cout << "  1. 写一个程序，验证vector的扩容倍数（GCC vs VS可能不同）" << endl;
    cout << "  2. 用list和vector分别实现一个『频繁在中间插入』的场景，" << endl;
    cout << "     对比性能，看看什么时候list才真的比vector快" << endl;
    cout << "  3. 自定义一个类作为unordered_map的key，" << endl;
    cout << "     自己实现哈希函数和相等比较函数" << endl;
    cout << "  4. 用 map 实现一个简单的字典，支持增删改查和范围查询" << endl;
    cout << "  5. 思考：为什么stack和queue默认用deque做底层，而不用vector？" << endl;
    cout << "  6. 动手画一下vector、list、deque、map的底层结构图" << endl;
    cout << "============================================================" << endl;

    return 0;
}
