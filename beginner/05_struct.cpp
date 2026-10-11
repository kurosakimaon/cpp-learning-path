// ============================================================
// 05_struct.cpp - 结构体
// 编译命令：g++ -std=c++17 05_struct.cpp -o 05_struct
// ============================================================

#include <iostream>
#include <string>
using namespace std;

// ============================================================
// 一、结构体的定义与使用
// ============================================================
// 老师傅比喻：结构体就像一个"工具箱"，把不同类型的东西装在一起。
// 比如描述一个学生，需要姓名、年龄、成绩，这些数据类型不一样，
// 但都属于同一个学生，这时候就用结构体把它们打包起来。

// 定义一个学生结构体
struct Student {
    string name;    // 姓名
    int age;        // 年龄
    double score;   // 成绩
    char grade;     // 等级
};
// 老师傅提醒：struct定义最后有个分号，别漏了！

// 定义一个点的结构体
struct Point {
    double x;
    double y;
};

// ============================================================
// 二、结构体嵌套
// ============================================================
// 结构体里面还可以包含别的结构体

struct Date {
    int year;
    int month;
    int day;
};

struct Employee {
    string name;
    int id;
    Date birthday;   // 嵌套的日期结构体
    double salary;
};

// ============================================================
// 三、typedef 与 using
// ============================================================
// 老师傅说：结构体类型名太长，typedef可以给它起个别名，用起来方便

// C语言风格：typedef
typedef struct {
    double width;
    double height;
} Rect;  // 现在 Rect 就是这个结构体类型的名字了

// C++11 风格：using（更清晰，推荐）
using Rectangle = struct {
    double width;
    double height;
};
// 其实在C++里，struct定义后直接用名字就行，typedef不是必须的
// 但给类型起别名在复杂场景下很有用

// ============================================================
// 四、结构体作函数参数
// ============================================================

// 4.1 值传递（会拷贝整个结构体，大结构体效率低）
void printStudent(Student s)
{
    cout << "姓名：" << s.name
         << "，年龄：" << s.age
         << "，成绩：" << s.score
         << "，等级：" << s.grade << endl;
}

// 4.2 引用传递（推荐！不用拷贝，效率高）
// 加const保证不修改原数据
void printStudentRef(const Student &s)
{
    // s.age = 100;  // 错误！const引用不能改
    cout << "姓名：" << s.name
         << "，年龄：" << s.age
         << "，成绩：" << s.score
         << "，等级：" << s.grade << endl;
}

// 4.3 指针传递
void printStudentPtr(const Student *s)
{
    cout << "姓名：" << s->name  // 结构体指针用 -> 访问成员
         << "，年龄：" << s->age
         << "，成绩：" << s->score
         << "，等级：" << s->grade << endl;
}

// 修改结构体的函数（用引用传递最方便）
void upgradeStudent(Student &s)
{
    if (s.grade < 'D') {
        s.grade--;  // 等级提升：B变A，C变B
        s.score += 5;  // 加5分
    }
}

