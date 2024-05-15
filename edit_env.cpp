#include "edit_env.h"
#include <irrlicht.h>
#include <iostream>

#include "CameraPanel.h"
#include "CGUIWindow.h"
#include "csg_classes.h"
#include "utils.h"
#include "edit_classes.h"
//#include "reflection.h"
#include "create_primitives.h"
#include "texture_picker.h"
#include "GUI_tools.h"
#include "geo_scene_settings.h"
#include "NodeClassesTool.h"
#include "NodeInstancesTool.h"
#include "node_properties.h"
#include "file_open.h"
#include "uv_tool.h"
#include "vtoolbar.h"
#include "ex_gui_elements.h"
#include "vulkan_app.h"

extern IrrlichtDevice* device;
using namespace irr;
using namespace core;
using namespace gui;
extern geometry_scene* g_scene;
extern TestPanel* ContextMenuOwner;


struct CubeOptions
{
    int value;
    REFLECT_MULTI_CHOICE()
};

REFLECT_MULTI_STRUCT_BEGIN(CubeOptions)
REFLECT_MULTI_STRUCT_LABEL("default")
REFLECT_MULTI_STRUCT_LABEL("Option A")
REFLECT_MULTI_STRUCT_LABEL("Option B")
REFLECT_MULTI_STRUCT_END()


struct TestCube
{
    bool isEnabled;
    int height;
    int width;
    int length;
    video::SColor color;
    video::ITexture* texture;
    core::vector3df scale;
    //CubeOptions type;
    //CubeOptions subtype;

    REFLECT()
};

REFLECT_STRUCT_BEGIN(TestCube)
REFLECT_STRUCT_MEMBER(isEnabled)
REFLECT_STRUCT_MEMBER(height)
REFLECT_STRUCT_MEMBER(width)
REFLECT_STRUCT_MEMBER(length)

REFLECT_STRUCT_MEMBER(color)
REFLECT_STRUCT_MEMBER(texture)

REFLECT_STRUCT_MEMBER(scale)
//REFLECT_STRUCT_MEMBER(type)
//REFLECT_STRUCT_MEMBER(subtype)
REFLECT_STRUCT_END()




void do_add_geometry()
{
    if(g_scene)
    {
        g_scene->add(g_scene->elements[0].brush);
    }
}

void do_subtract_geometry()
{
    if(g_scene)
    {
        g_scene->subtract(g_scene->elements[0].brush);
    }
}


void do_add_semisolid_geometry()
{
    if(g_scene)
    {
        g_scene->add_semisolid(g_scene->elements[0].brush);
    }
}

void do_intersect_brush()
{
    if(g_scene)
    {
        g_scene->intersect_active_brush();
    }
}


void do_clip_brush()
{
    if(g_scene)
    {
        g_scene->clip_active_brush();
    }
}

void do_rebuild()
{
    if(g_scene)
    {
        g_scene->rebuild_geometry();
    }
}

void do_new_scene()
{
    if(g_scene)
    {
        g_scene->clear_scene();
    }
}

void do_toggle_progressive_build()
{
    if(g_scene)
    {
        g_scene->toggle_progressive_build();

        gui::IGUIEnvironment* gui = device->getGUIEnvironment();
        gui::IGUIContextMenu* menu = (gui::IGUIContextMenu*)gui->getRootGUIElement()->getElementFromId(GUI_ID_MAIN_MENU);
        gui::IGUIContextMenu* submenu = menu->getSubMenu(2);
        submenu->setItemChecked(0,g_scene->progressive_build_enabled());
    }
}

void do_save()
{
    if(g_scene)
    {
        g_scene->WriteSceneNodesToFile("nodes.dat");
        g_scene->WriteTextures("textures.txt");
        g_scene->Write2("refl_serial.dat");
        WriteGUIStateToFile("gui_state.dat");
    }
}

void do_file_save_as()
{
    Save_Geometry_File* fsave = new Save_Geometry_File(g_scene);
    //std::cout<<"save as: not implemented\n";
}

