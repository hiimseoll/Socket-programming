cmake_minimum_required(VERSION 3.5)
project(finale_20205189)

add_executable(server.out server.c)

foreach(index RANGE 1 4)
    add_executable(c-term0${index} c-term.c)
    target_compile_definitions(c-term0${index} PRIVATE -DC${index})
endforeach()

foreach(index RANGE 1 4)
    add_executable(c-disp0${index} c-disp.c)
    target_compile_definitions(c-disp0${index} PRIVATE -DC${index})
endforeach()
