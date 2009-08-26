project( kuroo )
cmake_minimum_required(VERSION 2.6)
set(CMAKE_BUILD_TYPE DebugFull )

find_package(KDE4 REQUIRED )
include(KDE4Defaults)

include_directories( ${KDE4_INCLUDES} portage package queue history )

add_subdirectory( portage )
add_subdirectory( icons )

add_subdirectory( package )
add_subdirectory( queue )
add_subdirectory( history )
add_subdirectory( log )

set(kuroo_SRCS
 kuroo.cpp
)

kde4_add_kcfg_files(kuroo_SRCS config.kcfgc )
kde4_add_executable(kuroo ${kuroo_SRCS} )
target_link_libraries(kuroo ${KDE4_KDEUI_LIBS} libkportage kurooconfig kuroopackage kurooqueue kuroohistory kuroolog )
install(TARGETS kuroo ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kuroo.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kurooui.rc DESTINATION ${DATA_INSTALL_DIR}/kuroo)
install(FILES kuroo.desktop DESTINATION ${XDG_APPS_INSTALL_DIR}/System)
