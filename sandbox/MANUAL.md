# Sandbox Develop Manual

## Getting Started

This tutorial will show you how to create a basic sandbox-app
with cabin framework.

### 1. Preperation

First, make sure your system has satisfied the following requirements:

- Windows or Linux
- OpenGL 4.6 Support
- Has `C++20` compiler and latest [xmake](https://xmake.io) installed

Then, get [cabin](https://github.com/anpydx/cabin)'s source by `git` or downloading ZIP directly.

### 2. Create a Sandbox-App

All sandbox-apps are located in `sandbox` directory. The folder inside `sandbox` will be identified as a sandbox-app only if it contains a `xmake.lua`. To make the tutorial  simple, here is a most basic sandbox-app as an example.

A sandbox-app folder will look like this:

```txt
|- sandbox/
|   |- my_sandbox/
|   |   |- main.cc
|   |   |- xmake.lua
|   |   |- ... (misc, e.g. shaders)
|   |
|   |- ... (other sandbox-apps)
```

- `xmake.lua`

```lua
target("my_sandbox") -- App's target name
    set_rundir(".")
    set_kind("binary")
    add_files("main.cc")
```

- `main.cc`

```cpp
#include <GLFW/glfw3.h>

#include <cabin/sandbox.h>
using namespace cabin;

class MyApp: public Sandbox {
public:
    MyApp() : Sandbox("Title", 800, 600) {}

    void renderFrame() override {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }
};

int main() {
    return SandboxApp<MyApp>::run();
}
```

### 3. Run the App

Change to cabin's **root directory**. Then buuild and run sandbox-app with the command below:

```bash
xmake run <target>
```

The `<target>` depends on the name provided by `target("...")` in `xmake.lua`.

According to our previous `xmake.lua`, the command should be:

```bash
xmake run my_sandbox
```

### 4. Next Step

To further learn the details and usage of cabin's OpenGL wrappers and utilities, and the shader syntax, please check the [Hello-Triangle](hello_triangle) and other samples in `sandbox/`.
