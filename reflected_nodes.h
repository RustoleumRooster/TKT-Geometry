#ifndef _REFLECTED_NODES_H_
#define _REFLECTED_NODES_H_

#include <irrlicht.h>
#include "CGUIWindow.h"
#include "Reflection.h"
#include "GUI_tools.h"
#include "reflect_custom_types.h"

using namespace irr;
using namespace scene;

class TestPanel;
class Reflected_SceneNode;
class geometry_scene;
class ListReflectedNodes_Base;
class multi_tool_panel;

namespace reflect
{
    struct TypeDescriptor_SN_Struct : TypeDescriptor_Struct
    {
        bool placeable;
        Reflected_SceneNode* (*create_func)(irr::scene::ISceneManager* smgr, int id, const irr::core::vector3df& pos);
        TypeDescriptor_SN_Struct(void (*init)(TypeDescriptor_Struct*));
    };

    #define REFLECT2() \
        virtual reflect::TypeDescriptor_Struct* GetDynamicReflection(); \
        friend struct reflect::DefaultResolver; \
        static reflect::TypeDescriptor_SN_Struct Reflection; \
        static void initReflection(reflect::TypeDescriptor_Struct*); \
        static Reflected_SceneNode* create_self(ISceneManager* smgr, int id, const core::vector3df& pos); \

