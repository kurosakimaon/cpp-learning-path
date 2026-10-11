/**
 * @file bimap_demo.cpp
 * @brief Boost.Bimap 双向映射深度解析
 *
 * Boost.Bimap 是一个双向映射容器。
 * 普通的 map 只能从 key 查 value，bimap 可以双向查：
 *   - 从左查右（像普通 map）
 *   - 从右查左（反向查找）
 *
 * 没有 bimap 的时候，你得写两个 map，还要手动维护同步，麻烦又容易出错。
 * bimap 帮你把这件事做对了。
 *
 * 主要内容：
 * 1. bimap 基础：双向查找
 * 2. 左右视图：left 和 right
 * 3. 不同的集合类型（有序、无序、向量、列表）
 * 4. 标签（Tagged）：给左右两边起名字
 * 5. 插入与删除
 * 6. 遍历
 * 7. 实际应用：ID-名称映射、中英文翻译字典、状态码映射
 *
 * 老师傅的话：
 *   日常编码中，双向查找的需求太常见了。
 *   ID 查名称，名称查 ID；状态码查描述，描述查状态码……
 *   以前我都是维护两个 map，写一堆同步代码，又臭又长。
 *   自从用了 bimap，世界清净了。
 *
 * 编译命令: g++ -std=c++17 bimap_demo.cpp -o bimap_demo
 * （纯头文件库，不需要链接）
 *
 * 头文件：<boost/bimap.hpp>
 *        <boost/bimap/unordered_set_of.hpp>
 *        <boost/bimap/vector_of.hpp>
 *        <boost/bimap/list_of.hpp>
 *        <boost/bimap/multiset_of.hpp>
 */

#include <iostream>
#include <string>
#include <cassert>

// Boost.Bimap 头文件
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/bimap/vector_of.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/support/lambda.hpp>

namespace bm = boost::bimaps;

// ============================================================================
// 1. bimap 基础
// ============================================================================
void demo_basic_bimap() {
    std::cout << "\n===== 1. bimap 基础 =====\n";

    // 定义一个双向映射：int <-> std::string
    // 左边是 int（ID），右边是 string（名称）
    typedef boost::bimap<int, std::string> IdNameMap;
    IdNameMap id_name;

    // 插入数据（两种方式）
    id_name.insert(IdNameMap::value_type(1, "Alice"));
    id_name.insert(IdNameMap::value_type(2, "Bob"));
    id_name.insert(IdNameMap::value_type(3, "Charlie"));

    std::cout << "  已插入 3 条映射: 1↔Alice, 2↔Bob, 3↔Charlie\n\n";

    // 从左查右（按 ID 查名称）
    std::cout << "  从左查右（ID → 名称）:\n";
    auto it1 = id_name.left.find(2);
    if (it1 != id_name.left.end()) {
        std::cout << "    ID=2 的名称是: " << it1->second << "\n";
    }

    // 从右查左（按名称查 ID）
    std::cout << "\n  从右查左（名称 → ID）:\n";
    auto it2 = id_name.right.find("Charlie");
    if (it2 != id_name.right.end()) {
        std::cout << "    Charlie 的 ID 是: " << it2->second << "\n";
    }

    // 大小
    std::cout << "\n  映射数量: " << id_name.size() << "\n";

    // 检查是否存在
    std::cout << "\n  存在性检查:\n";
    std::cout << "    左边有 ID=1? "
              << std::boolalpha << (id_name.left.count(1) > 0) << "\n";
    std::cout << "    右边有 \"Dave\"? "
              << (id_name.right.count("Dave") > 0) << "\n";

    std::cout << "\n  核心概念：\n";
    std::cout << "    bimap.left  是左视图，像个普通 map（左为key）\n";
    std::cout << "    bimap.right 是右视图，像个反向 map（右为key）\n";
    std::cout << "    两边共享数据，修改一边另一边自动同步\n";
}

// ============================================================================
// 2. 遍历 bimap
// ============================================================================
void demo_traversal() {
    std::cout << "\n===== 2. 遍历 =====\n";

    typedef boost::bimap<int, std::string> BimapType;
    BimapType bm;

    bm.insert(BimapType::value_type(101, "北京"));
    bm.insert(BimapType::value_type(102, "上海"));
    bm.insert(BimapType::value_type(103, "广州"));
    bm.insert(BimapType::value_type(104, "深圳"));

    // 从左到右遍历（按左边排序）
    std::cout << "  从左到右遍历（按 ID 升序）:\n";
    for (const auto& item : bm.left) {
        std::cout << "    " << item.first << " → " << item.second << "\n";
    }

    // 从右到左遍历（按右边排序）
    std::cout << "\n  从右到左遍历（按名称排序）:\n";
    for (const auto& item : bm.right) {
        std::cout << "    " << item.first << " → " << item.second << "\n";
    }

    // 整体遍历（按关系）
    std::cout << "\n  整体遍历（关系视图）:\n";
    for (const auto& rel : bm) {
        std::cout << "    " << rel.left << " ↔ " << rel.right << "\n";
    }

    // 反向迭代
    std::cout << "\n  左视图反向遍历:\n";
    for (auto it = bm.left.rbegin(); it != bm.left.rend(); ++it) {
        std::cout << "    " << it->first << " → " << it->second << "\n";
    }
}

