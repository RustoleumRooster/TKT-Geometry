#ifndef _EDIT_ENV_H_
#define _EDIT_ENV_H_

#include <irrlicht.h>
#include <list>

using namespace irr;

bool GetPlaneClickVector(core::dimension2d<u32> screenSize, scene::ICameraSceneNode * camera, int clickx, int clicky, core::vector3df &hit_vec);
bool GetAnyPlaneClickVector(core::dimension2d<u32> screenSize, scene::ICameraSceneNode * camera, core::plane3df plane, int clickx, int clicky, core::vector3df &hit_vec);
void OnMenuItemSelected(gui::IGUIContextMenu* menu);

#define COLOR_ADD_SELECTED      video::SColor(255,30,60,255)
#define COLOR_ADD_NOT_SELECTED  video::SColor(255,10,22,140)
#define COLOR_SUB_SELECTED      video::SColor(255,31,200,12)
#define COLOR_SUB_NOT_SELECTED  video::SColor(255,20,110,20)
#define COLOR_RED_SELECTED      video::SColor(255,255,60,25)
#define COLOR_RED_NOT_SELECTED  video::SColor(255,140,38,0)
#define COLOR_SEMI_SELECTED      video::SColor(255,165,165,165)
#define COLOR_SEMI_NOT_SELECTED  video::SColor(255,75,75,75)

#define GUI_BUTTON_RIGHT_CLICKED s32{EGET_BUTTON_CLICKED + 253}
#define GUI_BUTTON_SHIFT_CLICKED s32{EGET_BUTTON_CLICKED + 254}
#define GUI_BUTTON_DOUBLE_CLICKED s32{EGET_BUTTON_CLICKED + 255}
#define GUI_REFLECTED_FORM_CLOSED s32{EGET_BUTTON_CLICKED + 256}

enum
{
    GUI_ID_DIALOGUE_ROOT_WINDOW = 0x10000,
	GUI_ID_BUTTON_CUBE,
	GUI_ID_BUTTON_CYLINDER,
	GUI_ID_BUTTON_SPHERE,
	GUI_ID_BUTTON_CONE,
	GUI_ID_BUTTON_PLANE,
	GUI_ID_BUTTON_ADD,
	GUI_ID_BUTTON_SUBTRACT,
	GUI_ID_BUTTON_SEMISOLID,
	GUI_ID_BUTTON_INTERSECT,
	GUI_ID_BUTTON_CLIP,
	GUI_ID_BUTTON_REBUILD,
	GUI_ID_BUTTON_SAVE,
	GUI_ID_BUTTON_TEST,
	GUI_ID_BUTTON_TEXTURES,
	GUI_ID_BUTTON_NODES,
    GUI_ID_BUTTON_LIGHTS,

	GUI_ID_CAMERA_QUAD,
	GUI_ID_PANEL_2D,
	GUI_ID_PANEL_2D_1,
	GUI_ID_PANEL_2D_2,
	GUI_ID_PANEL_3D,
    GUI_ID_PANEL_UV,
	GUI_ID_TEXTURE_PICKER,

	GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_VIEW_BRUSHES,
	GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_VIEW_GEOMETRY,
	GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_GRID_TOGGLE,

	GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_DELETE_BRUSH,
	GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_MAKE_RED_BRUSH,

	GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_NODE_PROPERTIES,

	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_BRUSHES,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_GEOMETRY,
    GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_LOOPS,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_TRIANGLES,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_RENDER,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_RENDER_FINAL,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_UNLIT,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_LIGHTMAP,
    GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_LIGHT_ONLY,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_GRID_TOGGLE,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_FULLSCREEN_TOGGLE,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_TEST,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_SET_TEXTURE,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_CHOOSE_TEXTURE,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADJUST_TEXTURE,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_MATERIAL_GROUP,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADD_LIGHT,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADD_NODE,
    GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADD_MESHBUFFER_SCENENODE,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_NODE_PROPERTIES,
	GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_DELETE_NODE,


	GUI_ID_TEXTURE_SCROLL_BAR,

	GUI_ID_MAIN_MENU,
	GUI_ID_MENU_FILE_NEW,
	GUI_ID_MENU_FILE_OPEN,
	GUI_ID_MENU_FILE_SAVE_AS,
	GUI_ID_MENU_BUILD_PROGRESSIVE_BUILD,
	GUI_ID_MENU_BUILD_BUILD_FINAL,

	GUI_ID_FILE_OPEN_DIALOGUE,
	GUI_ID_FILE_SAVE_AS_DIALOGUE,

	GUI_ID_MENU_EDIT_SNAP_2,
	GUI_ID_MENU_EDIT_SNAP_4,
	GUI_ID_MENU_EDIT_SNAP_8,
	GUI_ID_MENU_EDIT_SNAP_16,

	GUI_ID_MENU_EDIT_ROTATE_1875,
	GUI_ID_MENU_EDIT_ROTATE_375,
	GUI_ID_MENU_EDIT_ROTATE_75,
	GUI_ID_MENU_EDIT_ROTATE_15,

