set( CMAKE_BUILD_TYPE DebugFull )
#Warned by dev build, probably works with older than 3.7
cmake_minimum_required( VERSION 3.7 )
#Warned by dev build, apparently having mixed full path and search references
#causes different behavior depending on this policy, we should be fine with NEW
if( COMMAND cmake_policy)
	cmake_policy( SET CMP0003 NEW )
endif( COMMAND cmake_policy )

project( kuroo )
find_package( KDE4 REQUIRED )
include_directories( ${KDE4_INCLUDES} )
include( KDE4Defaults )

add_subdirectory( src )
add_subdirectory( po )
add_subdirectory( pics )
