#ifndef _TKT_CUSTOM_NODES_
#define _TKT_CUSTOM_NODES_

#include <irrlicht.h>
#include "Reflection.h"
#include "CameraPanel.h"
#include "myNodes.h"
#include "edit_env.h"
#include "reflected_nodes.h"

class Reflected_LightSceneNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_LightSceneNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
        Reflected_Sprite_SceneNode(parent, smgr, id, pos) {}

    virtual bool bShowEditorArrow() { return true; }
    virtual ESCENE_NODE_TYPE getType() { return ESNT_LIGHT; }

    virtual void render() { Reflected_Sprite_SceneNode::render(); }
    virtual void translate(core::matrix4);
    virtual void onClear() { my_light = NULL; }
    virtual void postEdit();
    virtual void addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override;

    scene::ILightSceneNode* my_light = NULL;
    bool enabled = true;
    int light_radius = 50;
    REFLECT2()
};

class Reflected_SimpleEmitterSceneNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_SimpleEmitterSceneNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
        Reflected_Sprite_SceneNode(parent, smgr, id, pos) {}

    virtual bool bShowEditorArrow() { return true; }

    virtual void render();
    virtual void addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override;

    core::vector3df EmitBox = core::vector3df(64, 64, 64);
    video::ITexture* texture = NULL;
    core::vector2df particle_scale = core::vector2df(10, 10);
    int minParticlesPerSecond = 10;
    int maxParticlesPerSecond = 25;
    int lifeTimeMin = 500;
    int lifeTimeMax = 1000;
    bool bool_A;
    bool bool_B;
    video::SColor color;
    core::vector3df Vector_A = core::vector3df(64, 64, 64);
    core::vector3df Vector_B = core::vector3df(64, 64, 64);
    core::vector3df Vector_C = core::vector3df(64, 64, 64);

    REFLECT2()
};

class ReflectionTestPanel : public TestPanel
{
public:
    ReflectionTestPanel(gui::IGUIEnvironment* environment, video::IVideoDriver* driver, gui::IGUIElement* parent, s32 id, core::rect<s32> rectangle) :
        TestPanel(environment, driver, parent, id, rectangle) {}
    ~ReflectionTestPanel();

    virtual scene::ICameraSceneNode* getCamera();
    virtual void Initialize(scene::ISceneManager* smgr, geometry_scene* geo_scene);
    virtual void render();
    virtual void resize(core::dimension2d<u32> new_size);
    virtual void SetMeshNodesVisible();
    virtual bool GetScreenCoords(core::vector3df V, core::vector2di& out_coords);

    void setCameraQuad(CameraQuad* q) { cameraQuad = q; }
    void set_material(s32 material_type) { render_material = (video::E_MATERIAL_TYPE) material_type; }
    void set_underwater_material(s32 material_type) { underwater_material = (video::E_MATERIAL_TYPE) material_type; }
    video::ITexture* getRender();
    video::ITexture* getRTT() { return my_rtt; }
    video::ITexture* getRTT2() { return my_rtt2; }
private:

    video::ITexture* get_rtt();
    video::ITexture* get_rtt2();

    CameraQuad* cameraQuad = NULL;
    //ISceneManager* smgr2 = NULL;
    video::E_MATERIAL_TYPE render_material = video::EMT_SOLID;
    video::E_MATERIAL_TYPE underwater_material = video::EMT_SOLID;
    video::ITexture* my_rtt = NULL;
    video::ITexture* my_rtt2 = NULL;
    TwoTriangleSceneNode* render_node = NULL;

    std::vector<int> hide_faces;
};

class Render_Tool_Base;

class Render_Tool_Widget : public gui::IGUIElement
{

public:
    Render_Tool_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene*, Render_Tool_Base*, s32 id, core::rect<s32> rect);
    ~Render_Tool_Widget();

    void show();

private:
    int my_ID;

    geometry_scene* g_scene = NULL;
    Render_Tool_Base* my_base = NULL;
};

class Render_Tool_Base : public simple_reflected_tool_base
{
    struct render_options_struct
    {
        int some_int;
        REFLECT()
    };

public:

