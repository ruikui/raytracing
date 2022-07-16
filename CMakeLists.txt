cmake_minimum_required(VERSION 3.22)

project(Project VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_subdirectory(ThirdParty)
add_subdirectory(InOneWeekend)