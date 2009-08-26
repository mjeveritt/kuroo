include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(kuroohistory_SRCS
 history.cpp
)

kde4_add_ui_files(kuroohistory_SRCS historybase.ui )
kde4_add_library(kuroohistory ${kuroohistory_SRCS})
target_link_libraries(kuroohistory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} libkportage )
install(TARGETS kuroohistory ${INSTALL_TARGETS_DEFAULT_ARGS})
