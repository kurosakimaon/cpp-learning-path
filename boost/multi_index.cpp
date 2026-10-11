/**
 * Boost.MultiIndex 多索引容器 Demo
 *
 * 知识点覆盖：
 *   1. 多索引容器基础：一个容器，多个视图（索引）
 *   2. 有序索引（ordered_unique / ordered_non_unique）：基于红黑树
 *   3. 散列索引（hashed_unique / hashed_non_unique）：基于哈希表
 *   4. 序列索引（sequenced）：保持插入顺序，类似 list
 *   5. 随机访问索引（random_access）：类似 vector，支持 O(1) 随机访问
 *   6. 元素修改：modify / replace 的区别与使用场景
 *   7. 索引迭代与投影：通过 get<N>() 获取不同索引视图
 *   8. 实际应用场景：员工数据库、缓存系统等
 *
 * Boost 优势：
 *   - 单一容器提供多种访问方式，无需维护多个容器
 *   - 内存效率高：元素只存储一份
 *   - 一致性保证：所有索引自动同步更新
 *   - 丰富的索引类型：有序/散列/序列/随机访问
 *   - 复合键（composite_key）支持多字段索引
 *   - C++ 标准库中无对应功能（最接近的是 std::set + 手动维护映射）
 *
 * 编译命令（示例）：
 *   g++ -std=c++17 multi_index.cpp -o multi_index
 *   （MultiIndex 是纯头文件库，无需链接 boost 库）
 */

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <random>

// ========== Boost 头文件说明 ==========
// <boost/multi_index_container.hpp>     - multi_index_container 主模板
// <boost/multi_index/ordered_index.hpp> - 有序索引
// <boost/multi_index/hashed_index.hpp>  - 散列索引
// <boost/multi_index/sequenced_index.hpp> - 序列索引
// <boost/multi_index/random_access_index.hpp> - 随机访问索引
// <boost/multi_index/member.hpp>        - member 提取器（访问成员变量）
// <boost/multi_index/identity.hpp>      - identity 提取器
// <boost/multi_index/composite_key.hpp> - 复合键
// <boost/multi_index/global_fun.hpp>    - 全局函数提取器
// <boost/multi_index/mem_fun.hpp>       - 成员函数提取器

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/tag.hpp>

namespace mi = boost::multi_index;

// ============================================================
// 一、数据模型：员工信息
// ============================================================
struct Employee {
    int id;              // 员工编号（唯一）
    std::string name;    // 姓名
    std::string dept;    // 部门
    double salary;       // 薪资
    int hire_year;       // 入职年份

    void print() const {
        std::cout << "  " << std::setw(3) << id
                  << "  " << std::left << std::setw(10) << name
                  << "  " << std::setw(10) << dept
                  << "  $" << std::right << std::setw(8) << std::fixed << std::setprecision(2) << salary
                  << "  " << hire_year
                  << std::endl;
    }
};

// ============================================================
// 二、标签（Tag）定义
// ============================================================
// 使用标签而不是数字索引来访问不同的索引视图，提高代码可读性。
struct ById {};
struct ByName {};
struct ByDept {};
struct BySalary {};
struct ByInsertion {};
struct ByDeptSalary {};  // 复合键：部门+薪资

// ============================================================
// 三、多索引容器定义
// ============================================================
// 一个员工数据库，提供 6 种索引方式：
//   1. ById:         按员工编号（有序，唯一）
//   2. ByName:       按姓名（散列，唯一）
//   3. ByDept:       按部门（有序，非唯一）
//   4. BySalary:     按薪资（有序，非唯一）
//   5. ByInsertion:  按插入顺序（序列索引）
//   6. ByDeptSalary: 按部门+薪资（复合键，有序）

using EmployeeContainer = mi::multi_index_container<
    Employee,
    mi::indexed_by<
        // 索引 0：按 ID 有序（唯一键）
        mi::ordered_unique<
            mi::tag<ById>,
            mi::member<Employee, int, &Employee::id>
        >,
        // 索引 1：按姓名散列（唯一键）
        mi::hashed_unique<
            mi::tag<ByName>,
            mi::member<Employee, std::string, &Employee::name>
        >,
        // 索引 2：按部门有序（非唯一）
        mi::ordered_non_unique<
            mi::tag<ByDept>,
            mi::member<Employee, std::string, &Employee::dept>
        >,
        // 索引 3：按薪资有序（非唯一，降序）
        mi::ordered_non_unique<
            mi::tag<BySalary>,
            mi::member<Employee, double, &Employee::salary>,
            std::greater<double>  // 降序排列
        >,
        // 索引 4：序列索引（保持插入顺序）
        mi::sequenced<
            mi::tag<ByInsertion>
        >,
        // 索引 5：复合键（部门 + 薪资降序）
        mi::ordered_non_unique<
            mi::tag<ByDeptSalary>,
            mi::composite_key<
                Employee,
                mi::member<Employee, std::string, &Employee::dept>,
                mi::member<Employee, double, &Employee::salary>
            >,
            // 复合键比较：部门升序，薪资降序
            mi::composite_key_compare<
                std::less<std::string>,
                std::greater<double>
            >
        >
    >
