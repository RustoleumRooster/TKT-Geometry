#include <irrlicht.h>
#include <iostream>
#include <sstream>
#include "heightmap_projection.h"
#include "edit_env.h"
#include "ex_gui_elements.h"
#include "geometry_scene.h"
#include "my_reflected_nodes.h"

using namespace irr;
using namespace gui;

extern IrrlichtDevice* device;

Heightmap_Projection_Window* Heightmap_Projection_Window::instance = NULL;
f32 Heightmap_Projection_Window::last_scale = 64.0f;

//true if the brush has terrain tiles (a grid canonical brush)
static bool brush_has_terrain(const polyfold& pf)
{
    for (const surface_group& sg : pf.surface_groups)
        if (sg.type == SURFACE_GROUP_CANONICAL && sg.c_brush.n_cols > 0)
            return true;
    return false;
}

Heightmap_Projection_Window::Heightmap_Projection_Window(IGUIEnvironment* env, IGUIElement* parent, geometry_scene* g_scene_, u64 node_uid_, s32 id, core::rect<s32> rect)
    : gui::CGUIWindow(env, parent, id, rect), g_scene(g_scene_), node_uid(node_uid_)
{
    setText(L"Project Heightmap to Terrain");

    core::rect<s32> cr = getClientRect();
    s32 x0 = cr.UpperLeftCorner.X + 12;
    s32 x1 = cr.LowerRightCorner.X - 12;
    s32 y = cr.UpperLeftCorner.Y + 10;

    env->addStaticText(L"Select terrain brushes in the views.", core::rect<s32>(x0, y, x1, y + 20), false, false, this);
    y += 28;

    env->addStaticText(L"Scale", core::rect<s32>(x0, y + 3, x0 + 60, y + 23), false, false, this);

    std::wstringstream ss;
    ss << last_scale;
    scale_box = env->addEditBox(ss.str().c_str(), core::rect<s32>(x0 + 64, y, x0 + 164, y + 24), true, this);
    y += 34;

    count_text = env->addStaticText(L"", core::rect<s32>(x0, y, x1, y + 20), false, false, this);

    s32 by = cr.LowerRightCorner.Y - 44;

    cancel_button = new Flat_Button(env, this, -1, core::rect<s32>(x1 - 80, by, x1, by + 36));
    cancel_button->setText(L"Cancel");
    cancel_button->drop();

    select_button = new Flat_Button(env, this, -1, core::rect<s32>(x1 - 168, by, x1 - 88, by + 36));
    select_button->setText(L"Select");
    select_button->drop();

    update_count_text();
}

Heightmap_Projection_Window::~Heightmap_Projection_Window()
{
    if (instance == this)
        instance = NULL;

    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();

    SEvent event;
    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1 = USER_EVENT_NODE_SELECTION_ENDED;

    receiver->OnEvent(event);
}

bool Heightmap_Projection_Window::open(geometry_scene* g_scene, Reflected_ReferenceImageNode* node)
{
    if (!g_scene || !node)
        return false;

    //one session at a time; the panel is already frozen by the running one
    if (instance)
    {
        instance->getParent()->bringToFront(instance);
        return true;
    }

    IGUIEnvironment* env = device->getGUIEnvironment();

    instance = new Heightmap_Projection_Window(env, env->getRootGUIElement(), g_scene, node->UID(), -1,
        core::rect<s32>(500, 64, 500 + 340, 64 + 200));
    instance->drop();

    return true;
}

void Heightmap_Projection_Window::update_count_text()
{
    if (!count_text || !g_scene)
        return;

    std::vector<int> sel = g_scene->getBrushSelection();

    int n_terrain = 0;
    for (int b : sel)
        if (b >= 0 && b < (int)g_scene->geoNode()->elements.size() && brush_has_terrain(g_scene->geoNode()->elements[b].brush))
            n_terrain++;

    if ((int)sel.size() == last_count && n_terrain == last_terrain_count)
        return;

    last_count = sel.size();
    last_terrain_count = n_terrain;

    std::wstringstream ss;
    ss << sel.size() << (sel.size() == 1 ? L" brush" : L" brushes") << L" selected, " << n_terrain << L" with terrain";
    count_text->setText(ss.str().c_str());
}

void Heightmap_Projection_Window::draw()
{
    //brush selection is made in the views, so poll it
    update_count_text();

    gui::CGUIWindow::draw();
}

void Heightmap_Projection_Window::apply()
{
    f32 scale = last_scale;
    if (scale_box)
    {
        const wchar_t* txt = scale_box->getText();
        wchar_t* end = NULL;
        f32 v = (f32)wcstod(txt, &end);
        if (end != txt)
            scale = v;
    }
    last_scale = scale;

    //the node could have been deleted while the window was open
    Reflected_ReferenceImageNode* node = dynamic_cast<Reflected_ReferenceImageNode*>(g_scene->get_reflected_node_by_uid(node_uid));
    if (!node)
    {
        std::cout << "Heightmap projection: reference image node no longer exists\n";
        return;
    }

    node->project_onto_terrain(g_scene->geoNode(), g_scene->getBrushSelection(), scale);
}

bool Heightmap_Projection_Window::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_GUI_EVENT && event.GUIEvent.EventType == EGET_BUTTON_CLICKED)
    {
        if (event.GUIEvent.Caller == select_button)
        {
            apply();
            remove();
            return true;
        }
        else if (event.GUIEvent.Caller == cancel_button)
        {
            remove();
            return true;
        }
    }

    return gui::CGUIWindow::OnEvent(event);
}
