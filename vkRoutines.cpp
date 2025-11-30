#include <irrlicht.h>
#include "LightMaps.h"
#include "vkModules.h"
#include "vkUtilModules.h"
#include "csg_classes.h"
#include "utils.h"
#include "geometry_scene.h"
#include "soa.h"
#include "vkSunlightModule.h"
#include "vkAreaLightModule.h"
#include "vkBouncedLightModule.h"
#include <vulkan/vulkan.h>

using namespace irr;
using namespace core;
using namespace std;

extern IrrlichtDevice* device;



void Lightmap_Routine(geometry_scene* g_scene, Lightmap_Configuration* configuration, std::vector<irr::video::ITexture*>& textures, Lightmap_Configuration* configuration1)
{
	//==================================
	// Setup 
	//

	Geometry_Assets geo_assets(g_scene, configuration);

	Vulkan_App vulkan(&geo_assets);

	//==================================
	// Initialize Modules
	//

	Geometry_Module geometry(&vulkan, &geo_assets);
	Create_Lightmap_Images_Module create_images(&vulkan, configuration);
	Create_Texture_Images_Module create_textures(&vulkan, configuration);
	AreaLight_Module arealights(&vulkan, &geometry, configuration);
	Lightmap_Edges_Module edges(&vulkan, configuration);
	Lightmap_Edges_Module edges2(&vulkan, configuration);
	MultiImage_To_ImageArray_Module map_images_as_array(&vulkan, configuration);
	MultiImage_Copy_Module copy_images(&vulkan, configuration);
	BouncedLight_Module bouncedLight(&vulkan, &geometry, configuration);
	Download_TextureArray_Module download_textures(&vulkan, device->getVideoDriver(), configuration);
	Merge_Images_Module merge_images(&vulkan, configuration);

	download_textures.bFlip = true;

	//==================================
	// Connect Modules
	//

	reflect::connect(&create_images.images_out, &arealights.images_in);
	//reflect::connect(&edges.images_out, &map_images_as_array.images_in);
	reflect::connect(&arealights.images_out, &edges.images_in);
	//reflect::connect(&edges.images_out, &copy_images.images_in);
	//reflect::connect(&copy_images.images_out, &download_textures.images_in);
	//reflect::connect(&edges.images_out, &download_textures.images_in);

	reflect::connect(&edges.images_out, &map_images_as_array.images_in);
	reflect::connect(&map_images_as_array.images_out, &bouncedLight.images_in);
	//reflect::connect(&map_images_as_array.images_out, &download_textures.images_in);
	//reflect::connect(&bouncedLight.images_out, &edges2.images_in);

	reflect::connect(&bouncedLight.images_out, &download_textures.images_in);
	//reflect::connect(&bouncedLight.images_out, &download_textures.images_in);
	//reflect::connect(&edges2.images_out, &merge_images.images_in_1);
	//reflect::connect(&edges2.images_out, &download_textures.images_in);
	//reflect::connect(&merge_images.images_out, &download_textures.images_in);

	//==================================
	// Run
	//
	geometry.run_and_push();
	//create_images.signaled();
	vulkan.run_workflow();

	//map_images_as_array.images_out.X.destroy(vulkan.m_device->getDevice());
	//edges.images_out.X.destroy(vulkan.m_device->getDevice());

	vulkan.status();

	textures = download_textures.textures;
	g_scene->special_graph = bouncedLight.graph;

	geometry.cleanup();
	vulkan.cleanup();
}