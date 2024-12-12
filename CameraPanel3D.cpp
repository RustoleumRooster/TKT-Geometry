
#include "CameraPanel.h"
#include <irrlicht.h>
#include <iostream>
#include "edit_env.h"
#include "CMeshBuffer.h"
#include "utils.h"
#include "node_properties.h"
#include "texture_adjust.h"

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


//===================================================================================================
// Camera Panel 3D
//
//

TestPanel_3D::TestPanel_3D(IGUIEnvironment* environment, video::IVideoDriver* driver, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
    : TestPanel(environment, driver, parent, id, rectangle)
{
    bShowBrushes = false;
    //#ifdef _DEBUG
    //setDebugName("TestPanel");
    //#endif
}

TestPanel_3D::~TestPanel_3D()
{
}

void TestPanel_3D::AddGraph(LineHolder& graph4)
{
    for (vector3df v : graph4.points)
        this->graph.points.push_back(v);
    for (line3df lv : graph4.lines)
        this->graph.lines.push_back(lv);
}


void TestPanel_3D::AddGraphs(LineHolder& graph1, LineHolder& graph2, LineHolder& graph3)
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
    if (this->camera == NULL)
    {
        if (this->smgr)
        {
            this->camera = smgr->addCameraSceneNode(0, core::vector3df(-300, 400, -300), core::vector3df(0, 0, 0), -1, false);
            this->camera->setNearValue(5.0);
            //this->camera->setPosition(core::vector3df(-300,400,-300));
            //this->camera->setTarget(core::vector3df(0,0,0));
        }
    }
    return this->camera;
}

void TestPanel_3D::resize(core::dimension2d<u32> new_size)
{
    this->getCamera()->setAspectRatio((f32)new_size.Width / (f32)new_size.Height);
    this->getCamera()->updateAbsolutePosition();
}

core::vector3df TestPanel_3D::getDragVector()
{
    return this->plane_drag_vec;
}

