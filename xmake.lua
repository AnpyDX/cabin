set_project("cabin")

set_warnings("all")
set_languages("c++20")
set_allowedplats("windows", "linux")
set_rules("mode.debug", "mode.release")

add_requires("glad", "glfw", "glm", "stb")
add_requires("imgui", {configs = { glfw = true, opengl3 = true }})
add_requires("assimp", {configs = { no_export = true }})
add_packages("glad", "glfw", "glm", "stb", "imgui", "assimp")

includes("src", "sandbox")