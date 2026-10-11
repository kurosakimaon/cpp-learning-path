/**
 * @file datetime_demo.cpp
 * @brief Boost.DateTime 日期时间库深度解析
 *
 * Boost.DateTime 提供了完整的日期时间处理能力，包括：
 * 日期计算、时间计算、时区、格式化、时间段等。
 *
 * 虽然 C++20 引入了 <chrono> 的日历和时区支持，
 * 但在 C++17 及之前，Boost.DateTime 是日期时间处理的首选。
 * 而且很多老项目还在大量使用它。
 *
 * 主要内容：
 * 1. 日期（Date）：创建、计算、格式化
 * 2. 时间（Time Duration）：时间段
 * 3. 日期时间（Ptime）：日期+时间
 * 4. 时间间隔（Time Period）：时间段判断
 * 5. 格式化与解析
 * 6. 本地化与时区
 * 7. 实际应用：倒计时、工作日计算、年龄计算
 *
 * 老师傅的话：
 *   日期时间处理看着简单，坑多得很。
 *   闰年、时区、夏令时、月份天数不一样……
 *   自己写日期计算，早晚出 bug。用库！
 *   Boost.DateTime 虽然接口有点啰嗦，但功能全面，值得掌握。
 *
 * 编译命令: g++ -std=c++17 datetime_demo.cpp -lboost_date_time
 *
 * 头文件：<boost/date_time/gregorian/gregorian.hpp>
 *        <boost/date_time/posix_time/posix_time.hpp>
 *        <boost/date_time/local_time/local_time.hpp>
 */

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>

// Boost.DateTime 头文件
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>

namespace bg = boost::gregorian;
namespace bpt = boost::posix_time;
namespace blt = boost::local_time;

// ============================================================================
// 1. 日期（Date）基础
// ============================================================================
void demo_date_basics() {
    std::cout << "\n===== 1. 日期基础 =====\n";

    // 创建日期
    bg::date d1(2024, 1, 1);         // 直接指定年月日
    bg::date d2 = bg::from_string("2024-06-15");  // 从字符串解析
    bg::date d3 = bg::from_uk_string("15/06/2024"); // 英式格式
    bg::date d4 = bg::from_us_string("06/15/2024"); // 美式格式
    bg::date today = bg::day_clock::local_day();     // 今天（本地时间）
    bg::date utc_today = bg::day_clock::universal_day(); // 今天（UTC）

    std::cout << "  2024年元旦: " << d1 << "\n";
    std::cout << "  字符串解析: " << d2 << "\n";
    std::cout << "  英式格式:   " << d3 << "\n";
    std::cout << "  美式格式:   " << d4 << "\n";
    std::cout << "  今天（本地）: " << today << "\n";
    std::cout << "  今天（UTC）: " << utc_today << "\n";

    // 访问年/月/日
    std::cout << "\n  日期拆解:\n";
    std::cout << "    年: " << d2.year() << "\n";
    std::cout << "    月: " << d2.month() << " (数字: "
              << d2.month().as_number() << ")\n";
    std::cout << "    日: " << d2.day() << "\n";
    std::cout << "    一年中第几天: " << d2.day_of_year() << "\n";
    std::cout << "    星期几: " << d2.day_of_week()
              << " (周" << d2.day_of_week().as_number() << ")\n";
    std::cout << "    一年中第几周: " << d2.week_number() << "\n";
    std::cout << "    该月最后一天: " << d2.end_of_month() << "\n";

    // 特殊日期
    bg::date neg_inf = bg::date(bg::neg_infin);   // 负无穷
    bg::date pos_inf = bg::date(bg::pos_infin);   // 正无穷
    bg::date not_a_date = bg::date(bg::not_a_date_time); // 无效日期
    std::cout << "\n  特殊日期:\n";
    std::cout << "    负无穷: " << neg_inf << "\n";
    std::cout << "    正无穷: " << pos_inf << "\n";
    std::cout << "    无效日期: " << not_a_date << "\n";

    // 日期有效性检查
    std::cout << "\n  有效性检查:\n";
    bg::date invalid(2024, 2, 30);  // 2月没有30号
    std::cout << "    2024-02-30 有效吗? "
              << std::boolalpha << invalid.is_not_a_date()
              << "（is_not_a_date 为 true 表示无效）\n";

    std::cout << "\n  常用构造方式：\n";
    std::cout << "    date(y, m, d)       — 直接指定\n";
    std::cout << "    from_string(str)    — ISO 格式\n";
    std::cout << "    day_clock::local_day() — 今天\n";
}

