# TinyGL

TinyGL is a compact CPU implementation of OpenGL.

It provides a public C API in [include/tinygl.h](/home/jango/code/tinygl/include/tinygl.h)
and builds as both a shared library (`libtinygl.so`) and a static library
(`libtinygl.a`).

TinyGL is a software renderer. It does not create windows or manage display
devices. The host application is responsible for context lifetime, surface
allocation, and presentation.

## Build

Requirements:

- CMake 3.16 or newer
- A C compiler
- SDL2 only if you want to build and run the sample application

Build the project with:

```bash
bash scripts/linux/build
```

Artifacts are generated in `build/`.

You can run the global smoke test with:

```bash
bash scripts/linux/smoke-test-global.sh
```

## Run The Sample

The sample application is `tinygl_demo_sdl2`.

If SDL2 is available on the system, it is built automatically by the standard
build. You can run it with:

```bash
bash scripts/linux/run.sh
```

Useful optional commands:

```bash
bash scripts/linux/clean
bash scripts/linux/rebuild
bash scripts/linux/update-version 0.1.0
```

## Integration

For integration details and the host-side responsibilities, see
[doc/TinyGL.md](/home/jango/code/tinygl/doc/TinyGL.md).