>;

// ============================================================
// 四、生成测试数据
// ============================================================
std::vector<Employee> generate_sample_data() {
    return {
        {1001, "张三",   "研发部",  15000.0, 2020},
        {1002, "李四",   "市场部",  12000.0, 2019},
        {1003, "王五",   "研发部",  18000.0, 2018},
        {1004, "赵六",   "人事部",  10000.0, 2021},
        {1005, "钱七",   "研发部",  22000.0, 2015},
        {1006, "孙八",   "市场部",  14000.0, 2020},
        {1007, "周九",   "财务部",  16000.0, 2017},
        {1008, "吴十",   "研发部",  19000.0, 2016},
        {1009, "郑十一", "市场部",  11000.0, 2022},
        {1010, "王十二", "财务部",  13000.0, 2021},
    };
}

// ============================================================
// 五、基本操作与有序索引
// ============================================================
void ordered_index_demo(const EmployeeContainer& db) {
    std::cout << "\n--- 1. 有序索引（ordered_index） ---" << std::endl;

    // 1.1 按 ID 查找（有序唯一索引，O(log n)）
    std::cout << "1.1 按 ID 查找（ById 索引）:" << std::endl;
    const auto& by_id = db.get<ById>();
    auto it = by_id.find(1005);
    if (it != by_id.end()) {
        it->print();
    }

    // 1.2 范围查询：查找 ID 在 1003-1007 之间的员工
    std::cout << "\n1.2 范围查询 (ID 1003 ~ 1007):" << std::endl;
    auto range = by_id.range(
        [](int id) { return id >= 1003; },  // 下界谓词
        [](int id) { return id <= 1007; }   // 上界谓词
    );
    for (auto it = range.first; it != range.second; ++it) {
        it->print();
    }

    // 1.3 按薪资降序遍历（有序非唯一索引）
    std::cout << "\n1.3 按薪资降序（BySalary 索引）:" << std::endl;
    const auto& by_salary = db.get<BySalary>();
    int rank = 0;
    for (const auto& emp : by_salary) {
        std::cout << "  第 " << ++rank << " 名";
        emp.print();
        if (rank >= 5) break;  // 只显示前 5 名
    }

    // 1.4 lower_bound / upper_bound
    std::cout << "\n1.4 lower_bound / upper_bound (薪资 >= 15000):" << std::endl;
    auto lb = by_salary.lower_bound(15000.0);
    auto ub = by_salary.upper_bound(12000.0);  // 注意：降序排列，upper_bound 更小
    // 因为是降序，lower_bound(15000) 返回第一个 <= 15000 的位置
    // 这里用 equal_range 更直观
    std::cout << "  （降序索引的边界查询需要注意方向）" << std::endl;

    // 1.5 按部门分组查询（非唯一索引）
    std::cout << "\n1.5 按部门分组（ByDept 索引）:" << std::endl;
    const auto& by_dept = db.get<ByDept>();

    // equal_range 获取同一部门的所有员工
    auto [dept_begin, dept_end] = by_dept.equal_range("研发部");
    std::cout << "  研发部员工:" << std::endl;
    for (auto it = dept_begin; it != dept_end; ++it) {
        it->print();
    }

    // 统计各部门人数
    std::cout << "\n  各部门人数统计:" << std::endl;
    std::string last_dept;
    int count = 0;
    for (auto it = by_dept.begin(); it != by_dept.end(); ++it) {
        if (it->dept != last_dept) {
            if (!last_dept.empty()) {
                std::cout << "    " << last_dept << ": " << count << " 人" << std::endl;
            }
            last_dept = it->dept;
            count = 1;
        } else {
            ++count;
        }
    }
    if (!last_dept.empty()) {
        std::cout << "    " << last_dept << ": " << count << " 人" << std::endl;
    }
}

