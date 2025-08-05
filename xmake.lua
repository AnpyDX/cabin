set_project("cabin")

set_warnings("all")
set_languages("c++20")
set_allowedplats("windows", "linux")
set_rules("mode.debug", "mode.release")

add_requires("glad", "glfw", "glm", "stb", "tinygltf")
add_requires("imgui", {configs = { glfw = true, opengl3 = true }})
add_packages("glad", "glfw", "glm", "stb", "imgui", "tinygltf")

includes("src", "sandbox")