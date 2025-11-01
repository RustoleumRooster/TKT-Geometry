#include <irrlicht.h>
#include "LightMaps.h"
#include "vkModules.h"
#include "csg_classes.h"
#include "utils.h"
#include "geometry_scene.h"
#include "soa.h"
#include "vkSunlightModule.h"
#include "vkAreaLightModule.h"
#include <vulkan/vulkan.h>

using namespace irr;
using namespace core;
using namespace std;

extern IrrlichtDevice* device;


void Lightmap_Routine2(geometry_scene* g_scene, Lightmap_Configuration* configuration, std::vector<irr::video::ITexture*>& textures, Lightmap_Configuration* configuration1)
{
	//==================================
	// Setup 
	//

	Geometry_Assets geo_assets(g_scene, configuration);

	Vulkan_App vulkan;

	vulkan.indices_soa = &geo_assets.indices_soa;
	vulkan.vertices_soa = &geo_assets.vertices_soa;
	vulkan.bvh = &geo_assets.bvh;
	vulkan.n_indices = geo_assets.indices_soa.data.size();
	vulkan.n_vertices = geo_assets.vertices_soa.data0.size();
	vulkan.n_nodes = geo_assets.bvh.node_count;
	vulkan.triangle_edges = &geo_assets.triangle_edges;
	vulkan.initBuffers();


	Load_Textures_Module load_textures(&vulkan, device->getVideoDriver(), configuration);
	load_textures.textures = textures;

	load_textures.run();
	//load_textures.destroyImages();

	Copy_Lightmaps_Module copy(&vulkan, configuration, configuration1);

	copy.element_by_element_id = &g_scene->geoNode()->element_by_element_id;
	copy.surface_index = &geo_assets.surface_index;

	copy.indexBuffer = vulkan.indexBuffer;
	copy.indexBufferMemory = vulkan.indexBufferMemory;
	copy.n_indices = vulkan.n_indices;

	copy.lightmapImages_in = load_textures.lightmapImages;
	copy.lightmapImageViews_in = load_textures.lightmapImageViews;
	copy.lightmapsMemory_in = load_textures.lightmapsMemory;

	copy.uv_struct_0 = &configuration->lm_raw_uvs.data;
	copy.uv_struct_1 = &configuration1->lm_raw_uvs.data;

	copy.run();
	//copy.destroyImages();

	load_textures.destroyImages();

	//==================================
	// Make ITextures from the vulkan images
	//

	Download_Textures_Module download_textures(&vulkan, device->getVideoDriver(), configuration1);

	download_textures.lightmapImages = copy.lightmapImages_out;
	download_textures.lightmapImageViews = copy.lightmapImageViews_out;
	download_textures.lightmapsMemory = copy.lightmapsMemory_out;
	download_textures.bFlip = false;

	download_textures.run();

	textures = download_textures.textures;

	//cleanup

	copy.destroyImages();

	vulkan.cleanup();
}

void Lightmap_Routine(geometry_scene* g_scene, Lightmap_Configuration* configuration, std::vector<irr::video::ITexture*>& textures, Lightmap_Configuration* configuration1)
{
	//==================================
	// Setup 
	//

	Geometry_Assets geo_assets(g_scene, configuration);

	Vulkan_App vulkan;

	vulkan.indices_soa = &geo_assets.indices_soa;
	vulkan.vertices_soa = &geo_assets.vertices_soa;
	vulkan.bvh = &geo_assets.bvh;
	vulkan.n_indices = geo_assets.indices_soa.data.size();
	vulkan.n_vertices = geo_assets.vertices_soa.data0.size();
	vulkan.n_nodes = geo_assets.bvh.node_count;
	vulkan.triangle_edges = &geo_assets.triangle_edges;
	vulkan.initBuffers();

	//==================================
	// Create images on the GPU
	//

	Create_Images_Module create_images(&vulkan, configuration);
	create_images.run();

	//==================================
	// Run the raytracing program
	//

	Sunlight_Module sunlight(&vulkan);

	std::vector<Reflected_SceneNode*> sun_nodes = g_scene->get_reflected_nodes_by_type("Reflected_LightSceneNode");

	if (sun_nodes.size() > 0)
	{
		sunlight.sun_direction = sun_nodes[0]->get_direction_vector();
	}

	sunlight.indices_soa = &geo_assets.indices_soa;
	sunlight.vertices_soa = &geo_assets.vertices_soa;
	sunlight.n_indices = geo_assets.indices_soa.data.size();
	sunlight.n_vertices = geo_assets.vertices_soa.data0.size();
	sunlight.n_nodes = geo_assets.bvh.node_count;

	sunlight.materials = &configuration->get_materials();

	sunlight.lightmapImages = create_images.lightmapImages;
	sunlight.lightmapImageViews = create_images.lightmapImageViews;
	sunlight.lightmapsMemory = create_images.lightmapsMemory;

	sunlight.edgeBuffer = vulkan.edgeBuffer;
	sunlight.edgeBufferMemory = vulkan.edgeBufferMemory;
	sunlight.indexBuffer = vulkan.indexBuffer;
	sunlight.indexBufferMemory = vulkan.indexBufferMemory;
	sunlight.vertexBuffer = vulkan.vertexBuffer;
	sunlight.vertexBufferMemory = vulkan.vertexBufferMemory;
	sunlight.nodeBuffer = vulkan.nodeBuffer;
	sunlight.nodeBufferMemory = vulkan.nodeBufferMemory;
	sunlight.uvBuffer = vulkan.uvBuffer;
	sunlight.uvBufferMemory = vulkan.uvBufferMemory;

	sunlight.run();

	//==================================
	// Edge / Corner correction
	//

	Lightmap_Edges_Module edges(&vulkan, configuration);

	edges.lightmapImages_in = sunlight.lightmapImages;
	edges.lightmapImageViews_in = sunlight.lightmapImageViews;
	edges.lightmapsMemory_in = sunlight.lightmapsMemory;

	edges.run();

	//==================================
	// Make ITextures from the vulkan images
	//

	Download_Textures_Module download_textures(&vulkan, device->getVideoDriver(), configuration);

	download_textures.lightmapImages = edges.lightmapImages_out;
	download_textures.lightmapImageViews = edges.lightmapImageViews_out;
	download_textures.lightmapsMemory = edges.lightmapsMemory_out;
	download_textures.bFlip = true;

	download_textures.run();

	textures = download_textures.textures;

	//cleanup
	edges.destroyImages();

	vulkan.cleanup();
}

