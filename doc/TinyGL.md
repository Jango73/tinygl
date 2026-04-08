# TinyGL OS Integration Guide

## Purpose

This document explains how to integrate TinyGL into an operating system or
graphics stack.

It does not explain how to use the OpenGL-style rendering API itself. It
focuses on:

- static and shared library integration
- host-side responsibilities
- surface attachment and presentation
- context lifetime
- minimal adapter structure

## Integration Model

TinyGL is a CPU renderer. It does not create windows, display devices, swap
chains, or compositor objects.

The host OS must provide the environment around the library:

- load or link the library
- create a TinyGL context
- allocate or map a pixel buffer
- attach that buffer to TinyGL
- decide how rendered pixels are copied or presented
- destroy the context and free host-owned resources

TinyGL is responsible for:

- storing rendering state
- rasterizing into the attached color buffer
- managing its own internal fallback surface when requested
- maintaining its own control-path and GL error state

## Public Integration Boundary

Host code must use only the public header:

- `include/tinygl.h`

Host code must not include anything from:

- `source/internal/`

The main public types involved in integration are:

- `TGLContext`
- `TGL_CONTEXT_DESC`
- `TGL_SURFACE_DESC`
- `TGL_SURFACE_MODE`
- `TGL_BRIDGE_CALLBACKS`
- `TGL_RESULT`
- `TGL_ERROR`
- `TGL_PRESENT_RESULT`

## Static And Shared Integration

### Static Library

Use the static library when the OS build system wants TinyGL to be linked
directly into a kernel module, graphics server, or user-space component.

Typical integration properties:

- no runtime loader logic is needed
- symbol resolution happens at link time
- deployment is simpler when the whole system image is built together

Artifacts:

- `libtinygl.a`
- `include/tinygl.h`

Typical host-side link model:

- add `include/` to the include path
- link against `libtinygl.a`
- also link the standard math library if your toolchain requires it

### Shared Library

Use the shared library when the OS or graphics server wants TinyGL to remain a
replaceable runtime component.

Typical integration properties:

- the library can be upgraded independently of the host binary
- multiple host components may share the same mapped library image
- the host can choose when and how the library is loaded

Artifacts:

- `libtinygl.so`
- `include/tinygl.h`

Typical host-side link model:

- add `include/` to the include path
- link against `libtinygl.so`, or load it dynamically through the OS loader
- ensure the runtime loader can resolve the library path

## Build Outputs

On Linux, the current build produces both forms:

- `build/libtinygl.a`
- `build/libtinygl.so`

Build command:

```bash
bash scripts/linux/build
```

Global verification command:

```bash
bash scripts/linux/smoke-test-global.sh
```

That verification checks:

- the library builds successfully
- the shared object exports expected symbols
- installation works
- an external program can link against the installed artifacts

## Host Responsibilities

The host OS or graphics stack is responsible for:

- library lifetime
- context lifetime
- framebuffer allocation or mapping
- presentation policy
- resize handling
- synchronization with the rest of the display pipeline

TinyGL must remain isolated from OS-native graphics concepts such as:

- windows
- swap chains
- display controllers
- compositors
- GPU driver handles
- graphics server protocols

## Surface Ownership Model

TinyGL supports two surface modes.

### Host-Provided Surface

This is the preferred mode for a real OS integration.

The host allocates or maps a pixel buffer and describes it with
`TGL_SURFACE_DESC`.

Required fields:

- `Pixels`
- `Width`
- `Height`
- `Pitch`
- `PixelFormat`

Rules:

- the host owns the memory
- the host must keep the memory valid while attached
- TinyGL must not free that memory
- the host must reattach a new surface after resize or reallocation

### TinyGL-Owned Surface

This is a fallback mode intended for tests, early bring-up, or very simple
embedding scenarios.

The host requests an internal surface with `tinyglCreateInternalSurface()` and
can inspect it with `tinyglGetSurface()`.

Rules:

- TinyGL owns the memory
- the host must treat the returned pixel pointer as borrowed memory
- the host must not free or resize TinyGL-owned memory

## Presentation Model

TinyGL renders into memory. It does not decide how pixels reach the screen.

The host may choose one of these models:

- read or copy the attached buffer directly
- provide a `Present` callback through `TGL_BRIDGE_CALLBACKS`
- feed the rendered buffer into another framebuffer or compositor input

`tinyglPresent()` is only a hand-off point between TinyGL and the host adapter.
It is not a swap operation in the window-system sense.

## Error Model

The host must treat TinyGL control errors and OpenGL-style command errors as
two separate channels.

Control-path side:

- `TGL_RESULT`
- `tinyglGetLastError()`

Rendering API side:

- `glGetError()`

The host must not depend on any OS-native error slot to understand TinyGL
failures.

## Recommended Integration Sequence

### 1. Link Or Load The Library

Choose one of these models:

- static: link against `libtinygl.a`
- shared: link against `libtinygl.so` or load it through the OS runtime loader

Always include only:

- `include/tinygl.h`

### 2. Create A Context

Fill a `TGL_CONTEXT_DESC` and create a context with:

- `tinyglCreateContext()`

Then make it current:

- `tinyglMakeCurrent()`

At minimum, the host should define:

- `MaxWidth`
- `MaxHeight`
- `HasDepthBuffer`
- `HasColorBuffer`

### 3. Attach A Surface

Preferred path:

- allocate or map a host framebuffer
- fill a `TGL_SURFACE_DESC`
- call `tinyglSetSurface()`

Fallback path:

- call `tinyglCreateInternalSurface()`
- inspect the result with `tinyglGetSurface()`

### 4. Render

Once the context is current and a surface is attached, the host can use the
OpenGL-style API exposed by TinyGL.

Examples:

- `glViewport()`
- `glClear()`
- `glBegin()`
- `glVertex*()`
- `glEnd()`
- `glFlush()`

### 5. Present Or Copy

After rendering, the host may:

- call `tinyglPresent()`
- copy the pixel buffer to another framebuffer
- expose the buffer to a compositor or graphics server

### 6. Handle Resize

When the framebuffer size or backing memory changes:

- create or map a new host surface
- update `TGL_SURFACE_DESC`
- reattach it with `tinyglSetSurface()`
- update the GL viewport as needed

### 7. Destroy Resources

When shutting down:

- stop using the context
- destroy any TinyGL-owned internal surface if it was created explicitly
- call `tinyglDestroyContext()`
- free host-owned buffers on the host side

## Minimal Host Adapter Shape

A small adapter layer is enough. In practice, it can stay split into three
parts:

- one unit for loading or linking TinyGL and managing `TGLContext` lifetime
- one unit for translating host framebuffer metadata into `TGL_SURFACE_DESC`
- one optional unit for `TGL_BRIDGE_CALLBACKS`

That adapter should be the only place where TinyGL-specific integration details
exist. The rest of the OS graphics stack should see a simple framebuffer-based
renderer interface.

## Practical Recommendations

- Prefer host-provided surfaces for any serious OS integration.
- Keep TinyGL outside the windowing or compositor policy layer.
- Treat the shared library and static library as equivalent rendering backends
  with different deployment models.
- Keep TinyGL isolated behind a small adapter so the host can replace it later
  if needed.
- Use the Linux scripts as build helpers only, not as an OS design model.
