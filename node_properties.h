#pragma once
#ifndef _NODE_PROPERTIES_H_
#define _NODE_PROPERTIES_H_

#include "tkt_properties.h"
#include "reflected_nodes.h"

using Node_Properties_Base   = tkt_properties_base<Reflected_SceneNode>;
using Node_Properties_Widget = tkt_properties_widget<Reflected_SceneNode>;

class NodeProperties_Tool
{
public:
    static void initialize(Node_Properties_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }

    static void show()
    {
        panel->add_tool(base);
    }

    static Node_Properties_Base* get_base()
    {
        return base;
    }

    static Node_Properties_Base* base;
    static multi_tool_panel* panel;
};

#endif
