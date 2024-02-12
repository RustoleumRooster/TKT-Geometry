# TKT-Geometry by Rustoleum Rooster aka Rusty Rooster
# Code is copyright 2022-2024

TKT is a full featured level editor built on the Irrlicht 3D game engine, intended to provide a way to easily create maps for Irrlicht games. Every game should have a level editor which is closely integrated with the engine and with the game's own functionality

Features:
1. Create and edit geometry using constructive solid geometry (CSG). Brushes can be added and subtracted to create arbitrary geometry. Brushes can be primitives such as cubes, spheres, etc, or more complex brushes can be created by intersecting or clipping against existing geometry. Brushes can be moved, rotated, or vertex edited and the geometry can be rebuilt around it, so that it's easy to make changes.

2. Select and apply textures to any surface. Rotate, scale, and skew textures, or use the UV editor to texture complex shapes like arches.

3. The geometry builder will create triangulated, opitimized meshes; all geometry using the same texture (or material texture group, or lightmap group) will be grouped into one mesh buffer, in order to minimize the number of draw calls.

4. Add, position, and edit the properties of Nodes (which is what the Irrlicht engine calls its game entities).

5. Easily support custom Nodes using the built in Reflection system.

6. Create lightmaps. Lightmap creation uses Vulkan. Currently only "ambient occlusion" lighting is supported. (Lightmap code not uploaded to github yet).

7. Export level geometry, triangulated meshes, and nodes to binary files, which are easily imported into any other program by using the Reflection system.

8. Optimizations for high-poly geometry significantly speed up processing of high-poly objects without affecting normal, low-poly use cases.
