#pragma once
#ifndef _VK_MODULE_PROPERTIES_H_
#define _VK_MODULE_PROPERTIES_H_

#include "tkt_properties.h"
#include "vkModules.h"

//====================================================
// VkModule_Properties_Base
//
// Property editor for the selected modules of the current scene's lightmap
// workflow. Not a tool of its own: Light_System_Base owns it and embeds its
// widget in the Lighting panel (showing a separate tool would close the
// Lighting panel, and with it the graph view).
//
// Ports and the module uid are hidden from the form (HIDE_IN_FORMS / FLAG_HIDDEN
// in vkModules.cpp); modules expose settings by reflecting ordinary members.
//

class VkModule_Properties_Base : public tkt_properties_base<Vulkan_Module>
{
public:
    VkModule_Properties_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel)
        : tkt_properties_base<Vulkan_Module>(name, my_id, env, panel, NULL)
    {}

    // the source is the scene's workflow, not the scene itself
    virtual void set_scene(geometry_scene* gs) override;
};

using VkModule_Properties_Widget = tkt_properties_widget<Vulkan_Module>;

#endif