// ============================================================
// 六、散列索引
// ============================================================
void hashed_index_demo(const EmployeeContainer& db) {
    std::cout << "\n--- 2. 散列索引（hashed_index） ---" << std::endl;

    const auto& by_name = db.get<ByName>();

    // 2.1 按姓名查找（平均 O(1)，最坏 O(n)）
    std::cout << "2.1 按姓名查找（ByName 散列索引）:" << std::endl;
    auto it = by_name.find("王五");
    if (it != by_name.end()) {
        it->print();
    } else {
        std::cout << "  未找到" << std::endl;
    }

    // 2.2 散列索引的迭代（无序遍历）
    std::cout << "\n2.2 散列索引迭代（顺序不确定）:" << std::endl;
    std::cout << "  前 5 个（按散列桶顺序）:" << std::endl;
    int count = 0;
    for (auto it = by_name.begin(); it != by_name.end() && count < 5; ++it, ++count) {
        it->print();
    }

    // 2.3 桶接口
    std::cout << "\n2.3 散列桶信息:" << std::endl;
    std::cout << "  桶数量: " << by_name.bucket_count() << std::endl;
    std::cout << "  负载因子: " << by_name.load_factor() << std::endl;
    std::cout << "  最大负载因子: " << by_name.max_load_factor() << std::endl;
}

// ============================================================
// 七、序列索引与随机访问索引
// ============================================================
void sequenced_index_demo(EmployeeContainer& db) {
    std::cout << "\n--- 3. 序列索引（sequenced_index） ---" << std::endl;

    auto& by_insertion = db.get<ByInsertion>();

    // 3.1 按插入顺序遍历
    std::cout << "3.1 按插入顺序（序列索引）:" << std::endl;
    int count = 0;
    for (const auto& emp : by_insertion) {
        std::cout << "  [" << count++ << "] ";
        emp.print();
    }

    // 3.2 序列操作：push_front / push_back
    std::cout << "\n3.2 序列操作:" << std::endl;
    Employee new_emp = {1011, "新员工", "研发部", 17000.0, 2023};
    by_insertion.push_back(new_emp);
    std::cout << "  push_back 新员工后，最后一个:" << std::endl;
    by_insertion.back().print();

    // 3.3 relocate：移动元素位置（序列索引特有）
    std::cout << "\n3.3 relocate 移动元素位置:" << std::endl;
    auto it = by_insertion.begin();
    auto last = --by_insertion.end();
    by_insertion.relocate(by_insertion.begin(), last);  // 最后一个移到最前面
    std::cout << "  将最后一个移到最前面后，第一个:" << std::endl;
    by_insertion.front().print();

    // 3.4 反转
    by_insertion.reverse();
    std::cout << "\n3.4 反转序列后，前 3 个:" << std::endl;
    int i = 0;
    for (const auto& emp : by_insertion) {
        if (i++ >= 3) break;
        emp.print();
    }

    // 恢复顺序
    by_insertion.reverse();

    // 删除测试员工（通过 ID 索引删除）
    auto& by_id = db.get<ById>();
    by_id.erase(1011);
    std::cout << "\n3.5 通过 ID 索引删除测试员工" << std::endl;
    std::cout << "  当前员工数: " << db.size() << std::endl;
}

// ============================================================
// 八、复合键索引
// ============================================================
void composite_key_demo(const EmployeeContainer& db) {
    std::cout << "\n--- 4. 复合键索引 ---" << std::endl;

    const auto& by_dept_salary = db.get<ByDeptSalary>();

    // 4.1 按部门+薪资排序遍历
    std::cout << "4.1 按部门分组、组内按薪资降序（复合键）:" << std::endl;
    std::string last_dept;
    for (const auto& emp : by_dept_salary) {
        if (emp.dept != last_dept) {
            std::cout << "  --- " << emp.dept << " ---" << std::endl;
            last_dept = emp.dept;
        }
        emp.print();
    }

    // 4.2 复合键范围查询：研发部薪资在 15000-20000 之间的员工
    std::cout << "\n4.2 复合键范围查询（研发部，薪资 15000~20000）:" << std::endl;

    // 使用 make_tuple 构造复合键进行查询
    auto range = by_dept_salary.range(
        std::make_tuple("研发部", 20000.0),   // 上界（降序薪资，所以 20000 在前）
        std::make_tuple("研发部", 15000.0)    // 下界
    );

    for (auto it = range.first; it != range.second; ++it) {
        it->print();
    }
    // 注意：因为薪资是降序排列，范围查询的上下界要反过来
}

