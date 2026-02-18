add_executable(rtype_client
    src/main.cpp
    src/Client.cpp

target_include_directories(rtype_client
    PRIVATE include
    PRIVATE ${CMAKE_SOURCE_DIR}/engine/include
)

find_package(asio REQUIRED)

target_link_libraries(rtype_client
    PRIVATE engine
    PRIVATE asio::asio
)

target_compile_features(rtype_client PRIVATE cxx_std_20)

configure_file(${CMAKE_SOURCE_DIR}/server/.config ${CMAKE_BINARY_DIR}/.config COPYONLY)
