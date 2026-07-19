cmake_minimum_required(VERSION 3.10)
project(simPhysique)

# On veut du C++17 (comme dans tes commandes g++ actuelles)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cherche GLFW installé sur le système (via pacman)
find_package(glfw3 REQUIRED)

# Liste tous les fichiers sources à compiler ensemble, en un seul exécutable
add_executable(simPhysique
    src/main.cpp
    src/gl.c
    src/physique.cpp
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

# Équivalent des -I de g++ : où chercher les headers
target_include_directories(simPhysique PRIVATE
    include
    external/imgui
    external/imgui/backends
)

# Équivalent des -l de g++ : quelles bibliothèques lier
target_link_libraries(simPhysique PRIVATE glfw dl)