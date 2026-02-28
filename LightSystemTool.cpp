#include <irrlicht.h>
#include "edit_env.h"
#include "LightSystemTool.h"
#include "GUI_tools.h"
#include "vkModules.h"
#include "edit_classes.h"
#include "ex_gui_elements.h"

using namespace irr;
using namespace gui;

extern IrrlichtDevice* device;

Light_System_Base* Light_System_Tool::base = NULL;
multi_tool_panel* Light_System_Tool::panel = NULL;




//======================================================
// System_GUI_Element
//



System_GUI_Element::System_GUI_Element(Light_System_Base* base, gui::IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
    : IGUIElement(EGUIET_ELEMENT, environment, parent, id, rectangle), my_base{base}
{

}

System_GUI_Element::~System_GUI_Element()
{

}

void System_GUI_Element::draw()
{
    
    gui::IGUIElement::draw();

    my_base->draw_text();
    //my_base->draw_lines();
}

bool System_GUI_Element::OnEvent(const SEvent& event)
{
    return gui::IGUIElement::OnEvent(event);
}

//======================================================
// Module_GUI_Element
//



Module_GUI_Element::Module_GUI_Element(Light_System_Base* base, IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
    : IGUIElement(EGUIET_ELEMENT, environment, parent, id, rectangle), my_base{base}
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

void Module_GUI_Element::resize(core::dimension2di nsize)
{
    this->DesiredRect = recti{ this->RelativeRect.UpperLeftCorner,nsize };
    this->updateAbsolutePosition();
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

    IGUIElement::draw();
}

void Module_GUI_Element::move(vector2di d)
{
    m_module->pos.X += d.X;
    m_module->pos.Y += d.Y;

    my_base->cached_texts[0]->pos[module_index] += d;
    my_base->cached_texts[1]->pos[module_index] += d;
    //my_base->cached_texts->move(module_index, d);
    this->Parent->move(d);

    //IGUIElement::move(d);
}

//======================================================
// Hookup_GUI_Element
//



Hookup_GUI_Element::Hookup_GUI_Element(Light_System_Base* base, IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
    : IGUIElement(EGUIET_ELEMENT, environment, parent, id, rectangle), my_base{ base }
{

}

void Hookup_GUI_Element::draw()
{
    gui::IGUISkin* skin = Environment->getSkin();
    video::IVideoDriver* driver = Environment->getVideoDriver();

    if(bMouseDown)
    {
        ((MySkin*)skin)->draw3DButtonPanePressed(this,
            AbsoluteRect,
            &my_base->m_panel->getViewPanel()->getAbsoluteClippingRect());
    }
    else
    {
        ((MySkin*)skin)->draw3DButtonPaneStandard(this,
            AbsoluteRect,
            &my_base->m_panel->getViewPanel()->getAbsoluteClippingRect());
    }

    IGUIElement::draw();
}

bool Hookup_GUI_Element::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_MOUSE_INPUT_EVENT)
    {
        vector2di mouse_pos = vector2di{ event.MouseInput.X,event.MouseInput.Y };

        switch (event.MouseInput.Event)
        {
        case EMIE_LMOUSE_LEFT_UP:
            bMouseDown = false;
            if (mouse_pos == click_pos)
            {
                left_click();
            }
            break;
        case EMIE_LMOUSE_PRESSED_DOWN:
            click_pos = mouse_pos;

            if (!bMouseDown)
            {
                bMouseDown = true;
            }
            break;
        }

        return true;
    }

    return IGUIElement::OnEvent(event);
}

void Hookup_GUI_Element::left_click()
{
}

/*
void Hookup_GUI_Element::move(vector2di d)
{
}
*/

//======================================================
// Vulkan Module
//

int reflect::TypeDescriptor_VkMod_Struct::calculate_height()
{
    int h = 8;
    int n_inputs = 0;

    for (reflect::Member& m : this->members)
    {
        reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

        if (m_tD->inherited_type == &reflect::input_type::Reflection)
        {
        h += 20;
        n_inputs++;
        }
    }

    if(n_inputs > 0)
        h += 8;

    for (reflect::Member& m : this->members)
    {
        reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

        if (m_tD->inherited_type == &reflect::output_type::Reflection)
        {
            h += 20;
        }
    }

    h += 2;

    return h;
}

reflect::input_type* reflect::TypeDescriptor_VkMod_Struct::get_input0(void* obj)
{
    for (reflect::Member& m : this->members)
    {
        reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

        if (m_tD->inherited_type == &reflect::input_type::Reflection)
        {
            return (reflect::input_type*)m.get(obj);
        }
    }

    return nullptr;
}

reflect::output_type* reflect::TypeDescriptor_VkMod_Struct::get_output0(void* obj)
{
    for (reflect::Member& m : this->members)
    {
        reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

        if (m_tD->inherited_type == &reflect::output_type::Reflection)
        {
            return (reflect::output_type*)m.get(obj);
        }
    }

    return nullptr;
}

int reflect::TypeDescriptor_VkMod_Struct::addComponents(Light_System_Base* base, gui::IGUIEnvironment* env, IGUIElement* module_base_element, CachedText& cache, vector2di module_pos, void* obj)
{
    dimension2di size{ 128,0 };

    int h = 8;
    int n_inputs = 0;
    int idx = 0;

    for (reflect::Member& m : this->members)
    {
        reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

        if (m_tD->inherited_type == &reflect::input_type::Reflection)
        {
            reflect::input_type* in = (reflect::input_type*)m.get(obj);

            std::string sname;

            if (m.alias)
                sname = m.alias;
            else
                sname = m.name;

            position2di pos{ -18, h };

            Hookup_GUI_Element* hookup = new Hookup_GUI_Element(base, env, module_base_element, -1, recti(pos + position2di{ 16,16 }, dimension2di{ 16,16 }));
            hookup->member_idx = idx;
            hookup->drop();

            in->m_gui_element = hookup;
            hookup->m_inout = in;
            
            h += 20;
            n_inputs++;

            base->font->getCachedText(cache, std::wstring(sname.begin(), sname.end()).c_str(), module_pos + position2di{ 0, h }, dimension2di{ 128,32 }, TKT_ALIGN_LEFT);
        }
        idx++;
    }

    if (n_inputs > 0)
        h += 8;

    idx = 0;
    for (reflect::Member& m : this->members)
    {
        reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

        if (m_tD->inherited_type == &reflect::output_type::Reflection)
        {
            reflect::output_type* out = (reflect::output_type*)m.get(obj);

            std::string sname;

            if (m.alias)
                sname = m.alias;
            else
                sname = m.name;

            //position2di pos{ -18, h };
            position2di pos{ (int)size.Width + 2, h };

            Hookup_GUI_Element* hookup = new Hookup_GUI_Element(base, env, module_base_element, -1, recti(pos + position2di{ 16,16 }, dimension2di{ 16,16 }));
            hookup->member_idx = idx;
            hookup->drop();

            h += 20;
            base->font->getCachedText(cache, std::wstring(sname.begin(), sname.end()).c_str(), module_pos + position2di{ 0, h }, dimension2di{ 128,32 }, TKT_ALIGN_RIGHT);

            out->m_gui_element = hookup;
            hookup->m_inout = out;
        }
        idx++;
    }

    h += 2;

    return h;
}

IGUIElement* reflect::TypeDescriptor_VkMod_Struct::get_gui_element(System_GUI_Element* system, Light_System_Base* base, gui::IGUIEnvironment* env, s32 id, core::rect<s32> rectangle, void* obj)
{
    vector2di pos = vector2di{ ((Vulkan_Module*)obj)->pos };

    int h = calculate_height();

    recti r0{ recti(pos - position2di{16,16}, dimension2du{128,(u32)h} + dimension2du{32,32}) };

    IGUIElement* module_base_element = new IGUIElement(EGUIET_ELEMENT, env, system, -1, r0);
    module_base_element->drop();

    Module_GUI_Element* element = new Module_GUI_Element(base, env, module_base_element, -1, recti(position2di{ 16,16 }, dimension2du{ 128,128 }));

    dimension2di size = dimension2di{ 128,128 };
    std::string mod_name;

    if (this->alias)
        mod_name = this->alias;
    else
        mod_name = (this->name);

    wstring wname = wstring(mod_name.begin(), mod_name.end());

    CachedText cache0;
    base->font->getCachedText(cache0, wname.c_str(), pos, dimension2di{ 128,32 }, TKT_ALIGN_LEFT);

    CachedText cache1;
    addComponents(base,env,module_base_element,cache1,pos,obj);
    size.Height = h;
    element->resize(size);

    base->cached_texts[0]->cache(cache0, position2di{ 0,-24 });
    base->cached_texts[1]->cache(cache1, position2di{ 0,-24 });

    element->text = wname;
    element->m_module = (Vulkan_Module*)obj;
    element->drop();

    return element;
    
}

//======================================================
// Widget
//

void Light_System_Widget::show()
{
    core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
    edit_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, Environment, this, -1, pr);

    OK_BUTTON_ID = my_ID + 1;

    my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, g_scene, my_base, my_ID + 2, pr);
    my_widget->show(false, my_base->getObj());

    int ypos = my_widget->getEditAreaHeight() + 8;

    ypos += 24;


    my_base->setWidgetHeight(ypos);

    core::rect<s32> tr = core::rect<s32>(32, ypos, getRelativePosition().getWidth() - 80, ypos + 24);

    my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, tr);
    my_button->setText(L"Ok");

    edit_panel->drop();
    my_widget->drop();

}

