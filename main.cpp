
#include <irrlicht.h>
#include <iostream>
#include "driverChoice.h"
#include "csg_classes.h"
#include "geometry_scene.h"
#include "CMeshSceneNode.h"
#include "edit_env.h"
#include "utils.h"
#include "CameraPanel.h"
#include "texture_picker.h"
#include "create_primitives.h"
#include "texture_adjust.h"
#include "node_properties.h"
#include "material_groups.h"
#include "Reflection.h"
#include "GUI_tools.h"
#include "LightMaps.h"

#include "geo_scene_settings.h"
#include "NodeClassesTool.h"
#include "NodeInstancesTool.h"
#include "file_open.h"
#include "uv_tool.h"
#include "ShaderCallbacks.h"

#include "clip_functions.h"
#include "tolerances.h"
#include <random>
#include <chrono>

using namespace irr;
using namespace scene;

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

core::matrix4 mView_;
core::matrix4 mProj_;

extern IrrlichtDevice* device = 0;
bool UseHighLevelShaders = true;
bool UseCgShaders = false;


extern geometry_scene* g_scene=NULL;

#define TIME_HEADER() auto startTime = std::chrono::high_resolution_clock::now();\
    auto timeZero = startTime;\
    auto currentTime = std::chrono::high_resolution_clock::now();\
    float time;
#define START_TIMER() startTime = std::chrono::high_resolution_clock::now(); \

#define PRINT_TIMER(text) currentTime = std::chrono::high_resolution_clock::now(); \
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); \
    std::cout << "---------time (" <<#text<< "): " << time << "\n";\


