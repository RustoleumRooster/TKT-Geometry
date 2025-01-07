#include "CameraPanel.h"
#include <irrlicht.h>
#include <iostream>
#include "edit_env.h"
#include "CMeshBuffer.h"
#include "utils.h"
#include "texture_adjust.h"
#include "reflected_nodes.h"
#include "node_properties.h"
#include "material_groups.h"
#include "BVH.h"
#include "lightmaps_tool.h"
#include "geometry_scene.h"
#include "CMeshSceneNode.h"

extern IrrlichtDevice* device;
using namespace irr;
using namespace core;
using namespace gui;
using namespace std;

extern IEventReceiver* ContextMenuOwner = NULL;
extern TestPanel* Active_Camera_Window = NULL;

extern irr::video::ITexture* small_circle_tex_add_selected;
extern irr::video::ITexture* small_circle_tex_add_not_selected;
extern irr::video::ITexture* small_circle_tex_sub_selected;
extern irr::video::ITexture* small_circle_tex_sub_not_selected;
extern irr::video::ITexture* small_circle_tex_red_selected;
extern irr::video::ITexture* small_circle_tex_red_not_selected;

extern irr::video::ITexture* med_circle_tex_add_selected;
extern irr::video::ITexture* med_circle_tex_add_not_selected;
extern irr::video::ITexture* med_circle_tex_sub_selected;
extern irr::video::ITexture* med_circle_tex_sub_not_selected;
extern irr::video::ITexture* med_circle_tex_red_selected;
extern irr::video::ITexture* med_circle_tex_red_not_selected;

CameraQuad::CameraQuad (IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle,video::IVideoDriver* driver_)
: IGUIElement(EGUIET_IMAGE,environment, parent, id, rectangle), driver(driver_)
{

}

void CameraQuad::render()
{
    if(panel_TL)
    {
        panel_TL->SetMeshNodesVisible();
        panel_TL->render();
    }
    if(panel_TR)
    {
        panel_TR->SetMeshNodesVisible();
        panel_TR->render();
    }
    if(panel_BL)
    {
        panel_BL->SetMeshNodesVisible();
        panel_BL->render();
    }
    if(panel_BR)
    {
        panel_BR->SetMeshNodesVisible();
        panel_BR->render();
    }
}

void CameraQuad::SetFullscreen(bool bFullscreen, ViewPanel* panel)
{

    if(bFullscreen)
    {
        //vp_FS->hookup(panel);
        //renderList->add(vp_FS);
        //renderList->remove(vp_TL);
        vp_TL->position(AbsoluteRect, 1.0, 1.0, 0);
        ((MyEventReceiver*)device->getEventReceiver())->resizeView(vp_TL->getSize());

        renderList->remove(vp_TR);
        renderList->remove(vp_BL);
        renderList->remove(vp_BR);

        vp_TR->position(AbsoluteRect, 0, 0, 0);
        vp_BL->position(AbsoluteRect, 0, 0, 0);
        vp_BR->position(AbsoluteRect, 0, 0, 0);
    }
    else
    {
        //vp_FS->disconnect();
        //renderList->remove(vp_FS);
        //renderList->add(vp_TL);
        vp_TL->position(AbsoluteRect, 0.5, 0.5, 0);
        ((MyEventReceiver*)device->getEventReceiver())->resizeView(vp_TL->getSize());

        renderList->add(vp_TR);
        renderList->add(vp_BL);
        renderList->add(vp_BR);

        vp_TR->position(AbsoluteRect, 0.5, 0.5, 1);
        vp_BL->position(AbsoluteRect, 0.5, 0.5, 2);
        vp_BR->position(AbsoluteRect, 0.5, 0.5, 3);
    }

    m_bFullscreen = bFullscreen;
}

