# filemanager module
add_library(filemanager_lib STATIC
    passworddialog.h passworddialog.cpp
)

target_include_directories(filemanager_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(filemanager_lib PUBLIC gsync_lib Qt6::Widgets)

# Export the UI compile step by listing the .ui file in the target sources during qt_add_library in modern setups,
# but with Qt's AUTOUIC enabled globally, including the .ui file in sources isn't strictly required here.
