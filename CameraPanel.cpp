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

extern IrrlichtDevice* device;
using namespace irr;
using namespace core;
using namespace gui;

extern TestPanel* ContextMenuOwner=NULL;

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

void CameraQuad::SetFullscreen(bool bFullscreen,TestPanel* panel)
{
    int border = 4;
    int width = AbsoluteRect.getWidth();
    int height = AbsoluteRect.getHeight();

    if(bFullscreen)
    {
        core::rect<s32> full_rect(core::vector2di(border,border),core::vector2di(width-border,height-border));

        //panel_TL->resize(core::position2d<s32>(8,8),core::dimension2d<u32>(900,600));

        panel_TL->resize(core::vector2di(border,border),core::dimension2du(width-border*2,height-border*2));

        if(panel_BL)
            panel_BL->setVisible(false);
        if(panel_TR)
            panel_TR->setVisible(false);
        if(panel_BR)
            panel_BR->setVisible(false);
    }
    else
    {
        panel_TL->resize(core::vector2di(border,border),core::dimension2d<u32>(width/2-border*1.5,height/2-border*1.5));

        if(panel_BL)
            panel_BL->setVisible(true);
        if(panel_TR)
            panel_TR->setVisible(true);
        if(panel_BR)
            panel_BR->setVisible(true);
    }

    m_bFullscreen = bFullscreen;
}

void CameraQuad::initialize(scene::ISceneManager* smgr,geometry_scene* geo_scene)
{
    int border = 4;
    int width = AbsoluteRect.getWidth();
    int height = AbsoluteRect.getHeight();

    TestPanel_2D* test_panel = new TestPanel_2D(Environment, driver, this,
		GUI_ID_PANEL_2D, core::rect<s32>(core::position2d<s32>(466,8), core::dimension2d<u32>(450,300)));
    //test_panel->resize(core::position2d<s32>(466,8), core::dimension2d<u32>(450,300));
    test_panel->Initialize(smgr, geo_scene);
    test_panel->resize(core::vector2di(width/2+border*0.5,border),core::dimension2d<u32>(width/2-border*1.5,height/2-border*1.5));
    test_panel->setAxis(CAMERA_X_AXIS);

    TestPanel_3D* test_panel2 = new TestPanel_3D(Environment, driver, this,
		GUI_ID_PANEL_3D, core::rect<s32>(core::position2d<s32>(8,8), core::dimension2d<u32>(450,300)));
    //test_panel2->resize(core::position2d<s32>(8,8), core::dimension2d<u32>(450,300));
    test_panel2->Initialize(smgr, geo_scene);
    test_panel2->resize(core::vector2di(border,border),core::dimension2d<u32>(width/2-border*1.5,height/2-border*1.5));
    //test_panel2->setTotalGeometry(&pf);
    //test_panel2->overrideMeshNode(mesh_node);
    test_panel2->SetViewStyle(PANEL3D_VIEW_LOOPS);

    TestPanel_2D* test_panel3 = new TestPanel_2D(Environment, driver, this,
		GUI_ID_PANEL_2D_1, core::rect<s32>(core::position2d<s32>(8,316), core::dimension2d<u32>(450,300)));
    //test_panel3->resize(core::position2d<s32>(8,316), core::dimension2d<u32>(450,300));
    test_panel3->Initialize(smgr, geo_scene);
    test_panel3->resize(core::vector2di(border,height/2+border*0.5),core::dimension2d<u32>(width/2-border*1.5,height/2-border*1.5));
    test_panel3->setAxis(CAMERA_Y_AXIS);

    TestPanel_2D* test_panel4 = new TestPanel_2D(Environment, driver, this,
		GUI_ID_PANEL_2D_2, core::rect<s32>(core::position2d<s32>(466,316), core::dimension2d<u32>(450,300)));
    //test_panel4->resize(core::position2d<s32>(466,316), core::dimension2d<u32>(450,300));
    test_panel4->Initialize(smgr, geo_scene);
    test_panel4->resize(core::vector2di(width/2+border*0.5,height/2+border*0.5),core::dimension2d<u32>(width/2-border*1.5,height/2-border*1.5));
    test_panel4->setAxis(CAMERA_Z_AXIS);

    panel_TL = test_panel2;
    panel_TR = test_panel;
    panel_BL = test_panel3;
    panel_BR = test_panel4;
}

void CameraQuad::resize(core::rect<s32> rect)
{
    DesiredRect = rect;
    updateAbsolutePosition();

    int border = 4;
    int width = AbsoluteRect.getWidth();
    int height = AbsoluteRect.getHeight();

    panel_TR->resize(core::vector2di(width / 2 + border * 0.5, border), core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    panel_BL->resize(core::vector2di(border, height / 2 + border * 0.5), core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));
    panel_BR->resize(core::vector2di(width / 2 + border * 0.5, height / 2 + border * 0.5), core::dimension2d<u32>(width / 2 - border * 1.5, height / 2 - border * 1.5));

    SetFullscreen(m_bFullscreen, NULL);
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

//! constructor
TestPanel::TestPanel(IGUIEnvironment* environment, video::IVideoDriver* driver_, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
: IGUIElement(EGUIET_IMAGE,environment, parent, id, rectangle), driver(driver_), Texture(0), Color(255,255,255,255),
	UseAlphaChannel(false), ScaleImage(false)
{
	#ifdef _DEBUG
	setDebugName("TestPanel");
	#endif
}


//! destructor
TestPanel::~TestPanel()
{
	if (Texture)
		Texture->drop();
}

bool TestPanel::OnEvent(const SEvent& event)
{
	if (isEnabled())
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
                        {
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                Environment->removeFocus(this);
                            }
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
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                Environment->removeFocus(this);
                            }
                            else
                            {
                                Environment->setFocus(this);
                               // std::cout<<"rmouse down\n";
                            }
                        }
                        return true;
                    case EMIE_RMOUSE_LEFT_UP:
                        {
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                Environment->removeFocus(this);
                                //std::cout<<"rmouse up\n";
                            }
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
    return IGUIElement::OnEvent(event);
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

//! sets the color of the image
void TestPanel::setColor(video::SColor color)
{
	Color = color;
}

//! Gets the color of the image
video::SColor TestPanel::getColor() const
{
	return Color;
}

//! draws the element and its children
void TestPanel::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	video::IVideoDriver* driver = Environment->getVideoDriver();

	if (Texture)
	{
		//if (ScaleImage)
		if(true)
		{
			const video::SColor Colors[] = {Color,Color,Color,Color};
            if (false)//(bTest)
            {
                /*
                if (tex)
                {
                    driver->draw2DImage(tex, AbsoluteRect,
                        core::rect<s32>(core::position2d<s32>(0, 0), core::dimension2di(tex->getOriginalSize())),
                        &AbsoluteClippingRect, Colors, UseAlphaChannel);
                }*/
            }
            else
                driver->draw2DImage(Texture, AbsoluteRect,
                    core::rect<s32>(core::position2d<s32>(0,0), core::dimension2di(Texture->getOriginalSize())),
                    &AbsoluteClippingRect, Colors, UseAlphaChannel);
		}
		else
		{
			driver->draw2DImage(Texture, AbsoluteRect.UpperLeftCorner,
				core::rect<s32>(core::position2d<s32>(0,0), core::dimension2di(Texture->getOriginalSize())),
				&AbsoluteClippingRect, Color, UseAlphaChannel);
		}
	}
	else
	{
		skin->draw2DRectangle(this, skin->getColor(EGDC_3D_DARK_SHADOW), AbsoluteRect, &AbsoluteClippingRect);
	}

	IGUIElement::draw();
}


