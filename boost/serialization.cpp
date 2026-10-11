/**
 * Boost.Serialization 对象序列化 Demo
 *
 * 知识点覆盖：
 *   1. 基本序列化：POD 类型、自定义类的序列化
 *   2. 版本控制：serialization::version + load_construct_data
 *   3. 指针序列化：裸指针、智能指针（shared_ptr）的序列化
 *   4. 多态序列化：基类指针指向派生类对象的序列化
 *   5. STL 容器序列化：vector/map/set/string 等
 *   6. 三种归档格式：二进制（binary）/ XML / 文本（text）
 *   7. 对象追踪与别名：同一个对象被多次引用时的处理
 *   8. 非侵入式序列化：不修改类定义的序列化方式
 *
 * Boost 优势：
 *   - 支持多种归档格式（二进制/XML/文本/宽文本/便携二进制）
 *   - 自动处理指针、引用、STL 容器、智能指针
 *   - 支持多态序列化（通过 BOOST_CLASS_EXPORT）
 *   - 版本控制机制保证向后兼容
 *   - 对象追踪避免重复序列化同一对象
 *   - C++ 标准库中无对应功能
 *
 * 编译命令（示例）：
 *   g++ -std=c++17 serialization.cpp -o serialization -lboost_serialization
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <memory>
#include <fstream>
#include <sstream>
#include <cassert>
#include <typeinfo>

// ========== Boost 头文件说明 ==========
// <boost/serialization/serialization.hpp>   - 序列化核心
// <boost/serialization/access.hpp>          - 友元访问（侵入式序列化用）
// <boost/serialization/version.hpp>         - 版本控制
// <boost/serialization/base_object.hpp>     - 基类序列化
// <boost/serialization/export.hpp>          - 多态导出宏
// <boost/serialization/shared_ptr.hpp>      - shared_ptr 序列化支持
// <boost/serialization/vector.hpp>          - vector 序列化支持
// <boost/serialization/map.hpp>             - map 序列化支持
// <boost/serialization/string.hpp>          - string 序列化支持
// <boost/archive/text_oarchive.hpp>         - 文本输出归档
// <boost/archive/text_iarchive.hpp>         - 文本输入归档
// <boost/archive/binary_oarchive.hpp>       - 二进制输出归档
// <boost/archive/binary_iarchive.hpp>       - 二进制输入归档
// <boost/archive/xml_oarchive.hpp>          - XML 输出归档
// <boost/archive/xml_iarchive.hpp>          - XML 输入归档

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>   // name-value pair（用于 XML）
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

namespace ser = boost::serialization;

// ============================================================
// 一、基本序列化（侵入式方式）
// ============================================================
// 侵入式序列化：在类内部定义 serialize 成员函数，需要 friend access。
// 优点：可以访问私有成员，代码集中
// 缺点：修改了类定义，引入了 Boost 依赖

class Point {
public:
    Point() : x_(0), y_(0) {}
    Point(int x, int y) : x_(x), y_(y) {}

    int x() const { return x_; }
    int y() const { return y_; }

    void print(const std::string& name) const {
        std::cout << "  " << name << ": (" << x_ << ", " << y_ << ")" << std::endl;
    }

private:
    int x_;
    int y_;

    // 序列化函数：一个函数同时处理保存和加载
    // Archive 类型在保存时是 oarchive，加载时是 iarchive
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/) {
        ar & BOOST_SERIALIZATION_NVP(x_);  // & 操作符自动处理读/写
        ar & BOOST_SERIALIZATION_NVP(y_);  // NVP 宏给字段命名（用于 XML）
        // 等价于：
        // ar & x_;
        // ar & y_;
    }
};

// ============================================================
// 二、版本控制
// ============================================================
// 当类的定义随版本演化时，版本控制可以保证向后兼容。
// 使用 BOOST_CLASS_VERSION 宏指定类版本号，
// serialize 函数中的 version 参数用于区分不同版本的反序列化逻辑。

// 假设这是一个演化中的类：
// v0: 只有 name 和 age
// v1: 增加了 email 字段
class Person {
public:
    Person() : age_(0) {}
    Person(const std::string& name, int age, const std::string& email = "")
        : name_(name), age_(age), email_(email) {}

    void print() const {
        std::cout << "  Person: " << name_ << ", " << age_ << " 岁"
                  << (email_.empty() ? "" : ", " + email_) << std::endl;
    }

    const std::string& name() const { return name_; }
    int age() const { return age_; }
    const std::string& email() const { return email_; }

private:
    std::string name_;
    int age_;
    std::string email_;  // v1 新增字段

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & BOOST_SERIALIZATION_NVP(name_);
        ar & BOOST_SERIALIZATION_NVP(age_);

        // 只有版本 >= 1 时才序列化 email 字段
        if (version >= 1) {
            ar & BOOST_SERIALIZATION_NVP(email_);
        }
        // 加载旧版本时 email_ 保持默认值（空字符串）
    }
};

// 给 Person 类指定版本号为 1
BOOST_CLASS_VERSION(Person, 1)

// ============================================================
// 三、非侵入式序列化
// ============================================================
// 当无法修改类定义时（如第三方库类），使用非侵入式序列化。
// 在 boost::serialization 命名空间中特化 serialize 函数。

// 假设有一个不能修改的第三方类
struct Rectangle {
    int width;
    int height;
    Rectangle() : width(0), height(0) {}
    Rectangle(int w, int h) : width(w), height(h) {}
    int area() const { return width * height; }
};

// 非侵入式序列化：在 boost::serialization 命名空间中定义
namespace boost {
namespace serialization {

template <class Archive>
void serialize(Archive& ar, Rectangle& r, const unsigned int /*version*/) {
    ar & BOOST_SERIALIZATION_NVP(r.width);
    ar & BOOST_SERIALIZATION_NVP(r.height);
}

} // namespace serialization
} // namespace boost

