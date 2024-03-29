#ifndef _CAMERA_PANEL_H_
#define _CAMERA_PANEL_H_

#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "geometry_scene.h"
#include "CMeshSceneNode.h"

using namespace irr;
using namespace gui;

class TestPanel;

class CameraQuad : public gui::IGUIElement
{
public:
    CameraQuad (IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle,video::IVideoDriver* driver);

    TestPanel* getPanel(int);
    void SetPanel(int,TestPanel*);
    void initialize(scene::ISceneManager* smgr,geometry_scene* geo_scene);
    virtual void render();
    void SetFullscreen(bool,TestPanel* = NULL);
    void setGridSnap(int);
    void setRotateSnap(f32);
    void resize(core::rect<s32> rect);
    int getGridSnap();
    f32 getRotateSnap();

private:

    bool m_bFullscreen = false;
    TestPanel* panel_TL = NULL;
    TestPanel* panel_TR = NULL;
    TestPanel* panel_BL = NULL;
    TestPanel* panel_BR = NULL;
    video::IVideoDriver* driver = NULL;

    int grid_snap=4;
    f32 rotate_snap=7.5;
};


struct click_brush_info
{
    bool hit;
    core::plane3df f_plane;
    core::vector3df hitvec;
    int brush_n;
    int face_n;
    f32 distance;
};

struct click_node_info
{
    bool hit;
    int node_n;
    f32 distance;
};

class TestPanel : public gui::IGUIElement
{
public:

    //! constructor
    TestPanel(IGUIEnvironment* environment, video::IVideoDriver* driver,IGUIElement* parent, s32 id, core::rect<s32> rectangle);

    //! destructor
    virtual ~TestPanel();

    virtual bool OnEvent(const SEvent& event);

    //! sets an image
    virtual void setImage(video::ITexture* image);

    //! Gets the image texture
    virtual video::ITexture* getImage() const;

    //! sets the color of the image
    virtual void setColor(video::SColor color);

    //! sets if the image should scale to fit the element
    virtual void setScaleImage(bool scale);

    //! draws the element and its children
    virtual void draw();

    //! sets if the image should use its alpha channel to draw itself
    virtual void setUseAlphaChannel(bool use);

    //! Gets the color of the image
    virtual video::SColor getColor() const;

    //! Returns true if the image is scaled to fit, false if not
    virtual bool isImageScaled() const;

    //! Returns true if the image is using the alpha channel, false if not
    virtual bool isAlphaChannelUsed() const;

    virtual scene::ICameraSceneNode* getCamera() {return NULL;}
    virtual void Initialize(scene::ISceneManager* smgr,geometry_scene* geo_scene);
    virtual void Render(video::IVideoDriver* driver) {};
    virtual void resize(core::dimension2d<s32> new_location, core::dimension2d<u32> new_size) {};

    //! Writes attributes of the element.
    //virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const;

    //! Reads attributes of the element
    //virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);

    virtual void render() {};

    virtual void toggle_grid();
    virtual void toggle_showBrushes();
    virtual void toggle_showGeometry();

    virtual bool IsShowGrid();
    virtual bool IsShowGeometry();
    virtual bool IsShowBrushes();
    virtual bool IsDynamicLight();

    virtual void setTotalGeometry(polyfold* pf) {this->total_geometry=pf;}
    virtual void overrideMeshNode(scene::CMeshSceneNode* node) {this->override_mesh_node=node;}
    virtual void setGridSnap(int snap){grid_snap=snap;}
    virtual void setRotateSnap(f32 snap){rotate_snap=snap;}

    virtual bool GetScreenCoords(core::vector3df V, core::vector2di &out_coords) =0;

    virtual void SetMeshNodesVisible() =0;

protected:

    void draw_arrow(core::vector3df v, core::vector3df dir_);

    std::vector<int> click_hits_poly_brushes(core::vector2di v);
    bool click_hits_poly(polyfold* brush, core::vector2di v);
    virtual bool get_click_brush(int x, int y, click_brush_info& ret);
    virtual bool get_click_node(int x, int y, click_node_info& ret);
    virtual int GetSceneNodeHit(int x, int y, bool);

    virtual void left_click(core::vector2di) {} ;
    virtual void right_click(core::vector2di) {} ;

    virtual void OnMenuItemSelected(IGUIContextMenu* menu) {};

    polyfold* total_geometry=NULL;
    scene::CMeshSceneNode* override_mesh_node=NULL;
    video::ITexture* Texture;
    video::ITexture* Special_Texture;
    bool bTest=false;

    video::SColor Color;
    bool UseAlphaChannel;
    bool ScaleImage;

    s32 clickx;
    s32 clicky;
    s32 mousex;
    s32 mousey;
    bool bMouseDown=false;
    bool rMouseDown=false;
    bool bShiftDown=false;
    bool bCtrlDown=false;

