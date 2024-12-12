
#include <irrlicht.h>
#include "driverChoice.h"
#include <stdlib.h>
#include "myNodes.h"

SkySprite::SkySprite(f32 size, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
		: scene::ISceneNode(parent, mgr, id)
	{
		Material.Wireframe = false;
		//Material.Lighting = false;

        Material.Lighting = false;
        Material.ZBuffer = video::ECFN_NEVER;
        Material.ZWriteEnable = false;
        Material.AntiAliasing = video::EAAM_OFF;

		f32 z = size;
		Vertices[0] = video::S3DVertex(-z,-z,0, 1,1,0,
				video::SColor(255,0,255,255), 0, 1);
		Vertices[1] = video::S3DVertex(-z,z,0, 1,0,0,
				video::SColor(255,255,0,255), 1, 1);
		Vertices[2] = video::S3DVertex(z,z,0, 0,1,1,
				video::SColor(255,255,255,0), 1, 0);
		Vertices[3] = video::S3DVertex(z,-z,0, 0,0,1,
				video::SColor(255,0,255,0), 0, 0);


	//irr::scene::ISceneNode::setAutomaticCulling(irr::scene::EAC_OFF);// with irr::scene::EAC_OFF.

		Box.reset(Vertices[0].Pos);
		for (s32 i=1; i<4; ++i)
			Box.addInternalPoint(Vertices[i].Pos);
	}
void SkySprite::OnRegisterSceneNode()
	{
		if (IsVisible)
			SceneManager->registerNodeForRendering(this, ESNRP_SKY_BOX );

		ISceneNode::OnRegisterSceneNode();
	}
void SkySprite::SetTexture(ITexture* tex)
	{
	    Material.setTexture( 0, tex);
	}

void SkySprite::render()
	{
		//u16 indices[] = {	0,2,3, 2,1,3, 1,0,3, 2,0,1	};
		u16 indices[] = {	0,2,3, 2,0,1	};
		video::IVideoDriver* driver = SceneManager->getVideoDriver();
		scene::ICameraSceneNode* camera = SceneManager->getActiveCamera();



		core::matrix4 translate(AbsoluteTransformation);
		//translate.setTranslation(camera->getAbsolutePosition());
        translate.setTranslation(camera->getAbsolutePosition()+this->getPosition());
		// Draw the sky box between the near and far clip plane
		//const f32 viewDistance = (camera->getNearValue() + camera->getFarValue()) * 0.5f;
		//core::matrix4 scale;
		//scale.setScale(core::vector3df(viewDistance, viewDistance, viewDistance));

		driver->setTransform(video::ETS_WORLD, translate );
		//driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
		//driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
		driver->setMaterial(Material);
		driver->drawVertexPrimitiveList(&Vertices[0], 4, &indices[0], 2, video::EVT_STANDARD, scene::EPT_TRIANGLES, video::EIT_16BIT);
	}

const core::aabbox3d<f32>& SkySprite::getBoundingBox() const
	{
		return Box;
	}

u32 SkySprite::getMaterialCount()
	{
		return 1;
	}

video::SMaterial& SkySprite::getMaterial(u32 i)
	{
		return Material;
	}

//--------------------------------------------------------
//
//--------------------------------------------------------


MySkyDomeNode:: MySkyDomeNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id,scene::IMeshBuffer* mb)
		: scene::ISceneNode(parent, mgr, id)
		{
		    //Buffer=mesh->getMeshBuffer(0);
		    Buffer = new SMeshBuffer();
		    Buffer->setHardwareMappingHint(scene::EHM_STATIC);


            setAutomaticCulling(scene::EAC_OFF);
           // std::cout<<"buffer: "<<Buffer->getIndexCount()<<"\n";
            Buffer->Material.Lighting = false;
            //Buffer->Material.ZBuffer = video::ECFN_NEVER;
           // Buffer->Material.ZWriteEnable = false;
            Buffer->Material.AntiAliasing = video::EAAM_OFF;
         //    Buffer->Material.Wireframe = true;
//            Material.setTexture(0, sky);
            Buffer->BoundingBox.MaxEdge.set(0,0,0);
            Buffer->BoundingBox.MinEdge.set(0,0,0);
           // Buffer->addMeshBuffer(mb);
         //  std::cout<<"MESH:  "<<Buffer->getVertexCount()<<"   "<<Buffer->getIndexCount()<<"\n";
            Buffer->append(mb->getVertices(),mb->getVertexCount(),mb->getIndices(),mb->getIndexCount());
            RelativeScale = core::vector3df(100,100,100);
          //generateMesh();
            std::cout<<"MESH:  "<<Buffer->getVertexCount()<<"   "<<Buffer->getIndexCount()<<"\n";

          //  irr::scene::IMeshBuffer::get
		}


//--------------------------------------------------------
//
//--------------------------------------------------------



void MySkyDomeNode::render()
		{
		    video::IVideoDriver* driver = SceneManager->getVideoDriver();
            scene::ICameraSceneNode* camera = SceneManager->getActiveCamera();

            if (!camera || !driver)
                return;

            if ( !camera->isOrthogonal() )
            {
                core::matrix4 mat(AbsoluteTransformation);
                mat.setTranslation(camera->getAbsolutePosition());

                driver->setTransform(video::ETS_WORLD, mat);

                driver->setMaterial(Buffer->Material);
                driver->drawMeshBuffer(Buffer);
            }
		}

void StretchPlaneSceneNode::render()
	{
		u16 indices[] = {	0,2,3, 2,1,3, 1,0,3, 2,0,1	};
		video::IVideoDriver* driver = SceneManager->getVideoDriver();
		scene::ICameraSceneNode* camera = SceneManager->getActiveCamera();

		core::matrix4 translate(AbsoluteTransformation);

		driver->setTransform(video::ETS_WORLD, translate );
		driver->setMaterial(Material);
		driver->drawVertexPrimitiveList(&Vertices[0], 4, &indices[0], 4, video::EVT_STANDARD, scene::EPT_TRIANGLES, video::EIT_16BIT);

	}
