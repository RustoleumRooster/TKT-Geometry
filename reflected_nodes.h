#ifndef _REFLECTED_NODES_H_
#define _REFLECTED_NODES_H_

#include <irrlicht.h>
#include "CGUIWindow.h"
#include "Reflection.h"
#include "GUI_tools.h"
#include "reflect_custom_types.h"
#include "USceneNode.h"

using namespace irr;
using namespace scene;

class TestPanel;
class Reflected_SceneNode;
class geometry_scene;
class ListReflectedNodes_Base;
class multi_tool_panel;
class USceneNode;

namespace reflect
{
    struct TypeDescriptor_SN_Struct : TypeDescriptor_Struct
    {
        bool placeable;
        Reflected_SceneNode* (*create_func)(USceneNode* parent, geometry_scene*, irr::scene::ISceneManager* smgr, int id, const irr::core::vector3df& pos);
        TypeDescriptor_SN_Struct(void (*init)(TypeDescriptor_Struct*));
    };

    #define REFLECT2() \
        virtual reflect::TypeDescriptor_Struct* GetDynamicReflection(); \
        friend struct reflect::DefaultResolver; \
        static reflect::TypeDescriptor_SN_Struct Reflection; \
        static void initReflection(reflect::TypeDescriptor_Struct*); \
        static Reflected_SceneNode* create_self(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos); \

    #define REFLECT_STRUCT2_BEGIN(type) \
        reflect::TypeDescriptor_SN_Struct type::Reflection{type::initReflection}; \
        Reflected_SceneNode* type::create_self(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos){ \
            return new type(parent, geo_scene, smgr, id, pos);\
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

    struct vector2i
    {
        int X;
        int Y;

        void operator=(core::vector2d<int> v) {
            X = v.X;
            Y = v.Y;
        }

        REFLECT()
    };

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

    struct uid_reference
    {
        struct attributes
        {
        };

        std::vector<u64> uids;
        std::vector<u64> old_uids; //not reflected

       // REFLECT()
        REFLECT_CUSTOM_STRUCT()
    };

    /*
    struct TypeDescriptor_UID_Reference : TypeDescriptor {
        TypeDescriptor_UID_Reference() : TypeDescriptor{ "uid reference", sizeof(uid_reference) } {
        }
        virtual void dump(const void* obj, int ) const override {
            std::cout << "uid{" << *(const unsigned long long*)obj << "}";
        }

        virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override;

    };*/
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

class Reflected_SceneNode : public USceneNode
{
public:
    Reflected_SceneNode(USceneNode* parent, geometry_scene*, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);

    virtual void render(){}
    void draw_arrow(video::IVideoDriver* driver, core::vector3df v, core::vector3df rot);
    void draw_box(video::IVideoDriver* driver, core::aabbox3df b);
    void draw_inputs(video::IVideoDriver* driver);

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

    virtual void endScene() {}
    virtual bool addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) { return false; }

    static void SetBaseMaterialType(video::E_MATERIAL_TYPE m) {base_material_type=m;}
    static void SetSpecialMaterialType(video::E_MATERIAL_TYPE m) {special_material_type=m;}

    template<typename T>
    void resolve_uid_references(const reflect::uid_reference&, std::vector<T*>&);

    int get_node_instance_id() { return node_instance_id; }

    void connect_input(Reflected_SceneNode*);
    virtual void disconnect(Reflected_SceneNode*);

    f32 getDistanceFromCamera(TestPanel* viewPanel);

//protected:

    void rebuild_input_ptrs_list();

    geometry_scene* geo_scene = NULL;

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

    int node_instance_id=0;

    std::vector<u64> input_nodes;
    std::vector<Reflected_SceneNode*> input_node_ptrs;

    REFLECT2()
};


class Reflected_Sprite_SceneNode : public Reflected_SceneNode
{
public:
    Reflected_Sprite_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);

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
    Reflected_Model_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);

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

class Reflected_MeshBuffer_SceneNode : public Reflected_Sprite_SceneNode
{
public:
    Reflected_MeshBuffer_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);

    virtual bool addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene) override;

    IMeshBuffer* get_mesh_buffer();
    void restore_original_texture();

    void set_uid(u64);
    u64 get_uid();
private:
    u64 face_uid;

    REFLECT2()
};

template<typename T>
void Reflected_SceneNode::resolve_uid_references(const reflect::uid_reference& uid_ref, std::vector<T*>& ptr_ref)
{
    ptr_ref.clear();

    core::list<scene::ISceneNode*> child_list = geo_scene->EditorNodes()->getChildren();

    for (scene::ISceneNode* inode : child_list)
    {
        Reflected_SceneNode* node = (Reflected_SceneNode*)inode;
        for (u64 uid : uid_ref.uids)
        {
            if (node->UID() == uid)
            {
                T* t_node = dynamic_cast<T*>(node);

                if (t_node != NULL)
                    ptr_ref.push_back(t_node);
            }
        }
    }
}

//========================================================================================
//  FACTORY CLASS
//
//

class Reflected_SceneNode_Factory
{
public:

    static void addType(reflect::TypeDescriptor_SN_Struct*);
    static int getNumTypes();
    static reflect::TypeDescriptor_Struct* getNodeTypeDescriptorByName(std::string name);
    static Reflected_SceneNode* CreateNodeByTypeName(std::string name, USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr);
    static int getTypeNum(reflect::TypeDescriptor_Struct*);
    static std::vector<reflect::TypeDescriptor_Struct*> getAllTypes() {
        return SceneNode_Types;
    }
private:
    static std::vector<reflect::TypeDescriptor_Struct*> SceneNode_Types;
};


#endif
