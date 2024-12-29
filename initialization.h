#pragma once

#include "geometry_scene.h"
#include <irrlicht.h>

class GUI_layout;
class multi_tool_panel;
class RenderList;

void initialize_tools(geometry_scene* scene, gui::IGUIEnvironment* gui, multi_tool_panel* tool_panel);
void initialize_materials(geometry_scene* scene);
void initialize_camera_quad(geometry_scene* scene, GUI_layout* gui_layout, RenderList* renderList);
void initialize_set_scene(geometry_scene* scene);