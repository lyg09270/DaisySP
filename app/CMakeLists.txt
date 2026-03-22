cmake_minimum_required(VERSION 3.14)

add_executable(daisy_app
    main.cpp
)

add_executable(drum
    drum.cpp
)


target_link_libraries(daisy_app
    DaisySP
    asound
)

target_link_libraries(drum
    DaisySP
    asound
)

target_include_directories(daisy_app PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
)

target_include_directories(drum PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
)