# gsync core library (lib/src)
add_library(gsync_lib STATIC
    syncfile.h syncfile.cpp
    syncfiledata.h syncfiledata.cpp
    googlefile.h googlefile.cpp
    storageformat.h
    encryption.h encryption_stub.cpp
    keymanager.h keymanager_stub.cpp
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
else()
    # Fallback: fetch a pinned release and build it
    include(FetchContent)
    FetchContent_Declare(
        libsodium_fallback
        GIT_REPOSITORY https://github.com/jedisct1/libsodium.git
        GIT_TAG 1.0.18
    )
    FetchContent_MakeAvailable(libsodium_fallback)

    if (TARGET libsodium::libsodium)
        target_link_libraries(gsync_lib PUBLIC libsodium::libsodium)
    elseif (TARGET sodium)
        target_link_libraries(gsync_lib PUBLIC sodium)
    else()
        # Create an imported interface target as a last resort (may need to adjust include dirs)
        add_library(libsodium::libsodium INTERFACE IMPORTED)
        target_include_directories(libsodium::libsodium INTERFACE ${libsodium_fallback_SOURCE_DIR}/src/libsodium/include)
        target_link_libraries(gsync_lib PUBLIC libsodium::libsodium)
    endif()
endif()