void CameraQuad::initialize(geometry_scene* geo_scene)
{
    int border = 4;
    int width = AbsoluteRect.getWidth();
    int height = AbsoluteRect.getHeight();

    TestPanel_2D* test_panel = new TestPanel_2D(Environment, driver, this,
		GUI_ID_PANEL_2D, core::rect<s32>(core::position2d<s32>(466,8), core::dimension2d<u32>(450,300)));
    TestPanel_3D* test_panel2 = new TestPanel_3D(Environment, driver, this,
		GUI_ID_PANEL_3D, core::rect<s32>(core::position2d<s32>(8,8), core::dimension2d<u32>(450,300)));
    TestPanel_2D* test_panel3 = new TestPanel_2D(Environment, driver, this,
		GUI_ID_PANEL_2D_1, core::rect<s32>(core::position2d<s32>(8,316), core::dimension2d<u32>(450,300)));
    TestPanel_2D* test_panel4 = new TestPanel_2D(Environment, driver, this,
		GUI_ID_PANEL_2D_2, core::rect<s32>(core::position2d<s32>(466,316), core::dimension2d<u32>(450,300)));
   
    test_panel->Initialize(geo_scene);
    test_panel2->Initialize(geo_scene);
    test_panel3->Initialize(geo_scene);
    test_panel4->Initialize(geo_scene);

    panel_TL = test_panel2;
    panel_TR = test_panel;
    panel_BL = test_panel3;
    panel_BR = test_panel4;

    core::rect<s32> TL_rect = core::rect<s32>(core::position2d<s32>(8, 8), core::dimension2d<u32>(450, 300));
    core::rect<s32> TR_rect = core::rect<s32>(core::position2d<s32>(466, 8), core::dimension2d<u32>(450, 300));
    core::rect<s32> BL_rect = core::rect<s32>(core::position2d<s32>(8, 316), core::dimension2d<u32>(450, 300));
    core::rect<s32> BR_rect = core::rect<s32>(core::position2d<s32>(466, 316), core::dimension2d<u32>(450, 300));
    core::rect<s32> FS_rect = core::rect<s32>(core::position2d<s32>(0, 0), core::dimension2d<u32>(0, 0));

    vp_TL = new ViewPanel(Environment, driver, this, GUI_ID_PANEL_3D, TL_rect);
    //vp_TL->resize(core::vector2di(border, border), core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    vp_TL->resize(core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    ((MyEventReceiver*)device->getEventReceiver())->resizeView(vp_TL->getSize());
    vp_TL->set_location(core::vector2di(border, border));

    vp_TR = new ViewPanel(Environment, driver, this, 0, TR_rect);
    //vp_TR->resize(core::vector2di(width / 2 + border * 0.5, border), core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    vp_TR->resize(core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    vp_TR->set_location(core::vector2di(width / 2 + border * 0.5, border));

    vp_BL = new ViewPanel(Environment, driver, this, 0, BL_rect);
    //vp_BL->resize(core::vector2di(border, height / 2 + border * 0.5), core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    vp_BL->resize(core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    vp_BL->set_location(core::vector2di(border, height / 2 + border * 0.5));

    vp_BR = new ViewPanel(Environment, driver, this, 0, BR_rect);
    //vp_BR->resize(core::vector2di(width / 2 + border * 0.5, height / 2 + border * 0.5), core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    vp_BR->resize(core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    vp_BR->set_location(core::vector2di(width / 2 + border * 0.5, height / 2 + border * 0.5));

    //vp_FS = new ViewPanel(Environment, driver, this, 0, FS_rect);
    //vp_FS->resize(core::vector2di(0, 0), core::dimension2d<u32>(0, 0));
   

    vp_TL->hookup(panel_TL);

    vp_TR->hookup(panel_TR);

    //uv_edit->setBase(LM_Viewer_base);
    //vp_TR->hookup(uv_edit);

    vp_BL->hookup(panel_BL);
    vp_BR->hookup(panel_BR);

    renderList->add(vp_TL);
    renderList->add(vp_TR);
    renderList->add(vp_BL);
    renderList->add(vp_BR);
    //renderList->add(vp_FS);

    
    test_panel->setAxis(CAMERA_X_AXIS);
    test_panel2->SetViewStyle(PANEL3D_VIEW_LOOPS);
    test_panel3->setAxis(CAMERA_Y_AXIS);
    test_panel4->setAxis(CAMERA_Z_AXIS);
}

void CameraQuad::set_scene(geometry_scene* geo_scene)
{
    panel_TL->set_scene(geo_scene);
    panel_TR->set_scene(geo_scene);
    panel_BL->set_scene(geo_scene);
    panel_BR->set_scene(geo_scene);
}

