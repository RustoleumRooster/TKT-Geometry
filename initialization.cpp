#include "initialization.h"
#include "NodeClassesTool.h"
#include "NodeInstancesTool.h"
#include "node_properties.h"
#include "material_groups.h"
#include "create_primitives.h"
#include "texture_adjust.h"
#include "lightmaps_tool.h"
#include "texture_picker.h"
#include "file_open.h"
#include "uv_tool.h"
#include "RenderTargetsTool.h"
#include "LightMaps.h"
#include "geometry_scene.h"
#include "ShaderCallbacks.h"
#include "my_nodes.h"
#include "edit_env.h"
#include "SceneTool.h"

extern IrrlichtDevice* device;
extern GUI_layout* gui_layout;

void initialize_tools(geometry_scene* scene, gui::IGUIEnvironment* gui, multi_tool_panel* tool_panel)
{
    Material_Groups_Base* material_groups_base = new Material_Groups_Base(L"Materials", GUI_ID_MATERIAL_GROUPS_BASE, gui, tool_panel);
    material_groups_base->set_scene(scene);
    Material_Groups_Tool::initialize(gui, material_groups_base, tool_panel);

    TexturePicker_Base* texture_picker_base = new TexturePicker_Base(L"Textures", GUI_ID_TEXTURES_BASE, gui, tool_panel);
    texture_picker_base->set_scene(scene);
    TexturePicker_Tool::initialize(texture_picker_base, tool_panel);

    GeometryFactory::initialize(gui, scene);
    Texture_Adjust_Tool::initialize(gui, scene);

    Node_Properties_Base* node_properties_base = new Node_Properties_Base(L"Properties", GUI_ID_NODE_PROPERTIES_BASE, gui, tool_panel);
    node_properties_base->set_scene(scene);
    NodeProperties_Tool::initialize(node_properties_base, tool_panel);

    //Geo_Settings_Base* geo_settings_base = new Geo_Settings_Base;
    //geo_settings_base->initialize(L"Settings", GUI_ID_GEO_SETTINGS_BASE, gui, &scene, tool_panel);
    //Geo_Settings_Tool::initialize(geo_settings_base, tool_panel);

    Material_Buffers_Base* material_buffers_base = new Material_Buffers_Base(L"Mesh Buffers", GUI_ID_MAT_BUFFERS_BASE, gui, tool_panel);
    material_buffers_base->set_scene(scene);
    material_buffers_base->initialize();
    Material_Buffers_Tool::initialize(material_buffers_base, tool_panel);

    /*
    std::cout << "\nReflection:\n";
    std::cout << Reflected_SceneNode_Factory::getNumTypes() << " reflected scene nodes registered:\n";
    for (int i = 0; i < Reflected_SceneNode_Factory::getNumTypes(); i++)
        std::cout << " " << Reflected_SceneNode_Factory::getAllTypes()[i]->alias << "\n";
        */

    Node_Classes_Base* node_classes_base = new Node_Classes_Base(L"All Node Classes", GUI_ID_NODE_CLASSES_BASE, gui, tool_panel);
    node_classes_base->set_scene(scene);
    Node_Classes_Tool::initialize(node_classes_base, tool_panel);

    Node_Instances_Base* node_instances_base = new Node_Instances_Base(L"All Node Instances", GUI_ID_NODE_INSTANCES_BASE, gui, tool_panel);
    node_instances_base->set_scene(scene);
    Node_Instances_Tool::initialize(node_instances_base, tool_panel);
    Node_Selector_Tool::initialize(node_instances_base);

    File_Open_Base* file_open_base = new File_Open_Base(L"All Node Instances", GUI_ID_FILE_OPEN_BASE, gui, tool_panel);
    file_open_base->set_scene(scene);
    File_Open_Tool::initialize(file_open_base, tool_panel);

    UV_Editor_Base* UV_Editor_base = new UV_Editor_Base(L"UV Edtior", GUI_ID_UV_EDITOR_BASE, gui, tool_panel);
    UV_Editor_base->set_scene(scene);
    UV_Editor_Tool::initialize(UV_Editor_base, tool_panel);

    Render_Tool_Base* render_tool_base = new Render_Tool_Base(L"Test Render", GUI_ID_UV_EDITOR_BASE, gui, tool_panel);
    render_tool_base->set_scene(scene);
    Render_Tool::initialize(render_tool_base, tool_panel);

    Lightmap_Manager* lightmap_manager = new Lightmap_Manager();
    Lightmaps_Tool::set_manager(lightmap_manager);

    Scene_Instances_Base* scene_instances_base = new Scene_Instances_Base(L"All Scenes", GUI_ID_SCENE_INSTANCES_BASE, gui, tool_panel);
    Scene_Instances_Tool::initialize(scene_instances_base, tool_panel);
}

