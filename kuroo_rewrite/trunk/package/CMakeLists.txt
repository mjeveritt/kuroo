include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(kuroopackage_SRCS
 package.cpp
)

kde4_add_ui_files(kuroopackage_SRCS packagebase.ui )
kde4_add_library(kuroopackage ${kuroopackage_SRCS})
target_link_libraries(kuroopackage ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} libkportage )
install(TARGETS kuroopackage ${INSTALL_TARGETS_DEFAULT_ARGS})