//! sets if the image should use its alpha channel to draw itself
void TestPanel::setUseAlphaChannel(bool use)
{
	UseAlphaChannel = use;
}


//! sets if the image should use its alpha channel to draw itself
void TestPanel::setScaleImage(bool scale)
{
	ScaleImage = scale;
}


//! Returns true if the image is scaled to fit, false if not
bool TestPanel::isImageScaled() const
{
	return ScaleImage;
}

//! Returns true if the image is using the alpha channel, false if not
bool TestPanel::isAlphaChannelUsed() const
{
	return UseAlphaChannel;
}

void TestPanel::Initialize(scene::ISceneManager* smgr_ , geometry_scene* geo_scene_)
{
    this->smgr=smgr_;
    this->geo_scene=geo_scene_;
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
    return bDynamicLight;
}
bool TestPanel::IsShowGeometry()
{
    return bShowGeometry;
}

bool click_hits_scene_node(Reflected_SceneNode*,core::vector2di v)
{

    return false;
}

int TestPanel::GetSceneNodeHit(int x, int y, bool bGeometry)
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

    std::vector<video::SColor> colors;

    for(Reflected_SceneNode* node : this->geo_scene->getSceneNodes())
    {
        colors.push_back(node->m_unique_color);
        node->render_special(someMaterial);
    }

    if(bGeometry)
        geo_scene->getMeshNode()->render_special(someMaterial);

    driver->setRenderTarget(NULL, true, true);

    video::IImage* img = driver->createImage(Special_Texture,core::vector2di(0,0),Special_Texture->getSize());
    video::SColor h_col = img->getPixel(x,y);
    img->drop();

    for(int i=0;i<colors.size();i++)
    {
        if(colors[i]==h_col)
            return i;
    }

    return -1;
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
    for(int i=0;i<geo_scene->elements.size();i++)
    {
        polyfold* brush = &geo_scene->elements[i].brush;

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

//===================================================================================================
// Camera Panel 3D
//
//

TestPanel_3D::TestPanel_3D(IGUIEnvironment* environment, video::IVideoDriver* driver, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
: TestPanel(environment, driver, parent, id, rectangle)
{
    bShowBrushes=false;
    ((MyEventReceiver*)device->getEventReceiver())->Register(this);
	//#ifdef _DEBUG
	//setDebugName("TestPanel");
	//#endif
}

TestPanel_3D::~TestPanel_3D()
{
    ((MyEventReceiver*)device->getEventReceiver())->UnRegister(this);
}

void TestPanel_3D::AddGraph(LineHolder& graph4)
{
    for(vector3df v :graph4.points)
        this->graph.points.push_back(v);
    for(line3df lv :graph4.lines)
        this->graph.lines.push_back(lv);
}


void TestPanel_3D::AddGraphs(LineHolder& graph1, LineHolder& graph2, LineHolder& graph3 )
{
    for (vector3df v : graph1.points)
        this->graph.points.push_back(v);
    for (vector3df v : graph2.points)
        this->graph2.points.push_back(v);
    for (line3df lv : graph1.lines)
        this->graph.lines.push_back(lv);
    for (line3df lv : graph2.lines)
        this->graph2.lines.push_back(lv);
    for (line3df lv : graph3.lines)
        this->graph3.lines.push_back(lv);
}

scene::ICameraSceneNode* TestPanel_3D::getCamera()
{
    if(this->camera == NULL)
    {
        if(this->smgr)
        {
            this->camera = smgr->addCameraSceneNode(0,core::vector3df(-300,400,-300),core::vector3df(0,0,0),-1,false);
            this->camera->setNearValue(5.0);
            //this->camera->setPosition(core::vector3df(-300,400,-300));
            //this->camera->setTarget(core::vector3df(0,0,0));
        }
    }
    return this->camera;
}

void TestPanel_3D::resize(core::dimension2d<s32> new_location, core::dimension2d<u32> new_size)
{
    if(this->Texture)
    {
        driver->removeTexture(this->Texture);
    }

    if(this->Special_Texture)
    {
        driver->removeTexture(this->Special_Texture);
    }

    this->Texture = driver->addRenderTargetTexture(new_size, "rtt" ,irr::video::ECF_A8R8G8B8);
    this->Special_Texture = driver->addRenderTargetTexture(new_size, "rtt" ,irr::video::ECF_A8R8G8B8);

    this->DesiredRect = core::rect<s32>(new_location,new_size);

    this->getCamera()->setAspectRatio((f32)new_size.Width / (f32)new_size.Height);
    this->getCamera()->updateAbsolutePosition();

    recalculateAbsolutePosition(false);
}

core::vector3df TestPanel_3D::getDragVector()
{
    return this->plane_drag_vec;
}

bool TestPanel_3D::OnEvent(const SEvent& event)
{
    core::vector3df hitvec;
    click_brush_info hit_results;

	if (isEnabled())
	{
		switch(event.EventType)

		{
            case EET_USER_EVENT:
                switch(event.UserEvent.UserData1)
                {
                    case USER_EVENT_GEOMETRY_REBUILT:
                    {
                       this->SetViewStyle(this->view_style);
                    }
                    break;
                    case USER_EVENT_TEXTURE_EDIT_MODE_BEGIN:
                    {
                        if(!bTextureEdit)
                            {
                            bTextureEdit=true;
                           // std::cout<<"Texture Edit Mode begin\n";
                            }
                    }
                    break;
                    case USER_EVENT_TEXTURE_EDIT_MODE_END:
                    {
                        bTextureEdit=false;
                       // std::cout<<"Texture Edit Mode end\n";
                    }
                    break;
                    case USER_EVENT_SELECTION_CHANGED:
                    {
                        if(geo_scene->getSelectedFaces().size() == 0)
                        {
                            if(bTextureEdit)
                            {
                                bTextureEdit=false;
                               // std::cout<<"Texture Edit Mode end\n";
                            }
                        }
                    }
                    break;
                }
                return true;
                break;
		    case EET_GUI_EVENT:
                switch(event.GUIEvent.EventType)
                {
                    case EGET_MENU_ITEM_SELECTED:
                        {
                            this->OnMenuItemSelected((gui::IGUIContextMenu*)event.GUIEvent.Caller);
                            return true;
                        }
                        break;
                    case EGET_ELEMENT_HOVERED:
                        {
                          //  Environment->setFocus(this);
                        }
                        break;
                    case EGET_ELEMENT_LEFT:
                        {
                            //std::cout<<"left\n";
                        }
                        break;
                    case EGET_ELEMENT_FOCUS_LOST:
                        {
                            this->bMouseDown=false;
                            this->rMouseDown=false;
                            this->bRotateCamera=false;
                            this->bZoomCamera=false;
                            this->bDragCamera=false;
                        }
                        break;
                    default:
                        break;
                }
                break;
		    case EET_MOUSE_INPUT_EVENT:

		        mousex = event.MouseInput.X - AbsoluteClippingRect.UpperLeftCorner.X;
                mousey = event.MouseInput.Y - AbsoluteClippingRect.UpperLeftCorner.Y;
                bShiftDown = event.MouseInput.Shift;

                switch(event.MouseInput.Event)
                {
                    case EMIE_LMOUSE_LEFT_UP:
                        {
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                Environment->removeFocus(this);
                                return true;
                            }

                            if(bPlaneDrag)
                            {
                                bPlaneDrag=false;
                                MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();

                                SEvent event;
                                event.EventType = EET_USER_EVENT;
                                event.UserEvent.UserData1=USER_EVENT_TEXTURE_PLANE_DRAG_END;
                                receiver->OnEvent(event);
                            }
                            bDragCamera=false;
                            bZoomCamera=false;
                            bMouseDown = false;

                            if(clickx==mousex && clicky==mousey)
                                left_click(core::vector2di(event.MouseInput.X,event.MouseInput.Y));
                            //std::cout<<"lmouse up\n";
                        }
                        return true;
                    case EMIE_LMOUSE_PRESSED_DOWN:
                        {
                            //std::cout<<"lmouse down\n";
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                Environment->removeFocus(this);
                                return false;
                            }

                            if(bMouseDown==false && rMouseDown == false)
                            {
                                clickx=mousex;
                                clicky=mousey;

                                //Initiate Drag
                                if(event.MouseInput.Shift && bTextureEdit)
                                {
                                    if(get_click_face(clickx,clicky,hit_results))
                                    {
                                        bool b=false;
                                        for(int f_n : this->geo_scene->getSelectedFaces())
                                        {
                                            if(hit_results.face_n == f_n)
                                                b=true;
                                        }
                                        if(b)
                                        {
                                            plane_drag_origin = hit_results.hitvec;
                                            plane_drag_plane = hit_results.f_plane;
                                            bPlaneDrag = true;

                                            MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();

                                            SEvent event;
                                            event.EventType = EET_USER_EVENT;
                                            event.UserEvent.UserData1=USER_EVENT_TEXTURE_PLANE_DRAG;
                                            receiver->OnEvent(event);
                                        }
                                    }
                                    bRotateCamera = false;
                                    bZoomCamera = false;

                                }
                                else if(get_click_face(clickx,clicky,hit_results))
                                {
                                    vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                                    //vDragCameraInitialTarget = this->getCamera()->getTarget();

                                    core::plane3df hit_plane(hit_results.hitvec,core::vector3df(0,1,0));
                                    core::vector3df r = this->getCamera()->getAbsolutePosition() - this->getCamera()->getTarget();
                                    hit_plane.getIntersectionWithLine(this->getCamera()->getAbsolutePosition(),r,hitvec);
                                    vDragCameraInitialTarget = hitvec;
                                    vDragCameraOrigin = hit_results.hitvec;

                                    if(r.dotProduct(this->getCamera()->getAbsolutePosition()-hitvec)<0)
                                    {
                                        r.Y *= -1;
                                        hit_plane.getIntersectionWithLine(this->getCamera()->getAbsolutePosition(),r,hitvec);
                                        vDragCameraInitialTarget = hitvec;
                                    }

                                    bDragCamera = true;
                                    bRotateCamera = false;
                                    bZoomCamera = false;
                                }
                                else //if(GetPlaneClickVector(this->Texture->getOriginalSize(),this->getCamera(),clickx,clicky,hitvec))
                                {
                                    vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                                    vDragCameraInitialTarget = this->getCamera()->getTarget();

                                    core::plane3df hit_plane(vDragCameraOrigin,core::vector3df(0,1,0));
                                    GetAnyPlaneClickVector(this->Texture->getOriginalSize(),this->getCamera(),hit_plane,clickx,clicky,hitvec);
                                    vDragCameraOrigin = hitvec;

                                    core::vector3df r = vDragCameraOrigin - this->getCamera()->getAbsolutePosition();

                                    if(r.getLength() > 1000)
                                    {
                                        r.normalize();
                                        r*=1000;
                                        vDragCameraOrigin = this->getCamera()->getAbsolutePosition()+r;
                                    }

                                    bDragCamera = true;
                                    bRotateCamera = false;
                                    bZoomCamera = false;
                                }
                                bMouseDown=true;
                            }
                            else if(bMouseDown == false)
                            {
                                bMouseDown = true;
                            }
                        }
                        return true;
                    case EMIE_RMOUSE_PRESSED_DOWN:
                        {
                            //if (Environment->hasFocus(this))
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                Environment->removeFocus(this);
                            }
                            else
                            {
                                Environment->setFocus(this);
                            }

                            if(Environment->getFocus()!=this)
                                return false;

                            if(rMouseDown==false && bMouseDown==false)
                            {
                                Environment->setFocus(this);
                                clickx=mousex;
                                clicky=mousey;

                                //Initiate Rotate Camera
                                vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                                vDragCameraInitialTarget = this->getCamera()->getTarget();

                                if(get_click_face(clickx,clicky,hit_results))
                                {
                                    vDragCameraRay = this->getCamera()->getAbsolutePosition()-hit_results.hitvec;
                                    vDragCameraOrigin = hit_results.hitvec;
                                }
                                else
                                {
                                    vDragCameraRay = this->getCamera()->getAbsolutePosition()-vDragCameraOrigin;
                                }

                                if(vDragCameraRay.getLength() > 1000)
                                    {
                                        vDragCameraRay.normalize();
                                        vDragCameraRay*=1000;
                                        vDragCameraOrigin = this->getCamera()->getAbsolutePosition()-vDragCameraRay;
                                    }

                                bRotateCamera = true;
                                rMouseDown = true;
                            }
                            else if(rMouseDown == false)
                            {
                                rMouseDown = true;
                            }
                        }
                        return true;
                    case EMIE_RMOUSE_LEFT_UP:
                        {
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                Environment->removeFocus(this);
                                //std::cout<<"rmouse up\n";
                            }
                             else
                                //std::cout<<"rmouse up\n";

                            bRotateCamera=false;
                            bZoomCamera=false;
                            rMouseDown = false;

                            if(clickx==mousex && clicky==mousey)
                                right_click(core::vector2di(event.MouseInput.X,event.MouseInput.Y));
                        }
                        return true;
                    case EMIE_MOUSE_MOVED:
                        if( bMouseDown == true && rMouseDown == true)
                        {
                            if(bZoomCamera==false)
                            {
                                clickx=mousex;
                                clicky=mousey;
                                vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                                vDragCameraInitialTarget = this->getCamera()->getTarget();
                                vDragCameraRay = this->getCamera()->getAbsolutePosition()-this->getCamera()->getTarget();

                                bZoomCamera=true;
                                bRotateCamera=false;
                                bDragCamera=false;
                            }
                            else
                            {
                                int ydif = mousey-clicky;
                                float zoom_f = 1.0 - (0.02 * ydif);
                                zoom_f = fmax(zoom_f,0.01);
                                this->getCamera()->setPosition(vDragCameraInitialTarget+(vDragCameraRay*zoom_f));
                                this->getCamera()->updateAbsolutePosition();
                            }
                        }
                        else if(bRotateCamera)
                        {
                            core::vector3df r  = vDragCameraOrigin - this->getCamera()->getTarget();
                            core::vector3df r2  = vDragCameraInitialPosition - vDragCameraInitialTarget;

                            f32 d = fmin(r2.getLength(),vDragCameraRay.getLength());

                            if(vDragCameraRay.getLength()>d)
                            {
                                vDragCameraRay*=0.99;
                            }

                            int xRot = mousex-clickx;

                            float yaw = 0.008 * xRot;
                            core::vector3df newCameraRay = vDragCameraRay;
                            core::matrix4 MCamRotate;
                            MCamRotate.setRotationAxisRadians(yaw,core::vector3df(0,1,0));
                            MCamRotate.rotateVect(newCameraRay);

                            int yRot = mousey-clicky;
                            float pitch = 0.008 * yRot;

                            core::vector3df rot_axis = newCameraRay.crossProduct(core::vector3df(0,1,0));
                            rot_axis.normalize();

                            MCamRotate.setRotationAxisRadians(pitch,rot_axis);
                            MCamRotate.rotateVect(newCameraRay);

                            this->getCamera()->setPosition(vDragCameraOrigin+newCameraRay);

                            int mouse_dist = sqrt(xRot*xRot + yRot*yRot);
                            this->camera->setTarget(this->getCamera()->getTarget()+r*0.04);

                            this->getCamera()->updateAbsolutePosition();
                        }
                        else if(bDragCamera )
                        {
                            core::vector3df tvec;

                            //Update the camera's view frustrum, or we may get errors
                            this->getCamera()->render();

                            core::plane3df plane(vDragCameraOrigin,core::vector3df(0,1,0));
                            GetAnyPlaneClickVector(this->Texture->getOriginalSize(),this->getCamera(),plane,mousex,mousey,tvec);
                            core::line3df dragLine = core::line3df(vDragCameraOrigin,tvec);

                            core::vector2di m_coords(mousex,mousey);
                            core::vector2di coords;
                            core::vector3df route = dragLine.getMiddle();

                            if(GetScreenCoords(route,coords))
                            {
                                if(m_coords.getDistanceFrom(coords)>1)
                                {
                                    tvec = dragLine.getVector();
                                    tvec *=0.8;

                                    while(tvec.getLength() > 25)
                                        tvec *= 0.75;

                                    this->getCamera()->setPosition(vDragCameraInitialPosition-tvec);

                                    this->getCamera()->setTarget(vDragCameraInitialTarget-tvec);
                                    this->getCamera()->updateAbsolutePosition();

                                    vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                                    vDragCameraInitialTarget = this->getCamera()->getTarget();
                                }
                            }
                        }
                        else if(event.MouseInput.Shift && bTextureEdit && bPlaneDrag)
                        {

                            //Update the camera's view frustrum, or we may get errors
                            this->getCamera()->render();

                            GetAnyPlaneClickVector(this->Texture->getOriginalSize(),this->getCamera(),plane_drag_plane,mousex,mousey,hitvec);
                            plane_drag_vec = hitvec - plane_drag_origin;

                            geo_scene->setDragVec(plane_drag_vec);

                            MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();

                            SEvent event;
                            event.EventType = EET_USER_EVENT;
                            event.UserEvent.UserData1=USER_EVENT_TEXTURE_PLANE_DRAG;
                            receiver->OnEvent(event);
                        }
                        return true;
                    default:
                        break;
			} //switch gui event type
			break;
		default:
			break;
		} //switch event type
		
	}//if enabled
    return IGUIElement::OnEvent(event);
}

