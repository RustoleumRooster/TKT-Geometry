// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
/*
#include "CMeshSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "S3DVertex.h"
#include "ICameraSceneNode.h"
#include "IMeshCache.h"
#include "IAnimatedMesh.h"
#include "IMaterialRenderer.h"
#include "IFileSystem.h"
#include "CShadowVolumeSceneNode.h"
*/
#include <irrlicht.h>
#include "CMeshSceneNode.h"
#include <iostream>
#include "utils.h"

namespace irr
{
namespace scene
{

video::SColor CMeshSceneNode::m_unique_color = video::SColor(255,0,0,0);
video::ITexture* CMeshSceneNode::m_color_tex = NULL;

void CMeshSceneNode::initialize_unique_color(video::IVideoDriver* driver)
{
    video::IImage* img = makeSolidColorImage(driver,CMeshSceneNode::m_unique_color);
    CMeshSceneNode::m_color_tex = driver->addTexture(irr::io::path("hit_color"), img);
    img->drop();
}

//! constructor
CMeshSceneNode::CMeshSceneNode(IMesh* mesh, ISceneNode* parent, ISceneManager* mgr, s32 id,
			const core::vector3df& position, const core::vector3df& rotation,
			const core::vector3df& scale)
: IMeshSceneNode(parent, mgr, id, position, rotation, scale), Mesh(0), Shadow(0),
	PassCount(0), ReadOnlyMaterials(false), bWireframe(false)
{
	#ifdef _DEBUG
	setDebugName("CMeshSceneNode");
	#endif
	//std::cout << "new mesh node!\n";

	setMesh(mesh);

	//DebugDataVisible=true;
	//bWireframe = true;

}


//! destructor
CMeshSceneNode::~CMeshSceneNode()
{
	//std::cout << "MESH NODE OUT OF SCOPE\n";
	if (Shadow)
		Shadow->drop();
	if (Mesh)
		Mesh->drop();
}


//! frame
void CMeshSceneNode::OnRegisterSceneNode()
{
	if (IsVisible)
	{
		// because this node supports rendering of mixed mode meshes consisting of
		// transparent and solid material at the same time, we need to go through all
		// materials, check of what type they are and register this node for the right
		// render pass according to that.

		video::IVideoDriver* driver = SceneManager->getVideoDriver();

		PassCount = 0;
		int transparentCount = 0;
		int solidCount = 0;

		// count transparent and solid materials in this scene node
		if (ReadOnlyMaterials && Mesh)
		{
			// count mesh materials

			for (u32 i=0; i<Mesh->getMeshBufferCount(); ++i)
			{
				scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
				video::IMaterialRenderer* rnd = mb ? driver->getMaterialRenderer(mb->getMaterial().MaterialType) : 0;

				if (rnd && rnd->isTransparent())
					++transparentCount;
				else
					++solidCount;

				if (solidCount && transparentCount)
					break;
			}
		}
		else
		{
			// count copied materials

			for (u32 i=0; i<Materials.size(); ++i)
			{
				video::IMaterialRenderer* rnd =
					driver->getMaterialRenderer(Materials[i].MaterialType);

				if (rnd && rnd->isTransparent())
					++transparentCount;
				else
					++solidCount;

				if (solidCount && transparentCount)
					break;
			}
		}

		// register according to material types counted

		if (solidCount)
			SceneManager->registerNodeForRendering(this, scene::ESNRP_SOLID);

		if (transparentCount)
			SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT);

		ISceneNode::OnRegisterSceneNode();
	}
}


void CMeshSceneNode::render_special(video::SMaterial& material)
{
    video::IVideoDriver* driver = SceneManager->getVideoDriver();

	if (!Mesh || !driver)
		return;

    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

    for (u32 i=0; i<Mesh->getMeshBufferCount(); ++i)
		{
			scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
			if (mb)
			{
				//const video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];
				material.BackfaceCulling = mb->getMaterial().BackfaceCulling;
                material.setTexture(0,m_color_tex);
                //material.setTexture(0,mb->getMaterial().getTexture(0));

                driver->setMaterial(material);
                driver->drawMeshBuffer(mb);
			}
		}
}

void CMeshSceneNode::render_with_material(video::SMaterial& material)
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	if (!Mesh || !driver)
		return;

	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

