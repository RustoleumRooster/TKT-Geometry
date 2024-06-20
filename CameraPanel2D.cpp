
#include "CameraPanel.h"
#include <irrlicht.h>
#include <iostream>
#include "edit_env.h"
#include "CMeshBuffer.h"
#include "utils.h"
#include "node_properties.h"

extern IrrlichtDevice* device;
using namespace irr;
using namespace core;
using namespace gui;

extern ViewPanel* ContextMenuOwner;

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


//=============================================================================================
// Camera Panel 2D
//
//

TestPanel_2D::TestPanel_2D(IGUIEnvironment* environment, video::IVideoDriver* driver, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
: TestPanel(environment, driver, parent, id, rectangle)
{
	bShowGeometry=false;
	bShowBrushes=true;
}

void TestPanel_2D::setImage(video::ITexture* image)
{
    if (image == Texture)
		return;

	if (Texture)
		Texture->drop();

	Texture = image;

	if (Texture)
    {
		Texture->grab();
		viewSize=Texture->getOriginalSize()*4;
    }
}

void TestPanel_2D::resize(core::dimension2d<u32> new_size)
{
    viewSize = this->Texture->getOriginalSize() * 4;

    core::matrix4 M;
    M.buildProjectionMatrixOrthoLH(this->Texture->getOriginalSize().Width * 4, this->Texture->getOriginalSize().Height * 4, 0, 10000);
    getCamera()->setProjectionMatrix(M, true);
}

scene::ICameraSceneNode* TestPanel_2D::getCamera()
{
    if(this->camera == NULL)
    {
        if(this->smgr)
        {
            core::matrix4 M;
            M.buildProjectionMatrixOrthoLH(this->Texture->getOriginalSize().Width*4,this->Texture->getOriginalSize().Height*4,0,10000);
            this->camera = smgr->addCameraSceneNode(0,core::vector3df(0,1000,0),core::vector3df(0,0,0),-1,false);

            this->camera->setProjectionMatrix(M,true);
        }
    }
    return this->camera;
}

f32 TestPanel_2D::getViewScaling()
{
    return ((f32)this->viewSize.Width / this->Texture->getOriginalSize().Width);
}


void TestPanel_2D::setAxis(int axis)
{
    if(axis==CAMERA_Y_AXIS)
    {
        this->getCamera()->setPosition(core::vector3df(0,5000,0));
        this->getCamera()->setTarget(core::vector3df(0,0,0));
        this->vHorizontal=core::vector3df(0,0,1);
        this->vVertical=core::vector3df(1,0,0);
        this->vAxis=core::vector3df(0,1,0);
    }
    else if(axis==CAMERA_X_AXIS)
    {
        this->getCamera()->setPosition(core::vector3df(5000,0,0));
        this->getCamera()->setTarget(core::vector3df(0,0,0));
        this->vHorizontal=core::vector3df(0,0,-1);
        this->vVertical=core::vector3df(0,1,0);
        this->vAxis=core::vector3df(1,0,0);
    }
    else if(axis==CAMERA_Z_AXIS)
    {
        this->getCamera()->setPosition(core::vector3df(0,0,5000));
        this->getCamera()->setTarget(core::vector3df(0,0,0));
        this->vHorizontal=core::vector3df(1,0,0);
        this->vVertical=core::vector3df(0,1,0);
        this->vAxis=core::vector3df(0,0,1);
    }
    this->m_axis=axis;
}


bool TestPanel_2D::OnEvent(const SEvent& event)
{
	{
		switch(event.EventType)

		{
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

                        }
                        break;
                    case EGET_ELEMENT_LEFT:
                        {

                        }
                        break;
                    case EGET_ELEMENT_FOCUS_LOST:
                        {

                            this->bMouseDown=false;
                            this->rMouseDown=false;
                            this->bDragBrush=false;
                            this->bZoomCamera=false;
                            this->bDragCamera=false;
                        }
                        break;
/*
                    case EGET_ELEMENT_FOCUS_LOST:
                       // if (event.GUIEvent.Caller == this)// && !isMyChild(event.GUIEvent.Element) )
                        {
                           // std::cout<<"lost focus\n";

                            if (event.GUIEvent.Caller == this && isMyChild(event.GUIEvent.Element))
                            {
                                  //  Environment->setFocus(event.GUIEvent.Element);
                                  //  std::cout<<"me\n";
                                  //  return true;
                            }

                            //return false;
                        }
                        break;
                    case EGET_ELEMENT_FOCUSED:
                        if (event.GUIEvent.Caller == this )
                        {
                            //std::cout<<"focus\n";
                            //return true;
                            //return false;
                        }
                        break;
*/
                    default:
                        break;
                }
                break;
		    case EET_MOUSE_INPUT_EVENT:

                mousex = event.MouseInput.X;
                mousey = event.MouseInput.Y;
                bShiftDown = event.MouseInput.Shift;
                bCtrlDown = event.MouseInput.Control;

                switch(event.MouseInput.Event)
                {
                    case EMIE_LMOUSE_LEFT_UP:
                        {
                            if(bZoomCamera)
                            {
                                bZoomCamera=false;
                            }

                            if(clickx==mousex && clicky==mousey)
                                left_click(core::vector2di(event.MouseInput.X,event.MouseInput.Y));

                            bMouseDown=false;
                            bDragCamera=false;
                            bDragBrush=false;
                            bDragNode = false;
                            bDragVertex = false;

                        }
                        return true;
                    case EMIE_LMOUSE_PRESSED_DOWN:
                        {

                            clickx=mousex;
                            clicky=mousey;

                            //Initiate Drag
                            if(!bMouseDown && !rMouseDown)
                            {
                            vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                            vDragCameraInitialTarget = this->getCamera()->getTarget();

                            bDragCamera = true;
                            bRotateCamera = false;
                            bZoomCamera = false;


                            bMouseDown=true;
                            }
                            else if(bMouseDown == false)
                            {
                                clickx=mousex;
                                clicky=mousey;
                                bMouseDown = true;
                            }
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
                            }

                            if(Environment->getFocus()!=this)
                                return false;
                                */
                            if(!rMouseDown)
                            {
                                clickx=mousex;
                                clicky=mousey;
                                rMouseDown=true;
                            }
                            if(geo_scene->getBrushSelection().size()>0 && bShiftDown==false && bCtrlDown==false)
                            {
                                vDragBrushOriginalPosition = this->geo_scene->getSelectedVertex();//elements[geo_scene->getSelection()[0]].brush.vertices[0].V;
                                bDragBrush=true;
                            }
                            else if(geo_scene->getBrushSelection().size()>0 && bShiftDown==true)
                            {
                                vRotateBrushOrigin=geo_scene->getSelectedVertex();
                                bRotateBrush=true;

                                core::vector2di coords;
                                GetScreenCoords(vRotateBrushOrigin,coords);
                                core::vector2di line = vector2di(mousex,mousey)-coords;
                                RotateBrushInitialAngle = line.getAngleTrig();
                            }
                            else if(geo_scene->getSelectedNodes().size()==1 && bShiftDown==true)
                            {
                                vRotateBrushOrigin=geo_scene->getSelectedSceneNode(0)->getPosition();
                                bRotateNode=true;

                                core::vector2di coords;
                                GetScreenCoords(vRotateBrushOrigin,coords);
                                core::vector2di line = vector2di(mousex,mousey)-coords;
                                RotateBrushInitialAngle = line.getAngleTrig();
                            }
                            else if(geo_scene->getBrushSelection().size()>0 && bCtrlDown==true &&
                                    geo_scene->getBrushSelection().size()==1)
                            {
                                vDragVertexOriginalPosition = this->geo_scene->getSelectedVertex();
                                bDragVertex=true;
                            }
                            else if(geo_scene->getSelectedNodes().size()>0)
                            {
                                vDragBrushOriginalPosition = this->geo_scene->getSelectedSceneNode(0)->getPosition();
                                bDragNode=true;
                            }
                        }
                        return true;
                    case EMIE_RMOUSE_LEFT_UP:
                        {
                            if (bDragNode)
                            {
                                bDragNode = false;
                                geo_scene->selectionChanged();
                            }
                            if (bRotateNode)
                            {
                                bRotateNode = false;
                                geo_scene->selectionChanged();
                            }

                            /* TODO
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                bZoomCamera=false;
                                bDragVertex=false;  
                               // Environment->removeFocus(this);
                            }*/

                            if (bDragBrush)
                            {
                                bDragBrush = false;
                            }
                            if(bDragVertex)
                            {
                                bDragVertex=false;
                            }
                            if(bRotateBrush)
                            {
                                core::vector2di coords;
                                GetScreenCoords(vRotateBrushOrigin,coords);
                                core::vector2di line = vector2di(mousex,mousey)-coords;
                                f32 angle = line.getAngleTrig();

                                f32 delta = angle -RotateBrushInitialAngle;
                                delta/=this->rotate_snap;
                                delta = core::round32(delta);
                                delta*=this->rotate_snap;

                                if(abs(delta)>0.01)
                                {
                                    core::matrix4 M;
                                    core::matrix4 R;
                                    core::matrix4 iM;
                                    M.setTranslation(vRotateBrushOrigin*(-1));
                                    iM.setTranslation(vRotateBrushOrigin*(1));
                                    R.setRotationAxisRadians(3.141592653589*delta/180.0,this->vAxis);
                                    RotateBrushInitialAngle=angle;
                                    for(int i : this->geo_scene->getBrushSelection())
                                    {
                                        this->geo_scene->elements[i].brush.translate(M);
                                        this->geo_scene->elements[i].brush.rotate(R);
                                        this->geo_scene->elements[i].brush.translate(iM);
                                    }
                                }

                                bRotateBrush=false;
                            }
                            if (clickx == mousex && clicky == mousey)
                            {
                                if (this->m_viewPanel)
                                {
                                    vector2d<s32> clickpos = m_viewPanel->getClickPos();
                                    right_click(clickpos);
                                }
                                
                            }

                            rMouseDown=false;
                        }
                        return true;
                    case EMIE_MOUSE_MOVED:

                        if( bMouseDown == true && rMouseDown == true)
                        {
                            if(bZoomCamera==false)
                            {
                                vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                                vDragCameraInitialTarget = this->getCamera()->getTarget();
                                vDragCameraRay = this->getCamera()->getAbsolutePosition()-this->getCamera()->getTarget();
                                this->oldViewSize=this->viewSize;

                                bZoomCamera=true;
                                bDragCamera=false;
                            }
                            else
                            {

                                int ydif = mousey-clicky;
                                float zoom_f = 1.0-(0.01 * ydif);
                                zoom_f = fmax(zoom_f,0.01);

                                core::matrix4 M;
                                M.buildProjectionMatrixOrthoLH(oldViewSize.Width*(zoom_f),oldViewSize.Height*(zoom_f),10,10000);
                                this->camera->setProjectionMatrix(M,true);
                                this->viewSize.Width=oldViewSize.Width*(zoom_f);
                                this->viewSize.Height=oldViewSize.Height*(zoom_f);
                            }
                        }
                        else if(bDragCamera )
                        {
                            int ydif = mousey-clicky;
                            int xdif = mousex-clickx;
                            core::vector3df vMove = xdif*this->vHorizontal+ydif*this->vVertical;
                            vMove*=((f32)this->viewSize.Width / this->Texture->getOriginalSize().Width);
                            this->getCamera()->setPosition(vDragCameraInitialPosition+vMove);
                            this->getCamera()->setTarget(vDragCameraInitialTarget+vMove);
                            this->getCamera()->updateAbsolutePosition();
                        }
                        else if(bDragBrush)
                        {
                            //if(core::vector2di(clickx,clicky).getDistanceFrom(core::vector2di(mousex,mousey))>4)
                            {
                                int ydif = clicky-mousey;
                                int xdif = clickx-mousex;
                                core::vector3df V=vDragBrushOriginalPosition;
                                V+=this->vVertical*ydif*this->getViewScaling();
                                V+=this->vHorizontal*xdif*this->getViewScaling();
                                f32 X_i = grid_snap*core::round32(V.X/this->grid_snap);
                                f32 Y_i = grid_snap*core::round32(V.Y/this->grid_snap);
                                f32 Z_i = grid_snap*core::round32(V.Z/this->grid_snap);
                                V=core::vector3df(X_i,Y_i,Z_i);
                                core::matrix4 M;
                                int v0 = 0;//this->geo_scene->elements[this->selected_brush].selected_vertex;
                                //core::vector3df T = V-this->geo_scene->elements[geo_scene->getSelection()[0]].brush.vertices[v0].V;
                                core::vector3df T = V-this->geo_scene->getSelectedVertex();
                                if(T.getLength()>0.05)
                                {
                                    M.setTranslation(T);
                                    for(int i=0;i<geo_scene->getBrushSelection().size();i++)
                                        geo_scene->elements[geo_scene->getBrushSelection()[i]].brush.translate(M);
                                }
                            }
                        }
                        else if(bDragVertex)
                        {
                            int ydif = clicky-mousey;
                            int xdif = clickx-mousex;
                            core::vector3df V=vDragVertexOriginalPosition;
                            V+=this->vVertical*ydif*this->getViewScaling();
                            V+=this->vHorizontal*xdif*this->getViewScaling();
                            f32 X_i = grid_snap*core::round32(V.X/this->grid_snap);
                            f32 Y_i = grid_snap*core::round32(V.Y/this->grid_snap);
                            f32 Z_i = grid_snap*core::round32(V.Z/this->grid_snap);
                            V=core::vector3df(X_i,Y_i,Z_i);
                            core::vector3df T = V-this->geo_scene->getSelectedVertex();
                            if(T.getLength()>0.05)
                            {
                                int v_i = geo_scene->elements[geo_scene->getBrushSelection()[0]].selected_vertex;
                                geo_scene->elements[geo_scene->getBrushSelection()[0]].brush.vertices[v_i].V = V;
                                geo_scene->elements[geo_scene->getBrushSelection()[0]].brush.topology=TOP_UNDEF;
                            }
                        }
                        else if(bDragNode)
                        {
                            int ydif = clicky-mousey;
                            int xdif = clickx-mousex;
                            core::vector3df V=vDragBrushOriginalPosition;
                            V+=this->vVertical*ydif*this->getViewScaling();
                            V+=this->vHorizontal*xdif*this->getViewScaling();
                            f32 X_i = grid_snap*core::round32(V.X/this->grid_snap);
                            f32 Y_i = grid_snap*core::round32(V.Y/this->grid_snap);
                            f32 Z_i = grid_snap*core::round32(V.Z/this->grid_snap);
                            V=core::vector3df(X_i,Y_i,Z_i);
                            core::matrix4 M;
                            int v0 = 0;//this->geo_scene->elements[this->selected_brush].selected_vertex;
                            //core::vector3df T = V-this->geo_scene->elements[geo_scene->getSelection()[0]].brush.vertices[v0].V;
                            core::vector3df T = V-this->geo_scene->getSelectedSceneNode(0)->getPosition();
                            if(T.getLength()>0.05)
                            {
                                M.setTranslation(T);
                                //this->geo_scene->elements[this->selected_brush].brush.translate(M);
                                for(int i : geo_scene->getSelectedNodes())
                                {
                                    geo_scene->getSceneNodes()[i]->translate(M);
                                }
                            }
                        }
                        else if(bRotateBrush)
                        {
                            //TO DO
                        }
                        else if(bRotateNode)
                        {
                            core::vector2di coords;
                            GetScreenCoords(vRotateBrushOrigin,coords);
                            core::vector2di line = vector2di(mousex,mousey)-coords;
                            f32 angle = line.getAngleTrig();

                            f32 delta = angle -RotateBrushInitialAngle;

                            delta/=this->rotate_snap;
                            delta = core::round32(delta);
                            delta*=this->rotate_snap;

                            if(abs(delta)>0.01 && geo_scene->getSelectedNodes().size() > 0)
                            {
                                core::vector3df rot = geo_scene->getSelectedSceneNode(0)->getRotation();
                                core::matrix4 R;

                                R.setRotationDegrees(rot);

                                core::vector3df z = core::vector3df(0,0,1);
                                R.rotateVect(z);

                                R.setRotationAxisRadians(3.141592653589*(delta)/180.0,this->vAxis);
                                RotateBrushInitialAngle=angle;

                                R.rotateVect(z);

                                rot = z.getHorizontalAngle();
                                geo_scene->getSelectedSceneNode(0)->rotate(rot);
                            }
                        }
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
    return false;
}

void TestPanel_2D::left_click(core::vector2di pos)
{
    std::vector<int> old_sel_faces = geo_scene->getSelectedFaces();
    std::vector<int> old_sel_nodes = geo_scene->getSelectedNodes();
    std::vector<int> old_sel_brushes = geo_scene->getBrushSelection();

    if(click_hits_poly(&geo_scene->elements[0].brush,core::vector2di(clickx,clicky)))
    {
        geo_scene->setBrushSelection(std::vector<int>{0});
    }
    else if(this->bShowBrushes && !this->bShowGeometry)
    {

        click_brush_info res;
        get_click_brush(clickx,clicky,res);

        int hit_node_i = GetSceneNodeHit(clickx,clicky,false);

        if(bShiftDown && geo_scene->getBrushSelection().size() > 0)
        {
            if(res.hit == true)
            {
                geo_scene->setBrushSelection_ShiftAdd(res.brush_n);
            }
        }
        else if(bShiftDown && geo_scene->getSelectedNodes().size() >0)
        {
            if(hit_node_i !=-1)
            {
                geo_scene->setSelectedNodes_ShiftAdd(hit_node_i);
            }
        }
        else if(hit_node_i !=-1)
        {
            geo_scene->setSelectedNodes(std::vector<int>{hit_node_i});
            geo_scene->setBrushSelection(std::vector<int>{});
        }
        else if(res.hit == true)
        {
            geo_scene->setBrushSelection(std::vector<int>{res.brush_n});
            geo_scene->setSelectedNodes(std::vector<int>{});
        }
        else
        {
            geo_scene->setBrushSelection(std::vector<int>{});
            geo_scene->setSelectedNodes(std::vector<int>{});
        }

    }

    geo_scene->setSelectedFaces(std::vector<int>{});

    if (old_sel_faces == geo_scene->getSelectedFaces() &&
        old_sel_nodes == geo_scene->getSelectedNodes() &&
        old_sel_brushes == geo_scene->getBrushSelection())
    {
        return;
    }

    geo_scene->selectionChanged();
}

void TestPanel_2D::right_click(core::vector2di pos)
{
    if(geo_scene && geo_scene->getBrushSelection().size()>0)
    {
        bool bVertexClick=false;
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
                        bVertexClick=true;
                    }
                }
            }
        }
        if(!bVertexClick)
        {
            gui::IGUIContextMenu* menu = environment->addContextMenu(core::rect<s32>(pos,core::vector2di(256,256)),0,-1);
            menu->addItem(L"Delete Brushes",GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_DELETE_BRUSH,true,false,false,false);
            //menu->addItem(L"Make Red Brush",GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_MAKE_RED_BRUSH,true,false,false,false);

            ContextMenuOwner = this->m_viewPanel;
        }
    }
    else if(geo_scene && geo_scene->getSelectedNodes().size()>0)
    {
        gui::IGUIContextMenu* menu = environment->addContextMenu(core::rect<s32>(pos,core::vector2di(256,256)),0,-1);

        std::vector<reflect::TypeDescriptor_Struct*> typeDescriptors = Node_Properties_Base::GetTypeDescriptors(geo_scene);

        if(typeDescriptors.size()>0)
        {
            int n = typeDescriptors.size()-1;
            std::wstring txt(typeDescriptors[n]->alias,typeDescriptors[n]->alias+strlen(typeDescriptors[n]->alias));
            txt += std::wstring(L" properties");

        menu->addItem(txt.c_str(),GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_NODE_PROPERTIES,true,false,false,false);
        ContextMenuOwner = this->m_viewPanel;
        }
    }
    else
    {
        gui::IGUIContextMenu* menu = environment->addContextMenu(core::rect<s32>(pos,core::vector2di(256,256)),0,-1);
        menu->addItem(L"View  ",-1,true,true,false,false);
        menu->addItem(L"Grid  ",GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_GRID_TOGGLE,true,false,true,true);
        menu->setItemChecked(1,this->bShowGrid);

        gui::IGUIContextMenu* submenu;
        submenu = menu->getSubMenu(0);
        submenu->addItem(L"Brushes",GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_VIEW_BRUSHES,true,false,true,true);
        submenu->addItem(L"Geometry ",GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_VIEW_GEOMETRY,true,false,true,true);

        submenu->setItemChecked(0,this->bShowBrushes);
        submenu->setItemChecked(1,this->bShowGeometry);
        ContextMenuOwner = m_viewPanel;
    }
}