void Lightmap_Routine3(geometry_scene* g_scene, Lightmap_Configuration* configuration, std::vector<irr::video::ITexture*>& textures, Lightmap_Configuration* configuration1)
{
	//==================================
	// Setup 
	//

	Geometry_Assets geo_assets(g_scene, configuration);

	Vulkan_App vulkan;

	vulkan.indices_soa = &geo_assets.indices_soa;
	vulkan.vertices_soa = &geo_assets.vertices_soa;
	vulkan.bvh = &geo_assets.bvh;
	vulkan.n_indices = geo_assets.indices_soa.data.size();
	vulkan.n_vertices = geo_assets.vertices_soa.data0.size();
	vulkan.n_nodes = geo_assets.bvh.node_count;
	vulkan.triangle_edges = &geo_assets.triangle_edges;
	vulkan.area_light_indices = &geo_assets.area_light_indices;
	vulkan.initBuffers();
	vulkan.init_area_light_buffer();
	vulkan.createOutputBuffer();

	//==================================
	// Create images on the GPU
	//

	Create_Images_Module create_images(&vulkan, configuration);
	create_images.run();

	//==================================
	// Run the raytracing program
	//

	AreaLight_Module arealights(&vulkan);

	arealights.indices_soa = &geo_assets.indices_soa;
	arealights.vertices_soa = &geo_assets.vertices_soa;
	arealights.lightsource_indices_soa = &geo_assets.area_light_indices;
	arealights.n_indices = geo_assets.indices_soa.data.size();
	arealights.n_vertices = geo_assets.vertices_soa.data0.size();
	arealights.n_nodes = geo_assets.bvh.node_count;
	arealights.selected_triangle_mg = geo_assets.selected_triangle_mg;
	arealights.selected_triangle_index = geo_assets.selected_triangle_index;
	arealights.selected_triangle_bary_coords = geo_assets.selected_triangle_bary_coords;

	arealights.materials = &configuration->get_materials();

	arealights.lightmapImages = create_images.lightmapImages;
	arealights.lightmapImageViews = create_images.lightmapImageViews;
	arealights.lightmapsMemory = create_images.lightmapsMemory;

	arealights.edgeBuffer = vulkan.edgeBuffer;
	arealights.edgeBufferMemory = vulkan.edgeBufferMemory;
	arealights.indexBuffer = vulkan.indexBuffer;
	arealights.indexBufferMemory = vulkan.indexBufferMemory;
	arealights.vertexBuffer = vulkan.vertexBuffer;
	arealights.vertexBufferMemory = vulkan.vertexBufferMemory;
	arealights.nodeBuffer = vulkan.nodeBuffer;
	arealights.nodeBufferMemory = vulkan.nodeBufferMemory;
	arealights.uvBuffer = vulkan.uvBuffer;
	arealights.uvBufferMemory = vulkan.uvBufferMemory;
	arealights.lightSourceBuffer = vulkan.AreaLightSourceBuffer;
	arealights.lightSourceBufferMemory = vulkan.AreaLightSourceBufferMemory;
	arealights.outputBuffer = vulkan.outputBuffer; //unused. using own buffer
	arealights.outputBufferMemory = vulkan.outputBufferMemory;

	arealights.run();

	g_scene->special_graph = arealights.graph;

	//==================================
	// Edge / Corner correction
	//

	Lightmap_Edges_Module edges(&vulkan, configuration);

	edges.lightmapImages_in = arealights.lightmapImages;
	edges.lightmapImageViews_in = arealights.lightmapImageViews;
	edges.lightmapsMemory_in = arealights.lightmapsMemory;

	edges.run();

	//==================================
	// Make ITextures from the vulkan images
	//

	Download_Textures_Module download_textures(&vulkan, device->getVideoDriver(), configuration);

	download_textures.lightmapImages = edges.lightmapImages_out;
	download_textures.lightmapImageViews = edges.lightmapImageViews_out;
	download_textures.lightmapsMemory = edges.lightmapsMemory_out;
	download_textures.bFlip = true;

	download_textures.run();

	textures = download_textures.textures;

	//cleanup
	edges.destroyImages();

	vulkan.cleanup();
}