#ifndef _GEOMETRY_SCENE_H_
#define _GEOMETRY_SCENE_H_

#include <vector>
#include <irrlicht.h>
#include "BufferManager.h"
#include "USceneNode.h"
#include "GeometryStack.h"
#include "utils.h"

void print_geometry_ops_timers();
void reset_geometry_ops_timers();

class GeometryStack;
class Node_Classes_Base;
class Lightmap_Manager;
class SceneCoordinator;

struct scene_selection
{
    std::vector<int> brushes;
    std::vector<int> faces;
    std::vector<Reflected_SceneNode*> scene_nodes;
};

class Geometry_Scene_File_IO
{
    struct metadata
    {
        int n_scenes;
    };

public:
    Geometry_Scene_File_IO(io::IFileSystem*);

    void WriteToFiles(SceneCoordinator*);
    void ReadFromFiles(SceneCoordinator*);
    void AutoLoad(SceneCoordinator*, io::path);

private:

    bool WriteMetadata(SceneCoordinator* sc, std::string fname);
    bool ReadMetadata(std::string fname, metadata&);
    bool WriteTextureNames(geometry_scene*, std::string fname);
    bool ReadTextures(io::path fname, std::vector<std::wstring>&);

    bool SerializeGeometryToFile(geometry_scene*, std::string fname);
    bool DeserializeGeometryFromFile(geometry_scene*, io::path fname, io::path tex_fname);

    bool SerializeSceneNodesToFile(geometry_scene*, std::string fname);
    bool DeserializeSceneNodesFromFile(geometry_scene*, io::path);

    io::IFileSystem* FileSystem = NULL;
};

class geometry_scene : public irr::IEventReceiver
{
public:

    geometry_scene();
    geometry_scene(video::IVideoDriver* driver_, MyEventReceiver* receiver); //used only by uv_tool
    ~geometry_scene();

    void enable();
    void disable();

    void initialize(scene::ISceneManager* smgr_, video::IVideoDriver* driver_, MyEventReceiver* receiver);
    //void set_default_materials(video::E_MATERIAL_TYPE base_material_type_, video::E_MATERIAL_TYPE special_material_type_);
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

    void setSelectedFaces(std::vector<int>, bool force = false);
    void setSelectedFaces_ShiftAdd(int new_sel);
    std::vector<int> getSelectedFaces();
    //std::vector<int> getSurfaceFromFace(int);
    void selectSurfaceGroup();
    void selectionChanged();
    surface_group getFaceSurfaceGroup(int);

    std::vector<Reflected_SceneNode*> editor_node_ptrs_from_uid(const std::vector<u64>& selection);
    MeshBuffer_Chunk get_face_buffer_by_uid(u64);

    void TextureToSelectedFaces();

    bool progressive_build_enabled() {return this->progressive_build;}
    void toggle_progressive_build() {this->progressive_build = !this->progressive_build;}

    //==============Helper Functions for UI
    void setDragVec(core::vector3df vec) {
        drag_vec=vec;
    }
    core::vector3df getDragVec() {
        return drag_vec;
    }

    //===============Material Groups
    void MaterialGroupToSelectedFaces();
    void visualizeMaterialGroups();

    //void set_new_geometry_material(int mg) { new_geometry_material_group = mg; }

    void set_selected_material_group(int mg) { selected_material_group = mg; }
    int get_selected_material_group() { return selected_material_group; }
    //===============Mesh Node Interfaces

    bool IsEditNode();
    scene::CMeshSceneNode* getMeshNode();
    bool DynamicLightEnabled() { return b_dynamic_light; }

    //===============Reflected Scene Node Stuff
    void buildSceneGraph(bool addObjects, int light_mode, bool finalscene = false);
    void beginScene();
    void rebuildSceneGraph();