bool TestPanel::get_click_node(int x, int y, click_node_info& ret)
{
    ret.distance = 9999;
    ret.node_n = -1;
    ret.hit = false;
    //for(Reflected_SceneNode* sn : geo_scene->getSceneNodes())
    for(int i=0;i<geo_scene->getSceneNodes().size();i++)
    {
        Reflected_SceneNode* sn = geo_scene->getSceneNodes()[i];

        core::rect<s32> v_rect = ((Reflected_Sprite_SceneNode*)sn)->GetVisibleRectangle(this);

        if(v_rect.isPointInside(core::vector2di(clickx,clicky)))
        {
            f32 d = sn->getDistanceFromCamera(this);
            if(d < ret.distance)
            {
                //std::cout<<"hit object!\n";
                ret.hit = true;
                ret.distance = d;
                ret.node_n = i;
            }
        }
    }

    return ret.hit;
}

bool TestPanel::get_click_brush(int x, int y, click_brush_info& ret)
{
    core::vector3df cam_pos = this->getCamera()->getAbsolutePosition();
    f32 distance = 9999;
    ret.hit = false;

    for(int i=0;i<geo_scene->elements.size();i++)
    {
        polyfold* brush = &geo_scene->elements[i].brush;

        for(int j=0;j<brush->edges.size();j++)
        {
            core::vector3df v0 = brush->getVertex(j,0).V;
            core::vector3df v1 = brush->getVertex(j,1).V;
            core::vector2di w0;
            core::vector2di w1;

            if(GetScreenCoords(v0,w0) && GetScreenCoords(v1,w1))
                {
                core::line2di screen_line(w0,w1);

                core::vector2di click_coord(x,y);
                core::vector2di r = screen_line.getClosestPoint(click_coord);

                f32 d = r.getDistanceFrom(click_coord);
                if(d<4)
                    {
                        f32 m = (f32) core::vector2di(r - w0).getLength() /
                                (f32) core::vector2di(w1 - w0).getLength();

                        core::vector3df hit_vec = v0 + core::vector3df(v1 - v0) * m;
                        f32 dist = hit_vec.getDistanceFrom(cam_pos);

                        if(dist < distance)
                        {
                            distance = dist;
                            ret.distance = dist;
                            ret.brush_n = i;
                            ret.hit = true;
                        }
                    }
                }
        }
    }
    return ret.hit;
}