	for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
		if (mb)
		{
			//const video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];
			material.BackfaceCulling = mb->getMaterial().BackfaceCulling;
			material.FrontfaceCulling = mb->getMaterial().FrontfaceCulling;

			material.setTexture(0, mb->getMaterial().getTexture(0));
			material.setTexture(1, mb->getMaterial().getTexture(1));

			driver->setMaterial(material);
			driver->drawMeshBuffer(mb);
		}
	}
}


void CMeshSceneNode::render_special(int buffer_i, video::SMaterial& material, video::ITexture* texture)
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	if (!Mesh || !driver)
		return;

	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

	scene::IMeshBuffer* mb = Mesh->getMeshBuffer(buffer_i);
	if (mb)
	{
		//const video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];
		material.BackfaceCulling = mb->getMaterial().BackfaceCulling;

		if(texture)
			material.setTexture(0, texture);
		//material.setTexture(0,mb->getMaterial().getTexture(0));

		driver->setMaterial(material);
		driver->drawMeshBuffer(mb);
	}
}

void CMeshSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	if (!Mesh || !driver)
		return;



	bool isTransparentPass =
		SceneManager->getSceneNodeRenderPass() == scene::ESNRP_TRANSPARENT;

	++PassCount;

	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
	//Box = Mesh->getBoundingBox();

	if (Shadow && PassCount==1)
		Shadow->updateShadowVolumes();

	// for debug purposes only:

	bool renderMeshes = true;
	video::SMaterial mat;
	if (DebugDataVisible && PassCount==1)
	{
		// overwrite half transparency
		if (DebugDataVisible & scene::EDS_HALF_TRANSPARENCY)
		{
			for (u32 g=0; g<Mesh->getMeshBufferCount(); ++g)
			{
				mat = Materials[g];
				mat.MaterialType = video::EMT_TRANSPARENT_ADD_COLOR;
				driver->setMaterial(mat);
				driver->drawMeshBuffer(Mesh->getMeshBuffer(g));
			}
			renderMeshes = false;
		}
	}

	// render original meshes
	if (renderMeshes && !bWireframe)
	{
		for (u32 i=0; i<Mesh->getMeshBufferCount(); ++i)
		{
			scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
			if (mb)
			{

				video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];

				video::IMaterialRenderer* rnd = driver->getMaterialRenderer(material.MaterialType);
				bool transparent = (rnd && rnd->isTransparent());

				// only render transparent buffer if this is the transparent render pass
				// and solid only in solid pass
				if (transparent == isTransparentPass)
				{
					driver->setMaterial(material);
					driver->drawMeshBuffer(mb);
				}
			}
		}
	}
	else if(bWireframe)
	{
	    video::SMaterial m;
		m.Lighting = false;
        m.Wireframe = true;
        driver->setMaterial(m);

        for (u32 g=0; g<Mesh->getMeshBufferCount(); ++g)
        {
            driver->drawMeshBuffer(Mesh->getMeshBuffer(g));
        }
	}

	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

	// for debug purposes only:
	if (DebugDataVisible && PassCount==1)
	{
		video::SMaterial m;
		m.Lighting = false;
		m.AntiAliasing=0;
		driver->setMaterial(m);

		if (DebugDataVisible & scene::EDS_BBOX)
		{
			driver->draw3DBox(Box, video::SColor(255,255,255,255));
		}
		if (DebugDataVisible & scene::EDS_BBOX_BUFFERS)
		{
			for (u32 g=0; g<Mesh->getMeshBufferCount(); ++g)
			{
				driver->draw3DBox(
					Mesh->getMeshBuffer(g)->getBoundingBox(),
					video::SColor(255,190,128,128));
			}
		}

		if (DebugDataVisible & scene::EDS_NORMALS)
		{
			// draw normals
			const f32 debugNormalLength = 32;// SceneManager->getParameters()->getAttributeAsFloat(DEBUG_NORMAL_LENGTH);
			const video::SColor debugNormalColor = SceneManager->getParameters()->getAttributeAsColor(DEBUG_NORMAL_COLOR);
			const u32 count = Mesh->getMeshBufferCount();

			for (u32 i=0; i != count; ++i)
			{
				driver->drawMeshBufferNormals(Mesh->getMeshBuffer(i), debugNormalLength, debugNormalColor);
			}
		}

		// show mesh
		if (DebugDataVisible & scene::EDS_MESH_WIRE_OVERLAY)
		{
			m.Wireframe = true;
			driver->setMaterial(m);

			for (u32 g=0; g<Mesh->getMeshBufferCount(); ++g)
			{
				driver->drawMeshBuffer(Mesh->getMeshBuffer(g));
			}
		}
	}
}