// ============================================================================
// 3. 插入、删除与修改
// ============================================================================
void demo_insert_erase() {
    std::cout << "\n===== 3. 插入、删除与修改 =====\n";

    typedef boost::bimap<int, std::string> BimapType;
    BimapType bm;

    // 插入
    std::cout << "  插入元素:\n";
    auto result = bm.insert(BimapType::value_type(1, "one"));
    std::cout << "    插入 (1, one) "
              << (result.second ? "成功" : "失败（已存在）") << "\n";

    result = bm.insert(BimapType::value_type(1, "yi"));
    std::cout << "    插入 (1, yi)  "
              << (result.second ? "成功" : "失败（key已存在）") << "\n";

    result = bm.insert(BimapType::value_type(2, "one"));
    std::cout << "    插入 (2, one)  "
              << (result.second ? "成功" : "失败（value已存在）")
              << "（两边都是唯一的！）\n";

    bm.insert(BimapType::value_type(2, "two"));
    bm.insert(BimapType::value_type(3, "three"));
    bm.insert(BimapType::value_type(4, "four"));
    bm.insert(BimapType::value_type(5, "five"));

    std::cout << "\n  当前映射（共 " << bm.size() << " 条）:\n";
    for (const auto& rel : bm) {
        std::cout << "    " << rel.left << " ↔ " << rel.right << "\n";
    }

    // 删除（按左边删除）
    std::cout << "\n  删除操作:\n";
    size_t erased = bm.left.erase(3);
    std::cout << "    左删除 key=3，删除了 " << erased << " 条\n";

    // 删除（按右边删除）
    erased = bm.right.erase("five");
    std::cout << "    右删除 key=\"five\"，删除了 " << erased << " 条\n";

    // 删除（迭代器删除）
    auto it = bm.left.find(4);
    if (it != bm.left.end()) {
        bm.left.erase(it);
        std::cout << "    迭代器删除 key=4，成功\n";
    }

    std::cout << "\n  删除后（剩 " << bm.size() << " 条）:\n";
    for (const auto& rel : bm) {
        std::cout << "    " << rel.left << " ↔ " << rel.right << "\n";
    }

    // 修改（replace）
    // 注意：不能直接修改，因为会破坏映射关系
    // 要用 replace 方法
    std::cout << "\n  修改操作（replace）:\n";
    auto it_mod = bm.left.find(1);
    if (it_mod != bm.left.end()) {
        bm.left.replace_key(it_mod, 100);
        std::cout << "    把左边 1 改成 100\n";
    }

    auto it_mod2 = bm.right.find("two");
    if (it_mod2 != bm.right.end()) {
        bm.right.replace_data(it_mod2, "第二");
        std::cout << "    把右边 two 改成 第二\n";
    }

    std::cout << "\n  修改后:\n";
    for (const auto& rel : bm) {
        std::cout << "    " << rel.left << " ↔ " << rel.right << "\n";
    }

    std::cout << "\n  注意：bimap 的两边都是唯一的（默认 set_of）\n";
    std::cout << "  不能有重复的 key，也不能有重复的 value\n";
}

