#ifndef _HEIGHTMAP_PROJECTION_H_
#define _HEIGHTMAP_PROJECTION_H_

#include <irrlicht.h>
#include "CGUIWindow.h"

using namespace irr;

class geometry_scene;
class Flat_Button;
class Reflected_ReferenceImageNode;

//"Select brushes" session opened from a Reference Image node's properties.
//The user selects brushes in the view windows as usual, then presses Select to
//project the heightmap onto the terrain tile vertices of the selected brushes,
//or Cancel. Closing the window (by any means) ends the session by sending
//USER_EVENT_NODE_SELECTION_ENDED, which restores the properties panel's node selection.
class Heightmap_Projection_Window : public gui::CGUIWindow
{
public:
    Heightmap_Projection_Window(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene, u64 node_uid, s32 id, core::rect<s32> rect);
    ~Heightmap_Projection_Window();

    //Returns true if a session is now running
    static bool open(geometry_scene* g_scene, Reflected_ReferenceImageNode* node);

    virtual void draw() override;
    virtual bool OnEvent(const SEvent& event) override;

private:
    void apply();
    void update_count_text();

    geometry_scene* g_scene = NULL;
    u64 node_uid = 0;

    gui::IGUIEditBox* scale_box = NULL;
    gui::IGUIStaticText* count_text = NULL;
    Flat_Button* select_button = NULL;
    Flat_Button* cancel_button = NULL;

    int last_count = -1;
    int last_terrain_count = -1;

    static Heightmap_Projection_Window* instance;
    static f32 last_scale;
};

#endif