void do_file_open()
{
    Open_Geometry_File* fopen = new Open_Geometry_File(g_scene);
}

void do_set_grid_snap(int snap)
{
    gui::IGUIEnvironment* env = device->getGUIEnvironment();
    gui::IGUIElement* root = env->getRootGUIElement();
    CameraQuad* quad = (CameraQuad*)root->getElementFromId(GUI_ID_CAMERA_QUAD,true);

    if(quad)
        quad->setGridSnap(snap);

    gui::IGUIContextMenu* menu = (gui::IGUIContextMenu*)env->getRootGUIElement()->getElementFromId(GUI_ID_MAIN_MENU,true);
    gui::IGUIContextMenu* submenu = menu->getSubMenu(1);
    submenu=submenu->getSubMenu(0);
    submenu->setItemChecked(0,snap==2);
    submenu->setItemChecked(1,snap==4);
    submenu->setItemChecked(2,snap==8);
    submenu->setItemChecked(3,snap==16);
}

void do_set_rotate_snap(f32 snap)
{
    gui::IGUIEnvironment* env = device->getGUIEnvironment();
    gui::IGUIElement* root = env->getRootGUIElement();
    CameraQuad* quad = (CameraQuad*)root->getElementFromId(GUI_ID_CAMERA_QUAD,true);

    if(quad)
        quad->setRotateSnap(snap);

    gui::IGUIContextMenu* menu = (gui::IGUIContextMenu*)env->getRootGUIElement()->getElementFromId(GUI_ID_MAIN_MENU,true);
    if (menu)
    {
        gui::IGUIContextMenu* submenu = menu->getSubMenu(1);
        submenu = submenu->getSubMenu(1);
        submenu->setItemChecked(0, snap == 1.875);
        submenu->setItemChecked(1, snap == 3.75);
        submenu->setItemChecked(2, snap == 7.5);
        submenu->setItemChecked(3, snap == 15.0);
    }
}

TestCube refl_cube{true,64,256,128,NULL};

class Test_EditWindow : public EditWindow
{
public:
    Test_EditWindow(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene,s32 id,core::rect<s32> rect)
    : EditWindow(env,parent,g_scene,id,rect){}

private:
    virtual void click_OK();
};

void refl_test_write(Test_EditWindow* win)
{
    win->write(&refl_cube);
}

void Test_EditWindow::click_OK()
{
    refl_test_write(this);
    this->remove();
}

void do_refl_test()
{

    //refl_cube.texture = NULL;
    //std::cout<<"OK\n";
    gui::IGUIEnvironment* env = device->getGUIEnvironment();
    gui::IGUIElement* root = env->getRootGUIElement();

    Test_EditWindow* win = new Test_EditWindow(env,env->getRootGUIElement(),g_scene,-1,core::rect<s32>(140,200,16+196,64+196));
    win->setText(L"Reflected GUI");

    //std::cout<<g_scene<<"\n";
    reflect::TypeDescriptor_Struct* typeDesc = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<TestCube>::get();

    //typeDesc->dump(&refl_cube,0);

    win->Show(typeDesc, &refl_cube);
    win->drop();
}

void do_add_plane_test()
{
    if(g_scene)
    {
        g_scene->add_plane(g_scene->elements[0].brush);
    }
}

void do_texture_picker_test()
{
    TexturePicker_Tool::show();
}


