
#include <irrlicht.h>
#include "CMeshSceneNode.h"
#include "BufferManager.h"
#include "csg_classes.h"
#include "geometry_scene.h"
#include "material_groups.h"
#include "tolerances.h"
#include "LightMaps.h"
#include "build_meshes.h"
#include "soa.h"

using namespace irr;
using namespace scene;

MeshNode_Interface::MeshNode_Interface(scene::ISceneManager* smgr_,video::IVideoDriver* driver_,MyEventReceiver* receiver)
{
    this->smgr = smgr_;
    this->driver = driver_;
    this->event_receiver = receiver;
}

void MeshNode_Interface::init(scene::ISceneManager* smgr_,video::IVideoDriver* driver_,MyEventReceiver* receiver)
{
    this->smgr = smgr_;
    this->driver = driver_;
    this->event_receiver = receiver;
}

MeshNode_Interface::MeshNode_Interface(GeometryStack* geo_scene) : geo_scene(geo_scene)
{
}

MeshNode_Interface::~MeshNode_Interface()
{
    if(this->m_mesh)
        this->m_mesh->drop();
}

void MeshNode_Interface_Edit::refresh_material_groups()
{
    materials_used.clear();
    int surf_no = 0;

    for(int e_i = 1; e_i < geo_scene->elements.size(); e_i++)
        for (int s_i = 0; s_i < geo_scene->elements[e_i].surfaces.size(); s_i++, surf_no++)
        {
            poly_surface* s = &geo_scene->elements[e_i].surfaces[s_i];

            int n_loops = 0;
            for (int f_i : s->my_faces)
            {
                n_loops += geo_scene->elements[e_i].geometry.faces[f_i].loops.size();
            }

            if (n_loops > 0)
            {
                video::ITexture* tex_j = driver->getTexture(s->texture_name);

                bool b = false;

                for (TextureMaterial& tm : materials_used)
                {
                    if (tm.texture == tex_j && tm.materialGroup == s->material_group)
                    {
                        b = true;
                        
                        tm.surfaces.push_back(std::pair<int, int>{geo_scene->elements[e_i].element_id, s_i});
                        for (int f_i : s->my_faces)
                        {
                            tm.n_faces++;
                            tm.faces.push_back(s->face_index_offset + f_i);
                        }
                    }
                }
                if (!b)
                {
                    materials_used.push_back(TextureMaterial{ tex_j,s->material_group,0 });
                    TextureMaterial& tm = materials_used[materials_used.size() - 1];

                    tm.surfaces.push_back(std::pair<int, int>{geo_scene->elements[e_i].element_id, s_i});
                    for (int f_i : s->my_faces)
                    {
                        tm.n_faces++;
                        tm.faces.push_back(s->face_index_offset + f_i);
                    }
                }
            }
        }

    Material_Groups_Base* materials_base = Material_Groups_Tool::get_base();

    for (TextureMaterial& mat : materials_used)
    {
        int mat_no = mat.materialGroup;
        if (mat_no < materials_base->material_groups.size() && materials_base->material_groups[mat_no].lightmap)
        {
            mat.has_lightmap = true;
        }
        else
        {
            mat.has_lightmap = false;
        }
    }

    face_to_material.assign(geo_scene->n_faces(), -1);

    for (int i = 0; i < materials_used.size(); i++)
    {
        //materials_used[i].original_material_no = i;

        for (int j = 0; j < materials_used[i].faces.size(); j++)
        {
            int f_i = materials_used[i].faces[j];
        }
    }
}

MeshNode_Interface_Edit::MeshNode_Interface_Edit(GeometryStack* geo_scene) : MeshNode_Interface(geo_scene)
{
}

MeshNode_Interface_Final::MeshNode_Interface_Final(GeometryStack* geo_scene) : MeshNode_Interface(geo_scene)
{
}

void MeshNode_Interface_Edit::generate_mesh_node()
{
    if(m_mesh)
        m_mesh->drop();

    this->m_mesh = new scene::SMesh;

    this->refresh_material_groups();

    this->generate_mesh_buffer(m_mesh);
    this->generate_uvs();
    
    this->fill_raw_vertices_buffer();
    this->generate_lightmap_info();
}