// ============================================================
// 四、指针序列化与对象追踪
// ============================================================
// Boost.Serialization 自动处理指针的序列化：
//   - 同一对象被多次引用时，只序列化一次（对象追踪）
//   - 反序列化时正确恢复指针关系
//   - 支持 null 指针
//   - 支持 shared_ptr / weak_ptr 等智能指针

class Node {
public:
    Node() : value(0), next(nullptr) {}
    Node(int v) : value(v), next(nullptr) {}

    int value;
    Node* next;  // 裸指针

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/) {
        ar & BOOST_SERIALIZATION_NVP(value);
        ar & BOOST_SERIALIZATION_NVP(next);  // 自动处理指针追踪
    }
};

// ============================================================
// 五、多态序列化
// ============================================================
// 通过基类指针序列化派生类对象时，需要：
//   1. 基类有虚函数（至少虚析构函数）
//   2. 使用 base_object 序列化基类部分
//   3. 使用 BOOST_CLASS_EXPORT 注册派生类（用于多态创建）
//   4. 通过指针或引用序列化（不能按值）

class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;
    virtual std::string type() const = 0;
    virtual void print() const = 0;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& /*ar*/, const unsigned int /*version*/) {
        // 基类没有需要序列化的成员
    }
};

class Circle : public Shape {
public:
    Circle() : radius_(0) {}
    Circle(double r) : radius_(r) {}

    double area() const override { return 3.14159 * radius_ * radius_; }
    std::string type() const override { return "Circle"; }
    void print() const override {
        std::cout << "  Circle(r=" << radius_ << ", area=" << area() << ")" << std::endl;
    }

private:
    double radius_;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/) {
        // 必须先序列化基类
        ar & boost::serialization::base_object<Shape>(*this);
        ar & BOOST_SERIALIZATION_NVP(radius_);
    }
};

class RectangleShape : public Shape {
public:
    RectangleShape() : width_(0), height_(0) {}
    RectangleShape(double w, double h) : width_(w), height_(h) {}

    double area() const override { return width_ * height_; }
    std::string type() const override { return "Rectangle"; }
    void print() const override {
        std::cout << "  Rectangle(" << width_ << "x" << height_
                  << ", area=" << area() << ")" << std::endl;
    }

private:
    double width_;
    double height_;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/) {
        ar & boost::serialization::base_object<Shape>(*this);
        ar & BOOST_SERIALIZATION_NVP(width_);
        ar & BOOST_SERIALIZATION_NVP(height_);
    }
};

// 导出派生类，使得通过基类指针可以正确反序列化
BOOST_CLASS_EXPORT(Circle)
BOOST_CLASS_EXPORT(RectangleShape)