void CMeshSceneNode::SetFaceTexture(int f_i, video::ITexture* texture)
{
    if(f_i < this->Mesh->getMeshBufferCount())
    {
        ((scene::SMeshBuffer*)this->Mesh->getMeshBuffer(f_i))->Material.setTexture(0,texture);
    }
    copyMaterials();
}


void CMeshSceneNode::SetFaceMaterialType(int f_i, video::E_MATERIAL_TYPE new_material)
{
    if(f_i < this->Mesh->getMeshBufferCount())
    {
        ((scene::SMeshBuffer*)this->Mesh->getMeshBuffer(f_i))->Material.MaterialType = new_material;

        //this->Mesh->getMeshBuffer(f_i)->
    }
    copyMaterials();
}


//! Removes a child from this scene node.
//! Implemented here, to be able to remove the shadow properly, if there is one,
//! or to remove attached childs.
bool CMeshSceneNode::removeChild(ISceneNode* child)
{
	if (child && Shadow == child)
	{
		Shadow->drop();
		Shadow = 0;
	}

	return ISceneNode::removeChild(child);
}


//! returns the axis aligned bounding box of this node
const core::aabbox3d<f32>& CMeshSceneNode::getBoundingBox() const
{
	//return Mesh ? Mesh->getBoundingBox() : Box;
	//Mesh->
	return Box;
}


//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hierarchy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
video::SMaterial& CMeshSceneNode::getMaterial(u32 i)
{
	if (Mesh && ReadOnlyMaterials && i<Mesh->getMeshBufferCount())
	{
		ReadOnlyMaterial = Mesh->getMeshBuffer(i)->getMaterial();
		return ReadOnlyMaterial;
	}

	if (i >= Materials.size())
		return ISceneNode::getMaterial(i);

	return Materials[i];
}


//! returns amount of materials used by this scene node.
u32 CMeshSceneNode::getMaterialCount() const
{
	if (Mesh && ReadOnlyMaterials)
		return Mesh->getMeshBufferCount();

	return Materials.size();
}


//! Sets a new mesh
void CMeshSceneNode::setMesh(IMesh* mesh)
{
	if (mesh)
	{
		mesh->grab();
		if (Mesh)
			Mesh->drop();

		Mesh = mesh;
		copyMaterials();

     //   Box.addInternalPoint(core::vector3df(100,100,100));
	//	Box.addInternalPoint(core::vector3df(-100,-100,-100));
/*
		if(Mesh->getMeshBufferCount()>0)
        {
            video::S3DVertex* ptr = (video::S3DVertex*)Mesh->getMeshBuffer(0)->getVertices();
            Box.reset(ptr[0].Pos);

            for(int i=0; i< Mesh->getMeshBufferCount(); i++)
            {

                video::S3DVertex* ptr = (video::S3DVertex*)Mesh->getMeshBuffer(i)->getVertices();
                for(int j=0; j< Mesh->getMeshBuffer(0)->getVertexCount(); j++)
                {
                    //std::cout<<ptr[j].Pos.X<<" "<<ptr[j].Pos.Y<<"\n";
                    Box.addInternalPoint(Mesh->getMeshBuffer(0)->Vertices[j].Pos);
                }
            }
        }
*/
	}

}

void CMeshSceneNode::SetBox(core::aabbox3df box)
{
 Box = box;
}



void CMeshSceneNode::copyMaterials()
{
	Materials.clear();

	if (Mesh)
	{
		video::SMaterial mat;

		for (u32 i=0; i<Mesh->getMeshBufferCount(); ++i)
		{
			IMeshBuffer* mb = Mesh->getMeshBuffer(i);
			if (mb)
				mat = mb->getMaterial();

			Materials.push_back(mat);
		}
	}
}