void Light_System_Widget::click_OK()
{
    my_base->save_workflow();
    cout << "click!\n";
}

void Light_System_Widget::onRefresh()
{
    //Display options

    int ypos = my_widget->getEditAreaHeight() + 8;

    core::rect<s32> pr2(0, ypos, getRelativePosition().getWidth(), ypos + 120);


    core::rect<s32> tr = core::rect<s32>(32, ypos, getRelativePosition().getWidth() - 80, ypos + 24);

    my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, tr);
    my_button->setText(L"Ok");

    my_base->setWidgetHeight(ypos);
}

bool Light_System_Widget::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_USER_EVENT)
    {
        if (event.UserEvent.UserData1 == USER_EVENT_REFLECTED_FORM_REFRESHED)
        {
            onRefresh();

            return true;
        }
    }
    else if (event.EventType == EET_GUI_EVENT)
    {
        s32 id = event.GUIEvent.Caller->getID();

        if (event.GUIEvent.EventType == EGET_BUTTON_CLICKED)
        {
            if (event.GUIEvent.Caller == (gui::IGUIElement*)my_button)
            {
                click_OK();
                return true;
            }
            else
            {
                FormField* field = my_widget->form->getFieldFromId(id);

                if (field)
                {
                    if (field->getButtonType() == FORM_FIELD_LABEL)
                    {
                        std::vector<int> leaf = field->tree_pos;
                        leaf.push_back(0); //node_class_item.id

                        reflect::Member* m = my_widget->m_typeDesc->getTreeNode(leaf);
                        size_t offset = my_widget->m_typeDesc->getTreeNodeOffset(leaf);

                        int sel;

                        m->type->copy(&sel, (char*)((char*)my_widget->temp_object) + offset);

                        //my_base->select(sel);

                        //my_widget->refresh();
                        //onRefresh();

                        MyEventReceiver* event_receiver = (MyEventReceiver*)device->getEventReceiver();
                        SEvent event;
                        event.EventType = EET_USER_EVENT;
                        event.UserEvent.UserData1 = USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED;
                        event_receiver->OnEvent(event);

                        return true;
                    }
                }
            }
            return true;
        }
        else if (event.GUIEvent.EventType == EGET_CHECKBOX_CHANGED)
        {
            //options_form->write(my_base->getOptions());
            my_base->refresh_panel_view();
        }
    }

    return IGUIElement::OnEvent(event);
}

