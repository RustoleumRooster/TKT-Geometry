#pragma once

#ifndef _USCENENODE_H_
#define _USCENENODE_H_

#include <irrlicht.h>

using namespace irr;

class USceneNode : public scene::ISceneNode
{
public:
    USceneNode(scene::ISceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos);

    virtual void render() { }
    virtual const core::aabbox3df& getBoundingBox() const { return core::aabbox3df(); }

    u64 UID() const { return my_UID; }

protected:
    u64 my_UID;
};



#endif