cmake_minimum_required(VERSION 3.5)

project(cropswc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cropswc main.cpp
    basic_surf_objs.h
    c_array_struct.h color_xyz.h
    v3d_basicdatatype.h
    neuron_format_converter.h
    v_neuronswc.h
    basic_surf_objs.cpp
    neuron_format_converter.cpp
    v_neuronswc.cpp
)