bool TestPanel_3D::get_click_face(int x, int y, click_brush_info& ret)
{
    core::vector3df cam_pos = this->getCamera()->getAbsolutePosition();
    int near_dist = this->getCamera()->getNearValue();
    core::vector3df hitvec;
    core::plane3df ret_plane;
    core::vector3df ret_vec;
    int selected_poly = -1;
    int selected_face = -1;
    f32 dist =9999;

    for(int j=0; j<this->geo_scene->get_total_geometry()->faces.size(); j++)
        {
            core::plane3df f_plane(this->geo_scene->get_total_geometry()->faces[j].m_center,
                           this->geo_scene->get_total_geometry()->faces[j].m_normal);

            if(GetAnyPlaneClickVector(this->Texture->getOriginalSize(),this->getCamera(),f_plane, x,y,hitvec) &&
               this->geo_scene->get_total_geometry()->is_point_on_face(j,hitvec))
                {
                 f32 d = hitvec.getDistanceFrom(cam_pos);
                 if(d<dist && d > near_dist &&
                    (this->geo_scene->get_total_geometry()->faces[j].m_normal.dotProduct(cam_pos-hitvec) > 0  ||
                     this->geo_scene->getMaterialGroupsBase()->material_groups[this->geo_scene->get_total_geometry()->faces[j].material_group].two_sided == true
                    )
                    &&
                    (hitvec-cam_pos).dotProduct(this->getCamera()->getTarget()-cam_pos) > 0)
                    {
                        ret_vec = hitvec;
                        ret_plane = f_plane;
                        dist = d;
                        selected_poly = 0;
                        selected_face = j;
                    }
                }
        }

    if(selected_poly != -1)
    {
        ret.hit=true;
        ret.hitvec = ret_vec;
        ret.f_plane = ret_plane;
        ret.brush_n=selected_poly;
        ret.face_n=selected_face;
        ret.distance = dist;
        return true;
    }

    ret.hit=false;

    return false;
}

