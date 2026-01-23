add_executable(rtype_server
    src/main.cpp
    # src/Server.cpp
    # src/Config.cpp
    # src/ClientRegistry.cpp
    # src/GameServer.cpp
    # src/GameServerInput.cpp
    # src/GameServerState.cpp
    # src/EntityFactory.cpp
    # src/lobby/LobbyManager.cpp
    # src/lobby/LobbySession.cpp
    # src/systems/EnemySpawnSystem.cpp
    # src/systems/EnemyBoundarySystem.cpp
    # src/systems/HealthSyncSystem.cpp
    # src/systems/NetworkSyncSystem.cpp
)

target_include_directories(rtype_server
    PRIVATE include
)

# Configuration spécifique Windows pour éviter les conflits avec Raylib
if(WIN32)
    target_compile_definitions(rtype_server PRIVATE
        NOGDI
        NOUSER
        WIN32_LEAN_AND_MEAN
    )
endif()

find_package(asio REQUIRED)

target_link_libraries(rtype_server
    PRIVATE engine
    PRIVATE asio::asio
)

target_compile_features(rtype_server PRIVATE cxx_std_20)

configure_file(${CMAKE_SOURCE_DIR}/server/.config ${CMAKE_BINARY_DIR}/.config COPYONLY)

