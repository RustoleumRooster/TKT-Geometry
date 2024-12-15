#include <irrlicht.h>
#include "reflection.h"
#include "reflected_nodes.h"
#include "CameraPanel.h"
#include "utils.h"
#include "geometry_scene.h"
#include "GUI_tools.h"
#include "custom_nodes.h"
#include "myNodes.h"
#include "CMeshSceneNode.h"

using namespace irr;
using namespace video;

extern IrrlichtDevice* device;
extern f32 global_clipping_plane[4];

Render_Tool_Base* Render_Tool::base = NULL;
multi_tool_panel* Render_Tool::panel = NULL;

//============================================================================
//  Reflected_LightSceneNode
//



void Reflected_LightSceneNode::postEdit()
{
    if (my_light)
    {
        my_light->setRadius(light_radius);
        my_light->setVisible(enabled);
    }

    Reflected_SceneNode::postEdit();
}

void Reflected_LightSceneNode::translate(core::matrix4 M)
{
    core::vector3df p = this->getPosition();
    M.translateVect(p);
    if (my_light)
        my_light->setPosition(p);

    Reflected_SceneNode::translate(M);
}

void Reflected_LightSceneNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    scene::ILightSceneNode* light = smgr->addLightSceneNode(parent, getPosition(), video::SColorf(1.0, 1.0, 1.0), this->light_radius, this->ID);
    this->my_light = light;
}


//============================================================================
//  Reflected_SimpleEmitterSceneNode
//

//================= Reflection Registration




void Reflected_SimpleEmitterSceneNode::render()
{
    video::IVideoDriver* driver = SceneManager->getVideoDriver();

    if (bSelected)
    {/*
        core::aabbox3df box = core::aabbox3df(getPosition().X - EmitBox.X /2,
                                              getPosition().Y - EmitBox.Y /2,
                                              getPosition().Z - EmitBox.Z /2,
                                              getPosition().X + EmitBox.X /2,
                                              getPosition().Y + EmitBox.Y /2,
                                              getPosition().Z + EmitBox.Z /2);*/
        core::aabbox3df box = core::aabbox3df(Location.X - EmitBox.X / 2,
            Location.Y - EmitBox.Y / 2,
            Location.Z - EmitBox.Z / 2,
            Location.X + EmitBox.X / 2,
            Location.Y + EmitBox.Y / 2,
            Location.Z + EmitBox.Z / 2);

        draw_box(driver, box);
    }

    Reflected_Sprite_SceneNode::render();
}


void Reflected_SimpleEmitterSceneNode::addSelfToScene(USceneNode* parent, scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    //std::cout<<"go!\n";
    core::aabbox3df box = core::aabbox3df(Location.X - EmitBox.X / 2,
        Location.Y - EmitBox.Y / 2,
        Location.Z - EmitBox.Z / 2,
        Location.X + EmitBox.X / 2,
        Location.Y + EmitBox.Y / 2,
        Location.Z + EmitBox.Z / 2);

    //std::cout<<Location.X<<","<<Location.Y<<","<<Location.Z<<"\n";

    scene::IParticleSystemSceneNode* ps = smgr->addParticleSystemSceneNode(); //TODO: parent
    /*
    scene::IParticleEmitter* em = ps->createBoxEmitter(core::aabbox3df(-5000,500,-5000,5000,1000,5000),
                                                        core::vector3df(0,0.0,0),
                                                        500,1000,
                                                        video::SColor(255,255,255,255),
                                                        video::SColor(255,255,255,255),
                                                        400,1000);
                                                        */

    scene::IParticleEmitter* em = ps->createBoxEmitter(box,
        core::vector3df(0, 0.0, 0),
        minParticlesPerSecond,
        maxParticlesPerSecond,
        video::SColor(255, 255, 255, 255),
        video::SColor(255, 255, 255, 255),
        lifeTimeMin, lifeTimeMax);
    em->setMinStartSize(core::dimension2df(32, 32));
    em->setMaxStartSize(core::dimension2df(48, 48));
    em->setDirection(core::vector3df(0, 0.09, 0));

    ps->setMaterialFlag(video::EMF_LIGHTING, false);
    video::IVideoDriver* driver = smgr->getVideoDriver();
    ps->setMaterialTexture(0, driver->getTexture("fireball.bmp"));
    //ps->setMaterialTexture(0,this->texture);
    ps->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    ps->setEmitter(em);
    em->drop();

    scene::IParticleAffector* paf = ps->createFadeOutParticleAffector();
    ps->addAffector(paf);
    paf->drop();

    paf = ps->createScaleParticleAffector(particle_scale);
    ps->addAffector(paf);
    paf->drop();

    //paf = ps->createGravityAffector()

  /*
    scene::IParticleSystemSceneNode* ps = smgr->addParticleSystemSceneNode();
    scene::IParticleEmitter* em = ps->createBoxEmitter(core::aabbox3df(-5000,500,-5000,5000,1000,5000),
                                                        core::vector3df(0,0.0,0),
                                                        500,1000,
                                                        video::SColor(255,255,255,255),
                                                        video::SColor(255,255,255,255),
                                                        400,1000);
    ps->setMaterialFlag(video::EMF_LIGHTING, false);
    ps->setMaterialTexture(0,driver->getTexture("sun.jpg"));
    ps->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    em->setMinStartSize(core::dimension2df(16,16));
    em->setMaxStartSize(core::dimension2df(32,32));
    ps->setEmitter(em);
    em->drop();

    scene::IParticleAffector* paf = ps->createFadeOutParticleAffector();
    ps->addAffector(paf);
    paf->drop();

    paf = ps->createScaleParticleAffector(core::dimension2df(64,64));
    ps->addAffector(paf);
    paf->drop();
    */
}