void MeshNode_Interface_Edit::generate_lightmap_info()
{
    polyfold* pf = geo_scene->get_total_geometry();

    for (int f_i = 0; f_i < geo_scene->n_faces(); f_i++)
    {
        pf->faces[f_i].temp_b = false;
    }


    for (int i = 0; i < materials_used.size(); i++)
    {
        if (!materials_used[i].has_lightmap)
            continue;

        auto lightmap_blocks_back = std::back_inserter(materials_used[i].blocks);

        for (int j = 0; j < materials_used[i].surfaces.size(); j++)
        {
            int e_i = materials_used[i].surfaces[j].first;
            int s_i = materials_used[i].surfaces[j].second;

            initialize_lightmap_block(geo_scene, e_i, s_i, lightmap_blocks_back, 0);
        }
    }

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        pf->faces[f_i].temp_b = false;
    }

    geo_scene->get_lightmap_config(0).initialize_soa_arrays(this->m_mesh);
    geo_scene->get_lightmap_config(1).initialize_soa_arrays(this->m_mesh);

    geo_scene->get_lightmap_config(0).set_reduction(1);
    geo_scene->get_lightmap_config(1).set_reduction(2);

    geo_scene->get_lightmap_config(0).calc_lightmap_uvs(materials_used);
    geo_scene->get_lightmap_config(1).calc_lightmap_uvs(materials_used);

    //This step splits material groups into smaller groups according to what can fit onto a single lightmap.
    //Needs to be done before generating the mesh buffer
    geo_scene->get_lightmap_config(0).split_material_groups();
    geo_scene->get_lightmap_config(1).split_material_groups();
}

void MeshNode_Interface_Edit::resize_lightmap_block(vector2di new_size, Lightmap_Block& b)
{
    b.width = new_size.X;
    b.height = new_size.Y;

    if (b.height > b.width)
    {
        swap(b.width, b.height);
        b.bFlipped = true;
    }

    b.bOverrideSize = true;

    //calc_lightmap_uvs(geo_scene, b);
}

void MeshNode_Interface_Final::generate_mesh_node()
{
    if (m_mesh)
        m_mesh->drop();

    this->m_mesh = new scene::SMesh;

    this->generate_mesh_buffer(m_mesh);
    this->fill_raw_vertices_buffer();

    //Index faces to buffers
    for (int i = 0; i < 2; i++)
    {
        for (TextureMaterial& tm : geo_scene->get_lightmap_config(i).materials)
        {
            for (int& f_i : tm.faces)
            {
                int f_j = face_to_mb_faces[f_i];
                edit_mb_buffer[f_j] = geo_scene->edit_meshnode_interface.get_buffer_index_by_face(f_i);
                f_i = f_j;
                
            }
        }
    }

    this->generate_uvs();
}

const std::vector<TextureMaterial>& MeshNode_Interface_Final::get_materials_used()
{
    return geo_scene->get_lightmap_config().materials;
}

void MeshNode_Interface_Final::generate_lightmap_info()
{
    geo_scene->get_lightmap_config(0).layout_lightmaps();
    geo_scene->get_lightmap_config(1).layout_lightmaps();

    geo_scene->get_lightmap_config(0).transform_lightmap_uvs();
    geo_scene->get_lightmap_config(0).apply_lightmap_uvs_to_mesh();

    geo_scene->get_lightmap_config(1).transform_lightmap_uvs();

}

CMeshSceneNode* MeshNode_Interface::addMeshSceneNode(ISceneNode* parent, scene::ISceneManager* smgr0,GeometryStack* geo_scene)
{
    if(this->m_mesh && smgr0)
    {
        CMeshSceneNode* mesh_node = new scene::CMeshSceneNode(this->m_mesh,parent,smgr0,747);
        mesh_node->drop();
        mesh_node->SetBox(geo_scene->get_total_geometry()->bbox);
        mesh_node->setVisible(true);
        return mesh_node;
    }

    return NULL;
}

void MeshNode_Interface_Edit::generate_mesh_buffer(SMesh* mesh)
{
    scene::IMeshBuffer* buffer;

    int b_count=0;
    int i_count=0;
    int v_count=0;
    LineHolder nograph;

    face_to_mb_buffer.clear();
    //lightmap_raw_uvs.clear();

    fill_surface_index();

    polyfold* pf = geo_scene->get_total_geometry();

    this->face_to_mb_buffer.assign(pf->faces.size(), -1);

    for (int t_i = 0; t_i < materials_used.size(); t_i++)
    {

        int total_indices = 0;

        std::vector<triangle_holder> triangle_groups;

        for (std::pair<int, int> surf : materials_used[t_i].surfaces)
        {
            int e_i = geo_scene->get_element_index_by_id(surf.first);
            int s_i = surf.second;

            surface_index.data[surface_index.offset[e_i] + s_i].begin_i = i_count;

            for (int f_i0 : geo_scene->elements[e_i].surfaces[s_i].my_faces)
            {
                int f_i = geo_scene->elements[e_i].reverse_index[f_i0];

                

                this->face_to_mb_buffer[f_i] = b_count;

                const triangle_holder& th = *geo_scene->get_triangles_for_face(f_i);

                buffer = new scene::CMeshBuffer<video::S3DVertex2TCoords>();

                buffer->getMaterial().setTexture(0, driver->getTexture(geo_scene->face_texture_by_n(f_i).c_str()));

                make_meshbuffer_from_triangles(th, buffer);
                mesh->addMeshBuffer(buffer);

                b_count++;

                i_count += buffer->getIndexCount();
                v_count += buffer->getVertexCount();

                

                //lightmap_raw_uvs.push_back(std::vector<core::vector2df>{});
                //lightmap_raw_uvs[lightmap_raw_uvs.size() - 1].resize(buffer->getVertexCount());
            }

            surface_index.data[surface_index.offset[e_i] + s_i].end_i = i_count;
        }
    }
     //std::cout<<""<<b_count<<" buffers with "<<v_count<<" vertices and "<<i_count/3<<" triangles... done\n";

    fill_index_struct_with_offsets(mesh, indices_soa);
}

