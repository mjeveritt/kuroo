include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(kurooqueue_SRCS
 queue.cpp
)

kde4_add_ui_files(kurooqueue_SRCS queuebase.ui)  
kde4_add_library(kurooqueue ${kurooqueue_SRCS})
target_link_libraries(kurooqueue ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} libkportage )
install(TARGETS kurooqueue ${INSTALL_TARGETS_DEFAULT_ARGS})
