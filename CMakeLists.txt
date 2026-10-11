# ============================================================================
# C++ 高级学习 Demo 构建文件
# 涵盖：C++17 新特性、Boost 库、设计模式
# ============================================================================
cmake_minimum_required(VERSION 3.14)
project(cpp_advanced_demos LANGUAGES CXX)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 公共头文件目录
include_directories(${CMAKE_SOURCE_DIR}/common)

# ============================================================================
# 查找 Boost 库（可选，找不到则跳过Boost相关demo）
# ============================================================================
find_package(Boost QUIET COMPONENTS
    system
    filesystem
    program_options
    serialization
    thread
    date_time
    log
    log_setup
)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    message(STATUS "Boost found: ${Boost_VERSION}")
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
else()
    message(WARNING "Boost not found - Boost demos will be skipped")
endif()

# 查找线程库（并行算法需要）
find_package(Threads REQUIRED)

# ============================================================================
# 查找 Google Test（可选，找不到则跳过单元测试相关demo）
# ============================================================================
find_package(GTest QUIET)

if(GTest_FOUND)
    message(STATUS "Google Test found")
else()
    message(WARNING "Google Test not found - testing demos will be skipped")
endif()

# ============================================================================
# 入门基础 Demo（筑基期）
# ============================================================================
set(BEGINNER_SOURCES
    beginner/01_basics.cpp
    beginner/02_functions.cpp
    beginner/03_array_string.cpp
    beginner/04_pointer_reference.cpp
    beginner/05_struct.cpp
    beginner/06_file_io.cpp
    beginner/07_class_basics.cpp
    beginner/08_inheritance_polymorphism.cpp
    beginner/09_operator_overload.cpp
    beginner/10_template_intro.cpp
)

