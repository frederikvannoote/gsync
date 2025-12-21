# gsync core library (lib/src)
add_library(gsync_lib STATIC
    syncfile.h syncfile.cpp
    syncfiledata.h syncfiledata.cpp
    googlefile.h googlefile.cpp
)

# Public include directory so dependents can include headers as <syncfile.h>
target_include_directories(gsync_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gsync_lib PUBLIC Qt6::Core Qt6::Network)
