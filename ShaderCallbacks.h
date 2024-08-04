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

		core::matrix4 worldView = driver->getTransform(video::ETS_VIEW);
		worldView *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant("mWorldView", worldView.pointer(), 16);

		core::matrix4 Proj = driver->getTransform(video::ETS_PROJECTION);
		services->setVertexShaderConstant("mProj", Proj.pointer(), 16);

		s32 TextureLayerID = 0;

		services->setPixelShaderConstant("myTexture", &TextureLayerID, 1);
	}
};

class MyShaderCallBack2 : public video::IShaderConstantSetCallBack
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

		core::matrix4 invWorldMat;
		driver->getTransform(video::ETS_WORLD).getInverse(invWorldMat);

		{

			video::SLight light;

			f32 lightPos[3 * 8];
			int lightRadius[8];

			int nLights = driver->getDynamicLightCount();

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


#endif