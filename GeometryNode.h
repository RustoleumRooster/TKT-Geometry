#ifndef _GEOMETRY_NODE_
#define _GEOMETRY_NODE_

#include <irrlicht.h>

using namespace irr;
using namespace scene;

class GeometryNode : public ISceneNode
{
	public:
		GeometryNode(video::ITexture* texture,
			ISceneNode* parent, ISceneManager* smgr, s32 id);
		virtual ~GeometryNode();
		virtual void OnRegisterSceneNode();
		virtual void render();
		virtual const core::aabbox3d<f32>& getBoundingBox() const;
		virtual video::SMaterial& getMaterial(u32 i);
		virtual u32 getMaterialCount() const;
		virtual ESCENE_NODE_TYPE getType() const { return scene::ESNT_MESH; }

		//virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const;
		//virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);
		//virtual ISceneNode* clone(ISceneNode* newParent=0, ISceneManager* newManager=0);

	//private:

        SMeshBuffer* Buffer;
        core::aabbox3d<f32> Box;
        video::SMaterial Material;

	//	IMesh* Mesh;

};


#endif