void OnMenuItemSelected(IGUIContextMenu* menu)
{
    s32 id = menu->getItemCommandId(menu->getSelectedItem());
    gui::IGUIEnvironment* env = device->getGUIEnvironment();
    gui::IGUIElement* root = env->getRootGUIElement();
    gui::IGUIElement* quad = (CameraQuad*)root->getElementFromId(GUI_ID_CAMERA_QUAD,true);
    switch(id)
    {
    case GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_GRID_TOGGLE:
    case GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_VIEW_BRUSHES:
    case GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_VIEW_GEOMETRY:
    case GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_DELETE_BRUSH:
    case GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_MAKE_RED_BRUSH:
    case GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_NODE_PROPERTIES:
        {
            TestPanel_2D* panel2D = (TestPanel_2D*)quad->getElementFromId(GUI_ID_PANEL_2D);
            if(panel2D && panel2D == ContextMenuOwner)
            {
                SEvent event;
                event.EventType = EET_GUI_EVENT;
                event.GUIEvent.Caller = menu;
                event.GUIEvent.Element = 0;
                event.GUIEvent.EventType = EGET_MENU_ITEM_SELECTED;
                panel2D->OnEvent(event);
            }
            else
            {
                panel2D = (TestPanel_2D*)quad->getElementFromId(GUI_ID_PANEL_2D_1);
                if(panel2D && panel2D == ContextMenuOwner)
                {
                SEvent event;
                event.EventType = EET_GUI_EVENT;
                event.GUIEvent.Caller = menu;
                event.GUIEvent.Element = 0;
                event.GUIEvent.EventType = EGET_MENU_ITEM_SELECTED;
                panel2D->OnEvent(event);
                }
                else
                {
                    panel2D = (TestPanel_2D*)quad->getElementFromId(GUI_ID_PANEL_2D_2);
                    if(panel2D && panel2D == ContextMenuOwner)
                    {
                    SEvent event;
                    event.EventType = EET_GUI_EVENT;
                    event.GUIEvent.Caller = menu;
                    event.GUIEvent.Element = 0;
                    event.GUIEvent.EventType = EGET_MENU_ITEM_SELECTED;
                    panel2D->OnEvent(event);
                    }
                }
            }
        }
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_GRID_TOGGLE:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_BRUSHES:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_GEOMETRY:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_LOOPS:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_TRIANGLES:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_RENDER:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_RENDER_FINAL:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_UNLIT:
	case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_DYNAMIC_LIGHT:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_SET_TEXTURE:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADJUST_TEXTURE:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_CHOOSE_TEXTURE:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADD_LIGHT:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADD_NODE:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_NODE_PROPERTIES:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_DELETE_NODE:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_MATERIAL_GROUP:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_FULLSCREEN_TOGGLE:
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_TEST:
        {
            TestPanel_3D* panel3D = (TestPanel_3D*)quad->getElementFromId(GUI_ID_PANEL_3D);
            if(panel3D && panel3D == ContextMenuOwner)
            {
                SEvent event;
                event.EventType = EET_GUI_EVENT;
                event.GUIEvent.Caller = menu;
                event.GUIEvent.Element = 0;
                event.GUIEvent.EventType = EGET_MENU_ITEM_SELECTED;
                panel3D->OnEvent(event);
            }
        }
        break;
    case GUI_ID_MENU_BUILD_PROGRESSIVE_BUILD:
        {
            do_toggle_progressive_build();
            break;
        }
    case GUI_ID_MENU_BUILD_BUILD_FINAL:
        {
            //do_build_final_mesh();
            break;
        }
    case GUI_ID_MENU_FILE_NEW:
        {
            do_new_scene();
            break;
        }
    case GUI_ID_MENU_FILE_OPEN:
        {
            //File_Open_Tool::show();
            do_file_open();
            break;
        }
    case GUI_ID_MENU_FILE_SAVE_AS:
        {
            do_file_save_as();
            break;
        }
    case GUI_ID_MENU_VIEW_TEXTURES:
    {
        TexturePicker_Tool::show();
        break;
    }
    case GUI_ID_MENU_VIEW_PROPERTIES:
    {
        NodeProperties_Tool::show();
        break;
    }
    case GUI_ID_MENU_VIEW_CLASSES:
    {
        Node_Classes_Tool::show();
        break;
    }
    case GUI_ID_MENU_VIEW_INSTANCES:
    {
        Node_Instances_Tool::show();
        break;
    }
    case GUI_ID_MENU_VIEW_MATGROUPS:
    {
        Material_Buffers_Tool::show();
        break;
    }
    case GUI_ID_MENU_VIEW_GEOSETTINGS:
    {
        Geo_Settings_Tool::show();
        break;
    }
    case GUI_ID_MENU_EDIT_SNAP_2:
        do_set_grid_snap(2);
        break;
    case GUI_ID_MENU_EDIT_SNAP_4:
        do_set_grid_snap(4);
        break;
    case GUI_ID_MENU_EDIT_SNAP_8:
        do_set_grid_snap(8);
        break;
    case GUI_ID_MENU_EDIT_SNAP_16:
        do_set_grid_snap(16);
        break;
    case GUI_ID_MENU_EDIT_ROTATE_1875:
        do_set_rotate_snap(1.875);
        break;
    case GUI_ID_MENU_EDIT_ROTATE_375:
        do_set_rotate_snap(3.75);
        break;
    case GUI_ID_MENU_EDIT_ROTATE_75:
        do_set_rotate_snap(7.5);
        break;
    case GUI_ID_MENU_EDIT_ROTATE_15:
        do_set_rotate_snap(15.0);
        break;
    }
}