void TestPanel_3D::left_click(core::vector2di pos)
{
    std::vector<int> old_sel_faces = geo_scene->getSelectedFaces();
    std::vector<int> old_sel_nodes = geo_scene->getSelectedNodes();
    std::vector<int> old_sel_brushes = geo_scene->getBrushSelection();

    if(click_hits_poly(&geo_scene->elements[0].brush,core::vector2di(clickx,clicky)))
        {
            geo_scene->setSelectedFaces(std::vector<int>{});
            geo_scene->setSelectedNodes(std::vector<int>{});
            geo_scene->setBrushSelection(std::vector<int>{0});
        }
    else if(this->bShowBrushes && !this->bShowGeometry)
    {

        click_brush_info res;
        get_click_brush(clickx,clicky,res);

        click_node_info node_hit;
        get_click_node(clickx,clicky,node_hit);

        if(bShiftDown && geo_scene->getBrushSelection().size() > 0)
        {
            if(res.hit == true)
            {
                geo_scene->setBrushSelection_ShiftAdd(res.brush_n);
            }
        }
        else if(bShiftDown && geo_scene->getSelectedNodes().size() >0)
        {
            if(node_hit.hit == true)
            {
                geo_scene->setSelectedNodes_ShiftAdd(node_hit.node_n);
            }
        }
        else if(node_hit.hit == true && (res.hit==false || node_hit.distance < res.distance))
        {
            geo_scene->setBrushSelection(std::vector<int>{});
            geo_scene->setSelectedNodes(std::vector<int>{node_hit.node_n});
        }
        else if(res.hit == true)
        {
            geo_scene->setSelectedNodes(std::vector<int>{});
            geo_scene->setBrushSelection(std::vector<int>{res.brush_n}); 
        }
        else
        {
            geo_scene->setBrushSelection(std::vector<int>{});
            geo_scene->setSelectedNodes(std::vector<int>{});
        }

    }
    else if(this->bShowGeometry && this->view_style==PANEL3D_VIEW_RENDER)
    {
        core::vector3df hitvec;
        int selected_face = -1;

        click_brush_info res;
        if(get_click_face(clickx,clicky,res))
        {
            selected_face = res.face_n;
        }

        int hit_node_i = GetSceneNodeHit(clickx,clicky,true);

        this->geo_scene->setBrushSelection(std::vector<int>{});

        if(bShiftDown && geo_scene->getSelectedFaces().size() > 0)
        {
            if(res.hit == true)
            {
                geo_scene->setSelectedFaces_ShiftAdd(selected_face);
            }
        }
        else if(bShiftDown && geo_scene->getSelectedNodes().size() >0)
        {
            if(hit_node_i != -1)
            {
                geo_scene->setSelectedNodes_ShiftAdd(hit_node_i);
            }
        }
        else if(hit_node_i != -1)
        {
            geo_scene->setSelectedFaces(std::vector<int>{});
            geo_scene->setSelectedNodes(std::vector<int>{hit_node_i});
        }
        else if(res.hit == true)
        {
            geo_scene->setSelectedNodes(std::vector<int>{});
            geo_scene->setSelectedFaces(std::vector<int>{selected_face});
        }
        else
        {
            geo_scene->setSelectedFaces(std::vector<int>{});
            geo_scene->setSelectedNodes(std::vector<int>{});

        }
    }
    else if(this->bShowGeometry && this->view_style==PANEL3D_VIEW_LOOPS)
    {
        int hit_node_i = GetSceneNodeHit(clickx,clicky,false);

        geo_scene->setSelectedFaces(std::vector<int>{});
        geo_scene->setBrushSelection(std::vector<int>{});

        if(bShiftDown && geo_scene->getSelectedNodes().size() >0)
        {
            if(hit_node_i != -1)
            {
                geo_scene->setSelectedNodes_ShiftAdd(hit_node_i);
            }
        }
        else if(hit_node_i != -1)
        {
            geo_scene->setSelectedNodes(std::vector<int>{hit_node_i});
        }
        else
            geo_scene->setSelectedNodes(std::vector<int>{});
    }
    else if(this->bShowGeometry)
    {
        this->geo_scene->setBrushSelection(std::vector<int>{});
        this->geo_scene->setSelectedNodes(std::vector<int>{});
        this->geo_scene->setSelectedFaces(std::vector<int>{});
    }

    if(old_sel_faces == geo_scene->getSelectedFaces() &&
        old_sel_nodes == geo_scene->getSelectedNodes() &&
        old_sel_brushes == geo_scene->getBrushSelection())
    {
        return;
    }

    geo_scene->selectionChanged();

}

void TestPanel_3D::ClickAddLight()
{
    int selected_face = -1;
    int selected_poly = -1;
    if(geo_scene && view_style == PANEL3D_VIEW_RENDER)
    {
        click_brush_info res;
        if(get_click_face(clickx,clicky,res))
        {
            selected_face = res.face_n;
            selected_poly = res.brush_n;

            core::vector3df N = geo_scene->get_total_geometry()->faces[selected_face].m_normal;

            geo_scene->addSceneLight(res.hitvec+ N*32);
        }
    }
}


void TestPanel_3D::ClickAddNode()
{
    int selected_face = -1;
    int selected_poly = -1;
    if(geo_scene && view_style == PANEL3D_VIEW_RENDER)
    {
        click_brush_info res;
        if(get_click_face(clickx,clicky,res))
        {
            selected_face = res.face_n;
            selected_poly = res.brush_n;

            core::vector3df N = geo_scene->get_total_geometry()->faces[selected_face].m_normal;

            geo_scene->addSceneSelectedSceneNodeType(res.hitvec+ N*32);
        }
    }
}