// ============================================================================
// 2. 日期计算
// ============================================================================
void demo_date_calculation() {
    std::cout << "\n===== 2. 日期计算 =====\n";

    bg::date start(2024, 1, 1);
    bg::date end(2024, 12, 31);

    // 日期差
    bg::date_duration diff = end - start;
    std::cout << "  " << start << " 到 " << end
              << " 相差 " << diff.days() << " 天\n";

    // 加减天数
    bg::date d = bg::from_string("2024-06-15");
    std::cout << "\n  基准日期: " << d << "\n";
    std::cout << "    加 10 天: " << d + bg::days(10) << "\n";
    std::cout << "    减 15 天: " << d - bg::days(15) << "\n";
    std::cout << "    加 3 周:   " << d + bg::weeks(3) << "\n";
    std::cout << "    加 2 月:   " << d + bg::months(2) << "\n";
    std::cout << "    加 1 年:   " << d + bg::years(1) << "\n";

    // 注意：月份加减会做月末调整
    bg::date jan31(2024, 1, 31);
    std::cout << "\n  月末调整（重要！）:\n";
    std::cout << "    " << jan31 << " + 1 个月 = "
              << jan31 + bg::months(1)
              << "（2月没有31号，自动调整到月末）\n";

    bg::date feb29(2024, 2, 29);  // 闰年
    std::cout << "    " << feb29 << " + 1 年 = "
              << feb29 + bg::years(1)
              << "（2025不是闰年，调整到28号）\n";

    // 日期比较
    std::cout << "\n  日期比较:\n";
    std::cout << "    " << start << " < " << end << " : "
              << std::boolalpha << (start < end) << "\n";
    std::cout << "    " << start << " == " << end << " : "
              << (start == end) << "\n";

    // 闰年判断
    std::cout << "\n  闰年判断:\n";
    std::cout << "    2024 是闰年吗? "
              << bg::gregorian_calendar::is_leap_year(2024) << "\n";
    std::cout << "    2023 是闰年吗? "
              << bg::gregorian_calendar::is_leap_year(2023) << "\n";
    std::cout << "    2000 是闰年吗? "
              << bg::gregorian_calendar::is_leap_year(2000) << "\n";
    std::cout << "    1900 是闰年吗? "
              << bg::gregorian_calendar::is_leap_year(1900)
              << "（整百年要被400整除才是闰年）\n";

    // 月份天数
    std::cout << "\n  2024年2月有 "
              << bg::gregorian_calendar::end_of_month_day(2024, 2)
              << " 天\n";
    std::cout << "  2023年2月有 "
              << bg::gregorian_calendar::end_of_month_day(2023, 2)
              << " 天\n";
}