    ~Render_Tool_Base() {

        if (view_panel)
            delete view_panel;
    }

    virtual void show();

    virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_) {}
    virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_, scene::ISceneManager* smgr);

    void setCameraQuad(CameraQuad* cameraQuad_);
    void close_panel();
    void refresh_panel_view();
    void set_material(s32 material_type);
    void set_underwater_material(s32 material_type);

    virtual void* getObj() {
        return &m_struct;
    }

    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);
    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor);
    void setRenderList(RenderList* renderList_);
    
    video::ITexture* getRender();
    video::ITexture* getRTT();
    video::ITexture* getRTT2();
private:

    int selection = -1;
    video::E_MATERIAL_TYPE render_material = video::EMT_SOLID;
    video::E_MATERIAL_TYPE underwater_material = video::EMT_SOLID;

    scene::ISceneManager* smgr = NULL;
    ReflectionTestPanel* view_panel = NULL;
    CameraQuad* cameraQuad = NULL;
    RenderList* renderList = NULL;

    //uv_editor_struct m_struct{ 0,true,true,true,true,16 };
    render_options_struct m_struct;

    friend class UV_Editor_Widget;
};

class Render_Tool
{
    static Render_Tool_Base* base;
    static multi_tool_panel* panel;

public:

    static void show()
    {
        panel->add_tool(base);
    }

    static void initialize(Render_Tool_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }

    static video::ITexture* getRender()
    {
        if (base)
        {
            return base->getRender();
        }
        return NULL;
    }

    static video::ITexture* getRTT()
    {
        if (base)
        {
            return base->getRTT();
        }
        return NULL;
    }

    static video::ITexture* getRTT2()
    {
        if (base)
        {
            return base->getRTT2();
        }
        return NULL;
    }
};

class WaterSurface_SceneNode : public ISceneNode, public ViewResizeObject
{
public:
    WaterSurface_SceneNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos, const core::vector3df rotation, geometry_scene*);
    ~WaterSurface_SceneNode();

    //virtual scene::ICameraSceneNode* getCamera();
    void resizeView(core::dimension2du newsize);

    virtual void render() override;
    virtual void OnRegisterSceneNode() override;
    virtual const core::aabbox3d<f32>& getBoundingBox() const override { return core::aabbox3d<f32>{}; }

    void set_material(s32 material_type) { render_material = (video::E_MATERIAL_TYPE)material_type; }
    void set_underwater_material(s32 material_type) { underwater_material = (video::E_MATERIAL_TYPE)material_type; }

    video::ITexture* getRTT() { return my_rtt; }
    video::ITexture* getRTT2() { return my_rtt2; }

private:

    video::E_MATERIAL_TYPE render_material = video::EMT_SOLID;
    video::E_MATERIAL_TYPE underwater_material = video::EMT_SOLID;
    video::ITexture* my_rtt = NULL;
    video::ITexture* my_rtt2 = NULL;
    scene::ICameraSceneNode* my_camera = NULL;

   // TwoTriangleSceneNode* render_node = NULL;

    geometry_scene* geo_scene = NULL;

    std::vector<int> hide_faces;
};

class Reflected_WaterSurfaceNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_WaterSurfaceNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);

    virtual bool bShowEditorArrow() { return true; }

    virtual void addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override;

    REFLECT2()
};

class Reflected_PointNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_PointNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);

    virtual void addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override {}

    REFLECT2()
};


class Reflected_TestNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_TestNode(USceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
        Reflected_Sprite_SceneNode(parent, smgr, id, pos) {}

    virtual void addSelfToScene(irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) {}
    bool bEnabled;
    int nParticles = 10;
    float velocity = 3.5;
    reflect::SomeOptions options;
    reflect::vector2 scale{ 1.0,1.0 };
    reflect::vector3 my_vec{ 5.0,4.0,3.0 };
    reflect::vector3 vec2{ 100.0,200.0,300.0 };
    reflect::color3 color;

    REFLECT2()
};


#endif