void TestPanel_2D::delete_selected_brushes()
{
    if(this->geo_scene && this->geo_scene->getBrushSelection().size() > 0 && this->geo_scene->getBrushSelection()[0] != 0)
    {
        this->geo_scene->delete_selected_brushes();
    }
}

void TestPanel_2D::OnMenuItemSelected(IGUIContextMenu* menu)
{
    s32 id = menu->getItemCommandId(menu->getSelectedItem());
    gui::IGUIEnvironment* env = device->getGUIEnvironment();
    gui::IGUIElement* root = env->getRootGUIElement();
    switch(id)
    {
    case GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_GRID_TOGGLE:
        this->toggle_grid();
        break;
    case GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_VIEW_BRUSHES:
        this->toggle_showBrushes();
        break;
    case GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_VIEW_GEOMETRY:
        this->toggle_showGeometry();
        break;
    case GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_DELETE_BRUSH:
        this->delete_selected_brushes();
        break;
    case GUI_ID_VIEWPORT_2D_RIGHTCLICK_MENU_ITEM_NODE_PROPERTIES:
        NodeProperties_Tool::show();
        break;
    default:
        break;
    }
}

bool TestPanel_2D::GetScreenCoords(core::vector3df V, core::vector2di &out_coords)
{
    return GetOrthoScreenCoords(V, out_coords);
}