// ============================================================
// 主函数
// ============================================================
int main()
{
    cout << "===== 1. 结构体的定义与使用 =====" << endl;

    // 创建结构体变量并初始化
    Student s1;
    s1.name = "张三";
    s1.age = 18;
    s1.score = 92.5;
    s1.grade = 'A';

    // 另一种初始化方式（C++11列表初始化）
    Student s2 = {"李四", 19, 85.0, 'B'};

    // C++17 结构化绑定（解构）
    auto [name, age, score, grade] = s2;
    // 直接把结构体成员拆出来赋值给单独的变量
    cout << "结构化绑定：name=" << name << ", age=" << age << endl;

    cout << "学生1：";
    printStudent(s1);
    cout << "学生2：";
    printStudent(s2);

    // 结构体变量之间可以直接赋值（逐个成员拷贝）
    Student s3 = s1;
    s3.name = "王五";
    cout << "学生3（拷贝s1后改名）：";
    printStudent(s3);

    // 结构体大小（内存对齐，可能比你算的大一点）
    cout << "Student结构体大小：" << sizeof(Student) << "字节" << endl;
    cout << endl;

    // ============================================================
    // 五、结构体数组
    // ============================================================
    cout << "===== 2. 结构体数组 =====" << endl;
    // 老师傅比喻：结构体数组就像一个班级花名册，每行是一个学生的信息

    // 定义并初始化结构体数组
    Student class1[] = {
        {"小明", 17, 88.5, 'B'},
        {"小红", 18, 95.0, 'A'},
        {"小刚", 17, 76.0, 'C'},
        {"小丽", 18, 92.0, 'A'},
        {"小强", 17, 68.0, 'D'}
    };

    int classSize = sizeof(class1) / sizeof(class1[0]);
    cout << "班级人数：" << classSize << endl;
    cout << "班级名单：" << endl;
    for (int i = 0; i < classSize; i++) {
        cout << "  " << (i + 1) << ". ";
        printStudentRef(class1[i]);
    }

    // 计算班级平均分
    double totalScore = 0;
    for (int i = 0; i < classSize; i++) {
        totalScore += class1[i].score;
    }
    cout << "班级平均分：" << (totalScore / classSize) << endl;

    cout << endl;

    // ============================================================
    // 六、结构体指针
    // ============================================================
    cout << "===== 3. 结构体指针 =====" << endl;

    Student stu = {"赵六", 20, 78.5, 'C'};
    Student *pStu = &stu;  // 结构体指针，指向stu

    // 访问成员的两种方式
    cout << "用 (*pStu).name 访问：" << (*pStu).name << endl;
    // 老师傅说：*pStu外面的括号不能省，因为.比*优先级高
    // *pStu.name 会被理解成 *(pStu.name)，那就错了

    cout << "用 pStu->name 访问：" << pStu->name << "（更常用，-> 是专门的运算符）" << endl;
    // -> 就是为结构体指针设计的，左边是指针，右边是成员名

    pStu->score = 85.0;  // 通过指针修改成员
    pStu->grade = 'B';
    cout << "通过指针修改后：";
    printStudentPtr(pStu);

    cout << endl;

    // ============================================================
    // 结构体嵌套
    // ============================================================
    cout << "===== 4. 结构体嵌套 =====" << endl;

    Employee emp;
    emp.name = "孙七";
    emp.id = 1001;
    emp.birthday.year = 1995;   // 嵌套成员用 . 一层一层点进去
    emp.birthday.month = 6;
    emp.birthday.day = 15;
    emp.salary = 8000.0;

    cout << "员工信息：" << endl;
    cout << "  姓名：" << emp.name << endl;
    cout << "  工号：" << emp.id << endl;
    cout << "  生日：" << emp.birthday.year << "年"
         << emp.birthday.month << "月"
         << emp.birthday.day << "日" << endl;
    cout << "  工资：" << emp.salary << "元" << endl;

    cout << endl;

    // ============================================================
    // typedef 与 using
    // ============================================================
    cout << "===== 5. typedef 与 using =====" << endl;

    Rect r1 = {10.0, 20.0};  // typedef定义的类型别名
    Rectangle r2 = {5.0, 8.0};  // using定义的类型别名

    cout << "矩形r1：宽=" << r1.width << "，高=" << r1.height
         << "，面积=" << r1.width * r1.height << endl;
    cout << "矩形r2：宽=" << r2.width << "，高=" << r2.height
         << "，面积=" << r2.width * r2.height << endl;

    cout << "typedef 和 using 都可以给类型起别名" << endl;
    cout << "C++推荐用 using，语法更清晰，还能用于模板别名" << endl;

    cout << endl;

    // ============================================================
    // 综合练习演示：学生管理系统
    // ============================================================
    cout << "===== 6. 综合演示：学生管理系统 =====" << endl;

    const int MAX_STUDENTS = 50;
    Student students[MAX_STUDENTS];
    int stuCount = 0;

    // 添加几个学生
    students[stuCount++] = {"张三", 18, 85.5, 'B'};
    students[stuCount++] = {"李四", 19, 92.0, 'A'};
    students[stuCount++] = {"王五", 18, 78.0, 'C'};

    cout << "当前学生列表：" << endl;
    for (int i = 0; i < stuCount; i++) {
        cout << "  [" << i << "] ";
        printStudentRef(students[i]);
    }

    // 给所有学生升级
    cout << endl << "全员升级中..." << endl;
    for (int i = 0; i < stuCount; i++) {
        upgradeStudent(students[i]);
    }

    cout << "升级后：" << endl;
    for (int i = 0; i < stuCount; i++) {
        cout << "  [" << i << "] ";
        printStudentRef(students[i]);
    }

    // 查找学生
    cout << endl << "查找\"李四\"：" << endl;
    bool found = false;
    for (int i = 0; i < stuCount; i++) {
        if (students[i].name == "李四") {
            cout << "  找到了！";
            printStudentRef(students[i]);
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "  没找到这个学生" << endl;
    }

    cout << endl;

    // ============================================================
    // 结构体与类的关系（预告面向对象）
    // ============================================================
    cout << "===== 7. 结构体与类的关系 =====" << endl;
    cout << "老师傅说：C++里struct和class其实很像，区别主要有两点：" << endl;
    cout << endl;
    cout << "  1. 默认访问权限不同：" << endl;
    cout << "     - struct 的成员默认是 public（公有的，谁都能访问）" << endl;
    cout << "     - class 的成员默认是 private（私有的，外面不能碰）" << endl;
    cout << endl;
    cout << "  2. 继承时默认权限不同：" << endl;
    cout << "     - struct 默认 public 继承" << endl;
    cout << "     - class 默认 private 继承" << endl;
    cout << endl;
    cout << "  什么时候用 struct，什么时候用 class？" << endl;
    cout << "    - 只是打包数据，没有什么方法 -> 用 struct" << endl;
    cout << "    - 有封装、继承、多态这些面向对象的需求 -> 用 class" << endl;
    cout << endl;
    cout << "  结构体是面向对象编程的入门，学会了struct，" << endl;
    cout << "  后面学class就水到渠成了。" << endl;

    cout << endl;

    // ============================================================
    // 老师傅的话
    // ============================================================
    cout << "========================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "========================================" << endl;
    cout << "  1. 结构体是C语言就有的东西，C++把它发扬光大了" << endl;
    cout << "  2. 结构体把相关数据打包在一起，代码更清晰" << endl;
    cout << "  3. 传结构体优先用引用（加const），避免拷贝开销" << endl;
    cout << "  4. 结构体指针用 -> 访问成员，比 (*p). 方便" << endl;
    cout << "  5. C++的struct不止能放数据，还能放函数（成员函数）" << endl;
    cout << "     这就已经是类的雏形了" << endl;
    cout << "  6. 结构体对齐会有内存填充，大小不一定是成员之和" << endl;
    cout << "     不过初学者不用太纠结这个，知道有这回事就行" << endl;
    cout << "========================================" << endl;
    cout << endl;

    // ============================================================
    // 小练习
    // ============================================================
    cout << "========================================" << endl;
    cout << "  小练习（动手写写看）" << endl;
    cout << "========================================" << endl;
    cout << "  1. 定义一个Book结构体，包含书名、作者、价格、出版年份" << endl;
    cout << "     然后创建几本书的数组，输出价格最高的那本书" << endl;
    cout << "  2. 定义一个Point结构体表示点，写一个函数计算两点之间的距离" << endl;
    cout << "  3. 定义一个Date结构体，写一个函数判断两个日期哪个更早" << endl;
    cout << "  4. 完善学生管理系统：" << endl;
    cout << "     - 添加学生" << endl;
    cout << "     - 删除学生（按下标删）" << endl;
    cout << "     - 按成绩排序" << endl;
    cout << "     - 按姓名查找" << endl;
    cout << "  5. 定义一个链表节点结构体（数据 + next指针），" << endl;
    cout << "     尝试实现一个简单的单链表（插入、遍历）" << endl;
    cout << "========================================" << endl;

    return 0;
}
