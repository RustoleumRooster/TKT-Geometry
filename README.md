# TKT-Geometry version by Rustoleum Rooster aka Rusty Rooster
Code is copyright 2022-2024. Version 0.9

TKT is a full featured level editor built on the Irrlicht 3D game engine, intended to provide a way to easily create maps for Irrlicht games. The philosphy behind TKT is that every game should have a level editor which is closely integrated with the engine and with the game's own functionality. TKT encompasses geometry and physical level design, lighting and lightmaps, as well as the ability to create complex scenes with models, actors, effects, factories, etc, and can be extended to support new kinds of materials and game logic.

# Features
1. Create and edit geometry using constructive solid geometry (CSG). Brushes can be added and subtracted to create arbitrary geometry. Brushes can be primitives such as cubes, spheres, etc, or more complex brushes can be created by intersecting or clipping against existing geometry. Brushes can be moved, rotated, or vertex edited and the geometry can be rebuilt around it, so that it's easy to make changes.

2. Select and apply textures to any surface. Rotate, scale, and skew textures, or use the UV editor to texture complex shapes like arches.

3. The geometry builder will create triangulated, opitimized meshes; all geometry using the same texture (or material texture group, or lightmap group) will be grouped into one mesh buffer, in order to minimize the number of draw calls.

4. Add, position, and edit the properties of Nodes (which is what the Irrlicht engine calls its game entities).

5. Easily support custom Nodes using the built in Reflection system.

6. Create lightmaps. Lightmap creation uses Vulkan. Currently only "ambient occlusion" lighting is supported. (Vulkan code not uploaded to github yet).

7. Export level geometry, triangulated meshes, and nodes to binary files, which are easily imported into any other program by using the Reflection system.

8. Optimizations for high-poly geometry significantly speed up processing of high-poly objects without affecting normal, low-poly use cases.


# Build Requirements

Irrlicht engine trunk version, available at https://irrlicht.sourceforge.io/ (may be compatible with Irrlicht 1.84).

Currently exists Eigen (https://eigen.tuxfamily.org/) dependency, but this will be removed soon as lighting calculations are being moved to Vulkan.

C++ 14

# Implementation details

The basic datatype is a Polyfold, which represents a 3d shape. A Polyfold has an array of Vertices, Edges, and Faces. Edges reference Vertices. Faces reference Vertices, Edges, and are composed of Loops. Loops reference vertices and are defined by their depth, which if even means it's an "outside" loop, if odd means it's a "hole" loop. A polyfold can represent either a brush or geometry. The difference is that a brush doesn't change unless it's moved, edited, deleted etc by the user. Geometry, on the oher hand, is what is calculated from combinations of brushes. The GeometryScene class manages the geometry stack; each element on the stack has a brush and a geometry component. When we rebuild the scene geometry, we iterate over the brushes, clipping each brush against the geometry built so far and adding that geometry to the scene. After rebuilding, triangulated meshes are computed from the resulting geometry. We can also continuiously build with each newly added or subtracted brush, as long as we're only adding new brushes to the geometry stack; but if we make changes to existing brushes then it is necessary to rebuild the whole scene to see the changes. Clipping geometry is the most computationally expensive and logically complicated part of the geometry build process. Hi-poly meshes in many cases will make use of spatial acceleration structures (bounding volume hierarchies) to speed up some key calculations.

A geometry scene can either be empty or solid. In an empty scene, we start by adding brushes, as if in empty space. A solid scene, on the other hand, is like a block of clay, and we start by subtracting, i.e. carving a space. An empty scene is suitable for modeling something like a single object. A solid scene is suitable for modeling a dungeon, and is often the best choice for level building, since we usually fake the outside world with skyboxes anyways.

Internally, two meshes are actually generated, one with a separate mesh buffer for every face for editing (to easily allow changing textures and highlighting faces), and a separate final mesh which is optimized with all faces that share textures/materials/lightmaps grouped into a single meshbuffer, and thus suitable for minimizing draw calls in production. 

If we have non-geometry objects (known in Irrlicht as SceneNodes or just Nodes), whether lights or any other type of node we might want to implement, we can easily add them to the Reflection system and place, manipulate, and edit their properties within the editor environment. If the node has a mesh, it can inherit the Reflected_Model_SceneNode class so it will appear in the editor as it will in the game. Nodes can also use sprites, for example lights or other abstract things we might want to add to the level. When we actually create the scene, the Reflected Node's addSelfToScene function is called to initialize the game Node, for example game logic such as spawning a particle factory.

Since everything is tracked by the Reflection system, we can easily export geometry as well as Node data into the game itself or any other program just by copy/pasting the reflection code and the reflected class definitions.

Most of the editor widgets are also implemented using the reflection system.