void MeshNode_Interface::recalc_uvs_for_face( int brush_i, int face_i, int f_j)
{
    poly_face* f = &geo_scene->elements[brush_i].brush.faces[face_i];

    switch (geo_scene->elements[brush_i].brush.surface_groups[f->surface_group].type)
    {
    case SURFACE_GROUP_CYLINDER:
        recalc_uvs_for_face_cylinder(brush_i, face_i, f_j);
        break;
    case SURFACE_GROUP_SPHERE:
        recalc_uvs_for_face_sphere(brush_i, face_i, f_j);
        break;
    case SURFACE_GROUP_DOME:
        recalc_uvs_for_face_dome(brush_i, face_i, f_j);
        break;
    case SURFACE_GROUP_CUSTOM_UVS_BRUSH:
    case SURFACE_GROUP_CUSTOM_UVS_GEOMETRY:
        recalc_uvs_for_face_custom(brush_i, face_i, f_j);
        break;
    case SURFACE_GROUP_CANONICAL:
        recalc_uvs_for_face_canonical(brush_i, face_i, f_j);
        break;
    case SURFACE_GROUP_STANDARD:
    default:
        recalc_uvs_for_face_cube(brush_i, face_i, f_j);
        break;
    }
}

void MeshNode_Interface::recalc_uvs_for_face_canonical(int e_i, int f_i, int f_j)
{
    MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_j);

    if (chunk.buffer)
    {
        calculate_meshbuffer_uvs_canonical(geo_scene, e_i, f_i, chunk.buffer, chunk.begin_i, chunk.end_i);
    }
}

void MeshNode_Interface::recalc_uvs_for_face_cube(int e_i, int f_i, int f_j)
{
    MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_j);

    if(chunk.buffer)
    {
        calculate_meshbuffer_uvs_cube(geo_scene, e_i, f_i, chunk.buffer, chunk.begin_i, chunk.end_i);
    }
}

void MeshNode_Interface::recalc_uvs_for_face_cylinder(int e_i, int f_i, int f_j)
{
    MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_j);

    if(chunk.buffer)
    {
        calculate_meshbuffer_uvs_cylinder(geo_scene, e_i, f_i, chunk.buffer, chunk.begin_i, chunk.end_i);
    }
}

void MeshNode_Interface::recalc_uvs_for_face_sphere(int e_i, int f_i, int f_j)
{
    MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_j);

    if(chunk.buffer)
    {
        calculate_meshbuffer_uvs_sphere(geo_scene, e_i, f_i, chunk.buffer, chunk.begin_i, chunk.end_i);
    }
}

void MeshNode_Interface::recalc_uvs_for_face_dome(int e_i, int f_i, int f_j)
{
    MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_j);

    if(chunk.buffer)
    {
        calculate_meshbuffer_uvs_dome(geo_scene, e_i, f_i, chunk.buffer, chunk.begin_i, chunk.end_i);
    }
}


void MeshNode_Interface::recalc_uvs_for_face_custom(int e_i, int f_i, int f_j)
{
    MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_j);

    if (chunk.buffer)
    {
        calculate_meshbuffer_uvs_custom(geo_scene,e_i,f_i, chunk.buffer, chunk.begin_i, chunk.end_i);
    }
}

struct Normals_Table
{
    struct averaged_normal
    {
        core::vector3df N;
        int n_faces;
    };