// ============================================================
// 九、元素修改：modify vs replace
// ============================================================
// modify：直接修改元素的成员，索引会自动重新排序
// replace：用一个新元素替换旧元素
//
// 两者的区别：
//   - modify 更高效（只修改需要改变的字段）
//   - replace 更安全（整体替换，不会有部分修改的中间状态）
//   - 如果修改了键字段，modify 会重新排列元素位置
//   - 如果修改导致键重复，modify 会失败并回滚

void modify_demo(EmployeeContainer& db) {
    std::cout << "\n--- 5. 元素修改：modify vs replace ---" << std::endl;

    auto& by_id = db.get<ById>();

    // 5.1 modify 修改元素
    std::cout << "5.1 modify 修改薪资（不影响键的修改）:" << std::endl;
    auto it = by_id.find(1001);
    if (it != by_id.end()) {
        std::cout << "  修改前: ";
        it->print();

        // modify 的参数是一个函数对象，用于修改元素
        bool success = by_id.modify(it, [](Employee& e) {
            e.salary += 2000;  // 涨薪 2000
        });

        std::cout << "  修改后: ";
        it->print();
        std::cout << "  modify 成功: " << std::boolalpha << success << std::endl;
    }

    // 5.2 修改键字段（ID）
    std::cout << "\n5.2 modify 修改键字段（会重新排序）:" << std::endl;
    it = by_id.find(1002);
    if (it != by_id.end()) {
        std::cout << "  修改前 ID = 1002: ";
        it->print();

        bool success = by_id.modify(it, [](Employee& e) {
            e.id = 2002;  // 修改 ID（键字段）
        });

        std::cout << "  修改 ID 为 2002，成功: " << std::boolalpha << success << std::endl;

        // 查找 1002（应该找不到了）
        std::cout << "  查找 1002: "
                  << (by_id.find(1002) == by_id.end() ? "找不到" : "还在")
                  << std::endl;

        // 查找 2002
        auto it2 = by_id.find(2002);
        if (it2 != by_id.end()) {
            std::cout << "  查找 2002: ";
            it2->print();
        }

        // 改回去
        by_id.modify(it2, [](Employee& e) { e.id = 1002; });
    }

    // 5.3 replace 替换元素
    std::cout << "\n5.3 replace 整体替换:" << std::endl;
    it = by_id.find(1003);
    if (it != by_id.end()) {
        Employee new_emp = *it;
        new_emp.name = "王五五";
        new_emp.salary = 19500.0;

        bool success = by_id.replace(it, new_emp);
        std::cout << "  replace 成功: " << std::boolalpha << success << std::endl;
        std::cout << "  替换后: ";
        it->print();
    }

    // 5.4 modify 失败回滚（修改为重复键）
    std::cout << "\n5.4 modify 失败自动回滚（键冲突）:" << std::endl;
    it = by_id.find(1001);
    if (it != by_id.end()) {
        int old_id = it->id;
        bool success = by_id.modify(it, [](Employee& e) {
            e.id = 1002;  // 与已有的 1002 冲突
        });

        std::cout << "  将 1001 改为 1002（重复键），成功: "
                  << std::boolalpha << success << std::endl;
        std::cout << "  元素 ID 保持原值: " << it->id
                  << "（自动回滚）" << std::endl;
    }
}

// ============================================================
// 十、索引投影（项目转换）
// ============================================================
// project 操作：在不同索引之间转换迭代器
// 这是 MultiIndex 的强大功能：从一个索引找到元素后，
// 可以快速获取该元素在其他索引中的迭代器。

void projection_demo(EmployeeContainer& db) {
    std::cout << "\n--- 6. 索引投影（Projection） ---" << std::endl;

    // 场景：按姓名找到员工，然后查看他在薪资排名中的位置
    std::cout << "6.1 场景：按姓名查找，然后看薪资排名" << std::endl;

    // 第一步：通过姓名索引找到员工
    const auto& by_name = db.get<ByName>();
    auto name_it = by_name.find("钱七");
    if (name_it != by_name.end()) {
        std::cout << "  找到员工: " << name_it->name << std::endl;

        // 第二步：投影到薪资索引
        const auto& by_salary = db.get<BySalary>();
        auto salary_it = by_salary.project<0>(name_it);  // 0 是 ById 的索引编号
        // 或者用 tag：
        // auto salary_it = db.project<BySalary>(name_it);

        // 计算排名
        int rank = std::distance(by_salary.begin(), salary_it) + 1;
        std::cout << "  薪资排名: 第 " << rank << " 名" << std::endl;
        std::cout << "  薪资: $" << name_it->salary << std::endl;
    }

    // 场景：在序列索引中找到某个元素，然后通过 ID 索引修改它
    std::cout << "\n6.2 场景：从序列索引投影到 ID 索引进行修改" << std::endl;
    auto& by_insertion = db.get<ByInsertion>();
    auto seq_it = std::next(by_insertion.begin(), 3);  // 第 4 个元素
    std::cout << "  序列中第 4 个元素: ";
    seq_it->print();

    // 投影到 ID 索引并修改
    auto& by_id = db.get<ById>();
    auto id_it = by_id.project<ByInsertion>(seq_it);
    by_id.modify(id_it, [](Employee& e) {
        e.salary += 500;
    });

    std::cout << "  投影修改后（涨薪 500）: ";
    seq_it->print();
}

