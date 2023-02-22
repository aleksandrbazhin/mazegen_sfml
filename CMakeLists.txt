cmake_minimum_required(VERSION 3.0.0)
project(mazegen-sfml VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

set(IMGUI_DIR PATH "${CMAKE_CURRENT_LIST_DIR}/imgui")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/imgui-sfml/cmake")
find_package(ImGui 1.80 REQUIRED)


if ((NOT ${CMAKE_VERSION} VERSION_LESS 3.11) AND (NOT OpenGL_GL_PREFERENCE))
  set(OpenGL_GL_PREFERENCE "LEGACY")
endif()
find_package(OpenGL REQUIRED)


# these headers will be installed alongside ImGui-SFML
set(IMGUI_PUBLIC_HEADERS
  imconfig.h
  ${IMGUI_INCLUDE_DIR}/imgui.h
  ${IMGUI_INCLUDE_DIR}/imgui_internal.h # not actually public, but users might need it
  ${IMGUI_INCLUDE_DIR}/imstb_rectpack.h
  ${IMGUI_INCLUDE_DIR}/imstb_textedit.h
  ${IMGUI_INCLUDE_DIR}/imstb_truetype.h
  ${IMGUI_INCLUDE_DIR}/misc/cpp/imgui_stdlib.h
)
add_library(ImGui-SFML imgui-sfml/imgui-SFML.cpp ${IMGUI_SOURCES})
add_library(ImGui-SFML::ImGui-SFML ALIAS ImGui-SFML) # Add pretty alias
target_include_directories(ImGui-SFML PUBLIC ${IMGUI_DIR})
target_link_libraries(ImGui-SFML
  PUBLIC
    sfml-graphics
    sfml-system
    sfml-window
    ${OPENGL_LIBRARIES}
)
set(IMGUI_SFML_PUBLIC_HEADERS
  "${CMAKE_CURRENT_LIST_DIR}/imgui-sfml/imgui-SFML.h"
  "${CMAKE_CURRENT_LIST_DIR}/imgui-sfml/imgui-SFML_export.h"
)
list(APPEND IMGUI_SFML_PUBLIC_HEADERS "${IMGUI_PUBLIC_HEADERS}")
set_target_properties(ImGui-SFML PROPERTIES
  PUBLIC_HEADER "${IMGUI_SFML_PUBLIC_HEADERS}"
)
target_compile_features(ImGui-SFML PUBLIC cxx_std_11)


add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${IMGUI_DIR} imgui-sfml mazegen)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-window sfml-system ImGui-SFML ${OPENGL_LIBRARIES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)


# copy contetnts of assets
add_custom_command(
  TARGET ${PROJECT_NAME} 
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/assets/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/)
