/**
 * Boost.Asio 基础与进阶用法 Demo
 *
 * 知识点覆盖：
 *   1. io_context 核心：事件循环、run()/poll()、restart()、多线程并发
 *   2. 同步 TCP echo 服务器与客户端
 *   3. 异步 TCP echo 服务器：async_accept / async_read / async_write
 *      （composed operations 与缓冲区生命周期）
 *   4. strand：串行化异步操作，保证线程安全
 *   5. 定时器（steady_timer）：同步/异步等待、取消、周期性定时器
 *   6. 错误处理：error_code、异常、错误类别
 *   7. Buffer 与 streambuf：内存视图 vs 拥有存储的流缓冲区
 *   8. work_guard：防止 io_context 提前退出
 *
 * Boost 优势：
 *   - Boost.Asio 是 C++ 网络编程的事实标准，也是 Networking TS 的参考实现；
 *     但 std::net 至今仍未正式进入 C++ 标准（C++20/23/26 都没有网络库）
 *   - 跨平台：Windows / Linux / macOS 统一接口
 *   - 支持同步/异步、TCP/UDP/ICMP、SSL、串口等多种协议
 *   - Proactor 模式设计，性能优异
 *   - 与 Boost 其他库（SmartPtr、Bind、Coroutine）深度集成
 *
 * 编译命令（Asio 在 Boost 1.90 是头文件实现，不需要 -lboost_system）：
 *   Linux:  g++ -std=c++17 asio_basics.cpp -o asio_basics -pthread
 *   MinGW:  g++ -std=c++17 asio_basics.cpp -o asio_basics -lws2_32 -lmswsock
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <chrono>
#include <functional>
#include <array>

// ========== Boost 头文件说明 ==========
// <boost/asio.hpp>                - Asio 总头文件（包含大部分常用组件）
// <boost/asio/io_context.hpp>     - io_context：I/O 事件循环核心
// <boost/asio/ip/tcp.hpp>         - TCP 协议相关：tcp::socket, tcp::acceptor, tcp::endpoint
// <boost/asio/steady_timer.hpp>   - steady_timer：稳定定时器（不受系统时间调整影响）
// <boost/asio/strand.hpp>         - strand：串行化异步操作的执行器
// <boost/asio/buffer.hpp>         - buffer：可修改/不可修改的内存缓冲区
// <boost/asio/streambuf.hpp>      - streambuf：基于流的缓冲区
// <boost/asio/write.hpp>          - async_write / write：写操作
// <boost/asio/read.hpp>           - async_read / read：读操作
// <boost/system/error_code.hpp>   - error_code：错误码系统
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/system/error_code.hpp>
#include <boost/bind/bind.hpp>

namespace asio = boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;
using namespace std::chrono_literals;

// ============================================================
// 一、同步 TCP Echo 服务器
// ============================================================
// 同步模型简单直接，但每个连接需要一个线程，高并发下扩展性差。
// 适合：连接数少、逻辑简单的场景。
class SyncEchoServer {
public:
    SyncEchoServer(asio::io_context& io, short port)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
        std::cout << "  [SyncServer] 启动，监听端口 " << port << std::endl;
    }

    // 处理单个客户端连接（同步方式）
    void handle_client(tcp::socket socket) {
        try {
            std::array<char, 1024> buffer;
            std::string client_addr = socket.remote_endpoint().address().to_string();
            std::cout << "  [SyncServer] 客户端连接: " << client_addr << std::endl;

            // 循环读取数据并回显
            while (true) {
                error_code ec;
                size_t len = socket.read_some(asio::buffer(buffer), ec);

                if (ec == asio::error::eof) {
                    std::cout << "  [SyncServer] 客户端断开: " << client_addr << std::endl;
                    break;
                } else if (ec) {
                    throw boost::system::system_error(ec);
                }

                // 同步写回
                asio::write(socket, asio::buffer(buffer, len));
            }
        } catch (const std::exception& e) {
            std::cerr << "  [SyncServer] 异常: " << e.what() << std::endl;
        }
    }

    tcp::acceptor& acceptor() { return acceptor_; }

private:
    tcp::acceptor acceptor_;
};

// 同步 TCP 客户端
std::string sync_echo_client(short port, const std::string& message) {
    asio::io_context io;
    tcp::socket socket(io);
    tcp::resolver resolver(io);

    // 解析地址并连接
    auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
    asio::connect(socket, endpoints);

    // 发送数据
    asio::write(socket, asio::buffer(message));

    // 读取回显。read_some 只保证“至少读到一点”，
    // 一次调用可能只拿到消息的一部分，必须循环读到完整长度。
    std::array<char, 1024> buffer;
    std::string reply;
    error_code ec;
    while (reply.size() < message.size()) {
        size_t len = socket.read_some(asio::buffer(buffer), ec);
        if (ec == asio::error::eof) {
            break;
        }
        if (ec) {
            throw boost::system::system_error(ec);
        }
        if (len == 0) {
            break;
        }
        reply.append(buffer.data(), len);
    }

    // 主动关闭连接
    socket.shutdown(tcp::socket::shutdown_both);
    socket.close();

    return reply;
}

// ============================================================
// 二、异步 TCP Echo 服务器
// ============================================================
// 异步模型基于 Proactor 模式，单线程即可处理大量并发连接。
// 关键概念：回调函数（handler）、io_context 事件循环、异步操作链。
//
// 补充：async_write / async_read / async_read_until 都是“组合操作”
// (composed operations)。它们内部可能多次调用 *_some 版本，直到
// 整个缓冲区处理完才调用一次 handler。因此传给组合操作的缓冲区
// 必须在 handler 被调用前一直有效（不能是即将销毁的局部变量）。

class AsyncEchoSession : public std::enable_shared_from_this<AsyncEchoSession> {
public:
    AsyncEchoSession(tcp::socket socket)
        : socket_(std::move(socket)) {
        std::cout << "  [AsyncSession] 新连接: "
                  << socket_.remote_endpoint().address() << std::endl;
    }

    ~AsyncEchoSession() {
        std::cout << "  [AsyncSession] 连接关闭" << std::endl;
    }

    // 启动会话：开始异步读
    void start() {
        do_read();
    }

private:
    // 异步读取数据
    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(
            asio::buffer(data_, max_length),
            [this, self](error_code ec, size_t bytes_transferred) {
                if (!ec) {
                    do_write(bytes_transferred);  // 异步操作链：读完 -> 写回
                } else if (ec == asio::error::eof) {
                    // 客户端正常断开，不报错
                    std::cout << "  [AsyncSession] 客户端 EOF" << std::endl;
                } else {
                    std::cerr << "  [AsyncSession] 读取错误: " << ec.message() << std::endl;
                }
            });
    }

    // 异步写入数据（写完后继续读，形成读写循环）
    void do_write(size_t length) {
        auto self(shared_from_this());
        asio::async_write(
            socket_,
            asio::buffer(data_, length),
            [this, self](error_code ec, size_t /*bytes_transferred*/) {
                if (!ec) {
                    do_read();  // 异步操作链：写完 -> 继续读
                } else {
                    std::cerr << "  [AsyncSession] 写入错误: " << ec.message() << std::endl;
                }
            });
    }

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class AsyncEchoServer {
public:
    AsyncEchoServer(asio::io_context& io, short port)
        : io_(io),
          acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
        std::cout << "  [AsyncServer] 启动，监听端口 " << port << std::endl;
        do_accept();
    }

