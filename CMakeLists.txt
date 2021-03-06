cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_CXX_STANDARD 11)

project(chemfilter)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED Core Widgets Sql Network Script)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

find_package(OpenBabel3)
INCLUDE_DIRECTORIES(${OPENBABEL3_INCLUDE_DIR})

find_package( OpenBLAS REQUIRED )
INCLUDE_DIRECTORIES(${OpenBLAS_INCLUDE_DIR})

SET (HEADERS chemfilter.h clearthread.h lap_checkbox.h newcolumn.h
    delegate.h qsartablemodel.h transformermodel.h qsarsortmodel.h
    style/qplastiquestyle.h
    style/qstylehelper_p.h
    style/qhexstring_p.h
    style/qstylecache_p.h
    histwidget.h
    enterresult.h
    heighttextedit.h
    freezetable.h
    )

SET (SOURCES delegate.cpp qsartablemodel.cpp newcolumn.cpp
             chemfilter.cpp main.cpp clearthread.cpp transformermodel.cpp qsarsortmodel.cpp
             style/qplastiquestyle.cpp
             style/qstylehelper.cpp histwidget.cpp enterresult.cpp heighttextedit.cpp freezetable.cpp
             )

SET(RCFILE
    info.rc)

SET ( UI chemfilter.ui newcolumn.ui enterresult.ui)

set(RESOURCES resources.qrc)
qt5_add_resources(QRC_SOURCES ${RESOURCES})

QT5_WRAP_UI(UIS ${UI})

add_subdirectory(${PROJECT_SOURCE_DIR}/prognosis)

add_executable(chemfilter WIN32 ${HEADERS} ${SOURCES} ${UIS} ${QRC_SOURCES} ${RCFILE})
target_link_libraries(chemfilter Qt5::Widgets Qt5::Core Qt5::Sql Qt5::Network Qt5::Script ${OPENBABEL3_LIBRARIES} ${OpenBLAS_LIB})

