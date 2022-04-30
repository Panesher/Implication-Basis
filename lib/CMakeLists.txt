cmake_minimum_required(VERSION 3.8)

project(lib)

find_package(Threads REQUIRED)

add_library(Utils STATIC Utils.h Utils.cpp)
add_library(Oracle STATIC Oracle.h Oracle.cpp)
add_library(BasisCalculator STATIC BasisCalculator.h BasisCalculator.cpp)

target_link_libraries(BasisCalculator Oracle Utils Threads::Threads)
