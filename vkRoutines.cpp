#include <irrlicht.h>
#include "LightMaps.h"
#include "vkModules.h"
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

	Geometry_Module geometry(&vulkan, &geo_assets);
	geometry.run();

	//==================================
	// Initialize Modules
	//

	Create_Images_Module create_images(&vulkan, configuration);
	AreaLight_Module arealights(&vulkan, &geometry, configuration);
	Lightmap_Edges_Module edges(&vulkan, configuration);
	BouncedLight_Module bouncedLight(&vulkan, &geometry, configuration);
	Download_Textures_Module download_textures(&vulkan, device->getVideoDriver(), configuration);

	download_textures.bFlip = true;

	//==================================
	// Connect Modules
	//

	reflect::connect(&create_images.images_out, &arealights.images_in);
	reflect::connect(&arealights.images_out, &edges.images_in);
	//reflect::connect(&edges.images_out, &download_textures.images_in);
	reflect::connect(&arealights.images_out, &download_textures.images_in);

	//==================================
	// Run
	//
	create_images.run();
	arealights.run();
	//edges.run();
	download_textures.run();

	textures = download_textures.textures;
	g_scene->special_graph = arealights.graph;

	geometry.cleanup();
	vulkan.cleanup();
}