    Normals_Table(polyfold* brush, int surface_group)
    {
        my_brush = brush;

        table.assign(brush->vertices.size(), averaged_normal{ core::vector3df(0,0,0),0 });
        for (int f_i = 0; f_i < brush->faces.size(); f_i++)
        {
            if (brush->faces[f_i].loops.size() > 0 && brush->faces[f_i].surface_group == surface_group)
            {
                for (int v_i = 0; v_i < brush->faces[f_i].vertices.size(); v_i++)
                {
                    int j = brush->faces[f_i].vertices[v_i];
                    table[j].N += brush->faces[f_i].m_normal;
                    table[j].n_faces += 1;
                }
            }
        }

        for (int i = 0; i < table.size(); i++)
        {
            if (table[i].n_faces > 0)
            {
                table[i].N /= table[i].n_faces;
                table[i].n_faces = 1;
                table[i].N.normalize();
            }
        }
    }

    void apply_normals_to_buffer(scene::IMeshBuffer* buffer, int begin_i, int end_i)
    {
        for (int i = begin_i; i < end_i; i++)
        {
            int idx = buffer->getIndices()[i];
            video::S3DVertex2TCoords* vtx = &((video::S3DVertex2TCoords*)buffer->getVertices())[idx];

            for (int j = 0; j < table.size(); j++)
            {
                if (is_same_point(vtx->Pos, my_brush->vertices[j].V))
                {
                    vtx->Normal = table[j].N;
                    break;
                }
            }
        }
    }

    std::vector<averaged_normal> table;
    polyfold* my_brush = NULL;
};

void MeshNode_Interface::fill_raw_vertices_buffer()
{
    typedef CMeshBuffer<video::S3DVertex2TCoords> mesh_buffer_type;
    scene::SMesh* mesh = getMesh();

    u32 n = mesh->getMeshBufferCount();

    //vertices
    {

        u32(*length)(SMesh*, u32);
        aligned_vec3(*item0)(SMesh*, u32, u32);

        length = [](SMesh* mesh_, u32 index) -> u32 {
            return ((mesh_buffer_type*)mesh_->getMeshBuffer(index))->getVertexCount();
            };

        item0 = [](SMesh* mesh_, u32 i, u32 j) -> aligned_vec3 {
            return aligned_vec3{ ((mesh_buffer_type*)mesh_->getMeshBuffer(i))->Vertices[j].Pos };
            };

        geometry_raw_vertices.fill_data(mesh, n, length, item0);
    }
}

