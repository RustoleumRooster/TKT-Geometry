#ifndef _REFLECT_CUSTOM_TYPES_
#define _REFLECT_CUSTOM_TYPES_

#include "Reflection.h"
namespace reflect
{


#define REFLECT_CUSTOM_PRIMITIVE() \
	struct my_typeDesc : reflect::TypeDescriptor_Struct { \
        my_typeDesc(void (*init)(reflect::TypeDescriptor_Struct*)) : reflect::TypeDescriptor_Struct(init) {} \
        virtual void addFormWidget(Reflected_GUI_Edit_Form* win, reflect::TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab); \
        virtual reflect::TypeDescriptor* get_flat_copy(void* obj, int indent) { \
            return this; \
            } \
        virtual void suicide() { \
        } \
        virtual reflect::TypeDescriptor_Struct* getNewEmptyStruct();\
        }; \
    friend struct reflect::DefaultResolver; \
    static my_typeDesc Reflection; \
    static void initReflection(reflect::TypeDescriptor_Struct*);


#define REFLECT_CUSTOM_PRIMITIVE_BEGIN(type) \
    type::my_typeDesc type::Reflection{type::initReflection}; \
    reflect::TypeDescriptor_Struct* type::my_typeDesc::getNewEmptyStruct() { \
        return NULL; \
        } \
    void type::initReflection(reflect::TypeDescriptor_Struct* typeDesc) { \
        using T = type; \
        typeDesc->name = #type; \
        typeDesc->size = sizeof(T); \
        typeDesc->inherited_type = NULL; \
        typeDesc->name_func = NULL; \
        typeDesc->alias = typeDesc->name; 


#define REFLECT_CUSTOM_STRUCT() \
	struct my_typeDesc : reflect::TypeDescriptor_Struct { \
        attributes my_attributes;\
        my_typeDesc(void (*init)(reflect::TypeDescriptor_Struct*)) : reflect::TypeDescriptor_Struct(init) {} \
        virtual void addFormWidget(Reflected_GUI_Edit_Form* win, reflect::TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab); \
        virtual reflect::TypeDescriptor* get_flat_copy(void* obj, int indent) { \
            return TypeDescriptor_Struct::get_flat_copy(obj,indent); \
            } \
        virtual reflect::TypeDescriptor_Struct* getNewEmptyStruct();\
        }; \
    friend struct reflect::DefaultResolver; \
    static my_typeDesc Reflection; \
    static void initReflection(reflect::TypeDescriptor_Struct*);


#define REFLECT_CUSTOM_STRUCT_BEGIN(type) \
    type::my_typeDesc type::Reflection{type::initReflection}; \
    reflect::TypeDescriptor_Struct* type::my_typeDesc::getNewEmptyStruct() { \
            return new type::my_typeDesc(reflect::EmptyStruct::initReflection); \
        } \
    void type::initReflection(reflect::TypeDescriptor_Struct* typeDesc) { \
        using T = type; \
        typeDesc->name = #type; \
        typeDesc->size = sizeof(T); \
        typeDesc->inherited_type = NULL; \
        typeDesc->name_func = NULL; \
        typeDesc->alias = typeDesc->name; 


}
#endif
