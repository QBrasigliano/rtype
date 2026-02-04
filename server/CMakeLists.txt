add_executable(rtype_server
    src/main.cpp
    src/Protocol.cpp
    src/network.cpp
)

target_include_directories(rtype_server
    PRIVATE include
)

find_package(asio REQUIRED)

target_link_libraries(rtype_server
    PRIVATE engine
    PRIVATE asio::asio
)

target_compile_features(rtype_server PRIVATE cxx_std_20)

configure_file(${CMAKE_SOURCE_DIR}/server/.config ${CMAKE_BINARY_DIR}/.config COPYONLY)