private:
    // 异步接受连接
    void do_accept() {
        acceptor_.async_accept(
            [this](error_code ec, tcp::socket socket) {
                if (!ec) {
                    // 创建会话并启动
                    std::make_shared<AsyncEchoSession>(std::move(socket))->start();
                } else {
                    std::cerr << "  [AsyncServer] accept 错误: " << ec.message() << std::endl;
                }
                do_accept();  // 继续接受下一个连接
            });
    }

    asio::io_context& io_;
    tcp::acceptor acceptor_;
};

// ============================================================
// 三、Strand：串行化异步操作
// ============================================================
// Strand 是 Boost.Asio 中的重要概念，它保证：
//   - 通过同一个 strand 提交的 handler 会按顺序串行执行
//   - 即使 io_context 运行在多线程中，strand 内的 handler 也不会并发执行
//   - 这使得在多线程 io_context 下，无需互斥锁即可保证共享数据的安全
//
// 使用场景：
//   - 多线程 io_context 中，某个连接的读写操作需要串行化
//   - 共享资源的异步操作需要互斥访问

class StrandDemo {
public:
    StrandDemo(asio::io_context& io)
        : strand_(asio::make_strand(io)),
          counter_(0) {
    }

    // 通过 strand 提交任务，保证 counter_ 的操作是串行的
    void increment(const std::string& name) {
        asio::post(strand_, [this, name]() {
            int old = counter_;
            // 模拟一些工作（即使多线程，strand 保证这里不会并发）
            std::this_thread::sleep_for(1ms);
            counter_ = old + 1;
            std::cout << "  [Strand] " << name << " 递增: counter = " << counter_
                      << " (线程 ID: " << std::this_thread::get_id() << ")" << std::endl;
        });
    }

