
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
#include "fonts.h"

#include <random>
#include <chrono>

#include "clip_functions.h"

#include <Windows.h>
#include <WinUser.h>
#include <WinDef.h>

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

int font_callback(const LOGFONT* lpelf, const TEXTMETRIC* lptnm, DWORD fonttype, LPARAM lParam)
{
    cout << lpelf->lfFaceName << "\n";
    return 1;
}

int main()
{
    video::E_DRIVER_TYPE driverType = video::EDT_OPENGL;

    if (driverType == video::EDT_COUNT)
        return 1;

    MyEventReceiver receiver;

    //device = createDevice(driverType, core::dimension2d<u32>(1200, 680), 16, false, false, false, &receiver);
    device = createDevice(driverType, core::dimension2d<u32>(1500, 850), 16, false, false, false, &receiver);

    video::IVideoDriver* driver = device->getVideoDriver();
    scene::ISceneManager* smgr = device->getSceneManager();
    gui::IGUIEnvironment* gui = device->getGUIEnvironment();

    bool dpiAware = true;
    if(dpiAware)
    {
        DPI_AWARENESS_CONTEXT dpi_context = DPI_AWARENESS_CONTEXT_SYSTEM_AWARE;
        SetProcessDpiAwarenessContext(dpi_context);

        video::SExposedVideoData videodata = driver->getExposedVideoData();
        RECT window_rect;
        GetClientRect((HWND)videodata.OpenGLWin32.HWnd, &window_rect);
        u32 dpi = GetDpiForWindow((HWND)videodata.OpenGLWin32.HWnd);

        cout << "dpi: " << dpi << "\n";
    }

    if (device == 0)
        return 1; // could not create selected driver.

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

    bool enumerateFonts = false;
    if(enumerateFonts)
    {
        LOGFONTA logfont;
        logfont.lfCharSet = DEFAULT_CHARSET;
        logfont.lfFaceName[0] = '\0';

        cout << "enumerating fonts:\n";
        HDC dc = ::GetDC(0);
        EnumFontFamiliesExA(dc, &logfont, &font_callback, NULL, 0);
    }

    //==============================================================
    //Choose a font and render it
    //

    AGG_TT_Font_Renderer font_render;

    AGG_TT_Font* agg_font = font_render.Render_Font(gui, "Calibri", 22, 1.0, -1, 2, dimension2du{ 256,256 });


    //==============================================================
    //Initialize GUI elements
    //

    gui_layout->initialize(core::rect<s32>(core::position2d<s32>(0, 0), core::dimension2d<u32>(1200, 680)), agg_font);
    //gui_layout->initialize(core::rect<s32>(core::position2d<s32>(0, 0), core::dimension2d<u32>(1200, 680)), NULL);



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
    texture_picker_base->addTexture("terrain2.jpg", "");
    texture_picker_base->addTexture("wall.bmp", "");
    texture_picker_base->addTexture("water3.jpg", "");
    texture_picker_base->addTexture("simple_light.png", "");
    texture_picker_base->addTexture("../media/textures/greek_cornice.jpg","AI");
    texture_picker_base->addTexture("../media/textures/temple_wall.jpg", "AI")
        .set_height_map("../media/textures/temple_wall_height.jpg")
        .set_normal_map("../media/textures/temple_wall_normal.jpg");
    texture_picker_base->addTexture("../media/textures/temple_floor.jpg","AI")
        .set_height_map("../media/textures/temple_floor_height.jpg")
        .set_normal_map("../media/textures/temple_floor_normal.jpg");
    texture_picker_base->addTexture("../media/textures/carved_stone_buddha.jpg","AI");
    texture_picker_base->addTexture("../media/textures/ganesha_stone0.jpg","AI");
    texture_picker_base->addTexture("../media/textures/ganesha_stone1.jpg","AI");
    texture_picker_base->addTexture("../media/textures/greek_cornice.jpg","AI");
    texture_picker_base->addTexture("../media/textures/kama_sutra.jpg","AI");



    MakeCircleImages(driver);

    bool s_down=false;

    core::rect<s32> windowsize_0 = driver->getViewPort();
    
    cout << "Loading Project\n";

    //**AUTO LOAD SCENE**
    Geometry_Scene_File_IO file_io(device->getFileSystem());
    file_io.AutoLoad(gs_coordinator, "../projects/test03");

    //===============================================================
    //scene = *gs_coordinator->current_scene();
    /*
    clip_results results;

    polyfold pf = gs_coordinator->current_scene()->geoNode()->elements[0].brush;
    pf.make_concave();

    //polyfold pf2 = *gs_coordinator->current_scene()->geoNode()->get_total_geometry();
    polyfold pf2 = gs_coordinator->current_scene()->geoNode()->elements[1].geometry;
    //gs_coordinator->current_scene()->geoNode()->build_intersecting_target(pf, pf2);

    clip_poly_accelerated2(pf, pf2, GEO_SUBTRACT, GEO_SOLID, results, graph);
    */
     //addDrawLines(pf, graph, graph2, graph4);
     //addDrawLines(pf2, graph, graph2, graph4);
     
    /*
     std::vector<triangle_holder> triangles;
     triangles.resize(pf2.faces.size());

     for (int i = 0; i < pf2.faces.size(); i++)
     {
         if (pf2.faces[i].loops.size() > 0)
         {
             pf2.trianglize(i, triangles[i], NULL, nograph, nograph);
         }
     }

     for (int i = 0; i < triangles.size(); i++)
     {
         for (int j = 0; j < triangles[i].triangles.size(); j++)
         {
             int a = triangles[i].triangles[j].A;
             int b = triangles[i].triangles[j].B;
             int c = triangles[i].triangles[j].C;
             core::line3df aline(triangles[i].vertices[a], triangles[i].vertices[b]);
             core::line3df bline(triangles[i].vertices[a], triangles[i].vertices[c]);
             core::line3df cline(triangles[i].vertices[c], triangles[i].vertices[b]);
             graph.lines.push_back(aline);
             graph.lines.push_back(bline);
             graph.lines.push_back(cline);
         }
     }
     */

     CameraQuad* cameraQuad = gui_layout->getCameraQuad();

     //((TestPanel_3D*)cameraQuad->getPanel(0))->AddGraphs(graph, graph2, graph4);
     
     TestPanel_3D* main_panel = (TestPanel_3D * )cameraQuad->panel_TL;
     
    //================================================================

     bool bWriteToFramebuffer = false;

     core::stringw str = L"Das Irrlicht Engine  [";
         str += driver->getName();
     str += "] ";
     device->setWindowCaption(str.c_str());

     //Render_Tool::show();
     {
         core::rect<s32> windowsize = driver->getViewPort();
         gui_layout->resize(windowsize);
     }

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

        if (receiver.IsKeyDown(KEY_KEY_F) && bWriteToFramebuffer == false)
        {
            cout << "Writing to Default Framebuffer\n";
            bWriteToFramebuffer = true;
        }

		driver->beginScene(true, true, video::SColor(255,32,32,32));

        if (bWriteToFramebuffer == false)
        {
            renderList->renderAll();

            gui->drawAll();
        }
        else
        { 
            main_panel->render_to_framebuffer();
        }

		driver->endScene();
        /*
		int fps = driver->getFPS();

		if (lastFPS != fps)
		{
			core::stringw str = L"Das Irrlicht Engine  [";
			str += driver->getName();
			str += "] FPS:";
			str += fps;

			device->setWindowCaption(str.c_str());
			lastFPS = fps;
		}*/
	}

	device->drop();

	return 0;
}
