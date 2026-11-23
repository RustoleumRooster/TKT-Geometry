#include "vk_module_properties.h"

void VkModule_Properties_Base::set_scene(geometry_scene* gs)
{
    tool_base::set_scene(gs);
    set_source(gs ? gs->get_lightmap_workflow() : NULL);
}
