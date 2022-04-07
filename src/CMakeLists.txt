set(PLASMA_RELATIVE_DATA_INSTALL_DIR "plasma")
set(KF5_DEP_VERSION "5.54.0") # handled by release scripts

find_package(ECM 5.54.0  NO_MODULE)
find_package(X11 REQUIRED)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(KF5 ${KF5_DEP_VERSION} REQUIRED
    COMPONENTS
    OPTIONAL_COMPONENTS
        Wayland
)
set_package_properties(KF5Wayland PROPERTIES DESCRIPTION "Integration with the Wayland compositor"
                       TYPE OPTIONAL
                      )
if(KF5Wayland_FOUND)
    set(HAVE_KWAYLAND 1)
endif()

#optional features
message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                       URL "http://www.x.org"
                       TYPE OPTIONAL
                      )
find_package(XCB MODULE COMPONENTS XCB COMPOSITE DAMAGE SHAPE XFIXES RENDER)
set_package_properties(XCB PROPERTIES DESCRIPTION "X protocol C-language Binding"
                       URL "http://xcb.freedesktop.org"
                       TYPE OPTIONAL
                      )

message(STATUS "XCB_XCB_FOUND = ${XCB_XCB_FOUND}")
message(STATUS "XCB_LIBRARIES = ${XCB_LIBRARIES}")

if(X11_FOUND AND XCB_XCB_FOUND)
  set(HAVE_X11 1)
  find_package(Qt5 REQUIRED NO_MODULE COMPONENTS X11Extras)
  #X11_Xrender discovery is done by FindX11
  #add_feature_info("X Rendering Extension (libXrender)" X11_Xrender_FOUND "Support for compositing, rendering operations, and alpha-blending. STRONGLY RECOMMENDED")
endif()

find_package(OpenGL)
set_package_properties(OpenGL PROPERTIES DESCRIPTION "The OpenGL libraries"
                       URL "http://www.opengl.org"
                       TYPE OPTIONAL
                      )

find_package(EGL)
set_package_properties(EGL PROPERTIES
                       PURPOSE "Support for Window Thumbnail on EGL platform"
                       TYPE OPTIONAL
                      )
set(HAVE_EGL ${EGL_FOUND})

if(OPENGL_FOUND AND (${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL"))
    set(HAVE_GLX ${HAVE_X11})
else()
    set(HAVE_GLX 0)
endif()

if(HAVE_X11 AND XCB_XCB_FOUND AND XCB_COMPOSITE_FOUND AND XCB_DAMAGE_FOUND)
    set(HAVE_XCB_COMPOSITE TRUE)
else()
    set(HAVE_XCB_COMPOSITE FALSE)
endif()

configure_file(config-x11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-x11.h)
configure_file(config-plasma.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-plasma.h)

find_package(Qt5 REQUIRED COMPONENTS Core Quick QuickWidgets LinguistTools)
find_package(KF5Config REQUIRED)
find_package(KF5GlobalAccel REQUIRED)
find_package(KF5I18n REQUIRED)
find_package(KF5CoreAddons REQUIRED)
find_package(KF5WindowSystem)
find_package(PkgConfig REQUIRED)

pkg_search_module(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)

get_target_property(QT_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE} IMPORTED_LOCATION)

macro(query_qmake args output)
    exec_program(${QT_QMAKE_EXECUTABLE} ARGS -query ${args} OUTPUT_VARIABLE ${output} RETURN_VALUE exitCode)

    if(NOT ${exitCode} EQUAL 0)
        message(FATAL_ERROR "exec ${QT_QMAKE_EXECUTABLE} failed, with args: ${args}, error message: ${output}")
    endif()
endmacro()

query_qmake("QT_INSTALL_PLUGINS" QT_INSTALL_PLUGINS)
set(PLUGIN_INSTALL_PATH ${QT_INSTALL_PLUGINS}/platforms)
set(INSTALL_PATH "${QT_INSTALL_PLUGINS}/kwin/effects/plugins")

set(SRCS 
  backgroundimageprovider.cpp
  imageprovider.cpp
  multitasking.cpp
  multitasking_model.cpp
  background.cpp
  factory.cpp
  windowthumbnail.cpp
  thumbmanager.qml
  PlusButton.qml
  DesktopBackgroundImage.qml
)

# translations
file(GLOB TS LIST_DIRECTORIES false translations/${PLUGIN_NAME}*.ts)
set_source_files_properties(${TS}
    PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/translations)

qt5_create_translation(QM ${SRCS} ${TS} OPTIONS -locations none)

add_library(
    ${PLUGIN_NAME} MODULE
    ${SRCS}
    ${QM}
    data.qrc
)

target_link_libraries(
    ${PLUGIN_NAME}
    PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Quick
    Qt5::QuickWidgets
    X11::X11
    PRIVATE
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::WindowSystem
    KF5::GlobalAccel
    KF5::I18n
    PkgConfig::QGSETTINGS
    Qt5::X11Extras
)

install(TARGETS ${PLUGIN_NAME} DESTINATION ${INSTALL_PATH})
