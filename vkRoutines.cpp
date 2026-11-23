#include <irrlicht.h>
#include <cstring>
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
#include <cstdio>

using namespace irr;
using namespace core;
using namespace std;

extern IrrlichtDevice* device;

void Vulkan_Workflow::make_default_workflow()
{
	clear();

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

	//Sunlight gets its own images: AreaLight and Sunlight both write in place, and the
	//edge correction needs each one's g = 1 "written" markers before they're combined
	Create_Lightmap_Images_Module* create_images_sun = new Create_Lightmap_Images_Module();
	Sunlight_Module* sunlight = new Sunlight_Module();
	Lightmap_Edges_Module* edges_sun = new Lightmap_Edges_Module();
	Merge_Images_Module* merge_direct = new Merge_Images_Module();

	download_textures->bFlip = true;

	//single triangle debugging: click a face, bake, then switch the 3D view to Triangles
	//	SUN_DEBUG_RAYS   - rays of the clicked texel
	//	SUN_DEBUG_TEXELS - texels the rasteriser visits for the clicked triangle
	sunlight->debug_mode.value = Sunlight_Module::SUN_DEBUG_OFF;

	ImageArray_To_MultiImage_Module* link0 = new ImageArray_To_MultiImage_Module();
	MultiImage_To_ImageArray_Module* link1 = new MultiImage_To_ImageArray_Module();
	ImageArray_To_MultiImage_Module* link2 = new ImageArray_To_MultiImage_Module();
	MultiImage_To_ImageArray_Module* link3 = new MultiImage_To_ImageArray_Module();

	
	add(create_images);
	add(arealights);
	add(edges);
	add(edges2);
	add(edges3);
	add(copy_images);
	add(bouncedLight);
	add(bouncedLight2);
	add(download_textures);
	add(merge_images);
	add(merge_images2);

	add(link0);
	add(link1);
	add(link2);
	add(link3);

	add(create_images_sun);
	add(sunlight);
	add(edges_sun);
	add(merge_direct);

	//==========================================
	// Direct light: area lights + sunlight
	//
	//	create_images     -> arealights -> edges     --\
	//	                                                 merge_direct -> (direct)
	//	create_images_sun -> sunlight   -> edges_sun --/
	//
	// merge_direct adds images_in_0 into images_in_1 in place, so the direct light
	// lives in edges_sun's images from here on
	//

	reflect::connect(&create_images->images_out, &arealights->images_in);
	reflect::connect(&arealights->images_out, &edges->images_in);

	reflect::connect(&create_images_sun->images_out, &sunlight->images_in);
	reflect::connect(&sunlight->images_out, &edges_sun->images_in);

	reflect::connect(&edges->images_out, &merge_direct->images_in_0);
	reflect::connect(&edges_sun->images_out, &merge_direct->images_in_1);

	//==========================================
	// Bounced light, from the combined direct light
	//

	reflect::connect(&merge_direct->images_out, &link1->images_in);
	reflect::connect(&link1->images_out, &bouncedLight->images_in);

	reflect::connect(&bouncedLight->images_out, &link2->images_in);
	reflect::connect(&link2->images_out, &edges2->images_in);

	reflect::connect(&edges2->images_out, &link3->images_in);
	reflect::connect(&link3->images_out, &bouncedLight2->images_in);

	reflect::connect(&bouncedLight2->images_out, &link0->images_in);
	reflect::connect(&link0->images_out, &edges3->images_in);


	reflect::connect(&edges2->images_out, &merge_images->images_in_0);
	reflect::connect(&edges3->images_out, &merge_images->images_in_1);

	//final = direct + (bounce 1 + bounce 2)
	reflect::connect(&merge_direct->images_out, &merge_images2->images_in_0);
	reflect::connect(&merge_images->images_out, &merge_images2->images_in_1);

	reflect::connect(&merge_images2->images_out, &download_textures->images_in);

	texture_module = download_textures;
}

void Vulkan_Workflow::initialize_and_run(Vulkan_App* vulkan)
{
	for (Vulkan_Module* mod : items)
	{
		mod->initialize(vulkan);
	}

	vulkan->run_workflow();
	vulkan->cleanup();
}

//Workflow files start with a magic + version. Files from before the header start straight
//with the module count (a small size_t), which can't match the magic.
//	version 1: module settings blocks (TypeDescriptor_VkMod_Struct::serialize)
static const char workflow_file_magic[8] = { 'T','K','T','V','K','W','F','\0' };
static const u32 workflow_file_version = 1;

