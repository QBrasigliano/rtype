add_executable(rtype_client
    main.cpp
    src/Client.cpp
    src/Protocol.cpp
)

target_include_directories(rtype_client
    PRIVATE include
)

find_package(asio REQUIRED)

target_link_libraries(rtype_client
    PRIVATE asio::asio
)

target_compile_features(rtype_client PRIVATE cxx_std_20)