void MeshNode_Interface::generate_uvs()
{
    for(int i=1; i<geo_scene->elements.size();i++)
        for(int f_i=0; f_i<geo_scene->elements[i].brush.faces.size(); f_i++)
            geo_scene->elements[i].brush.faces[f_i].temp_b=false;

    polyfold* pf = geo_scene->get_total_geometry();

    int original_face;
    int original_brush;

    for(int f_i = 0; f_i<pf->faces.size(); f_i++)
    {

        if(pf->faces[f_i].loops.size()>0)
        {

            face_index idx = pf->index_face(f_i);
            poly_face* f = geo_scene->get_brush_face(idx);
            polyfold* brush = geo_scene->get_brush(idx);

            original_face = idx.face; 
            original_brush = geo_scene->get_element_index(idx);

            std::vector<int> surface;

            if(f->temp_b==false)
            {
                switch(brush->surface_groups[ f->surface_group ].type)
                {
                case SURFACE_GROUP_CANONICAL:
                {
                    MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_i);

                    surface_group& sfg = brush->surface_groups[f->surface_group];

                    int n_sections = sfg.c_brush.n_quads;

                    sfg.c_brush.layout_uvs_texture(n_sections, 1.0);

                    if (chunk.buffer)
                    {
                        calculate_meshbuffer_uvs_canonical(geo_scene, original_brush, original_face, chunk.buffer, chunk.begin_i, chunk.end_i);
                    }
                    f->temp_b = true;
                } break;
                case SURFACE_GROUP_STANDARD:
                    {
                        MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_i);

                        if(chunk.buffer)
                        {
                            calculate_meshbuffer_uvs_cube(geo_scene, original_brush, original_face, chunk.buffer, chunk.begin_i, chunk.end_i);
                        }
                        f->temp_b = true;
                    } break;
                case SURFACE_GROUP_CUSTOM_UVS_BRUSH:
                case SURFACE_GROUP_CUSTOM_UVS_GEOMETRY:
                {
                    MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_i);

                    if (chunk.buffer)
                    {
                        calculate_meshbuffer_uvs_custom(geo_scene, original_brush, original_face, chunk.buffer, chunk.begin_i, chunk.end_i);
                    }
                    f->temp_b = true;
                } break;
                case SURFACE_GROUP_CYLINDER:
                    {
                        surface = pf->getSurfaceFromFace(f_i);
                        polyfold* brush1 = geo_scene->get_brush(idx);

                        original_brush = idx.brush;

                        Normals_Table calc_normals(brush1, f->surface_group);

                        for(int b_i : surface)
                        {
                            poly_face* ff = geo_scene->get_brush_face(pf->index_face(b_i));

                            original_face = pf->index_face(b_i).face;
                            
                            if(ff->temp_b == false)
                            {
                                MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(b_i);

                                if(chunk.buffer)
                                {
                                    calculate_meshbuffer_uvs_cylinder(geo_scene, original_brush, original_face, chunk.buffer, chunk.begin_i, chunk.end_i);
                                    calc_normals.apply_normals_to_buffer(chunk.buffer, chunk.begin_i, chunk.end_i);
                                }

                                ff->temp_b = true;
                            }
                        }
                    } break;
                case SURFACE_GROUP_SPHERE:
                    {
                        surface = pf->getSurfaceFromFace(f_i);
                        polyfold* brush1 = geo_scene->get_brush(idx);

                        original_brush = geo_scene->get_element_index_by_id(pf->faces[f_i].element_id);

                        Normals_Table calc_normals(brush1, f->surface_group);

                        for(int b_i : surface)
                        {
                            poly_face* ff = geo_scene->get_brush_face(pf->index_face(b_i));

                            original_face = pf->faces[b_i].face_id;

                            if(ff->temp_b == false)
                            {
                                MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(b_i);

                                if(chunk.buffer)
                                {
                                    calculate_meshbuffer_uvs_sphere(geo_scene, original_brush, original_face, chunk.buffer, chunk.begin_i, chunk.end_i);
                                    calc_normals.apply_normals_to_buffer(chunk.buffer, chunk.begin_i, chunk.end_i);
                                }

                                ff->temp_b = true;
                            }
                        }
                    } break;
                case SURFACE_GROUP_DOME:
                    {
                        surface = pf->getSurfaceFromFace(f_i);
                        polyfold* brush1 = geo_scene->get_brush(idx);

                        original_brush = geo_scene->get_element_index_by_id(pf->faces[f_i].element_id);

                        Normals_Table calc_normals(brush1, f->surface_group);

                        for(int b_i : surface)
                        {
                            poly_face* ff = geo_scene->get_brush_face(pf->index_face(b_i));

                            original_face = pf->faces[b_i].face_id;
                            
                            if(ff->temp_b == false)
                            {
                                MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(b_i);

                                if(chunk.buffer)
                                {
                                    calculate_meshbuffer_uvs_dome(geo_scene, original_brush, original_face, chunk.buffer, chunk.begin_i, chunk.end_i);
                                    calc_normals.apply_normals_to_buffer(chunk.buffer, chunk.begin_i, chunk.end_i);
                                }

                                ff->temp_b = true;
                            }
                        }

                    } break;
                }//switch
            }
        }
    }
}

int MeshNode_Interface_Edit::get_buffer_index_by_face(int i)
{
    if(i<face_to_mb_buffer.size())
    {
        return face_to_mb_buffer[i];
    }
    return -1;
}

MeshBuffer_Chunk MeshNode_Interface_Edit::get_mesh_buffer_by_face(int i)
{
    MeshBuffer_Chunk ret;

    if(i<face_to_mb_buffer.size())
    {
        int idx = face_to_mb_buffer[i];
        if(idx !=-1 )
        {
            ret.buffer = this->m_mesh->getMeshBuffer(idx);
            ret.begin_i=0;
            ret.end_i=this->m_mesh->getMeshBuffer(idx)->getIndexCount();
            return ret;
        }
    }

    ret.buffer = NULL;

    return ret;
}

MeshBuffer_Chunk MeshNode_Interface_Edit::get_mesh_buffer(int f_i)
{
    MeshBuffer_Chunk ret;
    ret.buffer = this->m_mesh->getMeshBuffer(f_i);
    ret.begin_i = 0;
    ret.end_i = this->m_mesh->getMeshBuffer(f_i)->getIndexCount();
    return ret;
}

/*
std::vector<core::vector2df>* MeshNode_Interface_Edit::get_lightmap_raw_uvs_by_face(int f_i)
{
    if (f_i < face_to_mb_buffer.size())
    {
        int idx = face_to_mb_buffer[f_i];
        if (idx != -1)
        {
            return &lightmap_raw_uvs[idx];
        }
    }

    return NULL;
}*/

MeshBuffer_Chunk MeshNode_Interface_Final::get_mesh_buffer_by_face(int i)
{
    MeshBuffer_Chunk ret;

    if(i<face_to_mb_buffer.size())
    {
        int idx = face_to_mb_buffer[i];
        if(idx !=-1 )
        {
            ret.buffer = this->m_mesh->getMeshBuffer(idx);
            ret.begin_i = face_to_mb_begin[i];
            ret.end_i = face_to_mb_end[i];
            return ret;
        }
    }

    ret.buffer = NULL;

    return ret;
}