Reflected_WaterSurfaceNode::Reflected_WaterSurfaceNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_Sprite_SceneNode(parent, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("triangle_symbol.png");
    Buffer->Material.setTexture(0, m_texture);
}

void Reflected_WaterSurfaceNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{

    WaterSurface_SceneNode* node = new WaterSurface_SceneNode(parent, smgr, random_number(), Location, Rotation, geo_scene);
    node->drop();

    node_instance_id = node->getID();
}

Reflected_PointNode::Reflected_PointNode(USceneNode* parent, ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_Sprite_SceneNode(parent, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("triangle_symbol.png");
    Buffer->Material.setTexture(0, m_texture);

}

//============================================================
// Water Surface Scene Node
//

WaterSurface_SceneNode::WaterSurface_SceneNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos, const core::vector3df rotation, geometry_scene* geo_scene)
    :ISceneNode(smgr->getRootSceneNode(), smgr, id, pos, rotation, vector3df(1.0, 1.0, 1.0)), geo_scene(geo_scene)
{
    my_camera = smgr->addCameraSceneNode(NULL, vector3df(0, 0, 0), vector3df(0, 0, 0), -1, false);
    my_camera->grab();

    ((MyEventReceiver*)device->getEventReceiver())->Register_ViewResize(this);

}

WaterSurface_SceneNode::~WaterSurface_SceneNode()
{
    ((MyEventReceiver*)device->getEventReceiver())->UnRegister_ViewResize(this);

    IVideoDriver* driver = SceneManager->getVideoDriver();

    if (my_rtt)
    {
        driver->removeTexture(my_rtt);
        my_rtt = NULL;
    }

    if (my_rtt2)
    {
        driver->removeTexture(my_rtt2);
        my_rtt2 = NULL;
    }

    my_camera->drop();
    my_camera = NULL;
}

void WaterSurface_SceneNode::resizeView(core::dimension2du new_size)
{
    IVideoDriver* driver = SceneManager->getVideoDriver();

    my_camera->setAspectRatio((f32)new_size.Width / (f32)new_size.Height);
    my_camera->updateAbsolutePosition();

    if (my_rtt)
    {
        driver->removeTexture(my_rtt);
        my_rtt = NULL;
    }

    my_rtt = driver->addRenderTargetTexture(new_size, "RTT_00", irr::video::ECF_A8R8G8B8);

    if (my_rtt2)
    {
        driver->removeTexture(my_rtt2);
        my_rtt2 = NULL;
    }

    core::dimension2d<u32> half_size(new_size.Width * 0.5, new_size.Height * 0.5);

    my_rtt2 = driver->addRenderTargetTexture(half_size, "RTT_01", irr::video::ECF_A8R8G8B8);
}

