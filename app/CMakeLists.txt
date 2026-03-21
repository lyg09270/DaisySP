cmake_minimum_required(VERSION 3.14)

add_executable(daisy_app
    main.cpp
)

target_link_libraries(daisy_app
    DaisySP
)

target_include_directories(daisy_app PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
)