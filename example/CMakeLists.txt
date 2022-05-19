cmake_minimum_required(VERSION 3.5)

project (external_app)

set(CMAKE_CXX_STANDARD 20)

find_package(libpolaris REQUIRED)

include_directories( 
  ${LIBPOLARIS_INCLUDE_DIRS} 
)

add_executable(${PROJECT_NAME}
        external_app.cpp)

target_link_libraries(${PROJECT_NAME}
  libpolaris
)