    void addSceneLight(core::vector3df pos);
    void setSelectedNodes(std::vector<Reflected_SceneNode*>);
    void setSelectedNodes_ShiftAdd(Reflected_SceneNode*);
    std::vector<Reflected_SceneNode*> getSelectedNodes();

    void deleteSelectedNodes();

    void addSceneSelectedSceneNodeType(core::vector3df pos);

    void set_selected_brush_vertex_editing(int);
    void drawGraph(LineHolder& graph);

    LineHolder special_graph;

    void setRenderType(bool brushes, bool geo, bool loops, bool triangles);

    scene::ISceneNode* EditorNodes() { return editor_nodes; }
    scene::ISceneNode* ActualNodes() { return actual_nodes; }

    GeometryStack* geoNode() { return geometry_stack; }

    void loadLightmapTextures();

    Reflected_SceneNode* get_reflected_node_by_uid(u64);

    std::vector<u64> get_reflected_node_uids_by_type(const char* node_type_name);
    std::vector<Reflected_SceneNode*> get_reflected_nodes_by_type(const char* node_type_name);

    void setFaceNodeType(const std::vector<int>&, const char* node_type_name);
    void addFaceNode(int f_i);

    void setFinalMeshDirty() { geometry_stack->final_mesh_dirty = true; }

    ISceneManager* get_smgr() { return smgr; }

    const std::string& name() { return scene_name; }
    void rename(const std::string& new_name);

    void save_gui_state();
    void restore_gui_state();
    void InitializeEmptyScene();
    int get_unique_id() { return scene_unique_inc_id; }
    
private:
    
    int build_progress=0;
    bool progressive_build=true;

    std::vector<int> selected_brushes;
    std::vector<int> selected_faces;
    std::vector<Reflected_SceneNode*> selected_scene_nodes;

    //scene_selection saved_selection;

    scene::ISceneManager* smgr=NULL;
    video::IVideoDriver* driver=NULL;
    MyEventReceiver* event_receiver=NULL;

    core::vector3df drag_vec;

    USceneNode* editor_nodes = NULL;
    USceneNode* actual_nodes = NULL;
    reflect::pointer<GeometryStack> geometry_stack;

    std::string scene_name;

    GUI_state_struct saved_gui_state;

    int scene_unique_inc_id=0;

    LineHolder intersections_graph;

    bool b_Visualize = false;
    bool bSceneInProgress = false;
    bool b_dynamic_light = false;

    int selected_material_group = 0;

    REFLECT()

    friend class Open_Geometry_File;
    friend class SceneCoordinator;
    friend class Geometry_Scene_File_IO;
    friend class GeometryStack;
};

class MySkybox_SceneNode;

class SceneCoordinator
{
    struct metadata
    {
        int n_scenes;
    };

public:
    SceneCoordinator(scene::ISceneManager* smgr_, video::IVideoDriver* driver_, MyEventReceiver* receiver);
    ~SceneCoordinator();

    geometry_scene* current_scene();
    geometry_scene* skybox_scene();
    geometry_scene* get_scene(int);
    void swap_scene(int);
    void add_scene();

    void rebuild_dirty_meshbuffers();

    ISceneManager* current_smgr();
    void CopyAllMaterials();
    void SetAllFinalMeshDirty();

    void connect_skybox(geometry_scene*);

    void clear();

private:
    //bool write_metadata(std::string fname);
    //bool read_metadata(std::string fname, metadata&);

    int scene_no = 0;
    
    std::vector<reflect::pointer<geometry_scene>> scenes;
    int unique_id_incrementer = 0;

    bool skybox_dirty = false;

    geometry_scene* m_skybox_scene = NULL;

    scene::ISceneManager* smgr;
    video::IVideoDriver* driver;
    MyEventReceiver* receiver;


    friend class Open_Geometry_File;
    friend class Save_Geometry_File;
    friend class Scene_Instances_Base;
    friend class Reflected_SkyNode;

    REFLECT()

    friend class Geometry_Scene_File_IO;
    
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
