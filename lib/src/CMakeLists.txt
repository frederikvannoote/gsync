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
    message(STATUS "libsodium found: using libsodium::libsodium")
    target_link_libraries(gsync_lib PUBLIC libsodium::libsodium)
    # Expose a macro so code can conditionally compile sodium-backed implementation
    target_compile_definitions(gsync_lib PUBLIC HAVE_LIBSODIUM=1)
elseif (TARGET sodium)
    message(STATUS "libsodium found: using legacy 'sodium' target")
    target_link_libraries(gsync_lib PUBLIC sodium)
    target_compile_definitions(gsync_lib PUBLIC HAVE_LIBSODIUM=1)
else()
    message(STATUS "libsodium not found: encryption will be disabled (HAVE_LIBSODIUM not defined)")
endif()
