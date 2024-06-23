#ifndef _BUFFER_MANAGER_H_
#define _BUFFER_MANAGER_H_

#include <irrlicht.h>
#include <vector>
#include <array>

#include "CMeshSceneNode.h"
#include "texture_picker.h"
#include "edit_env.h"


using namespace irr;
using namespace scene;

class polyfold;

struct MeshBuffer_Chunk
{
    IMeshBuffer* buffer;
    unsigned int begin_i;
    unsigned int end_i;
};

struct TextureMaterial
{
    struct lightmap_record
    {
        int face;
        int face_v0_index;
        rect<u16> block;
    };

    video::ITexture* texture;
    int materialGroup;
    int n_faces;
    int n_triangles;

    bool has_lightmap = true;
    int lightmap_size;

    std::vector<int> faces;
};


class MeshNode_Interface
{
public:

    MeshNode_Interface(scene::ISceneManager* smgr_,video::IVideoDriver* driver_,MyEventReceiver* receiver,video::E_MATERIAL_TYPE base_material_type_, video::E_MATERIAL_TYPE special_material_type_);
    
    MeshNode_Interface();
    ~MeshNode_Interface();

    void init(scene::ISceneManager* smgr_,video::IVideoDriver* driver_,MyEventReceiver* receiver,video::E_MATERIAL_TYPE base_material_type_, video::E_MATERIAL_TYPE special_material_type_);

    virtual void generate_mesh_node(geometry_scene* geo_scene) = 0;
    virtual MeshBuffer_Chunk get_mesh_buffer_by_face(int f_i) = 0;

    scene::SMesh* getMesh() { return m_mesh; }

    void recalc_uvs_for_face(geometry_scene* geo_scene, int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_cube(geometry_scene* geo_scene, int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_cylinder(geometry_scene* geo_scene, int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_sphere(geometry_scene* geo_scene, int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_dome(geometry_scene* geo_scene, int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_custom(geometry_scene* geo_scene, int e_i, int f_i, int f_j);

    CMeshSceneNode* addMeshSceneNode(scene::ISceneManager* smgr,geometry_scene* geo_scene);

    std::vector<TextureMaterial> getMaterialsUsed(){return materials_used;}

protected:

    virtual void generate_mesh_buffer(geometry_scene* geo_scene,SMesh*)=0;
    void generate_uvs(geometry_scene* geo_scene);


    scene::SMesh* m_mesh = NULL;

    std::vector<TextureMaterial> materials_used;
    //std::vector<int> mg_to_reference_face;

    scene::ISceneManager* smgr=NULL;
    video::IVideoDriver* driver=NULL;
    TexturePicker* texture_picker=NULL;
    MyEventReceiver* event_receiver=NULL;

    video::E_MATERIAL_TYPE base_material_type;
    video::E_MATERIAL_TYPE special_material_type;
    friend class geometry_scene;
};

class MeshNode_Interface_Edit : public MeshNode_Interface
{
public:
    void generate_mesh_node(geometry_scene* geo_scene);
    virtual MeshBuffer_Chunk get_mesh_buffer_by_face(int f_i);
    int get_buffer_index_by_face(int i);

protected:

    virtual void generate_mesh_buffer(geometry_scene* geo_scene,SMesh*);
    std::vector<int> face_to_mb_buffer;

    friend class geometry_scene;
};

class MeshNode_Interface_Final : public MeshNode_Interface
{
public:

    
    void generate_mesh_node(geometry_scene* geo_scene);

    virtual  MeshBuffer_Chunk get_mesh_buffer_by_face(int f_i);
    int get_buffer_index_by_face(int f_i);
  // int get_material_reference_face(int material_group){return mg_to_reference_face[material_group];}

protected:
    void refresh_material_groups(geometry_scene* geo_scene);
    virtual void generate_mesh_buffer(geometry_scene* geo_scene,SMesh*);
    void copy_lightmap_uvs(geometry_scene* geo_scene);

    std::vector<int> face_to_mb_buffer;
    std::vector<int> face_to_mb_begin;
    std::vector<int> face_to_mb_end;

    friend class geometry_scene;
};



#endif
