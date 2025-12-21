#ifndef _BUFFER_MANAGER_H_
#define _BUFFER_MANAGER_H_

#include <irrlicht.h>
#include <vector>
#include "Reflection.h"
#include "csg_classes.h" //only for face_index
#include "soa.h"

using namespace irr;

class polyfold;
class GeometryStack;
namespace irr
{
    namespace scene
    {
        class CMeshSceneNode;
    }
}
class MyEventReceiver;
class TexturePicker;

struct Lightmap_Block
{
    std::vector<int> faces;
    int element_id;
    int surface_no;
    u32 width = 128;
    u32 height = 128;
    bool bFlipped = false;
    bool bOverrideSize = false;
    //int bounding_verts_index0 = 0;

    REFLECT()
};

struct MeshBuffer_Chunk
{
    scene::IMeshBuffer* buffer;
    unsigned int begin_i;
    unsigned int end_i;
};

struct MeshIndex_Chunk
{
    unsigned int buffer;
    unsigned int begin_i;
    unsigned int end_i;
};

class TextureInfo;

struct TextureMaterial
{
    TextureInfo* texture;
    int materialGroup;
    int n_faces = 0;
    int n_triangles = 0;

    bool has_lightmap = false;
    int lightmap_size;
    int lightmap_no;
    int texture_idx;                            //index for accessing on the GPU
    //int original_material_no;

    std::vector<int> faces;                     //unique to entire scene
    std::vector<face_index> my_faces;           //local to element
    std::vector<std::pair<int, int>> surfaces;  //pair: element no, surface no
    //std::vector<int> surfaces;
    std::vector<Lightmap_Block> blocks;
};

class Reflected_MeshBuffer_AreaLight_SceneNode;

class MeshNode_Interface
{
public:

    MeshNode_Interface(scene::ISceneManager* smgr_,video::IVideoDriver* driver_,MyEventReceiver* receiver);
    
    MeshNode_Interface(GeometryStack* geo_scene);
    ~MeshNode_Interface();

    void init(scene::ISceneManager* smgr_,video::IVideoDriver* driver_,MyEventReceiver* receiver);

    virtual void generate_mesh_node() = 0;
    virtual MeshBuffer_Chunk get_mesh_buffer_by_face(int f_i) = 0;
    virtual MeshBuffer_Chunk get_mesh_buffer(int f_i) = 0;

    scene::SMesh* getMesh() { return m_mesh; }

    void recalc_uvs_for_face(int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_cube(int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_cylinder(int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_sphere(int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_dome(int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_custom(int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_canonical(int e_i, int f_i, int f_j);

    void calc_vertex_lighting_vectors(vector<Reflected_MeshBuffer_AreaLight_SceneNode*>&, LineHolder& graph);

    scene::CMeshSceneNode* addMeshSceneNode(scene::ISceneNode* parent, scene::ISceneManager* smgr,GeometryStack* geo_scene);

    virtual const std::vector<TextureMaterial>& get_materials_used() = 0;

    int get_material_group_by_face(int f_i);

    soa_struct<aligned_vec3> geometry_raw_vertices;
    void fill_raw_vertices_buffer();

    soa_struct<aligned_uint> indices_soa; //for indexing into a SOA buffer

    void fill_surface_index();

    soa_struct<MeshIndex_Chunk> surface_index;

protected:
    
    virtual void generate_mesh_buffer(scene::SMesh*)=0;
    void generate_uvs();

    scene::SMesh* m_mesh = NULL;

    scene::ISceneManager* smgr=NULL;
    video::IVideoDriver* driver=NULL;
    TexturePicker* texture_picker=NULL;
    MyEventReceiver* event_receiver=NULL;

    std::vector<int> face_to_material;
    
    GeometryStack* geo_scene = NULL;

    friend class GeometryStack;
    friend class Lightmap_Configuration;
};

class MeshNode_Interface_Edit : public MeshNode_Interface
{
public:
    MeshNode_Interface_Edit(GeometryStack* geo_scene);

    void generate_mesh_node();
    void generate_lightmap_info();
    virtual MeshBuffer_Chunk get_mesh_buffer_by_face(int f_i);
    virtual MeshBuffer_Chunk get_mesh_buffer(int f_i);
    int get_buffer_index_by_face(int i);

    void resize_lightmap_block(core::vector2di, Lightmap_Block&);

    //std::vector<core::vector2df>* get_lightmap_raw_uvs_by_face(int f_i);
    int get_lm_block_by_face(int f_i);
    Lightmap_Block& get_lm_block(int i, int j);

    void refresh_material_groups();
    virtual const std::vector<TextureMaterial>& get_materials_used() { return materials_used; }

protected:

    virtual void generate_mesh_buffer(scene::SMesh*);

    //void initialize_soa_arrays();

    std::vector<int> face_to_mb_buffer;

    std::vector<TextureMaterial> materials_used;

    friend class GeometryStack;
    
};

class MeshNode_Interface_Final : public MeshNode_Interface
{
public:
    MeshNode_Interface_Final(GeometryStack* geo_scene);

    void generate_mesh_node();
    void generate_lightmap_info();

    virtual  MeshBuffer_Chunk get_mesh_buffer_by_face(int f_i);
    virtual MeshBuffer_Chunk get_mesh_buffer(int f_i);
    int get_buffer_index_by_face(int f_i);

    virtual const std::vector<TextureMaterial>& get_materials_used();

    void copy_lightmap_uvs();

    int get_buffer_index(int);
    int get_n_triangles(int);
    int get_first_triangle(int);

    //===========================
    //This just converts an index on final buffer into an index on the edit buffer, since the order is different
    std::vector<int> edit_mb_buffer;
    

//protected:

    virtual void generate_mesh_buffer(scene::SMesh*);

    std::vector<int> face_to_mb_buffer;
    std::vector<int> face_to_mb_begin;
    std::vector<int> face_to_mb_end;
    std::vector<int> face_to_mb_faces;

    std::vector<int> surface_to_mb_buffer;
    std::vector<int> surface_to_mb_begin;
    std::vector<int> surface_to_mb_end;

    std::vector<int> mb_buffer;
    std::vector<int> mb_begin;
    std::vector<int> mb_end;

    friend class GeometryStack;
};

#endif
