cmake_minimum_required(VERSION 3.15)
project(3DViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(DEFINED ENV{VCPKG_ROOT} AND WIN32)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Поиск FLTK (универсальный способ)
if(NOT FLTK_FOUND)
    find_package(FLTK CONFIG QUIET)
    if(NOT FLTK_FOUND)
        find_package(FLTK REQUIRED)
    endif()
endif()

find_package(OpenGL REQUIRED)

set(SOURCES
    src/main.cpp
    src/models/ModelLoader.cpp
)

set(HEADERS
    src/models/Mesh.h
    src/models/Model3D.h
    src/models/ModelLoader.h
    src/models/ModelManager.h
    src/models/ObjLoader.h
    src/renders/OpenGLWindow.h
    src/renders/ModelRenderer.h
    src/analyzer/MeshQualityAnalyzer.h
)

if(WIN32)
    add_executable(3d-model-viewer WIN32 ${SOURCES} ${HEADERS})
else()
    add_executable(3d-model-viewer ${SOURCES} ${HEADERS})
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

target_link_libraries(3d-model-viewer PRIVATE
    fltk
    fltk_gl
    OpenGL::GL
    OpenGL::GLU
)

if(WIN32)
    target_link_libraries(3d-model-viewer PRIVATE comctl32)

    if(NOT VCPKG_TARGET_TRIPLET MATCHES "-static" AND DEFINED VCPKG_INSTALLED_DIR)
        find_file(FLTK_DLL NAMES fltk.dll PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
        if(FLTK_DLL)
            add_custom_command(TARGET 3d-model-viewer POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${FLTK_DLL}"
                $<TARGET_FILE_DIR:3d-model-viewer>)
        endif()
    endif()
endif()

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(3d-model-viewer PRIVATE X11 Xext)
endif()

target_include_directories(3d-model-viewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${FLTK_INCLUDE_DIRS}
)