message(STATUS "\n=== Beginner Demos (筑基期) ===")
foreach(src ${BEGINNER_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_executable(beginner_${target} ${src})
    target_link_libraries(beginner_${target} PRIVATE Threads::Threads)
    message(STATUS "  Added target: beginner_${target}")
endforeach()

# ============================================================================
# 中级进阶 Demo（蜕变期）
# ============================================================================
set(INTERMEDIATE_SOURCES
    intermediate/memory_model_raii.cpp
    intermediate/stl_containers.cpp
    intermediate/stl_algorithms.cpp
    intermediate/smart_ptr_deep.cpp
    intermediate/coding_practices.cpp
)

message(STATUS "\n=== Intermediate Demos (蜕变期) ===")
foreach(src ${INTERMEDIATE_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_executable(intermediate_${target} ${src})
    target_link_libraries(intermediate_${target} PRIVATE Threads::Threads)
    message(STATUS "  Added target: intermediate_${target}")
endforeach()

# ============================================================================
# 数据结构与算法 Demo（蜕变期）
# ============================================================================
set(ALGORITHM_SOURCES
    algorithms/sorting_algorithms.cpp
    algorithms/data_structures.cpp
)

message(STATUS "\n=== Algorithm Demos (算法与数据结构) ===")
foreach(src ${ALGORITHM_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_executable(algo_${target} ${src})
    target_link_libraries(algo_${target} PRIVATE Threads::Threads)
    message(STATUS "  Added target: algo_${target}")
endforeach()

# ============================================================================
# 调试与排错 Demo
# ============================================================================
set(DEBUGGING_SOURCES
    debugging/01_print_debugging.cpp
    debugging/02_common_bugs.cpp
    debugging/03_sanitizers_demo.cpp
)

message(STATUS "\n=== Debugging Demos (调试专题) ===")
foreach(src ${DEBUGGING_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_executable(debug_${target} ${src})
    target_link_libraries(debug_${target} PRIVATE Threads::Threads)
    message(STATUS "  Added target: debug_${target}")
endforeach()

# ============================================================================
# 异常与错误处理 Demo
# ============================================================================
set(EXCEPTION_SOURCES
    exceptions/exception_basics.cpp
)

message(STATUS "\n=== Exception Demos (异常与错误处理) ===")
foreach(src ${EXCEPTION_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_executable(exc_${target} ${src})
    target_link_libraries(exc_${target} PRIVATE Threads::Threads)
    message(STATUS "  Added target: exc_${target}")
endforeach()

# ============================================================================
# 性能优化 Demo
# ============================================================================
set(PERFORMANCE_SOURCES
    performance/01_cache_optimization.cpp
    performance/02_common_pitfalls.cpp
)

message(STATUS "\n=== Performance Demos (性能优化) ===")
foreach(src ${PERFORMANCE_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_executable(perf_${target} ${src})
    target_link_libraries(perf_${target} PRIVATE Threads::Threads)
    message(STATUS "  Added target: perf_${target}")
endforeach()

# ============================================================================
# 单元测试 Demo（需要 Google Test，找不到则跳过）
# ============================================================================
if(GTest_FOUND)
    set(TESTING_SOURCES
        testing/01_gtest_basics.cpp
        testing/02_gtest_advanced.cpp
    )

    # 不同 CMake 版本导出的目标名不一样，做个兼容
    if(TARGET GTest::gtest_main)
        set(GTEST_MAIN_LINK GTest::gtest_main)
    else()
        set(GTEST_MAIN_LINK ${GTEST_MAIN_LIBRARIES})
    endif()
    if(TARGET GTest::gtest)
        set(GTEST_LINK GTest::gtest)
    else()
        set(GTEST_LINK ${GTEST_LIBRARIES})
    endif()

    message(STATUS "\n=== Unit Testing Demos (单元测试) ===")
    foreach(src ${TESTING_SOURCES})
        get_filename_component(target ${src} NAME_WE)
        add_executable(unittest_${target} ${src})
        target_link_libraries(unittest_${target} PRIVATE
            ${GTEST_LINK}
            ${GTEST_MAIN_LINK}
            Threads::Threads
        )
        message(STATUS "  Added target: unittest_${target}")
    endforeach()
endif()

# ============================================================================
# 并发编程专题 Demo
# ============================================================================
set(CONCURRENCY_SOURCES
    concurrency/01_threading_basics.cpp
    concurrency/02_synchronization.cpp
    concurrency/03_race_deadlock.cpp
    concurrency/04_atomic_memory.cpp
    concurrency/05_thread_pool.cpp
    concurrency/06_async_future.cpp
)

message(STATUS "\n=== Concurrency Demos (并发专题) ===")
foreach(src ${CONCURRENCY_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_executable(concurrency_${target} ${src})
    target_link_libraries(concurrency_${target} PRIVATE Threads::Threads)
    message(STATUS "  Added target: concurrency_${target}")
endforeach()

# IPC demo 需要 Boost
if(Boost_FOUND)
    add_executable(concurrency_07_ipc_shared_memory concurrency/07_ipc_shared_memory.cpp)
    target_link_libraries(concurrency_07_ipc_shared_memory PRIVATE
        ${Boost_SYSTEM_LIBRARY}
        Threads::Threads
    )
    message(STATUS "  Added target: concurrency_07_ipc_shared_memory (Boost IPC)")
endif()

# ============================================================================
# C++17 高级特性 Demo
# ============================================================================
set(CPP17_SOURCES
    cpp17/type_deduction.cpp
    cpp17/structured_binding.cpp
    cpp17/if_constexpr.cpp
    cpp17/fold_expressions.cpp
    cpp17/optional_variant_any.cpp
    cpp17/parallel_algorithms.cpp
    cpp17/string_view_filesystem.cpp
    cpp17/move_semantics_advanced.cpp
)

message(STATUS "\n=== C++17 Demos ===")
foreach(src ${CPP17_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_executable(cpp17_${target} ${src})
    target_link_libraries(cpp17_${target} PRIVATE Threads::Threads)
    message(STATUS "  Added target: cpp17_${target}")
endforeach()

# ============================================================================
# C++11→C++14→C++17 演进对比 Demo
# ============================================================================
set(EVOLUTION_SOURCES
    cpp_evolution/cpp11_core_features.cpp
    cpp_evolution/cpp14_features.cpp
    cpp_evolution/evolution_comparison.cpp
)

message(STATUS "\n=== C++ Evolution Demos ===")
foreach(src ${EVOLUTION_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_executable(evo_${target} ${src})
    target_link_libraries(evo_${target} PRIVATE Threads::Threads)
    message(STATUS "  Added target: evo_${target}")
endforeach()

# C++20/23 特性速览（需要编译器支持 C++20，支持才编译）
if("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    add_executable(evo_cpp20_overview cpp_evolution/cpp20_overview.cpp)
    set_target_properties(evo_cpp20_overview PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(evo_cpp20_overview PRIVATE Threads::Threads)
    message(STATUS "  Added target: evo_cpp20_overview (C++20)")
else()
    message(STATUS "  Skipped: cpp20_overview.cpp (compiler lacks C++20 support)")
endif()

# ============================================================================
# Boost 库 Demo（仅在找到Boost时添加）
# ============================================================================
if(Boost_FOUND)
    message(STATUS "\n=== Boost Demos ===")

    # smart_ptr - header only + system
    add_executable(boost_smart_ptr boost/smart_ptr.cpp)
    target_link_libraries(boost_smart_ptr PRIVATE ${Boost_SYSTEM_LIBRARY})
    message(STATUS "  Added target: boost_smart_ptr")

    # asio - header only + system + thread
    add_executable(boost_asio_basics boost/asio_basics.cpp)
    target_link_libraries(boost_asio_basics PRIVATE
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        Threads::Threads
    )
    message(STATUS "  Added target: boost_asio_basics")

    # filesystem
    add_executable(boost_filesystem boost/filesystem.cpp)
    target_link_libraries(boost_filesystem PRIVATE
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
    )
    message(STATUS "  Added target: boost_filesystem")

    # program_options
    add_executable(boost_program_options boost/program_options.cpp)
    target_link_libraries(boost_program_options PRIVATE
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
    )
    message(STATUS "  Added target: boost_program_options")

    # serialization
    add_executable(boost_serialization boost/serialization.cpp)
    target_link_libraries(boost_serialization PRIVATE
        ${Boost_SERIALIZATION_LIBRARY}
    )
    message(STATUS "  Added target: boost_serialization")

    # spirit - header only
    add_executable(boost_spirit_parser boost/spirit_parser.cpp)
    message(STATUS "  Added target: boost_spirit_parser")

    # signals2 - header only + thread
    add_executable(boost_signals2 boost/signals2.cpp)
    target_link_libraries(boost_signals2 PRIVATE
        ${Boost_THREAD_LIBRARY}
        Threads::Threads
    )
    message(STATUS "  Added target: boost_signals2")

    # multi_index - header only
    add_executable(boost_multi_index boost/multi_index.cpp)
    message(STATUS "  Added target: boost_multi_index")

    # log - 需要 log 和 log_setup 库
    add_executable(boost_log_demo boost/log_demo.cpp)
    target_link_libraries(boost_log_demo PRIVATE
        ${Boost_LOG_LIBRARY}
        ${Boost_LOG_SETUP_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        Threads::Threads
    )
    message(STATUS "  Added target: boost_log_demo")

    # property_tree - 头文件为主 + system
    add_executable(boost_property_tree boost/property_tree_demo.cpp)
    target_link_libraries(boost_property_tree PRIVATE
        ${Boost_SYSTEM_LIBRARY}
    )
    message(STATUS "  Added target: boost_property_tree")

    # datetime - 需要 date_time 库
    add_executable(boost_datetime boost/datetime_demo.cpp)
    target_link_libraries(boost_datetime PRIVATE
        ${Boost_DATE_TIME_LIBRARY}
    )
    message(STATUS "  Added target: boost_datetime")

    # bimap - header only
    add_executable(boost_bimap boost/bimap_demo.cpp)
    message(STATUS "  Added target: boost_bimap")

    # algorithm - header only
    add_executable(boost_algorithm boost/algorithm_demo.cpp)
    message(STATUS "  Added target: boost_algorithm")
endif()

# ============================================================================
# 设计模式 Demo
# ============================================================================
set(PATTERN_SOURCES
    patterns/singleton.cpp
    patterns/factory_abstract.cpp
    patterns/builder.cpp
    patterns/adapter.cpp
    patterns/decorator.cpp
    patterns/proxy.cpp
    patterns/observer.cpp
    patterns/strategy.cpp
    patterns/visitor.cpp
    patterns/state.cpp
    patterns/command.cpp
)

message(STATUS "\n=== Design Pattern Demos ===")
foreach(src ${PATTERN_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_executable(pattern_${target} ${src})
    target_link_libraries(pattern_${target} PRIVATE Threads::Threads)
    message(STATUS "  Added target: pattern_${target}")
endforeach()

# ============================================================================
# 综合实战项目：Todo CLI（有独立的 CMakeLists.txt）
# ============================================================================
message(STATUS "\n=== Practical Project (综合实战项目) ===")
add_subdirectory(projects/todo_cli)
message(STATUS "  Added targets: todo_cli, test_task")

# ============================================================================
# 自定义目标：编译所有
# ============================================================================
add_custom_target(all_beginner
    COMMENT "Building all beginner demos..."
)
foreach(src ${BEGINNER_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_dependencies(all_beginner beginner_${target})
endforeach()

add_custom_target(all_intermediate
    COMMENT "Building all intermediate demos..."
)
foreach(src ${INTERMEDIATE_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_dependencies(all_intermediate intermediate_${target})
endforeach()

add_custom_target(all_algorithms
    COMMENT "Building all algorithm demos..."
)
foreach(src ${ALGORITHM_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_dependencies(all_algorithms algo_${target})
endforeach()

add_custom_target(all_debugging
    COMMENT "Building all debugging demos..."
)
foreach(src ${DEBUGGING_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_dependencies(all_debugging debug_${target})
endforeach()

add_custom_target(all_exceptions
    COMMENT "Building all exception demos..."
)
foreach(src ${EXCEPTION_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_dependencies(all_exceptions exc_${target})
endforeach()

add_custom_target(all_performance
    COMMENT "Building all performance demos..."
)
foreach(src ${PERFORMANCE_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_dependencies(all_performance perf_${target})
endforeach()

if(GTest_FOUND)
    add_custom_target(all_testing
        COMMENT "Building all unit testing demos..."
    )
    foreach(src ${TESTING_SOURCES})
        get_filename_component(target ${src} NAME_WE)
        add_dependencies(all_testing unittest_${target})
    endforeach()
endif()

add_custom_target(all_concurrency
    COMMENT "Building all concurrency demos..."
)
foreach(src ${CONCURRENCY_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_dependencies(all_concurrency concurrency_${target})
endforeach()
if(Boost_FOUND)
    add_dependencies(all_concurrency concurrency_07_ipc_shared_memory)
endif()

add_custom_target(all_cpp17
    COMMENT "Building all C++17 demos..."
)
foreach(src ${CPP17_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_dependencies(all_cpp17 cpp17_${target})
endforeach()

add_custom_target(all_evolution
    COMMENT "Building all evolution demos..."
)
foreach(src ${EVOLUTION_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_dependencies(all_evolution evo_${target})
endforeach()

add_custom_target(all_patterns
    COMMENT "Building all design pattern demos..."
)
foreach(src ${PATTERN_SOURCES})
    get_filename_component(target ${src} NAME_WE)
    add_dependencies(all_patterns pattern_${target})
endforeach()

if(Boost_FOUND)
    add_custom_target(all_boost
        COMMENT "Building all Boost demos..."
        DEPENDS
            boost_smart_ptr
            boost_asio_basics
            boost_filesystem
            boost_program_options
            boost_serialization
            boost_spirit_parser
            boost_signals2
            boost_multi_index
            boost_log_demo
            boost_property_tree
            boost_datetime
            boost_bimap
            boost_algorithm
    )
endif()

# 全部编译（不含Boost）
add_custom_target(all
    COMMENT "Building all demos..."
    DEPENDS
        all_beginner
        all_intermediate
        all_algorithms
        all_debugging
        all_exceptions
        all_performance
        all_concurrency
        all_cpp17
        all_evolution
        all_patterns
        todo_cli
        test_task
)

# 有 Google Test 时把单元测试也纳入 all
if(GTest_FOUND)
    add_dependencies(all all_testing)
endif()

message(STATUS "\n=== Build Summary ===")
message(STATUS "  Beginner demos:     10 targets (筑基期)")
message(STATUS "  Intermediate demos:  5 targets (蜕变期·进阶)")
message(STATUS "  Algorithm demos:     2 targets (蜕变期·算法)")
message(STATUS "  Debugging demos:     3 targets (调试专题)")
message(STATUS "  Exception demos:     1 target  (异常与错误处理)")
message(STATUS "  Performance demos:   2 targets (性能优化)")
message(STATUS "  Concurrency demos:   7 targets (并发专题)")
message(STATUS "  C++17 demos:         8 targets (内功期)")
message(STATUS "  Evolution demos:     3 targets (演进对比)")
message(STATUS "  Pattern demos:      11 targets (化境期)")
message(STATUS "  Project targets:     2 targets (todo_cli + test_task)")
if(GTest_FOUND)
    message(STATUS "  Unit testing demos:  2 targets (单元测试)")
else()
    message(STATUS "  Unit testing demos:  skipped (Google Test not found)")
endif()
if(Boost_FOUND)
    message(STATUS "  Boost demos:        13 targets (大师期)")
else()
    message(STATUS "  Boost demos:         skipped (Boost not found)")
endif()
message(STATUS "")
