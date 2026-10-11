/**
 * Boost.Spirit X3 解析器入门与进阶 Demo
 *
 * 知识点覆盖：
 *   1. Spirit X3 基础：解析器组合子、属性传播、phrase_parse
 *   2. 数字列表解析：int_ % ',' 模式、vector 属性
 *   3. 算术表达式求值：递归下降、运算符优先级、语义动作
 *   4. 小型 DSL 解析器：变量赋值、表达式计算、AST 构建
 *   5. 抽象语法树（AST）：变体类型（variant）、递归 AST 节点
 *   6. 错误处理：expect 操作符、错误位置、错误信息格式化
 *   7. 解析器命名与调试：BOOST_SPIRIT_DEFINE、调试宏
 *   8. 规则（rule）的使用与属性传播机制
 *
 * Boost 优势：
 *   - 纯头文件库，无需链接（仅 Spirit X3）
 *   - 使用表达式模板在编译期生成解析器，运行时性能极高
 *   - 支持 PEG（Parsing Expression Grammar）范式
 *   - 与 Boost.Fusion / Boost.Variant 深度集成
 *   - C++ 标准库中无对应功能
 *   - X3 版本比 V2 编译更快、代码更简洁
 *
 * 编译命令（示例）：
 *   g++ -std=c++17 spirit_parser.cpp -o spirit_parser
 *   （Spirit X3 是纯头文件库，无需链接 boost 库）
 */

#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <cmath>
#include <map>
#include <sstream>
#include <iomanip>

// ========== Boost 头文件说明 ==========
// Spirit X3 是 Boost.Spirit 的最新版本（v3），相比 V2：
//   - 编译更快
//   - API 更简洁
//   - 不需要链接 boost 库（纯头文件）
//
// <boost/spirit/home/x3.hpp>       - X3 总头文件
// <boost/spirit/home/x3/support/ast/variant.hpp> - AST 变体支持
// <boost/spirit/home/x3/support/ast/position_tagged.hpp> - 位置标记
// <boost/fusion/include/adapt_struct.hpp> - Fusion 结构体适配（用于 AST 属性传播）

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

// ============================================================
// 一、基础：解析数字列表
// ============================================================
// 最简单的解析器组合示例：解析逗号分隔的整数列表
// 核心概念：
//   - int_: 整数解析器
//   - char_(','): 匹配逗号
//   - % : 列表解析器（a % b 等价于 a >> *(b >> a)）
//   - phrase_parse: 带跳过分隔符（如空格）的解析

void parse_number_list_demo() {
    std::cout << "\n--- 1. 解析数字列表 ---" << std::endl;

    std::string input = "1, 2, 3, 4, 5, 6, 7, 8, 9, 10";
    std::cout << "  输入: " << input << std::endl;

    std::vector<int> result;
    auto begin = input.begin();
    auto end = input.end();

    // phrase_parse: 解析过程中跳过空格（ascii::space）
    // int_ % ',' : 匹配一个或多个 int_，用逗号分隔
    // 解析结果自动填充到 result（属性传播）
    bool success = x3::phrase_parse(
        begin, end,
        x3::int_ % ',',  // 解析器：逗号分隔的整数列表
        ascii::space,    // 跳过的字符（空格）
        result           // 输出属性
    );

    if (success && begin == end) {
        std::cout << "  解析成功! 共 " << result.size() << " 个数字:" << std::endl;
        std::cout << "  [";
        for (size_t i = 0; i < result.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << result[i];
        }
        std::cout << "]" << std::endl;
    } else {
        std::cout << "  解析失败，剩余: " << std::string(begin, end) << std::endl;
    }

    // 进阶：解析更复杂的列表
    std::cout << "\n1.2 嵌套括号的数字列表:" << std::endl;
    std::string input2 = "[ 10, 20, 30, 40, 50 ]";
    std::cout << "  输入: " << input2 << std::endl;

    std::vector<int> result2;
    auto b2 = input2.begin(), e2 = input2.end();

    // '[' >> (int_ % ',') >> ']' : 按顺序匹配 [ 列表 ]
    // >> 是序列操作符（顺序匹配）
    bool ok2 = x3::phrase_parse(
        b2, e2,
        '[' >> (x3::int_ % ',') >> ']',
        ascii::space,
        result2
    );

    if (ok2 && b2 == e2) {
        std::cout << "  解析成功! 共 " << result2.size() << " 个数字" << std::endl;
    } else {
        std::cout << "  解析失败" << std::endl;
    }
}

