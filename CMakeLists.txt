cmake_minimum_required(VERSION 2.6)

project(Opeke)
find_package(KDE4 REQUIRED)
set( QT_USE_QTGUI TRUE )
set( QT_USE_QTOPENGL TRUE )
include (KDE4Defaults)
include (FindOpenGL)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/")
find_package(OGRE REQUIRED)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${OPENGL_INCLUDE_DIR} ${QT_QTOPENGL_INCLUDE_DIR} ${OGRE_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS -fexceptions)
set(CMAKE_CXX_FLAGS -Wno-deprecated)

set(opeke_SRCS
	brick.cpp
   opeke.cpp
   main.cpp
	opekeview.cpp
	opeketool.cpp
	undoaction.cpp
	)

kde4_add_ui_files(opeke_SRCS opeketool_base.ui opekeview_base.ui prefs_base.ui prefs_mouse.ui)

kde4_add_kcfg_files(opeke_SRCS GENERATE_MOC settings.kcfgc )

kde4_add_executable(opeke ${opeke_SRCS})

target_link_libraries(opeke ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${OGRE_MAIN_LIBRARY})

install(TARGETS opeke DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES opeke.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES opeke.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES plugins.cfg resources.cfg  DESTINATION  ${DATA_INSTALL_DIR}/opeke )
install( FILES block.mesh roof.mesh	sphere.mesh	cylinder.mesh invcyl.mesh DESTINATION  ${DATA_INSTALL_DIR}/opeke )
install( FILES opekeui.rc  DESTINATION  ${DATA_INSTALL_DIR}/opeke )