// ============================================================================
// 4. 集合类型（左/右两边可以是不同的集合类型）
// ============================================================================
void demo_collection_types() {
    std::cout << "\n===== 4. 不同的集合类型 =====\n";

    // bimap 的左右两边可以指定不同的集合类型
    // 默认是 set_of（有序、唯一）
    // 可选类型：
    //   set_of          - 有序集合（红黑树，默认）
    //   multiset_of     - 有序多重集合（允许重复）
    //   unordered_set_of - 无序集合（哈希表）
    //   unordered_multiset_of - 无序多重集合
    //   vector_of       - 向量（有序按插入顺序，可随机访问）
    //   list_of         - 链表（按插入顺序，双向遍历）
    //   unconstrained_set_of - 无约束（不建立索引，节省空间）

    // 例子1：左边有序，右边无序
    std::cout << "  例子1：左 set（有序）+ 右 unordered_set（哈希）\n";
    typedef boost::bimap<
        bm::set_of<int>,
        bm::unordered_set_of<std::string>
    > Bimap1;

    Bimap1 b1;
    b1.insert(Bimap1::value_type(3, "three"));
    b1.insert(Bimap1::value_type(1, "one"));
    b1.insert(Bimap1::value_type(2, "two"));

    std::cout << "    左视图（有序）: ";
    for (const auto& item : b1.left) {
        std::cout << item.first << " ";
    }
    std::cout << "\n    右视图（无序，哈希）: ";
    for (const auto& item : b1.right) {
        std::cout << item.first << " ";
    }
    std::cout << "\n";

    // 例子2：允许重复（multiset）
    std::cout << "\n  例子2：左唯一 + 右可重复（一对多）\n";
    typedef boost::bimap<
        bm::set_of<int>,
        bm::multiset_of<std::string>  // 右边允许多个相同的值
    > Bimap2;

    Bimap2 b2;
    b2.insert(Bimap2::value_type(1, "fruit"));
    b2.insert(Bimap2::value_type(2, "fruit"));  // 右边可以重复
    b2.insert(Bimap2::value_type(3, "vegetable"));

    std::cout << "    同一个 category 可以有多个 ID:\n";
    auto range = b2.right.equal_range("fruit");
    for (auto it = range.first; it != range.second; ++it) {
        std::cout << "      fruit → ID=" << it->second << "\n";
    }

    // 例子3：vector_of（保持插入顺序）
    std::cout << "\n  例子3：向量视图（保持插入顺序）\n";
    typedef boost::bimap<
        bm::set_of<int>,
        bm::vector_of<std::string>  // 右边按插入顺序
    > Bimap3;

    Bimap3 b3;
    b3.insert(Bimap3::value_type(10, "last"));
    b3.insert(Bimap3::value_type(5, "middle"));
    b3.insert(Bimap3::value_type(1, "first"));

    std::cout << "    左视图（按key排序）: ";
    for (const auto& item : b3.left) {
        std::cout << item.first << " ";
    }
    std::cout << "\n    右视图（按插入顺序）: ";
    for (const auto& item : b3.right) {
        std::cout << item.first << " ";
    }
    std::cout << "\n";

    std::cout << "\n  选择集合类型的依据：\n";
    std::cout << "    - 需要排序查找 → set_of（红黑树，O(log n)）\n";
    std::cout << "    - 只需快速查找 → unordered_set_of（哈希，O(1)平均）\n";
    std::cout << "    - 允许重复 → multiset_of / unordered_multiset_of\n";
    std::cout << "    - 保持插入顺序 → vector_of / list_of\n";
    std::cout << "    - 很少从那边查 → unconstrained_set_of（省空间）\n";
}

// ============================================================================
// 5. 标签（Tagged）：给左右两边起有意义的名字
// ============================================================================
// 默认用 left 和 right，有时候语义不清晰。
// 用 tagged 可以给两边起名字，提高代码可读性。

struct id {};
struct name {};
struct code {};
struct description {};

void demo_tagged() {
    std::cout << "\n===== 5. 标签（Tagged）=====\n";

    // 用标签定义 bimap，语义更清晰
    typedef boost::bimap<
        bm::tagged<int, id>,          // 左边叫 id
        bm::tagged<std::string, name>  // 右边叫 name
    > UserMap;

    UserMap users;
    users.insert(UserMap::value_type(101, "张三"));
    users.insert(UserMap::value_type(102, "李四"));
    users.insert(UserMap::value_type(103, "王五"));

    // 通过标签访问，可读性更好
    std::cout << "  通过标签访问（by<id> / by<name>）:\n";

    auto it1 = users.by<id>().find(102);
    if (it1 != users.by<id>().end()) {
        std::cout << "    ID=102 的用户名: "
                  << it1->get<name>() << "\n";
    }

    auto it2 = users.by<name>().find("张三");
    if (it2 != users.by<name>().end()) {
        std::cout << "    张三 的 ID: "
                  << it2->get<id>() << "\n";
    }

    // 访问元素的值也用标签
    std::cout << "\n  带标签的遍历:\n";
    for (const auto& user : users) {
        std::cout << "    ID=" << user.get<id>()
                  << ", 姓名=" << user.get<name>() << "\n";
    }

    // 更复杂的例子：三字母代码 ↔ 中文名称
    struct city_code {};
    struct city_name {};

    typedef boost::bimap<
        bm::tagged<std::string, city_code>,
        bm::tagged<std::string, city_name>
    > CityMap;

    CityMap cities;
    cities.insert(CityMap::value_type("PEK", "北京"));
    cities.insert(CityMap::value_type("SHA", "上海"));
    cities.insert(CityMap::value_type("CAN", "广州"));
    cities.insert(CityMap::value_type("SZX", "深圳"));

    std::cout << "\n  城市代码映射:\n";
    std::cout << "    PEK → "
              << cities.by<city_code>().find("PEK")->get<city_name>()
              << "\n";
    std::cout << "    上海 → "
              << cities.by<city_name>().find("上海")->get<city_code>()
              << "\n";

    std::cout << "\n  标签的好处：\n";
    std::cout << "    - 代码语义清晰，不用记哪边是左哪边是右\n";
    std::cout << "    - 重构时改顺序不会影响业务逻辑\n";
    std::cout << "    - 推荐：实际项目中尽量用标签\n";
}

