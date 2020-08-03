# Procyon Engine

A lightweight game engine written in C.

Goals:
- Provide a straightforward, batteries-included toolkit for building roguelike games
- Implement an intuitive, event-based scripting API
- Facilitate simple distribution and (non-existent) asset management by bundling everything into a single executable binary file
- Target OpenGL 3.3 for broad device support

See the `lua` directory for a sample implementation that adds a runtime scripting API.

TODO:
- [x] Creating a window
- [x] Event-based input handling (keyboard only for now)
- [x] Compile-time asset embedding (currently done in CMakeLists.txt)
- [ ] More flexible non-cmake asset pipeline
- [x] Bitmap font rendering
- [x] Colored text
- [ ] Graphics primitives
- [ ] API calls for generating continuous 1D/2D/3D noise (via stb_perlin.h)

