
#include <irrlicht.h>
//#include "edit_classes.h"
#include "Reflection.h"
#include "GeometryStack.h"

namespace reflect {



REFLECT_STRUCT_BEGIN(EmptyStruct)
REFLECT_STRUCT_END()

void TypeDescriptor_MultiOption::dump(const void* obj, int indentLevel) const
{
    int i = *(const int*)obj;

    std::cout<<name<<" {";
    if(i<labels.size())
        std::cout<<labels[i];
    else std::cout<<"out of range";
    std::cout<<"}";
}

//--------------------------------------------------------
// A type descriptor for int
//--------------------------------------------------------

template <>
TypeDescriptor* getPrimitiveDescriptor<float>() {
    static TypeDescriptor_Float typeDesc;
    return &typeDesc;
}

template <>
TypeDescriptor* getPrimitiveDescriptor<int>() {
    static TypeDescriptor_Int typeDesc;
    return &typeDesc;
}

template <>
TypeDescriptor* getPrimitiveDescriptor<unsigned int>() {
    static TypeDescriptor_U32 typeDesc;
    return &typeDesc;
}


template <>
TypeDescriptor* getPrimitiveDescriptor<char>() {
    static TypeDescriptor_Byte typeDesc;
    return &typeDesc;
}


template <>
TypeDescriptor* getPrimitiveDescriptor<bool>() {
    static TypeDescriptor_Bool typeDesc;
    return &typeDesc;
}

template <>
TypeDescriptor* getPrimitiveDescriptor<irr::core::vector2df>() {
    static TypeDescriptor_Vector2 typeDesc;
    return &typeDesc;
}


template <>
TypeDescriptor* getPrimitiveDescriptor<irr::core::dimension2du>() {
    static TypeDescriptor_Dimension2 typeDesc;
    return &typeDesc;
}

template <>
TypeDescriptor* getPrimitiveDescriptor<irr::core::vector3df>() {
    static TypeDescriptor_Vector3 typeDesc;
    return &typeDesc;
}


template <>
TypeDescriptor* getPrimitiveDescriptor<irr::core::matrix4>() {
    static TypeDescriptor_Matrix4 typeDesc;
    return &typeDesc;
}


template <>
TypeDescriptor* getPrimitiveDescriptor<irr::video::SColor>() {
    static TypeDescriptor_SColor typeDesc;
    return &typeDesc;
}


template <>
TypeDescriptor* getPrimitiveDescriptor<irr::video::ITexture*>() {
    static TypeDescriptor_Texture typeDesc;
    return &typeDesc;
}


template <>
TypeDescriptor* getPrimitiveDescriptor<unsigned short>() {
    static TypeDescriptor_U16 typeDesc;
    return &typeDesc;
}

template <>
TypeDescriptor* getPrimitiveDescriptor<char*>() {
    static TypeDescriptor_Pointer typeDesc;
    return &typeDesc;
}

template <>
TypeDescriptor* getPrimitiveDescriptor<unsigned long long>() {
    static TypeDescriptor_U64 typeDesc;
    return &typeDesc;
}

//--------------------------------------------------------
// A type descriptor for std::string
//--------------------------------------------------------

struct TypeDescriptor_StdString : TypeDescriptor {
    TypeDescriptor_StdString() : TypeDescriptor{"std::string", sizeof(std::string)} {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        const std::string* str = (const std::string*)obj;
        std::cout << "std::string{\"" << str->c_str() << "\"}";
    }

    virtual void serialize_flat(void** flat_obj, const void* obj)
    {
        std::string* to_str = (std::string*) *flat_obj;
        const std::string* from_str = (std::string*)obj;

        new(to_str) std::string(*from_str);

        *((char**)flat_obj) += aligned_size();
    }

    virtual void deserialize_flat(void* obj, void** flat_obj)
    {
        *((char**)flat_obj) += aligned_size();
    }

    virtual void suicide(void* obj)
    {
        std::string* str = (std::string*) obj;
        str->std::string::~string();
    }
};

template <>
TypeDescriptor* getPrimitiveDescriptor<std::string>() {
    static TypeDescriptor_StdString typeDesc;
    return &typeDesc;
}

} // namespace reflect