// 给基类一个 GUID（可选，但推荐用于多态序列化）
BOOST_CLASS_VERSION(Shape, 0)

// ============================================================
// 六、STL 容器序列化
// ============================================================
// Boost.Serialization 内置支持大部分 STL 容器，
// 只需包含对应的头文件即可自动序列化。

struct ContainerData {
    std::vector<int> int_vec;
    std::map<std::string, double> price_map;
    std::set<std::string> name_set;
    std::list<Point> point_list;
    std::string text;

    void print() const {
        std::cout << "  int_vec: [";
        for (size_t i = 0; i < int_vec.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << int_vec[i];
        }
        std::cout << "]" << std::endl;

        std::cout << "  price_map: ";
        for (const auto& [k, v] : price_map) {
            std::cout << k << "=" << v << " ";
        }
        std::cout << std::endl;

        std::cout << "  name_set: ";
        for (const auto& s : name_set) std::cout << s << " ";
        std::cout << std::endl;

        std::cout << "  point_list (" << point_list.size() << " 个点):" << std::endl;
        for (const auto& p : point_list) {
            std::cout << "    (" << p.x() << ", " << p.y() << ")" << std::endl;
        }

        std::cout << "  text: " << text << std::endl;
    }

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/) {
        ar & BOOST_SERIALIZATION_NVP(int_vec);
        ar & BOOST_SERIALIZATION_NVP(price_map);
        ar & BOOST_SERIALIZATION_NVP(name_set);
        ar & BOOST_SERIALIZATION_NVP(point_list);
        ar & BOOST_SERIALIZATION_NVP(text);
    }
};

// ============================================================
// 七、三种归档格式对比
// ============================================================

// 7.1 文本归档（text_archive）
// - 人类可读，便于调试
// - 体积较大，效率较低
// - 跨平台（不同编译器/系统都可互操作）
void text_archive_demo() {
    std::cout << "\n--- 1. 文本归档 (text_archive) ---" << std::endl;

    std::ostringstream oss;
    {
        boost::archive::text_oarchive oa(oss);
        Point p(10, 20);
        std::string msg = "Hello, Serialization!";
        oa << BOOST_SERIALIZATION_NVP(p);
        oa << BOOST_SERIALIZATION_NVP(msg);
    }

    std::string serialized = oss.str();
    std::cout << "  序列化结果 (" << serialized.size() << " 字节):" << std::endl;
    std::cout << "  " << serialized.substr(0, 100)
              << (serialized.size() > 100 ? "..." : "") << std::endl;

    // 反序列化
    {
        std::istringstream iss(serialized);
        boost::archive::text_iarchive ia(iss);
        Point p;
        std::string msg;
        ia >> BOOST_SERIALIZATION_NVP(p);
        ia >> BOOST_SERIALIZATION_NVP(msg);
        p.print("反序列化 Point");
        std::cout << "  反序列化 msg: " << msg << std::endl;
    }
}

// 7.2 二进制归档（binary_archive）
// - 体积最小，效率最高
// - 不可读
// - 注意：二进制归档不保证跨平台/跨编译器兼容
void binary_archive_demo() {
    std::cout << "\n--- 2. 二进制归档 (binary_archive) ---" << std::endl;

    std::ostringstream oss(std::ios::binary);
    {
        boost::archive::binary_oarchive oa(oss);
        ContainerData data;
        data.int_vec = {1, 2, 3, 4, 5};
        data.price_map = {{"apple", 5.5}, {"banana", 3.2}, {"cherry", 9.9}};
        data.name_set = {"Alice", "Bob", "Charlie"};
        data.point_list = {Point(1, 2), Point(3, 4), Point(5, 6)};
        data.text = "Binary archive demo";
        oa << BOOST_SERIALIZATION_NVP(data);
    }

    std::string serialized = oss.str();
    std::cout << "  二进制大小: " << serialized.size() << " 字节" << std::endl;

    // 反序列化
    {
        std::istringstream iss(serialized, std::ios::binary);
        boost::archive::binary_iarchive ia(iss);
        ContainerData data;
        ia >> BOOST_SERIALIZATION_NVP(data);
        std::cout << "  反序列化 ContainerData:" << std::endl;
        data.print();
    }
}

