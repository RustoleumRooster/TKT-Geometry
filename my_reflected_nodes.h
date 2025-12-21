#ifndef _TKT_MY_REFLECTED_NODES_
#define _TKT_MY_REFLECTED_NODES_

#include <irrlicht.h>
#include "Reflection.h"
#include "CameraPanel.h"
#include "TwoTrianglesNode.h"
#include "edit_env.h"
#include "reflected_nodes.h"

class Reflected_LightSceneNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_LightSceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
        Reflected_Sprite_SceneNode(parent, geo_scene, smgr, id, pos) {}

    virtual bool bShowEditorArrow() { return true; }
    virtual ESCENE_NODE_TYPE getType() { return ESNT_LIGHT; }

    virtual void render() { Reflected_Sprite_SceneNode::render(); }
    virtual void translate(core::matrix4);
    virtual void onClear() { my_light = NULL; }
    virtual void postEdit();
    virtual bool addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override;

    scene::ILightSceneNode* my_light = NULL;
    bool enabled = true;
    int light_radius = 50;
    REFLECT2()
};

class Reflected_SimpleEmitterSceneNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_SimpleEmitterSceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
        Reflected_Sprite_SceneNode(parent, geo_scene, smgr, id, pos) {}

    virtual bool bShowEditorArrow() { return true; }

    virtual void render();
    virtual bool addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override;

    core::vector3df EmitBox = core::vector3df(64, 64, 64);
    TextureInfo* texture = NULL;
    core::vector2df particle_scale = core::vector2df(10, 10);
    int minParticlesPerSecond = 10;
    int maxParticlesPerSecond = 25;
    int lifeTimeMin = 500;
    int lifeTimeMax = 1000;
    bool bool_A;
    bool bool_B;
    video::SColor color;
    core::vector3df Vector_A = core::vector3df(64, 64, 64);
    core::vector3df Vector_B = core::vector3df(64, 64, 64);
    core::vector3df Vector_C = core::vector3df(64, 64, 64);

    REFLECT2()
};

class Reflected_SkyNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_SkyNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);
    ~Reflected_SkyNode();

    virtual bool bShowEditorArrow() { return true; }

    virtual bool addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override;
    virtual void onSceneInit() override;

    virtual void preEdit() override;
    virtual void postEdit() override;
    virtual void endScene() override;

    int my_material_group = 44;

    bool enabled;
    reflect::uid_reference target;
  
    MySkybox_SceneNode* regular_skynode = NULL;
    MySkybox_SceneNode* underwater_skynode = NULL;
    
    REFLECT2()
};

class Reflected_WaterSurfaceNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_WaterSurfaceNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);
    ~Reflected_WaterSurfaceNode();

    virtual bool bShowEditorArrow() { return true; }

    virtual bool addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override;
    virtual void onSceneInit() override;

    virtual void preEdit() override;
    virtual void postEdit() override;
    virtual void endScene() override;

    void connect_sky_sceneNode(MySkybox_SceneNode*);

    bool enabled = true;
    reflect::uid_reference target;
    
    WaterSurface_SceneNode* my_node = NULL;

    REFLECT2()
};

class Reflected_MeshBuffer_Clouds_SceneNode : public Reflected_MeshBuffer_SceneNode
{
public:

    Reflected_MeshBuffer_Clouds_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);
    ~Reflected_MeshBuffer_Clouds_SceneNode();

    virtual bool addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override;
    virtual void onSceneInit() override;

    REFLECT2()
};

class Reflected_MeshBuffer_AreaLight_SceneNode : public Reflected_MeshBuffer_SceneNode
{
public:

    Reflected_MeshBuffer_AreaLight_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);
    ~Reflected_MeshBuffer_AreaLight_SceneNode();

    virtual bool addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override;
    virtual void onSceneInit() override;

    u32 intensity = 1000;
    u32 power;                  //this is a transient value
    core::vector3df normal;     //transient

    REFLECT2()
};


#endif
