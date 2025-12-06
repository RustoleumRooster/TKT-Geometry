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

	Vulkan_App vulkan(&geo_assets,configuration, device->getVideoDriver());

	//==================================
	// Initialize Modules
	//

	//Geometry_Module geometry(&vulkan, &geo_assets);
	Create_Lightmap_Images_Module* create_images = (Create_Lightmap_Images_Module*)vulkan.create_module<Create_Lightmap_Images_Module>();
	//Create_Texture_Images_Module create_textures(&vulkan, configuration);
	AreaLight_Module* arealights = (AreaLight_Module*)vulkan.create_module<AreaLight_Module>();
	Lightmap_Edges_Module* edges = (Lightmap_Edges_Module*)vulkan.create_module<Lightmap_Edges_Module>();
	Lightmap_Edges_Module* edges2 = (Lightmap_Edges_Module*)vulkan.create_module<Lightmap_Edges_Module>();
	MultiImage_To_ImageArray_Module* map_images_as_array = (MultiImage_To_ImageArray_Module*)vulkan.create_module<MultiImage_To_ImageArray_Module>();
	ImageArray_To_MultiImage_Module* map_array_as_images = (ImageArray_To_MultiImage_Module*)vulkan.create_module<ImageArray_To_MultiImage_Module>();
	MultiImage_Copy_Module* copy_images = (MultiImage_Copy_Module*)vulkan.create_module<MultiImage_Copy_Module>();
	BouncedLight_Module* bouncedLight = (BouncedLight_Module*)vulkan.create_module<BouncedLight_Module>();
	//BouncedLight_Module* bouncedLight2 = (BouncedLight_Module*)vulkan.create_module<BouncedLight_Module>();
	Download_Textures_Module* download_textures = (Download_Textures_Module*)vulkan.create_module<Download_Textures_Module>();
	//Download_TextureArray_Module* download_textures = (Download_TextureArray_Module*)vulkan.create_module<Download_TextureArray_Module>();
	Merge_Images_Module* merge_images = (Merge_Images_Module*)vulkan.create_module<Merge_Images_Module>();

	download_textures->bFlip = true;

	//==================================
	// Connect Modules
	//

	reflect::connect(&create_images->images_out, &arealights->images_in);
	//reflect::connect(&edges.images_out, &map_images_as_array.images_in);
	reflect::connect(&arealights->images_out, &edges->images_in);
	//reflect::connect(&edges.images_out, &copy_images.images_in);
	//reflect::connect(&copy_images.images_out, &download_textures.images_in);
	//reflect::connect(&edges->images_out, &download_textures->images_in);

	reflect::connect(&edges->images_out, &map_images_as_array->images_in);
	reflect::connect(&map_images_as_array->images_out, &bouncedLight->images_in);
	//reflect::connect(&map_images_as_array->images_out, &download_textures->images_in);
	//reflect::connect(&bouncedLight->images_out, &download_textures->images_in);

	//reflect::connect(&edges->images_out, &download_textures->images_in);
	reflect::connect(&bouncedLight->images_out, &map_array_as_images->images_in);
	reflect::connect(&map_array_as_images->images_out, &edges2->images_in);
	reflect::connect(&edges->images_out, &merge_images->images_in_0);
	//reflect::connect(&edges2->images_out, &merge_images->images_in_1);
	reflect::connect(&edges2->images_out, &download_textures->images_in);
	//reflect::connect(&merge_images->images_out, &download_textures->images_in);
	//reflect::connect(&edges->images_out, &download_textures->images_in);
	//reflect::connect(&merge_images->images_out, &download_textures->images_in);


	//==================================
	// Run
	//
	
	vulkan.run_workflow();

	textures = download_textures->textures;
	g_scene->special_graph = bouncedLight->graph;

	vulkan.cleanup();
}