cmake_minimum_required(VERSION 3.0.0)
project(map_gen VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
 

# include(CTest)
# enable_testing()

add_library(mazegen INTERFACE mazegen/mazegen.hpp)

add_executable(map_gen main.cpp)

find_package(SFML REQUIRED graphics window system)

target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-window sfml-system)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# copy contetnts of assets
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)