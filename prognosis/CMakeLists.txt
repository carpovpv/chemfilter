project(prognosis)

SET (HEADERS ../transformermodel.h )
SET (SOURCES prognosis.cpp ../transformermodel.cpp)

INCLUDE_DIRECTORIES(/usr/local/include/openbabel3)

find_package( BLAS REQUIRED )
message( STATUS BLAS found: ${BLAS_LIBRARIES} )

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
INCLUDE_DIRECTORIES(/usr/local/mxe/usr/i686-w64-mingw32.shared/include/openbabel3)
INCLUDE_DIRECTORIES(/usr/local/mxe/usr/i686-w64-mingw32.shared/include/openblas)
LINK_DIRECTORIES(/usr/local/mxe/usr/i686-w64-mingw32.shared/bin/)
endif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)

add_executable(prognosis ${HEADERS} ${SOURCES})
target_link_libraries(prognosis openbabel ${BLAS_LIBRARIES})

   
   
   
   
