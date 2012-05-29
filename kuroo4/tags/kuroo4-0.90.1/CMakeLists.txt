set( CMAKE_BUILD_TYPE DebugFull )

project( kuroo )
find_package( KDE4 REQUIRED )
include_directories( ${KDE4_INCLUDES} )
include( KDE4Defaults )

add_subdirectory( src )
add_subdirectory( po )
add_subdirectory( pics )
