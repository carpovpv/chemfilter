cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_CXX_STANDARD 11)

project(prognosis)

SET (HEADERS ../transformermodel.h )
SET (SOURCES prognosis.cpp ../transformermodel.cpp)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake/modules)

find_package(OpenBabel3)
INCLUDE_DIRECTORIES(${OPENBABEL3_INCLUDE_DIR})

find_package( OpenBLAS REQUIRED )
INCLUDE_DIRECTORIES(${OpenBLAS_INCLUDE_DIR})

add_executable(prognosis ${HEADERS} ${SOURCES})
target_link_libraries(prognosis ${OPENBABEL3_LIBRARIES} ${OpenBLAS_LIB})
