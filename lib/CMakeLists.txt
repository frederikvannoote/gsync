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