//! Writes attributes of the scene node.
void CMeshSceneNode::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	IMeshSceneNode::serializeAttributes(out, options);

	if (options && (options->Flags&io::EARWF_USE_RELATIVE_PATHS) && options->Filename)
	{
		const io::path path = SceneManager->getFileSystem()->getRelativeFilename(
				SceneManager->getFileSystem()->getAbsolutePath(SceneManager->getMeshCache()->getMeshName(Mesh).getPath()),
				options->Filename);
		out->addString("Mesh", path.c_str());
	}
	else
		out->addString("Mesh", SceneManager->getMeshCache()->getMeshName(Mesh).getPath().c_str());
	out->addBool("ReadOnlyMaterials", ReadOnlyMaterials);
}


//! Reads attributes of the scene node.
void CMeshSceneNode::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	io::path oldMeshStr = SceneManager->getMeshCache()->getMeshName(Mesh);
	io::path newMeshStr = in->getAttributeAsString("Mesh");
	ReadOnlyMaterials = in->getAttributeAsBool("ReadOnlyMaterials");

	if (newMeshStr != "" && oldMeshStr != newMeshStr)
	{
		IMesh* newMesh = 0;
		IAnimatedMesh* newAnimatedMesh = SceneManager->getMesh(newMeshStr.c_str());

		if (newAnimatedMesh)
			newMesh = newAnimatedMesh->getMesh(0);

		if (newMesh)
			setMesh(newMesh);
	}

	// optional attribute to assign the hint to the whole mesh
	if (in->existsAttribute("HardwareMappingHint") &&
		in->existsAttribute("HardwareMappingBufferType"))
	{
		scene::E_HARDWARE_MAPPING mapping = scene::EHM_NEVER;
		scene::E_BUFFER_TYPE bufferType = scene::EBT_NONE;

		core::stringc smapping = in->getAttributeAsString("HardwareMappingHint");
		if (smapping.equals_ignore_case("static"))
			mapping = scene::EHM_STATIC;
		else if (smapping.equals_ignore_case("dynamic"))
			mapping = scene::EHM_DYNAMIC;
		else if (smapping.equals_ignore_case("stream"))
			mapping = scene::EHM_STREAM;

		core::stringc sbufferType = in->getAttributeAsString("HardwareMappingBufferType");
		if (sbufferType.equals_ignore_case("vertex"))
			bufferType = scene::EBT_VERTEX;
		else if (sbufferType.equals_ignore_case("index"))
			bufferType = scene::EBT_INDEX;
		else if (sbufferType.equals_ignore_case("vertexindex"))
			bufferType = scene::EBT_VERTEX_AND_INDEX;

		IMesh* mesh = getMesh();
		if (mesh)
			mesh->setHardwareMappingHint(mapping, bufferType);
	}

	IMeshSceneNode::deserializeAttributes(in, options);
}


//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
/* In this way it is possible to change the materials a mesh causing all mesh scene nodes
referencing this mesh to change too. */
void CMeshSceneNode::setReadOnlyMaterials(bool readonly)
{
	ReadOnlyMaterials = readonly;
}


//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
bool CMeshSceneNode::isReadOnlyMaterials() const
{
	return ReadOnlyMaterials;
}


//! Creates a clone of this scene node and its children.
ISceneNode* CMeshSceneNode::clone(ISceneNode* newParent, ISceneManager* newManager)
{
	if (!newParent)
		newParent = Parent;
	if (!newManager)
		newManager = SceneManager;

	CMeshSceneNode* nb = new CMeshSceneNode(Mesh, newParent,
		newManager, ID, RelativeTranslation, RelativeRotation, RelativeScale);

	nb->cloneMembers(this, newManager);
	nb->ReadOnlyMaterials = ReadOnlyMaterials;
	nb->Materials = Materials;
	nb->Shadow = Shadow;
	if ( nb->Shadow )
		nb->Shadow->grab();

	if (newParent)
		nb->drop();
	return nb;
}


} // end namespace scene
} // end namespace irr

