#ifndef _GEOMETRY_SCENE_H_
#define _GEOMETRY_SCENE_H_

#include <vector>
#include <irrlicht.h>
#include "csg_classes.h"
#include "CMeshSceneNode.h"
#include "texture_picker.h"
#include "BufferManager.h"
#include "reflected_nodes.h"
#include "material_groups.h"
#include "NodeClassesTool.h"

void print_geometry_ops_timers();
void reset_geometry_ops_timers();

typedef core::list<scene::ISceneNode*> iNodeList;

class geo_element
{
public:
    int type=0;
    int selected_vertex=0;
    bool control_vertex_selected = false;
    bool bSelected=false;

    polyfold brush;
    polyfold geometry;

    bool has_geometry();
    void draw_brush(video::IVideoDriver* driver, const video::SMaterial material);
    void draw_geometry(video::IVideoDriver* driver, const video::SMaterial material);
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

class GeometryStack : public USceneNode
{
public:
    GeometryStack(ISceneNode* parent, scene::ISceneManager* smgr_, MyEventReceiver* receiver,
        video::E_MATERIAL_TYPE base_material_type_,
        video::E_MATERIAL_TYPE special_material_type_,
        TexturePicker_Base*,
        Material_Groups_Base*);

    GeometryStack(ISceneNode* parent, scene::ISceneManager* smgr_, MyEventReceiver* receiver);

    ~GeometryStack();

    virtual void GeometryStack::OnRegisterSceneNode() override;
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

    polyfold get_intersecting_geometry(polyfold pf);
    void build_intersecting_target(const polyfold& pf, polyfold& out);
    polyfold* get_total_geometry();
    std::vector<int> getSurfaceFromFace(int);

    poly_face* get_original_brush_face(int f_i);
    polyfold* get_original_brush(int f_i);

    void buildSceneNode(bool finalMesh, int light_mode);
    scene::CMeshSceneNode* getMeshNode() { return my_MeshNode; }
    void setFinalMeshDirty(bool dirty = true) { final_mesh_dirty = dirty; }

    triangle_holder* get_triangles_for_face(int f_i);

    void setRenderType(bool brushes, bool geo, bool loops, bool triangles);

    bool renderGeometry() { return render_geometry; }

    MeshNode_Interface_Edit edit_meshnode_interface;
    MeshNode_Interface_Final final_meshnode_interface;

    std::vector<geo_element> elements;

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

    TexturePicker_Base* texture_picker_base = NULL;
    Material_Groups_Base* material_groups_base = NULL;
    MyEventReceiver* event_receiver = NULL;

    video::E_MATERIAL_TYPE base_material_type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE special_material_type = video::EMT_SOLID;

    int new_geometry_material_group = 0;

    scene::CMeshSceneNode* my_MeshNode = NULL;

    bool final_mesh_dirty = false;
    bool b_isEditNode = true;
    bool render_brushes = false;
    bool render_geometry = true;
    bool render_loops = false;
    bool render_triangles = false;
    bool render_active_brush = true;

    LineHolder loops_graph;

    friend class Open_Geometry_File;
    friend class geometry_scene;
};

class geometry_scene : public irr::IEventReceiver
{
public:

    geometry_scene(video::IVideoDriver* driver_, MyEventReceiver* receiver);
    geometry_scene(scene::ISceneManager* smgr_,video::IVideoDriver* driver_,MyEventReceiver* receiver,video::E_MATERIAL_TYPE base_material_type_, video::E_MATERIAL_TYPE special_material_type_);
    ~geometry_scene();

    int base_type=GEO_SOLID;

    void set_type(int t) { base_type = t; }
    //std::vector<geo_element> elements;
    //void add(polyfold);
    //void subtract(polyfold);
    //void add_semisolid(polyfold);
    //void add_plane(polyfold);
    //void rebuild_geometry(bool = false);
    void clear_scene();

    virtual bool OnEvent(const SEvent& event);

    //polyfold get_intersecting_geometry(polyfold pf);
    //void build_intersecting_target(const polyfold& pf, polyfold& out);
    //polyfold* get_total_geometry();

   

    //========= Selection
    void setBrushSelection(std::vector<int>);
    void setBrushSelection_ShiftAdd(int);
    std::vector<int> getBrushSelection();
    core::vector3df getSelectedVertex();
    void delete_selected_brushes();

    //GeometryStack* getSelectedGeo();

    void setSelectedFaces(std::vector<int>);
    void setSelectedFaces_ShiftAdd(int new_sel);
    std::vector<int> getSelectedFaces();
    std::vector<int> getSurfaceFromFace(int);
    void selectSurfaceGroup();
    void selectionChanged();
    surface_group getFaceSurfaceGroup(int);

    std::vector<Reflected_SceneNode*> editor_node_ptrs_from_uid(const std::vector<u64>& selection);
    MeshBuffer_Chunk get_face_buffer_by_uid(u64);

    //poly_face* get_original_brush_face(int f_i);
    //polyfold* get_original_brush(int f_i);

    void setTexturePickerBase(TexturePicker_Base*);
    TexturePicker_Base* getTexturePickerBase();

    void TextureToSelectedFaces();

    bool progressive_build_enabled() {return this->progressive_build;}
    void toggle_progressive_build() {this->progressive_build = !this->progressive_build;}
    //void intersect_active_brush();
    //void clip_active_brush();
    //void clip_active_brush_plane_geometry();

