#ifndef _REFLECTION2_H_
#define _REFLECTION2_H_

#include <vector>
#include <iostream>
#include <string>
#include <cstddef>
#include <fstream>

#include <irrlicht.h>

//class Reflected_GUI_Edit_Form;

class Reflected_GUI_Edit_Form;
class Reflected_SceneNode;

#define ALIGN_BYTES 8


namespace reflect {

//--------------------------------------------------------
// Base class of all type descriptors
//--------------------------------------------------------

struct TypeDescriptor_Struct;

struct TypeDescriptor {

    const char* name;
    const char* alias;
    size_t size;

    TypeDescriptor(const char* name, size_t size) : name{ name }, alias{ NULL }, size{ size } {}
    virtual ~TypeDescriptor() {}
    virtual std::string getFullName() const { return name; }

    virtual void dump(const void* obj, int indentLevel = 0) const = 0;

    size_t aligned_size() {
        return ALIGN_BYTES * ((size / ALIGN_BYTES) + (size % ALIGN_BYTES == 0 ? 0 : 1));
    }

    virtual void addFormWidget(Reflected_GUI_Edit_Form*, 
        TypeDescriptor_Struct*, //The struct which owns this member (NULL for top level)
        std::vector<int> tree,  //position of the member within the TypeDescriptor "Tree" structure
        size_t offset,          //offset of the owner struct if not top level (add to member offset)
        bool bVisible,          //currently visible or not
        bool bEditable,         //create edit or non-edit form widget
        int tab)                //how far to tab in
        {
    }

    virtual bool expandable(){return false;}

    virtual void serialize(std::ofstream& f, const void* obj)
    {
        f.write((char*)obj,size);
    }

    virtual void deserialize(std::ifstream& f, void* obj)
    {
        f.read((char*)obj,size);
    }

    virtual bool isEqual(const void* obj, const void* obj2)
    {
        return memcmp(obj,obj2,size)==0;
    }

    virtual TypeDescriptor* getDynamicTempType()
    {
        return NULL;
    }

    virtual void suicide(void* obj)
    {

    }

    //this can fail and cause crash for container classes !!!
    virtual void serialize_flat(void** flat_obj, const void* obj)
    {
        //void* p = *flat_obj;
        //std::cout << " "<< obj << " -> " << p << ", +" << size << "\n";

        copy(*flat_obj, obj);
        *((char**)flat_obj) += aligned_size();
    }

    //this can fail and cause crash for container classes !!!
    virtual void deserialize_flat(void* obj, void** flat_obj)
    {
        void* p = *flat_obj;
        //std::cout << obj << " <- " << p << "\n";

        copy(obj, *flat_obj);
        *((char**)flat_obj) += aligned_size();
    }

    virtual TypeDescriptor* get_flat_copy(void* obj, int indent)
    {
        //for (int i = 0; i < indent; i++)
        //    std::cout << "  ";
        //std::cout << name << ", size = "<<size<<"\n";
        return this;
    }

    virtual std::string getAlias()
    {
        return std::string(alias);
    }
//private:
    //a very dangerous function
    virtual void copy(void* obj, const void* obj2)
    {
        memcpy(obj, obj2, size);
    }
};

//--------------------------------------------------------
// Finding type descriptors
//--------------------------------------------------------

// Declare the function template that handles primitive types such as int, std::string, etc.:
template <typename T>
TypeDescriptor* getPrimitiveDescriptor();

// A helper class to find TypeDescriptors in different ways:
struct DefaultResolver {
    template <typename T> static char func(decltype(&T::Reflection));
    template <typename T> static int func(...);
    template <typename T>
    struct IsReflected {
        enum { value = (sizeof(func<T>(nullptr)) == sizeof(char)) };
    };

    // This version is called if T has a static member named "Reflection":
    template <typename T, typename std::enable_if<IsReflected<T>::value, int>::type = 0>
    static TypeDescriptor* get() {\
            return &T::Reflection;
    }

