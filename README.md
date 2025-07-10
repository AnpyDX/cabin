# Cabin

**Cabin** is a grahpics *sandbox* framework based on OpenGL.

## Showcase

- `sandbox/hello_triangle`

![](showcase/hello_triangle.png)

## Build & Run

1. Build framework and sandbox:

```bash
xmake config -m release
xmake build
```

2. Run sandbox by specifying its name (e.g. `hello_triangle`):

```bash
xmake run <target>
```

## Structure

- `src`: Source code of cabin framework.
- `sandbox`: Sandbox collections.

## Thirdparty

- **GLFW**: https://github.com/glfw/glfw
- **GLAD**: https://github.com/Dav1dde/glad
- **GLM**: https://github.com/g-truc/glm

## References

- **LearnOpenGL-CN**: https://learnopengl-cn.github.io

## License

Licensed under the MIT license, check [LICENSE](LICENSE) for details.