bool MyEventReceiver::OnEvent(const SEvent& event)
{
    if(event.EventType == EET_GUI_EVENT)
    {
        s32 id = event.GUIEvent.Caller->getID();
        gui::IGUIEnvironment* env = device->getGUIEnvironment();


        switch(event.GUIEvent.EventType)
        {
            case EGET_MENU_ITEM_SELECTED:
            {
                OnMenuItemSelected((gui::IGUIContextMenu*)event.GUIEvent.Caller);
            }
            break;
            case EGET_BUTTON_CLICKED:
            {
                switch(id)
                {
                    case GUI_ID_BUTTON_CUBE:
                        GeometryFactory::MakeCubeWindow();
                        //std::cout<<"Cube\n";
                        break;
                    case GUI_ID_BUTTON_CYLINDER:
                        GeometryFactory::MakeCylinderWindow();
                        //std::cout<<"Cylinder\n";
                        break;
                    case GUI_ID_BUTTON_SPHERE:
                        GeometryFactory::MakeSphereWindow();
                        //std::cout<<"Sphere\n";
                        break;
                    case GUI_ID_BUTTON_CONE:
                        GeometryFactory::MakeConeWindow();
                        //std::cout<<"Cone\n";
                        break;
                    case GUI_ID_BUTTON_PLANE:
                        GeometryFactory::MakePlaneWindow();
                        //std::cout<<"Plane\n";
                        break;
                    case GUI_ID_BUTTON_ADD:
                        do_add_geometry();
                        //std::cout<<"Add Geometry\n";
                        break;
                    case GUI_ID_BUTTON_SUBTRACT:
                        do_subtract_geometry();
                        //std::cout<<"Subtract Geometry\n";
                        break;
                     case GUI_ID_BUTTON_SEMISOLID:
                        do_add_semisolid_geometry();
                        //std::cout<<"Add Semisolid Geometry\n";
                        break;
                    case GUI_ID_BUTTON_INTERSECT:
                        do_intersect_brush();
                       // std::cout<<"Intersect Active Brush\n";
                        break;
                    case GUI_ID_BUTTON_CLIP:
                        do_clip_brush();
                        //std::cout<<"Clip Active Brush\n";
                        break;
                    case GUI_ID_BUTTON_REBUILD:
                        do_rebuild();
                        //std::cout<<"Rebuild\n";
                        break;
                    case GUI_ID_BUTTON_SAVE:
                        do_save();
                        //std::cout<<"Save\n";
                        break;
                    case GUI_ID_BUTTON_TEXTURES:
                        //TexturePicker_Tool::show();
                        //Geo_Settings_Tool::show();
                        //Material_Buffers_Tool::show();
                        //Node_Instances_Tool::show();
                        //g_scene->clip_active_brush_plane_geometry();
                        UV_Editor_Tool::show();
                        break;
                    case GUI_ID_BUTTON_NODES:
                        ListReflectedNodes_Tool::show();
                        break;
                    case GUI_ID_BUTTON_LIGHTS:

                        VulkanApp vk;
                        vk.run(&g_scene->final_meshnode_interface);

                        break;
                }
            }
            break;
            case EGET_FILE_SELECTED:
            {
                if(id==GUI_ID_FILE_OPEN_DIALOGUE)
                   {
                        gui::IGUIFileOpenDialog* fopen  = (gui::IGUIFileOpenDialog*)event.GUIEvent.Caller;
                        if(fopen)
                        {
                            std::cout<<"error- file open not implemented :P\n";
                        }
                   }
            }
            break;
        }

    }

    // Remember the mouse state
    if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
    {
        switch(event.MouseInput.Event)
        {
        case EMIE_LMOUSE_PRESSED_DOWN:
            MouseState.LeftButtonDown = true;
            break;

        case EMIE_LMOUSE_LEFT_UP:
            MouseState.LeftButtonDown = false;
            break;

        case EMIE_MOUSE_MOVED:
            MouseState.Position.X = event.MouseInput.X;
            MouseState.Position.Y = event.MouseInput.Y;
            break;

        case EMIE_MOUSE_WHEEL:
            if(event.MouseInput.Wheel > 0)
                MouseState.WheelPos++;
            else
                MouseState.WheelPos--;
            break;

        case EMIE_RMOUSE_LEFT_UP:
            MouseState.RightButtonDown = false;
            break;

        case EMIE_RMOUSE_PRESSED_DOWN:
            MouseState.RightButtonDown = true;
            break;

        default:
            // We won't use the wheel
            break;
        }
    }
    else if(event.EventType ==   irr::EET_KEY_INPUT_EVENT)
    {
            KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
    }
    else if(event.EventType == irr::EET_USER_EVENT)
    {
        for(auto it = receivers.begin(); it != receivers.end(); ++it)
            (*it)->OnEvent(event);

        for (auto it = safe_remove_receivers.begin(); it != safe_remove_receivers.end(); ++it)
            receivers.remove(*it);

        safe_remove_receivers.clear();
    }

    return false;
}
/*
bool hasModalDialogue()
{
    if(!device)
        return false;
    gui::IGUIEnvironment* env = device->getGUIEnvironment();
    gui::IGUIElement* focused = env->getFocus();
    while(focused)
    {
        if(focused->isVisible() && focused->hasType(gui::EGUIET_MODAL_SCREEN))
           return true;
        focused = focused->getParent();
    }
    return false;
}*/