// ============================================================
// 十一、性能对比：不同索引的查找效率
// ============================================================
void performance_demo(const EmployeeContainer& db) {
    std::cout << "\n--- 7. 性能对比：不同索引的查找效率 ---" << std::endl;

    const auto& by_id = db.get<ById>();
    const auto& by_name = db.get<ByName>();
    const auto& by_insertion = db.get<ByInsertion>();

    const int iterations = 100000;

    // 有序索引查找
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        by_id.find(1005);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ordered_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // 散列索引查找
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        by_name.find("钱七");
    }
    end = std::chrono::high_resolution_clock::now();
    auto hashed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // 线性查找（序列索引，模拟没有索引的情况）
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::find_if(by_insertion.begin(), by_insertion.end(),
                     [](const Employee& e) { return e.id == 1005; });
    }
    end = std::chrono::high_resolution_clock::now();
    auto linear_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "  数据量: " << db.size() << " 条，查找 " << iterations << " 次" << std::endl;
    std::cout << "  有序索引 (O(log n)):  " << ordered_time << " us" << std::endl;
    std::cout << "  散列索引 (O(1)):      " << hashed_time << " us" << std::endl;
    std::cout << "  线性查找 (O(n)):      " << linear_time << " us" << std::endl;
    std::cout << "  散列/有序 速度比: " << (double)ordered_time / hashed_time << "x" << std::endl;
    std::cout << "  线性/有序 速度比: " << (double)linear_time / ordered_time << "x" << std::endl;
}

// ============================================================
// 十二、实际应用场景：LRU 缓存
// ============================================================
// 使用 MultiIndex 实现 LRU（最近最少使用）缓存：
//   - 散列索引：按键快速查找
//   - 序列索引：维护使用顺序，头部是最新的，尾部是最旧的
//   - 访问元素时将其移到头部
//   - 缓存满时删除尾部元素

template <typename Key, typename Value>
class LRUCache {
public:
    explicit LRUCache(size_t capacity) : capacity_(capacity) {}

    // 获取值（不存在返回空指针）
    const Value* get(const Key& key) {
        auto& by_key = cache_.template get<ByKey>();
        auto it = by_key.find(key);
        if (it == by_key.end()) {
            return nullptr;
        }

        // 访问后移到头部（标记为最近使用）
        auto& by_seq = cache_.template get<BySeq>();
        by_seq.relocate(by_seq.begin(), cache_.template project<ByKey>(it));

        return &it->value;
    }

    // 插入或更新
    void put(const Key& key, const Value& value) {
        auto& by_key = cache_.template get<ByKey>();
        auto it = by_key.find(key);

        if (it != by_key.end()) {
            // 更新值并移到头部
            by_key.modify(it, [&value](auto& item) { item.value = value; });
            auto& by_seq = cache_.template get<BySeq>();
            by_seq.relocate(by_seq.begin(), cache_.template project<ByKey>(it));
        } else {
            // 检查容量
            if (cache_.size() >= capacity_) {
                // 删除最久未使用的（尾部）
                auto& by_seq = cache_.template get<BySeq>();
                by_seq.pop_back();
            }
            // 插入到头部
            auto& by_seq = cache_.template get<BySeq>();
            by_seq.push_front(CacheItem{key, value});
        }
    }

    size_t size() const { return cache_.size(); }
    size_t capacity() const { return capacity_; }

    void print() const {
        const auto& by_seq = cache_.template get<BySeq>();
        std::cout << "  Cache (最近 -> 最久):" << std::endl;
        for (const auto& item : by_seq) {
            std::cout << "    " << item.key << " = " << item.value << std::endl;
        }
    }

private:
    struct CacheItem {
        Key key;
        Value value;
    };