void TestPanel_3D::right_click(core::vector2di pos)
{
    gui::IGUIElement* root = Environment->getRootGUIElement();
    gui::IGUIElement* e = root->getElementFromId(GUI_ID_DIALOGUE_ROOT_WINDOW);

    if(geo_scene && view_style == PANEL3D_VIEW_RENDER && geo_scene->getSelectedFaces().size()>0)
    {
        gui::IGUIContextMenu* menu = Environment->addContextMenu(core::rect<s32>(pos,core::vector2di(256,256)),0,-1);
        menu->addItem(L"Use Current Texture",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_SET_TEXTURE,true,false,false,false);
        menu->addItem(L"Choose Texture",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_CHOOSE_TEXTURE,true,false,false,false);
        menu->addItem(L"Align Texture",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADJUST_TEXTURE,true,false,false,false);
        menu->addSeparator();
        menu->addItem(L"Set Material Group",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_MATERIAL_GROUP,true,false,false,false);
        menu->addSeparator();
        menu->addItem(L"Add Light",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADD_LIGHT,true,false,false,false);

        reflect::TypeDescriptor_Struct* node_type = geo_scene->getChooseNodeType();

        if(node_type)
        {
            std::wstring txt0(L"Add ");
            std::wstring txt(node_type->alias,node_type->alias+strlen(node_type->alias));
            txt0 += txt;
            menu->addItem(txt0.c_str(),GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADD_NODE,true,false,false,false);
        }
        else
        {
             menu->addItem(L"(No node type selected)",-1,true,false,false,false);
        }

        ContextMenuOwner = this;
    }
    else if(geo_scene && geo_scene->getBrushSelection().size()>0)
    {
        for(int p_i : geo_scene->getBrushSelection())
        {
            if(click_hits_poly(&geo_scene->elements[p_i].brush,core::vector2di(clickx,clicky)))
            {
                for(int v_i=0;v_i<geo_scene->elements[p_i].brush.vertices.size();v_i++)
                {
                    core::vector2di coords;
                    GetScreenCoords(geo_scene->elements[p_i].brush.vertices[v_i].V,coords);
                    if(core::vector2di(clickx,clicky).getDistanceFrom(coords)<4)
                    {
                        geo_scene->selected_brush_vertex_editing =p_i;
                        geo_scene->elements[p_i].selected_vertex=v_i;
                    }
                }
            }
        }
    }
    else if(geo_scene  && geo_scene->getSelectedNodes().size()>0)
    {
        gui::IGUIContextMenu* menu = Environment->addContextMenu(core::rect<s32>(pos,core::vector2di(256,256)),0,-1);

        std::vector<reflect::TypeDescriptor_Struct*> typeDescriptors = Node_Properties_Base::GetTypeDescriptors(geo_scene);

        if(typeDescriptors.size()>0)
        {
            int n = typeDescriptors.size()-1;
            std::wstring txt(typeDescriptors[n]->alias,typeDescriptors[n]->alias+strlen(typeDescriptors[n]->alias));
            std::wstring txt0(L"delete ");
            txt0 += txt;
            txt0 += std::wstring(L"(s)");
            txt += std::wstring(L" properties");

            menu->addItem(txt.c_str(),GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_NODE_PROPERTIES,true,false,false,false);
            menu->addSeparator();
            menu->addItem(txt0.c_str(),GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_DELETE_NODE,true,false,false,false);
            ContextMenuOwner = this;
        }
    }
    else
    {
        gui::IGUIContextMenu* menu = Environment->addContextMenu(core::rect<s32>(pos,core::vector2di(256,256)),0,-1);

        menu->addItem(L"View  ",-1,true,true,false,false);
        menu->addItem(L"Grid  ",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_GRID_TOGGLE,true,false,true,true);
        menu->addItem(L"Fullscreen ",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_FULLSCREEN_TOGGLE,true,false,true,true);
        menu->addItem(L"Build Lighting ",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_TEST,true,false,false,true);

        menu->setItemChecked(1,this->bShowGrid);
        menu->setItemChecked(2,this->bFullscreen);

        gui::IGUIContextMenu* submenu;
        submenu = menu->getSubMenu(0);
        submenu->addItem(L"Brushes",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_BRUSHES,true,false,true,true);
        submenu->addItem(L"Geometry ",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_GEOMETRY,true,false,true,true);
        submenu->addSeparator();
        submenu->addItem(L"Loops",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_LOOPS,true,false,true,true);
        submenu->addItem(L"Triangles",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_TRIANGLES,true,false,true,true);
        submenu->addItem(L"Edit Mesh",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_RENDER,true,false,true,true);
        submenu->addItem(L"Final Mesh",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_RENDER_FINAL,true,false,true,true);
        submenu->addSeparator();
        submenu->addItem(L"Unlit",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_UNLIT,true,false,true,true);
        submenu->addItem(L"Dyanmic Light",GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_DYNAMIC_LIGHT,true,false,true,true);

        submenu->setItemChecked(0,this->bShowBrushes);
        submenu->setItemChecked(1,this->bShowGeometry);
        submenu->setItemChecked(3,this->view_style==PANEL3D_VIEW_LOOPS);
        submenu->setItemChecked(4,this->view_style==PANEL3D_VIEW_TRIANGLES);
        submenu->setItemChecked(5,this->view_style==PANEL3D_VIEW_RENDER);
        submenu->setItemChecked(6,this->view_style==PANEL3D_VIEW_RENDER_FINAL);

        if(this->view_style==PANEL3D_VIEW_RENDER || this->view_style==PANEL3D_VIEW_RENDER_FINAL)
        {
            submenu->setItemEnabled(8,true);
            submenu->setItemEnabled(9,true);
            submenu->setItemChecked(8,!this->bDynamicLight);
            submenu->setItemChecked(9,this->bDynamicLight);
        }
        else
        {
            submenu->setItemEnabled(8,false);
            submenu->setItemEnabled(9,false);
        }

        ContextMenuOwner = this;
    }
}

void TestPanel_3D::TestLM()
{
    //std::cout << "Test LM\n";
   // if (geo_scene->getSelectedFaces().size() > 0)
    {
        /*

        core::vector3df pos;
        core::vector3df dir = vector3df(100,0,0);

        for (int i = 0; i < geo_scene->getSceneNodes().size(); i++)
        {
            reflect::TypeDescriptor_Struct* td = geo_scene->getSceneNodes()[i]->GetDynamicReflection();

            std::cout << td->name << "\n";

            if (strcmp(td->name, "Reflected_LightSceneNode") == 0)
            {
                std::cout << "found one light!\n";
                pos = geo_scene->getSceneNodes()[i]->getPosition();

                core::vector3df rot = geo_scene->getSceneNodes()[i]->getRotation();
                core::matrix4 R;

                R.setRotationDegrees(rot);

                core::vector3df z = core::vector3df(0, 0, 1);
                R.rotateVect(z);
                dir = z;
                break;
            }
        }
        
        GenLightMaps* LM = LightMaps_Tool::getLightmaps();
        //std::cout << "start lightmaps\n";
        LM->init(pos,dir);
        //LM->init(getCamera()->getPosition(), getCamera()->getRotation());
        int f_j = 0;// geo_scene->getSelectedFaces()[0];

       //LM->render(1);
        //std::cout << "end lightmaps \n";
        
        for (int i = 0; i < geo_scene->get_total_geometry()->faces.size(); i++)
        {
            if(geo_scene->get_total_geometry()->faces[i].loops.size() > 0)
                LM->render(i);
        }
        //std::cout << "....\n";
        */
    }
}

