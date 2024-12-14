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
class GeometryStack;

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
        rect<u16> block;
        vector3df bounding_verts[4];
    };

    video::ITexture* texture;
    int materialGroup;
    int n_faces;
    int n_triangles;

    bool has_lightmap = true;
    int lightmap_size;

    std::vector<int> faces;
    std::vector<lightmap_record> records;
};


class MeshNode_Interface
{
public:

    MeshNode_Interface(scene::ISceneManager* smgr_,video::IVideoDriver* driver_,MyEventReceiver* receiver,video::E_MATERIAL_TYPE base_material_type_, video::E_MATERIAL_TYPE special_material_type_);
    
    MeshNode_Interface();
    ~MeshNode_Interface();

    void init(scene::ISceneManager* smgr_,video::IVideoDriver* driver_,MyEventReceiver* receiver,video::E_MATERIAL_TYPE base_material_type_, video::E_MATERIAL_TYPE special_material_type_);

    virtual void generate_mesh_node(GeometryStack* geo_scene) = 0;
    virtual MeshBuffer_Chunk get_mesh_buffer_by_face(int f_i) = 0;

    scene::SMesh* getMesh() { return m_mesh; }

    void recalc_uvs_for_face(GeometryStack* geo_scene, int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_cube(GeometryStack* geo_scene, int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_cylinder(GeometryStack* geo_scene, int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_sphere(GeometryStack* geo_scene, int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_dome(GeometryStack* geo_scene, int e_i, int f_i, int f_j);
    void recalc_uvs_for_face_custom(GeometryStack* geo_scene, int e_i, int f_i, int f_j);

    CMeshSceneNode* addMeshSceneNode(ISceneNode* parent, scene::ISceneManager* smgr,GeometryStack* geo_scene);

    std::vector<TextureMaterial> getMaterialsUsed(){return materials_used;}

protected:
    
    virtual void generate_mesh_buffer(GeometryStack* geo_scene,SMesh*)=0;
    void generate_uvs(GeometryStack* geo_scene);


    scene::SMesh* m_mesh = NULL;

    std::vector<TextureMaterial> materials_used;
    //std::vector<int> mg_to_reference_face;

    scene::ISceneManager* smgr=NULL;
    video::IVideoDriver* driver=NULL;
    TexturePicker* texture_picker=NULL;
    MyEventReceiver* event_receiver=NULL;

    video::E_MATERIAL_TYPE base_material_type;
    video::E_MATERIAL_TYPE special_material_type;

    friend class GeometryStack;
    friend class GeometryStack;
};

class MeshNode_Interface_Edit : public MeshNode_Interface
{
public:
    void generate_mesh_node(GeometryStack* geo_scene);
    virtual MeshBuffer_Chunk get_mesh_buffer_by_face(int f_i);
    int get_buffer_index_by_face(int i);

protected:

    virtual void generate_mesh_buffer(GeometryStack* geo_scene,SMesh*);
    std::vector<int> face_to_mb_buffer;

    friend class GeometryStack;
};

class MeshNode_Interface_Final : public MeshNode_Interface
{
public:

    
    void generate_mesh_node(GeometryStack* geo_scene);

    virtual  MeshBuffer_Chunk get_mesh_buffer_by_face(int f_i);
    int get_buffer_index_by_face(int f_i);
  // int get_material_reference_face(int material_group){return mg_to_reference_face[material_group];}

protected:
    void refresh_material_groups(GeometryStack* geo_scene);
    virtual void generate_mesh_buffer(GeometryStack* geo_scene,SMesh*);
    void copy_lightmap_uvs(GeometryStack* geo_scene);

    std::vector<int> face_to_mb_buffer;
    std::vector<int> face_to_mb_begin;
    std::vector<int> face_to_mb_end;

    friend class GeometryStack;
};



#endif