//==================================================================================================
//
//


bool GetPlaneClickVector(dimension2d<u32> screenSize, scene::ICameraSceneNode * camera, int clickx, int clicky, vector3df &hit_vec)
{
    const scene::SViewFrustum* frustum = camera->getViewFrustum();
    const core::vector3df cameraPosition = camera->getAbsolutePosition();

    vector3df vNearLeftDown = frustum->getNearLeftDown();
    vector3df vNearRightDown = frustum->getNearRightDown();
    vector3df vNearLeftUp = frustum->getNearLeftUp();
    vector3df vNearRightUp = frustum->getNearRightUp();

    f32 t_X = (f32)clickx / screenSize.Width;
    f32 t_Y = 1.0 - (f32)clicky / screenSize.Height;

    //screen space: Y is horizontal axis
    vector3df X_vec = (vNearRightDown - vNearLeftDown) * t_X;
    vector3df Y_vec = (vNearLeftUp - vNearLeftDown) * t_Y;
    vector3df target = vNearLeftDown + Y_vec + X_vec;

    vector3df ray = target-cameraPosition;
    ray.normalize();

    //world space: Y is vertical axis
    if(ray.Y < 0)
    {
        f32 t_Y = - (target.Y - 0.0) / ray.Y;
        hit_vec = target + (ray*t_Y);

        return true;
    }
    return false;
}