// 7.3 XML 归档（xml_archive）
// - 结构化、人类可读
// - 体积最大
// - 适合配置文件、数据交换等场景
// - 必须使用 NVP（name-value pair）宏
void xml_archive_demo() {
    std::cout << "\n--- 3. XML 归档 (xml_archive) ---" << std::endl;

    std::ostringstream oss;
    {
        boost::archive::xml_oarchive oa(oss);
        Person p("张三", 30, "zhangsan@example.com");
        oa << BOOST_SERIALIZATION_NVP(p);
    }

    std::string xml_str = oss.str();
    std::cout << "  XML 大小: " << xml_str.size() << " 字节" << std::endl;
    std::cout << "  XML 内容:" << std::endl;
    // 输出 XML 内容（缩进显示）
    std::istringstream iss(xml_str);
    std::string line;
    while (std::getline(iss, line)) {
        std::cout << "    " << line << std::endl;
    }

    // 反序列化
    {
        std::istringstream iss2(xml_str);
        boost::archive::xml_iarchive ia(iss2);
        Person p;
        ia >> BOOST_SERIALIZATION_NVP(p);
        std::cout << "  反序列化 Person:" << std::endl;
        p.print();
    }
}

// ============================================================
// 八、指针序列化与对象追踪演示
// ============================================================
void pointer_serialization_demo() {
    std::cout << "\n--- 4. 指针序列化与对象追踪 ---" << std::endl;

    // 构建一个链表：node1 -> node2 -> node3
    Node* node1 = new Node(1);
    Node* node2 = new Node(2);
    Node* node3 = new Node(3);
    node1->next = node2;
    node2->next = node3;

    // 同时有另一个指针也指向 node2（对象别名）
    Node* alias = node2;

    std::ostringstream oss;
    {
        boost::archive::text_oarchive oa(oss);
        // 序列化多个指针，Boost 会自动追踪同一对象
        oa << BOOST_SERIALIZATION_NVP(node1);
        oa << BOOST_SERIALIZATION_NVP(alias);  // 指向同一 node2，不会重复序列化
    }

    std::cout << "  序列化后大小: " << oss.str().size() << " 字节" << std::endl;

    // 反序列化
    Node* restored1 = nullptr;
    Node* restored_alias = nullptr;
    {
        std::istringstream iss(oss.str());
        boost::archive::text_iarchive ia(iss);
        ia >> BOOST_SERIALIZATION_NVP(restored1);
        ia >> BOOST_SERIALIZATION_NVP(restored_alias);
    }

    std::cout << "  反序列化后:" << std::endl;
    std::cout << "    node1 value = " << restored1->value << std::endl;
    std::cout << "    node1->next value = " << restored1->next->value << std::endl;
    std::cout << "    node1->next->next value = " << restored1->next->next->value << std::endl;
    std::cout << "    alias value = " << restored_alias->value << std::endl;
    std::cout << "    alias == node1->next: "
              << (restored_alias == restored1->next ? "是（同一对象）" : "否")
              << std::endl;

    // 清理
    // 注意：反序列化的指针需要手动删除
    // 实际项目中推荐使用 shared_ptr
    delete node1;  // 会连锁删除吗？不会，需要手动管理
    delete node2;
    delete node3;

    // 反序列化对象的清理：删除链表
    // 由于对象追踪，alias 和 node1->next 指向同一对象，不能重复 delete
    delete restored1->next->next;  // node3
    delete restored1->next;        // node2 (alias 也指向这里，但只删一次)
    delete restored1;              // node1
}

