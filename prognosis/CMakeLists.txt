project(prognosis)

SET (HEADERS ../transformermodel.h )
SET (SOURCES prognosis.cpp ../transformermodel.cpp)

LINK_DIRECTORIES(/usr/local/lib/)

INCLUDE_DIRECTORIES(/usr/local/include)
INCLUDE_DIRECTORIES(/usr/local/include/openbabel3)

#find_package(OpenMP)
#if (OPENMP_FOUND)
#    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
#endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
INCLUDE_DIRECTORIES(/usr/local/mxe/usr/i686-w64-mingw32.shared/include/openbabel3)
INCLUDE_DIRECTORIES(/usr/local/mxe/usr/i686-w64-mingw32.shared/include/openblas)
LINK_DIRECTORIES(/usr/local/mxe/usr/i686-w64-mingw32.shared/bin/)
endif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)

add_executable(prognosis ${HEADERS} ${SOURCES})
target_link_libraries(prognosis openbabel openblas)

   
   
   
   
