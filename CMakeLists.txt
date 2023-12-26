cmake_minimum_required(VERSION 3.20)
project(sandbg)

set(CMAKE_CXX_STANDARD 20)

include_directories(/usr/local/include/libelfin/elf/ /usr/local/include/libelfin/dwarf/)

add_executable(sandbg sandbg.cpp)
target_link_libraries(sandbg linenoise /usr/local/lib/libelf++.so /usr/local/lib/libdwarf++.so)

add_executable(test_program examples/test_program.cpp)
set_target_properties(test_program PROPERTIES COMPILE_FLAGS "-g -gdwarf-4 -O0")

add_executable(hello_world examples/hello_world.cpp)
set_target_properties(hello_world PROPERTIES COMPILE_FLAGS "-g -gdwarf-4 -O0")

add_executable(elfin_eg_dump_lines examples/elfin_eg_dump_lines.cpp)
set_target_properties(elfin_eg_dump_lines PROPERTIES COMPILE_FLAGS "-g -O0")
target_link_libraries(elfin_eg_dump_lines /usr/local/lib/libelf++.so /usr/local/lib/libdwarf++.so)

add_executable(elfin_eg_dump_internals examples/elfin_eg_dump_internals.cpp)
set_target_properties(elfin_eg_dump_internals PROPERTIES COMPILE_FLAGS "-g -O0")
target_link_libraries(elfin_eg_dump_internals /usr/local/lib/libelf++.so /usr/local/lib/libdwarf++.so)