    GUI_ID_MENU_VIEW_TEXTURES,
    GUI_ID_MENU_VIEW_PROPERTIES,
    GUI_ID_MENU_VIEW_CLASSES,
    GUI_ID_MENU_VIEW_INSTANCES,
    GUI_ID_MENU_VIEW_MATGROUPS,
    GUI_ID_MENU_VIEW_GEOSETTINGS,
    GUI_ID_MENU_VIEW_UV_EDITOR,
    GUI_ID_MENU_VIEW_SCENE_INSTANCES,

    GUI_ID_SCENE_INSTANCES_RIGHTCLICK_MENU_ITEM_RENAME,
    GUI_ID_SCENE_INSTANCES_RIGHTCLICK_MENU_ITEM_DELETE,

	GUI_ID_REFLECTED_BASE = 1000,
	GUI_ID_NODE_PROPERTIES_BASE,
	GUI_ID_TEXTURE_ADJUST_BASE,
	GUI_ID_NODE_CLASSES_BASE,
    GUI_ID_NODE_INSTANCES_BASE,
	GUI_ID_TEXTURES_BASE,
	GUI_ID_MATERIAL_GROUPS_BASE,
    GUI_ID_GEO_SETTINGS_BASE,
    GUI_ID_MAT_BUFFERS_BASE,
    GUI_ID_RENDER_TOOL_BASE,
    GUI_ID_FILE_OPEN_BASE,
    GUI_ID_LM_VIEWER_BASE,
    GUI_ID_UV_EDITOR_BASE,
    GUI_ID_SCENE_INSTANCES_BASE
};

enum
{
    USER_EVENT_GEOMETRY_REBUILT = 0x10000,
    USER_EVENT_SELECTION_CHANGED,
    USER_EVENT_TEXTURE_SELECTED,
    USER_EVENT_TEXTURE_EDIT_MODE_BEGIN,
    USER_EVENT_TEXTURE_EDIT_MODE_END,
    USER_EVENT_TEXTURE_PLANE_DRAG,
    USER_EVENT_TEXTURE_PLANE_DRAG_END,
    USER_EVENT_TEXTURE_UVS_MODIFIED,

    USER_EVENT_REFLECTED_FORM_REFRESHED,
    USER_EVENT_REFLECTED_FORM_CLOSED,

    USER_EVENT_DIRECTORY_SELECTED,
    USER_EVENT_FILE_SELECTED,
    USER_EVENT_FILE_DIALOGUE_CLOSED,

    USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED,

    USER_EVENT_CLEAR_LIGHTMAP_TEXTURES,

    USER_EVENT_NODES_SELECTED,
    USER_EVENT_NODE_SELECTION_ENDED
};


class ViewResizeObject
{
public:
    virtual void resizeView(core::dimension2du newsize) = 0;
    virtual int get_image_count() = 0;
    virtual video::ITexture* get_image(int) = 0;
};

class MyEventReceiver : public IEventReceiver, public ViewResizeObject
{
public:
    // We'll create a struct to record info on the mouse state
    struct SMouseState
    {
        core::position2di Position;
        bool LeftButtonDown;
        bool RightButtonDown;
        int WheelPos;
        SMouseState() : LeftButtonDown(false),RightButtonDown(false),WheelPos(0) { }

    } MouseState;

    // This is the one method that we have to implement
    virtual bool OnEvent(const SEvent& event);

    virtual bool IsKeyDown(EKEY_CODE keyCode) const;

    const SMouseState & GetMouseState(void) const;

    virtual int get_image_count() override { return 0; }
    virtual video::ITexture* get_image(int n) { return NULL; };

    MyEventReceiver();

    void Register(IEventReceiver* receiver)
    {
        receivers.push_back(receiver);
    }
    void UnRegister(IEventReceiver* receiver)
    {
        receivers.remove(receiver);
    }
    void UnRegisterFromInsideEvent(IEventReceiver* receiver)
    {
        safe_remove_receivers.push_back(receiver);
    }

    virtual void resizeView(core::dimension2du newsize);

    void Register_ViewResize(ViewResizeObject* receiver)
    {
        resize_receivers.push_back(receiver);
        receiver->resizeView(view_size);
    }
    void UnRegister_ViewResize(ViewResizeObject* receiver)
    {
        resize_receivers.remove(receiver);
    }

private:
	// We use this array to store the current state of each key
	bool KeyIsDown[KEY_KEY_CODES_COUNT];
	std::list<IEventReceiver*> receivers;
    std::list<IEventReceiver*> safe_remove_receivers;

    std::list<ViewResizeObject*> resize_receivers;

    // The dimensions of the main camera windows. Needed for nodes that create render targets.
    core::dimension2du view_size;
};

class CameraQuad;
class multi_tool_panel;

class GUI_layout
{
public:
    GUI_layout(video::IVideoDriver*, gui::IGUIEnvironment*);

    void initialize(core::rect<s32>);

    CameraQuad* getCameraQuad() {
        return cameraQuad;
    }

    multi_tool_panel* getToolPanel() {
        return tool_panel;
    }

    void resize(core::rect<s32>);

private:
    void menu_layout();

    gui::IGUIElement* main_panel = NULL;
    video::IVideoDriver* driver = NULL;
    gui::IGUIEnvironment* env = NULL;
    CameraQuad* cameraQuad = NULL;
    multi_tool_panel* tool_panel = NULL;

};

#endif