    struct ByKey {};
    struct BySeq {};

    using CacheContainer = mi::multi_index_container<
        CacheItem,
        mi::indexed_by<
            mi::hashed_unique<mi::tag<ByKey>, mi::member<CacheItem, Key, &CacheItem::key>>,
            mi::sequenced<mi::tag<BySeq>>
        >
    >;

    CacheContainer cache_;
    size_t capacity_;
};

void lru_cache_demo() {
    std::cout << "\n--- 8. 实际应用：LRU 缓存 ---" << std::endl;

    LRUCache<std::string, int> cache(4);  // 容量为 4

    std::cout << "8.1 插入元素:" << std::endl;
    cache.put("a", 1);
    cache.put("b", 2);
    cache.put("c", 3);
    cache.put("d", 4);
    cache.print();

    std::cout << "\n8.2 访问 'b'（移到头部）:" << std::endl;
    const int* val = cache.get("b");
    if (val) std::cout << "  b = " << *val << std::endl;
    cache.print();

    std::cout << "\n8.3 插入 'e'（淘汰最久的 'a'）:" << std::endl;
    cache.put("e", 5);
    cache.print();

    std::cout << "\n8.4 更新 'c' 的值:" << std::endl;
    cache.put("c", 30);
    cache.print();

    std::cout << "\n8.5 查找不存在的 'a':" << std::endl;
    val = cache.get("a");
    std::cout << "  a = " << (val ? std::to_string(*val) : "(不存在)") << std::endl;
}

// ============================================================
// Demo 入口函数
// ============================================================
int main() {
    std::cout << "========== Boost.MultiIndex 多索引容器 Demo ==========" << std::endl;

    // 初始化员工数据库
    EmployeeContainer db;
    auto sample = generate_sample_data();
    for (const auto& emp : sample) {
        db.insert(emp);
    }

    std::cout << "初始化完成，共 " << db.size() << " 名员工" << std::endl;

    // 打印表头
    auto print_header = [](const std::string& title) {
        std::cout << title << std::endl;
        std::cout << "  " << std::string(55, '-') << std::endl;
        std::cout << "  " << std::setw(3) << "ID"
                  << "  " << std::left << std::setw(10) << "姓名"
                  << "  " << std::setw(10) << "部门"
                  << "  " << std::right << std::setw(9) << "薪资"
                  << "  " << "入职"
                  << std::endl;
        std::cout << "  " << std::string(55, '-') << std::endl;
    };

    // 1. 有序索引
    print_header("");
    ordered_index_demo(db);

    // 2. 散列索引
    hashed_index_demo(db);

    // 3. 序列索引
    sequenced_index_demo(db);

    // 4. 复合键索引
    composite_key_demo(db);

    // 5. 元素修改
    modify_demo(db);

    // 6. 索引投影
    projection_demo(db);

    // 7. 性能对比
    performance_demo(db);

    // 8. LRU 缓存
    lru_cache_demo();

    // --------------------------------------------------------
    // 总结
    // --------------------------------------------------------
    std::cout << "\n--- Boost.MultiIndex 核心要点总结 ---" << std::endl;
    std::cout << "  1. 单一容器，多种视图：元素只存一份，多索引自动同步" << std::endl;
    std::cout << "  2. 索引类型：" << std::endl;
    std::cout << "     - ordered_unique/non_unique: 有序，O(log n) 查找" << std::endl;
    std::cout << "     - hashed_unique/non_unique:  散列，O(1) 查找" << std::endl;
    std::cout << "     - sequenced: 序列，保持插入顺序" << std::endl;
    std::cout << "     - random_access: 随机访问，类似 vector" << std::endl;
    std::cout << "  3. 元素修改：modify（函数式修改）vs replace（整体替换）" << std::endl;
    std::cout << "  4. 索引投影：在不同索引间转换迭代器" << std::endl;
    std::cout << "  5. 复合键：多字段联合索引" << std::endl;
    std::cout << "  6. 典型应用：" << std::endl;
    std::cout << "     - 员工/学生信息管理（多维度查询）" << std::endl;
    std::cout << "     - LRU/LFU 缓存" << std::endl;
    std::cout << "     - 排行榜系统（多个排名维度）" << std::endl;
    std::cout << "     - 任何需要多种访问方式的数据集" << std::endl;

    std::cout << "\n========== Demo 结束 ==========" << std::endl;
    return 0;
}
