find_package(Qt6 COMPONENTS Test REQUIRED)

add_executable(test_passworddialog
    test_passworddialog.cpp
)

target_link_libraries(test_passworddialog PRIVATE filemanager_lib Qt6::Test Qt6::Widgets)
add_test(NAME test_passworddialog COMMAND test_passworddialog)