    // This version is called otherwise:
    template <typename T, typename std::enable_if<!IsReflected<T>::value, int>::type = 0>
    static TypeDescriptor* get() {
        return getPrimitiveDescriptor<T>();
    }
};

// This is the primary class template for finding all TypeDescriptors:
template <typename T>
struct TypeResolver {
    static TypeDescriptor* get() {
        return DefaultResolver::get<T>();
    }
};


struct EmptyStruct
{
    friend struct reflect::DefaultResolver; 
    static reflect::TypeDescriptor_Struct Reflection; 
    static void initReflection(reflect::TypeDescriptor_Struct*);
};

//--------------------------------------------------------
// Type descriptors for user-defined structs/classes
//--------------------------------------------------------

#define FLAG_NON_EDITABLE 1
#define FLAG_NON_COLLAPSABLE 2

struct Member {
         struct Attribute {
            int A;
            int B;
            };
        const char* name;
        size_t offset;
        TypeDescriptor* type;
        std::vector<Attribute> attributes;
        bool expanded;   //for editor widgets
        bool readwrite;  //for editor widgets
        bool modified;   //for editor widgets
        char flags = 0;

        void* get(const void* obj)
        {
            return (char*)obj+offset;
        }

        bool isEqual(const void* obj, const void* obj2)
        {
            return type->isEqual((char*)obj+offset,(char*)obj2+offset);
        }

        /*
        void copy(void* obj, const void* obj2)
        {
            type->copy((char*)obj+offset,(char*)obj2+offset);
        }*/

    };

struct TypeDescriptor_Struct : TypeDescriptor {

    TypeDescriptor_Struct* inherited_type;
    std::string (*name_func)(void);
    std::vector<Member> members;
    bool expanded;

    TypeDescriptor_Struct(void (*init)(TypeDescriptor_Struct*)) : TypeDescriptor{nullptr, 0} {
        init(this);
    }

    virtual ~TypeDescriptor_Struct()
    {
        //std::cout << this->name << " out of scope\n";
    }

   // TypeDescriptor_Struct(const char* name, size_t size, const std::initializer_list<Member>& init) : TypeDescriptor{nullptr, 0}, members{init} {
   // }
    virtual void dump(const void* obj, int indentLevel) const override {
        std::cout << name <<" "<<obj<<" {" << std::endl;
        for (const Member& member : members) {
            std::cout << std::string(4 * (indentLevel + 1), ' ') << member.name << " = ";
            member.type->dump((char*) obj + member.offset, indentLevel + 1);
            std::cout<<"("<<member.offset<<")";
            std::cout << std::endl;
        }
        std::cout << std::string(4 * indentLevel, ' ') << "}";
    }

    size_t getTreeNodeOffset(std::vector<int> tree_pos)
    {
        size_t ret = 0;

        if (tree_pos.size() > 0)
        {
            reflect::TypeDescriptor_Struct* t = this;

            for (int i = 0; i < tree_pos.size() - 1; i++)
            {
                int m_i = tree_pos[i];

                ret += t->members[m_i].offset;

                t = dynamic_cast<reflect::TypeDescriptor_Struct*>(t->members[m_i].type);
            }

            int m_i = tree_pos[tree_pos.size() - 1];
            ret += t->members[m_i].offset;
        }

        return ret;
    }

    Member* getTreeNode(std::vector<int> tree_pos)
    {
        if(tree_pos.size() > 0)
        {
            reflect::TypeDescriptor_Struct* t = this;

            for(int i=0; i<tree_pos.size()-1; i++)
            {
                int m_i = tree_pos[i];

                if (m_i < t->members.size())
                {
                    t = dynamic_cast<reflect::TypeDescriptor_Struct*>(t->members[m_i].type);

                    if (!t)
                        return NULL;
                }
            }
            int m_0 = tree_pos[tree_pos.size()-1];

            if (t && m_0 < t->members.size())
                return &t->members[m_0];
            else return NULL;
            
        }
        else
            return NULL;
    }

    virtual void addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab);

    virtual void serialize(std::ofstream& f, const void* obj)
    {
        for (const Member& member : members)
        {
            member.type->serialize(f,(char*) obj + member.offset);
        }
    }

    virtual void deserialize(std::ifstream& f, void* obj)
    {
        for (const Member& member : members)
        {
            member.type->deserialize(f,(char*) obj + member.offset);
        }
    }

