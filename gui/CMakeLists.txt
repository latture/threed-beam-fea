set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)

qt5_add_resources(fea_gui_resources fea_gui.qrc)

add_library(fea_gui_lib mainwindow.cpp)
add_executable(fea_gui main.cpp ${fea_gui_resources})
target_link_libraries(fea_gui fea_gui_lib threed_beam_fea)
add_dependencies(fea_gui fea_cmd)

qt5_use_modules(fea_gui_lib Widgets)
