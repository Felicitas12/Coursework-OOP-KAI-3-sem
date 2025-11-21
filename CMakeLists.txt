cmake_minimum_required(VERSION 3.10)
project(ElectronicGradeJournal)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)

add_library(DAL INTERFACE)
target_include_directories(DAL INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/DAL)
target_link_libraries(DAL INTERFACE nlohmann_json::nlohmann_json)

add_library(BLL INTERFACE)
target_include_directories(BLL INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/BLL)
target_link_libraries(BLL INTERFACE DAL)

add_library(PL INTERFACE)
target_include_directories(PL INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/PL)
target_link_libraries(PL INTERFACE BLL)

add_executable(GradeJournal main.cpp)
target_link_libraries(GradeJournal PRIVATE PL)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(UnitTests Tests/Tests.cpp)
target_link_libraries(UnitTests PRIVATE BLL GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(UnitTests)