    virtual TypeDescriptor* getDynamicTempType()
    {
        return this;
    }

    virtual void suicide(void* obj)
    {
        for (int i = 0; i < members.size(); i++)
        {
             members[i].type->suicide((char*)obj + members[i].offset);
        }

        //std::cout << "delete " << this << "\n";
        delete this;
    }

    virtual TypeDescriptor* get_flat_copy(void* obj, int indent)
    {
        //for (int i = 0; i < indent; i++)
        //    std::cout << "  ";
        //std::cout << name << "\n";
        

        //TypeDescriptor_Struct* ret = new TypeDescriptor_Struct(EmptyStruct::initReflection);
        TypeDescriptor_Struct* ret = getNewEmptyStruct();
        

        //for (int i = 0; i < indent; i++)
        //    std::cout << "  ";
        //std::cout << " " << ret->name << "\n";
        //std::cout << "{\n";

        ret->size = 0;
        ret->alias = "";

        size_t p_inc = 0;

        for (int i = 0; i < members.size(); i++)
        {
            //std::cout << "member " << i << "\n";
            //for (int i = 0; i < indent; i++)
            //    std::cout << "  ";
            //std::cout << p_inc << " = offset \n";

            TypeDescriptor* flat_type = members[i].type->get_flat_copy((char*)obj + members[i].offset, indent + 1);

            if (flat_type)
            {
                Member m{ members[i].name, p_inc, flat_type };
                
                m.flags = members[i].flags;
                m.expanded = members[i].expanded;
                m.modified = false;

                ret->members.push_back(m);

                size_t aligned_size = ALIGN_BYTES * ((m.type->size / ALIGN_BYTES) + (m.type->size % ALIGN_BYTES == 0 ? 0 : 1));
                
                ret->size += aligned_size;
                p_inc += aligned_size;
            }

        }
        ret->expanded = expanded;

        //for (int i = 0; i < indent; i++)
        //    std::cout << "  ";
        //std::cout << "} size = " << ret->size << "\n";

        //std::cout<<"new empty td (struct) @ "<<ret<<"\n";

        return ret;
    }

    virtual void serialize_flat(void** flat_obj, const void* obj)
    {
	//void* p = *flat_obj;
	//std::cout << obj << " -> " << p << "\n";
        for (int i = 0; i < members.size(); i++)
        {
            members[i].type->serialize_flat(flat_obj, (char*)obj + members[i].offset);
        }
    }

    virtual void deserialize_flat(void* obj, void** flat_obj)
    {
        for (int i = 0; i < members.size(); i++)
        {
            members[i].type->deserialize_flat((char*)obj + members[i].offset, flat_obj );
        }

    }

    virtual TypeDescriptor_Struct* getNewEmptyStruct()
    {
        TypeDescriptor_Struct* ret = new TypeDescriptor_Struct(EmptyStruct::initReflection);
        return ret;
    }

    virtual std::string getAlias()
    {
        return std::string(alias);
    }
};



 
//=================

#define REFLECT() \
    friend struct reflect::DefaultResolver; \
    static reflect::TypeDescriptor_Struct Reflection; \
    static void initReflection(reflect::TypeDescriptor_Struct*);

