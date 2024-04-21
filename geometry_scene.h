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

void print_geometry_ops_timers();
void reset_geometry_ops_timers();

class geo_element
{
public:
    int type=0;
    int selected_vertex=0;
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

enum
{
    SCENE_EDIT_UNLIT,
    SCENE_EDIT_LIGHTING,
    SCENE_FINAL_UNLIT,
    SCENE_FINAL_LIGHTING
};

class MyEventReceiver;
class geometry_scene : public irr::IEventReceiver
{
public:

    geometry_scene(video::IVideoDriver* driver_, MyEventReceiver* receiver);
    geometry_scene(scene::ISceneManager* smgr_,video::IVideoDriver* driver_,MyEventReceiver* receiver,video::E_MATERIAL_TYPE base_material_type_, video::E_MATERIAL_TYPE special_material_type_);
    ~geometry_scene();

    int base_type=GEO_SOLID;

    void set_type(int);
    std::vector<geo_element> elements;
    void add(polyfold);
    void subtract(polyfold);
    void add_semisolid(polyfold);
    void add_plane(polyfold);
    void rebuild_geometry(bool = false);
    void clear_scene();

    virtual bool OnEvent(const SEvent& event);

    polyfold get_intersecting_geometry(polyfold pf);
    void build_intersecting_target(const polyfold& pf, polyfold& out);
    polyfold* get_total_geometry();

    void setBrushSelection(std::vector<int>);
    void setBrushSelection_ShiftAdd(int);
    std::vector<int> getBrushSelection();
    core::vector3df getSelectedVertex();
    void delete_selected_brushes();

    //========= Selection
    void setSelectedFaces(std::vector<int>);
    void setSelectedFaces_ShiftAdd(int new_sel);
    std::vector<int> getSelectedFaces();
    std::vector<int> getSurfaceFromFace(int);
    void selectSurfaceGroup();
    void selectionChanged();
    surface_group getFaceSurfaceGroup(int);


    poly_face* get_original_brush_face(int f_i);
    polyfold* get_original_brush(int f_i);

    void setTexturePickerBase(TexturePicker_Base*);
    TexturePicker_Base* getTexturePickerBase();

    void TextureToSelectedFaces();

    bool progressive_build_enabled() {return this->progressive_build;}
    void toggle_progressive_build() {this->progressive_build = !this->progressive_build;}
    void intersect_active_brush();
    void clip_active_brush();
    void clip_active_brush_plane_geometry();

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
   void setFinalMeshDirty(bool dirty = true) { final_mesh_dirty = dirty; }
   void visualizeMaterialGroups();
   void showLightMaps();

   void set_new_geometry_material(int mg) { new_geometry_material_group = mg; }

    //===============Mesh Node Interfaces

    bool IsEditNode(){return b_isEditNode; }
    MeshNode_Interface_Edit edit_meshnode_interface;
    MeshNode_Interface_Final final_meshnode_interface;
    scene::CMeshSceneNode* getMeshNode() { return my_MeshNode; }
    bool DynamicLightEnabled() { return b_dynamic_light; }

    //===============Reflected Scene Node Stuff
    void buildSceneGraph(bool finalMesh, bool addObjects, bool addLights, bool finalscene = false);
    void rebuildSceneGraph();

    void addSceneLight(core::vector3df pos);
    std::vector<Reflected_SceneNode*> getSceneNodes();
    void setSelectedNodes(std::vector<int>);
    void setSelectedNodes_ShiftAdd(int new_sel);
    std::vector<int> getSelectedNodes();
    Reflected_SceneNode* getSelectedSceneNode(int i) {return getSceneNodes()[getSelectedNodes()[i]];}
    void deleteSelectedNodes();

    reflect::TypeDescriptor_Struct* getChooseNodeType();
    void set_choose_reflected_node_base(ListReflectedNodes_Base* base) {choose_reflected_node_base=base;}
    void addSceneSelectedSceneNodeType(core::vector3df pos);

    int selected_brush_vertex_editing;
    void drawGraph(LineHolder& graph);

    triangle_holder* get_triangles_for_face(int f_i);

private:
    
    void trianglize_total_geometry();
    void generate_meshes();
    void build_total_geometry();
    void set_originals();

    int build_progress=0;
    bool progressive_build=true;

    polyfold total_geometry;
    std::vector<int> selected_brushes;
    std::vector<int> selected_faces;
    std::vector<int> selected_scene_nodes;
    std::vector<int> edit_mesh_buffer_faces;

    scene::ISceneManager* smgr=NULL;
    video::IVideoDriver* driver=NULL;
    MyEventReceiver* event_receiver=NULL;

    TexturePicker_Base* texture_picker_base=NULL;
    Material_Groups_Base* material_groups_base=NULL;
    ListReflectedNodes_Base* choose_reflected_node_base=NULL;

    scene::CMeshSceneNode* my_MeshNode=NULL;

    int new_geometry_material_group = 0;
    video::E_MATERIAL_TYPE base_material_type;
    video::E_MATERIAL_TYPE special_material_type;

    core::vector3df drag_vec;
    std::vector<Reflected_SceneNode*> scene_nodes;

    LineHolder intersections_graph;

    bool b_Visualize = false;
    bool b_isEditNode = false;
    bool b_dynamic_light = false;
    bool final_mesh_dirty = false;

    std::vector<triangle_holder> total_geometry_triangles;

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
polyfold make_cylinder(int height, int radius, int faces);
polyfold make_sphere(int radius, int faces, int zen_faces, bool simplify);
polyfold make_cone(int height, int radius, int faces);


#endif