bool Vulkan_Workflow::save(const std::string& fname)
{
	ofstream wf(fname, ios::out | ios::binary);

	if (!wf)
	{
		cout << "Cannot open file " << fname << "\n";
		return false;
	}

	wf.write(workflow_file_magic, sizeof(workflow_file_magic));
	wf.write((char*)&workflow_file_version, sizeof(u32));

	reflect::TypeDescriptor_Struct* tD = &Vulkan_Workflow::Reflection;
	//tD->dump(this, 0);
	tD->serialize(wf, this);

	return wf.good();
}

bool Vulkan_Workflow::load(const std::string& fname)
{
	ifstream rf(fname, ios::in | ios::binary);

	if (!rf)
	{
		cout << "Cannot open file " << fname << "\n";
		return false;
	}

	u32 version = 0;
	{
		char magic[sizeof(workflow_file_magic)] = {};
		rf.read(magic, sizeof(magic));

		if (rf && memcmp(magic, workflow_file_magic, sizeof(magic)) == 0)
			rf.read((char*)&version, sizeof(u32));
		else
		{
			rf.clear();
			rf.seekg(0);
		}
	}

	if (version > workflow_file_version)
	{
		cout << fname << ": workflow file version " << version << " is newer than this editor reads ("
			<< workflow_file_version << ")\n";
		return false;
	}

	//deserializing resizes items over the existing pointers, so free those first
	clear();

	reflect::TypeDescriptor_Struct* tD = &Vulkan_Workflow::Reflection;
	reflect::TypeDescriptor_VkMod_Struct::load_version = version;
	tD->deserialize(rf, this);
	reflect::TypeDescriptor_VkMod_Struct::load_version = workflow_file_version;
	//tD->dump(this, 0);

	texture_module = NULL;
	for (Vulkan_Module* mod : items)
	{
		if (!texture_module)
			texture_module = dynamic_cast<Download_Textures_Module*>(mod);
	}

	return true;
}

bool Vulkan_Workflow::clone_from(Vulkan_Workflow* src)
{
	//round trip through the serializer, which only takes file streams
	const char* tmp_name = "lm_workflow_clone.tmp";

	if (!src || !src->save(tmp_name))
		return false;

	bool ok = load(tmp_name);
	std::remove(tmp_name);
	return ok;
}

//================================================================
// geometry_scene lightmap workflow
//

Vulkan_Workflow* geometry_scene::get_lightmap_workflow()
{
	if (!lightmap_workflow)
	{
		lightmap_workflow = new Vulkan_Workflow();
		lightmap_workflow->make_default_workflow();
	}
	return lightmap_workflow;
}

bool geometry_scene::save_lightmap_workflow(const std::string& fname)
{
	return get_lightmap_workflow()->save(fname);
}

bool geometry_scene::load_lightmap_workflow(const std::string& fname)
{
	Vulkan_Workflow* wf = new Vulkan_Workflow();

	if (!wf->load(fname))
	{
		delete wf;
		return false;
	}

	free_lightmap_workflow();
	lightmap_workflow = wf;
	return true;
}

void geometry_scene::free_lightmap_workflow()
{
	if (lightmap_workflow)
		delete lightmap_workflow;
	lightmap_workflow = NULL;
}

void Vulkan_Workflow::onItemRemoved(Vulkan_Module* mod)
{
	if (mod == texture_module)
		texture_module = NULL;
}

void Lightmap_Routine(geometry_scene* g_scene, Lightmap_Configuration* configuration, std::vector<irr::video::ITexture*>& textures, Lightmap_Configuration* configuration1)
{
	//==================================
	// Setup 
	//

	Geometry_Assets geo_assets(g_scene, configuration);

	Vulkan_App vulkan(&geo_assets,configuration, device->getVideoDriver());

	//run a copy of the scene's workflow: running changes module/input state, and the
	//scene's copy is also what the editor graph view displays
	Vulkan_Workflow* workflow = new Vulkan_Workflow();

	if (!workflow->clone_from(g_scene->get_lightmap_workflow()) || !workflow->texture_module)
	{
		cout << "could not copy the scene's lightmap workflow, using the default\n";
		delete workflow;
		workflow = new Vulkan_Workflow();
		workflow->make_default_workflow();
	}

	//bFlip isn't serialized (module member lists are assumed to hold only inputs/outputs),
	//so restore what make_default_workflow sets
	workflow->texture_module->bFlip = true;

	workflow->initialize_and_run(&vulkan);

	//debug lines from the modules, drawn by the 3D panel in Triangles view
	if (geo_assets.debug_graph.lines.size() > 0 || geo_assets.debug_graph.points.size() > 0)
	{
		g_scene->special_graph.lines = geo_assets.debug_graph.lines;
		g_scene->special_graph.points = geo_assets.debug_graph.points;
	}

	textures = workflow->texture_module->textures;
	delete workflow;
}