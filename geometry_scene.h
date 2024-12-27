#ifndef _GEOMETRY_SCENE_H_
#define _GEOMETRY_SCENE_H_

#include <vector>
#include <irrlicht.h>
#include "BufferManager.h"
#include "USceneNode.h"
#include "GeometryStack.h"

void print_geometry_ops_timers();
void reset_geometry_ops_timers();

class GeometryStack;
class Node_Classes_Base;
class Lightmap_Manager;

struct scene_selection
{
    std::vector<int> brushes;
    std::vector<int> faces;
    std::vector<Reflected_SceneNode*> scene_nodes;
};

class geometry_scene : public irr::IEventReceiver
{
public:

    geometry_scene();
    geometry_scene(video::IVideoDriver* driver_, MyEventReceiver* receiver);
    geometry_scene(scene::ISceneManager* smgr_,video::IVideoDriver* driver_,MyEventReceiver* receiver,video::E_MATERIAL_TYPE base_material_type_, video::E_MATERIAL_TYPE special_material_type_);
    ~geometry_scene();

    void initialize(scene::ISceneManager* smgr_, video::IVideoDriver* driver_, MyEventReceiver* receiver, video::E_MATERIAL_TYPE base_material_type_, video::E_MATERIAL_TYPE special_material_type_);

    int base_type=GEO_SOLID;

    void set_type(int t) { base_type = t; }
    void clear_scene();

    virtual bool OnEvent(const SEvent& event);
   
    //========= Selection
    void setBrushSelection(std::vector<int>);
    void setBrushSelection_ShiftAdd(int);
    std::vector<int> getBrushSelection();
    core::vector3df getSelectedVertex();
    void delete_selected_brushes();

    void setSelectedFaces(std::vector<int>);
    void setSelectedFaces_ShiftAdd(int new_sel);
    std::vector<int> getSelectedFaces();
    //std::vector<int> getSurfaceFromFace(int);
    void selectSurfaceGroup();
    void selectionChanged();
    surface_group getFaceSurfaceGroup(int);

    std::vector<Reflected_SceneNode*> editor_node_ptrs_from_uid(const std::vector<u64>& selection);
    MeshBuffer_Chunk get_face_buffer_by_uid(u64);

    void setTexturePickerBase(TexturePicker_Base*);
    TexturePicker_Base* getTexturePickerBase();

    void TextureToSelectedFaces();

    bool progressive_build_enabled() {return this->progressive_build;}
    void toggle_progressive_build() {this->progressive_build = !this->progressive_build;}

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

    bool IsEditNode();
    scene::CMeshSceneNode* getMeshNode();
    bool DynamicLightEnabled() { return b_dynamic_light; }

    //===============Reflected Scene Node Stuff
    void buildSceneGraph(bool addObjects, int light_mode, bool finalscene = false);
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

    scene::ISceneNode* EditorNodes() { return editor_nodes; }
    scene::ISceneNode* ActualNodes() { return actual_nodes; }
    //ISceneNode* GeometryNodes() { return geometry_nodes; }
    GeometryStack* geoNode() { return geometry_stack.value; }
    //int nGeoNodes() { return geometry_stack.size(); }

    void loadLightmapTextures();

    void save_selection();
    scene_selection get_saved_selection();
    std::vector<u64> get_saved_selection_uids();
    Reflected_SceneNode* get_reflected_node_by_uid(u64);
    void addFaceNode(int f_i);

    void setFinalMeshDirty() { geometry_stack->final_mesh_dirty = true; }

private:
    
    int build_progress=0;
    bool progressive_build=true;

    //polyfold total_geometry;
    std::vector<int> selected_brushes;
    std::vector<int> selected_faces;
    std::vector<Reflected_SceneNode*> selected_scene_nodes;

    scene_selection saved_selection;

    scene::ISceneManager* smgr=NULL;
    video::IVideoDriver* driver=NULL;
    MyEventReceiver* event_receiver=NULL;

    TexturePicker_Base* texture_picker_base=NULL;
    Material_Groups_Base* material_groups_base=NULL;
    Node_Classes_Base* node_classes_base=NULL;
    Lightmap_Manager* lightmap_manager = NULL;

    int new_geometry_material_group = 0;
    video::E_MATERIAL_TYPE base_material_type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE special_material_type = video::EMT_SOLID;

    core::vector3df drag_vec;

    USceneNode* editor_nodes = NULL;
    USceneNode* actual_nodes = NULL;
    reflect::pointer<GeometryStack> geometry_stack;

    LineHolder intersections_graph;

    bool b_Visualize = false;
    bool bSceneInProgress = false;
    bool b_dynamic_light = false;

    //std::vector<triangle_holder> total_geometry_triangles;

    int selected_material_group = 0;

    REFLECT()

    friend class Open_Geometry_File;
};

class Geometry_Scene_Manager
{
public:

    std::vector<geometry_scene> geo_scenes;

    REFLECT()
};

struct clip_results
{
    int n_intersections;
};

void clip_poly(polyfold& a, polyfold& b, int rule, int rule2, clip_results& results, LineHolder&);
void clip_poly_single(polyfold& pf, polyfold& pf2, int rule, int base_type, clip_results& results, LineHolder &graph);

//void clip_poly7(polyfold& pf, polyfold& pf2, int rule, int base_type, clip_results& results, LineHolder& graph);

polyfold make_poly_cube(int,int,int);
polyfold make_poly_plane(int,int);
polyfold make_cylinder(int height, int radius, int faces, int radius_type);
polyfold make_sphere(int radius, int faces, int zen_faces, bool simplify);
polyfold make_cone(int height, int radius, int faces);


#endif
