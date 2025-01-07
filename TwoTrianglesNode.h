#ifndef _TWO_TRIANGLES_NODE_
#define _TWO_TRIANGLES_NODE_

#include <irrlicht.h>

using namespace irr;

class TwoTriangleSceneNode : public scene::ISceneNode
{
	core::aabbox3d<f32> Box;
	video::S3DVertex Vertices[4];
	video::SMaterial Material;

public:

	TwoTriangleSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
		: scene::ISceneNode(parent, mgr, id)
	{
		Material.Wireframe = false;
		Material.Lighting = false;

		//Material.ZBuffer = video::ECFN_ALWAYS;

		f32 z = 1;

		Vertices[0] = video::S3DVertex(-z,-z,0, 1,1,0,
				video::SColor(255,0,255,255), 0, 1);
		Vertices[1] = video::S3DVertex(-z,z,0, 1,0,0,
				video::SColor(255,255,0,255), 1, 1);
		Vertices[2] = video::S3DVertex(z,z,0, 0,1,1,
				video::SColor(255,255,255,0), 1, 0);
		Vertices[3] = video::S3DVertex(z,-z,0, 0,0,1,
				video::SColor(255,0,255,0), 0, 0);

		irr::scene::ISceneNode::setAutomaticCulling(irr::scene::EAC_OFF);// with irr::scene::EAC_OFF.

		Box.reset(Vertices[0].Pos);
		for (s32 i=1; i<4; ++i)
			Box.addInternalPoint(Vertices[i].Pos);
	}

	void defaultUVs()
	{
		f32 z = 1;

		Vertices[0] = video::S3DVertex(-z, -z, 0, 1, 1, 0,
			video::SColor(255, 0, 255, 255), 0, 0);
		Vertices[1] = video::S3DVertex(-z, z, 0, 1, 0, 0,
			video::SColor(255, 255, 0, 255), 0, 1);
		Vertices[2] = video::S3DVertex(z, z, 0, 0, 1, 1,
			video::SColor(255, 255, 255, 0), 1, 1);
		Vertices[3] = video::S3DVertex(z, -z, 0, 0, 0, 1,
			video::SColor(255, 0, 255, 0), 1, 0);
	}

	void mirrorUVs()
	{
		f32 z = 1;

		Vertices[0] = video::S3DVertex(-z, -z, 0, 1, 1, 0,
			video::SColor(255, 0, 255, 255), 0, 1);
		Vertices[1] = video::S3DVertex(-z, z, 0, 1, 0, 0,
			video::SColor(255, 255, 0, 255), 0, 0);
		Vertices[2] = video::S3DVertex(z, z, 0, 0, 1, 1,
			video::SColor(255, 255, 255, 0), 1, 0);
		Vertices[3] = video::S3DVertex(z, -z, 0, 0, 0, 1,
			video::SColor(255, 0, 255, 0), 1, 1);

	}

	virtual void OnRegisterSceneNode()
	{
		if (IsVisible)
            SceneManager->registerNodeForRendering(this, scene::ESNRP_SOLID );
			//SceneManager->registerNodeForRendering(this,ESNRP_SKY_BOX);

		ISceneNode::OnRegisterSceneNode();
	}
	void SetTexture(int n, video::ITexture* tex)
	{
	    Material.setTexture( n, tex);
	}
	void SetVisible(bool bVis)
	{
        this->IsVisible = bVis;
	}

	virtual void render()
	{
		u16 indices[] = {	0,2,3, 2,1,3, 1,0,3, 2,0,1	};
		video::IVideoDriver* driver = SceneManager->getVideoDriver();
		scene::ICameraSceneNode* camera = SceneManager->getActiveCamera();

		core::matrix4 translate(AbsoluteTransformation);

		driver->setTransform(video::ETS_WORLD, translate );
		driver->setMaterial(Material);
		driver->drawVertexPrimitiveList(&Vertices[0], 4, &indices[0], 4, video::EVT_STANDARD, scene::EPT_TRIANGLES, video::EIT_16BIT);
	}

	virtual const core::aabbox3d<f32>& getBoundingBox() const
	{
		return Box;
	}

	virtual u32 getMaterialCount() const
	{
		return 1;
	}

	virtual video::SMaterial& getMaterial(u32 i)
	{
		return Material;
	}
};

#endif