void initialize_set_scene(geometry_scene* scene)
{
    Material_Groups_Tool::get_base()->set_scene(scene);
    TexturePicker_Tool::get_base()->set_scene(scene);
    GeometryFactory::set_scene(scene);
    Texture_Adjust_Tool::set_scene(scene);
    NodeProperties_Tool::get_base()->set_scene(scene);

    //Geo_Settings_Tool::initialize(geo_settings_base, tool_panel);

    Material_Buffers_Tool::get_base()->set_scene(scene);
    Node_Classes_Tool::get_base()->set_scene(scene);
    Node_Instances_Tool::get_base()->set_scene(scene);
    File_Open_Tool::get_base()->set_scene(scene);

    UV_Editor_Tool::get_base()->set_scene(scene);
    Render_Tool::get_base()->set_scene(scene);

    CameraQuad* cameraQuad = gui_layout->getCameraQuad();
    cameraQuad->set_scene(scene);
}

void initialize_materials(geometry_scene* scene)
{
    video::IVideoDriver* driver = device->getVideoDriver();
    video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();

    s32 materialType_dynamicLight = 0;
    s32 materialType_unlit_selected = 0;
    s32 materialType_lightmap = 0;
    s32 materialType_lightmap_selected = 0;
    s32 materialType_light_only = 0;
    s32 materialType_light_only_selected = 0;
    s32 materialType_reflection_unlit = 0;
    s32 materialType_projection = 0;
    s32 materialType_lightmap_clipped = 0;
    s32 materialType_underwater = 0;
    s32 materialType_water_surface = 0;
    s32 materialType_passthru = 0;

    if (gpu)
    {
        MyShaderCallBack* mc = new MyShaderCallBack();
        MyShaderCallBack2* mc2 = new MyShaderCallBack2();
        LightmapShaderCallback* mc3 = new LightmapShaderCallback();
        ProjectionShaderCallback* mc4 = new ProjectionShaderCallback();
        MyShaderCallBackTwoTextures* mc5 = new MyShaderCallBackTwoTextures();
        LightmapShaderCallbackClipped* mc6 = new LightmapShaderCallbackClipped();
        UnderwaterShaderCallback* mc7 = new UnderwaterShaderCallback();
        WaterSurfaceShaderCallback* mc8 = new WaterSurfaceShaderCallback();
        VanillaShaderCallBack* mc9 = new VanillaShaderCallBack();


        materialType_unlit_selected = gpu->addHighLevelShaderMaterialFromFiles(
            "vert_shader_unlit_selected.txt", "vertexMain", video::EVST_VS_1_1,
            "frag_shader_unlit_selected.txt", "pixelMain", video::EPST_PS_1_1,
            mc, video::EMT_SOLID, 0);

        materialType_dynamicLight = gpu->addHighLevelShaderMaterialFromFiles(
            "vert_shader_dynamicLight.txt", "vertexMain", video::EVST_VS_1_1,
            "frag_shader_dynamicLight.txt", "pixelMain", video::EPST_PS_1_1,
            mc2, video::EMT_SOLID, 0);

        materialType_lightmap = gpu->addHighLevelShaderMaterialFromFiles(
            "vert_shader_lightmap.txt", "vertexMain", video::EVST_VS_1_1,
            "frag_shader_lightmap.txt", "pixelMain", video::EPST_PS_1_1,
            mc3, video::EMT_SOLID, 0);

        materialType_lightmap_selected = gpu->addHighLevelShaderMaterialFromFiles(
            "vert_shader_lightmap.txt", "vertexMain", video::EVST_VS_1_1,
            "frag_shader_lightmap_selected.txt", "pixelMain", video::EPST_PS_1_1,
            mc3, video::EMT_SOLID, 0);

        materialType_light_only = gpu->addHighLevelShaderMaterialFromFiles(
            "vert_shader_lightmap.txt", "vertexMain", video::EVST_VS_1_1,
            "frag_shader_light_only.txt", "pixelMain", video::EPST_PS_1_1,
            mc3, video::EMT_SOLID, 0);

        materialType_light_only_selected = gpu->addHighLevelShaderMaterialFromFiles(
            "vert_shader_lightmap.txt", "vertexMain", video::EVST_VS_1_1,
            "frag_shader_light_only_selected.txt", "pixelMain", video::EPST_PS_1_1,
            mc3, video::EMT_SOLID, 0);

        materialType_reflection_unlit = gpu->addHighLevelShaderMaterialFromFiles(
            "vert_shader_tt.txt", "vertexMain", video::EVST_VS_1_1,
            "frag_shader_tt_unlit.txt", "pixelMain", video::EPST_PS_1_1,
            mc5, video::EMT_SOLID, 0);

        materialType_projection = gpu->addHighLevelShaderMaterialFromFiles(
            "vert_shader_pj.txt", "vertexMain", video::EVST_VS_1_1,
            "frag_shader_pj.txt", "pixelMain", video::EPST_PS_1_1,
            mc4, video::EMT_SOLID, 0);

        materialType_lightmap_clipped = gpu->addHighLevelShaderMaterialFromFiles(
            "vert_shader_lightmap_clipped.txt", "vertexMain", video::EVST_VS_1_1,
            "frag_shader_lightmap.txt", "pixelMain", video::EPST_PS_1_1,
            mc6, video::EMT_SOLID, 0);

        materialType_underwater = gpu->addHighLevelShaderMaterialFromFiles(
            "vert_shader_underwater.txt", "vertexMain", video::EVST_VS_1_1,
            "frag_shader_underwater.txt", "pixelMain", video::EPST_PS_1_1,
            mc7, video::EMT_SOLID, 0);

        materialType_water_surface = gpu->addHighLevelShaderMaterialFromFiles(
            "vert_shader_water_surface.txt", "vertexMain", video::EVST_VS_1_1,
            "frag_shader_water_surface.txt", "pixelMain", video::EPST_PS_1_1,
            mc8, video::EMT_SOLID, 0);

        materialType_passthru = gpu->addHighLevelShaderMaterialFromFiles(
            "shaders/blur_vertshader.txt", "vertexMain", video::EVST_VS_1_1,
            "shaders/passthru_fragshader.txt", "pixelMain", video::EPST_PS_1_1,
            mc9, video::EMT_SOLID, 0);
            

        mc->drop();
        mc2->drop();
        mc3->drop();
        mc4->drop();
        mc5->drop();
        mc6->drop();
        mc7->drop();
        mc8->drop();
        mc9->drop();
    }

    Material_Groups_Base* material_groups_base = Material_Groups_Tool::get_base();

    Material_Group m0{ "Default",false,false,true,false,video::SColor(255,128,64,64) };
    Material_Group m1{ "Unlit",false,false,false,false,video::SColor(255,128,64,64) };
    Material_Group m2{ "Lightmap",false,false,true,false,video::SColor(255,128,128,64) };
    Material_Group m3{ "Transparent",true,true,false,false,video::SColor(255,32,64,96) };
    Material_Group m4{ "Sky",false,false,false,true,video::SColor(255,128,128,64) };
    Material_Group m5{ "Water",false,false,false,false,video::SColor(255,128,128,64) };


    material_groups_base->LightingMaterial_Type = (video::E_MATERIAL_TYPE)materialType_lightmap;
    material_groups_base->LightingMaterial_Selected_Type = (video::E_MATERIAL_TYPE)materialType_lightmap_selected;
    material_groups_base->LightingOnlyMaterial_Type = (video::E_MATERIAL_TYPE)materialType_light_only;
    material_groups_base->LightingOnlyMaterial_Selected_Type = (video::E_MATERIAL_TYPE)materialType_light_only_selected;
    material_groups_base->SolidMaterial_Type = video::EMT_SOLID;
    material_groups_base->SolidMaterial_Selected_Type = (video::E_MATERIAL_TYPE)materialType_unlit_selected;
    material_groups_base->SolidMaterial_WaterSurface_Type = (video::E_MATERIAL_TYPE)materialType_water_surface;
    material_groups_base->Material_Projection_Type = (video::E_MATERIAL_TYPE)materialType_projection;

    material_groups_base->material_groups.push_back(m0);
    material_groups_base->material_groups.push_back(m1);
    material_groups_base->material_groups.push_back(m2);
    material_groups_base->material_groups.push_back(m3);
    material_groups_base->material_groups.push_back(m4);
    material_groups_base->material_groups.push_back(m5);

    Render_Tool_Base* render_tool_base = Render_Tool::get_base();
    render_tool_base->setMaterial(materialType_passthru);

    Reflected_SceneNode::SetBaseMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    Reflected_SceneNode::SetSpecialMaterialType((video::E_MATERIAL_TYPE)materialType_unlit_selected);

}

void initialize_camera_quad(geometry_scene* scene, GUI_layout* gui_layout, RenderList* renderList)
{
    CameraQuad* cameraQuad = gui_layout->getCameraQuad();

    cameraQuad->setRenderList(renderList);
    cameraQuad->initialize(scene);
    cameraQuad->setGridSnap(8);
    cameraQuad->setRotateSnap(7.5);

    Material_Buffers_Tool::get_base()->setCameraQuad(cameraQuad);
    UV_Editor_Tool::get_base()->setCameraQuad(cameraQuad);
    Render_Tool::get_base()->setCameraQuad(cameraQuad);

    ((TestPanel_3D*)cameraQuad->getPanel(0))->SetViewStyle(PANEL3D_VIEW_RENDER);

}

