cmake_minimum_required (VERSION 3.5)
project (Test)
set(HOMEBREW_PREFIX "/opt/homebrew"
    CACHE PATH "Path to Homebrew installation")
set(CMAKE_PREFIX_PATH 
    "/Users/chrisschilling/Downloads/wxWidgets-3.2.6/build-mac"
    "/usr/local"
    # "${HOMEBREW_PREFIX}"
    # # These libraries are keg-only and not loaded into
    # # the root prefix by default (to avoid clashes).
    # "${HOMEBREW_PREFIX}/opt/lapack"
    # "${HOMEBREW_PREFIX}/opt/openblas"
    # "${HOMEBREW_PREFIX}/opt/gcc/lib/gcc/11"
)
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/;${CMAKE_MODULE_PATH}")
list(TRANSFORM CMAKE_PREFIX_PATH APPEND "/include"
     OUTPUT_VARIABLE CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES)
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES "${CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(wxWidgets REQUIRED COMPONENTS net core base gl)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(log4cpp REQUIRED)
include_directories( ${log4cpp_INCLUDE_DIRS} )
link_directories( ${log4cpp_LIBRARIES} )
message("Log4Cpp available ${LOG4CPP_LIBRARIES}")

include(${wxWidgets_USE_FILE})

add_executable(wxgl 
    graphics/DrawGLUtils.cpp 
    graphics/DrawGLUtils.h 
    graphics/xlGLCanvas.cpp 
    graphics/xlGLCanvas.h 
    graphics/xlGraphicsAccumulators.cpp 
    graphics/xlGraphicsAccumulators.h 
    graphics/xlGraphicsContext.h 
    graphics/xlOGL3GraphicsContext.cpp 
    graphics/xlOGL3GraphicsContext.cpp 
    graphics/ogl_error.h
    graphics/ogl.cpp
    graphics/ogl.h
    graphics/shader.cpp
    graphics/shader.h
    Color.cpp
    Color.h 
    wxgl.cpp     
    wxgl.h
    stb/stb_image.h
)
target_link_libraries(wxgl PRIVATE ${wxWidgets_LIBRARIES} GLEW::GLEW ${LOG4CPP_LIBRARIES})