// ============================================================
// 二、算术表达式解析器（带语义动作）
// ============================================================
// 经典的表达式求值示例，演示：
//   - 递归解析器
//   - 运算符优先级（通过语法层级实现）
//   - 语义动作（[](auto& ctx){ ... }）
//   - _val, _attr 等 Phoenix 占位符
//
// 语法（巴科斯范式）：
//   expression = term (('+' | '-') term)*
//   term       = factor (('*' | '/') factor)*
//   factor     = number | '(' expression ')' | ('-' | '+') factor

namespace calc {

    // 语义动作中使用 x3::_val(ctx) 表示当前规则的属性值
    // x3::_attr(ctx) 表示匹配到的属性

    // 因子解析器（递归需要用规则）
    struct expression_class;
    struct term_class;
    struct factor_class;

    // 定义规则
    x3::rule<expression_class, double> const expression("expression");
    x3::rule<term_class, double> const term("term");
    x3::rule<factor_class, double> const factor("factor");

    // 表达式定义
    auto const expression_def =
        term
        >> *(   ('+' >> term)[([](auto& ctx) { x3::_val(ctx) += x3::_attr(ctx); })]
            |   ('-' >> term)[([](auto& ctx) { x3::_val(ctx) -= x3::_attr(ctx); })]
            )
        ;

    auto const term_def =
        factor
        >> *(   ('*' >> factor)[([](auto& ctx) { x3::_val(ctx) *= x3::_attr(ctx); })]
            |   ('/' >> factor)[([](auto& ctx) { x3::_val(ctx) /= x3::_attr(ctx); })]
            )
        ;

    auto const factor_def =
        x3::double_
        |   '(' >> expression >> ')'
        |   ('-' >> factor)[([](auto& ctx) { x3::_val(ctx) = -x3::_attr(ctx); })]
        |   ('+' >> factor)
        ;

    // 将规则与定义绑定
    BOOST_SPIRIT_DEFINE(expression, term, factor)

} // namespace calc

void arithmetic_expression_demo() {
    std::cout << "\n--- 2. 算术表达式解析与求值 ---" << std::endl;

    std::vector<std::string> tests = {
        "1 + 2",
        "1 + 2 * 3",
        "(1 + 2) * 3",
        "10 / 2 + 5 * 2",
        "-3.5 * 2 + 10",
        "((1 + 2) * (3 + 4)) / 2.5"
    };

    for (const auto& expr : tests) {
        double result = 0;
        auto begin = expr.begin();
        auto end = expr.end();

        bool success = x3::phrase_parse(begin, end, calc::expression, ascii::space, result);

        std::cout << "  " << std::left << std::setw(25) << expr;
        if (success && begin == end) {
            std::cout << " = " << result << std::endl;
        } else {
            std::cout << " 解析失败 (剩余: " << std::string(begin, end) << ")" << std::endl;
        }
    }
}

// ============================================================
// 三、AST（抽象语法树）构建
// ============================================================
// 对于复杂的 DSL，通常先构建 AST，再对 AST 进行求值或其他处理。
// 使用 x3::variant（基于 boost::variant）表示 AST 节点的变体类型。
// 使用 Boost.Fusion 适配结构体，实现自动属性传播。

namespace ast {

    // 前置声明（用于递归定义）
    struct expression;

    // 数字字面量
    struct number {
        double value;
    };

    // 变量引用
    struct variable {
        std::string name;
    };

    // 一元操作（取负）
    struct unary_op {
        char op;
        std::shared_ptr<expression> operand;
    };

    // 二元操作
    struct binary_op {
        std::shared_ptr<expression> left;
        char op;
        std::shared_ptr<expression> right;
    };