// ============================================================================
// 3. 时间段（Time Duration）
// ============================================================================
void demo_time_duration() {
    std::cout << "\n===== 3. 时间段 =====\n";

    // 创建时间段
    bpt::time_duration t1 = bpt::hours(2) + bpt::minutes(30) + bpt::seconds(45);
    bpt::time_duration t2 = bpt::time_duration(1, 45, 30);  // 时:分:秒
    bpt::time_duration t3 = bpt::duration_from_string("03:15:20"); // 字符串

    std::cout << "  t1 = " << t1 << "（2小时30分45秒）\n";
    std::cout << "  t2 = " << t2 << "（1小时45分30秒）\n";
    std::cout << "  t3 = " << t3 << "（3小时15分20秒）\n";

    // 访问各部分
    std::cout << "\n  t1 拆解:\n";
    std::cout << "    小时: " << t1.hours() << "\n";
    std::cout << "    分钟: " << t1.minutes() << "\n";
    std::cout << "    秒:   " << t1.seconds() << "\n";
    std::cout << "    总秒数: " << t1.total_seconds() << "\n";
    std::cout << "    总毫秒: " << t1.total_milliseconds() << "\n";

    // 运算
    std::cout << "\n  运算:\n";
    std::cout << "    t1 + t2 = " << t1 + t2 << "\n";
    std::cout << "    t1 - t2 = " << t1 - t2 << "\n";
    std::cout << "    t1 * 2  = " << t1 * 2 << "\n";
    std::cout << "    t1 / 2  = " << t1 / 2 << "\n";

    // 微秒/纳秒精度
    bpt::time_duration precise = bpt::microsec(123456);
    std::cout << "\n  微秒精度: " << precise << "\n";
    std::cout << "    总微秒数: " << precise.total_microseconds() << "\n";

    // 特殊值
    bpt::time_duration pos_inf = bpt::pos_infin;
    bpt::time_duration neg_inf = bpt::neg_infin;
    std::cout << "\n  特殊值:\n";
    std::cout << "    正无穷: " << pos_inf << "\n";
    std::cout << "    负无穷: " << neg_inf << "\n";

    std::cout << "\n  注意：time_duration 可以是负数\n";
    bpt::time_duration neg = bpt::hours(-5);
    std::cout << "    负数时间段: " << neg << "\n";
}

// ============================================================================
// 4. 日期时间（Ptime）
// ============================================================================
void demo_ptime() {
    std::cout << "\n===== 4. 日期时间（Ptime）=====\n";

    // 创建日期时间
    bpt::ptime t1(bg::date(2024, 6, 15), bpt::hours(14) + bpt::minutes(30) + bpt::seconds(0));
    bpt::ptime t2 = bpt::time_from_string("2024-06-15 14:30:00");
    bpt::ptime t3 = bpt::from_iso_string("20240615T143000");

    std::cout << "  t1 = " << t1 << "\n";
    std::cout << "  t2 = " << t2 << "\n";
    std::cout << "  t3 (ISO格式) = " << t3 << "\n";

    // 当前时间
    bpt::ptime now = bpt::second_clock::local_time();
    bpt::ptime utc_now = bpt::second_clock::universal_time();
    bpt::ptime now_micro = bpt::microsec_clock::local_time();

    std::cout << "\n  当前时间（本地，秒精度）: " << now << "\n";
    std::cout << "  当前时间（UTC，秒精度）:  " << utc_now << "\n";
    std::cout << "  当前时间（本地，微秒精度）: " << now_micro << "\n";

    // 访问日期和时间部分
    bg::date d = t1.date();
    bpt::time_duration td = t1.time_of_day();
    std::cout << "\n  拆解 t1:\n";
    std::cout << "    日期部分: " << d << "\n";
    std::cout << "    时间部分: " << td << "\n";

    // 日期时间的运算
    std::cout << "\n  运算:\n";
    std::cout << "    t1 + 2小时 = " << t1 + bpt::hours(2) << "\n";
    std::cout << "    t1 - 30分钟 = " << t1 - bpt::minutes(30) << "\n";
    std::cout << "    t1 + 1天 = " << t1 + bg::days(1) << "\n";

    // 两个时间的差
    bpt::ptime start = bpt::time_from_string("2024-01-01 00:00:00");
    bpt::ptime finish = bpt::time_from_string("2024-01-02 12:30:45");
    bpt::time_duration delta = finish - start;
    std::cout << "\n  " << start << " 到 " << finish << "\n";
    std::cout << "    相差 " << delta.hours() << " 小时 "
              << delta.minutes() << " 分 "
              << delta.seconds() << " 秒\n";
    std::cout << "    共 " << delta.total_seconds() << " 秒\n";
}