void WaterSurface_SceneNode::render()
{
    if (geo_scene->getMeshNode()->isVisible() == false)
        return;
    /*
    driver->setRenderTarget(getImage(), true, true, video::SColor(255, 16, 16, 16));

    ICameraSceneNode* fp_camera = SceneManager->getActiveCamera();

    vector3df cam_pos = fp_camera->getAbsolutePosition();
    vector3df cam_rot = fp_camera->getRotation();
    if (cam_pos.Y > getAbsolutePosition().Y)
    {
        f32 y_pos = this->getAbsolutePosition().Y - (cam_pos.Y - this->getAbsolutePosition().Y);
        this->camera->setPosition(vector3df(cam_pos.X, y_pos, cam_pos.Z));
        this->camera->setRotation(vector3df(cam_rot.X, -cam_rot.Y, cam_rot.Z));

        SceneManager->setActiveCamera(this->camera);
        this->camera->render();
        SceneManager->drawAll();

    }
    cout << "Rendering!\n";*/

    ICameraSceneNode* fp_camera = SceneManager->getActiveCamera();

    IVideoDriver* driver = SceneManager->getVideoDriver();

    vector3df cam_pos = fp_camera->getAbsolutePosition();
    vector3df cam_target = fp_camera->getTarget();

    if (cam_pos.Y > 0)
    {
        f32 y_pos = -cam_pos.Y;
        my_camera->setPosition(vector3df(cam_pos.X, y_pos, cam_pos.Z));
        my_camera->setTarget(vector3df(cam_target.X, -cam_target.Y, cam_target.Z));
    }

    for (int f_i : hide_faces)
    {
       // geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_i).buffer->getMaterial().FrontfaceCulling = true;
       // geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_i).buffer->getMaterial().BackfaceCulling = true;
    }
    // geo_scene->getMeshNode()->copyMaterials();

    //=================
    //Render from the reflection camera (above water)

    ITexture* rtt = my_rtt; // get_rtt();

    driver->setRenderTarget(rtt, true, true, video::SColor(255, 16, 16, 16));

    global_clipping_plane[0] = 0.0;
    global_clipping_plane[1] = 1.0;
    global_clipping_plane[2] = 0.0;
    global_clipping_plane[3] = 0.0;

    driver->enableClipPlane(0, true);


    SceneManager->setActiveCamera(my_camera);

    my_camera->render();
    
    geo_scene->getMeshNode()->render();


    //=================
    //Render from the normal camera (underwater)

    ITexture* rtt2 = my_rtt2;// get_rtt2();

    driver->setRenderTarget(rtt2, true, true, video::SColor(255, 16, 16, 16));
    plane3df clip_plane2(vector3df(0, 4.0, 0), vector3df(0, -1, 0));

    SceneManager->setActiveCamera(fp_camera);

    fp_camera->render();
    geo_scene->getMeshNode()->render();


    global_clipping_plane[0] = 0.0;
    global_clipping_plane[1] = -1.0;
    global_clipping_plane[2] = 0.0;
    global_clipping_plane[3] = 0.0;

    geo_scene->getMeshNode()->render();

    video::SMaterial some_material;
    some_material.MaterialType = underwater_material;

    geo_scene->getMeshNode()->render_with_material(some_material);

    driver->enableClipPlane(0, false);

    //===============
    //Render the Image

    //render_node->getMaterial(0).MaterialType = render_material;
    //render_node->setMaterialTexture(0, rtt);
    //render_node->setMaterialTexture(1, rtt2);
    //render_node->setVisible(true);
    //render_node->mirrorUVs();
    ////render_node->defaultUVs();

   // driver->setRenderTarget(getImage(), true, true, video::SColor(255, 0, 0, 0));

    //render_node->render();

    driver->setRenderTarget(NULL, true, true, video::SColor(0, 0, 0, 0));

    for (int f_i : hide_faces)
    {
       // geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_i).buffer->getMaterial().FrontfaceCulling = false;
       // geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_i).buffer->getMaterial().BackfaceCulling = true;
    }
    geo_scene->getMeshNode()->copyMaterials();
}


void WaterSurface_SceneNode::OnRegisterSceneNode()
{

    if (IsVisible)
        SceneManager->registerNodeForRendering(this, ESNRP_TRANSPARENT_EFFECT);

    ISceneNode::OnRegisterSceneNode();
}

//========================================================
// Panel
//

ReflectionTestPanel::~ReflectionTestPanel()
{
    if (my_rtt)
    {
        driver->removeTexture(my_rtt);
        my_rtt = NULL;
    }

    if (my_rtt2)
    {
        driver->removeTexture(my_rtt2);
        my_rtt2 = NULL;
    }

    if (render_node)
    {
        delete render_node;
        render_node = NULL;
    }
}