Light_System_Widget::Light_System_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, Light_System_Base* base, s32 id, core::rect<s32> rect)
    : gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base), my_ID(id)
{
    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->Register(this);
}

Light_System_Widget::~Light_System_Widget()
{
    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->UnRegister(this);

    my_base->close_uv_panel();
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

void LM_System_Panel::startCameraPan(vector2di r)
{
    cameraDragStartPos = vector2di{ 0,0 };
}

void LM_System_Panel::cameraPan(vector2di mov)
{
    core::list<IGUIElement*>::ConstIterator it = system_gui_element->getChildren().begin();

    for (; it != system_gui_element->getChildren().end(); ++it)
    {
        (*it)->move(mov - cameraDragStartPos);
    }

    for(int i=0;i< m_base->cached_texts[0]->pos.size();i++)
    {
        m_base->cached_texts[0]->pos[i] += mov - cameraDragStartPos;
        m_base->cached_texts[1]->pos[i] += mov - cameraDragStartPos;
    }

    cameraDragStartPos = mov;
}

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
    someMaterial.Thickness = 2.0;
    someMaterial.MaterialType = video::EMT_SOLID;
    //someMaterial.

    

    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(someMaterial);

    this->m_base->draw_lines(driver);

    //if (bShowGrid)
    //this->drawGrid(driver, someMaterial);

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
    m_typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<lm_system_struct>::get();

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
            cameraQuad->hookup_fullscreen_panel(m_panel);
        }/*
        else if (mb_options.show_uv_view == false && m_panel->hooked_up() == true)
        {
            m_panel->getViewPanel()->disconnect();
        }*/

        if (m_panel->hooked_up() == true)
        {
            MyEventReceiver* event_receiver = (MyEventReceiver*)device->getEventReceiver();
            SEvent event;
            event.EventType = EET_USER_EVENT;
            event.UserEvent.UserData1 = USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED;
            event_receiver->OnEvent(event);
        }

        m_panel->clear_childern();

        if (m_panel->getViewPanel())
        {
            recti r{ position2di{0,0}, m_panel->getViewPanel()->getAbsoluteClippingRect().getSize() };
            
            System_GUI_Element* system_element = new System_GUI_Element(this, env, m_panel->getViewPanel(), -1, r);
            system_element->drop();

            m_panel->system_gui_element = system_element;

            int i = 0;
            for (Vulkan_Module* mod : workflow->items)
            {
                reflect::TypeDescriptor_VkMod_Struct* tD = (reflect::TypeDescriptor_VkMod_Struct*)mod->GetDynamicReflection();
                
                if (!(tD->flags & REFLECT_VKMOD_FLAG_TRANSITION_MODULE))
                {
                    Module_GUI_Element* element = (Module_GUI_Element*)tD->get_gui_element(system_element, this, env, -1, recti(position2di{ 16,16 }, dimension2du{ 128,128 }), mod);

                    element->module_index = i;
                    i++;
                }
            }

            for (Vulkan_Module* mod : workflow->items)
            {
                typedef std::pair<Hookup_GUI_Element*, Hookup_GUI_Element*> hookup_pair;
                reflect::TypeDescriptor_VkMod_Struct* tD = (reflect::TypeDescriptor_VkMod_Struct*)mod->GetDynamicReflection();
                
                //normal connections
                if (!(tD->flags & REFLECT_VKMOD_FLAG_TRANSITION_MODULE))
                {
                    for (reflect::Member& m : tD->members)
                    {
                        reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

                        if (m_tD->inherited_type == &reflect::output_type::Reflection)
                        {
                            reflect::output_type* out = (reflect::output_type*)m.get(mod);

                            for (reflect::input_type* in : out->dest_inputs)
                            {
                                this->gui_connections.push_back(hookup_pair(in->m_gui_element, out->m_gui_element));
                            }
                        }


                    }
                }
                else //connections that have a hidden transition module
                {
                    reflect::output_type* out0 = tD->get_output0(mod);
                    reflect::input_type* in0 = out0->dest_inputs[0];
                    
                    reflect::input_type* in1 = tD->get_input0(mod);
                    reflect::output_type* out1 = in1->src_output;

                    this->gui_connections.push_back(hookup_pair(in0->m_gui_element, out1->m_gui_element));

                    cout << "new connection\n";
                }
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

void Light_System_Base::save_workflow()
{
    workflow->save();
    cout << "workflow saved\n";
}

Light_System_Base::~Light_System_Base()
{
}

void Light_System_Base::show()
{
    if (workflow)
        delete workflow;

    workflow = new Vulkan_Workflow();
    //workflow->make_default_workflow();
    workflow->load();
    
    //modules.clear();

    cached_texts[0] = new CachedText_Manager((AGG_TT_Font*)env->getSkin()->getFont());
    cached_texts[1] = new CachedText_Manager((AGG_TT_Font*)env->getSkin()->getFont());

    core::rect<s32> client_rect(core::vector2di(0, 0),
        core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
            this->panel->getClientRect()->getAbsolutePosition().getHeight()));

    Light_System_Widget* widget = new Light_System_Widget(env, this->panel->getClientRect(), this, GUI_ID_LIGHT_SYSTEM_BASE, client_rect);

    widget->show();
    widget->drop();
    
    refresh_panel_view();

    cached_texts[0]->fill_positions();
    cached_texts[1]->fill_positions();
}

s32 Light_System_Base::getWidgetHeight()
{
    return widget_total_height;
}

void Light_System_Base::setWidgetHeight(s32 h)
{
    widget_total_height = h;
}

void Light_System_Base::draw_text()
{
    const rect<s32> panel_rect = m_panel->getViewPanel()->getAbsoluteClippingRect();
    cached_texts[0]->draw(panel_rect.UpperLeftCorner, video::SColor(255, 80, 80, 80), &panel_rect);
    cached_texts[1]->draw(panel_rect.UpperLeftCorner, video::SColor(255, 70, 70, 160), &panel_rect);
}


void Light_System_Base::draw_lines(video::IVideoDriver* driver)
{
    typedef std::pair<Hookup_GUI_Element*, Hookup_GUI_Element*> hookup_pair;

    for (hookup_pair& hookup : gui_connections)
    {
        vector3df pos0;
        vector3df pos1;

       // this->m_panel->Get3DPosFromScreenCoords(hookup.first->getRelativePosition().UpperLeftCorner, pos0);
       // this->m_panel->Get3DPosFromScreenCoords(hookup.first->getRelativePosition().UpperLeftCorner, pos1);
        if(hookup.first && hookup.second)
        {
            vector2di panel_offset = m_panel->getViewPanel()->getAbsolutePosition().UpperLeftCorner;
            vector2di pos0 = hookup.first->getAbsolutePosition().UpperLeftCorner + vector2di{ 8,8 } - panel_offset;
            vector2di pos1 = hookup.second->getAbsolutePosition().UpperLeftCorner + vector2di{ 8,8 } - panel_offset;
            driver->draw2DLine(pos0, pos1, video::SColor(255, 180, 100, 120));
          //driver->draw3DLine(vector3df{ (f32)pos0.X+100,-100,(f32)pos0.Y+100 }, vector3df{ (f32)pos1.X-100,-100,(f32)pos1.Y-100 }, video::SColor(50, 150, 100, 255));
        }
    }
}

void Light_System_Base::set_scene(geometry_scene* gs)
{
    tool_base::set_scene(gs);

    if (m_panel)
        m_panel->set_scene(gs);
}

REFLECT_STRUCT_BEGIN(lm_system_struct)
    REFLECT_STRUCT_MEMBER(aVariable)
REFLECT_STRUCT_END()

