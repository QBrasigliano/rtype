cmake_minimum_required(VERSION 3.15)
project(rtype)

# ========== Optimisations macOS/Apple ==========
# Optimisations natives pour Apple Silicon/Intel
add_compile_options(-march=native -mtune=native)
# Vectorisation SIMD
add_compile_options(-fvectorize -fslp-vectorize)
# Optimisations LTO (Link-Time Optimization)
add_compile_options(-flto=thin)
add_link_options(-flto=thin)
# Optimisations supplémentaires
add_compile_options(-ffast-math -funroll-loops)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)
enable_testing()

add_subdirectory(engine)
add_subdirectory(server)