    int counter() const { return counter_; }

private:
    asio::strand<asio::io_context::executor_type> strand_;
    int counter_;
};

// ============================================================
// 四、定时器详解
// ============================================================
// Boost.Asio 提供多种定时器：
//   - steady_timer：基于 steady_clock，不受系统时间调整影响（推荐用于超时控制）
//   - system_timer：基于 system_clock，受系统时间调整影响
//   - high_resolution_timer：高精度定时器
//
// 定时器操作：
//   - wait()：同步等待
//   - async_wait()：异步等待
//   - expires_after()：设置过期时间（相对时间）
//   - expires_at()：设置过期时间点（绝对时间）
//   - cancel()：取消定时器（会使等待的 handler 以 operation_aborted 错误返回）

class TimerDemo {
public:
    TimerDemo(asio::io_context& io) : io_(io) {}

    // 4.1 同步等待
    void sync_wait_demo() {
        std::cout << "  [Timer] 同步等待 1 秒..." << std::endl;
        asio::steady_timer timer(io_, 1s);
        timer.wait();
        std::cout << "  [Timer] 同步等待完成" << std::endl;
    }

    // 4.2 异步等待 + 错误处理
    void async_wait_demo() {
        std::cout << "  [Timer] 异步等待 500ms..." << std::endl;
        auto timer = std::make_shared<asio::steady_timer>(io_, 500ms);
        timer->async_wait([timer](const error_code& ec) {
            if (ec) {
                std::cout << "  [Timer] 异步等待被取消: " << ec.message() << std::endl;
            } else {
                std::cout << "  [Timer] 异步等待触发" << std::endl;
            }
        });
    }

    // 4.3 周期性定时器（通过递归 async_wait 实现）
    //
    // 关键：异步 handler 在函数返回之后才执行，它引用的状态必须一直存活。
    // 这里用 shared_ptr<State> 让每一轮 handler 持有一份状态，最后一轮结束后
    // 引用计数归零、自动释放。若把 handler 写成局部 std::function 再按引用
    // 捕获（[=, &handler]），函数一返回引用就悬空，程序会崩溃。
    void periodic_timer_demo(int count, std::function<void()> on_done) {
        struct State {
            asio::steady_timer timer;
            int total;
            int current = 0;
            std::function<void()> on_done;

            State(asio::io_context& io, int total_, std::function<void()> done)
                : timer(io), total(total_), on_done(std::move(done)) {}

            void arm(std::shared_ptr<State> self) {
                timer.expires_after(200ms);
                timer.async_wait([self](const error_code& ec) {
                    if (ec) return;

                    ++self->current;
                    std::cout << "  [Timer] 周期定时器第 " << self->current
                              << " 次触发" << std::endl;

                    if (self->current < self->total) {
                        self->arm(self);  // 把 self 继续传下去，保持状态存活
                    } else {
                        std::cout << "  [Timer] 周期定时器结束" << std::endl;
                        if (self->on_done) self->on_done();
                    }
                });
            }
        };

        auto state = std::make_shared<State>(io_, count, std::move(on_done));
        state->arm(state);
    }

private:
    asio::io_context& io_;
};

// ============================================================
// 五、错误处理对比
// ============================================================
// Boost.Asio 提供两种错误处理方式：
//   1. error_code 输出参数（不抛异常，更适合异步操作）
//   2. 异常抛出（更适合同步操作，代码简洁）

void error_handling_demo() {
    std::cout << "\n--- 5. 错误处理方式对比 ---" << std::endl;
    asio::io_context io;

    // 方式1：error_code（推荐用于生产代码）
    std::cout << "5.1 error_code 方式:" << std::endl;
    {
        tcp::socket socket(io);
        tcp::endpoint endpoint(asio::ip::make_address("127.0.0.1"), 9999);
        error_code ec;
        socket.connect(endpoint, ec);
        if (ec) {
            // Windows 的 ec.message() 返回系统 ANSI 编码（中文系统为 GBK），
            // 在 UTF-8 终端直接输出会乱码，所以这里以错误码为主。
            std::cout << "  连接失败 (预期): 端口 9999 没有服务监听" << std::endl;
            std::cout << "  错误值: " << ec.value() << std::endl;
            std::cout << "  错误类别: " << ec.category().name() << std::endl;
        }
    }

    // 方式2：异常方式（代码简洁，但需要 try-catch）
    std::cout << "5.2 异常方式:" << std::endl;
    {
        try {
            tcp::socket socket(io);
            tcp::endpoint endpoint(asio::ip::make_address("127.0.0.1"), 9999);
            socket.connect(endpoint);  // 失败会抛出 boost::system::system_error
        } catch (const boost::system::system_error& e) {
            std::cout << "  捕获 system_error (预期)，错误码: "
                      << e.code().value() << std::endl;
            std::cout << "  错误类别: " << e.code().category().name() << std::endl;
        }
    }
}