// ============================================================================
// 5. 格式化输出
// ============================================================================
void demo_formatting() {
    std::cout << "\n===== 5. 格式化输出 =====\n";

    bpt::ptime now = bpt::second_clock::local_time();

    // 默认格式
    std::cout << "  默认格式: " << now << "\n";

    // 使用 time_facet 自定义格式
    // 常用格式符：
    //   %Y - 四位年  %m - 月  %d - 日
    //   %H - 时(24) %M - 分  %S - 秒
    //   %a - 周几缩写 %A - 周几全称
    //   %b - 月缩写   %B - 月全称
    //   %j - 一年中第几天
    //   %W - 一年中第几周

    // 注意：Boost.DateTime 的格式化比较特殊，要用 facet
    // 这里演示常用的格式

    // 日期格式化
    bg::date today = now.date();
    std::cout << "\n  日期格式化:\n";
    std::cout << "    默认:        " << today << "\n";
    std::cout << "    ISO 格式:    " << bg::to_iso_string(today) << "\n";
    std::cout << "    简单格式:    " << bg::to_simple_string(today) << "\n";

    // 时间格式化
    bpt::time_duration td = now.time_of_day();
    std::cout << "\n  时间格式化:\n";
    std::cout << "    默认:        " << td << "\n";
    std::cout << "    简化:        " << bpt::to_simple_string(td) << "\n";

    // ptime 格式化
    std::cout << "\n  日期时间格式化:\n";
    std::cout << "    默认:        " << now << "\n";
    std::cout << "    ISO 格式:    " << bpt::to_iso_string(now) << "\n";
    std::cout << "    ISO 扩展:    " << bpt::to_iso_extended_string(now) << "\n";
    std::cout << "    简单格式:    " << bpt::to_simple_string(now) << "\n";

    // 自定义格式（使用 stringstream + facet）
    std::ostringstream oss;
    auto* facet = new bpt::time_facet("%Y年%m月%d日 %H:%M:%S");
    oss.imbue(std::locale(oss.getloc(), facet));
    oss << now;
    std::cout << "    中文格式:    " << oss.str() << "\n";

    std::cout << "\n  格式化常用场景：\n";
    std::cout << "    - 日志输出（统一时间格式）\n";
    std::cout << "    - 文件命名（带时间戳）\n";
    std::cout << "    - 接口传参（ISO格式）\n";
    std::cout << "    - 界面显示（本地化格式）\n";
}

// ============================================================================
// 6. 时间段（Time Period）
// ============================================================================
void demo_time_period() {
    std::cout << "\n===== 6. 时间段（Time Period）=====\n";

    // 定义时间段
    bpt::ptime start(bg::date(2024, 6, 1), bpt::hours(9));
    bpt::ptime end(bg::date(2024, 6, 1), bpt::hours(18));
    bpt::time_period work_hours(start, end);  // 上午9点到下午6点

    std::cout << "  工作时间: " << work_hours << "\n";
    std::cout << "    开始: " << work_hours.begin() << "\n";
    std::cout << "    结束: " << work_hours.end() << "\n";
    std::cout << "    时长: " << work_hours.length().total_hours() << " 小时\n";

    // 包含关系
    bpt::ptime morning(bg::date(2024, 6, 1), bpt::hours(10));
    bpt::ptime evening(bg::date(2024, 6, 1), bpt::hours(20));

    std::cout << "\n  包含关系:\n";
    std::cout << "    10点在工作时间吗? "
              << std::boolalpha << work_hours.contains(morning) << "\n";
    std::cout << "    20点在工作时间吗? "
              << work_hours.contains(evening) << "\n";

    // 两个时间段的关系
    bpt::time_period meeting(
        bpt::ptime(bg::date(2024, 6, 1), bpt::hours(14)),
        bpt::ptime(bg::date(2024, 6, 1), bpt::hours(16))
    );
    bpt::time_period overtime(
        bpt::ptime(bg::date(2024, 6, 1), bpt::hours(17)),
        bpt::ptime(bg::date(2024, 6, 1), bpt::hours(21))
    );

    std::cout << "\n  时间段关系:\n";
    std::cout << "    会议时间: " << meeting << "\n";
    std::cout << "    加班时间: " << overtime << "\n";
    std::cout << "    会议在工作时间内? "
              << work_hours.contains(meeting) << "\n";
    std::cout << "    加班和工作时间相交? "
              << work_hours.intersects(overtime) << "\n";

    bpt::time_period intersection = work_hours.intersection(overtime);
    std::cout << "    交集: " << intersection
              << "（" << intersection.length().total_hours() << "小时）\n";

    // 日期时间段也一样
    bg::date_period vacation(bg::date(2024, 10, 1), bg::date(2024, 10, 8));
    std::cout << "\n  日期时间段（假期）: " << vacation << "\n";
    std::cout << "    共 " << vacation.length().days() << " 天\n";
}