bool TestPanel_3D::OnEvent(const SEvent& event)
{
    core::vector3df hitvec;
    click_brush_info hit_results;

    {
        switch (event.EventType)

        {
        case EET_USER_EVENT:
            switch (event.UserEvent.UserData1)
            {
            case USER_EVENT_GEOMETRY_REBUILT:
            {
                this->SetViewStyle(this->view_style);
            }
            break;
            case USER_EVENT_TEXTURE_EDIT_MODE_BEGIN:
            {
                if (!bTextureEdit)
                {
                    bTextureEdit = true;
                    // std::cout<<"Texture Edit Mode begin\n";
                }
            }
            break;
            case USER_EVENT_TEXTURE_EDIT_MODE_END:
            {
                bTextureEdit = false;
                // std::cout<<"Texture Edit Mode end\n";
            }
            break;
            case USER_EVENT_SELECTION_CHANGED:
            {
                if (geo_scene->getSelectedFaces().size() == 0)
                {
                    if (bTextureEdit)
                    {
                        bTextureEdit = false;
                        // std::cout<<"Texture Edit Mode end\n";7
                    }
                }
            }
            break;
            }
            return true;
            break;
        case EET_GUI_EVENT:
            switch (event.GUIEvent.EventType)
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
                this->bMouseDown = false;
                this->rMouseDown = false;
                this->bRotateCamera = false;
                this->bZoomCamera = false;
                this->bDragCamera = false;
            }
            break;
            default:
                break;
            }
            break;
        case EET_MOUSE_INPUT_EVENT:

            mousex = event.MouseInput.X;// -AbsoluteClippingRect.UpperLeftCorner.X;
            mousey = event.MouseInput.Y;// -AbsoluteClippingRect.UpperLeftCorner.Y;
            bShiftDown = event.MouseInput.Shift;

            switch (event.MouseInput.Event)
            {
            case EMIE_LMOUSE_LEFT_UP:
            {
                /*
                if (!AbsoluteClippingRect.isPointInside(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y)))
                {
                    Environment->removeFocus(this);
                    return true;
                }*/

                if (bPlaneDrag)
                {
                    bPlaneDrag = false;
                    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();

                    SEvent event;
                    event.EventType = EET_USER_EVENT;
                    event.UserEvent.UserData1 = USER_EVENT_TEXTURE_PLANE_DRAG_END;
                    receiver->OnEvent(event);
                }
                bDragCamera = false;
                bZoomCamera = false;
                bMouseDown = false;

                if (clickx == mousex && clicky == mousey)
                    left_click(core::vector2di(event.MouseInput.X, event.MouseInput.Y));
                //std::cout<<"lmouse up\n";
            }
            return true;
            case EMIE_LMOUSE_PRESSED_DOWN:
            {
                /*
                //std::cout<<"lmouse down\n";
                if (!AbsoluteClippingRect.isPointInside(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y)))
                {
                    Environment->removeFocus(this);
                    return false;
                }*/

                if (bMouseDown == false && rMouseDown == false)
                {
                    clickx = mousex;
                    clicky = mousey;

                    //Initiate Drag
                    if (event.MouseInput.Shift && bTextureEdit)
                    {
                        if (get_click_face(clickx, clicky, hit_results))
                        {
                            bool b = false;
                            for (int f_n : this->geo_scene->getSelectedFaces())
                            {
                                if (hit_results.face_n == f_n)
                                    b = true;
                            }
                            if (b)
                            {
                                plane_drag_origin = hit_results.hitvec;
                                plane_drag_plane = hit_results.f_plane;
                                bPlaneDrag = true;

                                MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();

                                SEvent event;
                                event.EventType = EET_USER_EVENT;
                                event.UserEvent.UserData1 = USER_EVENT_TEXTURE_PLANE_DRAG;
                                receiver->OnEvent(event);
                            }
                        }
                        bRotateCamera = false;
                        bZoomCamera = false;

                    }
                    else if (get_click_face(clickx, clicky, hit_results))
                    {
                        vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                        //vDragCameraInitialTarget = this->getCamera()->getTarget();

                        core::plane3df hit_plane(hit_results.hitvec, core::vector3df(0, 1, 0));
                        core::vector3df r = this->getCamera()->getAbsolutePosition() - this->getCamera()->getTarget();
                        hit_plane.getIntersectionWithLine(this->getCamera()->getAbsolutePosition(), r, hitvec);
                        vDragCameraInitialTarget = hitvec;
                        vDragCameraOrigin = hit_results.hitvec;

                        if (r.dotProduct(this->getCamera()->getAbsolutePosition() - hitvec) < 0)
                        {
                            r.Y *= -1;
                            hit_plane.getIntersectionWithLine(this->getCamera()->getAbsolutePosition(), r, hitvec);
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

                        core::plane3df hit_plane(vDragCameraOrigin, core::vector3df(0, 1, 0));
                        GetAnyPlaneClickVector(this->Texture->getOriginalSize(), this->getCamera(), hit_plane, clickx, clicky, hitvec);
                        vDragCameraOrigin = hitvec;

                        core::vector3df r = vDragCameraOrigin - this->getCamera()->getAbsolutePosition();

                        if (r.getLength() > 1000)
                        {
                            r.normalize();
                            r *= 1000;
                            vDragCameraOrigin = this->getCamera()->getAbsolutePosition() + r;
                        }

                        bDragCamera = true;
                        bRotateCamera = false;
                        bZoomCamera = false;
                    }
                    bMouseDown = true;
                }
                else if (bMouseDown == false)
                {
                    bMouseDown = true;
                }
            }
            return true;
            case EMIE_RMOUSE_PRESSED_DOWN:
            {
                /*
                //if (Environment->hasFocus(this))
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
                */
                if (rMouseDown == false && bMouseDown == false)
                {
                    //NB / TODO
                    //Environment->setFocus(this);
                    clickx = mousex;
                    clicky = mousey;

                    //Initiate Rotate Camera
                    vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                    vDragCameraInitialTarget = this->getCamera()->getTarget();

                    if (get_click_face(clickx, clicky, hit_results))
                    {
                        vDragCameraRay = this->getCamera()->getAbsolutePosition() - hit_results.hitvec;
                        vDragCameraOrigin = hit_results.hitvec;
                    }
                    else
                    {
                        vDragCameraRay = this->getCamera()->getAbsolutePosition() - vDragCameraOrigin;
                    }

                    if (vDragCameraRay.getLength() > 1000)
                    {
                        vDragCameraRay.normalize();
                        vDragCameraRay *= 1000;
                        vDragCameraOrigin = this->getCamera()->getAbsolutePosition() - vDragCameraRay;
                    }

                    bRotateCamera = true;
                    rMouseDown = true;
                }
                else if (rMouseDown == false)
                {
                    rMouseDown = true;
                }
            }
            return true;
            case EMIE_RMOUSE_LEFT_UP:
            {/*
                if (!AbsoluteClippingRect.isPointInside(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y)))
                {
                    Environment->removeFocus(this);
                    //std::cout<<"rmouse up\n";
                }*/
                //else
                    //std::cout<<"rmouse up\n";

                bRotateCamera = false;
                bZoomCamera = false;
                rMouseDown = false;

                if (clickx == mousex && clicky == mousey)
                { 
                    if (this->m_viewPanel)
                    {
                        vector2d<s32> clickpos = m_viewPanel->getClickPos();
                        right_click(clickpos);
                    }
                }
            }
            return true;
            case EMIE_MOUSE_MOVED:
                if (bMouseDown == true && rMouseDown == true)
                {
                    if (bZoomCamera == false)
                    {
                        clickx = mousex;
                        clicky = mousey;
                        vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                        vDragCameraInitialTarget = this->getCamera()->getTarget();
                        vDragCameraRay = this->getCamera()->getAbsolutePosition() - this->getCamera()->getTarget();

                        bZoomCamera = true;
                        bRotateCamera = false;
                        bDragCamera = false;
                    }
                    else
                    {
                        int ydif = mousey - clicky;
                        float zoom_f = 1.0 - (0.02 * ydif);
                        zoom_f = fmax(zoom_f, 0.01);
                        this->getCamera()->setPosition(vDragCameraInitialTarget + (vDragCameraRay * zoom_f));
                        this->getCamera()->updateAbsolutePosition();
                    }
                }
                else if (bRotateCamera)
                {
                    core::vector3df r = vDragCameraOrigin - this->getCamera()->getTarget();
                    core::vector3df r2 = vDragCameraInitialPosition - vDragCameraInitialTarget;

                    f32 d = fmin(r2.getLength(), vDragCameraRay.getLength());

                    if (vDragCameraRay.getLength() > d)
                    {
                        vDragCameraRay *= 0.99;
                    }

                    int xRot = mousex - clickx;

                    float yaw = 0.008 * xRot;
                    core::vector3df newCameraRay = vDragCameraRay;
                    core::matrix4 MCamRotate;
                    MCamRotate.setRotationAxisRadians(yaw, core::vector3df(0, 1, 0));
                    MCamRotate.rotateVect(newCameraRay);

                    int yRot = mousey - clicky;
                    float pitch = 0.008 * yRot;

                    core::vector3df rot_axis = newCameraRay.crossProduct(core::vector3df(0, 1, 0));
                    rot_axis.normalize();

                    MCamRotate.setRotationAxisRadians(pitch, rot_axis);
                    MCamRotate.rotateVect(newCameraRay);

                    this->getCamera()->setPosition(vDragCameraOrigin + newCameraRay);

                    int mouse_dist = sqrt(xRot * xRot + yRot * yRot);
                    this->camera->setTarget(this->getCamera()->getTarget() + r * 0.04);

                    this->getCamera()->updateAbsolutePosition();
                }
                else if (bDragCamera)
                {
                    core::vector3df tvec;

                    //Update the camera's view frustrum, or we may get errors
                    this->getCamera()->render();

                    core::plane3df plane(vDragCameraOrigin, core::vector3df(0, 1, 0));
                    GetAnyPlaneClickVector(this->Texture->getOriginalSize(), this->getCamera(), plane, mousex, mousey, tvec);
                    core::line3df dragLine = core::line3df(vDragCameraOrigin, tvec);

                    core::vector2di m_coords(mousex, mousey);
                    core::vector2di coords;
                    core::vector3df route = dragLine.getMiddle();

                    if (GetScreenCoords(route, coords))
                    {
                        if (m_coords.getDistanceFrom(coords) > 1)
                        {
                            tvec = dragLine.getVector();
                            tvec *= 0.8;

                            while (tvec.getLength() > 25)
                                tvec *= 0.75;

                            this->getCamera()->setPosition(vDragCameraInitialPosition - tvec);

                            this->getCamera()->setTarget(vDragCameraInitialTarget - tvec);
                            this->getCamera()->updateAbsolutePosition();

                            vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                            vDragCameraInitialTarget = this->getCamera()->getTarget();
                        }
                    }
                }
                else if (event.MouseInput.Shift && bTextureEdit && bPlaneDrag)
                {

                    //Update the camera's view frustrum, or we may get errors
                    this->getCamera()->render();

                    GetAnyPlaneClickVector(this->Texture->getOriginalSize(), this->getCamera(), plane_drag_plane, mousex, mousey, hitvec);
                    plane_drag_vec = hitvec - plane_drag_origin;

                    geo_scene->setDragVec(plane_drag_vec);

                    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();

                    SEvent event;
                    event.EventType = EET_USER_EVENT;
                    event.UserEvent.UserData1 = USER_EVENT_TEXTURE_PLANE_DRAG;
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
   // return IGUIElement::OnEvent(event);
    return false;
}

bool TestPanel::get_click_node(int x, int y, click_node_info& ret)
{
    ret.distance = 0xFFFF;
    ret.node_hit = NULL;
    ret.hit = false;
    for(Reflected_SceneNode* sn : geo_scene->getSelectedNodes())
    //for (int i = 0; i < geo_scene->getSceneNodes().size(); i++)
    {
        //Reflected_SceneNode* sn = geo_scene->getSceneNodes()[i];

        core::rect<s32> v_rect = ((Reflected_Sprite_SceneNode*)sn)->GetVisibleRectangle(this);

        if (v_rect.isPointInside(core::vector2di(clickx, clicky)))
        {
            f32 d = sn->getDistanceFromCamera(this);
            if (d < ret.distance)
            {
                //std::cout<<"hit object!\n";
                ret.hit = true;
                ret.distance = d;
                //ret.node_n = i;
                ret.node_hit = sn;
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

    for (int i = 0; i < geo_scene->elements.size(); i++)
    {
        polyfold* brush = &geo_scene->elements[i].brush;

        for (int j = 0; j < brush->edges.size(); j++)
        {
            core::vector3df v0 = brush->getVertex(j, 0).V;
            core::vector3df v1 = brush->getVertex(j, 1).V;
            core::vector2di w0;
            core::vector2di w1;

            if (GetScreenCoords(v0, w0) && GetScreenCoords(v1, w1))
            {
                core::line2di screen_line(w0, w1);

                core::vector2di click_coord(x, y);
                core::vector2di r = screen_line.getClosestPoint(click_coord);

                f32 d = r.getDistanceFrom(click_coord);
                if (d < 4)
                {
                    f32 m = (f32)core::vector2di(r - w0).getLength() /
                        (f32)core::vector2di(w1 - w0).getLength();

                    core::vector3df hit_vec = v0 + core::vector3df(v1 - v0) * m;
                    f32 dist = hit_vec.getDistanceFrom(cam_pos);

                    if (dist < distance)
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
    f32 dist = 9999;

    for (int j = 0; j < this->geo_scene->get_total_geometry()->faces.size(); j++)
    {
        core::plane3df f_plane(this->geo_scene->get_total_geometry()->faces[j].m_center,
            this->geo_scene->get_total_geometry()->faces[j].m_normal);

        if (GetAnyPlaneClickVector(this->Texture->getOriginalSize(), this->getCamera(), f_plane, x, y, hitvec) &&
            this->geo_scene->get_total_geometry()->is_point_on_face(j, hitvec))
        {
            f32 d = hitvec.getDistanceFrom(cam_pos);
            if (d<dist && d > near_dist &&
                (this->geo_scene->get_total_geometry()->faces[j].m_normal.dotProduct(cam_pos - hitvec) > 0 ||
                    this->geo_scene->getMaterialGroupsBase()->material_groups[this->geo_scene->get_total_geometry()->faces[j].material_group].two_sided == true
                    )
                &&
                (hitvec - cam_pos).dotProduct(this->getCamera()->getTarget() - cam_pos) > 0)
            {
                ret_vec = hitvec;
                ret_plane = f_plane;
                dist = d;
                selected_poly = 0;
                selected_face = j;
            }
        }
    }

    if (selected_poly != -1)
    {
        ret.hit = true;
        ret.hitvec = ret_vec;
        ret.f_plane = ret_plane;
        ret.brush_n = selected_poly;
        ret.face_n = selected_face;
        ret.distance = dist;
        return true;
    }

    ret.hit = false;

    return false;
}

void TestPanel_3D::left_click(core::vector2di pos)
{
    std::vector<int> old_sel_faces = geo_scene->getSelectedFaces();
    std::vector<Reflected_SceneNode*> old_sel_nodes = geo_scene->getSelectedNodes();
    std::vector<int> old_sel_brushes = geo_scene->getBrushSelection();

    if (click_hits_poly(&geo_scene->elements[0].brush, core::vector2di(clickx, clicky)))
    {
        geo_scene->setSelectedFaces(std::vector<int>{});
        geo_scene->setSelectedNodes(std::vector<Reflected_SceneNode*>{});
        geo_scene->setBrushSelection(std::vector<int>{0});
    }
    else if (this->bShowBrushes && !this->bShowGeometry)
    {

        click_brush_info res;
        get_click_brush(clickx, clicky, res);

        click_node_info node_hit;
        get_click_node(clickx, clicky, node_hit);

        if (bShiftDown && geo_scene->getBrushSelection().size() > 0)
        {
            if (res.hit == true)
            {
                geo_scene->setBrushSelection_ShiftAdd(res.brush_n);
            }
        }
        else if (bShiftDown && geo_scene->getSelectedNodes().size() > 0)
        {
            if (node_hit.hit == true)
            {
                geo_scene->setSelectedNodes_ShiftAdd(node_hit.node_hit);
            }
        }
        else if (node_hit.hit == true && (res.hit == false || node_hit.distance < res.distance))
        {
            geo_scene->setBrushSelection(std::vector<int>{});
            geo_scene->setSelectedNodes(std::vector<Reflected_SceneNode*>{node_hit.node_hit});
        }
        else if (res.hit == true)
        {
            geo_scene->setSelectedNodes(std::vector<Reflected_SceneNode*>{});
            geo_scene->setBrushSelection(std::vector<int>{res.brush_n});
        }
        else
        {
            geo_scene->setBrushSelection(std::vector<int>{});
            geo_scene->setSelectedNodes(std::vector<Reflected_SceneNode*>{});
        }

    }
    else if (this->bShowGeometry && this->view_style == PANEL3D_VIEW_RENDER)
    {
        core::vector3df hitvec;
        int selected_face = -1;

        click_brush_info res;
        if (get_click_face(clickx, clicky, res))
        {
            selected_face = res.face_n;
        }

        //int hit_node_i = GetSceneNodeHit(clickx, clicky, true);
        Reflected_SceneNode* node_hit = GetSceneNodeHit(clickx, clicky, true);

        this->geo_scene->setBrushSelection(std::vector<int>{});

        if (bShiftDown && geo_scene->getSelectedFaces().size() > 0)
        {
            if (res.hit == true)
            {
                geo_scene->setSelectedFaces_ShiftAdd(selected_face);
            }
        }
        else if (bShiftDown && geo_scene->getSelectedNodes().size() > 0)
        {
            if (node_hit != NULL)
            {
                geo_scene->setSelectedNodes_ShiftAdd(node_hit);
            }
        }
        else if (node_hit != NULL)
        {
            geo_scene->setSelectedFaces(std::vector<int>{});
            geo_scene->setSelectedNodes(std::vector<Reflected_SceneNode*>{node_hit});
        }
        else if (res.hit == true)
        {
            geo_scene->setSelectedNodes(std::vector<Reflected_SceneNode*>{});
            geo_scene->setSelectedFaces(std::vector<int>{selected_face});
        }
        else
        {
            geo_scene->setSelectedFaces(std::vector<int>{});
            geo_scene->setSelectedNodes(std::vector<Reflected_SceneNode*>{});

        }
    }
    else if (this->bShowGeometry && this->view_style == PANEL3D_VIEW_LOOPS)
    {
        Reflected_SceneNode* node_hit = GetSceneNodeHit(clickx, clicky, false);

        geo_scene->setSelectedFaces(std::vector<int>{});
        geo_scene->setBrushSelection(std::vector<int>{});

        if (bShiftDown && geo_scene->getSelectedNodes().size() > 0)
        {
            if (node_hit != NULL)
            {
                geo_scene->setSelectedNodes_ShiftAdd(node_hit);
            }
        }
        else if (node_hit != NULL)
        {
            geo_scene->setSelectedNodes(std::vector<Reflected_SceneNode*>{node_hit});
        }
        else
            geo_scene->setSelectedNodes(std::vector<Reflected_SceneNode*>{});
    }
    else if (this->bShowGeometry)
    {
        this->geo_scene->setBrushSelection(std::vector<int>{});
        this->geo_scene->setSelectedNodes(std::vector<Reflected_SceneNode*>{});
        this->geo_scene->setSelectedFaces(std::vector<int>{});
    }

    if (old_sel_faces == geo_scene->getSelectedFaces() &&
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
    if (geo_scene && view_style == PANEL3D_VIEW_RENDER)
    {
        click_brush_info res;
        if (get_click_face(clickx, clicky, res))
        {
            selected_face = res.face_n;
            selected_poly = res.brush_n;

            core::vector3df N = geo_scene->get_total_geometry()->faces[selected_face].m_normal;

            geo_scene->addSceneLight(res.hitvec + N * 32);
        }
    }
}


void TestPanel_3D::ClickAddNode()
{
    int selected_face = -1;
    int selected_poly = -1;
    if (geo_scene && view_style == PANEL3D_VIEW_RENDER)
    {
        click_brush_info res;
        if (get_click_face(clickx, clicky, res))
        {
            selected_face = res.face_n;
            selected_poly = res.brush_n;

            core::vector3df N = geo_scene->get_total_geometry()->faces[selected_face].m_normal;

            geo_scene->addSceneSelectedSceneNodeType(res.hitvec + N * 32);
        }
    }
}

void TestPanel_3D::right_click(core::vector2di pos)
{
    //gui::IGUIElement* root = Environment->getRootGUIElement();
    //gui::IGUIElement* e = root->getElementFromId(GUI_ID_DIALOGUE_ROOT_WINDOW);

    if (geo_scene && view_style == PANEL3D_VIEW_RENDER && geo_scene->getSelectedFaces().size() > 0)
    {
        gui::IGUIContextMenu* menu = environment->addContextMenu(core::rect<s32>(pos, core::vector2di(256, 256)), 0, -1);
        menu->addItem(L"Use Current Texture", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_SET_TEXTURE, true, false, false, false);
        menu->addItem(L"Choose Texture", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_CHOOSE_TEXTURE, true, false, false, false);
        menu->addItem(L"Align Texture", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADJUST_TEXTURE, true, false, false, false);
        menu->addSeparator();
        menu->addItem(L"Set Material Group", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_MATERIAL_GROUP, true, false, false, false);
        menu->addSeparator();
        menu->addItem(L"Add Light", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADD_LIGHT, true, false, false, false);

        reflect::TypeDescriptor_Struct* node_type = geo_scene->getSelectedNodeClass();

        if (node_type)
        {
            std::wstring txt0(L"Add ");
            std::wstring txt(node_type->alias, node_type->alias + strlen(node_type->alias));
            txt0 += txt;
            menu->addItem(txt0.c_str(), GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_ADD_NODE, true, false, false, false);
        }
        else
        {
            menu->addItem(L"(No node type selected)", -1, true, false, false, false);
        }

        ContextMenuOwner = this->m_viewPanel;
    }
    else if (geo_scene && geo_scene->getBrushSelection().size() > 0)
    {
        for (int p_i : geo_scene->getBrushSelection())
        {
            if (click_hits_poly(&geo_scene->elements[p_i].brush, core::vector2di(clickx, clicky)))
            {
                for (int v_i = 0; v_i < geo_scene->elements[p_i].brush.vertices.size(); v_i++)
                {
                    core::vector2di coords;
                    GetScreenCoords(geo_scene->elements[p_i].brush.vertices[v_i].V, coords);
                    if (core::vector2di(clickx, clicky).getDistanceFrom(coords) < 4)
                    {
                        geo_scene->selected_brush_vertex_editing = p_i;
                        geo_scene->elements[p_i].selected_vertex = v_i;
                        geo_scene->elements[p_i].control_vertex_selected = false;
                    }
                }
            }
            for (int v_i = 0; v_i < geo_scene->elements[p_i].brush.control_vertices.size(); v_i++)
            {
                core::vector2di coords;
                GetScreenCoords(geo_scene->elements[p_i].brush.control_vertices[v_i].V, coords);
                if (core::vector2di(clickx, clicky).getDistanceFrom(coords) < 4)
                {
                    geo_scene->selected_brush_vertex_editing = p_i;
                    geo_scene->elements[p_i].selected_vertex = v_i;
                    geo_scene->elements[p_i].control_vertex_selected = true;
                }
            }
        }
    }
    else if (geo_scene && geo_scene->getSelectedNodes().size() > 0)
    {
        gui::IGUIContextMenu* menu = environment->addContextMenu(core::rect<s32>(pos, core::vector2di(256, 256)), 0, -1);

        std::vector<reflect::TypeDescriptor_Struct*> typeDescriptors = Node_Properties_Base::GetTypeDescriptors(geo_scene);

        if (typeDescriptors.size() > 0)
        {
            int n = typeDescriptors.size() - 1;
            std::wstring txt(typeDescriptors[n]->alias, typeDescriptors[n]->alias + strlen(typeDescriptors[n]->alias));
            std::wstring txt0(L"delete ");
            txt0 += txt;
            txt0 += std::wstring(L"(s)");
            txt += std::wstring(L" properties");

            menu->addItem(txt.c_str(), GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_NODE_PROPERTIES, true, false, false, false);
            menu->addSeparator();
            menu->addItem(txt0.c_str(), GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_DELETE_NODE, true, false, false, false);
            ContextMenuOwner = this->m_viewPanel;
        }
    }
    else
    {
        gui::IGUIContextMenu* menu = environment->addContextMenu(core::rect<s32>(pos, core::vector2di(256, 256)), 0, -1);

        menu->addItem(L"View  ", -1, true, true, false, false);
        menu->addItem(L"Grid  ", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_GRID_TOGGLE, true, false, true, true);
        menu->addItem(L"Fullscreen ", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_FULLSCREEN_TOGGLE, true, false, true, true);
        menu->addItem(L"Build Lighting ", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_TEST, true, false, false, true);

        menu->setItemChecked(1, this->bShowGrid);
        menu->setItemChecked(2, this->bFullscreen);

        gui::IGUIContextMenu* submenu;
        submenu = menu->getSubMenu(0);
        submenu->addItem(L"Brushes", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_BRUSHES, true, false, true, true);
        submenu->addItem(L"Geometry ", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_GEOMETRY, true, false, true, true);
        submenu->addSeparator();
        submenu->addItem(L"Loops", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_LOOPS, true, false, true, true);
        submenu->addItem(L"Triangles", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_TRIANGLES, true, false, true, true);
        submenu->addItem(L"Edit Mesh", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_RENDER, true, false, true, true);
        submenu->addItem(L"Final Mesh", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_RENDER_FINAL, true, false, true, true);
        submenu->addSeparator();
        submenu->addItem(L"Unlit", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_UNLIT, true, false, true, true);
        submenu->addItem(L"Lighting", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_LIGHTMAP, true, false, true, true);
        submenu->addItem(L"Light Only", GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_LIGHT_ONLY, true, false, true, true);

        submenu->setItemChecked(0, this->bShowBrushes);
        submenu->setItemChecked(1, this->bShowGeometry);
        submenu->setItemChecked(3, this->view_style == PANEL3D_VIEW_LOOPS);
        submenu->setItemChecked(4, this->view_style == PANEL3D_VIEW_TRIANGLES);
        submenu->setItemChecked(5, this->view_style == PANEL3D_VIEW_RENDER);
        submenu->setItemChecked(6, this->view_style == PANEL3D_VIEW_RENDER_FINAL);

        if (this->view_style == PANEL3D_VIEW_RENDER || this->view_style == PANEL3D_VIEW_RENDER_FINAL)
        {
            submenu->setItemEnabled(8, true);
            submenu->setItemEnabled(9, true);
            submenu->setItemEnabled(10, true);

            submenu->setItemChecked(8, this->lighting_type == LIGHTING_UNLIT);
            submenu->setItemChecked(9, this->lighting_type == LIGHTING_LIGHTMAP);
            submenu->setItemChecked(10, this->lighting_type == LIGHTING_LIGHT_ONLY);
        }
        else
        {
            submenu->setItemEnabled(8, false);
            submenu->setItemEnabled(9, false);
            submenu->setItemEnabled(10, false);
        }

        ContextMenuOwner = this->m_viewPanel;
    }
}

void TestPanel_3D::OnMenuItemSelected(IGUIContextMenu* menu)
{
    s32 id = menu->getItemCommandId(menu->getSelectedItem());

    scene::CMeshSceneNode* mesh_node = this->geo_scene->getMeshNode();

    //BVH_structure<poly_edge> test_bvh(geo_scene->get_total_geometry());

    switch (id)
    {
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_GRID_TOGGLE:
        this->toggle_grid();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_BRUSHES:
        this->toggle_showBrushes();
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_GEOMETRY:
        this->toggle_showGeometry();
        if (this->bShowGeometry)
            this->SetViewStyle(this->view_style);
        else if (mesh_node)
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
        if (this->lighting_type != LIGHTING_UNLIT)
        {
            this->lighting_type = LIGHTING_UNLIT;
            this->SetViewStyle(this->view_style);
        }
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_LIGHTMAP:
        if (this->lighting_type != LIGHTING_LIGHTMAP)
        {
            this->lighting_type = LIGHTING_LIGHTMAP;
            this->SetViewStyle(this->view_style);
        }
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_VIEW_LIGHT_ONLY:
        if (this->lighting_type != LIGHTING_LIGHT_ONLY)
        {
            this->lighting_type = LIGHTING_LIGHT_ONLY;
            this->SetViewStyle(this->view_style);
        }
        break;
    case GUI_ID_VIEWPORT_3D_RIGHTCLICK_MENU_ITEM_SET_TEXTURE:
        if (this->geo_scene)
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
       // ((CameraQuad*)this->getParent())->SetFullscreen(this->bFullscreen, this);
        m_viewPanel->set_fullscreen(bFullscreen);
        break;
    default:
        break;
    }
}

void TestPanel_3D::SetMeshNodesVisible()
{
    if (bShowGeometry && geo_scene->getMeshNode())
    {
        switch (this->view_style)
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
    this->view_style = vtype;


    scene::CMeshSceneNode* mesh_node;
    if (this->override_mesh_node == NULL)
        mesh_node = this->geo_scene->getMeshNode();
    else mesh_node = this->override_mesh_node;

    switch (vtype)
    {
    case PANEL3D_VIEW_LOOPS:
    {
        bShowGeometry = true;
        graph.lines.clear();
        graph2.lines.clear();
        graph3.lines.clear();
        graph.points.clear();
        graph2.points.clear();
        graph3.points.clear();

        for (geo_element el : geo_scene->elements)
        {
            el.geometry.addDrawLinesEdges(graph2);
        }

        geo_scene->drawGraph(graph);

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

       // geo_scene->drawGraph(graph);

        if (this->geo_scene->get_total_geometry())
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
        bShowGeometry = true;
        //std::cout << "triangles\n";
        geo_scene->buildSceneGraph(false, false, false);
        mesh_node = geo_scene->getMeshNode();

        graph.lines.clear();
        geo_scene->drawGraph(graph);

        if (geo_scene->getSelectedFaces().size() > 0)
        {
            geo_scene->setSelectedFaces(std::vector<int>{});
        }
        if (mesh_node)
        {
            mesh_node->setVisible(true);
            mesh_node->setWireFrame(true);
        }
        break;
    case PANEL3D_VIEW_RENDER:
        //std::cout << "edit\n";
        bShowGeometry = true;

        geo_scene->buildSceneGraph(false, true, this->lighting_type, false);

        mesh_node = geo_scene->getMeshNode();

        if (mesh_node)
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
            geo_scene->buildSceneGraph(false, true, true);
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

        geo_scene->buildSceneGraph(true, false, this->lighting_type, true);
        mesh_node = geo_scene->getMeshNode();

        bShowGeometry = true;

        if (mesh_node)
        {
            mesh_node->setVisible(true);
            mesh_node->setWireFrame(false);
        }

        break;
    }
}


void TestPanel_3D::render()
{

    driver->setRenderTarget(getImage(), true, true, video::SColor(255, 16, 16, 16));
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

    if (this->geo_scene)
    {
        if (bShowBrushes)
        {
            for (geo_element& geo : this->geo_scene->elements)
            {
                if (!geo.bSelected)
                    geo.draw_brush(driver, someMaterial);
            }
            for (geo_element& geo : this->geo_scene->elements)
            {
                if (geo.bSelected)
                    geo.draw_brush(driver, someMaterial);
            }
            for (int e_i = 0; e_i < this->geo_scene->elements.size(); e_i++)
            {
                geo_element* geo = &this->geo_scene->elements[e_i];
                if (geo->bSelected)
                {

                    core::vector2di coords;
                    for (int i = 0; i < geo->brush.vertices.size(); i++)
                    {
                        GetScreenCoords(geo->brush.vertices[i].V, coords);
                        coords.X -= 4;
                        coords.Y -= 4;
                        if (geo_scene->selected_brush_vertex_editing == e_i && geo->control_vertex_selected == false && geo->selected_vertex == i)
                        {
                            if (geo->type == GEO_ADD)
                                driver->draw2DImage(med_circle_tex_add_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_SUBTRACT)
                                driver->draw2DImage(med_circle_tex_sub_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_RED)
                                driver->draw2DImage(med_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        }
                        else
                        {
                            if (geo->type == GEO_ADD)
                                driver->draw2DImage(small_circle_tex_add_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_SUBTRACT)
                                driver->draw2DImage(small_circle_tex_sub_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_RED)
                                driver->draw2DImage(small_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        }
                    }
                    for (int i = 0; i < geo->brush.control_vertices.size(); i++)
                    {
                        GetScreenCoords(geo->brush.control_vertices[i].V, coords);
                        coords.X -= 4;
                        coords.Y -= 4;
                        if (geo_scene->selected_brush_vertex_editing == e_i && geo->control_vertex_selected == true && geo->selected_vertex == i)
                        {
                            if (geo->type == GEO_ADD)
                                driver->draw2DImage(med_circle_tex_add_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_SUBTRACT)
                                driver->draw2DImage(med_circle_tex_sub_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_RED)
                                driver->draw2DImage(med_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        }
                        else
                        {
                            if (geo->type == GEO_ADD)
                                driver->draw2DImage(small_circle_tex_add_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_SUBTRACT)
                                driver->draw2DImage(small_circle_tex_sub_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_RED)
                                driver->draw2DImage(small_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        }
                    }
                }
            }
        }
        else
        {   //always draw the red brush
            geo_scene->elements[0].draw_brush(driver, someMaterial);

            if (geo_scene->elements[0].bSelected)
            {

                core::vector2di coords;
                for (int i = 0; i < geo_scene->elements[0].brush.vertices.size(); i++)
                {
                    GetScreenCoords(geo_scene->elements[0].brush.vertices[i].V, coords);
                    coords.X -= 4;
                    coords.Y -= 4;
                    if (geo_scene->selected_brush_vertex_editing == 0 && geo_scene->elements[0].control_vertex_selected == false && geo_scene->elements[0].selected_vertex == i)
                    {
                        driver->draw2DImage(med_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                    }
                    else
                    {
                        driver->draw2DImage(small_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                    }
                }
                for (int i = 0; i < geo_scene->elements[0].brush.control_vertices.size(); i++)
                {
                    GetScreenCoords(geo_scene->elements[0].brush.control_vertices[i].V, coords);
                    coords.X -= 4;
                    coords.Y -= 4;
                    if (geo_scene->selected_brush_vertex_editing == 0 && geo_scene->elements[0].control_vertex_selected == true && geo_scene->elements[0].selected_vertex == i)
                    {
                        driver->draw2DImage(med_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                    }
                    else
                    {
                        driver->draw2DImage(small_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
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

    if (bShowGeometry && view_style == PANEL3D_VIEW_LOOPS)
    {
        for (core::line3df aline : graph2.lines)
        {
            driver->draw3DLine(aline.start, aline.end, video::SColor(255, 96, 128, 96));
        }
        for (core::line3df aline : graph3.lines)
        {
            driver->draw3DLine(aline.start, aline.end, video::SColor(255, 32, 0, 150));
        }
        for (core::line3df aline : graph.lines)
        {
            driver->draw3DLine(aline.start, aline.end, video::SColor(255, 255, 0, 255));
        }
        for (core::vector3df v : graph.points)
        {
            int len = 4;
            driver->draw3DLine(v + core::vector3df(len, 0, 0), v - core::vector3df(len, 0, 0), video::SColor(128, 255, 0, 255));
            driver->draw3DLine(v + core::vector3df(0, len, 0), v - core::vector3df(0, len, 0), video::SColor(128, 255, 0, 255));
            driver->draw3DLine(v + core::vector3df(0, 0, len), v - core::vector3df(0, 0, len), video::SColor(128, 255, 0, 255));
        }

        for (core::vector3df v : graph2.points)
        {
            int len = 4;
            driver->draw3DLine(v + core::vector3df(len, 0, 0), v - core::vector3df(len, 0, 0), video::SColor(255, 96, 128, 96));
            driver->draw3DLine(v + core::vector3df(0, len, 0), v - core::vector3df(0, len, 0), video::SColor(255, 96, 128, 96));
            driver->draw3DLine(v + core::vector3df(0, 0, len), v - core::vector3df(0, 0, len), video::SColor(255, 96, 128, 96));
        }
    }
    else if (view_style == PANEL3D_VIEW_TRIANGLES)
    {
        for (core::line3df aline : graph.lines)
        {
            driver->draw3DLine(aline.start, aline.end, video::SColor(255, 255, 0, 255));
        }
    }

    driver->setRenderTarget(NULL, true, true, video::SColor(0, 0, 0, 0));

}


bool TestPanel_3D::GetScreenCoords(core::vector3df V, core::vector2di& out_coords)
{
    return Get3DScreenCoords(V, out_coords);
}

bool TestPanel_3D::Get3DScreenCoords(core::vector3df V, core::vector2di& out_coords)
{
    const scene::SViewFrustum* frustum = this->getCamera()->getViewFrustum();
    const core::vector3df cameraPosition = this->getCamera()->getAbsolutePosition();

    vector3df vNearLeftDown = frustum->getNearLeftDown();
    vector3df vNearRightDown = frustum->getNearRightDown();
    vector3df vNearLeftUp = frustum->getNearLeftUp();
    vector3df vNearRightUp = frustum->getNearRightUp();

    vector3df ray = V - cameraPosition;

    core::plane3df aplane(vNearLeftDown, vNearLeftUp, vNearRightDown);
    core::vector3df vIntersect;

    if (aplane.getIntersectionWithLimitedLine(cameraPosition, V, vIntersect))
    {
        core::vector3df v_X = (vNearRightDown - vNearLeftDown);
        v_X.normalize();
        f32 t_X = v_X.dotProduct(vIntersect - vNearLeftDown);
        t_X /= (vNearRightDown - vNearLeftDown).getLength();

        core::vector3df v_Y = (vNearLeftUp - vNearLeftDown);
        v_Y.normalize();
        f32 t_Y = v_Y.dotProduct(vIntersect - vNearLeftDown);
        t_Y /= (vNearLeftUp - vNearLeftDown).getLength();

        out_coords.X = core::round32(t_X * this->Texture->getOriginalSize().Width);
        out_coords.Y = core::round32((1 - t_Y) * this->Texture->getOriginalSize().Height);
        return true;
    }
    return false;
}