// ============================================================
// 六、Buffer 与 streambuf
// ============================================================
void buffer_demo() {
    std::cout << "\n--- 7. Buffer 与 streambuf ---" << std::endl;

    // asio::buffer 不拥有内存，只是对一段连续内存的视图。
    // 视图有效的前提是底层内存仍然存活。
    std::string text = "Hello, Asio";
    auto const_buf = asio::buffer(text);
    std::cout << "  buffer(string) 大小: "
              << asio::buffer_size(const_buf) << std::endl;

    std::array<char, 32> raw{};
    auto mutable_buf = asio::buffer(raw);
    std::cout << "  buffer(array) 大小: "
              << asio::buffer_size(mutable_buf) << std::endl;

    // asio::streambuf 拥有内部存储，适合“读多少不确定”的场景，
    // 常与 read_until / async_read_until 配合按行或按分隔符读取。
    asio::streambuf sb;
    std::ostream os(&sb);
    os << "first line\nsecond line\n";

    std::istream is(&sb);
    std::string line;
    std::getline(is, line);
    std::cout << "  streambuf 读出一行: " << line << std::endl;
    std::cout << "  剩余可读字节: " << sb.size() << std::endl;

    std::cout << "  区别：buffer 只是视图，不复制也不拥有数据；"
              << "streambuf 拥有缓冲区，可动态增长" << std::endl;
}

// ============================================================
// 七、Work Guard：保持 io_context 运行
// ============================================================
// io_context::run() 在没有待处理工作时会立即返回。
// 使用 work_guard 可以让 io_context 即使暂无任务也保持运行状态，
// 等待后续任务的到来。这在多线程服务器中非常重要。

void work_guard_demo() {
    std::cout << "\n--- 8. work_guard 保持 io_context 运行 ---" << std::endl;

    asio::io_context io;

    // 创建 work_guard，防止 io_context 因无任务而退出
    auto work = asio::make_work_guard(io);

    std::thread t([&io]() {
        std::cout << "  工作线程启动" << std::endl;
        io.run();
        std::cout << "  工作线程结束（io_context 已停止）" << std::endl;
    });

    // 提交一些任务
    asio::post(io, []() {
        std::cout << "  任务 1 执行" << std::endl;
    });
    asio::post(io, []() {
        std::cout << "  任务 2 执行" << std::endl;
    });

    // 等待任务完成
    std::this_thread::sleep_for(100ms);

    // 重置 work_guard，允许 io_context 在任务完成后退出
    std::cout << "  重置 work_guard" << std::endl;
    work.reset();

    t.join();
}

