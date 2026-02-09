#include <irrlicht.h>
#include "edit_env.h"
#include "LightSystemTool.h"
#include "GUI_tools.h"
#include "vkModules.h"

using namespace irr;
using namespace gui;

extern IrrlichtDevice* device;

Light_System_Base* Light_System_Tool::base = NULL;
multi_tool_panel* Light_System_Tool::panel = NULL;

//======================================================
// Module_GUI_Element
//

class Module_GUI_Element : public gui::IGUIElement
{
public:
    //! constructor
    Module_GUI_Element(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle);

    //! destructor
    virtual ~Module_GUI_Element();

    virtual void draw();

    virtual bool OnEvent(const SEvent& event);

    void left_click();

    std::wstring text;
    vector2di click_pos;
    vector2di drag_pos;
    bool bDragging = false;
    bool bMouseDown = false;
    Vulkan_Module* m_module = NULL;

};

Module_GUI_Element::Module_GUI_Element(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
    : IGUIElement(EGUIET_ELEMENT, environment, parent, id, rectangle)
{

}

Module_GUI_Element::~Module_GUI_Element()
{

}

bool Module_GUI_Element::OnEvent(const SEvent& event)
{

    if(event.EventType == EET_MOUSE_INPUT_EVENT)
    {
        vector2di mouse_pos = vector2di{ event.MouseInput.X,event.MouseInput.Y };

        switch (event.MouseInput.Event)
        {
        case EMIE_LMOUSE_LEFT_UP:
            bDragging = false;
            if (mouse_pos == click_pos)
            {
                left_click();
            }
            break;
        case EMIE_LMOUSE_PRESSED_DOWN:
            if (!bMouseDown)
            {
                bDragging = true;
                click_pos = mouse_pos;
                drag_pos = mouse_pos;
            }
            break;
        case EMIE_MOUSE_MOVED:
            if (bDragging)
            {
                move(mouse_pos - drag_pos);
                drag_pos = mouse_pos;
            }
            break;
        }

        return true;
    }

    return IGUIElement::OnEvent(event);
}

void Module_GUI_Element::left_click()
{
}

void Module_GUI_Element::draw()
{
    if (!IsVisible)
        return;

    gui::IGUISkin* skin = Environment->getSkin();
    video::IVideoDriver* driver = Environment->getVideoDriver();

    skin->draw3DToolBar(this,
        AbsoluteRect,
        &AbsoluteClippingRect);

    recti r(0, 0, 128, 32);

    gui::IGUIStaticText* stxt = Environment->addStaticText(text.c_str(),
        r, false, false, this, -1);

    IGUIElement::draw();
}


//======================================================
// UV Editor Camera Panel
//

LM_System_Panel::LM_System_Panel(IGUIEnvironment* environment, video::IVideoDriver* driver, IGUIElement* parent, LM_Viewer_Window* win, s32 id, core::rect<s32> rectangle)
    : TestPanel_2D(environment, driver, parent, id, rectangle)
{
    bZoomAllowed = false;
    //my_window = win;
    vHorizontal = core::vector3df(0, 0, 1);
    vVertical = core::vector3df(1, 0, 0);
    vAxis = core::vector3df(0, 1, 0);

    // uv_scene = new geometry_scene(device->getVideoDriver(), (MyEventReceiver*)device->getEventReceiver());
    // uv_scene = new GeometryStack(NULL,Scen)
}

void LM_System_Panel::Initialize(geometry_scene* geo_scene)
{
    //uv_scene = new GeometryStack();
    //uv_scene->initialize(geo_scene->get_smgr(), (MyEventReceiver*)device->getEventReceiver());
    TestPanel::Initialize(geo_scene);
}

LM_System_Panel::~LM_System_Panel()
{
    //if (uv_scene)
    //    delete uv_scene;
}

void LM_System_Panel::resize(core::dimension2d<u32> new_size)
{

    f32 ratio = (f32)this->Texture->getOriginalSize().Width / (f32)this->Texture->getOriginalSize().Height;
    core::matrix4 M;

    if (ratio >= 1.0)
    {
        M.buildProjectionMatrixOrthoLH(516 * ratio, 516, 0, 10000);

        viewSize = dimension2du(516 * ratio, 516);
    }
    else
    {
        M.buildProjectionMatrixOrthoLH(516, 516 / ratio, 0, 10000);

        viewSize = dimension2du(516, 516 / ratio);
    }

    getCamera()->setProjectionMatrix(M, true);
}

void LM_System_Panel::clear_childern()
{
    core::list<IGUIElement*>::Iterator it = m_viewPanel->getChildren().begin();
    for (; it != m_viewPanel->getChildren().end(); ++it)
        m_viewPanel->removeChild(*it);
}

void LM_System_Panel::cameraPan(vector2di mov)
{
    core::list<IGUIElement*>::Iterator it = m_viewPanel->getChildren().begin();
    for (; it != m_viewPanel->getChildren().end(); ++it)
    {
        (*it)->move(-mov);
    }
}
/*
void LM_System_Panel::showTriangles(bool bShowTriangles)
{
    bShowBrushes = bShowTriangles;
}

void LM_System_Panel::showLightmap(bool bShowLightmap)
{
    bRenderLightmap = bShowLightmap;
}
*/
scene::ICameraSceneNode* LM_System_Panel::getCamera()
{
    if (this->camera == NULL)
    {
        if (this->smgr)
        {
            core::dimension2di texture_size(512, 512);
            core::matrix4 M;

            M.buildProjectionMatrixOrthoLH(512, 512, 0, 10000);
            this->camera = smgr->addCameraSceneNode(0, core::vector3df(-texture_size.Width * 0.5, 1000, -texture_size.Height * 0.5), core::vector3df(-texture_size.Width * 0.5, 0, -texture_size.Height * 0.5), -1, false);
            this->camera->setProjectionMatrix(M, true);
        }
    }
    return this->camera;
}
/*
void LM_System_Panel::showMaterialGroup(int mg_n)
{
    if (uv_scene)
    {
        uv_scene->elements.clear();
        faces.clear();

        polyfold* pf = geo_scene->geoNode()->get_total_geometry();

        const vector<TextureMaterial>& mat_groups = geo_scene->geoNode()->final_meshnode_interface.get_materials_used();

        if (mg_n < mat_groups.size())
        {
            current_mat_group = mg_n;

            for (const Lightmap_Block& b : mat_groups[mg_n].blocks)
            {
                int offset = geo_scene->geoNode()->get_element_by_id(b.element_id)->surfaces[b.surface_no].face_index_offset;
                for (int f_i : b.faces)
                {
                    make_face(pf, f_i + offset, NULL);
                    faces.push_back(f_i + offset);
                }
            }

            vector<int> selection = geo_scene->getSelectedFaces();
            for (int i = 0; i < faces.size(); i++)
            {
                for (int f_j : selection)
                {
                    if (f_j == faces[i] && i < uv_scene->elements.size())
                    {
                        uv_scene->elements[i].bSelected = true;
                    }
                }
            }

            if (bRenderLightmap)
            {

                if (my_image)
                    delete my_image;
                my_image = NULL;

                //vector<video::ITexture*>& lm_textures = Lightmaps_Tool::get_manager()->lightmap_textures;
                const vector<video::ITexture*>& lm_textures = geo_scene->geoNode()->get_lightmap_config().get_textures();

                if (mat_groups[mg_n].lightmap_no < lm_textures.size() && mat_groups[mg_n].has_lightmap
                    && lm_textures[mat_groups[mg_n].lightmap_no] != NULL)
                {
                    my_image = new TextureImage(lm_textures[mat_groups[mg_n].lightmap_no]);
                }
            }
        }
    }
}
*/
bool LM_System_Panel::OnEvent(const SEvent& event)
{
    switch (event.EventType)
    {
    case EET_USER_EVENT:
         break;
    }

    return TestPanel_2D::OnEvent(event);
}

void LM_System_Panel::left_click(core::vector2di pos)
{
  
    
}

void LM_System_Panel::right_click(core::vector2di)
{
}

void LM_System_Panel::drawGrid(video::IVideoDriver* driver, const video::SMaterial material)
{
    int far_value = this->getCamera()->getFarValue();
    far_value *= -1;

    {
        int interval = gridSpace;
        while (viewSize.Width / interval > 36 || viewSize.Height / interval > 36)
        {
            interval = interval << 1;
        }
        int h_lines = viewSize.Width / interval;
        int v_lines = viewSize.Height / interval;
        core::vector3df vDownLeft = this->getCamera()->getAbsolutePosition();
        vDownLeft.Z -= viewSize.Width / 2;
        vDownLeft.X -= viewSize.Height / 2;
        int start_x = interval * ((int)vDownLeft.Z / interval);
        int start_y = interval * ((int)vDownLeft.X / interval);

        video::SColor col = video::SColor(255, 128, 128, 128);
        /*
        for (int i = -1; i < h_lines + 2; i++)
        {
            if (start_x + i * interval >= 0)
                driver->draw3DLine(core::vector3df(0, far_value, start_x + i * interval), core::vector3df(vDownLeft.X + viewSize.Height, far_value, start_x + i * interval), col);
        }
        for (int i = -1; i < v_lines + 2; i++)
        {
            if (start_y + i * interval >= 0)
                driver->draw3DLine(core::vector3df(start_y + i * interval, far_value, 0), core::vector3df(start_y + i * interval, far_value, vDownLeft.Z + viewSize.Width), col);
        }*/

        driver->draw3DLine(core::vector3df(0, 0, 0), core::vector3df(0, 0, -512), col);
        driver->draw3DLine(core::vector3df(0, 0, 0), core::vector3df(-512, 0, 0), col);
        driver->draw3DLine(core::vector3df(-512, 0, -512), core::vector3df(0, 0, -512), col);
        driver->draw3DLine(core::vector3df(-512, 0, -512), core::vector3df(-512, 0, 0), col);
    }
}

void LM_System_Panel::render()
{
    driver->setRenderTarget(getImage(), true, true, video::SColor(255, 4, 4, 4));
    smgr->setActiveCamera(getCamera());

    getCamera()->render();

    video::SMaterial someMaterial;
    someMaterial.Lighting = false;
    someMaterial.Thickness = 1.0;
    someMaterial.MaterialType = video::EMT_SOLID;


    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(someMaterial);

    //if (bShowGrid)
    this->drawGrid(driver, someMaterial);

    

    driver->setRenderTarget(0, true, true, video::SColor(0, 0, 0, 0));
}

void LM_System_Panel::OnMenuItemSelected(IGUIContextMenu* menu)
{
}


void LM_System_Panel::setGridSpacing(int spacing)
{
    gridSpace = spacing;
}

/*
void LM_System_Panel::centerCamera()
{
    this->getCamera()->setPosition(core::vector3df((int)texture_size.Width * 0.5, 5000, (int)texture_size.Height * 0.5));
    this->getCamera()->setTarget(core::vector3df((int)texture_size.Width * 0.5, 0, (int)texture_size.Height * 0.5));
}
*/

void LM_System_Panel::setAxis(int axis)
{
    if (axis == CAMERA_Y_AXIS)
    {
        core::dimension2du texture_size(128, 128);

        this->getCamera()->setPosition(core::vector3df((int)texture_size.Width * 0.5, 5000, (int)texture_size.Height * 0.5));
        this->getCamera()->setTarget(core::vector3df((int)texture_size.Width * 0.5, 0, (int)texture_size.Height * 0.5));

        this->getCamera()->updateAbsolutePosition();
        this->vHorizontal = core::vector3df(0, 0, 1);
        this->vVertical = core::vector3df(1, 0, 0);
        this->vAxis = core::vector3df(0, 1, 0);
    }
}

//============================================================================
// Light_System_Base
//

void Light_System_Base::initialize()
{
    if (m_panel)
        delete m_panel;

    m_panel = new LM_System_Panel(env, device->getVideoDriver(), NULL, NULL, 0, core::recti());
    m_panel->Initialize(g_scene);
    m_panel->set_base(this);
}

void Light_System_Base::close_uv_panel()
{
    if (m_panel && m_panel->getViewPanel())
        m_panel->getViewPanel()->disconnect();
}

void Light_System_Base::refresh_panel_view()
{
    if (cameraQuad && m_panel)
    {
        if (m_panel->hooked_up() == false)
        {
            cameraQuad->hookup_aux_panel(m_panel);
        }/*
        else if (mb_options.show_uv_view == false && m_panel->hooked_up() == true)
        {
            m_panel->getViewPanel()->disconnect();
        }*/

        if (m_panel->hooked_up() == true)
        {
            //m_panel->showTriangles(mb_options.show_triangles);
            //m_panel->showLightmap(mb_options.show_lightmap);

            MyEventReceiver* event_receiver = (MyEventReceiver*)device->getEventReceiver();
            SEvent event;
            event.EventType = EET_USER_EVENT;
            event.UserEvent.UserData1 = USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED;
            event_receiver->OnEvent(event);
        }

        m_panel->clear_childern();

        if (m_panel->getViewPanel())
        {
            for (Reflected_Module& mod : modules)
            {
                Module_GUI_Element* element = new Module_GUI_Element(env, m_panel->getViewPanel(), -1, recti(mod.pos, mod.size));
                element->text = mod.name;
                element->m_module = mod.m_module;
                m_panel->getViewPanel()->bringToFront(element);
                element->drop();
            }
        }

    }
}

void Light_System_Base::init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
}

