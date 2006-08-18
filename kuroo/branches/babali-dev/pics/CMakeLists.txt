INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL_FILES( /share/apps/$project/pics FILES  )

KDE3_INSTALL_ICONS( hicolor )



#original Makefile.am contents follow:

#
#KDE_ICON = AUTO
#kurooPicsdir = $(kde_datadir)/$project/pics
#