// ============================================================================
// 7. 时区
// ============================================================================
void demo_timezone() {
    std::cout << "\n===== 7. 时区 =====\n";

    // 注意：Boost.DateTime 的时区功能需要时区数据库
    // 这里演示基本概念，实际使用需要加载时区数据文件

    // 简单演示：UTC 时间
    bpt::ptime utc_now = bpt::second_clock::universal_time();
    bpt::ptime local_now = bpt::second_clock::local_time();

    std::cout << "  UTC 时间:    " << utc_now << "\n";
    std::cout << "  本地时间:    " << local_now << "\n";

    bpt::time_duration diff = local_now - utc_now;
    std::cout << "  时差: " << diff.hours() << " 小时\n";
    std::cout << "  （中国是 UTC+8，所以本地时间比UTC早8小时）\n";

    // local_time 库的完整用法需要 tz_database
    // 基本思路：
    //   1. 加载时区数据库
    //   2. 创建不同时区的时间
    //   3. 在时区间转换
    //
    // 示例伪代码：
    //   blt::tz_database tz_db;
    //   tz_db.load_from_file("date_time_zonespec.csv");
    //   blt::time_zone_ptr sh_tz = tz_db.time_zone_from_region("Asia/Shanghai");
    //   blt::time_zone_ptr ny_tz = tz_db.time_zone_from_region("America/New_York");
    //   blt::local_date_time sh_time(... , sh_tz);
    //   blt::local_date_time ny_time = sh_time.local_time_in(ny_tz);

    std::cout << "\n  时区处理是个大坑：\n";
    std::cout << "    - 夏令时：部分国家一年变两次时间\n";
    std::cout << "    - 时区数据库：需要定期更新（政策会变）\n";
    std::cout << "    - 历史时间：时区规则可能和现在不一样\n";
    std::cout << "  建议：服务端统一用 UTC，客户端才转本地时间\n";
}

// ============================================================================
// 8. 实际应用场景
// ============================================================================
namespace practical {

// 计算年龄
int calculate_age(bg::date birth_date, bg::date today = bg::day_clock::local_day()) {
    int age = today.year() - birth_date.year();
    // 如果今年生日还没到，减一岁
    if (birth_date.month() > today.month() ||
        (birth_date.month() == today.month() && birth_date.day() > today.day())) {
        age--;
    }
    return age;
}

// 计算两个日期之间的工作日（排除周末）
int count_workdays(bg::date start, bg::date end) {
    int count = 0;
    for (bg::day_iterator it(start); it <= end; ++it) {
        bg::date d = *it;
        // 0=周日, 6=周六... 实际上 0=周一, 6=周日
        // Boost中 day_of_week: Monday=0, Sunday=6
        if (d.day_of_week() < 5) {  // 周一到周五
            count++;
        }
    }
    return count;
}

// 获取下个月的第一天
bg::date first_day_of_next_month(bg::date d) {
    if (d.month() == 12) {
        return bg::date(d.year() + 1, 1, 1);
    }
    return bg::date(d.year(), d.month() + 1, 1);
}

// 倒计时
std::string count_down(bg::date target) {
    bg::date today = bg::day_clock::local_day();
    if (target < today) {
        return "已过";
    }
    bg::date_duration diff = target - today;
    return "还有 " + std::to_string(diff.days()) + " 天";
}

} // namespace practical