bool GetAnyPlaneClickVector(dimension2d<u32> screenSize, scene::ICameraSceneNode * camera, core::plane3df plane, int clickx, int clicky, vector3df &hit_vec)
{
    const scene::SViewFrustum* frustum = camera->getViewFrustum();
    const core::vector3df cameraPosition = camera->getAbsolutePosition();

    vector3df vNearLeftDown = frustum->getNearLeftDown();
    vector3df vNearRightDown = frustum->getNearRightDown();
    vector3df vNearLeftUp = frustum->getNearLeftUp();
    vector3df vNearRightUp = frustum->getNearRightUp();

    f32 t_X = (f32)clickx / screenSize.Width;
    f32 t_Y = 1.0 - (f32)clicky / screenSize.Height;

    //screen space: Y is horizontal axis
    vector3df X_vec = (vNearRightDown - vNearLeftDown) * t_X;
    vector3df Y_vec = (vNearLeftUp - vNearLeftDown) * t_Y;
    vector3df target = vNearLeftDown + Y_vec + X_vec;

    vector3df ray = target-cameraPosition;
    ray.normalize();

    //World Space
    core::vector3df hitvec;

    if(plane.getIntersectionWithLine(cameraPosition,ray,hitvec))
        {
         hit_vec = hitvec;
         return true;
        }

    return false;
}

GUI_layout::GUI_layout(video::IVideoDriver* driv, scene::ISceneManager* smg, gui::IGUIEnvironment* gui)
{
    driver = driv;
    smgr = smg;
    env = gui;
}

void GUI_layout::initialize(core::rect<s32> win_rect)
{
    //core::rect<s32>(core::position2d<s32>(0, 0), core::dimension2d<u32>(900, 680))
    core::rect<s32> main_panel_rect = win_rect;

    main_panel_rect.LowerRightCorner.X -= 300;
    main_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, env, env->getRootGUIElement(), -1, main_panel_rect);
    
    menu_layout();

    MySkin* skin = new MySkin(EGST_WINDOWS_CLASSIC, driver);

    //gui::IGUISkin* skin = gui->getSkin();
    gui::IGUIFont* builtinfont = env->getBuiltInFont();
    gui::IGUIFontBitmap* bitfont = 0;
    if (builtinfont && builtinfont->getType() == EGFT_BITMAP)
        bitfont = (IGUIFontBitmap*)builtinfont;

    IGUISpriteBank* bank = 0;
    skin->setFont(builtinfont);

    if (bitfont)
        bank = bitfont->getSpriteBank();

    skin->setSpriteBank(bank);


    gui::IGUIFont* font = env->getFont("fonthaettenschweiler.bmp");
    if (font)
        skin->setFont(font);

    env->setSkin(skin);
    skin->drop();

    skin->setColor(EGDC_BUTTON_TEXT, video::SColor(255, 236, 236, 236));
    skin->setColor(EGDC_3D_FACE, video::SColor(255, 16, 16, 16));
    skin->setColor(EGDC_EDITABLE, video::SColor(255, 24, 24, 24));
    skin->setColor(EGDC_FOCUSED_EDITABLE, video::SColor(255, 40, 50, 65));

    skin->setColor(EGDC_SCROLLBAR, video::SColor(255, 0, 0, 0));

    skin->setColor(EGDC_WINDOW_SYMBOL, video::SColor(255, 236, 236, 236)),
        skin->setColor(EGDC_3D_HIGH_LIGHT, video::SColor(255, 32, 32, 32));
    skin->setColor(EGDC_3D_LIGHT, video::SColor(255, 48, 48, 48));
    skin->setColor(EGDC_3D_SHADOW, video::SColor(255, 32, 32, 32));
    skin->setColor(EGDC_3D_DARK_SHADOW, video::SColor(255, 16, 16, 16));


    for (int i = 0; i < irr::gui::EGDC_COUNT; i++)
    {
        video::SColor col = skin->getColor((EGUI_DEFAULT_COLOR)i);
        col.setAlpha(255);
        skin->setColor((EGUI_DEFAULT_COLOR)i, col);
    }

    core::rect<s32> quad_rect(core::position2d<s32>(32, 20), main_panel_rect.LowerRightCorner);

    cameraQuad = new CameraQuad(env, main_panel,GUI_ID_CAMERA_QUAD, quad_rect, driver);

    core::rect<s32> tool_panel_r(core::position2d<s32>(main_panel_rect.getWidth(), 0), win_rect.LowerRightCorner);

    tool_panel = new multi_tool_panel(env, env->getRootGUIElement(), -1,
        tool_panel_r);

}