void CameraQuad::resize(core::rect<s32> rect)
{
    DesiredRect = rect;
    updateAbsolutePosition();

    int border = 4;
    int width = AbsoluteRect.getWidth();
    int height = AbsoluteRect.getHeight();

    //vp_TL->resize(core::vector2di(border, border), core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    //vp_TR->resize(core::vector2di(width / 2 + border * 0.5, border), core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    //vp_BL->resize(core::vector2di(border, height / 2 + border * 0.5), core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    //vp_BR->resize(core::vector2di(width / 2 + border * 0.5, height / 2 + border * 0.5), core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    //vp_FS->resize(core::vector2di(border, border), core::dimension2d<u32>(width  - border * 2, height  - border * 2));
    SetFullscreen(m_bFullscreen, NULL);
}

void CameraQuad::hookup_aux_panel(TestPanel* pan)
{
    vp_TR->hookup(pan);
}

vector3df CameraQuad::get_fp_camera_pos()
{
    if (panel_TL)
    {
        return panel_TL->getCameraPos();
    }
    return vector3df();
}

vector3df CameraQuad::get_fp_camera_rot()
{
    if (panel_TL)
    {
        return panel_TL->getCamera()->getRotation();
    }
    return vector3df();
}

ICameraSceneNode* CameraQuad::get_fp_camera()
{
    if (panel_TL)
    {
        return panel_TL->getCamera();
    }
    return nullptr;
}

void CameraQuad::SetPanel(int i,TestPanel* panel)
{
    switch(i)
    {
    case 0:
        panel_TL = panel ;
        break;
    case 1:
        panel_TR = panel ;
        break;
    case 2:
        panel_BL = panel ;
        break;
    case 3:
        panel_BR = panel ;
        break;
    }
}

TestPanel* CameraQuad::getPanel(int i)
{
    switch(i)
    {
    case 0:
        return panel_TL;
    case 1:
        return panel_TR;
    case 2:
        return panel_BL;
    case 3:
        return panel_BR;
    }

    return NULL;
}

void CameraQuad::setGridSnap(int snap)
{
    this->grid_snap=snap;

    if(panel_BL)
        panel_BL->setGridSnap(snap);
    if(panel_BR)
        panel_BR->setGridSnap(snap);
    if(panel_TL)
        panel_TL->setGridSnap(snap);
    if(panel_TR)
        panel_TR->setGridSnap(snap);
}

void CameraQuad::setRotateSnap(f32 snap)
{
    this->rotate_snap=snap;

    if(panel_BL)
        panel_BL->setRotateSnap(snap);
    if(panel_BR)
        panel_BR->setRotateSnap(snap);
    if(panel_TL)
        panel_TL->setRotateSnap(snap);
    if(panel_TR)
        panel_TR->setRotateSnap(snap);
}

int CameraQuad::getGridSnap()
{
    return grid_snap;
}

f32 CameraQuad::getRotateSnap()
{
    return rotate_snap;
}

