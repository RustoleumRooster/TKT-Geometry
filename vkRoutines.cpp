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

void Vulkan_Workflow::make_default_workflow()
{

	Create_Lightmap_Images_Module* create_images = new Create_Lightmap_Images_Module();
	//Create_Texture_Images_Module create_textures(&vulkan, configuration);
	AreaLight_Module* arealights = new AreaLight_Module();
	Lightmap_Edges_Module* edges = new Lightmap_Edges_Module();
	Lightmap_Edges_Module* edges2 = new Lightmap_Edges_Module();
	Lightmap_Edges_Module* edges3 = new Lightmap_Edges_Module();
	//MultiImage_To_ImageArray_Module* map_images_as_array = (MultiImage_To_ImageArray_Module*)vulkan.create_module<MultiImage_To_ImageArray_Module>();
	//ImageArray_To_MultiImage_Module* map_array_as_images = (ImageArray_To_MultiImage_Module*)vulkan.create_module<ImageArray_To_MultiImage_Module>();
	MultiImage_Copy_Module* copy_images = new MultiImage_Copy_Module();
	BouncedLight_Module* bouncedLight = new BouncedLight_Module();
	BouncedLight_Module* bouncedLight2 = new BouncedLight_Module();
	Download_Textures_Module* download_textures = new Download_Textures_Module();
	//Download_TextureArray_Module* download_textures = (Download_TextureArray_Module*)vulkan.create_module<Download_TextureArray_Module>();
	Merge_Images_Module* merge_images = new Merge_Images_Module();
	Merge_Images_Module* merge_images2 = new Merge_Images_Module();

	download_textures->bFlip = true;

	ImageArray_To_MultiImage_Module* link0 = new ImageArray_To_MultiImage_Module();
	MultiImage_To_ImageArray_Module* link1 = new MultiImage_To_ImageArray_Module();
	ImageArray_To_MultiImage_Module* link2 = new ImageArray_To_MultiImage_Module();
	MultiImage_To_ImageArray_Module* link3 = new MultiImage_To_ImageArray_Module();

	
	Modules.push_back(VkMod_Reference{ create_images });
	Modules.push_back(VkMod_Reference{ arealights });
	Modules.push_back(VkMod_Reference{ edges });
	Modules.push_back(VkMod_Reference{ edges2 });
	Modules.push_back(VkMod_Reference{ edges3 });
	Modules.push_back(VkMod_Reference{ copy_images });
	Modules.push_back(VkMod_Reference{ bouncedLight });
	Modules.push_back(VkMod_Reference{ bouncedLight2 });
	Modules.push_back(VkMod_Reference{ download_textures });
	Modules.push_back(VkMod_Reference{ merge_images });
	Modules.push_back(VkMod_Reference{ merge_images2 });

	Modules.push_back(VkMod_Reference{ link0 });
	Modules.push_back(VkMod_Reference{ link1 });
	Modules.push_back(VkMod_Reference{ link2 });
	Modules.push_back(VkMod_Reference{ link3 });

	reflect::connect(&create_images->images_out, &arealights->images_in);

	reflect::connect(&arealights->images_out, &edges->images_in);

	reflect::connect(&edges->images_out, &link1->images_in);
	reflect::connect(&link1->images_out, &bouncedLight->images_in);

	reflect::connect(&bouncedLight->images_out, &link2->images_in);
	reflect::connect(&link2->images_out, &edges2->images_in);

	reflect::connect(&edges2->images_out, &link3->images_in);
	reflect::connect(&link3->images_out, &bouncedLight2->images_in);

	reflect::connect(&bouncedLight2->images_out, &link0->images_in);
	reflect::connect(&link0->images_out, &edges3->images_in);


	reflect::connect(&edges2->images_out, &merge_images->images_in_0);
	reflect::connect(&edges3->images_out, &merge_images->images_in_1);

	reflect::connect(&edges->images_out, &merge_images2->images_in_0);
	reflect::connect(&merge_images->images_out, &merge_images2->images_in_1);

	reflect::connect(&merge_images2->images_out, &download_textures->images_in);

	items.clear();
	for (VkMod_Reference& ref : Modules)
		items.push_back(ref.X);

	texture_module = download_textures;
}

void Vulkan_Workflow::initialize_and_run(Vulkan_App* vulkan)
{
	for (VkMod_Reference& mod : Modules)
	{
		mod.X->initialize(vulkan);
	}

	vulkan->run_workflow();
	vulkan->cleanup();
}

void Vulkan_Workflow::save()
{
	ofstream wf("Vulkan_System", ios::out | ios::binary);

	if (!wf)
	{
		cout << "Cannot open file\n";
		return;
	}

	reflect::TypeDescriptor_Struct* tD = &Vulkan_Workflow::Reflection;
	//tD->dump(this, 0);
	tD->serialize(wf, this);

}

void Vulkan_Workflow::load()
{
	ifstream rf("Vulkan_System", ios::in | ios::binary);

	if (!rf)
	{
		cout << "Cannot open file\n";
		return;
	}

	reflect::TypeDescriptor_Struct* tD = &Vulkan_Workflow::Reflection;
	tD->deserialize(rf, this);
	//tD->dump(this, 0);

	items.clear();
	for (VkMod_Reference& ref : Modules)
		items.push_back(ref.X);

}

Vulkan_Workflow::~Vulkan_Workflow()
{
	for (VkMod_Reference& mod : Modules)
	{
		delete mod.X;
	}
}

void Lightmap_Routine(geometry_scene* g_scene, Lightmap_Configuration* configuration, std::vector<irr::video::ITexture*>& textures, Lightmap_Configuration* configuration1)
{
	//==================================
	// Setup 
	//

	Geometry_Assets geo_assets(g_scene, configuration);

	Vulkan_App vulkan(&geo_assets,configuration, device->getVideoDriver());

	Vulkan_Workflow workflow;
	
	workflow.make_default_workflow();
	workflow.initialize_and_run(&vulkan);
	workflow.save();

	textures = workflow.texture_module->textures;
}