// ============================================================================
// 6. 实战1：状态码映射
// ============================================================================
void demo_status_code_map() {
    std::cout << "\n===== 6. 实战1：状态码映射 =====\n";

    // 实际项目中经常要做状态码和描述的双向转换
    struct code {};
    struct desc {};

    typedef boost::bimap<
        bm::tagged<int, code>,
        bm::tagged<std::string, desc>
    > StatusMap;

    StatusMap status_map;

    // 初始化状态码
    status_map.insert(StatusMap::value_type(0, "成功"));
    status_map.insert(StatusMap::value_type(1001, "参数错误"));
    status_map.insert(StatusMap::value_type(1002, "未授权"));
    status_map.insert(StatusMap::value_type(1003, "资源不存在"));
    status_map.insert(StatusMap::value_type(1004, "系统繁忙"));
    status_map.insert(StatusMap::value_type(2001, "数据库错误"));
    status_map.insert(StatusMap::value_type(3001, "网络超时"));

    // 正向查询：状态码 → 描述（给用户看）
    auto get_desc = [&](int code_val) -> std::string {
        auto it = status_map.by<code>().find(code_val);
        if (it != status_map.by<code>().end()) {
            return it->get<desc>();
        }
        return "未知错误";
    };

    // 反向查询：描述 → 状态码（比如日志分析时用）
    auto get_code = [&](const std::string& desc_val) -> int {
        auto it = status_map.by<desc>().find(desc_val);
        if (it != status_map.by<desc>().end()) {
            return it->get<code>();
        }
        return -1;
    };

    std::cout << "  正向查询（码→描述）:\n";
    for (int code : {0, 1001, 2001, 9999}) {
        std::cout << "    " << code << " → " << get_desc(code) << "\n";
    }

    std::cout << "\n  反向查询（描述→码）:\n";
    for (const auto& desc : {"成功", "参数错误", "未知的描述"}) {
        std::cout << "    " << desc << " → " << get_code(desc) << "\n";
    }

    std::cout << "\n  这比维护两个 map 简洁多了，也不容易出错\n";
}

// ============================================================================
// 7. 实战2：中英文翻译字典
// ============================================================================
void demo_translation_dict() {
    std::cout << "\n===== 7. 实战2：中英文字典 =====\n";

    struct cn {};
    struct en {};

    typedef boost::bimap<
        bm::tagged<std::string, cn>,
        bm::tagged<std::string, en>
    > Dict;

    Dict dict;
    dict.insert(Dict::value_type("苹果", "apple"));
    dict.insert(Dict::value_type("香蕉", "banana"));
    dict.insert(Dict::value_type("橙子", "orange"));
    dict.insert(Dict::value_type("葡萄", "grape"));
    dict.insert(Dict::value_type("西瓜", "watermelon"));

    // 英译中
    auto to_cn = [&](const std::string& word) -> std::string {
        auto it = dict.by<en>().find(word);
        return it != dict.by<en>().end() ? it->get<cn>() : "（未找到）";
    };

    // 中译英
    auto to_en = [&](const std::string& word) -> std::string {
        auto it = dict.by<cn>().find(word);
        return it != dict.by<cn>().end() ? it->get<en>() : "(not found)";
    };

    std::cout << "  中译英:\n";
    for (const auto& word : {"苹果", "香蕉", "火龙果"}) {
        std::cout << "    " << word << " → " << to_en(word) << "\n";
    }

    std::cout << "\n  英译中:\n";
    for (const auto& word : {"apple", "grape", "mango"}) {
        std::cout << "    " << word << " → " << to_cn(word) << "\n";
    }

    std::cout << "\n  注意：bimap 两边都是唯一的，所以是一一对应\n";
    std::cout << "  如果要支持一词多义，用 multiset_of\n";
}

