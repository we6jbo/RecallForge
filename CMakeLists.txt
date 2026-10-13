cmake_minimum_required(VERSION 3.21)
project(RecallForge VERSION 0.3.9.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_add_executable(RecallForge main.cpp mainwindow.cpp mainwindow.h mainwindow.ui)
target_link_libraries(RecallForge PRIVATE Qt6::Widgets)
install(TARGETS RecallForge RUNTIME DESTINATION bin)

install(FILES org.oneal.RecallForge.desktop
        DESTINATION share/applications)

install(FILES org.oneal.RecallForge.metainfo.xml
        DESTINATION share/metainfo)

install(FILES org.oneal.RecallForge.svg
        DESTINATION share/icons/hicolor/scalable/apps)
