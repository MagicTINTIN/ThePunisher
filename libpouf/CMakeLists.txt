cmake_minimum_required(VERSION 3.5)

project(pouf)


# add_library(pouf STATIC ${SOURCES})
add_library(${PROJECT_NAME} STATIC libpouf.c)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# install(TARGETS ${PROJECT_NAME} DESTINATION .)