void Light_System_Base::write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor)
{
}

Light_System_Base::~Light_System_Base()
{
}

void Light_System_Base::show()
{
    if (workflow)
        delete workflow;

    workflow = new Vulkan_Workflow();
    workflow->load();
    
    modules.clear();

    for (Vulkan_Module* mod : workflow->modules)
    {
        std::string str(mod->GetDynamicReflection()->name);
        vector2di pos = vector2di(10 + mod->pos.X, 10 + mod->pos.Y);
        //vector2di pos = vector2di(10, 10);
        modules.push_back(Reflected_Module{ wstring(str.begin(),str.end()), pos, dimension2du{128,128},mod});
    }
    
    refresh_panel_view();
}

void Light_System_Base::set_scene(geometry_scene* gs)
{
    tool_base::set_scene(gs);

    if (m_panel)
        m_panel->set_scene(gs);
}

void Reflected_Module::addComponents(reflect::TypeDescriptor_Struct* typeDescriptor, Vulkan_Module* obj)
{
    for (reflect::Member& m : typeDescriptor->members)
    {
        reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

        if (m_tD->inherited_type == &reflect::input_type::Reflection)
        {
            reflect::input_type* in = (reflect::input_type*)m.get(this);

            std::string name(m.alias);

            inputs.push_back(module_connector{ std::wstring(name.begin(),name.end()) });
        }
        else if (m_tD->inherited_type == &reflect::output_type::Reflection)
        {
            reflect::output_type* out = (reflect::output_type*)m.get(this);

            std::string name(m.alias);

            outputs.push_back(module_connector{ std::wstring(name.begin(),name.end()) });
        }
    }
}

Module_GUI_Element* Reflected_Module::get_GUI_Element(IGUIEnvironment* env, IGUIElement* parent, s32 id)
{

    size.Width = 128;
    size.Height = 64 + 24*(inputs.size()+outputs.size());

    Module_GUI_Element* element = new Module_GUI_Element(env, parent, -1, recti{position2di{0,0},size});
    element->text = name;
    element->drop();

    return element;
}
