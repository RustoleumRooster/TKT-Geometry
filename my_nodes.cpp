#include <irrlicht.h>
#include "my_nodes.h"
#include "USceneNode.h"
#include "TwoTrianglesNode.h"
#include "geometry_scene.h"
#include "CMeshSceneNode.h"
#include "material_groups.h"

using namespace irr;
using namespace video;
using namespace core;

extern IrrlichtDevice* device;
extern f32 global_clipping_plane[4];
extern SceneCoordinator* gs_coordinator;

extern float g_time;

bool IsWaterReflectionPass = false;


//============================================================
// Water Surface Scene Node
//

WaterSurface_SceneNode::WaterSurface_SceneNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos, const core::vector3df rotation, geometry_scene* geo_scene)
    :ISceneNode(parent, smgr, id, pos, rotation, vector3df(1.0, 1.0, 1.0)), geo_scene(geo_scene)
{
    my_camera = smgr->addCameraSceneNode(NULL, vector3df(0, 0, 0), vector3df(0, 0, 0), -1, false);

    ((MyEventReceiver*)device->getEventReceiver())->Register_ViewResize(this);

    this->setAutomaticCulling(scene::EAC_OFF);

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

    my_camera->remove();
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

    for (IMeshBuffer* buffer : buffers)
    {
        buffer->getMaterial().setTexture(0, getRTT());
        buffer->getMaterial().setTexture(1, getRTT2());
    }

    geo_scene->getMeshNode()->copyMaterials();
}

void getReflectedCamera(ICameraSceneNode* camera, f32 z, const ICameraSceneNode* src)
{
    vector3df cam_pos = src->getAbsolutePosition();
    vector3df cam_target = src->getTarget();

    if (cam_pos.Y > 0)
    {
        f32 y_pos = -cam_pos.Y;
        camera->setPosition(vector3df(cam_pos.X, y_pos, cam_pos.Z));
        camera->setTarget(vector3df(cam_target.X, -cam_target.Y, cam_target.Z));
    }
}

void WaterSurface_SceneNode::render()
{
    if (this->isVisible() == false || geo_scene->getMeshNode()->isVisible() == false)
        return;


    ICameraSceneNode* fp_camera = SceneManager->getActiveCamera();

    IVideoDriver* driver = SceneManager->getVideoDriver();

    getReflectedCamera(my_camera, 0, fp_camera);

    for (IMeshBuffer* buffer : buffers)
    {
        buffer->getMaterial().FrontfaceCulling = true;
        buffer->getMaterial().BackfaceCulling = true;
    }
    geo_scene->getMeshNode()->copyMaterials();

    //=================
    //Render from the reflection camera (scene above water)

    ITexture* rtt = my_rtt; // get_rtt();

    driver->setRenderTarget(rtt, true, true, video::SColor(255, 16, 16, 16));

    global_clipping_plane[0] = 0.0;
    global_clipping_plane[1] = 1.0;
    global_clipping_plane[2] = 0.0;
    global_clipping_plane[3] = 0.0;

    driver->enableClipPlane(0, true);


    SceneManager->setActiveCamera(my_camera);

    my_camera->render();

    //std::cout << "reflection pass\n";
    this->setVisible(false);
    IsWaterReflectionPass = true;

    SceneManager->clearAllRegisteredNodesForRendering();
    SceneManager->drawAll();

    this->setVisible(true);
    IsWaterReflectionPass = false;
    //geo_scene->getMeshNode()->render();


    //=================
    //Render from the normal camera (scene underwater)

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

    // std::cout << "underwatere pass\n";

    geo_scene->getMeshNode()->render();

    // std::cout << "------\n";

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

    for (IMeshBuffer* buffer : buffers)
    {
        buffer->getMaterial().FrontfaceCulling = false;
        buffer->getMaterial().BackfaceCulling = true;
    }
    geo_scene->getMeshNode()->copyMaterials();
}


void WaterSurface_SceneNode::OnRegisterSceneNode()
{

    if (IsVisible)
        SceneManager->registerNodeForRendering(this, ESNRP_TRANSPARENT_EFFECT);
    //SceneManager->registerNodeForRendering(this, ESNRP_SKY_BOX);

    ISceneNode::OnRegisterSceneNode();
}