int MeshNode_Interface_Final::get_buffer_index_by_face(int i)
{
    if (i < face_to_mb_buffer.size())
    {
        return face_to_mb_buffer[i];
    }

    return -1;
}

MeshBuffer_Chunk MeshNode_Interface_Final::get_mesh_buffer(int f_n)
{
    MeshBuffer_Chunk ret;
    ret.buffer = this->m_mesh->getMeshBuffer(mb_buffer[f_n]);
    ret.begin_i = mb_begin[f_n];
    ret.end_i = mb_end[f_n];
    return ret;
}

int MeshNode_Interface_Final::get_buffer_index(int f_n)
{
    return mb_buffer[f_n];
}

int MeshNode_Interface_Final::get_n_triangles(int f_n)
{
    return (mb_end[f_n] - mb_begin[f_n]) / 3;
}

int MeshNode_Interface_Final::get_first_triangle(int f_n)
{
    return mb_begin[f_n] / 3;
}

int MeshNode_Interface::get_material_group_by_face(int f_i)
{
    if (f_i < face_to_material.size())
    {
        return face_to_material[f_i];
    }

    return -1;
}

Lightmap_Block& MeshNode_Interface_Edit::get_lm_block(int i, int j) 
{ 
    return materials_used[i].blocks[j]; 
}

class trianglizer_base
{
public:
    GeometryStack* geo_scene = NULL;
    virtual triangle_holder get_triangles(std::vector<int>) {triangle_holder th; return th;}
    virtual triangle_holder get_triangles(int) {triangle_holder th; return th;}
};

class trianglizer_vanilla : public trianglizer_base
{
public:
    void init(GeometryStack* g)
    {
        geo_scene = g;
    }
    virtual void get_triangles(int f_i, triangle_holder* th)
    {
        LineHolder nograph;
        th = geo_scene->get_triangles_for_face(f_i);
    }
};

class trianglizer_sphere : public trianglizer_base
{
public:

    core::vector3df v0;
    core::vector3df iY;
    core::vector3df r0;

    void init(GeometryStack* gs,surface_group* sfg)
    {
        geo_scene = gs;

        v0 = sfg->vec1;
        iY = sfg->vec.crossProduct(sfg->vec1);
        iY.normalize();

        r0 = sfg->point;
    }
    virtual void get_triangles(const std::vector<int>& surface, triangle_holder& ret)
    {
        LineHolder nograph;

        int c0=0;
        int c1=0;

        for(int b_i : surface)
        {
            triangle_holder* th;
            th = geo_scene->get_triangles_for_face(b_i);

            std::vector<int> indexes;
            for(int i=0;i<th->vertices.size();i++)
            {
                //===============================================
                //Commented out section creates fungible vertices
                // 
                //
                
                //core::vector3df r = th->vertices[i]-r0;

                //core::vector2df k;
                //k.X = r.dotProduct(v0);
                //k.Y = r.dotProduct(iY);

                //f32 theta = k.getAngleTrig();

                //if(fabs(theta-360.0)<0.001  ||  fabs(theta) < 0.001)
                {
                    ret.vertices.push_back(th->vertices[i]);
                    indexes.push_back(ret.vertices.size()-1);
                    c1++;
                }
                //else
                //{
                //    indexes.push_back(ret.get_point_or_add( th->vertices[i] ));
                //    c0++;
                //}
            }

            for(const triangle &T : th->triangles)
            {
                triangle new_T;
                new_T.A = indexes[T.A];
                new_T.B = indexes[T.B];
                new_T.C = indexes[T.C];
                ret.triangles.push_back(new_T);
            }
            ret.f_index.push_back(th->triangles.size());
        }

        std::cout<<" "<<c0 <<" fungible vertices, "<<c1<<" unique vertices\n";

       // return ret;
    }
};

void MeshNode_Interface::fill_surface_index()
{
    int n = geo_scene->elements.size();

    if (n == 0) return;

    surface_index.len.resize(n);
    surface_index.offset.resize(n);

    int offset = 0;
    for (int i = 1; i < geo_scene->elements.size(); i++)
    {
        surface_index.offset[i] = offset;
        surface_index.len[i] = geo_scene->elements[i].surfaces.size();
        offset += geo_scene->elements[i].surfaces.size();
    }

    surface_index.data.resize(offset);
}