    //==============Read and Write to file
    bool WriteTextures(std::string fname);
    bool ReadTextures(io::path fname, std::vector<std::wstring>&);

    bool Write2(std::string fname);
    bool Read2(io::path fname, io::path tex_fname);

    bool WriteSceneNodesToFile(std::string fname);
    bool ReadSceneNodesFromFile(io::path);

    bool ExportFinalMesh(std::string fname);

    //==============Helper Functions for UI
    void setDragVec(core::vector3df vec) {
        drag_vec=vec;
    }
    core::vector3df getDragVec() {
        return drag_vec;
    }

    //===============Material Groups
    void MaterialGroupToSelectedFaces();


    void setMaterialGroupsBase(Material_Groups_Base*);
    Material_Groups_Base* getMaterialGroupsBase();
    //void setFinalMeshDirty(bool dirty = true) { final_mesh_dirty = dirty; }
    void visualizeMaterialGroups();
    void showLightMaps();

    void setLightmapManager(Lightmap_Manager*);
    Lightmap_Manager* getLightmapManager() { return lightmap_manager; }

    void set_new_geometry_material(int mg) { new_geometry_material_group = mg; }

    void set_selected_material_group(int mg) { selected_material_group = mg; }
    int get_selected_material_group() { return selected_material_group; }
    //===============Mesh Node Interfaces

    bool IsEditNode(){return b_isEditNode; }
    //MeshNode_Interface_Edit edit_meshnode_interface;
    //MeshNode_Interface_Final final_meshnode_interface;
    scene::CMeshSceneNode* getMeshNode() { return geometry_stack->getMeshNode(); }
    bool DynamicLightEnabled() { return b_dynamic_light; }

    //===============Reflected Scene Node Stuff
    void buildSceneGraph(bool finalMesh, bool addObjects, int light_mode, bool finalscene = false);
    void rebuildSceneGraph();

    void addSceneLight(core::vector3df pos);
    void setSelectedNodes(std::vector<Reflected_SceneNode*>);
    void setSelectedNodes_ShiftAdd(Reflected_SceneNode*);
    std::vector<Reflected_SceneNode*> getSelectedNodes();

    void deleteSelectedNodes();

    reflect::TypeDescriptor_Struct* getSelectedNodeClass();
    void set_node_classes_base(Node_Classes_Base* base) {node_classes_base=base;}
    void addSceneSelectedSceneNodeType(core::vector3df pos);

    int selected_brush_vertex_editing;
    void drawGraph(LineHolder& graph);

    //triangle_holder* get_triangles_for_face(int f_i);

    LineHolder special_graph;

    //core::list<ISceneNode*> getGeometryNodes();
    //int nGeometryNodes();

    void setRenderType(bool brushes, bool geo, bool loops, bool triangles);

    ISceneNode* EditorNodes() { return editor_nodes; }
    ISceneNode* ActualNodes() { return actual_nodes; }
    //ISceneNode* GeometryNodes() { return geometry_nodes; }
    GeometryStack* geoNode() { return geometry_stack; }

private:
    
   // void trianglize_total_geometry();
   // void generate_meshes();
   // void build_total_geometry();
   // void set_originals();

    int build_progress=0;
    bool progressive_build=true;

    polyfold total_geometry;
    std::vector<int> selected_brushes;
    std::vector<int> selected_faces;
    std::vector<Reflected_SceneNode*> selected_scene_nodes;
    std::vector<int> edit_mesh_buffer_faces;

    scene::ISceneManager* smgr=NULL;
    video::IVideoDriver* driver=NULL;
    MyEventReceiver* event_receiver=NULL;

    TexturePicker_Base* texture_picker_base=NULL;
    Material_Groups_Base* material_groups_base=NULL;
    Node_Classes_Base* node_classes_base=NULL;
    Lightmap_Manager* lightmap_manager = NULL;

    //scene::CMeshSceneNode* my_MeshNode=NULL;

    int new_geometry_material_group = 0;
    video::E_MATERIAL_TYPE base_material_type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE special_material_type = video::EMT_SOLID;

    core::vector3df drag_vec;

    USceneNode* editor_nodes = NULL;
    USceneNode* actual_nodes = NULL;
    GeometryStack* geometry_stack = NULL;

    //USceneNode* geometry_nodes = NULL;

    LineHolder intersections_graph;

    bool b_Visualize = false;
    bool b_isEditNode = false;
    bool b_dynamic_light = false;
    bool final_mesh_dirty = false;

    std::vector<triangle_holder> total_geometry_triangles;

    int selected_material_group = 0;

    REFLECT()

    friend class Open_Geometry_File;
};

struct clip_results
{
    int n_intersections;
};

void clip_poly(polyfold& a, polyfold& b, int rule, int rule2, clip_results& results, LineHolder&);
void clip_poly_single(polyfold& pf, polyfold& pf2, int rule, int base_type, clip_results& results, LineHolder &graph);

void clip_poly7(polyfold& pf, polyfold& pf2, int rule, int base_type, clip_results& results, LineHolder& graph);

polyfold make_poly_cube(int,int,int);
polyfold make_poly_plane(int,int);
polyfold make_cylinder(int height, int radius, int faces, int radius_type);
polyfold make_sphere(int radius, int faces, int zen_faces, bool simplify);
polyfold make_cone(int height, int radius, int faces);


#endif
