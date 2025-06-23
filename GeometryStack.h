#pragma once

#ifndef _GEOMETRY_STACK_H_
#define _GEOMETRY_STACK_H_

#include <irrlicht.h>
#include "csg_classes.h"
#include "USceneNode.h"
#include "BufferManager.h"
#include "reflected_nodes.h"

using namespace irr;

class geo_element
{
public:
    int type = 0;
    int selected_vertex = 0;
    bool control_vertex_selected = false;
    bool bSelected = false;

    polyfold brush;
    polyfold geometry;

    bool has_geometry();
    void draw_brush(video::IVideoDriver* driver);
    void draw_geometry(video::IVideoDriver* driver, const video::SMaterial material);
    void draw_brush_vertices(const core::matrix4& trans, core::dimension2du view_dim, bool bDrawSelectedVertex, video::IVideoDriver* driver);
    video::SColor getColor();
    //bool isDeleted(){return bDeleted;}

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

private:
    //bool bDeleted=false;

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

    void add(polyfold);
    void subtract(polyfold);
    void add_semisolid(polyfold);
    void add_plane(polyfold);
    void rebuild_geometry(bool = false);
    void clear_scene();

    void intersect_active_brush();
    void clip_active_brush();
    void clip_active_brush_plane_geometry();

    polyfold get_intersecting_geometry(const polyfold& pf);
    void build_intersecting_target(const polyfold& pf, polyfold& out);
    polyfold* get_total_geometry();
    std::vector<int> getSurfaceFromFace(int);

    poly_face* get_original_brush_face(int f_i);
    polyfold* get_original_brush(int f_i);

    void buildSceneNode(bool finalMesh, int light_mode);
    scene::CMeshSceneNode* getMeshNode() { return my_MeshNode; }
    void setFinalMeshDirty(bool dirty = true) { final_mesh_dirty = dirty; }
    void recalculate_final_meshbuffer();

    triangle_holder* get_triangles_for_face(int f_i);

    void setRenderType(bool brushes, bool geo, bool loops, bool triangles);
    void GetGeometryLoopLines(LineHolder&,LineHolder&);

    bool renderGeometry() { return render_geometry; }

    MeshNode_Interface_Edit edit_meshnode_interface;
    MeshNode_Interface_Final final_meshnode_interface;

    std::vector<geo_element> elements;

    REFLECT()

private:

    void trianglize_total_geometry();
    void generate_meshes();
    void build_total_geometry();
    void set_originals();

    int base_type = GEO_SOLID;

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

    LineHolder loops_graph;

    friend class Open_Geometry_File;
    friend class geometry_scene;
    friend class Geometry_Scene_File_IO;
   
};

#endif