    #define REFLECT_STRUCT2_BEGIN(type) \
        reflect::TypeDescriptor_SN_Struct type::Reflection{type::initReflection}; \
        Reflected_SceneNode* type::create_self(ISceneManager* smgr, int id, const core::vector3df& pos){ \
            return new type(smgr,id,pos);\
            }\
        reflect::TypeDescriptor_Struct* type::GetDynamicReflection() {\
            return &type::Reflection;\
            }\
        void type::initReflection(reflect::TypeDescriptor_Struct* typeDesc) { \
            using T = type; \
            typeDesc->name = #type; \
            typeDesc->size = sizeof(T); \
            typeDesc->inherited_type = NULL; \
            typeDesc->name_func = NULL; \
            typeDesc->alias = typeDesc->name; \
            ((reflect::TypeDescriptor_SN_Struct*)typeDesc)->placeable = true; \
            ((reflect::TypeDescriptor_SN_Struct*)typeDesc)->create_func = type::create_self;

    #define ALIAS(name) \
            typeDesc->alias = name;

    #define PLACEABLE(b) \
            ((reflect::TypeDescriptor_SN_Struct*)typeDesc)->placeable = b;

    #define INHERIT_FROM(name) \
            typeDesc->inherited_type = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<name>::get();

    #define REFLECT_STRUCT2_MEMBER(name) \
            typeDesc->members.push_back(reflect::Member{#name, offsetof(T, name), reflect::TypeResolver<decltype(T::name)>::get()});

    #define REFLECT_STRUCT2_END() \
    }

    struct vector2u
    {
        u32 X;
        u32 Y;

        void operator=(core::vector2d<u32> v) {
            X = v.X;
            Y = v.Y;
        }

        REFLECT()
    };

    struct vector2
    {
        f32 X;
        f32 Y;

        void operator=(core::vector2df v) {
            X = v.X;
            Y = v.Y;
        }

        REFLECT()
    };

    struct vector3
    {
        f32 X;
        f32 Y;
        f32 Z;

        void operator=(core::vector3df v) {
            X = v.X;
            Y = v.Y;
            Z = v.Z;
        }

        operator core::vector3df() const {
            core::vector3df ret;
            ret.X = X;
            ret.Y = Y;
            ret.Z = Z;
            return ret;
        }

        REFLECT()
    };
    struct SomeOptions
    {
        int value;
        REFLECT_MULTI_CHOICE()
    };

	struct color3
	{
        struct attributes
        {
        };

		char Blue;
		char Green;
		char Red;
		char Alpha;

        REFLECT_CUSTOM_STRUCT()
	};
}


class ListReflectedNodesWindow : public gui::IGUIElement
{
public:
    ListReflectedNodesWindow(gui::IGUIEnvironment* env, gui::IGUIElement* parent,ListReflectedNodes_Base*,s32 id,core::rect<s32> rect);
    ~ListReflectedNodesWindow();
    void click_OK();
    bool OnEvent(const SEvent& event);

    ListReflectedNodes_Base* base=NULL;
    int OK_BUTTON_ID;
    int LISTBOX_ID;
    int my_ID;

};


class ListReflectedNodes_Base : public tool_base
{
public:
    virtual void initialize(std::wstring name_,int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel*);
    virtual void show();
    reflect::TypeDescriptor_Struct* getSelectedTypeDescriptor();

private:
    std::vector<reflect::TypeDescriptor_Struct*> typeDescriptors;
    reflect::TypeDescriptor_Struct* selectedDescriptor = NULL;

    friend class ListReflectedNodesWindow;
};


class ListReflectedNodes_Tool
{
public:
    static void initialize(ListReflectedNodes_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }
    static void show()
    {
        panel->add_tool(base);
    }

    static ListReflectedNodes_Base* base;
    static multi_tool_panel* panel;
};


//
//========================================================================================
//  REFLECTED IRRLICHT SCENE NODES
//
//

class Reflected_SceneNode : public ISceneNode
{
public:
    Reflected_SceneNode(ISceneManager* smgr, int id, const core::vector3df& pos);

    virtual void render(){}
    void draw_arrow(video::IVideoDriver* driver, core::vector3df v, core::vector3df rot);
    void draw_box(video::IVideoDriver* driver, core::aabbox3df b);

    //Render to the node selection RTT
    virtual void render_special(video::SMaterial& material) {}

    virtual const core::aabbox3df& getBoundingBox() const;
    virtual void OnRegisterSceneNode();
    virtual ESCENE_NODE_TYPE getType() {return ESNT_UNKNOWN;}

    virtual void Select(){ bSelected=true; }
    virtual void UnSelect(){ bSelected=false; }
    virtual void translate(core::matrix4);
    virtual void rotate(core::vector3df);

    virtual void preEdit();
    virtual void postEdit();
    virtual bool bShowEditorArrow() {return false;}
    virtual void setUnlit(bool){}

    virtual void onClear() {}
    virtual void addSelfToScene(ISceneManager* smgr) {}

    static void SetBaseMaterialType(video::E_MATERIAL_TYPE m) {base_material_type=m;}
    static void SetSpecialMaterialType(video::E_MATERIAL_TYPE m) {special_material_type=m;}

    f32 getDistanceFromCamera(TestPanel* viewPanel);

    const static int icon_size=32;

    static video::E_MATERIAL_TYPE base_material_type;
    static video::E_MATERIAL_TYPE special_material_type;

    //Each Node instance has a unique color and a texture of this color
    video::SColor m_unique_color;
    video::ITexture* m_color_tex = NULL;

    //int ID;
    reflect::vector3 Location;
    reflect::vector3 Rotation;
    bool bSelected=false;

    REFLECT2()
};


class Reflected_Sprite_SceneNode : public Reflected_SceneNode
{
public:
    Reflected_Sprite_SceneNode(ISceneManager* smgr, int id, const core::vector3df& pos);

    virtual const core::aabbox3df& getBoundingBox() const;
    virtual void OnRegisterSceneNode();

    void set_buffer();
    virtual void render();
    virtual void render_special(video::SMaterial& material);

    virtual void Select();
    virtual void UnSelect();

    core::rect<s32> GetVisibleRectangle(TestPanel* viewPanel);

    video::ITexture* m_texture = NULL;
    scene::SMeshBuffer* Buffer = NULL;

    REFLECT2()
};

class Reflected_Model_SceneNode : public Reflected_SceneNode
{
public:
    Reflected_Model_SceneNode(ISceneManager* smgr, int id, const core::vector3df& pos);

    virtual const core::aabbox3df& getBoundingBox() const;
    virtual void OnRegisterSceneNode();

    virtual void preEdit();
    virtual void postEdit();

    virtual void setUnlit(bool);
    virtual void render();
    virtual void render_special(video::SMaterial& material);

    virtual bool bShowEditorArrow() {return true;}
    virtual bool isPlaceable() { return true; }

    video::ITexture* m_texture = NULL;
    core::aabbox3d<f32> Box;
    IMesh* Mesh;

    reflect::vector3 Scale;

    REFLECT2()
};

class Reflected_LightSceneNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_LightSceneNode(ISceneManager* smgr, int id, const core::vector3df& pos) :
        Reflected_Sprite_SceneNode(smgr,id,pos){}

    virtual bool bShowEditorArrow() {return true;}
    virtual ESCENE_NODE_TYPE getType() {return ESNT_LIGHT;}

    virtual void render(){Reflected_Sprite_SceneNode::render();}
    virtual void translate(core::matrix4);
    virtual void onClear() { my_light = NULL; }
    virtual void postEdit();
    virtual void addSelfToScene(ISceneManager* smgr);

    scene::ILightSceneNode* my_light=NULL;
    bool enabled=true;
    int light_radius=50;
    REFLECT2()
};

class Reflected_SimpleEmitterSceneNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_SimpleEmitterSceneNode(ISceneManager* smgr, int id, const core::vector3df& pos) :
        Reflected_Sprite_SceneNode(smgr,id,pos){}

