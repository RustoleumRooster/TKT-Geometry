#ifndef _MY_NODES_H_
#define _MY_NODES_H_

#include <irrlicht.h>

using namespace irr;


class SkySprite : public scene::ISceneNode
{

	core::aabbox3d<f32> Box;
	video::S3DVertex Vertices[4];
	video::SMaterial Material;

public:

	SkySprite(f32 size, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);

	virtual void OnRegisterSceneNode();

	void SetTexture(video::ITexture* tex);

	virtual void render();
	virtual const core::aabbox3d<f32>& getBoundingBox() const;
	virtual u32 getMaterialCount();
	virtual video::SMaterial& getMaterial(u32 i);

};


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

class MySkyDomeNode : public scene::ISceneNode
{
    private:

	//	void generateMesh();

		scene::SMeshBuffer* Buffer;
		video::SMaterial Material;

	public:
		MySkyDomeNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id,scene::IMeshBuffer* mb);

		virtual ~MySkyDomeNode()
		{
		}
		virtual void OnRegisterSceneNode()
		{
        if (IsVisible)
            {
               SceneManager->registerNodeForRendering(this, scene::ESNRP_SKY_BOX );

            }

            ISceneNode::OnRegisterSceneNode();
		}
		virtual void render();
		void SetTexture(int n, video::ITexture* tex)
        {
            Buffer->Material.setTexture( n, tex);
        }
		virtual const core::aabbox3d<f32>& getBoundingBox() const
		{
		    return Buffer->BoundingBox;
		}
		virtual video::SMaterial& getMaterial(u32 i)
		{
            return Buffer->Material;
		}
		//virtual u32 getMaterialCount() const;
		virtual scene::ESCENE_NODE_TYPE getType() const { return scene::ESNT_SKY_DOME; }

	//	virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const;
	//	virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);
	//	virtual ISceneNode* clone(ISceneNode* newParent=0, ISceneManager* newManager=0);

	private:

	//	void generateMesh();

	//	u32 HorizontalResolution, VerticalResolution;
	//	f32 TexturePercentage, SpherePercentage, Radius;
};


class StretchPlaneSceneNode : public scene::ISceneNode
{

	core::aabbox3d<f32> Box;
	video::S3DVertex Vertices[4];
	video::SMaterial Material;


public:

	StretchPlaneSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
		: scene::ISceneNode(parent, mgr, id)
	{
		Material.Wireframe = false;
		Material.Lighting = false;
        Material.BackfaceCulling = true;

		//Material.ZBuffer = video::ECFN

		f32 z = 100;
		Vertices[0] = video::S3DVertex(-z,0,-z, 1,1,0,
				video::SColor(255,0,255,255), 0, 1);
		Vertices[1] = video::S3DVertex(-z,0,z, 1,0,0,
				video::SColor(255,255,0,255), 1, 1);
		Vertices[2] = video::S3DVertex(z,0,z, 0,1,1,
				video::SColor(255,255,255,0), 1, 0);
		Vertices[3] = video::S3DVertex(z,0,-z, 0,0,1,
				video::SColor(255,0,255,0), 0, 0);


	irr::scene::ISceneNode::setAutomaticCulling(irr::scene::EAC_OFF);// with irr::scene::EAC_OFF.
		Box.reset(Vertices[0].Pos);
		for (s32 i=1; i<4; ++i)
			Box.addInternalPoint(Vertices[i].Pos);
	}

	void setExtent(float leftx,float rightx, float botz, float topz)
	{
	    Vertices[0] = video::S3DVertex(leftx,0,botz, 1,1,0,
				video::SColor(255,255,255,255), leftx*0.0025, botz*0.0025);
		Vertices[1] = video::S3DVertex(leftx,0,topz, 1,0,0,
				video::SColor(255,255,255,255), leftx*0.0025, topz*0.0025);
		Vertices[2] = video::S3DVertex(rightx,0,topz, 0,1,1,
				video::SColor(255,255,255,255), rightx*0.0025, topz*0.0025);
		Vertices[3] = video::S3DVertex(rightx,0,botz, 0,0,1,
				video::SColor(255,255,255,255), rightx*0.0025, botz*0.0025);


	}

	virtual void OnRegisterSceneNode()
	{
		if (IsVisible)
			SceneManager->registerNodeForRendering(this,scene::ESNRP_SOLID);

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

	virtual void render();

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
