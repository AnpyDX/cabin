set_project("cabin")

set_warnings("all")
set_languages("c++20")
set_allowedplats("windows", "linux")
set_rules("mode.debug", "mode.release")

add_requires("glad", "glfw", "glm")
add_packages("glad", "glfw", "glm")

includes("src", "sandbox")