#ifndef _TKT_CUSTOM_NODES_
#define _TKT_CUSTOM_NODES_

#include <irrlicht.h>
#include "Reflection.h"
#include "CameraPanel.h"
#include "myNodes.h"
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
    video::ITexture* texture = NULL;
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


class PlaneShaderCallBack : public video::IShaderConstantSetCallBack
{
public:

    virtual void OnSetConstants(video::IMaterialRendererServices* services, s32 userData) override;
};

class SkyShaderCallBack : public video::IShaderConstantSetCallBack
{
public:

    virtual void OnSetConstants(video::IMaterialRendererServices* services, s32 userData) override;
};


class VanillaShaderCallBack : public video::IShaderConstantSetCallBack
{
public:

    virtual void OnSetConstants(video::IMaterialRendererServices* services, s32 userData) override;
};

class myProjectionShaderCallback : public video::IShaderConstantSetCallBack
{
public:

    virtual void OnSetConstants(video::IMaterialRendererServices* services, s32 userData) override;

};

class MySkybox_SceneNode : public ISceneNode, public ViewResizeObject
{
public:
    MySkybox_SceneNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos, const core::vector3df rotation, geometry_scene*);
    ~MySkybox_SceneNode();

    virtual void resizeView(core::dimension2du newsize) override;

    virtual void render() override;
    virtual void OnRegisterSceneNode() override;
    virtual const core::aabbox3d<f32>& getBoundingBox() const override { return my_box; }

    virtual int get_image_count() override {return 1;}
    virtual video::ITexture* get_image(int n) { return my_rtt2; };

    void attach_to_buffer(IMeshBuffer*);
    //void detach_all();

    static void load_shaders();

    video::ITexture* getRTT() { return my_rtt2; }

    core::aabbox3d<f32> my_box;

    static s32 skyShaderType;
    static s32 xblurShaderType;
    static s32 yblurShaderType;
    static s32 skyPPShaderType;
    static s32 planeMaterialType;
    static s32 projectionShaderType;

    geometry_scene* geo_scene = NULL;

    //final texture
    video::ITexture* my_rtt = NULL;

    //two blur textures
    video::ITexture* my_rtt2 = 0;
    video::ITexture* my_rtt3 = 0;

    //scene::ICameraSceneNode* my_camera = NULL;

    bool WaterReflectionPass = false;

    TwoTriangleSceneNode* cloudLayerNode = NULL;
    TwoTriangleSceneNode* blurNode = NULL;

    scene::ICameraSceneNode* my_camera = NULL;

    std::vector<IMeshBuffer*> buffers;
};

class WaterSurface_SceneNode : public ISceneNode, public ViewResizeObject
{
public:
    WaterSurface_SceneNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos, const core::vector3df rotation, geometry_scene*);
    ~WaterSurface_SceneNode();

    //virtual scene::ICameraSceneNode* getCamera();
    virtual void resizeView(core::dimension2du newsize) override;

    virtual void render() override;
    virtual void OnRegisterSceneNode() override;
    virtual const core::aabbox3d<f32>& getBoundingBox() const override { return my_box; }

    virtual int get_image_count() override { return 1; }
    virtual video::ITexture* get_image(int n) { return my_rtt; };

    void set_material(s32 material_type) { render_material = (video::E_MATERIAL_TYPE)material_type; }
    void set_underwater_material(s32 material_type) { underwater_material = (video::E_MATERIAL_TYPE)material_type; }

    video::ITexture* getRTT() { return my_rtt; }
    video::ITexture* getRTT2() { return my_rtt2; }

    void attach_to_buffer(IMeshBuffer*);
    //void detach_all();

private:
    core::aabbox3d<f32> my_box;

    video::E_MATERIAL_TYPE render_material = video::EMT_SOLID;
    video::E_MATERIAL_TYPE underwater_material = video::EMT_SOLID;
    video::ITexture* my_rtt = NULL;
    video::ITexture* my_rtt2 = NULL;
    scene::ICameraSceneNode* my_camera = NULL;

   // TwoTriangleSceneNode* render_node = NULL;

    geometry_scene* geo_scene = NULL;

    std::vector<int> hide_faces;
    std::vector<IMeshBuffer*> buffers;
};


class Reflected_SkyNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_SkyNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);

    virtual bool bShowEditorArrow() { return true; }

    virtual bool addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override;

    virtual void preEdit() override;
    virtual void postEdit() override;
    virtual void endScene() override;

    int my_material_group = 44;

    bool enabled;
    reflect::uid_reference target;
  

    REFLECT2()
};

class Reflected_WaterSurfaceNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_WaterSurfaceNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);

    virtual bool bShowEditorArrow() { return true; }

    virtual bool addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override;

    virtual void preEdit() override;
    virtual void postEdit() override;
    virtual void endScene() override;

    int my_material_group = 55;

    bool enabled = true;
    reflect::uid_reference target;
    

    REFLECT2()
};

class Reflected_PointNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_PointNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);

    //virtual bool addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override {}

    REFLECT2()
};


class Reflected_TestNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_TestNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
        Reflected_Sprite_SceneNode(parent, geo_scene, smgr, id, pos) {}

    //virtual bool addSelfToScene(irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) {}
    bool bEnabled;
    int nParticles = 10;
    float velocity = 3.5;
    reflect::SomeOptions options;
    reflect::vector2 scale{ 1.0,1.0 };
    reflect::vector3 my_vec{ 5.0,4.0,3.0 };
    reflect::vector3 vec2{ 100.0,200.0,300.0 };
    reflect::color3 color;

    REFLECT2()
};


#endif
