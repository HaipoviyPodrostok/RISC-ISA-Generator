cmake_minimum_required(VERSION 3.14)
project(RISK-ISA-Generator)

include(FetchContent)

find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
    )
    FetchContent_MakeAvailable(json)
endif()

find_package(GTest QUIET)

if(NOT GTest_FOUND)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.12.1
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    main.cpp
    src/layout_solver.cpp
    src/output_formatter.cpp
    src/json_parser.cpp
)

add_executable(risk_isa_generator.x ${SOURCES})

target_include_directories(risk_isa_generator.x PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(risk_isa_generator.x PRIVATE nlohmann_json::nlohmann_json)

add_executable(risk_isa_tests 
    test/main_test.cpp 
    test/test_parser.cpp 
    test/test_solver.cpp
    src/json_parser.cpp
    src/layout_solver.cpp
    src/output_formatter.cpp
)
target_include_directories(risk_isa_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(risk_isa_tests PRIVATE gtest gmock nlohmann_json::nlohmann_json)

include(GoogleTest)
gtest_discover_tests(risk_isa_tests)