    // 表达式变体（可以是数字、变量、一元操作、二元操作）
    struct expression : x3::variant<
        number,
        variable,
        x3::forward_ast<unary_op>,    // forward_ast 用于递归变体
        x3::forward_ast<binary_op>
    > {
        using base_type::base_type;
        using base_type::operator=;
    };

    // 赋值语句
    struct assignment {
        std::string name;
        expression value;
    };

    // 语句列表
    using statement_list = std::vector<assignment>;

} // namespace ast

// Fusion 适配结构体，使 Spirit 可以自动填充结构体成员
BOOST_FUSION_ADAPT_STRUCT(ast::number, value)
BOOST_FUSION_ADAPT_STRUCT(ast::variable, name)
BOOST_FUSION_ADAPT_STRUCT(ast::unary_op, op, operand)
BOOST_FUSION_ADAPT_STRUCT(ast::binary_op, left, op, right)
BOOST_FUSION_ADAPT_STRUCT(ast::assignment, name, value)

// ============================================================
// 四、AST 求值器（访问者模式）
// ============================================================
// 使用 std::visit + 重载 lambda 实现 AST 求值
// 这是 C++17 的 variant 访问模式

class ASTEvaluator {
public:
    double evaluate(const ast::expression& expr) {
        return std::visit(*this, expr);
    }

    void assign(const std::string& name, double value) {
        variables_[name] = value;
    }

    double operator()(const ast::number& num) {
        return num.value;
    }

    double operator()(const ast::variable& var) {
        auto it = variables_.find(var.name);
        if (it == variables_.end()) {
            throw std::runtime_error("未定义的变量: " + var.name);
        }
        return it->second;
    }

    double operator()(const ast::unary_op& op) {
        double val = evaluate(*op.operand);
        switch (op.op) {
            case '-': return -val;
            case '+': return val;
            default: throw std::runtime_error("未知一元运算符");
        }
    }

    double operator()(const ast::binary_op& op) {
        double left = evaluate(*op.left);
        double right = evaluate(*op.right);
        switch (op.op) {
            case '+': return left + right;
            case '-': return left - right;
            case '*': return left * right;
            case '/': return left / right;
            default: throw std::runtime_error("未知二元运算符");
        }
    }

private:
    std::map<std::string, double> variables_;
};

// ============================================================
// 五、DSL 解析器语法定义
// ============================================================
// 实现一个简单的赋值语言：
//   var x = 10
//   var y = x + 5
//   result = x * y + 3

namespace dsl {

    // 规则类（用于错误信息中的规则名称）
    struct statement_class;
    struct expression_class;
    struct term_class;
    struct factor_class;
    struct variable_class;

    // 规则定义（属性类型即 AST 节点类型）
    x3::rule<statement_class, ast::assignment> const statement("statement");
    x3::rule<expression_class, ast::expression> const expression("expression");
    x3::rule<term_class, ast::expression> const term("term");
    x3::rule<factor_class, ast::expression> const factor("factor");
    x3::rule<variable_class, std::string> const variable("variable");

    // 变量名：字母开头，后跟字母数字下划线
    auto const variable_def =
        x3::lexeme[  // lexeme: 内部不跳过空格
            (x3::alpha | '_') >> *(x3::alnum | '_')
        ];

    // 因子：数字 | 变量 | 括号表达式 | 一元 +/-
    auto const factor_def =
            x3::double_
        |   variable
        |   '(' >> expression >> ')'
        |   (x3::char_('-') > factor)  // > 是 expect：失败则报错
        |   (x3::char_('+') > factor)
        ;

    // 项：因子的乘除
    auto const term_def =
        factor >> *(
                (x3::char_('*') > factor)
            |   (x3::char_('/') > factor)
        );

    // 表达式：项的加减
    auto const expression_def =
        term >> *(
                (x3::char_('+') > term)
            |   (x3::char_('-') > term)
        );

    // 赋值语句：变量 = 表达式
    auto const statement_def =
        variable >> '=' >> expression
        ;

    // 绑定规则与定义
    BOOST_SPIRIT_DEFINE(
        statement, expression, term, factor, variable
    )

} // namespace dsl