// ============================================================
// 九、多态序列化演示
// ============================================================
void polymorphic_demo() {
    std::cout << "\n--- 5. 多态序列化 ---" << std::endl;

    // 基类指针指向不同的派生类对象
    std::vector<std::shared_ptr<Shape>> shapes;
    shapes.push_back(std::make_shared<Circle>(5.0));
    shapes.push_back(std::make_shared<RectangleShape>(3.0, 4.0));
    shapes.push_back(std::make_shared<Circle>(2.5));

    std::cout << "  原始 shapes:" << std::endl;
    for (const auto& s : shapes) s->print();

    // 序列化（通过基类指针）
    std::ostringstream oss;
    {
        boost::archive::text_oarchive oa(oss);
        // 注意：必须通过指针或引用序列化才能触发多态
        oa << BOOST_SERIALIZATION_NVP(shapes);
    }

    std::cout << "  序列化大小: " << oss.str().size() << " 字节" << std::endl;

    // 反序列化
    std::vector<std::shared_ptr<Shape>> restored;
    {
        std::istringstream iss(oss.str());
        boost::archive::text_iarchive ia(iss);
        ia >> BOOST_SERIALIZATION_NVP(restored);
    }

    std::cout << "  反序列化后 shapes:" << std::endl;
    for (const auto& s : restored) {
        s->print();
        std::cout << "    实际类型: " << typeid(*s).name() << std::endl;
    }

    // 验证面积是否一致
    double total_area = 0, restored_area = 0;
    for (const auto& s : shapes) total_area += s->area();
    for (const auto& s : restored) restored_area += s->area();
    std::cout << "  总面积一致: " << (total_area == restored_area ? "是" : "否")
              << " (" << total_area << ")" << std::endl;
}

// ============================================================
// 十、shared_ptr 序列化
// ============================================================
void shared_ptr_demo() {
    std::cout << "\n--- 6. shared_ptr 序列化 ---" << std::endl;

    auto p1 = std::make_shared<Point>(100, 200);
    auto p2 = p1;  // 两个 shared_ptr 指向同一对象

    std::cout << "  p1.use_count = " << p1.use_count() << " (序列化前)" << std::endl;

    std::ostringstream oss;
    {
        boost::archive::text_oarchive oa(oss);
        oa << BOOST_SERIALIZATION_NVP(p1);
        oa << BOOST_SERIALIZATION_NVP(p2);
    }

    std::shared_ptr<Point> r1, r2;
    {
        std::istringstream iss(oss.str());
        boost::archive::text_iarchive ia(iss);
        ia >> BOOST_SERIALIZATION_NVP(r1);
        ia >> BOOST_SERIALIZATION_NVP(r2);
    }

    std::cout << "  r1.use_count = " << r1.use_count() << " (反序列化后)" << std::endl;
    std::cout << "  r1 和 r2 指向同一对象: "
              << (r1.get() == r2.get() ? "是" : "否") << std::endl;
    r1->print("r1");
    r2->print("r2");
}

// ============================================================
// Demo 入口函数
// ============================================================
int main() {
    std::cout << "========== Boost.Serialization 序列化 Demo ==========" << std::endl;

    // 1. 文本归档
    text_archive_demo();

    // 2. 二进制归档（含 STL 容器）
    binary_archive_demo();

    // 3. XML 归档（含版本控制）
    xml_archive_demo();

    // 4. 指针序列化与对象追踪
    pointer_serialization_demo();

    // 5. 多态序列化
    polymorphic_demo();

    // 6. shared_ptr 序列化
    shared_ptr_demo();

    // --------------------------------------------------------
    // 总结
    // --------------------------------------------------------
    std::cout << "\n--- Boost.Serialization 核心要点总结 ---" << std::endl;
    std::cout << "  1. 侵入式 vs 非侵入式：根据能否修改类定义选择" << std::endl;
    std::cout << "  2. serialize 函数同时处理保存和加载（& 操作符）" << std::endl;
    std::cout << "  3. 版本控制：BOOST_CLASS_VERSION + version 参数" << std::endl;
    std::cout << "  4. 对象追踪：同一对象被多次引用时自动去重" << std::endl;
    std::cout << "  5. 多态序列化：需要 BOOST_CLASS_EXPORT 宏注册派生类" << std::endl;
    std::cout << "  6. STL 容器：包含对应头文件即可自动支持" << std::endl;
    std::cout << "  7. 三种归档格式：" << std::endl;
    std::cout << "     - 二进制：体积小、速度快，不跨平台" << std::endl;
    std::cout << "     - 文本：可读，跨平台，体积中等" << std::endl;
    std::cout << "     - XML：结构化，体积大，适合配置/数据交换" << std::endl;
    std::cout << "  8. 推荐用 shared_ptr 管理序列化对象的生命周期" << std::endl;

    std::cout << "\n========== Demo 结束 ==========" << std::endl;
    return 0;
}