int main()
{
    TIME_HEADER()

	video::E_DRIVER_TYPE driverType=video::EDT_OPENGL;

	if (driverType==video::EDT_COUNT)
		return 1;

    MyEventReceiver receiver;

	device = createDevice(driverType,core::dimension2d<u32>(1200, 680),16,false,false,false,&receiver);

    
	if (device == 0)
		return 1; // could not create selected driver.

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	gui::IGUIEnvironment* gui = device->getGUIEnvironment();


    if (gui->getFileSystem()->addFileArchive("../media/", true, true, io::EFAT_FOLDER))
    {
        std::cout << "added ../media/ as directory archive\n";
    }
    else 
        std::cout << "failed to add ../media/ as directory archive\n";


    UseHighLevelShaders = true;
    UseCgShaders = false;

	io::path vsFileName; // filename for the vertex shader
	io::path psFileName; // filename for the pixel shader

	// create materials

	video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();
	s32 materialType_dynamicLight = 0;
	s32 materialType_unlit_selected = 0;
    s32 materialType_lightmap = 0;
    s32 materialType_lightmap_selected = 0;

	if (gpu)
	{
        MyShaderCallBack* mc = new MyShaderCallBack();
		MyShaderCallBack2* mc2 = new MyShaderCallBack2();
        LightmapShaderCallback* mc3 = new LightmapShaderCallback();

		if (UseHighLevelShaders)
		{
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

		}

		mc->drop();
		mc2->drop();
		mc3->drop();

	}

	bool bMouseDown=false;
    bool rMouseDown=false;
    bool letterDown[26];
    bool numberDown[10];
    s32 clickx;
    s32 clicky;

	//============================================================================================

    LineHolder graph;
    LineHolder graph2;
    LineHolder graph3;
    LineHolder graph4,graph5;
    LineHolder nograph;
    polyfold my_poly;

    Reflected_SceneNode::SetBaseMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    Reflected_SceneNode::SetSpecialMaterialType((video::E_MATERIAL_TYPE)materialType_unlit_selected);

    geometry_scene scene(smgr,driver,&receiver,video::EMT_SOLID,(video::E_MATERIAL_TYPE)materialType_unlit_selected);

    g_scene = &scene;

    core::matrix4 MAT;
    core::matrix4 MAT2;

    polyfold pf, pf1, pf2, pf3, pf4, cube;

    scene.set_type(GEO_SOLID);
    scene.rebuild_geometry();

    //==========
    Material_Groups_Base* material_groups_base = new Material_Groups_Base;
    material_groups_base->preinitialize(gui,&scene);

    Material_Group m0{"Unlit",false,false,false,false,video::SColor(255,128,64,64)};
    Material_Group m1{"Lightmap",false,false,true,false,video::SColor(255,128,128,64)};
    Material_Group m2{"Transparent",true,true,true,false,video::SColor(255,32,64,96)};
    Material_Group m3{"Sky",false,false,false,true,video::SColor(255,128,128,64)};

    
    material_groups_base->LightingMaterial_Type = (video::E_MATERIAL_TYPE)materialType_lightmap;
    material_groups_base->LightingMaterial_Selected_Type = (video::E_MATERIAL_TYPE)materialType_lightmap_selected;
    material_groups_base->SolidMaterial_Type = video::EMT_SOLID;
    material_groups_base->SolidMaterial_Selected_Type = (video::E_MATERIAL_TYPE)materialType_unlit_selected;

    material_groups_base->material_groups.push_back(m0);
    material_groups_base->material_groups.push_back(m1);
    material_groups_base->material_groups.push_back(m2);
    material_groups_base->material_groups.push_back(m3);

    scene.set_new_geometry_material(1); //Dynamic Light

    scene.setMaterialGroupsBase(material_groups_base);

    GUI_layout* gui_layout = new GUI_layout(driver, smgr, gui);

    gui_layout->initialize(core::rect<s32>(core::position2d<s32>(0, 0), core::dimension2d<u32>(1200, 680)));

    CameraQuad* cameraQuad = gui_layout->getCameraQuad();
    multi_tool_panel* tool_panel = gui_layout->getToolPanel();

    RenderList* renderList = new RenderList(driver);

    cameraQuad->setRenderList(renderList);

    CMeshSceneNode::initialize_unique_color(driver);

    cameraQuad->initialize(smgr, &scene);
    cameraQuad->setGridSnap(8);
    cameraQuad->setRotateSnap(7.5);
    //=========


    //=============================================================================================

	device->getCursorControl()->setVisible(true);

	int lastFPS = -1;

    ((TestPanel_3D*)cameraQuad->getPanel(0))->SetViewStyle(PANEL3D_VIEW_RENDER);

    //((TestPanel_3D*)cameraQuad->getPanel(0))->overrideMeshNode(mesh_node);
    //((TestPanel_3D*)cameraQuad->getPanel(0))->setTotalGeometry(&pf);
    //((TestPanel_3D*)cameraQuad->getPanel(0))->SetViewStyle(PANEL3D_VIEW_LOOPS);
    //((TestPanel_3D*)cameraQuad->getPanel(0))->AddGraphs(graph,graph2,graph3);

    material_groups_base->initialize(L"Materials", GUI_ID_MATERIAL_GROUPS_BASE, tool_panel);
    Material_Groups_Tool::initialize(gui,&scene,material_groups_base, tool_panel);

    TexturePicker_Base* texture_picker_base = new TexturePicker_Base;
    texture_picker_base->initialize(L"Textures", GUI_ID_TEXTURES_BASE, gui,&scene, tool_panel);
    TexturePicker_Tool::initialize(texture_picker_base, tool_panel);
    scene.setTexturePickerBase(texture_picker_base);

    GeometryFactory::initialize(gui,&scene);
    Texture_Adjust_Tool::initialize(gui,&scene);

    Node_Properties_Base* node_properties_base = new Node_Properties_Base;
    node_properties_base->initialize(L"Properties", GUI_ID_NODE_PROPERTIES_BASE, gui,&scene, tool_panel);
    NodeProperties_Tool::initialize(node_properties_base, tool_panel);

    ListReflectedNodes_Base* reflected_nodes_base = new ListReflectedNodes_Base;

    reflected_nodes_base->initialize(L"Classes",GUI_ID_NODE_CLASSES_BASE,gui,&scene, tool_panel);
    ListReflectedNodes_Tool::initialize(reflected_nodes_base, tool_panel);
    scene.set_choose_reflected_node_base(reflected_nodes_base);


    Geo_Settings_Base* geo_settings_base = new Geo_Settings_Base;
    geo_settings_base->initialize(L"Settings", GUI_ID_GEO_SETTINGS_BASE, gui, &scene, tool_panel);
    Geo_Settings_Tool::initialize(geo_settings_base, tool_panel);

    Material_Buffers_Base* material_buffers_base = new Material_Buffers_Base;
    material_buffers_base->initialize(L"Mesh Buffers", GUI_ID_MAT_BUFFERS_BASE, gui, &scene, tool_panel);
    Material_Buffers_Tool::initialize(material_buffers_base, tool_panel);

    std::cout << "\nReflection:\n";
    std::cout << Reflected_SceneNode_Factory::getNumTypes() << " reflected scene nodes registered:\n";
    for (int i = 0; i < Reflected_SceneNode_Factory::getNumTypes(); i++)
        std::cout << " " << Reflected_SceneNode_Factory::getAllTypes()[i]->alias << "\n";

    Node_Classes_Base* node_classes_base = new Node_Classes_Base;
    node_classes_base->initialize(L"All Node Classes", GUI_ID_NODE_CLASSES_BASE, gui, &scene, tool_panel);
    Node_Classes_Tool::initialize(node_classes_base, tool_panel);

    Node_Instances_Base* node_instances_base = new Node_Instances_Base;
    node_instances_base->initialize(L"All Node Instances", GUI_ID_NODE_INSTANCES_BASE, gui, &scene, tool_panel);
    Node_Instances_Tool::initialize(node_instances_base, tool_panel);

    File_Open_Base* file_open_base = new File_Open_Base;
    file_open_base->initialize(L"All Node Instances", GUI_ID_FILE_OPEN_BASE, gui, &scene, tool_panel);
    File_Open_Tool::initialize(file_open_base, tool_panel);

    UV_Editor_Base* uv_editor_base = new UV_Editor_Base;
    uv_editor_base->initialize(L"UV Edtior", GUI_ID_UV_EDITOR_BASE, gui, &scene, tool_panel);
    UV_Editor_Tool::initialize(uv_editor_base, tool_panel);


    //multi_tool::initialize(tool_panel);

    texture_picker_base->addTexture("terrain2.jpg");
    texture_picker_base->addTexture("wall.bmp");
    texture_picker_base->addTexture("water3.jpg");
    texture_picker_base->addTexture("../media/textures/greek_cornice.jpg","AI");
    texture_picker_base->addTexture("../media/textures/temple_wall.jpg","AI");
    texture_picker_base->addTexture("../media/textures/temple_floor.jpg","AI");
    texture_picker_base->addTexture("../media/textures/carved_stone_buddha.jpg","AI");
    texture_picker_base->addTexture("../media/textures/ganesha_stone0.jpg","AI");
    texture_picker_base->addTexture("../media/textures/ganesha_stone1.jpg","AI");
    texture_picker_base->addTexture("../media/textures/greek_cornice.jpg","AI");
    texture_picker_base->addTexture("../media/textures/kama_sutra.jpg","AI");

    for(int i=0;i<10;i++)
    {
    texture_picker_base->addTexture("terrain2.jpg");
    texture_picker_base->addTexture("wall.bmp");
    texture_picker_base->addTexture("water3.jpg");
    }

    Lightmap_Manager* lightmap_manager = new Lightmap_Manager;
    scene.setLightmapManager(lightmap_manager);

    MakeCircleImages(driver);

    bool s_down=false;

    core::rect<s32> windowsize_0 = driver->getViewPort();

	while(device->run())
		if (device->isWindowActive())
	{
            
        core::rect<s32> windowsize = driver->getViewPort();
        if(windowsize != windowsize_0)
        {
           // std::cout << "viewport size is " << windowsize.getWidth() << " by " << windowsize.getHeight() << "\n";
            windowsize_0 = windowsize;
            gui_layout->resize(windowsize);
        }

		driver->beginScene(true, true, video::SColor(255,32,32,32));

        renderList->renderAll();

		//cameraQuad->render();
        gui->drawAll();

		driver->endScene();

		int fps = driver->getFPS();

		if (lastFPS != fps)
		{
			core::stringw str = L"Das Irrlicht Engine  [";
			str += driver->getName();
			str += "] FPS:";
			str += fps;

			device->setWindowCaption(str.c_str());
			lastFPS = fps;
		}
	}

	device->drop();

	return 0;
}