// ============================================================
// 六、错误处理
// ============================================================
// Spirit X3 提供了完善的错误处理机制：
//   - expect 操作符（>）：期望匹配，失败则抛出 expectation_failure
//   - on_error 回调：自定义错误处理
//   - 位置迭代器：记录错误位置
//
// 这里我们实现一个带错误信息的解析函数

struct ParseResult {
    bool success;
    std::string error_message;
    size_t error_pos;
    ast::statement_list statements;
};

ParseResult parse_dsl(const std::string& input) {
    ParseResult result;
    result.success = false;
    result.error_pos = 0;

    auto begin = input.begin();
    auto end = input.end();

    // 使用迭代器记录位置
    auto iter = begin;

    try {
        ast::statement_list stmts;

        // 解析多条语句，以分号或换行分隔
        bool ok = x3::phrase_parse(
            iter, end,
            *(dsl::statement % (x3::lit(';') | x3::eol)),
            ascii::space,
            stmts
        );

        if (ok && iter == end) {
            result.success = true;
            result.statements = std::move(stmts);
        } else {
            result.success = false;
            result.error_pos = iter - begin;
            result.error_message = "解析未完成，剩余输入";
        }
    } catch (const x3::expectation_failure<decltype(iter)>& e) {
        result.success = false;
        result.error_pos = e.where() - begin;
        result.error_message = "期望 '" + std::string(e.which().begin(), e.which().end())
                             + "' 在 '" + std::string(e.where(), end).substr(0, 20) + "'";
    }

    return result;
}

void ast_and_dsl_demo() {
    std::cout << "\n--- 3. AST 构建与 DSL 解析 ---" << std::endl;

    // 测试 DSL 代码
    std::string dsl_code = R"(
        x = 10
        y = x + 5
        z = x * y + 3
        result = (z - 10) / 2
        neg = -x + y
    )";

    std::cout << "  DSL 代码:" << std::endl;
    {
        std::istringstream iss(dsl_code);
        std::string line;
        int lineno = 0;
        while (std::getline(iss, line)) {
            if (!line.empty()) {
                std::cout << "    " << ++lineno << ": " << line << std::endl;
            }
        }
    }

    // 解析
    ParseResult pr = parse_dsl(dsl_code);

    if (!pr.success) {
        std::cout << "  解析失败: " << pr.error_message << std::endl;
        std::cout << "  错误位置: 第 " << pr.error_pos << " 个字符" << std::endl;
        return;
    }

    std::cout << "  解析成功! 共 " << pr.statements.size() << " 条语句" << std::endl;

    // 求值
    ASTEvaluator evaluator;
    std::cout << "\n  执行结果:" << std::endl;
    for (const auto& stmt : pr.statements) {
        try {
            double value = evaluator.evaluate(stmt.value);
            evaluator.assign(stmt.name, value);
            std::cout << "    " << stmt.name << " = " << value << std::endl;
        } catch (const std::exception& e) {
            std::cout << "    " << stmt.name << " = 错误: " << e.what() << std::endl;
        }
    }

    // 错误处理演示
    std::cout << "\n3.2 错误处理演示:" << std::endl;
    std::string bad_code = "x = 10 + * 5";  // 语法错误
    std::cout << "  错误代码: " << bad_code << std::endl;

    ParseResult bad_result = parse_dsl(bad_code);
    if (!bad_result.success) {
        std::cout << "  捕获错误: " << bad_result.error_message << std::endl;
        std::cout << "  位置: " << std::string(bad_code.size(), ' ') << "^\n";
        std::cout << "         " << bad_code << std::endl;
    }
}

// ============================================================
// 七、更复杂的解析：CSV 行解析
// ============================================================
// 演示 quoted string 的解析和更复杂的属性组合

