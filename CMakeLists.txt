cmake_minimum_required(VERSION 3.15)
project(rtype)

if(WIN32)
    add_compile_definitions(
        NOGDI                    # Évite les conflits CloseWindow/ShowCursor avec Raylib
        NOUSER                   # Évite les conflits avec les fonctions utilisateur Windows
        WIN32_LEAN_AND_MEAN      # Réduit la taille des headers Windows
        _WIN32_WINNT=0x0A00      # Windows 10 (au lieu de 0x0601 pour Windows 7)
    )
endif()

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)
enable_testing()

add_subdirectory(engine)
add_subdirectory(server)
add_subdirectory(client)