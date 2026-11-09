set_project("cabin")

set_warnings("all")
set_languages("c++20")
set_allowedplats("windows", "linux")
set_rules("mode.debug", "mode.release")

add_requires(
    "glad 2.0.8", 
    "glfw 3.4", 
    "glm 1.0.3", 
    "stb 2026.03.18", 
    "tinygltf 3.0.0"
)
add_requires("imgui", {configs = { glfw = true, opengl3 = true }})
add_packages("glad", "glfw", "glm", "stb", "imgui", "tinygltf")

includes("src", "sandbox")