scene::ICameraSceneNode* ReflectionTestPanel::getCamera()
{
    if (this->camera == NULL)
    {
        if (this->smgr)
        {
            this->camera = smgr->addCameraSceneNode(0, core::vector3df(-300, 400, -300), core::vector3df(0, 0, 0), -1, false);
            this->camera->setNearValue(5.0);
            //this->camera->setPosition(core::vector3df(-300,400,-300));
            //this->camera->setTarget(core::vector3df(0,0,0));
        }
    }
    return this->camera;
}

void ReflectionTestPanel::Initialize(scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    //smgr2 = smgr->createNewSceneManager();
    render_node = new TwoTriangleSceneNode(NULL, smgr, 9876);

    TestPanel::Initialize(smgr, geo_scene);

    hide_faces.clear();
    /*
    for (int i = 0; i < geo_scene->get_total_geometry()->faces.size(); i++)
    {
        if (geo_scene->get_total_geometry()->faces[i].material_group == 4)
        {
            if (geo_scene->get_total_geometry()->faces[i].loops.size() > 0)
            {
                hide_faces.push_back(i);
            }
        }
    }*/
}

void ReflectionTestPanel::render()
{
    if (!cameraQuad)
        return;

    vector3df cam_pos = cameraQuad->get_fp_camera_pos();
    vector3df cam_target = cameraQuad->get_fp_camera()->getTarget();

    if (cam_pos.Y > 0)
    {
        //cameraQuad->get_fp_camera()->
        //cout << cam_target.Y << "\n";
        f32 y_pos = -cam_pos.Y;
        getCamera()->setPosition(vector3df(cam_pos.X, y_pos, cam_pos.Z));
        getCamera()->setTarget(vector3df(cam_target.X, -cam_target.Y, cam_target.Z));
    }

    for (int f_i : hide_faces)
    {
       // geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_i).buffer->getMaterial().FrontfaceCulling = true;
       // geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_i).buffer->getMaterial().BackfaceCulling = true;
    }
    geo_scene->getMeshNode()->copyMaterials();
    //geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(5).buffer->getMaterial().FrontfaceCulling = true;
    //geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(5).buffer->getMaterial().BackfaceCulling = true;

    //=================
    //Render from the reflection camera (above water)

    ITexture* rtt = get_rtt();

    driver->setRenderTarget(rtt, true, true, video::SColor(255, 16, 16, 16));

    global_clipping_plane[0] = 0.0;
    global_clipping_plane[1] = 1.0;
    global_clipping_plane[2] = 0.0;
    global_clipping_plane[3] = 0.0;

    driver->enableClipPlane(0, true);


    smgr->setActiveCamera(getCamera());

    getCamera()->render();

    //smgr->drawAll();
    geo_scene->getMeshNode()->render();

   // driver->setClipPlane(0, clip_plane, false);
    

    //=================
    //Render from the normal camera (underwater)

    ITexture* rtt2 = get_rtt2();

    driver->setRenderTarget(rtt2, true, true, video::SColor(255, 16, 16, 16));
    plane3df clip_plane2(vector3df(0, 4.0, 0), vector3df(0, -1, 0));

    //driver->setClipPlane(0, clip_plane2, true);
    smgr->setActiveCamera(cameraQuad->get_fp_camera());
    //geo_scene->getMeshNode()->render();

    cameraQuad->get_fp_camera()->render();
    geo_scene->getMeshNode()->render();

    //smgr->drawAll();

    global_clipping_plane[0] = 0.0;
    global_clipping_plane[1] = -1.0;
    global_clipping_plane[2] = 0.0;
    global_clipping_plane[3] = 0.0;

   // geo_scene->getMeshNode()->render();

    video::SMaterial some_material;
    some_material.MaterialType = underwater_material;

    geo_scene->getMeshNode()->render_with_material(some_material);

  //  driver->setClipPlane(0, clip_plane2, false);

    driver->enableClipPlane(0, false);

    //===============
    //Render the Image

    render_node->getMaterial(0).MaterialType = render_material;
    render_node->setMaterialTexture(0, rtt);
    render_node->setMaterialTexture(1, rtt2);
    render_node->setVisible(true);
    //render_node->mirrorUVs();
    render_node->defaultUVs();

    driver->setRenderTarget(getImage(), true, true, video::SColor(255, 0, 0, 0));

    render_node->render();

    driver->setRenderTarget(NULL, true, true, video::SColor(0, 0, 0, 0));

    for (int f_i : hide_faces)
    {
        //geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_i).buffer->getMaterial().FrontfaceCulling = false;
        //geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_i).buffer->getMaterial().BackfaceCulling = true;
    }
    geo_scene->getMeshNode()->copyMaterials();

}

