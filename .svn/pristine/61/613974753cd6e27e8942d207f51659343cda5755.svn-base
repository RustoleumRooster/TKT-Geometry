
#include <irrlicht.h>
#include "GeometryNode.h"


GeometryNode::GeometryNode(video::ITexture* texture,
			ISceneNode* parent, ISceneManager* smgr, s32 id)
			: ISceneNode(parent, smgr, id)
{


}

GeometryNode::~GeometryNode()
{
}

void GeometryNode::OnRegisterSceneNode()
{
    if (IsVisible)
        SceneManager->registerNodeForRendering(this);

    ISceneNode::OnRegisterSceneNode();
}


//! renders the node.
void GeometryNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

}

const core::aabbox3d<f32>& GeometryNode::getBoundingBox() const
	{
		return Box;
	}

u32 GeometryNode::getMaterialCount() const
	{
		return 1;
	}

video::SMaterial& GeometryNode::getMaterial(u32 i)
	{
		return Material;
	}
