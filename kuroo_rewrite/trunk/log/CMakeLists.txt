include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(kuroolog_SRCS
 log.cpp
)

kde4_add_ui_files(kuroolog_SRCS logbase.ui )
kde4_add_library(kuroolog ${kuroolog_SRCS})
target_link_libraries(kuroolog ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} libkportage )
install(TARGETS kuroolog ${INSTALL_TARGETS_DEFAULT_ARGS})
