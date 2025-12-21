# Tests for lib
find_package(Qt6 COMPONENTS Test REQUIRED)

add_executable(test_syncfile
    test_syncfile.cpp
)

target_link_libraries(test_syncfile PRIVATE gsync_lib Qt6::Test)
add_test(NAME test_syncfile COMMAND test_syncfile)