void WaterSurface_SceneNode::attach_to_buffer(IMeshBuffer* buffer)
{
    buffers.push_back(buffer);

    E_MATERIAL_TYPE water_material = Material_Groups_Tool::get_base()->SolidMaterial_WaterSurface_Type;

    buffer->getMaterial().MaterialType = water_material;

    buffer->getMaterial().setTexture(0, getRTT());
    buffer->getMaterial().setTexture(1, getRTT2());

    my_box.addInternalBox(buffer->getBoundingBox());
}


//============================================================
// Skybox Scene Node
//

//s32 MySkybox_SceneNode::skyShaderType = 0;
s32 MySkybox_SceneNode::xblurShaderType = 0;
s32 MySkybox_SceneNode::yblurShaderType = 0;
//s32 MySkybox_SceneNode::skyPPShaderType = 0;
//s32 MySkybox_SceneNode::planeMaterialType = 0;
s32 MySkybox_SceneNode::projectionShaderType = 0;

extern float g_time;

void PlaneShaderCallBack::OnSetConstants(video::IMaterialRendererServices* services,
    s32 userData)
{
    video::IVideoDriver* driver = services->getVideoDriver();


    //Projection and WorldView matrices

    core::matrix4 Proj = driver->getTransform(video::ETS_PROJECTION);

    services->setPixelShaderConstant("mProj", Proj.pointer(), 16);

    core::matrix4 MV = driver->getTransform(video::ETS_VIEW);


    core::matrix4 World = driver->getTransform(video::ETS_WORLD);
    MV *= World;

    services->setPixelShaderConstant("mWorldView", MV.pointer(), 16);


    core::matrix4 iMV = MV;
    iMV.makeInverse();

    //Inverse ModelView and Inverse Projection matrices

    core::matrix4 iproj = driver->getTransform(video::ETS_PROJECTION);
    iproj.makeInverse();

    services->setVertexShaderConstant("mInvProj", iproj.pointer(), 16);

    services->setVertexShaderConstant("iMV", iMV.pointer(), 16);
    services->setPixelShaderConstant("iMV", iMV.pointer(), 16);

    core::vector3df pos = device->getSceneManager()->getActiveCamera()->getAbsolutePosition();

    services->setVertexShaderConstant("mCamPos", reinterpret_cast<f32*>(&pos), 3);


    // set light color

    video::SColorf col(0.0f, 0.0f, 0.0f, 1.0f);

    services->setPixelShaderConstant("Color", reinterpret_cast<f32*>(&col), 4);

    services->setPixelShaderConstant("Time", reinterpret_cast<f32*>(&g_time), 1);

    s32 TextureLayerID;

    TextureLayerID = 0;

    services->setPixelShaderConstant("myTexture", &TextureLayerID, 1);

    TextureLayerID = 2;

    services->setPixelShaderConstant("myTexture2", &TextureLayerID, 1);

    TextureLayerID = 3;

    services->setPixelShaderConstant("myTexture3", &TextureLayerID, 1);

}

void VanillaShaderCallBack::OnSetConstants(video::IMaterialRendererServices* services,
    s32 userData)
{
    video::IVideoDriver* driver = services->getVideoDriver();


    //Projection and WorldView matrices

    core::matrix4 Proj = driver->getTransform(video::ETS_PROJECTION);

    services->setPixelShaderConstant("mProj", Proj.pointer(), 16);

    core::matrix4 MV = driver->getTransform(video::ETS_VIEW);
    core::matrix4 World = driver->getTransform(video::ETS_WORLD);
    MV *= World;

    services->setPixelShaderConstant("mWorldView", MV.pointer(), 16);


    core::matrix4 iMV = MV;
    iMV.makeInverse();

    //Inverse ModelView and Inverse Projection matrices

    core::matrix4 iproj = driver->getTransform(video::ETS_PROJECTION);
    iproj.makeInverse();

    services->setVertexShaderConstant("mInvProj", iproj.pointer(), 16);

    services->setVertexShaderConstant("iMV", iMV.pointer(), 16);
    services->setPixelShaderConstant("iMV", iMV.pointer(), 16);



    core::vector3df pos = device->getSceneManager()->getActiveCamera()->getAbsolutePosition();

    services->setVertexShaderConstant("mCamPos", reinterpret_cast<f32*>(&pos), 3);

    services->setPixelShaderConstant("Time", reinterpret_cast<f32*>(&g_time), 1);
}

