cmake_minimum_required(VERSION 3.13.4)
project(vec)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

file(GLOB sourceglob 
    "${PROJECT_SOURCE_DIR}/src/*.h" "${PROJECT_SOURCE_DIR}/src/*.c" 
    "${PROJECT_SOURCE_DIR}/src/utility/*.h" "${PROJECT_SOURCE_DIR}/src/utility/*.c"
    "${PROJECT_SOURCE_DIR}/src/nsvg/*.h" "${PROJECT_SOURCE_DIR}/src/nsvg/*.c"
    "${PROJECT_SOURCE_DIR}/src/imagefile/*.h" "${PROJECT_SOURCE_DIR}/src/imagefile/*.c")

add_library(${PROJECT_NAME} STATIC ${sourceglob})
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "./src/entrypoint.h")

file(COPY "../template.svg" DESTINATION "./bin")
