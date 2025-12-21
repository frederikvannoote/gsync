cmake_minimum_required(VERSION 3.19)

# gsync core library
add_library(gsync_lib STATIC
    src/syncfile.cpp
    src/syncfiledata.cpp
)

target_include_directories(gsync_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(gsync_lib PUBLIC Qt6::Core)

# Tests will be added under lib/tests
enable_testing()

find_package(Qt6 COMPONENTS Test REQUIRED)

add_executable(test_syncfile
    tests/test_syncfile.cpp
)

target_link_libraries(test_syncfile PRIVATE gsync_lib Qt6::Test)
add_test(NAME test_syncfile COMMAND test_syncfile)
