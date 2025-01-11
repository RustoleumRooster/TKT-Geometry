#include <irrlicht.h>
#include "reflection.h"
#include "reflected_nodes.h"
#include "CameraPanel.h"
#include "utils.h"
#include "geometry_scene.h"
#include "GUI_tools.h"
#include "my_reflected_nodes.h"
#include "TwoTrianglesNode.h"
#include "CMeshSceneNode.h"
#include "material_groups.h"
#include "my_nodes.h"

using namespace irr;
using namespace video;

extern IrrlichtDevice* device;
extern f32 global_clipping_plane[4];
extern SceneCoordinator* gs_coordinator;

extern float g_time;

//============================================================================
//  Reflected_LightSceneNode
//

REFLECT_STRUCT2_BEGIN(Reflected_LightSceneNode)
    ALIAS("Light")
    INHERIT_FROM(Reflected_Sprite_SceneNode)
    REFLECT_STRUCT2_MEMBER(enabled)
    REFLECT_STRUCT2_MEMBER(light_radius)
REFLECT_STRUCT2_END()

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

bool Reflected_LightSceneNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    scene::ILightSceneNode* light = smgr->addLightSceneNode(parent, getPosition(), video::SColorf(1.0, 1.0, 1.0), this->light_radius, this->ID);
    this->my_light = light;

    return true;
}


//============================================================================
//  Reflected_SimpleEmitterSceneNode
//

REFLECT_STRUCT2_BEGIN(Reflected_SimpleEmitterSceneNode)
    ALIAS("Particle Emitter")
    INHERIT_FROM(Reflected_Sprite_SceneNode)

    REFLECT_STRUCT2_MEMBER(EmitBox)
    REFLECT_STRUCT2_MEMBER(texture)
    REFLECT_STRUCT2_MEMBER(particle_scale)
    REFLECT_STRUCT2_MEMBER(minParticlesPerSecond)
    REFLECT_STRUCT2_MEMBER(maxParticlesPerSecond)
    REFLECT_STRUCT2_MEMBER(lifeTimeMin)
    REFLECT_STRUCT2_MEMBER(lifeTimeMax)

    REFLECT_STRUCT2_MEMBER(bool_A)
    REFLECT_STRUCT2_MEMBER(bool_B)
    REFLECT_STRUCT2_MEMBER(color)
    REFLECT_STRUCT2_MEMBER(Vector_A)
    REFLECT_STRUCT2_MEMBER(Vector_B)
    REFLECT_STRUCT2_MEMBER(Vector_C)
REFLECT_STRUCT2_END()

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


bool Reflected_SimpleEmitterSceneNode::addSelfToScene(USceneNode* parent, scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    //std::cout<<"go!\n";
    core::aabbox3df box = core::aabbox3df(Location.X - EmitBox.X / 2,
        Location.Y - EmitBox.Y / 2,
        Location.Z - EmitBox.Z / 2,
        Location.X + EmitBox.X / 2,
        Location.Y + EmitBox.Y / 2,
        Location.Z + EmitBox.Z / 2);

    //std::cout<<Location.X<<","<<Location.Y<<","<<Location.Z<<"\n";

    scene::IParticleSystemSceneNode* ps = smgr->addParticleSystemSceneNode(false, parent, -1, vector3df(0.0,0.0,0.0), Rotation, vector3df(1.0, 1.0, 1.0));
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
    return true;
}

//============================================================================
//  Reflected Water Surface Node
//

REFLECT_STRUCT2_BEGIN(Reflected_WaterSurfaceNode)
    ALIAS("Water Surface")
    REFLECT_STRUCT2_MEMBER(enabled)
    REFLECT_STRUCT2_MEMBER(target)
    INHERIT_FROM(Reflected_Sprite_SceneNode)
REFLECT_STRUCT2_END()

Reflected_WaterSurfaceNode::Reflected_WaterSurfaceNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_Sprite_SceneNode(parent, geo_scene, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("triangle_symbol.png");
    Buffer->Material.setTexture(0, m_texture);
}

bool Reflected_WaterSurfaceNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    if (!enabled)
        return false;

    WaterSurface_SceneNode* node = new WaterSurface_SceneNode(parent, smgr, 77, Location, Rotation, geo_scene);
    node->drop();

    node_instance_id = node->getID();


    //==================

    bool dirty = false;

    std::vector<Reflected_MeshBuffer_SceneNode*> buffer_nodes;

    resolve_uid_references<Reflected_MeshBuffer_SceneNode>(geo_scene, target, buffer_nodes);

    polyfold* pf = geo_scene->geoNode()->get_total_geometry();

    for (Reflected_MeshBuffer_SceneNode* n: buffer_nodes)
    {
        for (int f_i = 0; f_i < pf->faces.size(); f_i++)
        {
            if (pf->faces[f_i].uid == n->get_uid())
            {
                MeshBuffer_Chunk chunk = geo_scene->geoNode()->final_meshnode_interface.get_mesh_buffer_by_face(f_i);

                IMeshBuffer* buffer = chunk.buffer;

                node->attach_to_buffer(buffer);
            }
        }
    }

    //caller calls
    //geo_scene->getMeshNode()->copyMaterials();

    return true;
}