void myProjectionShaderCallback::OnSetConstants(video::IMaterialRendererServices* services,
    s32 userData)
{
    video::IVideoDriver* driver = services->getVideoDriver();

    core::matrix4 worldView = driver->getTransform(video::ETS_VIEW);
    worldView *= driver->getTransform(video::ETS_WORLD);
    services->setVertexShaderConstant("mWorldView", worldView.pointer(), 16);

    core::matrix4 Proj = driver->getTransform(video::ETS_PROJECTION);
    services->setVertexShaderConstant("mProj", Proj.pointer(), 16);

    s32 TextureLayerID;

    if (IsWaterReflectionPass)
        TextureLayerID = 1;
    else
        TextureLayerID = 0;

    services->setPixelShaderConstant("myTexture2", &TextureLayerID, 1);

}

void SkyShaderCallBack::OnSetConstants(video::IMaterialRendererServices* services,
    s32 userData)
{
    video::IVideoDriver* driver = services->getVideoDriver();


    //Projection and WorldView matrices

    core::matrix4 Proj = driver->getTransform(video::ETS_PROJECTION);

    services->setPixelShaderConstant("P", Proj.pointer(), 16);

    core::matrix4 MV = driver->getTransform(video::ETS_VIEW);
    core::matrix4 World = driver->getTransform(video::ETS_WORLD);
    MV *= World;

    services->setPixelShaderConstant("MV", MV.pointer(), 16);

    core::vector3df pos = device->getSceneManager()->getActiveCamera()->getAbsolutePosition();

    services->setVertexShaderConstant("mCamPos", reinterpret_cast<f32*>(&pos), 3);

    services->setPixelShaderConstant("Time", reinterpret_cast<f32*>(&g_time), 1);

    s32 TextureLayerID = 0;

    services->setPixelShaderConstant("myTexture", &TextureLayerID, 1);

}

MySkybox_SceneNode::MySkybox_SceneNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos, const core::vector3df rotation, geometry_scene* geo_scene)
    :ISceneNode(parent, smgr, id, pos, rotation, vector3df(1.0, 1.0, 1.0)), geo_scene(geo_scene)
{
    //cloudLayerNode = new TwoTriangleSceneNode(NULL, smgr, 23456);

    my_camera = smgr->addCameraSceneNode(NULL, pos, vector3df(0, 0, 0), -1, false);
    //reflection_camera = smgr->addCameraSceneNode(NULL, pos, vector3df(0, 0, 0), -1, false);

    IVideoDriver* driver = device->getVideoDriver();
    /*
    cloudLayerNode->getMaterial(0).MaterialType = (video::E_MATERIAL_TYPE)planeMaterialType;
    cloudLayerNode->setMaterialTexture(0, driver->getTexture("new_pnoise.bmp"));
    cloudLayerNode->setMaterialTexture(1, driver->getTexture("new_pnoise2.bmp"));
    cloudLayerNode->setMaterialTexture(2, driver->getTexture("new_pnoise3.bmp"));
    cloudLayerNode->SetVisible(false);
    */

    blurNode = new TwoTriangleSceneNode(NULL, smgr, 66421);
    blurNode->getMaterial(0).MaterialType = (video::E_MATERIAL_TYPE)yblurShaderType;
    //blurNode->setMaterialTexture(0, rt2);
    blurNode->setVisible(false);

    //my_camera = smgr->addCameraSceneNode(NULL, vector3df(0, 0, 0), vector3df(0, 0, 0), -1, false);

    ((MyEventReceiver*)device->getEventReceiver())->Register_ViewResize(this);

    this->setAutomaticCulling(EAC_OFF);

}

