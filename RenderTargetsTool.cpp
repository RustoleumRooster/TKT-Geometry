
#include "RenderTargetsTool.h"
#include "myNodes.h"

using namespace irr;
using namespace video;

extern IrrlichtDevice* device;

Render_Tool_Base* Render_Tool::base = NULL;
multi_tool_panel* Render_Tool::panel = NULL;



//========================================================
// Panel
//

RenderTargetPanel::RenderTargetPanel(gui::IGUIEnvironment* environment, video::IVideoDriver* driver, gui::IGUIElement* parent, s32 id, core::rect<s32> rectangle) :
    TestPanel(environment, driver, parent, id, rectangle)
{
    //this->setImage(driver->getTexture("wall.bmp"));


}

RenderTargetPanel::~RenderTargetPanel()
{
    if (render_node)
        delete render_node;
}

scene::ICameraSceneNode* RenderTargetPanel::getCamera()
{
    return NULL;
}

void RenderTargetPanel::Initialize(scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    //smgr2 = smgr->createNewSceneManager();
    render_node = new TwoTriangleSceneNode(NULL, smgr, 9876);

    render_node->getMaterial(0).setTexture(0, driver->getTexture("wall.bmp"));
    render_node->getMaterial(0).MaterialType = my_material;

    TestPanel::Initialize(smgr, geo_scene);

}

void RenderTargetPanel::render()
{
    if (!cameraQuad)
        return;

    driver->setRenderTarget(getImage(), true, true, video::SColor(255, 16, 16, 16));

    //driver->draw2DImage(getImage(), G

    render_node->render();

    driver->setRenderTarget(0, true, true, video::SColor(0, 0, 0, 0));
}

void RenderTargetPanel::setMaterial(s32 someMaterial)
{
    my_material = video::E_MATERIAL_TYPE(someMaterial);
    render_node->getMaterial(0).MaterialType = my_material;
}

void RenderTargetPanel::resize(core::dimension2d<u32> new_size)
{


}

bool RenderTargetPanel::GetScreenCoords(core::vector3df V, core::vector2di& out_coords)
{
    //return Get3DScreenCoords(V, out_coords);
    return false;
}

void RenderTargetPanel::connect_image(ViewResizeObject* view)
{
    if (render_node && view && view->get_image(0))
    {
        render_node->getMaterial(0).setTexture(0, view->get_image(0));
    }
    else if (render_node)
    {
        render_node->getMaterial(0).setTexture(0, driver->getTexture("wall.bmp"));
    }
}


void RenderTargetPanel::SetMeshNodesVisible()
{

}


Render_Tool_Widget::Render_Tool_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* scene, Render_Tool_Base* base, s32 id, core::rect<s32> rect)
    : gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base), g_scene(scene), my_ID(id)
{

}
Render_Tool_Widget::~Render_Tool_Widget()
{
    my_base->close_panel();
}

void Render_Tool_Widget::show()
{

}

void Render_Tool_Base::setCameraQuad(CameraQuad* cameraQuad_)
{
    cameraQuad = cameraQuad_;

    if (view_panel)
        view_panel->setCameraQuad(cameraQuad);
}

void Render_Tool_Base::setRenderList(RenderList* renderList_)
{
    renderList = renderList_;
}

void Render_Tool_Base::close_panel()
{
    if (view_panel && view_panel->getViewPanel())
        view_panel->getViewPanel()->disconnect();

    if (view_panel)
    {
        delete view_panel;
        view_panel = NULL;
    }
}

void Render_Tool_Base::refresh_panel_view()
{
    if (cameraQuad && view_panel)
    {
        if (view_panel->hooked_up() == false)
        {
            cameraQuad->hookup_aux_panel(view_panel);
        }
        else if (view_panel->hooked_up() == true)
        {
            view_panel->getViewPanel()->disconnect();
        }

        if (view_panel->hooked_up() == true)
        {

        }
    }
}

void Render_Tool_Base::connect_image(ViewResizeObject* view)
{
    my_view = view;

    if (view_panel)
        view_panel->connect_image(view);
}

void Render_Tool_Base::init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
    reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);
    //material_group_struct::my_typeDesc* m_type = (material_group_struct::my_typeDesc*)(m->type);
    m->readwrite = true;

    if (tree_pos.size() > 1)
    {
        //m_type->my_attributes.selected = m_struct.material_groups[tree_pos[1]].selected;
    }
}

void Render_Tool_Base::write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor)
{
    /*
    for (int i = 0; i < m_struct.material_groups.size(); i++)
    {
        std::vector<int> tree_pos{ 1, i };
//
        reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);

        material_group_struct::my_typeDesc* m_type = (material_group_struct::my_typeDesc*)(m->type);
        m_type->my_attributes.selected = m_struct.material_groups[i].selected;
    }*/
}


ITexture* Render_Tool_Base::getRender()
{

    return NULL;
}

ITexture* Render_Tool_Base::getRTT()
{

    return nullptr;
}



Render_Tool_Base::Render_Tool_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel)
    :simple_reflected_tool_base(name,my_id,env,panel)
{
    m_typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<render_options_struct>::get();
}

void Render_Tool_Base::show()
{
    core::rect<s32> client_rect(core::vector2di(0, 0),
        core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
            this->panel->getClientRect()->getAbsolutePosition().getHeight()));

    Render_Tool_Widget* widget = new Render_Tool_Widget(env, this->panel->getClientRect(), g_scene, this, GUI_ID_MAT_BUFFERS_BASE, client_rect);

    widget->show();
    widget->drop();

    view_panel = new RenderTargetPanel(env, device->getVideoDriver(), NULL, 0, core::recti());
    view_panel->Initialize(smgr, g_scene);
    view_panel->setCameraQuad(cameraQuad);
    view_panel->setMaterial(my_material);
    view_panel->connect_image(my_view);

    refresh_panel_view();
}

void Render_Tool_Base::set_scene(geometry_scene* g_scene_, scene::ISceneManager* smgr_)
{
    tool_base::set_scene(g_scene_);
    smgr = smgr_;
}



REFLECT_STRUCT_BEGIN(Render_Tool_Base::render_options_struct)
REFLECT_STRUCT_MEMBER(some_int)
REFLECT_STRUCT_END()