// ============================================================================
// 8. 性能对比：bimap vs 两个map
// ============================================================================
void demo_performance() {
    std::cout << "\n===== 8. bimap vs 两个map =====\n";

    std::cout << "  功能对比:\n";
    std::cout << "    特性          两个 std::map    boost::bimap\n";
    std::cout << "    ─────────────────────────────────────────\n";
    std::cout << "    正向查找          O(log n)        O(log n)\n";
    std::cout << "    反向查找          O(n) 或 O(log n) O(log n)\n";
    std::cout << "    插入              O(log n)        O(log n)\n";
    std::cout << "    删除              O(log n)        O(log n)\n";
    std::cout << "    同步维护          手动（容易错）   自动\n";
    std::cout << "    内存占用          两份key         一份数据\n";
    std::cout << "    代码复杂度        高              低\n";

    std::cout << "\n  bimap 的优势：\n";
    std::cout << "    1. 数据只存一份，节省内存\n";
    std::cout << "    2. 自动双向同步，不会出现不一致\n";
    std::cout << "    3. 代码更简洁，语义更清晰\n";
    std::cout << "    4. 支持多种集合类型（有序/无序/向量/...）\n";

    std::cout << "\n  bimap 的注意事项：\n";
    std::cout << "    1. 默认两边都是唯一的（set），不能有重复\n";
    std::cout << "    2. 不能直接修改元素，要用 replace\n";
    std::cout << "    3. 头文件较重，编译时间略长\n";
    std::cout << "    4. 调试时查看数据不如普通 map 直观\n";
}

// ============================================================================
// 老师傅的话
// ============================================================================
void laoshifu_says() {
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "  👴 老师傅的话\n";
    std::cout << "============================================================\n";
    std::cout << "\n";
    std::cout << "  双向映射这个需求，真的太常见了。\n";
    std::cout << "  早期我都是自己维护两个 map，insert 要插两个，\n";
    std::cout << "  delete 要删两个，一个漏了就是 bug。\n";
    std::cout << "  后来发现了 bimap，简直是救星。\n";
    std::cout << "\n";
    std::cout << "  bimap 不只是两个 map 拼起来那么简单。\n";
    std::cout << "  它的数据只存一份，两边的索引都指向同一份数据。\n";
    std::cout << "  内存和性能都比手写两个 map 要好。\n";
    std::cout << "\n";
    std::cout << "  几个实用建议：\n";
    std::cout << "  1️⃣ 能用标签就用标签，left/right 太容易搞混了\n";
    std::cout << "  2️⃣  两边查询频率不一样时，低频的一边可以用 vector_of 省内存\n";
    std::cout << "  3️⃣  如果一对多，用 multiset_of 或 unordered_multiset_of\n";
    std::cout << "  4️⃣  哈希查找更快的场景用 unordered_set_of\n";
    std::cout << "\n";
    std::cout << "  小小的一个库，解决大大的痛点。\n";
    std::cout << "  工具箱里多一件趁手的兵器，干活就更顺手。\n";
    std::cout << "============================================================\n";
}

// ============================================================================
// 小练习
// ============================================================================
void exercises() {
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "  📝 小练习\n";
    std::cout << "============================================================\n";
    std::cout << "\n";
    std::cout << "  1. 用 bimap 实现一个罗马数字转换器：\n";
    std::cout << "     支持整数 → 罗马数字（如 3→III, 4→IV）\n";
    std::cout << "     也支持罗马数字 → 整数（如 IX→9）\n";
    std::cout << "     （提示：1~10的基本映射即可）\n";
    std::cout << "\n";
    std::cout << "  2. 思考：bimap 和 std::map + std::unordered_map 组合，\n";
    std::cout << "     各自有什么优缺点？什么场景选哪个？\n";
    std::cout << "\n";
    std::cout << "  3. 用 bimap 实现一个简单的配置项映射：\n";
    std::cout << "     配置项的枚举值 ↔ 配置项的字符串名称\n";
    std::cout << "     （提示：枚举值在代码里用，字符串在配置文件里用）\n";
    std::cout << "============================================================\n";
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  Boost.Bimap 双向映射深度解析\n";
    std::cout << "  告别手写两个 map —— 双向查找一次搞定\n";
    std::cout << "============================================================\n";

    demo_basic_bimap();
    demo_traversal();
    demo_insert_erase();
    demo_collection_types();
    demo_tagged();
    demo_status_code_map();
    demo_translation_dict();
    demo_performance();

    laoshifu_says();
    exercises();

    return 0;
}