void TestPanel_3D::OnMenuItemSelected(IGUIContextMenu* menu)
{
    s32 id = menu->getItemCommandId(menu->getSelectedItem());

    scene::CMeshSceneNode* mesh_node = this->geo_scene->getMeshNode();

    //BVH_structure<poly_edge> test_bvh(geo_scene->get_total_geometry());

    switch(id)
    {
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_GRID_TOGGLE:
        this->toggle_grid();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_BRUSHES:
        this->toggle_showBrushes();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_GEOMETRY:
        this->toggle_showGeometry();
        if(this->bShowGeometry)
            this->SetViewStyle(this->view_style);
        else if(mesh_node)
            mesh_node->setVisible(false);
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_LOOPS:
        this->SetViewStyle(PANEL3D_VIEW_LOOPS);
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_TRIANGLES:
        this->SetViewStyle(PANEL3D_VIEW_TRIANGLES);
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_RENDER:
        this->SetViewStyle(PANEL3D_VIEW_RENDER);
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_RENDER_FINAL:
        this->SetViewStyle(PANEL3D_VIEW_RENDER_FINAL);
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_UNLIT:
        if(this->bDynamicLight==true)
        {
            this->bDynamicLight = false;
            this->SetViewStyle(this->view_style);
        }
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_DYNAMIC_LIGHT:
        if(this->bDynamicLight==false)
        {
            this->bDynamicLight = true;
            this->SetViewStyle(this->view_style);
        }
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_SET_TEXTURE:
        if(this->geo_scene)
            this->geo_scene->TextureToSelectedFaces();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADJUST_TEXTURE:
        Texture_Adjust_Tool::show();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_MATERIAL_GROUP:
        Material_Groups_Tool::show();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_CHOOSE_TEXTURE:
        TexturePicker_Tool::show();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADD_LIGHT:
        ClickAddLight();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADD_NODE:
        ClickAddNode();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_NODE_PROPERTIES:
        NodeProperties_Tool::show();
        //NodeProperties_Tool2::show();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_DELETE_NODE:
        geo_scene->deleteSelectedNodes();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_TEST:
        //bTest = !bTest;
        //if(bTest)
       // TestLM();
        
        //test_bvh.build(geo_scene->get_total_geometry()->edges.data(), geo_scene->get_total_geometry()->edges.size());

        graph.lines.clear();
        graph2.lines.clear();
        graph3.lines.clear();
        graph.points.clear();
        graph2.points.clear();
        graph3.points.clear();

        geo_scene->drawGraph(graph);

       // test_bvh.addDrawLines(graph);
        //    RenderHitTexture();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_FULLSCREEN_TOGGLE:
        this->bFullscreen = !this->bFullscreen;
        ((CameraQuad*)this->getParent())->SetFullscreen(this->bFullscreen,this);
        break;
    default:
        break;
    }
}

void TestPanel_3D::SetMeshNodesVisible()
{
    if(bShowGeometry && geo_scene->getMeshNode())
    {
        switch(this->view_style)
        {
            case PANEL3D_VIEW_RENDER:
                geo_scene->getMeshNode()->setWireFrame(false);
                geo_scene->getMeshNode()->setVisible(true);
            break;
            case PANEL3D_VIEW_RENDER_FINAL:
                geo_scene->getMeshNode()->setWireFrame(false);
                geo_scene->getMeshNode()->setVisible(true);
            break;
            case PANEL3D_VIEW_TRIANGLES:
                geo_scene->getMeshNode()->setWireFrame(true);
                geo_scene->getMeshNode()->setVisible(true);
            break;
            case PANEL3D_VIEW_LOOPS:
                geo_scene->getMeshNode()->setVisible(false);
            break;
        }

    }
    else
    {
        geo_scene->getMeshNode()->setVisible(false);
        //geo_scene->getFinalMeshNode()->setVisible(false);
    }

}


void TestPanel_3D::SetViewStyle(s32 vtype)
{
    this->view_style=vtype;


    scene::CMeshSceneNode* mesh_node;
    if(this->override_mesh_node==NULL)
        mesh_node = this->geo_scene->getMeshNode();
    else mesh_node = this->override_mesh_node;

    switch(vtype)
    {
    case PANEL3D_VIEW_LOOPS:
        {
        //std::cout << "loops\n";
        bShowGeometry=true;

        geo_scene->drawGraph(graph);

        if(!mesh_node)
        {
            geo_scene->buildSceneGraph(false,true,false);
            mesh_node = geo_scene->getMeshNode();
        }

        if(mesh_node)
            mesh_node->setVisible(false);

        if(geo_scene->getSelectedFaces().size()>0)
        {
            geo_scene->setSelectedFaces(std::vector<int>{});
        }
        if(this->geo_scene->get_total_geometry())
            {/*
            graph.lines.clear();
            graph2.lines.clear();
            graph3.lines.clear();
            graph.points.clear();
            graph2.points.clear();
            graph3.points.clear();
            
            if (LightMaps_Tool::getLightmaps())
            {
                LightMaps_Tool::getLightmaps()->addDrawLines(graph);
            }*/
            /*
            {
                if (this->total_geometry)
                    addDrawLines(*this->total_geometry, graph, graph2, graph3);
                else
                    addDrawLines(*this->geo_scene->get_total_geometry(), graph, graph2, graph2);
            }*/
            
            }
        }
        break;
    case PANEL3D_VIEW_TRIANGLES:
        bShowGeometry=true;
        //std::cout << "triangles\n";
        geo_scene->buildSceneGraph(false,false,false);
        mesh_node = geo_scene->getMeshNode();

        if(geo_scene->getSelectedFaces().size()>0)
        {
            geo_scene->setSelectedFaces(std::vector<int>{});
        }
        if(mesh_node)
        {
            mesh_node->setVisible(true);
            mesh_node->setWireFrame(true);
        }
        break;
    case PANEL3D_VIEW_RENDER:
        //std::cout << "edit\n";
        bShowGeometry=true;

        geo_scene->buildSceneGraph(false,true,this->bDynamicLight,false);

        mesh_node = geo_scene->getMeshNode();

        if(mesh_node)
        {
            mesh_node->setVisible(true);
            mesh_node->setWireFrame(false);
        }
        break;
    case PANEL3D_VIEW_RENDER_FINAL:
        //std::cout << "final\n";
        //

        if (!mesh_node)
        {
            geo_scene->buildSceneGraph(false, true, false);
            mesh_node = geo_scene->getMeshNode();
        }

        if (mesh_node)
            mesh_node->setVisible(false);

        if (geo_scene->getSelectedFaces().size() > 0)
        {
            geo_scene->setSelectedFaces(std::vector<int>{});
        }
        if (this->geo_scene->get_total_geometry())
        {
            graph.lines.clear();
            graph2.lines.clear();
            graph3.lines.clear();
            graph.points.clear();
            graph2.points.clear();
            graph3.points.clear();

            if (this->total_geometry)
                addDrawLines(*this->total_geometry, graph, graph2, graph3);
            else
                addDrawLines(*this->geo_scene->get_total_geometry(), graph, graph2, graph2);

        }
       //

        geo_scene->buildSceneGraph(true,false, this->bDynamicLight,false);
        mesh_node = geo_scene->getMeshNode();

        bShowGeometry=true;

        if(mesh_node)
        {
            mesh_node->setVisible(true);
            mesh_node->setWireFrame(false);
        }
        
        break;
    }
}