TestPanel::TestPanel(IGUIEnvironment* environment, video::IVideoDriver* driver_, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
    : environment(environment), driver(driver_), Texture(0), Special_Texture(0), Color(255, 255, 255, 255)
{
	#ifdef _DEBUG
	//setDebugName("TestPanel");
	#endif
}

TestPanel::~TestPanel()
{
	if (Texture)
		Texture->drop();
}

bool TestPanel::OnEvent(const SEvent& event)
{
	{
		switch(event.EventType)

		{
		    case EET_GUI_EVENT:
                switch(event.GUIEvent.EventType)
                {
                    case EGET_ELEMENT_HOVERED:
                        {
                            //std::cout<<"hover\n";
                           // Environment->setFocus(this);
                        }
                        break;
                    case EGET_ELEMENT_LEFT:
                        {
                            //std::cout<<"left\n";
                        }
                        break;
                        /*
                    case EGET_ELEMENT_FOCUS_LOST:
                       // if (event.GUIEvent.Caller == this)// && !isMyChild(event.GUIEvent.Element) )
                        {
                            //std::cout<<"lost focus\n";

                            return false;
                        }
                        break;
                    case EGET_ELEMENT_FOCUSED:
                        if (event.GUIEvent.Caller == this )
                        {
                            //std::cout<<"focus\n";
                            //return true;
                            return false;
                        }
                        break;
                        */
                    default:
                        break;
                }
                break;
		    case EET_MOUSE_INPUT_EVENT:

                switch(event.MouseInput.Event)
                {
                    case EMIE_LMOUSE_LEFT_UP:
                        {/*
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                Environment->removeFocus(this);
                            }*/
                           // std::cout<<"lmouse up\n";
                        }
                        return true;
                    case EMIE_LMOUSE_PRESSED_DOWN:
                        {

                            //std::cout<<"lmouse down\n";;
                        }
                        return true;
                    case EMIE_RMOUSE_PRESSED_DOWN:
                        {
                            //if (Environment->hasFocus(this))
                        /*
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                Environment->removeFocus(this);
                            }
                            else
                            {
                                Environment->setFocus(this);
                               // std::cout<<"rmouse down\n";
                            }*/
                        }
                        return true;
                    case EMIE_RMOUSE_LEFT_UP:
                        {
                        /*
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                Environment->removeFocus(this);
                                //std::cout<<"rmouse up\n";
                            }*/
                             //else
                                //std::cout<<"rmouse up\n";
                        }
                        return true;
                    case EMIE_MOUSE_MOVED:
                        //if (Environment->hasFocus(this))
                        //	highlight(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y), true);
                        //std::cout<<"move\n";
                        return true;
                default:
                    break;
			}
			break;
		default:
			break;
		}
	}
    //return IGUIElement::OnEvent(event);
    return true;
}


//! sets an image
void TestPanel::setImage(video::ITexture* image)
{
	if (image == Texture)
		return;

	if (Texture)
		Texture->drop();

	Texture = image;

	if (Texture)
		Texture->grab();
}

//! Gets the image texture
video::ITexture* TestPanel::getImage() const
{
	return Texture;
}
void TestPanel::Initialize(geometry_scene* geo_scene_)
{
    set_scene(geo_scene_);
}

void TestPanel::set_scene(geometry_scene* scene)
{
    this->smgr = scene->get_smgr();
    this->geo_scene = scene;
}

void ViewPanel::position(const core::recti& myrect, f32 x_split, f32 y_split, int quad)
{

    //  0   1
    //  2   3

    int border = 4;
    int width = myrect.getWidth();
    int height = myrect.getHeight();

    int xpos;
    int ypos;
    u32 xlen, ylen;

    if (quad == 0 || quad == 1)
    {
        ypos = border;
        ylen = height * y_split - (border * 1.5);
    }
    else
    {
        ypos = height * y_split + (border * 0.5);
        ylen = height * (1.0f - y_split) - (border * 1.5);
    }

    if (quad == 0 || quad == 2)
    {
        xpos = border;
        xlen = width * x_split - (border * 1.5);
    }
    else
    {
        xpos = width * x_split + (border * 0.5);
        xlen = width * (1.0f - x_split) - (border * 1.5);
    }

    //resize(core::vector2di(xpos, ypos), core::dimension2d<u32>(xlen, ylen)); 
    resize(core::dimension2d<u32>(xlen, ylen)); 
    set_location(core::vector2di(xpos, ypos));
}



void TestPanel::toggle_grid()
{
    bShowGrid=!bShowGrid;
}
void TestPanel::toggle_showBrushes()
{
    bShowBrushes=!bShowBrushes;
}
void TestPanel::toggle_showGeometry()
{
    bShowGeometry=!bShowGeometry;
}
bool TestPanel::IsShowGrid()
{
    return bShowGrid;
}
bool TestPanel::IsShowBrushes()
{
    return bShowBrushes;
}

bool TestPanel::IsDynamicLight()
{
    return lighting_type != LIGHTING_UNLIT;
}

void TestPanel::setTotalGeometry(polyfold* pf) { this->total_geometry = pf; }

void TestPanel::overrideMeshNode(scene::CMeshSceneNode* node) { this->override_mesh_node = node; }

bool TestPanel::IsShowGeometry()
{
    return bShowGeometry;
}

bool click_hits_scene_node(Reflected_SceneNode*,core::vector2di v)
{

    return false;
}

Reflected_SceneNode* TestPanel::GetSceneNodeHit(int x, int y, bool bGeometry)
{
    driver->setRenderTarget(Special_Texture, true, true, video::SColor(255,0,0,0));
    smgr->setActiveCamera(getCamera());

    driver->setTransform ( video::ETS_PROJECTION, core::IdentityMatrix );
	driver->setTransform ( video::ETS_VIEW, core::IdentityMatrix );
	driver->setTransform ( video::ETS_WORLD, core::IdentityMatrix );

	getCamera()->render();

	// ????
	//for (i=video::ETS_COUNT-1; i>=video::ETS_TEXTURE_0; --i)
	//	Driver->setTransform ( (video::E_TRANSFORMATION_STATE)i, core::IdentityMatrix );

    video::SMaterial someMaterial;
    someMaterial.Lighting = false;
    someMaterial.MaterialType = video::EMT_SOLID;

    //std::vector<video::SColor> colors;

    //for(Reflected_SceneNode* node : this->geo_scene->getSceneNodes())
    for(ISceneNode* it : geo_scene->EditorNodes()->getChildren())
    {
        Reflected_SceneNode* node = (Reflected_SceneNode*)it;

        //colors.push_back(node->m_unique_color);
        node->render_special(someMaterial);
    }

    if(bGeometry)
        geo_scene->getMeshNode()->render_special(someMaterial);

    driver->setRenderTarget(NULL, true, true);

    video::IImage* img = driver->createImage(Special_Texture,core::vector2di(0,0),Special_Texture->getSize());
    video::SColor h_col = img->getPixel(x,y);
    img->drop();

    for (ISceneNode* it : geo_scene->EditorNodes()->getChildren())
    {
        Reflected_SceneNode* node = (Reflected_SceneNode*)it;

        //colors.push_back(node->m_unique_color);
        if (node->m_unique_color == h_col)
            return node;
    }

    //for(int i=0;i<colors.size();i++)
    //{
    //    if(colors[i]==h_col)
    //        return i;
    //}

    return NULL;
}

bool TestPanel::click_hits_poly(polyfold* brush, core::vector2di v)
{
    for(int j=0;j<brush->edges.size();j++)
        {
            core::vector3df v0 = brush->getVertex(j,0).V;
            core::vector3df v1 = brush->getVertex(j,1).V;
            core::vector2di w0;
            core::vector2di w1;
            if(GetScreenCoords(v0,w0) && GetScreenCoords(v1,w1))
                {
                core::line2di screen_line(w0,w1);

                core::vector2di click_coord(v.X,v.Y);
                core::vector2di r = screen_line.getClosestPoint(click_coord);
                f32 d = r.getDistanceFrom(click_coord);
                if(d<4)
                    {
                    return true;
                    }
                }
        }
    return false;
}

std::vector<int>  TestPanel::click_hits_poly_brushes(core::vector2di v)
{
    std::vector<int> ret;
    if(this->geo_scene == NULL) return ret;
/*
    core::vector3df hit_vec;
    GetOrthoClickPoint(this->Texture->getOriginalSize(),this->getCamera(),v.X,v.Y,hit_vec);

    core::vector3df look_dir = this->getCamera()->getTarget()-this->getCamera()->getAbsolutePosition();
    look_dir.normalize();
    core::line3df line(hit_vec,hit_vec+look_dir*3000);
*/
    GeometryStack* geo_node = geo_scene->geoNode();
    for (int i = 0; i < geo_node->elements.size(); i++)
    {
        polyfold* brush = &geo_node->elements[i].brush;

        for (int j = 0; j < brush->edges.size(); j++)
        {
            core::vector3df v0 = brush->getVertex(j, 0).V;
            core::vector3df v1 = brush->getVertex(j, 1).V;
            core::vector2di w0;
            core::vector2di w1;

            if (GetScreenCoords(v0, w0) && GetScreenCoords(v1, w1))
            {
                core::line2di screen_line(w0, w1);

                core::vector2di click_coord(v.X, v.Y);
                core::vector2di r = screen_line.getClosestPoint(click_coord);
                f32 d = r.getDistanceFrom(click_coord);
                if (d < 4)
                {
                    bool b = false;
                    for (int k : ret)
                        if (k == i)
                            b = true;
                    if (!b)
                        ret.push_back(i);
                }
            }
        }
    }
 
    return ret;
}

/*
std::vector<int>  TestPanel::click_hits_poly_faces(core::vector2di v)
{
    std::vector<int> ret;
    if(this->geo_scene == NULL) return ret;

    for(int i=0;i<geo_scene->elements.size();i++)
    {
        polyfold* brush = &geo_scene->elements[i].brush;

        for(int j=0;j<brush->faces.size();j++)
        {
            core::vector3df v0 = brush->getVertex(j,0).V;
            core::vector3df v1 = brush->getVertex(j,1).V;
            core::vector2di w0;
            core::vector2di w1;
            if(GetScreenCoords(v0,w0) && GetScreenCoords(v1,w1))
                {
                core::line2di screen_line(w0,w1);

                core::vector2di click_coord(v.X,v.Y);
                core::vector2di r = screen_line.getClosestPoint(click_coord);
                f32 d = r.getDistanceFrom(click_coord);
                if(d<4)
                    {
                    bool b = false;
                    for(int k : ret)
                        if(k==i)
                            b=true;
                    if(!b)
                        ret.push_back(i);
                    }
                }
        }
    }
    return ret;
}
*/

void TestPanel::hookup_panel(ViewPanel* panel)
{
    Texture = panel->getTexture();
    Special_Texture = panel->getSpecialTexture();
    m_viewPanel = panel;
}

void TestPanel::disconnect_panel()
{
    Texture = NULL;
    m_viewPanel = NULL;
}


void RenderList::add(RenderTarget* vp)
{
    render_targets.push_back(vp);
}
void RenderList::remove(RenderTarget* vp)
{
    vector<RenderTarget*> new_panels;
    for (RenderTarget* p : render_targets)
    {
        if (p != vp)
            new_panels.push_back(p);
    }
    render_targets = std::move(new_panels);
}

void RenderList::renderAll()
{
    for (RenderTarget* vp : render_targets)
    {
        if (vp->isActive())
            vp->render(driver);
    }
}


//! constructor
ViewPanel::ViewPanel(IGUIEnvironment* environment, video::IVideoDriver* driver_, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
    : IGUIElement(EGUIET_IMAGE, environment, parent, id, rectangle), driver(driver_), Texture(0), Special_Texture(0), Color(255, 255, 255, 255), m_panel(NULL)
{
#ifdef _DEBUG
    setDebugName("ViewPanel");
#endif
    ((MyEventReceiver*)device->getEventReceiver())->Register(this);
}

//! destructor
ViewPanel::~ViewPanel()
{
    if (Texture)
        Texture->drop();

    if (Special_Texture)
        Special_Texture->drop();

    Texture = NULL;
    Special_Texture = NULL;

    ((MyEventReceiver*)device->getEventReceiver())->UnRegister(this);
}

void ViewPanel::set_location(core::dimension2d<s32> new_location)
{
    this->DesiredRect = core::rect<s32>(new_location, panel_size);

    recalculateAbsolutePosition(false);
}

void ViewPanel::resize(core::dimension2d<u32> new_size)
{
    if (this->Texture)
    {
        driver->removeTexture(this->Texture);
    }

    if (this->Special_Texture)
    {
        driver->removeTexture(this->Special_Texture);
    }
  
    this->Texture = driver->addRenderTargetTexture(new_size, "rtt", irr::video::ECF_A8R8G8B8);
    this->Special_Texture = driver->addRenderTargetTexture(new_size, "rtt", irr::video::ECF_A8R8G8B8);

    //this->DesiredRect = core::rect<s32>(new_location, new_size);

    //recalculateAbsolutePosition(false);

    panel_size = new_size;

    if (m_panel)
    {
        m_panel->hookup_panel(this);
        m_panel->resize(new_size);
    }
}

void ViewPanel::render(video::IVideoDriver* driver) 
{ 
    if (m_panel)
    {
        m_panel->SetMeshNodesVisible();
        m_panel->render();
    }
}

void ViewPanel::disconnect()
{
    if (m_panel)
    {
        m_panel->disconnect_panel();
    }

    if (panel_stack.size() > 0)
    {
        m_panel = panel_stack[panel_stack.size() - 1];
        panel_stack.pop_back();

        m_panel->hookup_panel(this);
        m_panel->resize(this->getTexture()->getOriginalSize());
    }
    else
        m_panel = NULL;
}

void ViewPanel::draw()
{
    if (!IsVisible)
        return;

    IGUISkin* skin = Environment->getSkin();
    video::IVideoDriver* driver = Environment->getVideoDriver();

    if (Texture)
    {
        //if (ScaleImage)
        if (true)
        {
            const video::SColor Colors[] = { Color,Color,Color,Color };
            if (false)//(bTest)
            {

                //if (tex)
                //{
                //    driver->draw2DImage(tex, AbsoluteRect,
                //        core::rect<s32>(core::position2d<s32>(0, 0), core::dimension2di(tex->getOriginalSize())),
                //        &AbsoluteClippingRect, Colors, UseAlphaChannel);
               // }
            }
            else
                driver->draw2DImage(Texture, AbsoluteRect,
                    core::rect<s32>(core::position2d<s32>(0, 0), core::dimension2di(Texture->getOriginalSize())),
                    &AbsoluteClippingRect, Colors, true);
        }
        else
        {
            driver->draw2DImage(Texture, AbsoluteRect.UpperLeftCorner,
                core::rect<s32>(core::position2d<s32>(0, 0), core::dimension2di(Texture->getOriginalSize())),
                &AbsoluteClippingRect, Color, true);
        }
    }
    else
    {
        skin->draw2DRectangle(this, skin->getColor(EGDC_3D_DARK_SHADOW), AbsoluteRect, &AbsoluteClippingRect);
    }

    IGUIElement::draw();
}

void ViewPanel::hookup(TestPanel* panel)
{
    if (m_panel)
    {
        panel_stack.push_back(m_panel);
    }

    m_panel = panel;
    m_panel->hookup_panel(this);
    m_panel->resize(this->getTexture()->getOriginalSize());
}

bool ViewPanel::OnEvent(const SEvent& event)
{
    if (isEnabled())
    {
        switch (event.EventType)
        {
        case EET_USER_EVENT:
            m_panel->OnEvent(event);
            return true;
            break;
        case EET_MOUSE_INPUT_EVENT:
        {
            clickx = event.MouseInput.X;
            clicky = event.MouseInput.Y;

            switch (event.MouseInput.Event)
            {
                case EMIE_LMOUSE_LEFT_UP:
                {
                    if (!AbsoluteClippingRect.isPointInside(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y)))
                    {
                        Environment->removeFocus(this);
                        return true;
                    }
                }
                break;

                case EMIE_LMOUSE_PRESSED_DOWN:
                {
                    if (!AbsoluteClippingRect.isPointInside(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y)))
                    {
                        Environment->removeFocus(this);
                        return false;
                    }
                }
                break;
                case EMIE_RMOUSE_PRESSED_DOWN:
                {
                    if (!AbsoluteClippingRect.isPointInside(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y)))
                    {
                        Environment->removeFocus(this);
                    }
                    else
                    {
                        Environment->setFocus(this);
                    }

                    if (Environment->getFocus() != this)
                        return false;
                }
                break;
                case EMIE_RMOUSE_LEFT_UP:
                {
                    if (!AbsoluteClippingRect.isPointInside(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y)))
                    {
                        Environment->removeFocus(this);
                    }
                } break;
            }
            SEvent e = event;

            e.MouseInput.X = event.MouseInput.X - AbsoluteClippingRect.UpperLeftCorner.X;
            e.MouseInput.Y = event.MouseInput.Y - AbsoluteClippingRect.UpperLeftCorner.Y;

            if (m_panel)
            {
                if (m_panel->OnEvent(e))
                    return true;
            }

        }break;
        }

        if (m_panel)
        {
            if(m_panel->OnEvent(event))
                return true;
        }
    }
    return false;
   // return IGUIElement::OnEvent(event);
}

void ViewPanel::set_fullscreen(bool bFullscreen)
{
    ((CameraQuad*)this->getParent())->SetFullscreen(bFullscreen, this);
}
