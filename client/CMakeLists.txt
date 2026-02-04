cmake_minimum_required(VERSION 3.20)

add_executable(rtype_client
    src/main.cpp
    src/Client.cpp
    ${CMAKE_SOURCE_DIR}/server/src/Protocol.cpp
)

target_include_directories(rtype_client
    PRIVATE include
    PRIVATE ${CMAKE_SOURCE_DIR}/engine/include
)

find_package(raylib REQUIRED)
find_package(asio REQUIRED)

target_link_libraries(rtype_client
    PRIVATE engine
    PRIVATE raylib
    PRIVATE asio::asio
)

target_compile_features(rtype_client PRIVATE cxx_std_20)

add_custom_command(TARGET rtype_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets to build directory"
)
