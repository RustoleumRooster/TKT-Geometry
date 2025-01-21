#ifndef _TKT_MY_NODES_
#define _TKT_MY_NODES_

#include <irrlicht.h>
#include "USceneNode.h"
#include "TwoTrianglesNode.h"
#include "edit_env.h"
#include <vector>

using namespace irr;

class geometry_scene;

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

class MySkybox_SceneNode : public scene::ISceneNode, public ViewResizeObject
{
public:
    MySkybox_SceneNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos, const core::vector3df rotation, geometry_scene*);
    ~MySkybox_SceneNode();

    virtual void resizeView(core::dimension2du newsize) override;

    virtual void render() override;
    virtual void OnRegisterSceneNode() override;
    virtual const core::aabbox3d<f32>& getBoundingBox() const override { return my_box; }

    virtual int get_image_count() override { return 1; }
    virtual video::ITexture* get_image(int n) { return my_rtt2; };

    void attach_to_buffer(scene::IMeshBuffer*);

    static void load_shaders();

    video::ITexture* getRTT() { return my_rtt2; }

    core::aabbox3d<f32> my_box;

    static s32 xblurShaderType;
    static s32 yblurShaderType;
    static s32 projectionShaderType;

    geometry_scene* geo_scene = NULL;

    //final texture
    video::ITexture* my_rtt = NULL;

    //two blur textures
    video::ITexture* my_rtt2 = NULL;
    video::ITexture* my_rtt3 = NULL;

    bool WaterReflectionPass = false;

    TwoTriangleSceneNode* blurNode = NULL;

    scene::ICameraSceneNode* my_camera = NULL;

    std::vector<scene::IMeshBuffer*> buffers;
};

class WaterSurface_SceneNode : public scene::ISceneNode, public ViewResizeObject
{
public:
    WaterSurface_SceneNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos, const core::vector3df rotation, geometry_scene*);
    ~WaterSurface_SceneNode();

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

    void attach_to_buffer(scene::IMeshBuffer*);
    void set_z_depth(f32 z) { my_z_depth = z; }

    void getReflectedCamera(scene::ICameraSceneNode* camera, f32 z, const scene::ICameraSceneNode* src);

    void setSkybox(MySkybox_SceneNode* sky) { my_skybox = sky; }

private:
    core::aabbox3d<f32> my_box;

    video::E_MATERIAL_TYPE render_material = video::EMT_SOLID;
    video::E_MATERIAL_TYPE underwater_material = video::EMT_SOLID;
    video::ITexture* my_rtt = NULL;
    video::ITexture* my_rtt2 = NULL;
    scene::ICameraSceneNode* my_camera = NULL;
    MySkybox_SceneNode* my_skybox = NULL;


    geometry_scene* geo_scene = NULL;

    std::vector<int> hide_faces;
    std::vector<scene::IMeshBuffer*> buffers;

    f32 my_z_depth = 0.0;
};

//=================================================================================================
// RenderSky_SceneNode: Reflected_SkyNode creates this node in each scene that uses the skybox. Its 
// render() function calls Reflected_SkyNode to render()
//

class RenderSky_SceneNode : public scene::ISceneNode
{
public:
    RenderSky_SceneNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos, const core::vector3df rotation, geometry_scene*);
    ~RenderSky_SceneNode();

    virtual void render() override;
    virtual const core::aabbox3d<f32>& getBoundingBox() const override { return my_box; }
    virtual void OnRegisterSceneNode() override;

    void set_skynode(MySkybox_SceneNode* sky) { my_skybox = sky; }

private:
    core::aabbox3d<f32> my_box;

    MySkybox_SceneNode* my_skybox = NULL;
};



#endif
