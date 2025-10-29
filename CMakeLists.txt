cmake_minimum_required(VERSION 3.19)

project(GSync VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets LinguistTools NetworkAuth)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES en de fr zh da nl fi fr el ja lv no pt ro ru es sv uk)

qt_add_executable(GSync
        main.cpp
        googleauthenticator.h googleauthenticator.cpp
        googledrive.h googledrive.cpp
        googlefile.h googlefile.cpp
        googlefilelist.h googlefilelist.cpp
        googlesync.h googlesync.cpp
        googlefilesync.h googlefilesync.cpp
        googlefiledownload.h googlefiledownload.cpp
        googlefilesyncmodel.h googlefilesyncmodel.cpp
        progressdelegate.h progressdelegate.cpp
        fileanalyzerunnable.h fileanalyzerunnable.cpp
        googlesyncdata.h googlesyncdata.cpp
        mainwindow.h mainwindow.cpp mainwindow.ui)

target_link_libraries(GSync PRIVATE Qt6::Widgets)
target_link_libraries(GSync PRIVATE Qt6::Core)
target_link_libraries(GSync PRIVATE Qt6::NetworkAuth)

add_subdirectory(i18n)
