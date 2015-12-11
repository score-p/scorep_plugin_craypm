cmake_minimum_required(VERSION 2.8)
project(pm_plugin)

option(ENABLE_FRESHNESS "Enables measurement of freshness counter" ON)

set(PLUGIN_SOURCE pm_plugin.cpp)
set(LIB_NAME pm)
set(LIB_SOURCE pm_lib.c)


include(common/FindScorep.cmake)
if(SCOREP_FOUND)
    include_directories(${SCOREP_INCLUDE_DIRS})    
    link_directories(${SCOREP_LIBRARY_DIRS})
    set(CMAKE_INSTALL_PREFIX ${SCOREP_ROOT_PATH} CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
else()
    message(SEND_ERROR "Score-P was not found but is required!")
endif()


if(ENABLE_FRESHNESS)
    add_definitions("-DENABLE_FRESHNESS_COUNTER")
endif()


#additional cxx flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -D_GLIBCXX_USE_NANOSLEEP")

add_library(${LIB_NAME} SHARED ${LIB_SOURCE})
add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCE})
target_link_libraries(${PROJECT_NAME} ${LIB_NAME})

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
install(TARGETS ${LIB_NAME} LIBRARY DESTINATION lib)
