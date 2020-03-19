cmake_minimum_required(VERSION 3.10)

project(lifesim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)


find_package(OpenMP)


include_directories(SFML-2.5.1/include)
link_directories(SFML-2.5.1/lib)

set(SFML_LIBS sfml-system sfml-window sfml-graphics)

add_library(supportlib STATIC support.cpp)

add_executable(lifesim lifesim.cpp matrix.cpp)
target_compile_options(lifesim PUBLIC -Wall -Werror -Wextra -O3)


target_link_libraries(lifesim PUBLIC supportlib)
target_link_libraries(lifesim PUBLIC ${SFML_LIBS})
target_link_libraries(lifesim PUBLIC OpenMP::OpenMP_CXX)