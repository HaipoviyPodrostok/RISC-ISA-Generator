cmake_minimum_required(VERSION 3.14)
project(RISK-ISA-Generator)

find_package(nlohmann_json QUIET)

if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
        GIT_PROGRESS TRUE
    )

    FetchContent_MakeAvailable(json)
endif()


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