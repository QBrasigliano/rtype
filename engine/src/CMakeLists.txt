add_library(engine
    main.cpp
)

target_include_directories(engine PUBLIC ../include)
target_link_libraries(engine PUBLIC raylib asio::asio)
