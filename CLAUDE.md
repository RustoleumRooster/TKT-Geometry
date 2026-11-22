# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

TKT-Geometry is a 3D level editor built on the Irrlicht game engine. It supports constructive solid geometry (CSG) editing, texture/UV tools, node/scene management, and Vulkan-based lightmap generation.

## Build System

This is a Visual Studio 2022 project (`TKT-dev.vcxproj`), built with MSBuild and platform toolset v143 (MSVC).

**To build from the command line:**
```
msbuild TKT-dev.vcxproj /p:Configuration=Debug /p:Platform=x64
```

The x64 configurations are the primary targets — they include Vulkan and AGG font support. Win32 configurations lack Vulkan/AGG.

**Compile shaders** (GLSL → SPIR-V) with `glslc` from the Vulkan SDK:
```
glslc shader_arealight.comp -o shaders/arealight.spv
```

## Dependencies (local paths hardcoded in .vcxproj)

- **Irrlicht**: `C:\Users\nateh\Documents\Visual Studio 2022\irrlicht` (trunk version)
- **Vulkan SDK**: `C:\VulkanSDK\1.3.268.0`
- **Anti-Grain Geometry 2.6**: `..\agg\agg-2.6\agg-src` (relative to project root)
- **Eigen**: `C:\Users\nateh\eigen`
- **STB**: `C:\Users\nateh\Documents\Visual Studio 2022\Libraries\stb`

## Architecture

### Core Geometry

The fundamental data type is the **Polyfold** (`csg_classes.h`): a 3D shape stored as arrays of `poly_vert` (vertices), `poly_edge` (edges referencing vertices), and `poly_face` (faces referencing vertices/edges, composed of Loops). Loops with even depth are outer loops; odd depth are holes.

**Two representations of a Polyfold exist simultaneously:**
- `brush` — the user-editable shape; only changes on user action
- `geometry` — computed from CSG operations against the accumulated scene

A SOA (Structure of Arrays) representation (`polyfold_soa.h`, `soa.h`) is used for performance-critical Vulkan passes and spatial queries.

### CSG Pipeline

`GeometryStack` (`GeometryStack.h`) manages an ordered list of `geo_element` objects, each containing a brush, geometry, and surface data (`poly_surface` with texture/material/lightmap info). Rebuilding clips each brush against the accumulated geometry. BVH acceleration (`BVH.h`, `vk_BVH.h`) is used for high-poly meshes.

`geometry_scene` (`geometry_scene.h`) is the main scene class — it owns the `GeometryStack`, handles selection, manages node placement, and dispatches events. `SceneCoordinator` (global `gs_coordinator`) manages multiple geometry scenes.

Scene I/O is handled by `Geometry_Scene_File_IO`, which serializes geometry, textures, and reflected scene nodes separately.

### Reflection System

`Reflection.h` provides a custom runtime reflection system (no external library). Every reflected type defines:
- `REFLECT()` macro in the class body — adds static `Reflection` and `initReflection` members
- `REFLECT_STRUCT_BEGIN/END` macros in the `.cpp` file — lists member fields via `REFLECT_MEMBER`

The reflection system drives:
- **Serialization/deserialization** (`serialize`/`deserialize` on `TypeDescriptor`)
- **GUI property forms** (`Reflected_Widget_EditArea`, `Reflected_GUI_Edit_Form` in `GUI_tools.h`)
- **Scene node registration** and instantiation

### Scene Nodes

Custom game entities extend `Reflected_SceneNode` and use `REFLECT2()` in the header and `REFLECT_STRUCT2_BEGIN/END` in the `.cpp`. The `create_func` pointer in `TypeDescriptor_SN_Struct` enables the editor to instantiate nodes by type. Nodes with meshes extend `Reflected_Model_SceneNode`. Definitions live in `my_reflected_nodes.h/.cpp`.

### Vulkan Lightmapping

A module/workflow graph pattern (`vkModules.h`, `vkRoutines.cpp`) runs Vulkan compute shaders for lightmap generation. Key classes:
- `Vulkan_Module` — base for compute pipeline stages (e.g., `AreaLight_Module`, `BouncedLight_Module`, `Lightmap_Edges_Module`)
- `Vulkan_Workflow` — graph of connected modules with typed `input_type`/`output_type` ports
- `TypeDescriptor_VkMod_Struct` / `TypeDescriptor_VkWorkflow_Struct` — reflection types that extend the base system to support Vulkan module serialization and GUI representation

The `LightSystemTool` (`LightSystemTool.h/.cpp`) provides the editor UI for building and running lightmap workflows. `Module_GUI_Element` and `System_GUI_Element` are custom `IGUIElement` subclasses that render the node graph.

### Rendering & GUI

- The editor uses Irrlicht's OpenGL renderer with GLSL shaders (`vert_shader_*.txt`, `frag_shader_*.txt`) applied via `ShaderCallbacks.h`
- `BufferManager.h` manages Irrlicht mesh buffers for editor and final geometry meshes
- `CameraPanel` / `CameraPanel2D` manage orthographic and perspective viewports
- Font rendering uses Anti-Grain Geometry (`fonts.h`, `fonts.cpp`); `AGG_TT_Font` renders Windows TrueType fonts to textures; `CachedText` / `CachedText_Manager` batch text draw calls
- GUI layout is managed by `GUI_layout` (global `gui_layout`), with tools inheriting from `tool_base` / `reflected_tool_base`

### Global State

Key globals defined in `main.cpp` and declared `extern` throughout:
- `IrrlichtDevice* device`
- `SceneCoordinator* gs_coordinator`
- `GUI_layout* gui_layout`
- `float g_time`

## Code Conventions

- Header guards use `#ifndef _NAME_H_` style (some files also use `#pragma once`)
- Irrlicht types are used throughout (`core::vector3df`, `video::SColor`, `s32`, `u32`, `f32`, etc.)
- `using namespace irr;` and `using namespace std;` are common in headers
- The `unused/` subdirectory contains superseded Vulkan system implementations kept for reference
