
#include <irrlicht.h>
//#include "edit_classes.h"
#include "Reflection.h"

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

//--------------------------------------------------------
// A type descriptor for std::string
//--------------------------------------------------------

struct TypeDescriptor_StdString : TypeDescriptor {
    TypeDescriptor_StdString() : TypeDescriptor{"std::string", sizeof(std::string)} {
    }
    virtual void dump(const void* obj, int /* unused */) const override {
        std::cout << "std::string{\"" << *(const std::string*) obj << "\"}";
    }
    /*
    virtual void copy(void* obj, const void* obj2)
    {
        //std::cout << "Cannot copy string\n";
        //std::cout << "copying " << this->name << "\n";
        memcpy(obj, obj2, size);
        //std::string str("aa");
        //*((std::string*)obj) = str;// (std::string&&)(*(std::string*)obj2);
        //new(obj) std::string(*(std::string*)obj2);
        //std::cout << obj2 << " --->  " << obj << "\n";
        //std::cout << ((std::string*)obj2)->c_str() << "("<< ((std::string*)obj2)->length()<<")\n";
        //new(obj) std::string(((std::string*)obj2)->c_str());
    }*/
    /*
    virtual void serialize_flat(void** flat_obj, const void* obj)
    {
        //void* p = *flat_obj;
        //std::cout << " "<< obj << " -> " << p << ", +" << size << "\n";

        //copy(*flat_obj, obj);
        std::string* to_string = (std::string*) *flat_obj;
        const std::string* from_string = (std::string*) obj;

        new(to_string) std::string(from_string->c_str());

        *((char**)flat_obj) += size;
    }

    virtual void deserialize_flat(void* obj, void** flat_obj)
    {
        //void* p = *flat_obj;
        //std::cout << obj << " <- " << p << "\n";

       // copy(obj, *flat_obj);
        std::string* to_string = (std::string*)*flat_obj;
        const std::string* from_string = (std::string*)obj;

        to_string->operator=(*from_string);

        *((char**)flat_obj) += size;
    }
    *//*
    virtual void suicide(void* obj)
    {
        //((std::string*)obj)->std::string::~string();
    }*/
};

template <>
TypeDescriptor* getPrimitiveDescriptor<std::string>() {
    static TypeDescriptor_StdString typeDesc;
    return &typeDesc;
}

} // namespace reflect