bool TestPanel_2D::GetOrthoScreenCoords(core::vector3df V, core::vector2di &out_coords)
{
    const scene::SViewFrustum* frustum = this->getCamera()->getViewFrustum();
    const core::vector3df cameraPosition = this->getCamera()->getAbsolutePosition();

    vector3df vNearLeftDown = frustum->getNearLeftDown();
    vector3df vNearRightDown = frustum->getNearRightDown();
    vector3df vNearLeftUp = frustum->getNearLeftUp();
    vector3df vNearRightUp = frustum->getNearRightUp();

    vector3df ray = frustum->getNearLeftDown() - frustum->getFarLeftDown();

    core::plane3df aplane(vNearLeftDown,vNearLeftUp,vNearRightDown);
    core::vector3df vIntersect;

    if(aplane.getIntersectionWithLine(V,ray,vIntersect))
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

void TestPanel_2D::render()
{
    //if(isVisible()==false)
    //    return;

    driver->setRenderTarget(getImage(), true, true, video::SColor(255,16,16,16));
    smgr->setActiveCamera(getCamera());

    smgr->drawAll();

    getCamera()->render();

    video::SMaterial someMaterial;
    someMaterial.Lighting = false;
    someMaterial.Thickness = 1.0;
    someMaterial.MaterialType = video::EMT_SOLID;

    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(someMaterial);

    if(bShowGrid)
        this->drawGrid(driver,someMaterial);

    if(this->geo_scene)
    {
        if(bShowBrushes)
        {
            for(int e_i=0;e_i<this->geo_scene->elements.size();e_i++)
            {
                this->geo_scene->elements[e_i].draw_brush(driver,someMaterial);
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
                    if(geo_scene->selected_brush_vertex_editing == e_i && geo->selected_vertex == i)
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

        if(bShowGeometry)
            for(geo_element geo : this->geo_scene->elements)
                geo.draw_geometry(driver,someMaterial);
    }

    driver->setRenderTarget(0, true, true, video::SColor(0,0,0,0));
}


void TestPanel_2D::SetMeshNodesVisible()
{
    geo_scene->getMeshNode()->setVisible(false);
    //geo_scene->getFinalMeshNode()->setVisible(false);
}

void TestPanel_2D::drawGrid(video::IVideoDriver* driver, const video::SMaterial material)
{
    //driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    //driver->setMaterial(material);

    int far_value=this->getCamera()->getFarValue();
    far_value*=-1;

    if(m_axis==CAMERA_Y_AXIS)
    {
        int interval = this->grid_interval;
        while(viewSize.Width / interval > 36 || viewSize.Height / interval > 36)
        {
            interval = interval<<1;
        }
        int h_lines = viewSize.Width / interval;
        int v_lines = viewSize.Height / interval;
        core::vector3df vDownLeft = this->getCamera()->getAbsolutePosition();
        vDownLeft.Z-=viewSize.Width/2;
        vDownLeft.X-=viewSize.Height/2;
        int start_x = interval*((int)vDownLeft.Z/interval);
        int start_y = interval*((int)vDownLeft.X/interval);

        video::SColor col;
        for(int i=-1;i<h_lines+2;i++)
        {
            if(start_x+i*interval==0)
                col = video::SColor(255,128,128,128);
            else
                col = video::SColor(255,32,32,32);

            driver->draw3DLine(core::vector3df(vDownLeft.X,far_value,start_x+i*interval),core::vector3df(vDownLeft.X+viewSize.Height,far_value,start_x+i*interval), col);
        }

        for(int i=-1;i<v_lines+2;i++)
        {
            if(start_y+i*interval==0)
                col = video::SColor(255,128,128,128);
            else
                col = video::SColor(255,32,32,32);
            driver->draw3DLine(core::vector3df(start_y+i*interval,far_value,vDownLeft.Z),core::vector3df(start_y+i*interval,far_value,vDownLeft.Z+viewSize.Width), col);
        }
    }
    else if(m_axis==CAMERA_X_AXIS)
    {
        int interval = this->grid_interval;
        while(viewSize.Width / interval > 36 || viewSize.Height / interval > 36)
        {
            interval = interval<<1;
        }
        int h_lines = viewSize.Width / interval;
        int v_lines = viewSize.Height / interval;
        core::vector3df vDownLeft = this->getCamera()->getAbsolutePosition();
        vDownLeft.Z-=viewSize.Width/2;
        vDownLeft.Y-=viewSize.Height/2;
        int start_x = interval*((int)vDownLeft.Z/interval);
        int start_y = interval*((int)vDownLeft.Y/interval);

        video::SColor col;
        for(int i=-1;i<h_lines+2;i++)
        {
            if(start_x+i*interval==0)
                col = video::SColor(255,128,128,128);
            else
                col = video::SColor(255,32,32,32);
            driver->draw3DLine(core::vector3df(far_value,vDownLeft.Y,start_x+i*interval),core::vector3df(far_value,vDownLeft.Y+viewSize.Height,start_x+i*interval),col);
        }

        for(int i=-1;i<v_lines+2;i++)
        {
            if(start_y+i*interval==0)
                col = video::SColor(255,128,128,128);
            else
                col = video::SColor(255,32,32,32);
            driver->draw3DLine(core::vector3df(far_value,start_y+i*interval,vDownLeft.Z),core::vector3df(far_value,start_y+i*interval,vDownLeft.Z+viewSize.Width),col);
        }

    }
    else if(m_axis==CAMERA_Z_AXIS)
    {
        int interval = this->grid_interval;
        while(viewSize.Width / interval > 36 || viewSize.Height / interval > 36)
        {
            interval = interval<<1;
        }
        int h_lines = viewSize.Width / interval;
        int v_lines = viewSize.Height / interval;
        core::vector3df vDownLeft = this->getCamera()->getAbsolutePosition();
        vDownLeft.X-=viewSize.Width/2;
        vDownLeft.Y-=viewSize.Height/2;
        int start_x = interval*((int)vDownLeft.X/interval);
        int start_y = interval*((int)vDownLeft.Y/interval);

        video::SColor col;

        for(int i=-1;i<h_lines+2;i++)
        {
            if(start_x+i*interval==0)
                col = video::SColor(255,128,128,128);
            else
                col = video::SColor(255,32,32,32);
            driver->draw3DLine(core::vector3df(start_x+i*interval,vDownLeft.Y,far_value),core::vector3df(start_x+i*interval,vDownLeft.Y+viewSize.Height,far_value),col);
        }

        for(int i=-1;i<v_lines+2;i++)
        {
            if(start_y+i*interval==0)
                col = video::SColor(255,128,128,128);
            else
                col = video::SColor(255,32,32,32);
            driver->draw3DLine(core::vector3df(vDownLeft.X,start_y+i*interval,far_value),core::vector3df(vDownLeft.X+viewSize.Width,start_y+i*interval,far_value),col);
        }
    }
}

bool GetOrthoClickPoint(dimension2d<u32> viewSize, scene::ICameraSceneNode * camera, int clickx, int clicky, vector3df &hit_vec)
{
    const scene::SViewFrustum* frustum = camera->getViewFrustum();
    const core::vector3df cameraPosition = camera->getAbsolutePosition();

    vector3df vNearLeftDown = frustum->getNearLeftDown();
    vector3df vNearRightDown = frustum->getNearRightDown();
    vector3df vNearLeftUp = frustum->getNearLeftUp();
    vector3df vNearRightUp = frustum->getNearRightUp();

    f32 t_X = (f32)clickx / viewSize.Width;
    f32 t_Y = 1.0 - (f32)clicky / viewSize.Height;

    //screen space: Y is horizontal axis
    vector3df X_vec = (vNearRightDown - vNearLeftDown) * t_X;
    vector3df Y_vec = (vNearLeftUp - vNearLeftDown) * t_Y;
    vector3df target = vNearLeftDown + Y_vec + X_vec;

    hit_vec = target;
    return true;
}

