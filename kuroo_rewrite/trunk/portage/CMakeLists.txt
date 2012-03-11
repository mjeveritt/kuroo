project( libkportage )
include_directories( ${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )

set(libkportage_SRCS
 portage.cpp
 scan.cpp
)

kde4_add_library(libkportage ${libkportage_SRCS})
target_link_libraries(libkportage ${QT_QTSQL_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )
install(TARGETS libkportage ${INSTALL_TARGETS_DEFAULT_ARGS})