MySkybox_SceneNode::~MySkybox_SceneNode()
{
    ((MyEventReceiver*)device->getEventReceiver())->UnRegister_ViewResize(this);


    //if (cloudLayerNode)
    //    delete cloudLayerNode;

    if (blurNode)
        delete blurNode;

    my_camera->remove();
    my_camera = NULL;

    //reflection_camera->remove();
    //my_camera = NULL;

}

void MySkybox_SceneNode::resizeView(core::dimension2du new_size)
{
    my_camera->setAspectRatio((f32)new_size.Width / (f32)new_size.Height);
    my_camera->updateAbsolutePosition();

    // reflection_camera->setAspectRatio((f32)new_size.Width / (f32)new_size.Height);
    // reflection_camera->updateAbsolutePosition();

    IVideoDriver* driver = SceneManager->getVideoDriver();

    if (my_rtt)
    {
        driver->removeTexture(my_rtt);
        my_rtt = NULL;
    }

    //my_rtt = driver->addRenderTargetTexture(new_size, "RTT_00", irr::video::ECF_A8R8G8B8);

    core::dimension2d<u32> half_size(new_size.Width * 0.25, new_size.Height * 0.25);

    if (my_rtt2)
    {
        driver->removeTexture(my_rtt2);
        my_rtt2 = NULL;
    }

    if (my_rtt3)
    {
        driver->removeTexture(my_rtt3);
        my_rtt3 = NULL;
    }

    my_rtt2 = driver->addRenderTargetTexture(new_size, "RTT_02", irr::video::ECF_A8R8G8B8);
    // my_rtt3 = driver->addRenderTargetTexture(half_size, "RTT_03", irr::video::ECF_A8R8G8B8);

     //Render_Tool::connect_image(this);

    for (IMeshBuffer* buffer : buffers)
    {
        if (this->WaterReflectionPass)
            buffer->getMaterial().setTexture(1, my_rtt2);
        else
            buffer->getMaterial().setTexture(0, my_rtt2);
    }

    gs_coordinator->CopyAllMaterials();
    //geo_scene->getMeshNode()->copyMaterials();
}

void getReflectedCameraAngleOnly(ICameraSceneNode* camera, f32 z, const ICameraSceneNode* src)
{
    vector3df cam_pos = src->getAbsolutePosition();
    vector3df cam_target = src->getTarget();

    if (cam_pos.Y > 0)
    {
        f32 y_pos = -cam_pos.Y;
        camera->setPosition(vector3df(cam_pos.X, y_pos, cam_pos.Z));
        camera->setTarget(vector3df(cam_target.X, -cam_target.Y, cam_target.Z));
    }
}

void MySkybox_SceneNode::render()
{
    if (geo_scene->getMeshNode()->isVisible() == false || this->WaterReflectionPass != IsWaterReflectionPass)
        return;

    int a;

    if (this->WaterReflectionPass)
    {
        a = 0;
        //std::cout << "water reflection sky\n";
    }
    else
    {
        a = 1;
        //std::cout << "sky\n";
    }


    ICameraSceneNode* fp_camera = gs_coordinator->current_smgr()->getActiveCamera();

    //vector3df fp_pos = fp_camera->getAbsolutePosition();
    //fp_pos *= 0.025;

    //my_camera->setPosition(fp_pos);
    //my_camera->updateAbsolutePosition();

    vector3df t_pos = my_camera->getAbsolutePosition() + fp_camera->getTarget() - fp_camera->getAbsolutePosition();


    //my_camera->setRotation(fp_camera->getRotation());
    my_camera->setTarget(t_pos);
    my_camera->render();

    IVideoDriver* driver = SceneManager->getVideoDriver();

    driver->setRenderTarget(my_rtt2, true, true, video::SColor(255, 0, 0, 0));

    geo_scene->getMeshNode()->render();

    /*
    //1) down sample to my_rtt2s
    driver->setRenderTarget(my_rtt2, true, true, video::SColor(255, 0, 0, 0));
    cloudLayerNode->render();

    //2) Y BLUR to my_rtt3
    blurNode->getMaterial(0).MaterialType = (video::E_MATERIAL_TYPE)yblurShaderType;
    blurNode->setMaterialTexture(0, my_rtt2);

    driver->setRenderTarget(my_rtt3, true, true, video::SColor(255, 0, 0, 0));
    blurNode->render();

    //3) X BLUR to my_rtt2
    blurNode->getMaterial(0).MaterialType = (video::E_MATERIAL_TYPE)xblurShaderType;
    blurNode->setMaterialTexture(0, my_rtt3);

    driver->setRenderTarget(my_rtt2, true, true, video::SColor(255, 0, 0, 0));
    blurNode->render();
    */

    driver->setRenderTarget(NULL, true, true, video::SColor(0, 0, 0, 0));
}

