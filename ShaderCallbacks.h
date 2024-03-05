#ifndef _ShaderCallbacks_H_
#define _ShaderCallbacks_H_

#include <irrlicht.h>

extern IrrlichtDevice* device;

#define MAT_COUT(M) std::cout<<M[0]<<" "<<M[1]<<" "<<M[2]<<" "<<M[3]<<"\n"; \
					std::cout<<M[4]<<" "<<M[5]<<" "<<M[6]<<" "<<M[7]<<"\n"; \
					std::cout<<M[8]<<" "<<M[9]<<" "<<M[10]<<" "<<M[11]<<"\n"; \
					std::cout<<M[12]<<" "<<M[13]<<" "<<M[14]<<" "<<M[15]<<"\n"; 

class MyShaderCallBack : public video::IShaderConstantSetCallBack
{
public:

	virtual void OnSetConstants(video::IMaterialRendererServices* services,
		s32 userData)
	{
		video::IVideoDriver* driver = services->getVideoDriver();

		// set inverted world matrix
		// if we are using highlevel shaders (the user can select this when
		// starting the program), we must set the constants by name.

		core::matrix4 invWorld = driver->getTransform(video::ETS_WORLD);
		//core::matrix4 invWorld = mWorld.makeInverse();

		services->setVertexShaderConstant("mInvWorld", invWorld.pointer(), 16);

		// set clip matrix

		core::matrix4 worldView = driver->getTransform(video::ETS_VIEW);
		worldView *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant("mWorldView", worldView.pointer(), 16);

		core::matrix4 Proj = driver->getTransform(video::ETS_PROJECTION);
		services->setVertexShaderConstant("mProj", Proj.pointer(), 16);
		/*
		core::matrix4 worldView2 = mView_;
		worldView2 *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant("mWorldView2", worldView2.pointer(), 16);

		core::matrix4 Proj2 = mProj_;
		services->setVertexShaderConstant("mProj2", Proj2.pointer(), 16);
		*/

		// set camera position

		core::vector3df pos = device->getSceneManager()->
			getActiveCamera()->getAbsolutePosition();
		//core::vector3df pos(3.0f,3.0f,3.0f);

		services->setVertexShaderConstant("mLightPos", reinterpret_cast<f32*>(&pos), 3);

		// set light color

		video::SColorf col(1.0f, 1.0f, 1.0f, 0.0f);

		services->setVertexShaderConstant("mLightColor", reinterpret_cast<f32*>(&col), 4);

		// set transposed world matrix

		core::matrix4 world = driver->getTransform(video::ETS_WORLD);
		world = world.getTransposed();


		services->setVertexShaderConstant("mTransWorld", world.pointer(), 16);

		// set texture, for textures you can use both an int and a float setPixelShaderConstant interfaces (You need it only for an OpenGL driver).
		s32 TextureLayerID = 0;

		services->setPixelShaderConstant("myTexture", &TextureLayerID, 1);

		TextureLayerID = 1;

		services->setPixelShaderConstant("myTexture2", &TextureLayerID, 1);

	}
};

class MyShaderCallBack2 : public video::IShaderConstantSetCallBack
{
public:

	virtual void OnSetConstants(video::IMaterialRendererServices* services,
		s32 userData)
	{
		video::IVideoDriver* driver = services->getVideoDriver();

		// set inverted world matrix
		// if we are using highlevel shaders (the user can select this when
		// starting the program), we must set the constants by name.

		//core::matrix4 invWorld = driver->getTransform(video::ETS_WORLD);
		//core::matrix4 invWorld = mWorld.makeInverse();

		//	services->setVertexShaderConstant("mInvWorld", invWorld.pointer(), 16);

		// set clip matrix

		core::matrix4 worldView = driver->getTransform(video::ETS_VIEW);
		worldView *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant("mWorldView", worldView.pointer(), 16);

		core::matrix4 Proj = driver->getTransform(video::ETS_PROJECTION);
		services->setVertexShaderConstant("mProj", Proj.pointer(), 16);
		/*
				core::matrix4 worldView2 = mView_;
				worldView2 *= driver->getTransform(video::ETS_WORLD);
				services->setVertexShaderConstant("mWorldView2", worldView2.pointer(), 16);

				core::matrix4 Proj2 = mProj_;
				services->setVertexShaderConstant("mProj2", Proj2.pointer(), 16);


				// set camera position

				core::vector3df pos = device->getSceneManager()->
					getActiveCamera()->getAbsolutePosition();
				//core::vector3df pos(3.0f,3.0f,3.0f);

					services->setVertexShaderConstant("mLightPos", reinterpret_cast<f32*>(&pos), 3);

				// set light color

				video::SColorf col(1.0f,1.0f,1.0f,0.0f);

					services->setVertexShaderConstant("mLightColor",					reinterpret_cast<f32*>(&col), 4);

				// set transposed world matrix

				core::matrix4 world = driver->getTransform(video::ETS_WORLD);
				world = world.getTransposed();


					services->setVertexShaderConstant("mTransWorld", world.pointer(), 16);

					// set texture, for textures you can use both an int and a float setPixelShaderConstant interfaces (You need it only for an OpenGL driver).
		*/

		s32 TextureLayerID = 0;

		services->setPixelShaderConstant("myTexture", &TextureLayerID, 1);

		//  TextureLayerID = 1;

	   //       services->setPixelShaderConstant("myTexture2", &TextureLayerID, 1);

		core::matrix4 invWorldMat;
		driver->getTransform(video::ETS_WORLD).getInverse(invWorldMat);

		//if(driver->getDynamicLightCount()>0)
		{

			video::SLight light;

			f32 lightPos[3 * 8];
			int lightRadius[8];

			int nLights = driver->getDynamicLightCount();

			 //std::cout << nLights << "\n";

			for (int i = 0; i < nLights; i++)
			{
				light = driver->getDynamicLight(i);
				invWorldMat.transformVect(light.Position);
				lightPos[i * 3] = light.Position.X;
				lightPos[i * 3 + 1] = light.Position.Y;
				lightPos[i * 3 + 2] = light.Position.Z;
				lightRadius[i] = light.Radius;
			}

			nLights = std::min(nLights, 8);

			services->setPixelShaderConstant("nLights", reinterpret_cast<int*>(&nLights), 1);
			services->setPixelShaderConstant("LightPos[0]", reinterpret_cast<f32*>(&lightPos[0]), 8 * 3);
			services->setPixelShaderConstant("LightRadius[0]", reinterpret_cast<int*>(&lightRadius[0]), 8);
		}

		//f32 num[5] = {1.0,1.0,1.0,1.0,1.0};
		//num[0] = 1.0;

		//std::cout << services->getPixelShaderConstantID("float_array[0]") << " ";
		//std::cout<<services->setPixelShaderConstant("float_array[0]",num,5)<<"\n";


	}
};


class DynamicLightOneSourceCallback : public video::IShaderConstantSetCallBack
{
public:

	virtual void OnSetConstants(video::IMaterialRendererServices* services,
		s32 userData)
	{/*
		video::IVideoDriver* driver = services->getVideoDriver();

		core::matrix4 worldView = driver->getTransform(video::ETS_VIEW);
		worldView *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant("mWorldView", worldView.pointer(), 16);

		core::matrix4 Proj = driver->getTransform(video::ETS_PROJECTION);
		services->setVertexShaderConstant("mProj", Proj.pointer(), 16);

		s32 TextureLayerID = 0;

		services->setPixelShaderConstant("myTexture", &TextureLayerID, 1);

		core::matrix4 invWorldMat;
		driver->getTransform(video::ETS_WORLD).getInverse(invWorldMat);

		core::vector3df pos = LightMaps_Tool::getLightmaps()->GetCurrentLightPos();
		int radius = LightMaps_Tool::getLightmaps()->GetCurrentLightRadius();

		invWorldMat.transformVect(pos);

		services->setPixelShaderConstant("LightPos", reinterpret_cast<f32*>(&pos), 3);
		services->setPixelShaderConstant("LightRadius", reinterpret_cast<int*>(&radius), 1);
		*/
	}
};


class LightmapShaderCallback : public video::IShaderConstantSetCallBack
{
public:

	virtual void OnSetConstants(video::IMaterialRendererServices* services,
		s32 userData)
	{
		video::IVideoDriver* driver = services->getVideoDriver();

		core::matrix4 worldView = driver->getTransform(video::ETS_VIEW);
		worldView *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant("mWorldView", worldView.pointer(), 16);

		core::matrix4 Proj = driver->getTransform(video::ETS_PROJECTION);
		services->setVertexShaderConstant("mProj", Proj.pointer(), 16);
		

		s32 TextureLayerID = 0;

		services->setPixelShaderConstant("myTexture", &TextureLayerID, 1);

		TextureLayerID = 1;

		services->setPixelShaderConstant("myTexture2", &TextureLayerID, 1);
	
	}
};


class ProjectionShaderCallback : public video::IShaderConstantSetCallBack
{
public:

	virtual void OnSetConstants(video::IMaterialRendererServices* services,
		s32 userData)
	{
		/*
		video::IVideoDriver* driver = services->getVideoDriver();

		core::matrix4 worldView = driver->getTransform(video::ETS_VIEW);
		worldView *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant("mWorldView", worldView.pointer(), 16);

		core::matrix4 Proj = driver->getTransform(video::ETS_PROJECTION);
		services->setVertexShaderConstant("mProj", Proj.pointer(), 16);


		core::matrix4 worldView3 = LightMaps_Tool::getLightmaps()->getViewMatrix();
		
		worldView3 *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant("mWorldView2", worldView3.pointer(), 16);


		core::matrix4 Proj3 = LightMaps_Tool::getLightmaps()->getProjectionMatrix();
		services->setVertexShaderConstant("mProj2", Proj3.pointer(), 16);

		

		// set texture, for textures you can use both an int and a float setPixelShaderConstant interfaces (You need it only for an OpenGL driver).
		s32 TextureLayerID = 0;

		services->setPixelShaderConstant("myTexture2", &TextureLayerID, 1);
		*/

	}

	core::matrix4 mView;
	core::matrix4 mProj;
};



class MyShaderCallBack_PathTracing : public video::IShaderConstantSetCallBack
{
public:

	virtual void OnSetConstants(video::IMaterialRendererServices* services,
		s32 userData)
	{
		video::IVideoDriver* driver = services->getVideoDriver();

		// set inverted world matrix
		// if we are using highlevel shaders (the user can select this when
		// starting the program), we must set the constants by name.

		//core::matrix4 invWorld = driver->getTransform(video::ETS_WORLD);
		//core::matrix4 invWorld = mWorld.makeInverse();

		//	services->setVertexShaderConstant("mInvWorld", invWorld.pointer(), 16);

		// set clip matrix

		core::matrix4 worldView = driver->getTransform(video::ETS_VIEW);
		worldView *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant("mWorldView", worldView.pointer(), 16);

		core::matrix4 Proj = driver->getTransform(video::ETS_PROJECTION);
		services->setVertexShaderConstant("mProj", Proj.pointer(), 16);
		/*
				core::matrix4 worldView2 = mView_;
				worldView2 *= driver->getTransform(video::ETS_WORLD);
				services->setVertexShaderConstant("mWorldView2", worldView2.pointer(), 16);

				core::matrix4 Proj2 = mProj_;
				services->setVertexShaderConstant("mProj2", Proj2.pointer(), 16);


				// set camera position

				core::vector3df pos = device->getSceneManager()->
					getActiveCamera()->getAbsolutePosition();
				//core::vector3df pos(3.0f,3.0f,3.0f);

					services->setVertexShaderConstant("mLightPos", reinterpret_cast<f32*>(&pos), 3);

				// set light color

				video::SColorf col(1.0f,1.0f,1.0f,0.0f);

					services->setVertexShaderConstant("mLightColor",					reinterpret_cast<f32*>(&col), 4);

				// set transposed world matrix

				core::matrix4 world = driver->getTransform(video::ETS_WORLD);
				world = world.getTransposed();


					services->setVertexShaderConstant("mTransWorld", world.pointer(), 16);

					// set texture, for textures you can use both an int and a float setPixelShaderConstant interfaces (You need it only for an OpenGL driver).
		*/

		s32 TextureLayerID = 0;

		services->setPixelShaderConstant("myTexture", &TextureLayerID, 1);

		//  TextureLayerID = 1;

	   //       services->setPixelShaderConstant("myTexture2", &TextureLayerID, 1);

		core::matrix4 invWorldMat;
		driver->getTransform(video::ETS_WORLD).getInverse(invWorldMat);

		//if(driver->getDynamicLightCount()>0)
		{

			video::SLight light;

			f32 lightPos[3 * 8];
			int lightRadius[8];

			int nLights = driver->getDynamicLightCount();

			//std::cout << nLights << "\n";

			for (int i = 0; i < nLights; i++)
			{
				light = driver->getDynamicLight(i);
				invWorldMat.transformVect(light.Position);
				lightPos[i * 3] = light.Position.X;
				lightPos[i * 3 + 1] = light.Position.Y;
				lightPos[i * 3 + 2] = light.Position.Z;
				lightRadius[i] = light.Radius;
			}

			nLights = std::min(nLights, 8);

			services->setPixelShaderConstant("nLights", reinterpret_cast<int*>(&nLights), 1);
			services->setPixelShaderConstant("LightPos[0]", reinterpret_cast<f32*>(&lightPos[0]), 8 * 3);
			services->setPixelShaderConstant("LightRadius[0]", reinterpret_cast<int*>(&lightRadius[0]), 8);
		}

		//f32 num[5] = {1.0,1.0,1.0,1.0,1.0};
		//num[0] = 1.0;

		//std::cout << services->getPixelShaderConstantID("float_array[0]") << " ";
		//std::cout<<services->setPixelShaderConstant("float_array[0]",num,5)<<"\n";


	}
};

#endif