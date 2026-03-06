#pragma once
#ifndef _TKT_REFLECTION_FACTORY_H_
#define _TKT_REFLECTION_FACTORY_H_

#include <vector>
#include <string>
#include "Reflection.h"

// Generic reflection factory for any TypeDescriptor_Struct subtype.
// T must inherit from reflect::TypeDescriptor_Struct.
//
// Usage:
//   using Vulkan_Reflection_Factory = tkt_reflection_factory<TypeDescriptor_VkMod_Struct>;
//
template<typename T>
class tkt_reflection_factory
{
public:
    using descriptor_type = T;

    void addType(T* typeDesc)
    {
        Types.push_back(typeDesc);
    }

    int getNumTypes()
    {
        return (int)Types.size();
    }

    reflect::TypeDescriptor_Struct* getNodeTypeDescriptorByName(std::string name)
    {
        for (T* typeDesc : Types)
        {
            if (typeDesc->name == name)
                return typeDesc;
        }
        return nullptr;
    }

    int getTypeNum(reflect::TypeDescriptor_Struct* td)
    {
        for (int i = 0; i < (int)Types.size(); i++)
        {
            if (Types[i] == td)
                return i;
        }
        return 0;
    }

    std::vector<reflect::TypeDescriptor_Struct*> getAllTypes()
    {
        return std::vector<reflect::TypeDescriptor_Struct*>(Types.begin(), Types.end());
    }

private:
    std::vector<T*> Types;
};

#endif