    video::IVideoDriver* driver=NULL;
    scene::ISceneManager* smgr=NULL;
    scene::ICameraSceneNode* camera=NULL;
    geometry_scene* geo_scene=NULL;

    bool bShowGrid=true;
    bool bShowBrushes=true;
    bool bShowGeometry=true;
    bool bFullscreen=false;
    bool bDynamicLight=false;

    core::vector3df vDragCameraInitialPosition;
    core::vector3df vDragCameraInitialTarget;
    core::vector3df vDragCameraOrigin;
    core::vector3df vDragCameraRay;


    bool bDragCamera=false;
    bool bRotateCamera=false;
    bool bZoomCamera=false;
    bool bTextureEdit=false;

    int grid_snap=4;
    f32 rotate_snap=7.5;

    io::path rtt_name;
};

enum
{
    PANEL3D_VIEW_LOOPS,
    PANEL3D_VIEW_TRIANGLES,
    PANEL3D_VIEW_RENDER,
    PANEL3D_VIEW_RENDER_FINAL
};

class TestPanel_3D : public TestPanel
{
public:

    TestPanel_3D(IGUIEnvironment* environment, video::IVideoDriver* driver, IGUIElement* parent, s32 id, core::rect<s32> rectangle);
    ~TestPanel_3D();

    virtual bool OnEvent(const SEvent& event);
    virtual scene::ICameraSceneNode* getCamera();

    virtual void OnMenuItemSelected(IGUIContextMenu* menu);
    void SetViewStyle(s32);

    s32 GetViewStyle() {
        return view_style;
    }

    void SetDynamicLight(bool b) {
        bDynamicLight = b;
    }

    virtual void resize(core::dimension2d<s32> new_location, core::dimension2d<u32> new_size);

    virtual void render();
    void AddGraph(LineHolder& graph);
    void AddGraphs(LineHolder&, LineHolder&, LineHolder&);
    core::vector3df getDragVector();

    virtual bool GetScreenCoords(core::vector3df V, core::vector2di &out_coords);

    virtual void SetMeshNodesVisible();
    void TestLM();

protected:

    virtual bool get_click_face(int x, int y, click_brush_info& ret);

    virtual void left_click(core::vector2di pos);
    virtual void right_click(core::vector2di);

private:

    bool Get3DScreenCoords(core::vector3df V, core::vector2di &out_coords);
    void ClickAddLight();
    void ClickAddNode();

    bool bPlaneDrag=false;
    core::vector3df plane_drag_vec;
    core::vector3df plane_drag_origin;
    core::plane3df plane_drag_plane;

    s32 view_style;

    LineHolder graph;
    LineHolder graph2;
    LineHolder graph3;

};


class TestPanel_2D : public TestPanel
{
public:

    TestPanel_2D(IGUIEnvironment* environment, video::IVideoDriver* driver, IGUIElement* parent,  s32 id, core::rect<s32> rectangle);
    virtual bool OnEvent(const SEvent& event);
    virtual scene::ICameraSceneNode* getCamera();
    virtual void setAxis(int);
    virtual void setImage(video::ITexture* image);
    virtual void drawGrid(video::IVideoDriver* driver, const video::SMaterial material);

    virtual void resize(core::dimension2d<s32> new_location, core::dimension2d<u32> new_size);
    virtual void render();
    virtual void SetMeshNodesVisible();

    virtual bool GetScreenCoords(core::vector3df V, core::vector2di &out_coords);

    core::dimension2du getViewSize() {
        return viewSize;
    }
    
    void setViewSize(core::dimension2du vs) {
        viewSize = vs;
    }

protected:
    virtual void left_click(core::vector2di);
    virtual void right_click(core::vector2di);

    virtual void OnMenuItemSelected(IGUIContextMenu* menu);

    core::dimension2du viewSize;

    core::vector3df vHorizontal;
    core::vector3df vVertical;
    core::vector3df vAxis;

private:
    bool GetOrthoScreenCoords(core::vector3df V, core::vector2di &out_coords);
    void delete_selected_brushes();

    
    
    core::dimension2du oldViewSize;

    core::vector3df vDragBrushOriginalPosition;
    core::vector3df vDragVertexOriginalPosition;
    f32 RotateBrushInitialAngle;
    core::vector3df vRotateBrushOrigin;

    bool bDragBrush=false;
    bool bRotateBrush=false;
    bool bDragVertex=false;
    bool bRotateNode=false;
    bool bDragNode=false;

    int selected_brush=0;
    std::vector<int> selections;

    int m_axis=0;
    int grid_interval=16;
    f32 getViewScaling();
};

void OnMenuItemSelected(IGUIContextMenu* menu);
bool GetOrthoClickPoint(core::vector2di viewSize, scene::ICameraSceneNode * camera, int clickx, int clicky, core::vector3df &hit_vec);

#endif