    virtual bool bShowEditorArrow() {return true;}

    virtual void render();
    virtual void addSelfToScene(ISceneManager* smgr);

    core::vector3df EmitBox = core::vector3df(64,64,64);
    video::ITexture* texture = NULL;
    core::vector2df particle_scale = core::vector2df(10,10);
    int minParticlesPerSecond = 10;
    int maxParticlesPerSecond = 25;
    int lifeTimeMin = 500;
    int lifeTimeMax = 1000;
    bool bool_A;
    bool bool_B;
    video::SColor color;
    core::vector3df Vector_A = core::vector3df(64,64,64);
    core::vector3df Vector_B = core::vector3df(64,64,64);
    core::vector3df Vector_C = core::vector3df(64,64,64);

    REFLECT2()
};

class Reflected_PointNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_PointNode(ISceneManager* smgr, int id, const core::vector3df& pos);

    virtual void addSelfToScene(ISceneManager* smgr) {}

    REFLECT2()
};


class Reflected_TestNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_TestNode(ISceneManager* smgr, int id, const core::vector3df& pos) :
        Reflected_Sprite_SceneNode(smgr,id,pos){}

    virtual void addSelfToScene(ISceneManager* smgr) {}
    bool bEnabled;
    int nParticles=10;
    float velocity=3.5;
    reflect::SomeOptions options;
    reflect::vector2 scale{1.0,1.0};
    reflect::vector3 my_vec{5.0,4.0,3.0};
    reflect::vector3 vec2{100.0,200.0,300.0};
    reflect::color3 color;

    REFLECT2()
};

class Reflected_SceneNode_Factory
{
public:

    static void addType(reflect::TypeDescriptor_SN_Struct*);
    static int getNumTypes();
    static reflect::TypeDescriptor_Struct* getNodeTypeDescriptorByName(std::string name);
    static Reflected_SceneNode* CreateNodeByTypeName(std::string name, ISceneManager* smgr);
    static int getTypeNum(reflect::TypeDescriptor_Struct*);
    static std::vector<reflect::TypeDescriptor_Struct*> getAllTypes() {
        return SceneNode_Types;
    }
private:
    static std::vector<reflect::TypeDescriptor_Struct*> SceneNode_Types;
};


#endif