void MeshNode_Interface_Final::generate_mesh_buffer(SMesh* mesh)
{
    scene::IMeshBuffer* buffer;
    LineHolder nograph;

    polyfold* pf = geo_scene->get_total_geometry();
    const std::vector<TextureMaterial>& materials_used = get_materials_used();

	//std::cout<<"geometry uses "<<materials_used.size()<<" material groups\n";

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        pf->faces[f_i].temp_b = false;
    }

	this->face_to_mb_buffer.assign(pf->faces.size(),-1);
	this->face_to_mb_begin.assign(pf->faces.size(),0);
	this->face_to_mb_end.assign(pf->faces.size(),0);
	this->face_to_mb_faces.assign(pf->faces.size(),-1);

    int n_geometry_faces = 0;
    for (int t_i = 0; t_i < get_materials_used().size(); t_i++)
    {
        n_geometry_faces += get_materials_used()[t_i].faces.size();
    }

	this->mb_begin.assign(n_geometry_faces,0);
	this->mb_end.assign(n_geometry_faces,0);
	this->mb_buffer.assign(n_geometry_faces,0);
	this->edit_mb_buffer.assign(n_geometry_faces,0);

    int n_geometry_surfaces = 0;

    for (const TextureMaterial& tm : materials_used)
    {
        n_geometry_surfaces += tm.surfaces.size();
    }

    this->surface_to_mb_begin.assign(n_geometry_surfaces, 0);
    this->surface_to_mb_end.assign(n_geometry_surfaces, 0);
    this->surface_to_mb_buffer.assign(n_geometry_surfaces, 0);

    fill_surface_index();

	//std::cout<< pf->faces.size()<<" faces\n\n";

	trianglizer_sphere tri_sphere;

	int tot_v=0;
	int tot_i=0;
    int face_c = 0;
    int surface_c = 0;

    
    int total_total_indices = 0;

	//========loop material groups
	for(int t_i=0; t_i< materials_used.size(); t_i++)
    {
        
        int total_indices=0;

        std::vector<triangle_holder> triangle_groups;

        //for(int f_i : materials_used[t_i].faces)
        for(std::pair<int,int> surf: materials_used[t_i].surfaces)
        {
            int e_i = geo_scene->get_element_index_by_id(surf.first);
            int s_i = surf.second;

            surface_to_mb_buffer[surface_c] = t_i;
            surface_to_mb_begin[surface_c] = total_indices;
            surface_index.data[surface_index.offset[e_i] + s_i].buffer = t_i;
            surface_index.data[surface_index.offset[e_i] + s_i].begin_i = total_total_indices + total_indices;

            for (int f_i0 : geo_scene->elements[e_i].surfaces[s_i].my_faces)
            {
                int f_i = geo_scene->elements[e_i].reverse_index[f_i0];

                if (pf->faces[f_i].loops.size() > 0 && pf->faces[f_i].temp_b == false)
                    //for(int f_i : geo_scene->elements[e_i].surfaces[s_i].my_faces)
                {
                    video::ITexture* tex_j = driver->getTexture(geo_scene->elements[e_i].surfaces[s_i].texture_name);

                    if (tex_j == materials_used[t_i].texture && geo_scene->elements[e_i].surfaces[s_i].material_group == materials_used[t_i].materialGroup)
                    {
                        switch (pf->getFaceSurfaceGroup(f_i)->type)
                        {
                        case SURFACE_GROUP_STANDARD:
                        case SURFACE_GROUP_CUSTOM_UVS_BRUSH:
                        case SURFACE_GROUP_CUSTOM_UVS_GEOMETRY:
                        case SURFACE_GROUP_CANONICAL:
                        {
                            triangle_holder* th = geo_scene->get_triangles_for_face(f_i);
                            triangle_groups.push_back(*th);

                            face_to_mb_buffer[f_i] = t_i;
                            face_to_mb_begin[f_i] = total_indices;
                            face_to_mb_end[f_i] = total_indices + th->triangles.size() * 3;

                            mb_begin[face_c] = face_to_mb_begin[f_i];
                            mb_end[face_c] = face_to_mb_end[f_i];
                            mb_buffer[face_c] = face_to_mb_buffer[f_i];
                            face_to_mb_faces[f_i] = face_c;
                            face_c++;

                            total_indices += th->triangles.size() * 3;

                            pf->faces[f_i].temp_b = true;
                        } break;
                        case SURFACE_GROUP_DOME:
                        case SURFACE_GROUP_SPHERE:
                        {
                            std::vector<int> sfg = pf->getSurfaceFromFace(f_i);

                            tri_sphere.init(geo_scene, pf->getFaceSurfaceGroup(f_i));
                            triangle_holder th;
                            tri_sphere.get_triangles(sfg, th);

                            for (int i = 0; i < sfg.size(); i++)
                            {
                                int b_i = sfg[i];

                                pf->faces[b_i].temp_b = true;
                                this->face_to_mb_buffer[b_i] = t_i;
                                this->face_to_mb_begin[b_i] = total_indices;
                                this->face_to_mb_end[b_i] = total_indices + th.f_index[i] * 3;

                                total_indices += th.f_index[i] * 3;

                                mb_begin[face_c] = face_to_mb_begin[b_i];
                                mb_end[face_c] = face_to_mb_end[b_i];
                                mb_buffer[face_c] = face_to_mb_buffer[b_i];
                                face_to_mb_faces[b_i] = face_c;

                                face_c++;
                            }

                            
                            // std::cout<<sfg.size()<<" faces in sphere\n";

                            triangle_groups.push_back(th);
                        } break;
                        case SURFACE_GROUP_CYLINDER:
                        {
                            std::vector<int> sfg = pf->getSurfaceFromFace(f_i);

                            tri_sphere.init(geo_scene, pf->getFaceSurfaceGroup(f_i));
                            triangle_holder th;
                            tri_sphere.get_triangles(sfg, th);

                            for (int i = 0; i < sfg.size(); i++)
                            {
                                int b_i = sfg[i];

                                pf->faces[b_i].temp_b = true;
                                this->face_to_mb_buffer[b_i] = t_i;
                                this->face_to_mb_begin[b_i] = total_indices;
                                this->face_to_mb_end[b_i] = total_indices + th.f_index[i] * 3;

                                total_indices += th.f_index[i] * 3;

                                mb_begin[face_c] = face_to_mb_begin[b_i];
                                mb_end[face_c] = face_to_mb_end[b_i];
                                mb_buffer[face_c] = face_to_mb_buffer[b_i];
                                face_to_mb_faces[b_i] = face_c;

                                face_c++;
                            }

                            //std::cout<<sfg.size()<<" faces in cylinder\n";

                            triangle_groups.push_back(th);
                        } break;
                        default:
                            std::cout << "uknown surface properties\n";
                            break;
                        }
                    }
                }   
            }
            surface_index.data[surface_index.offset[e_i] + s_i].end_i = total_total_indices + total_indices;

            surface_to_mb_end[surface_c] = total_indices;

            surface_c++;
        }

        total_total_indices += total_indices;

        buffer = new scene::CMeshBuffer<video::S3DVertex2TCoords>();
        buffer->getMaterial().setTexture(0,materials_used[t_i].texture);

        make_meshbuffer_from_triangles(triangle_groups,buffer);

        //std::cout<<"buffer "<<t_i<<" has "<<buffer->getVertexCount()<<" vertices and "<<buffer->getIndexCount()<<" indices\n";
        tot_i+=buffer->getIndexCount();
        tot_v+=buffer->getVertexCount();
       
        //TODO
        //materials_used[t_i].n_triangles = buffer->getVertexCount() / 3;

        mesh->addMeshBuffer(buffer);
    }   //material groups (unique textures used)

    fill_index_struct_with_offsets(mesh, indices_soa);

    std::cout<<materials_used.size()<<" buffers with "<<tot_v<<" vertices and "<<tot_i<<" indices\n";
    geo_scene->setFinalMeshDirty(false);
}