void Reflected_WaterSurfaceNode::preEdit()
{
    target.old_uids = target.uids;

    Reflected_Sprite_SceneNode::preEdit();
}

void Reflected_WaterSurfaceNode::postEdit()
{
    for (u64 uid : target.old_uids)
    {
        Reflected_SceneNode* node = geo_scene->get_reflected_node_by_uid(uid);

        if (node)
        {
            node->disconnect(this);
        }
    }

    for (u64 uid : target.uids)
    {
        Reflected_SceneNode* node = geo_scene->get_reflected_node_by_uid(uid);

        if (node)
        {
            node->connect_input(this);
        }
    }

    Reflected_Sprite_SceneNode::postEdit();
}

void Reflected_WaterSurfaceNode::endScene()
{
    for (u64 uid : target.uids)
    {
        Reflected_SceneNode* node = geo_scene->get_reflected_node_by_uid(uid);
        Reflected_MeshBuffer_SceneNode* face_node = dynamic_cast<Reflected_MeshBuffer_SceneNode*>(node);

        if (face_node)
        {
            face_node->restore_original_texture();
        }
    }
}

Reflected_PointNode::Reflected_PointNode(USceneNode* parent, geometry_scene* geo_scene, ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_Sprite_SceneNode(parent, geo_scene, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("triangle_symbol.png");
    Buffer->Material.setTexture(0, m_texture);

}

//===========================================================
// Reflected Skybox Node
//

REFLECT_STRUCT2_BEGIN(Reflected_SkyNode)
    ALIAS("Skybox Node")
    INHERIT_FROM(Reflected_Sprite_SceneNode)
    REFLECT_STRUCT2_MEMBER(enabled)
    REFLECT_STRUCT2_MEMBER(target)
REFLECT_STRUCT2_END()

Reflected_SkyNode::Reflected_SkyNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_Sprite_SceneNode(parent, geo_scene, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("triangle_symbol.png");
    Buffer->Material.setTexture(0, m_texture);
}

Reflected_SkyNode::~Reflected_SkyNode()
{
    gs_coordinator->set_skyox_dirty();
    gs_coordinator->SetAllFinalMeshDirty();
}

bool Reflected_SkyNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    if (!enabled)
        return false;

    MySkybox_SceneNode* node = new MySkybox_SceneNode(parent, smgr, -1, Location, Rotation, geo_scene);
    node->WaterReflectionPass = false;
    node->drop();


    MySkybox_SceneNode* node1 = new MySkybox_SceneNode(parent, smgr, -1, Location, Rotation, geo_scene);
    node1->WaterReflectionPass = true;
    node1->drop();

    node_instance_id = node->getID();

    bool dirty = false;

    //==================

    int c = 0;
    for (geometry_scene* a_geo_scene : gs_coordinator->scenes)
    {
        std::vector<Reflected_MeshBuffer_SceneNode*> buffer_nodes;

        resolve_uid_references<Reflected_MeshBuffer_SceneNode>(a_geo_scene, target, buffer_nodes);

        polyfold* pf = a_geo_scene->geoNode()->get_total_geometry();

        for (Reflected_MeshBuffer_SceneNode* n : buffer_nodes)
        {
            for (int f_i = 0; f_i < pf->faces.size(); f_i++)
            {
                if (pf->faces[f_i].uid == n->get_uid())
                {
                    MeshBuffer_Chunk chunk = a_geo_scene->geoNode()->final_meshnode_interface.get_mesh_buffer_by_face(f_i);

                    IMeshBuffer* buffer = chunk.buffer;

                    node->attach_to_buffer(buffer);
                    node1->attach_to_buffer(buffer);
                    c++;
                }
            }
        }
    }

    //caller calls
    //geo_scene->getMeshNode()->copyMaterials();

    return true;
}

void Reflected_SkyNode::preEdit()
{
    //target.old_uids = target.uids;

    Reflected_Sprite_SceneNode::preEdit();
}

void Reflected_SkyNode::postEdit()
{

    gs_coordinator->set_skyox_dirty();

    /*
    for (u64 uid : target.old_uids)
    {
        Reflected_SceneNode* node = geo_scene->get_reflected_node_by_uid(uid);

        if (node)
        {
            node->disconnect(this);
        }
    }

    for (u64 uid : target.uids)
    {
        Reflected_SceneNode* node = geo_scene->get_reflected_node_by_uid(uid);

        if (node)
        {
            node->connect_input(this);
        }
    }
    */

    Reflected_Sprite_SceneNode::postEdit();
}

void Reflected_SkyNode::endScene()
{
}