void TestPanel_3D::render()
{

    driver->setRenderTarget(getImage(), true, true, video::SColor(255,16,16,16));
    smgr->setActiveCamera(getCamera());

    getCamera()->render();

    smgr->drawAll();

    video::SMaterial someMaterial;
    someMaterial.Lighting = false;
    someMaterial.Thickness = 1.0;
    someMaterial.MaterialType = video::EMT_SOLID;

    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(someMaterial);

    //if(bShowGrid)
    //    this->drawGrid(driver,someMaterial);

    if(this->geo_scene)
    {
        if(bShowBrushes)
        {
            for(geo_element geo : this->geo_scene->elements)
            {
                if(!geo.bSelected)
                geo.draw_brush(driver,someMaterial);
            }
            for(geo_element geo : this->geo_scene->elements)
            {
                if(geo.bSelected)
                    geo.draw_brush(driver,someMaterial);
            }
            for(int e_i=0;e_i<this->geo_scene->elements.size();e_i++)
            {
                geo_element* geo = &this->geo_scene->elements[e_i];
                if(geo->bSelected)
                {

                    core::vector2di coords;
                    for(int i=0;i<geo->brush.vertices.size();i++)
                    {
                    GetScreenCoords(geo->brush.vertices[i].V,coords);
                    coords.X-=4;
                    coords.Y-=4;
                    if(geo_scene->selected_brush_vertex_editing ==e_i && geo->selected_vertex==i)
                        {
                        if(geo->type==GEO_ADD)
                            driver->draw2DImage(med_circle_tex_add_selected,coords,core::rect<int>(0,0,8,8),0,video::SColor(255,255,255,255),true);
                        else if(geo->type==GEO_SUBTRACT)
                            driver->draw2DImage(med_circle_tex_sub_selected,coords,core::rect<int>(0,0,8,8),0,video::SColor(255,255,255,255),true);
                         else if(geo->type==GEO_RED)
                            driver->draw2DImage(med_circle_tex_red_selected,coords,core::rect<int>(0,0,8,8),0,video::SColor(255,255,255,255),true);
                        }
                    else
                        {
                        if(geo->type==GEO_ADD)
                            driver->draw2DImage(small_circle_tex_add_selected,coords,core::rect<int>(0,0,8,8),0,video::SColor(255,255,255,255),true);
                        else if(geo->type==GEO_SUBTRACT)
                            driver->draw2DImage(small_circle_tex_sub_selected,coords,core::rect<int>(0,0,8,8),0,video::SColor(255,255,255,255),true);
                         else if(geo->type==GEO_RED)
                            driver->draw2DImage(small_circle_tex_red_selected,coords,core::rect<int>(0,0,8,8),0,video::SColor(255,255,255,255),true);
                        }
                    }
                }
            }
        }
        else
        {   //always draw the red brush
            geo_scene->elements[0].draw_brush(driver,someMaterial);

            if(geo_scene->elements[0].bSelected)
            {

                core::vector2di coords;
                for(int i=0;i<geo_scene->elements[0].brush.vertices.size();i++)
                {
                GetScreenCoords(geo_scene->elements[0].brush.vertices[i].V,coords);
                coords.X-=4;
                coords.Y-=4;
                if(geo_scene->selected_brush_vertex_editing ==0 && geo_scene->elements[0].selected_vertex==i)
                    {
                        driver->draw2DImage(med_circle_tex_red_selected,coords,core::rect<int>(0,0,8,8),0,video::SColor(255,255,255,255),true);
                    }
                else
                    {
                        driver->draw2DImage(small_circle_tex_red_selected,coords,core::rect<int>(0,0,8,8),0,video::SColor(255,255,255,255),true);
                    }
                }
            }
        }
    }

    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(someMaterial);
/*
    if(geo_scene->getSelectedNodes().size() > 0)
    {
        for(int i : geo_scene->getSelectedNodes())
        {
            if(geo_scene->getSceneNodes()[i]->bShowEditorArrow())
            {
                draw_arrow(geo_scene->getSceneNodes()[i]->getPosition(),geo_scene->getSceneNodes()[i]->getRotation());
            }
        }
    }*/

    if(bShowGeometry && view_style == PANEL3D_VIEW_LOOPS)
    {
        for(core::line3df aline: graph2.lines)
        {
        driver->draw3DLine(aline.start,aline.end,video::SColor(255,96,128,96));
        }
        for(core::line3df aline: graph3.lines)
        {
        driver->draw3DLine(aline.start,aline.end,video::SColor(255,32,0,150));
        }
        for(core::line3df aline: graph.lines)
        {
        driver->draw3DLine(aline.start,aline.end,video::SColor(255,255,0,255));
        }
        for(core::vector3df v: graph.points)
        {
            int len=4;
            driver->draw3DLine(v+core::vector3df(len,0,0),v-core::vector3df(len,0,0),video::SColor(128,255,0,255));
            driver->draw3DLine(v+core::vector3df(0,len,0),v-core::vector3df(0,len,0),video::SColor(128,255,0,255));
            driver->draw3DLine(v+core::vector3df(0,0,len),v-core::vector3df(0,0,len),video::SColor(128,255,0,255));
        }

        for (core::vector3df v : graph2.points)
        {
            int len = 4;
            driver->draw3DLine(v + core::vector3df(len, 0, 0), v - core::vector3df(len, 0, 0), video::SColor(255, 96, 128, 96));
            driver->draw3DLine(v + core::vector3df(0, len, 0), v - core::vector3df(0, len, 0), video::SColor(255, 96, 128, 96));
            driver->draw3DLine(v + core::vector3df(0, 0, len), v - core::vector3df(0, 0, len), video::SColor(255, 96, 128, 96));
        }
    }

    driver->setRenderTarget(NULL, true, true, video::SColor(0,0,0,0));

}


bool TestPanel_3D::GetScreenCoords(core::vector3df V, core::vector2di &out_coords)
{
    return Get3DScreenCoords(V, out_coords);
}

bool TestPanel_3D::Get3DScreenCoords(core::vector3df V, core::vector2di &out_coords)
{
    const scene::SViewFrustum* frustum = this->getCamera()->getViewFrustum();
    const core::vector3df cameraPosition =this->getCamera()->getAbsolutePosition();

    vector3df vNearLeftDown = frustum->getNearLeftDown();
    vector3df vNearRightDown = frustum->getNearRightDown();
    vector3df vNearLeftUp = frustum->getNearLeftUp();
    vector3df vNearRightUp = frustum->getNearRightUp();

    vector3df ray = V - cameraPosition;

    core::plane3df aplane(vNearLeftDown,vNearLeftUp,vNearRightDown);
    core::vector3df vIntersect;

    if(aplane.getIntersectionWithLimitedLine(cameraPosition,V,vIntersect))
    {
        core::vector3df v_X = (vNearRightDown - vNearLeftDown);
        v_X.normalize();
        f32 t_X = v_X.dotProduct(vIntersect-vNearLeftDown);
        t_X /= (vNearRightDown - vNearLeftDown).getLength();

        core::vector3df v_Y = (vNearLeftUp - vNearLeftDown);
        v_Y.normalize();
        f32 t_Y = v_Y.dotProduct(vIntersect-vNearLeftDown);
        t_Y /= (vNearLeftUp - vNearLeftDown).getLength();

        out_coords.X = core::round32(t_X*this->Texture->getOriginalSize().Width);
        out_coords.Y = core::round32((1-t_Y)*this->Texture->getOriginalSize().Height);
        return true;
    }
    return false;
}

