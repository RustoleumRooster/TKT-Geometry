
#include <irrlicht.h>
#include <iostream>

#include "csg_classes.h"
#include "geometry_scene.h"
#include "CMeshSceneNode.h"
#include "edit_env.h"
#include "utils.h"
#include "CameraPanel.h"
#include "texture_picker.h"
#include "create_primitives.h"

#include "material_groups.h"
#include "Reflection.h"
#include "GUI_tools.h"
#include "LightMaps.h"
#include "geo_scene_settings.h"
#include "my_reflected_nodes.h"
#include "initialization.h"
#include "polyfold_soa.h"
#include "my_nodes.h"

#include <random>
#include <chrono>

using namespace irr;
using namespace scene;
using namespace std;

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

extern IrrlichtDevice* device = 0;
extern f32 global_clipping_plane[4] = { 1.0,1.0,0.0,0.0 };

extern SceneCoordinator* gs_coordinator = NULL;
extern GUI_layout* gui_layout = NULL;
extern float g_time = 0;

struct TestStruct
{
    reflect::pointer<geometry_scene> scene_ptr{ NULL };

    REFLECT()
};


int main()
{

    video::E_DRIVER_TYPE driverType = video::EDT_OPENGL;

    if (driverType == video::EDT_COUNT)
        return 1;

    MyEventReceiver receiver;

    device = createDevice(driverType, core::dimension2d<u32>(1200, 680), 16, false, false, false, &receiver);


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


    bool bMouseDown = false;
    bool rMouseDown = false;
    bool letterDown[26];
    bool numberDown[10];
    s32 clickx;
    s32 clicky;

    //============================================================================================

    LineHolder graph;
    LineHolder graph2;
    LineHolder graph3;
    LineHolder graph4, graph5;
    LineHolder nograph;
    polyfold my_poly;

    /*
    geometry_scene* scene0 = new geometry_scene();
    scene0->initialize(smgr, driver, &receiver);
    scene0->set_type(GEO_SOLID);
    scene0->geoNode()->rebuild_geometry();
    */

    SceneCoordinator coord(smgr, driver, &receiver);

    geometry_scene& scene = *coord.current_scene();
    scene.InitializeEmptyScene();

    gs_coordinator = &coord;

    gui_layout = new GUI_layout(driver, gui);

    gui_layout->initialize(core::rect<s32>(core::position2d<s32>(0, 0), core::dimension2d<u32>(1200, 680)));

    multi_tool_panel* tool_panel = gui_layout->getToolPanel();

    initialize_tools(&scene, gui, tool_panel);

    initialize_materials(&scene);

    CMeshSceneNode::initialize_unique_color(driver);

    
    //=============================================================================================

	device->getCursorControl()->setVisible(true);

	int lastFPS = -1;


    MySkybox_SceneNode::load_shaders();

    RenderList* renderList = new RenderList(driver);
    initialize_camera_quad(&scene, gui_layout, renderList);

    TexturePicker_Base* texture_picker_base = TexturePicker_Tool::get_base();
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

    MakeCircleImages(driver);

    bool s_down=false;

    core::rect<s32> windowsize_0 = driver->getViewPort();

    cout << "Loading Project\n";
    Open_Geometry_File::LoadProject(gs_coordinator, "../projects/test01");

    //===============================================================

    /*
    clip_results results;

    pf = scene.elements[0].brush;
    pf.make_convex();

    pf2 = scene.elements[1].geometry;

    clip_poly_accelerated2(pf, pf2, GEO_ADD, GEO_SOLID, results, nograph);

     addDrawLines(pf, graph, graph2, graph4);

     ((TestPanel_3D*)cameraQuad->getPanel(0))->AddGraphs(graph, graph2, graph4);
     */

     CameraQuad* cameraQuad = gui_layout->getCameraQuad();

     //((TestPanel_3D*)cameraQuad->getPanel(0))->AddGraphs(graph, graph2, graph4);
     
     
    //================================================================

	while(device->run())
		if (device->isWindowActive())
	{
        const u32 now = device->getTimer()->getTime();
        g_time = double(now) / 1000;
         
        core::rect<s32> windowsize = driver->getViewPort();
        if(windowsize != windowsize_0)
        {
            windowsize_0 = windowsize;
            gui_layout->resize(windowsize);
        }

		driver->beginScene(true, true, video::SColor(255,32,32,32));

        renderList->renderAll();

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
