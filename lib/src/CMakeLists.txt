# gsync core library (lib/src)
add_library(gsync_lib STATIC
    syncfile.h syncfile.cpp
    syncfiledata.h syncfiledata.cpp
    googlefile.h googlefile.cpp
    storageformat.h
    encryption.h encryption.cpp
    keymanager.h keymanager.cpp
)

# Public include directory so dependents can include headers as <syncfile.h>
target_include_directories(gsync_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gsync_lib PUBLIC Qt6::Core Qt6::Network)

# Try to find libsodium provided by Conan (CMakeDeps) or the system
find_package(libsodium CONFIG QUIET)
if (TARGET libsodium::libsodium)
    target_link_libraries(gsync_lib PUBLIC libsodium::libsodium)
elseif (TARGET sodium)
    target_link_libraries(gsync_lib PUBLIC sodium)
endif()