void demo_practical() {
    std::cout << "\n===== 8. 实际应用场景 =====\n";

    // 年龄计算
    bg::date birth(1995, 6, 15);
    int age = practical::calculate_age(birth);
    std::cout << "  出生日期: " << birth << "\n";
    std::cout << "  年龄: " << age << " 岁\n";

    // 工作日计算
    bg::date start(2024, 6, 1);
    bg::date end(2024, 6, 30);
    int workdays = practical::count_workdays(start, end);
    std::cout << "\n  " << start << " 到 " << end << "\n";
    std::cout << "    工作日: " << workdays << " 天\n";
    std::cout << "    周末: " << (end - start).days() + 1 - workdays << " 天\n";

    // 下个月第一天
    bg::date today = bg::day_clock::local_day();
    std::cout << "\n  今天: " << today << "\n";
    std::cout << "  下个月第一天: "
              << practical::first_day_of_next_month(today) << "\n";

    // 倒计时
    bg::date new_year(today.year() + 1, 1, 1);
    std::cout << "\n  距离新年 " << new_year << ":\n";
    std::cout << "    " << practical::count_down(new_year) << "\n";

    // 程序运行时间统计
    std::cout << "\n  程序运行时间统计（性能测试常用）:\n";
    bpt::ptime start_time = bpt::microsec_clock::local_time();

    // 模拟一些工作
    volatile int sum = 0;
    for (int i = 0; i < 1000000; i++) {
        sum += i;
    }

    bpt::ptime end_time = bpt::microsec_clock::local_time();
    bpt::time_duration elapsed = end_time - start_time;
    std::cout << "    循环 100 万次耗时: "
              << elapsed.total_microseconds() << " 微秒\n";
    (void)sum;  // 避免警告

    std::cout << "\n  这些都是工作中天天要写的代码，掌握了效率翻倍\n";
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
    std::cout << "  日期时间这玩意儿，看着简单，写起来全是坑。\n";
    std::cout << "  我年轻的时候也自己写过日期计算，结果闰年算错了，\n";
    std::cout << "  线上出了bug，查了半天才找到原因，丢人。\n";
    std::cout << "\n";
    std::cout << "  记住：永远不要自己写日期计算逻辑！\n";
    std::cout << "  闰年、大小月、夏令时、时区……你考虑不全的。\n";
    std::cout << "  用库，用经过千锤百炼的库。\n";
    std::cout << "\n";
    std::cout << "  Boost.DateTime 虽然 API 设计得有点啰嗦，\n";
    std::cout << "  但功能全，覆盖了各种边缘情况。\n";
    std::cout << "  C++20 的 std::chrono 日历功能就是参考它设计的。\n";
    std::cout << "\n";
    std::cout << "  几个经验：\n";
    std::cout << "  1️⃣ 服务端存 UTC 时间，展示时再转本地时间\n";
    std::cout << "  2️⃣  日志打时间戳一定要包含时区或明确是UTC\n";
    std::cout << "  3️⃣  月份加减注意月末调整（1月31号+1个月是几号？）\n";
    std::cout << "  4️⃣  不要用 time_t 了，那是C时代的老东西\n";
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
    std::cout << "  1. 写一个函数，计算两个日期之间相差多少个完整的月？\n";
    std::cout << "     （注意：2024-01-15 到 2024-02-14 不算一个完整月）\n";
    std::cout << "\n";
    std::cout << "  2. 写一个工具类，生成带时间戳的文件名：\n";
    std::cout << "     格式：log_20240615_143025.log\n";
    std::cout << "     （精确到秒，确保不会重名）\n";
    std::cout << "\n";
    std::cout << "  3. 写一个函数，判断某一年的某个月有多少个工作日？\n";
    std::cout << "     （只排除周六周日，先不考虑节假日）\n";
    std::cout << "\n";
    std::cout << "  4. 思考：为什么不建议用 time_t 和 struct tm？\n";
    std::cout << "     它们有什么局限性？\n";
    std::cout << "============================================================\n";
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  Boost.DateTime 日期时间库深度解析\n";
    std::cout << "  工作中天天用 —— 别自己写日期计算，用库！\n";
    std::cout << "============================================================\n";

    demo_date_basics();
    demo_date_calculation();
    demo_time_duration();
    demo_ptime();
    demo_formatting();
    demo_time_period();
    demo_timezone();
    demo_practical();

    laoshifu_says();
    exercises();

    return 0;
}
