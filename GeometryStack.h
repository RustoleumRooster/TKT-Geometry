#pragma once

#ifndef _GEOMETRY_STACK_H_
#define _GEOMETRY_STACK_H_

#include <irrlicht.h>
#include "csg_classes.h"
#include "USceneNode.h"
#include "BufferManager.h"
#include "reflected_nodes.h"
#include "soa.h"
#include "LightMaps.h"

using namespace irr;

struct lightmap_info_struct
{
    u32 width = 128;
    u32 height = 128;
    bool bFlipped = false;
    bool bOverrideSize = false;

    REFLECT()
};

struct poly_surface
{
    std::vector<int> my_faces;
    core::stringw texture_name;
    int material_group;
    int surface_type;
    int texture_index; //for serialization only
    int face_index_offset; //set when building geometry
    lightmap_info_struct lightmap_info;

    REFLECT()
};

void combine_polyfolds(const std::vector<polyfold*>& polies, const std::vector<std::vector<poly_surface>>& surfaces, polyfold& res, std::vector<poly_surface>&);

class geo_element
{
public:
    int type = 0;
    int selected_vertex = 0;
    int element_id = -1; //unique incrementing ID
    bool control_vertex_selected = false;
    bool bSelected = false;

    polyfold brush;
    polyfold geometry;

    std::vector<poly_surface> surfaces;
    std::vector<int> reverse_index;

    bool has_geometry();
    void draw_brush(video::IVideoDriver* driver);
    void draw_geometry(video::IVideoDriver* driver, const video::SMaterial material);
    void draw_brush_vertices(const core::matrix4& trans, core::dimension2du view_dim, bool bDrawSelectedVertex, video::IVideoDriver* driver);
    video::SColor getColor();

    REFLECT()

        template<typename T>
    void grow(T* obj) const
    {
        obj->aabbMin[0] = fmin(obj->aabbMin[0], brush.bbox.MinEdge.X);
        obj->aabbMin[1] = fmin(obj->aabbMin[1], brush.bbox.MinEdge.Y);
        obj->aabbMin[2] = fmin(obj->aabbMin[2], brush.bbox.MinEdge.Z);

        obj->aabbMax[0] = fmax(obj->aabbMax[0], brush.bbox.MaxEdge.X);
        obj->aabbMax[1] = fmax(obj->aabbMax[1], brush.bbox.MaxEdge.Y);
        obj->aabbMax[2] = fmax(obj->aabbMax[2], brush.bbox.MaxEdge.Z);
    }
    core::vector3df position() const {
        return brush.bbox.getCenter();
    }
};


class Lightmap_Manager;
class MyEventReceiver;
class Node_Classes_Base;
class Material_Groups_Base;
class TexturePicker_Base;

class GeometryStack : public USceneNode
{
public:
    GeometryStack();


    void initialize(ISceneNode* parent, scene::ISceneManager* smgr_, MyEventReceiver* receiver);
    void initialize(scene::ISceneManager* smgr_, MyEventReceiver* receiver);
    void initialize(geometry_scene*);

    ~GeometryStack();

    virtual void OnRegisterSceneNode() override;
    virtual void render() override;

    void set_type(int);

    void add();
    void subtract();
    void add_semisolid();
    void add_plane();
    void rebuild_geometry(bool = false);
    void clear_scene();

    void intersect_active_brush();
    void clip_active_brush();
    void clip_active_brush_plane_geometry();

    polyfold get_intersecting_geometry(const polyfold& pf);
    void build_intersecting_target(const polyfold& pf, polyfold& out);
    polyfold* get_total_geometry();
    

    void buildSceneNode(bool finalMesh, int light_mode);
    scene::CMeshSceneNode* getMeshNode() { return my_MeshNode; }
    void setFinalMeshDirty(bool dirty = true) { final_mesh_dirty = dirty; }
    void recalculate_final_meshbuffer();

    triangle_holder* get_triangles_for_face(int f_i);

    void setRenderType(bool brushes, bool geo, bool loops, bool triangles);
    void GetGeometryLoopLines(LineHolder&,LineHolder&);

    bool renderGeometry() { return render_geometry; }
    geo_element* get_element_by_id(int);
    int get_element_index_by_id(int);

    poly_face* get_brush_face(face_index);
    poly_face* get_geometry_face(face_index);
    polyfold* get_brush(face_index);
    polyfold* get_geometry(face_index);
    int get_element_index(face_index);

    int n_faces();
    poly_face* brush_face_by_n(int);
    poly_face* geo_face_by_n(int);
    polyfold* brush_by_n(int);
    polyfold* geo_by_n(int);
    geo_element* element_by_n(int);
    int element_id_by_n(int);
    int face_no_by_n(int);
    core::stringw face_texture_by_n(int);
    poly_surface* surface_by_n(int);

    void make_index_lists();

    MeshNode_Interface_Edit edit_meshnode_interface;
    MeshNode_Interface_Final final_meshnode_interface;

    std::vector<geo_element> elements;

    Lightmap_Configuration& get_lightmap_config();
    Lightmap_Configuration& get_lightmap_config(int);
    void set_lightmap_config(int);
    
    REFLECT()

private:

    void trianglize_total_geometry();
    void fill_geometry_triangles();
    void generate_meshes();
    void build_total_geometry();

    int base_type = GEO_SOLID;

    int current_lm_config = 0;
    std::vector<Lightmap_Configuration> lightmap_configs;

    //not used yet
    soa_struct<vector3df> geometry_triangles_vertices;
    soa_struct<u16> geometry_triangles_indices;

    //to be replaced
    std::vector<triangle_holder> total_geometry_triangles;

    int build_progress = 0;
    bool progressive_build = true;

    polyfold total_geometry;

    MyEventReceiver* event_receiver = NULL;

    int new_geometry_material_group = 0;

    scene::CMeshSceneNode* my_MeshNode = NULL;

    bool final_mesh_dirty = false;
    bool b_isEditNode = true;
    bool render_brushes = false;
    bool render_geometry = true;
    bool render_loops = false;
    bool render_triangles = false;
    bool render_active_brush = true;
    int selected_brush_vertex_editing = 0;
    int element_id_incrementer = 0;

    LineHolder loops_graph;

    int n_total_faces = 0;
    std::vector<int> element_id_by_face_n;
    std::vector<int> face_j_by_face_n;
    std::vector<int> element_by_element_id;

    friend class Open_Geometry_File;
    friend class geometry_scene;
    friend class Geometry_Scene_File_IO;
   
};

#endif