void GUI_layout::resize(core::rect<s32> win_rect)
{
    cameraQuad->grab();

    if (main_panel)
        main_panel->remove();

    core::rect<s32> main_panel_rect = win_rect;

    main_panel_rect.LowerRightCorner.X -= 300;

    //core::rect<s32>(core::position2d<s32>(0, 0), core::dimension2d<u32>(900, 680))
    main_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, env, env->getRootGUIElement(), -1, main_panel_rect);

    menu_layout();

    core::rect<s32> quad_rect(core::position2d<s32>(32, 20), main_panel_rect.LowerRightCorner);

    main_panel->addChild(cameraQuad);
    cameraQuad->drop();

    cameraQuad->resize(quad_rect);

    core::rect<s32> tool_panel_r(core::position2d<s32>(main_panel_rect.getWidth(), 0), win_rect.LowerRightCorner);
    tool_panel->resize(tool_panel_r);
}

void GUI_layout::menu_layout()
{
    if (!main_panel || !env)
        return;

    gui::IGUIContextMenu* menu = env->addMenu(main_panel, GUI_ID_MAIN_MENU);
    gui::IGUIContextMenu* submenu;
    gui::IGUIContextMenu* submenu2;

    menu->addItem(L"File", -1, true, true);
    submenu = menu->getSubMenu(0);
    submenu->addItem(L"New Scene", GUI_ID_MENU_FILE_NEW, true, false);
    submenu->addItem(L"Open", GUI_ID_MENU_FILE_OPEN, true, false);
    submenu->addItem(L"Save As", GUI_ID_MENU_FILE_SAVE_AS, true, false);

    menu->addItem(L"Editor", -1, true, true);
    submenu = menu->getSubMenu(1);

    submenu->addItem(L"Grid Snap", -1, true, true);
    submenu2 = submenu->getSubMenu(0);
    submenu2->addItem(L"2", GUI_ID_MENU_EDIT_SNAP_2, true, false, true);
    submenu2->addItem(L"4", GUI_ID_MENU_EDIT_SNAP_4, true, false, true);
    submenu2->addItem(L"8", GUI_ID_MENU_EDIT_SNAP_8, true, false, true);
    submenu2->addItem(L"16", GUI_ID_MENU_EDIT_SNAP_16, true, false, true);
    submenu2->setItemChecked(0, false);
    submenu2->setItemChecked(1, false);
    submenu2->setItemChecked(2, true);
    submenu2->setItemChecked(3, false);

    submenu->addItem(L"Rotate Snap", -1, true, true);
    submenu2 = submenu->getSubMenu(1);
    submenu2->addItem(L"1.875", GUI_ID_MENU_EDIT_ROTATE_1875, true, false, true);
    submenu2->addItem(L"3.75", GUI_ID_MENU_EDIT_ROTATE_375, true, false, true);
    submenu2->addItem(L"7.5", GUI_ID_MENU_EDIT_ROTATE_75, true, false, true);
    submenu2->addItem(L"15", GUI_ID_MENU_EDIT_ROTATE_15, true, false, true);
    submenu2->setItemChecked(0, false);
    submenu2->setItemChecked(1, false);
    submenu2->setItemChecked(2, true);
    submenu2->setItemChecked(3, false);

    menu->addItem(L"Build", -1, true, true);
    submenu = menu->getSubMenu(2);
    submenu->addItem(L"Enable Progressive Build", GUI_ID_MENU_BUILD_PROGRESSIVE_BUILD, true, false, true, true);
    submenu->addItem(L"Build Final Geometry", GUI_ID_MENU_BUILD_BUILD_FINAL, true, false, false, false);
    submenu->setItemChecked(0, true);

    menu->addItem(L"View", -1, true, true);
    submenu = menu->getSubMenu(3);
    submenu->addItem(L"Texture Browser", GUI_ID_MENU_VIEW_TEXTURES, true, false, false, false);
    submenu->addItem(L"Node Properties", GUI_ID_MENU_VIEW_PROPERTIES, true, false, false, false);
    submenu->addItem(L"Node Instances", GUI_ID_MENU_VIEW_INSTANCES, true, false, false, false);
    submenu->addItem(L"Node Classes", GUI_ID_MENU_VIEW_CLASSES, true, false, false, false);
    submenu->addItem(L"Material Groups", GUI_ID_MENU_VIEW_MATGROUPS, true, false, false, false);
    submenu->addItem(L"Editor Settings", GUI_ID_MENU_VIEW_GEOSETTINGS, true, false, false, false);

    //gui::IGUIToolBar* bar = gui->addToolBar(main_panel,-1);
    VToolBar* bar = new VToolBar(env, main_panel, -1, core::rect<s32>(0, 0, 10, 10));

    video::ITexture* image = driver->getTexture("cube_icon_small.png");
    bar->addButton(GUI_ID_BUTTON_CUBE, 0, L"Make a cube", image, 0, false, false);

    image = driver->getTexture("cylinder_icon.png");
    bar->addButton(GUI_ID_BUTTON_CYLINDER, 0, L"Make a cylinder", image, 0, false, false);

    image = driver->getTexture("sphere_icon.png");
    bar->addButton(GUI_ID_BUTTON_SPHERE, 0, L"Make a sphere", image, 0, false, false);

    image = driver->getTexture("cone_icon.png");
    bar->addButton(GUI_ID_BUTTON_CONE, 0, L"Make a cone", image, 0, false, false);

    image = driver->getTexture("plane_icon.png");
    bar->addButton(GUI_ID_BUTTON_PLANE, 0, L"Make a plane", image, 0, false, false);

    image = driver->getTexture("add_icon_small.png");
    bar->addButton(GUI_ID_BUTTON_ADD, 0, L"Add Geometry", image, 0, false, false);

    image = driver->getTexture("subtract_icon_small.png");
    bar->addButton(GUI_ID_BUTTON_SUBTRACT, 0, L"Subtract Geometry", image, 0, false, false);

    image = driver->getTexture("semisolid_icon.png");
    bar->addButton(GUI_ID_BUTTON_SEMISOLID, 0, L"Add Semisolid Geometry", image, 0, false, false);

    image = driver->getTexture("intersect_icon.png");
    bar->addButton(GUI_ID_BUTTON_INTERSECT, 0, L"Intersect Red Brush", image, 0, false, false);

    image = driver->getTexture("clip_icon.png");
    bar->addButton(GUI_ID_BUTTON_CLIP, 0, L"Clip Red Brush", image, 0, false, false);

    image = driver->getTexture("rebuild_icon.png");
    bar->addButton(GUI_ID_BUTTON_REBUILD, 0, L"Rebuild Geometry", image, 0, false, false);

    image = driver->getTexture("save_icon.png");
    bar->addButton(GUI_ID_BUTTON_SAVE, 0, L"Save", image, 0, false, false);

    image = driver->getTexture("small_test_icon.png");
    bar->addButton(GUI_ID_BUTTON_TEXTURES, 0, L"test", image, 0, false, false);

    image = driver->getTexture("small_N_icon.png");
    bar->addButton(GUI_ID_BUTTON_NODES, 0, L"test", image, 0, false, false);

    image = driver->getTexture("lights_icon.png");
    bar->addButton(GUI_ID_BUTTON_LIGHTS, 0, L"Build Lighting", image, 0, false, false);
}