void MySkybox_SceneNode::OnRegisterSceneNode()
{
    if (IsVisible)
        SceneManager->registerNodeForRendering(this, ESNRP_TRANSPARENT_EFFECT);
    //SceneManager->registerNodeForRendering(this, ESNRP_SKY_BOX);

    ISceneNode::OnRegisterSceneNode();
}

void MySkybox_SceneNode::attach_to_buffer(IMeshBuffer* buffer)
{
    buffers.push_back(buffer);

    //E_MATERIAL_TYPE projection_material = Material_Groups_Tool::get_base()->Material_Projection_Type;
    E_MATERIAL_TYPE projection_material = (E_MATERIAL_TYPE)projectionShaderType;

    buffer->getMaterial().MaterialType = projection_material;

    if (this->WaterReflectionPass)
        buffer->getMaterial().setTexture(1, my_rtt2);
    else
        buffer->getMaterial().setTexture(0, my_rtt2);

    my_box.addInternalBox(buffer->getBoundingBox());

    //copyMaterials is called from the calling function

}

void MySkybox_SceneNode::load_shaders()
{
    SkyShaderCallBack* mc5;
    VanillaShaderCallBack* mc6;
    VanillaShaderCallBack* mc7;
    myProjectionShaderCallback* mc8;

    PlaneShaderCallBack* mc2 = new PlaneShaderCallBack();
    mc5 = new SkyShaderCallBack();
    mc6 = new VanillaShaderCallBack();
    mc7 = new VanillaShaderCallBack();
    mc8 = new myProjectionShaderCallback();

    IVideoDriver* driver = device->getVideoDriver();
    video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();

    /*
    planeMaterialType = gpu->addHighLevelShaderMaterialFromFiles(
        "shaders/pl_vertshader.txt", "vertexMain", video::EVST_VS_1_1,
        "shaders/pl_fragshader.txt", "pixelMain", video::EPST_PS_1_1,
        mc2, video::EMT_SOLID, 0);

    skyShaderType = gpu->addHighLevelShaderMaterialFromFiles(
        "shaders/skydome_vertshader.txt", "vertexMain", video::EVST_VS_1_1,
        "shaders/skydome_fragshader.txt", "pixelMain", video::EPST_PS_1_1,
        mc5, video::EMT_SOLID, 0);
        */

    xblurShaderType = gpu->addHighLevelShaderMaterialFromFiles(
        "shaders/blur_vertshader.txt", "vertexMain", video::EVST_VS_1_1,
        "shaders/xblur_fragshader.txt", "pixelMain", video::EPST_PS_1_1,
        mc6, video::EMT_SOLID, 0);

    yblurShaderType = gpu->addHighLevelShaderMaterialFromFiles(
        "shaders/blur_vertshader.txt", "vertexMain", video::EVST_VS_1_1,
        "shaders/yblur_fragshader.txt", "pixelMain", video::EPST_PS_1_1,
        mc7, video::EMT_SOLID, 0);

    projectionShaderType = gpu->addHighLevelShaderMaterialFromFiles(
        "vert_shader_pj.txt", "vertexMain", video::EVST_VS_1_1,
        "frag_shader_pj.txt", "pixelMain", video::EPST_PS_1_1,
        mc8, video::EMT_SOLID, 0);

    /*
    skyPPShaderType = gpu->addHighLevelShaderMaterialFromFiles(
        "shaders/blur_vertshader.txt", "vertexMain", video::EVST_VS_1_1,
        "shaders/skypp_fragshader.txt", "pixelMain", video::EPST_PS_1_1,
        mc8, video::EMT_SOLID, 0);
        */

    mc5->drop();
    mc6->drop();
    mc7->drop();
    mc8->drop();
}