#define REFLECT_STRUCT_BEGIN(type) \
    reflect::TypeDescriptor_Struct type::Reflection{type::initReflection}; \
    void type::initReflection(reflect::TypeDescriptor_Struct* typeDesc) { \
        using T = type; \
        typeDesc->name = #type; \
        typeDesc->size = sizeof(T); \
        typeDesc->inherited_type = NULL; \
        typeDesc->name_func = NULL; \
        typeDesc->alias = typeDesc->name; 
        //typeDesc->members = {

#define REFLECT_STRUCT_MEMBER(name) \
        typeDesc->members.push_back(reflect::Member{#name, offsetof(T, name), reflect::TypeResolver<decltype(T::name)>::get()});
        //   {#name, offsetof(T, name), reflect::TypeResolver<decltype(T::name)>::get()},

#define REFLECT_STRUCT_MEMBER_FLAG(flag) \
        typeDesc->members[typeDesc->members.size()-1].flags |= flag;

#define REFLECT_STRUCT_END() \
    }
    //}; \


//=========================

#define REFLECT_MULTI_CHOICE() \
    friend struct reflect::DefaultResolver; \
    static reflect::TypeDescriptor_MultiOption Reflection; \
    static void initReflection(reflect::TypeDescriptor_MultiOption*);

#define REFLECT_MULTI_STRUCT_BEGIN(type) \
    reflect::TypeDescriptor_MultiOption type::Reflection{type::initReflection}; \
    void type::initReflection(reflect::TypeDescriptor_MultiOption* typeDesc) { \
    using T = type; \
    typeDesc->name = #type; \
    typeDesc->size = sizeof(T); \
    typeDesc->labels = {

#define REFLECT_MULTI_STRUCT_LABEL(name) \
            name,

#define REFLECT_MULTI_STRUCT_END() \
        }; \
    }


//--------------------------------------------------------
// Type descriptors for std::vector
//--------------------------------------------------------


struct TypeDescriptor_MultiOption : TypeDescriptor {

    std::vector<std::string> labels;

    TypeDescriptor_MultiOption(void (*init)(TypeDescriptor_MultiOption*)) : TypeDescriptor{nullptr, 0} {
        init(this);
    }

    virtual void dump(const void* obj, int indentLevel) const override;
    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override;
};

struct TypeDescriptor_StdVector : TypeDescriptor {
    TypeDescriptor* itemType;
    size_t (*getSize)(const void*);
    const void* (*getItem)(const void*, size_t);
    void* (*getItem_nc)(void*, size_t);
    void (*allocate)(void*, size_t);

    template <typename ItemType>
    TypeDescriptor_StdVector(ItemType*)
        : TypeDescriptor{"std::vector<>", sizeof(std::vector<ItemType>)},
                         itemType{TypeResolver<ItemType>::get()} {
        getSize = [](const void* vecPtr) -> size_t {
            const auto& vec = *(const std::vector<ItemType>*) vecPtr;
            return vec.size();
        };
        getItem = [](const void* vecPtr, size_t index) -> const void* {
            const auto& vec = *(const std::vector<ItemType>*) vecPtr;
            return &vec[index];
        };
        getItem_nc = [](void* vecPtr, size_t index) -> void* {
            auto& vec = *(std::vector<ItemType>*) vecPtr;
            return &vec[index];
        };
        allocate = [](void* vecPtr, size_t new_size) {
            auto& vec = *(std::vector<ItemType>*) vecPtr;
            vec.resize(new_size);
        };
    }
    virtual std::string getFullName() const override {
        return std::string("std::vector<") + itemType->getFullName() + ">";
    }
    virtual void dump(const void* obj, int indentLevel) const override {
        size_t numItems = getSize(obj);
        std::cout << getFullName();
        if (numItems == 0) {
            std::cout << "{}";
        } else {
            std::cout << "{" << std::endl;
            for (size_t index = 0; index < numItems; index++) {
                std::cout << std::string(4 * (indentLevel + 1), ' ') << "[" << index << "] ";
                itemType->dump(getItem(obj, index), indentLevel + 1);
                std::cout << std::endl;
            }
            std::cout << std::string(4 * indentLevel, ' ') << "}";
        }
    }
    virtual void serialize(std::ofstream& f, const void* obj) override
    {
        size_t numItems = getSize(obj);

        f.write((char*)&numItems,sizeof(size_t));
        for (size_t index = 0; index < numItems; index++)
        {
            itemType->serialize(f,getItem(obj, index));
        }
    }

    virtual void deserialize(std::ifstream& f, void* obj) override
    {
        size_t numItems;
        f.read((char*)&numItems,sizeof(size_t));

        allocate(obj,numItems);
        for (size_t index = 0; index < numItems; index++)
        {
            itemType->deserialize(f,getItem_nc(obj, index));
        }
    }

    size_t getNumElements(void* obj)
    {
        return getSize(obj);
    }

    virtual void serialize_flat(void** flat_obj, const void* obj)
    {
        size_t numItems = getSize(obj);

        for (size_t index = 0; index < numItems; index++)
        {
            itemType->serialize_flat(flat_obj, getItem(obj, index));
        }
    }

    virtual void deserialize_flat(void* obj, void** flat_obj)
    {
        size_t numItems = getSize(obj);

        for (size_t index = 0; index < numItems; index++)
        {
            itemType->deserialize_flat(getItem_nc(obj, index), flat_obj);
        }
    }

    virtual void copy(void* obj, const void* obj2)
    {
        size_t numItems = getSize(obj2);

        allocate(obj, numItems);
        for (size_t index = 0; index < numItems; index++)
        {
            itemType->copy(getItem_nc(obj,index),getItem(obj2,index));
        }
    }

    virtual TypeDescriptor* get_flat_copy(void* obj, int indent)
    {
       // for (int i = 0; i < indent; i++)
       //     std::cout << "  ";
       // std::cout << name << ", " << getSize(obj) << " " << itemType->getAlias() << " items in container\n";
       //
       // for (int i = 0; i < indent; i++)
       //     std::cout << "  ";
       // std::cout << "{\n";

        //std::cout << "vector::get_flat_copy, "<< itemType->name<< "\n";

        TypeDescriptor_Struct* ret = new TypeDescriptor_Struct(EmptyStruct::initReflection);
        //TypeDescriptor_Struct* ret = getNewEmptyStruct();
        ret->alias = "";
        ret->size = 0;

        size_t numItems = getSize(obj);
        size_t p_inc = 0;

        TypeDescriptor* flat_type = NULL;

        for (int i = 0; i < numItems; i++)
        {
            flat_type = itemType->get_flat_copy(getItem_nc(obj, i), indent+1);
            if (flat_type)
            {
                Member m{ "item", p_inc, flat_type };
                //for (int i = 0; i < indent; i++)
                //    std::cout << "  ";
                //std::cout << (flat_type->size * i) << " = offset \n";
                m.modified = false;

                ret->members.push_back(m);
                p_inc += (size_t)flat_type->size;
                //ret->size += (size_t)flat_type->size;
            }
        }

        ret->size = p_inc;

        //for (int i = 0; i < indent; i++)
        //    std::cout << "  ";
        
        if (ret->size == 0)
        {
            //std::cout << "} size = 0 (nothing created)\n";
            //std::cout << "empty vector, not creating type\n";
            //delete ret;
            //ret = NULL;
            
        }
        else
        {
            //ret->size = (size_t)(flat_type->size * numItems);
            //std::cout << "} size = " << ret->size << " \n";
            //std::cout << "new empty td (vector) @ " << ret << "\n";
            //std::cout << "  " << numItems << " items, type = " << itemType->name << "\n";
        }

        return ret;
    }

    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override;
};

// Partially specialize TypeResolver<> for std::vectors:
template <typename T>
class TypeResolver<std::vector<T>> {
public:
    static TypeDescriptor* get() {
        static TypeDescriptor_StdVector typeDesc{(T*) nullptr};
        return &typeDesc;
    }
};

struct TypeDescriptor_Float : TypeDescriptor {
    TypeDescriptor_Float() : TypeDescriptor{"float", sizeof(irr::f32)} {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "float{" << *(const irr::f32*) obj << "}";
    }
    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override;

    virtual bool isEqual(void* obj, void* obj2) {
        return *(const irr::f32*) obj == *(const irr::f32*)obj2;
        }
};

struct TypeDescriptor_Int : TypeDescriptor {
    TypeDescriptor_Int() : TypeDescriptor{"int", sizeof(int)} {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "int{" << *(const int*) obj << "}";
    }

    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override;

    virtual bool isEqual(void* obj, void* obj2) {
        return *(const int*) obj == *(const int*)obj2;
        }
};


struct TypeDescriptor_U16 : TypeDescriptor {
    TypeDescriptor_U16() : TypeDescriptor{ "u16", sizeof(unsigned short) } {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "u16{" << *(const unsigned short*)obj << "}";
    }

    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override
    {}

    virtual bool isEqual(void* obj, void* obj2) {
        return *(const unsigned short*)obj == *(const unsigned short*)obj2;
    }
};

struct TypeDescriptor_U32 : TypeDescriptor {
    TypeDescriptor_U32() : TypeDescriptor{ "u32", sizeof(unsigned int) } {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "u32{" << *(const unsigned int*)obj << "}";
    }

    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override
    {}

    virtual bool isEqual(void* obj, void* obj2) {
        return *(const unsigned int*)obj == *(const unsigned int*)obj2;
    }
};


struct TypeDescriptor_Byte : TypeDescriptor {
    TypeDescriptor_Byte() : TypeDescriptor{"char", sizeof(char)} {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "char{" << *(const char*) obj << "}";
    }

    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override;

    virtual bool isEqual(void* obj, void* obj2) {
        return *(const char*) obj == *(const char*)obj2;
        }
};

struct TypeDescriptor_Bool : TypeDescriptor {
    TypeDescriptor_Bool() : TypeDescriptor{"bool", sizeof(bool)} {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "bool{" << *(const bool*) obj << "}";
    }

    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override;
    
    virtual bool isEqual(void* obj, void* obj2) {
        return *(const bool*) obj == *(const bool*)obj2;
        }
};

struct TypeDescriptor_Vector2 : TypeDescriptor {
    TypeDescriptor_Vector2() : TypeDescriptor{"vector2", sizeof(irr::core::vector2df)} {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "vector2{ not implemented - " << *(const bool*) obj << "}";
    }

    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override
    {}
};

struct TypeDescriptor_Vector3 : TypeDescriptor {
    TypeDescriptor_Vector3() : TypeDescriptor{"vector3", sizeof(irr::core::vector3df)} {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "vector3{" <<  *(const float*) (char*)obj << "," <<
                                    *(const float*) ((char*)obj + sizeof(float)) << "," <<
                                    *(const float*) ((char*)obj + sizeof(float)) << "," <<
                                    *(const float*) ((char*)obj + sizeof(float)*2)<<
                                                        "}";
    }

    virtual bool expandable(){return true;}
    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override
    {}
};


struct TypeDescriptor_Dimension2 : TypeDescriptor {
    TypeDescriptor_Dimension2() : TypeDescriptor{ "dimension2", sizeof(irr::core::dimension2du) } {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "dimension2{" << *(const unsigned*)(char*)obj << "," <<
            *(const unsigned*)((char*)obj + sizeof(unsigned)) <<
            "}";
    }

    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override
    {}
};

struct TypeDescriptor_Matrix4 : TypeDescriptor {
    TypeDescriptor_Matrix4() : TypeDescriptor{"matrix4", sizeof(irr::core::matrix4)} {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "Matrix4{" <<  *(const float*) (char*)obj << "," <<
                                    *(const float*) ((char*)obj + sizeof(float)) << "," <<
                                    *(const float*) ((char*)obj + sizeof(float)*2)<<
                                                        "}";
    }

    virtual bool expandable(){return true;}
    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override
    {}
};


struct TypeDescriptor_SColor : TypeDescriptor {
    TypeDescriptor_SColor() : TypeDescriptor{"SColor", sizeof(irr::video::SColor)} {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "SColor{" << *(const char*) obj << "}";
    }

    virtual bool expandable(){return true;}
    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override
    {}
};

struct TypeDescriptor_Texture : TypeDescriptor {
    TypeDescriptor_Texture() : TypeDescriptor{"Texture", sizeof(irr::video::ITexture*)} {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "Texture{" << *(const irr::video::ITexture**) obj << "}";
    }

    virtual bool expandable(){return true;}
    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override;
};

struct TypeDescriptor_Pointer : TypeDescriptor {
    TypeDescriptor_Pointer() : TypeDescriptor{ "ptr", sizeof(char*) } {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "ptr{" << *(const unsigned short*)obj << "}";
    }

    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override
    {}

};

struct TypeDescriptor_U64 : TypeDescriptor {
    TypeDescriptor_U64() : TypeDescriptor{ "u64", sizeof(unsigned long long) } {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "u64{" << *(const unsigned long long*)obj << "}";
    }

    virtual void addFormWidget(Reflected_GUI_Edit_Form*, TypeDescriptor_Struct*, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab) override
    {}

    virtual bool isEqual(void* obj, void* obj2) {
        return *(const unsigned long long*)obj == *(const unsigned long long*)obj2;
    }
};

} // namespace reflect

#endif