// ============================================================
// Demo 入口函数
// ============================================================
int main() {
    std::cout << "========== Boost.Asio 基础与进阶用法 Demo ==========" << std::endl;

    // --------------------------------------------------------
    // 1. io_context 基础
    // --------------------------------------------------------
    std::cout << "\n--- 1. io_context 核心概念 ---" << std::endl;
    {
        asio::io_context io;

        // post() 提交任务到 io_context，由 run() 驱动执行
        asio::post(io, []() {
            std::cout << "  任务 A 执行（线程 ID: " << std::this_thread::get_id() << "）" << std::endl;
        });
        asio::post(io, []() {
            std::cout << "  任务 B 执行" << std::endl;
        });

        std::cout << "  调用 io_context::run()" << std::endl;
        io.run();  // 阻塞执行所有任务，完成后返回
        std::cout << "  io_context::run() 返回（所有任务已完成）" << std::endl;

        // run() 返回后 io_context 处于 stopped 状态
        // 需要调用 restart() 才能再次使用
        io.restart();
        asio::post(io, []() {
            std::cout << "  restart 后新任务执行" << std::endl;
        });
        io.run();
    }

    // --------------------------------------------------------
    // 2. 同步 TCP Echo
    // --------------------------------------------------------
    std::cout << "\n--- 2. 同步 TCP Echo 服务器 ---" << std::endl;
    {
        const short port = 18080;
        asio::io_context io;
        SyncEchoServer server(io, port);

        // 在独立线程中运行同步服务器
        std::thread server_thread([&server, &io]() {
            try {
                tcp::socket socket(io);
                server.acceptor().accept(socket);
                server.handle_client(std::move(socket));
            } catch (const std::exception& e) {
                std::cerr << "  [SyncServer] 异常: " << e.what() << std::endl;
            }
        });

        // 等待服务器启动
        std::this_thread::sleep_for(100ms);

        // 客户端发送消息
        std::string msg = "Hello, Boost.Asio (Sync)!";
        std::cout << "  客户端发送: " << msg << std::endl;
        std::string reply = sync_echo_client(port, msg);
        std::cout << "  客户端收到: " << reply << std::endl;

        server_thread.join();
    }

    // --------------------------------------------------------
    // 3. 异步 TCP Echo
    // --------------------------------------------------------
    std::cout << "\n--- 3. 异步 TCP Echo 服务器 ---" << std::endl;
    {
        const short port = 18081;
        asio::io_context io;
        AsyncEchoServer server(io, port);

        // 运行 io_context（在独立线程中，避免阻塞主线程）
        std::thread io_thread([&io]() { io.run(); });

        // 等待服务器启动
        std::this_thread::sleep_for(100ms);

        // 客户端测试
        std::string msg = "Hello, Boost.Asio (Async)!";
        std::cout << "  客户端发送: " << msg << std::endl;
        std::string reply = sync_echo_client(port, msg);
        std::cout << "  客户端收到: " << reply << std::endl;

        // 停止 io_context
        io.stop();
        io_thread.join();
    }

    // --------------------------------------------------------
    // 4. Strand 演示
    // --------------------------------------------------------
    std::cout << "\n--- 4. Strand 串行化异步操作 ---" << std::endl;
    {
        asio::io_context io;
        StrandDemo demo(io);

        // 先投递任务，再启动 io_context。
        // 反过来的话，工作线程可能在任务入队前就执行完 run() 并退出，
        // 导致只有部分任务被执行（甚至一个都没执行）。
        for (int i = 0; i < 6; ++i) {
            demo.increment("任务" + std::to_string(i + 1));
        }

        // 多线程运行 io_context，strand 保证 6 个 handler 不会并发执行
        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i) {
            threads.emplace_back([&io]() { io.run(); });
        }

        // 6 个任务全部完成后 run() 才会返回，join 即可等待它们结束
        for (auto& t : threads) t.join();

        std::cout << "  最终 counter = " << demo.counter() << " (预期 6)" << std::endl;
        std::cout << "  Strand 保证了即使多线程，counter 操作也是串行安全的" << std::endl;
    }

    // --------------------------------------------------------
    // 5. 定时器演示
    // --------------------------------------------------------
    std::cout << "\n--- 5. 定时器使用 ---" << std::endl;
    {
        asio::io_context io;
        TimerDemo demo(io);

        // 同步等待
        demo.sync_wait_demo();

        // 异步等待
        demo.async_wait_demo();

        // 运行 io_context 处理异步任务
        io.run();

        // 周期性定时器（需要 restart io_context）
        io.restart();
        bool done = false;
        demo.periodic_timer_demo(3, [&done]() { done = true; });
        io.run();
    }

    // --------------------------------------------------------
    // 6. 错误处理
    // --------------------------------------------------------
    error_handling_demo();

    // --------------------------------------------------------
    // 7. Buffer 与 streambuf
    // --------------------------------------------------------
    buffer_demo();

    // --------------------------------------------------------
    // 8. Work Guard
    // --------------------------------------------------------
    work_guard_demo();

    // --------------------------------------------------------
    // 总结
    // --------------------------------------------------------
    std::cout << "\n--- Boost.Asio 核心要点总结 ---" << std::endl;
    std::cout << "  1. io_context 是事件循环核心，run() 驱动所有异步操作" << std::endl;
    std::cout << "  2. 同步 API 简单但阻塞；异步 API 高效但需要理解回调链" << std::endl;
    std::cout << "  3. Strand 提供了无锁的串行化机制，是多线程安全的关键" << std::endl;
    std::cout << "  4. 定时器支持同步/异步、取消、周期性触发" << std::endl;
    std::cout << "  5. error_code 优于异常，尤其在异步场景下" << std::endl;
    std::cout << "  6. work_guard 防止 io_context 提前退出" << std::endl;
    std::cout << "  7. C++ 标准库至今没有网络库；Asio 是 Networking TS 的参考实现，"
              << "std::net 尚未正式进入标准" << std::endl;
    std::cout << "  8. buffer 是内存视图，streambuf 拥有存储；组合操作期间缓冲区必须保持有效"
              << std::endl;

    std::cout << "\n========== Demo 结束 ==========" << std::endl;
    return 0;
}