void MeshNode_Interface_Final::copy_lightmap_uvs()
{
    polyfold* pf = geo_scene->get_total_geometry();

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        if (pf->faces[f_i].loops.size() == 0)
            continue;

        MeshBuffer_Chunk chunk0 = geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_i);
        MeshBuffer_Chunk chunk1 = get_mesh_buffer_by_face(f_i);

        int i, j;

        for (i = chunk0.begin_i, j = chunk1.begin_i; i < chunk0.end_i && j < chunk1.end_i; i++, j++)
        {
            u16 idx0 = chunk0.buffer->getIndices()[i];
            u16 idx1 = chunk1.buffer->getIndices()[j];

            video::S3DVertex2TCoords* vtx0 = &((video::S3DVertex2TCoords*)chunk0.buffer->getVertices())[idx0];
            video::S3DVertex2TCoords* vtx1 = &((video::S3DVertex2TCoords*)chunk1.buffer->getVertices())[idx1];

            vtx1->TCoords2 = vtx0->TCoords2;

            //cout << f_i << ":  " << vtx1->TCoords2.X << "," << vtx1->TCoords2.Y << "\n";
        }
    }
}


REFLECT_STRUCT_BEGIN(Lightmap_Block)
    REFLECT_STRUCT_MEMBER(width)
        REFLECT_STRUCT_MEMBER_FLAG( FLAG_UINT_WIDGET_POWER2 )
    REFLECT_STRUCT_MEMBER(height)
        REFLECT_STRUCT_MEMBER_FLAG( FLAG_UINT_WIDGET_POWER2 )
REFLECT_STRUCT_END()
