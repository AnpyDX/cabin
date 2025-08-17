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

### 2. Create Sandbox

All sandbox-apps are located in `sandbox` directory. The folder inside `sandbox` will be considered as a sandbox-app only if it contains a `xmake.lua`. To make the tutorial  simple, here is a most basic sandbox-app as an example.

A sandbox-app folder will look like this:

```txt
|- \my_sandbox
   |- xmake.lua
   |- main.cc
   |- ... (e.g. shaders)
```

- `xmake.lua`

```lua
target("my_sandbox") -- Sandbox target name
    set_kind("binary")
    add_files("*.cc")
```

- `main.cc`

```cpp
#include <GLFW/glfw3.h>
#include "cabin/sandbox"
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

### 3. Run Sandbox

Change to cabin's **root directory**. Then buuild and run sandbox-app with the command below:

```bash
xmake run <target>
```

The `<target>` depends on the name provided by `target("...")` in `xmake.lua`.

For this tutorial's context, the command should be:

```bash
xmake run my_sandbox
```

**NOTICE**: The default workspace directory is `sandbox`, which is important while loading external assets by specifying relative paths.

### 4. Advance

To learn how to use cabin's OpenGL wrappers and utilities, and the shader syntax, please check the [Hello-Triangle](hello_triangle) sample.