void ReflectionTestPanel::resize(core::dimension2d<u32> new_size)
{
    this->getCamera()->setAspectRatio((f32)new_size.Width / (f32)new_size.Height);
    this->getCamera()->updateAbsolutePosition();

    if (my_rtt)
    {
        driver->removeTexture(my_rtt);
        my_rtt = NULL;
    }

    my_rtt = driver->addRenderTargetTexture(new_size, "RTT_00", irr::video::ECF_A8R8G8B8);

    if (my_rtt2)
    {
        driver->removeTexture(my_rtt2);
        my_rtt2 = NULL;
    }

    core::dimension2d<u32> half_size(new_size.Width * 0.5, new_size.Height * 0.5);

    my_rtt2 = driver->addRenderTargetTexture(half_size, "RTT_01", irr::video::ECF_A8R8G8B8);

}

bool ReflectionTestPanel::GetScreenCoords(core::vector3df V, core::vector2di& out_coords)
{
    //return Get3DScreenCoords(V, out_coords);
    return false;
}

ITexture* ReflectionTestPanel::getRender()
{
    return getImage();
}

video::ITexture* ReflectionTestPanel::get_rtt()
{
    return my_rtt;
}

video::ITexture* ReflectionTestPanel::get_rtt2()
{
    return my_rtt2;
}

void ReflectionTestPanel::SetMeshNodesVisible()
{
    if (geo_scene->getMeshNode())
    {
        geo_scene->getMeshNode()->setWireFrame(false);
        geo_scene->getMeshNode()->setVisible(true);
    }
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

void Render_Tool_Base::set_material(s32 material_type)
{
    render_material = (video::E_MATERIAL_TYPE) material_type;

    if (view_panel)
        view_panel->set_material(render_material);
}

void Render_Tool_Base::set_underwater_material(s32 material_type)
{
    underwater_material = (video::E_MATERIAL_TYPE)material_type;

    if (view_panel)
        view_panel->set_underwater_material(underwater_material);
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

        reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);

        material_group_struct::my_typeDesc* m_type = (material_group_struct::my_typeDesc*)(m->type);
        m_type->my_attributes.selected = m_struct.material_groups[i].selected;
    }*/
}


ITexture* Render_Tool_Base::getRender()
{
    if (view_panel)
    {
        return view_panel->getRender();
    }

    return NULL;
}

ITexture* Render_Tool_Base::getRTT()
{
    if (view_panel)
    {
        return view_panel->getRTT();
    }

    return nullptr;
}

ITexture* Render_Tool_Base::getRTT2()
{
    if (view_panel)
    {
        return view_panel->getRTT2();
    }

    return nullptr;
}

void Render_Tool_Base::show()
{
    core::rect<s32> client_rect(core::vector2di(0, 0),
        core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
            this->panel->getClientRect()->getAbsolutePosition().getHeight()));

    Render_Tool_Widget* widget = new Render_Tool_Widget(env, this->panel->getClientRect(), g_scene, this, GUI_ID_MAT_BUFFERS_BASE, client_rect);

    widget->show();
    widget->drop();

    view_panel = new ReflectionTestPanel(env, device->getVideoDriver(), NULL, 0, core::recti());
    view_panel->Initialize(smgr, g_scene);
    view_panel->setCameraQuad(cameraQuad);
    view_panel->set_material(render_material);
    view_panel->set_underwater_material(underwater_material);

    refresh_panel_view();
}

void Render_Tool_Base::initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_, scene::ISceneManager* smgr_)
{
    tool_base::initialize(name_, my_id, env_, g_scene_, panel_);
    m_typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<render_options_struct>::get();
    smgr = smgr_;
}



REFLECT_STRUCT_BEGIN(Render_Tool_Base::render_options_struct)
    REFLECT_STRUCT_MEMBER(some_int)
REFLECT_STRUCT_END()