void csv_parser_demo() {
    std::cout << "\n--- 4. CSV 行解析器 ---" << std::endl;

    // CSV 字段：可以是带引号的字符串（含转义）或普通字符串
    std::string csv_line = R"("Smith, John",42,"New York",,active)";
    std::cout << "  输入: " << csv_line << std::endl;

    // 定义 quoted_string 解析器
    auto quoted_string = x3::lexeme[
        '"' >> *('\\' >> x3::char_ | ~x3::char_('"')) >> '"'
    ];

    // 普通字段（不含逗号和引号）
    auto plain_field = *~x3::char_(',');

    // CSV 字段：要么带引号，要么是普通字段
    auto csv_field = quoted_string | plain_field;

    // CSV 行
    auto csv_line_parser = csv_field % ',';

    std::vector<std::string> fields;
    auto begin = csv_line.begin();
    auto end = csv_line.end();

    // 注意：CSV 解析不能跳过空格（空格是数据的一部分）
    bool success = x3::parse(begin, end, csv_line_parser, fields);

    if (success && begin == end) {
        std::cout << "  解析成功! 共 " << fields.size() << " 个字段:" << std::endl;
        for (size_t i = 0; i < fields.size(); ++i) {
            std::cout << "    [" << i << "] \"" << fields[i] << "\"" << std::endl;
        }
    } else {
        std::cout << "  解析失败" << std::endl;
    }
}

// ============================================================
// 八、Spirit 核心概念总结
// ============================================================
void spirit_concepts_summary() {
    std::cout << "\n--- Boost.Spirit X3 核心概念 ---" << std::endl;
    std::cout << "  1. 解析器组合子（Parser Combinators）:" << std::endl;
    std::cout << "     - 基本解析器: int_, double_, char_, lit, string" << std::endl;
    std::cout << "     - 序列: a >> b (a 后面跟 b)" << std::endl;
    std::cout << "     - 选择: a | b (a 或 b)" << std::endl;
    std::cout << "     - 列表: a % b (b 分隔的 a 列表)" << std::endl;
    std::cout << "     - 零或多次: *a" << std::endl;
    std::cout << "     - 一或多次: +a" << std::endl;
    std::cout << "     - 可选: -a" << std::endl;
    std::cout << "     - 期望: a > b (b 必须匹配，否则报错)" << std::endl;
    std::cout << std::endl;
    std::cout << "  2. 属性传播（Attribute Propagation）:" << std::endl;
    std::cout << "     - 解析器自动将匹配结果转换为对应类型" << std::endl;
    std::cout << "     - int_ -> int, double_ -> double" << std::endl;
    std::cout << "     - a % b -> vector<T>" << std::endl;
    std::cout << "     - 通过 Fusion 适配自动填充结构体" << std::endl;
    std::cout << std::endl;
    std::cout << "  3. 语义动作（Semantic Actions）:" << std::endl;
    std::cout << "     - parser[action]: 匹配成功时执行 action" << std::endl;
    std::cout << "     - _val(ctx): 当前规则的属性值" << std::endl;
    std::cout << "     - _attr(ctx): 当前匹配到的属性" << std::endl;
    std::cout << std::endl;
    std::cout << "  4. 规则（Rule）:" << std::endl;
    std::cout << "     - 用于递归解析器和命名" << std::endl;
    std::cout << "     - BOOST_SPIRIT_DEFINE 宏绑定定义" << std::endl;
    std::cout << std::endl;
    std::cout << "  5. phrase_parse vs parse:" << std::endl;
    std::cout << "     - parse: 严格按输入匹配" << std::endl;
    std::cout << "     - phrase_parse: 在匹配过程中跳过分隔符" << std::endl;
    std::cout << std::endl;
    std::cout << "  6. lexeme 指令:" << std::endl;
    std::cout << "     - 内部不跳过空白（即使在 phrase_parse 中）" << std::endl;
    std::cout << "     - 用于标识符、字符串等不允许中间有空格的 token" << std::endl;
}

// ============================================================
// Demo 入口函数
// ============================================================
int main() {
    std::cout << "========== Boost.Spirit X3 解析器 Demo ==========" << std::endl;

    // 1. 数字列表解析
    parse_number_list_demo();

    // 2. 算术表达式求值
    arithmetic_expression_demo();

    // 3. AST 构建与 DSL 解析
    ast_and_dsl_demo();

    // 4. CSV 解析
    csv_parser_demo();

    // 5. 核心概念总结
    spirit_concepts_summary();

    std::cout << "\n========== Demo 结束 ==========" << std::endl;
    return 0;
}
