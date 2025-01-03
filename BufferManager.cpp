
#include <irrlicht.h>
#include "CMeshSceneNode.h"
#include "BufferManager.h"
#include "csg_classes.h"
#include "geometry_scene.h"
#include "material_groups.h"
#include "tolerances.h"
#include "LightMaps.h"
#include "build_meshes.h"

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

MeshNode_Interface::MeshNode_Interface()
{
}

MeshNode_Interface::~MeshNode_Interface()
{
    if(this->m_mesh)
        this->m_mesh->drop();
}

void MeshNode_Interface_Final::refresh_material_groups(GeometryStack* geo_scene)
{
    materials_used.clear();

    polyfold* pf = geo_scene->get_total_geometry();

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        int brush_j = geo_scene->get_total_geometry()->faces[f_i].original_brush;
        int face_j = geo_scene->get_total_geometry()->faces[f_i].original_face;

        pf->faces[f_i].material_group = geo_scene->elements[brush_j].brush.faces[face_j].material_group;
        pf->faces[f_i].texture_name = geo_scene->elements[brush_j].brush.faces[face_j].texture_name;
    }

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        if (pf->faces[f_i].loops.size() > 0)
        {
            video::ITexture* tex_j = driver->getTexture(pf->faces[f_i].texture_name.c_str());

            bool b = false;

            for (TextureMaterial& tm : materials_used)
            {
                if (tm.texture == tex_j && tm.materialGroup == pf->faces[f_i].material_group)
                {
                    b = true;
                    tm.n_faces++;
                    tm.faces.push_back(f_i);
                }
            }
            if (!b)
            {
                materials_used.push_back(TextureMaterial{ tex_j,pf->faces[f_i].material_group,1 });

                materials_used[materials_used.size() - 1].faces.push_back(f_i);
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

    lightmaps_divideMaterialGroups(geo_scene, materials_used);

    for (int i = 0; i < materials_used.size(); i++)
    {
        materials_used[i].records.resize(materials_used[i].faces.size());

        for (int j = 0; j < materials_used[i].records.size(); j++)
        {
            int f_i = materials_used[i].faces[j];

            geo_scene->get_total_geometry()->faces[f_i].get3DBoundingQuad(&materials_used[i].records[j].bounding_verts[0],0);
            
        }
    }
    
    /*
    for (int i = 0; i < materials_used.size(); i++)
    {
        std::cout << i << ": " << materials_used[i].texture->getName().getPath().c_str() << " ";
        std::cout << materials_used[i].records.size() << " records, LM \n";
        for (int j = 0; j < materials_used[i].records.size(); j++)
        {
            std::cout <<" "<< materials_used[i].records[j].face << ": ";
            std::cout <<" "<< materials_used[i].records[j].block.UpperLeftCorner.X << "/" << materials_used[i].records[j].block.LowerRightCorner.X << "\n";
        }
       // std::cout << materials_used[i].lightmap_size << "x" << materials_used[i].lightmap_size << "\n";
    }*/

}

void MeshNode_Interface_Edit::generate_mesh_node(GeometryStack* geo_scene)
{
    if(m_mesh)
        m_mesh->drop();

    this->m_mesh = new scene::SMesh;

    this->generate_mesh_buffer(geo_scene,m_mesh);

    this->generate_uvs(geo_scene);
}

void MeshNode_Interface_Final::generate_mesh_node(GeometryStack* geo_scene)
{
    if (m_mesh)
        m_mesh->drop();

    this->m_mesh = new scene::SMesh;

    refresh_material_groups(geo_scene);

    this->generate_mesh_buffer(geo_scene, m_mesh);

    for (TextureMaterial& tm : materials_used)
    {
        tm.n_faces = tm.faces.size();

        int n_triangles = 0;

        if(tm.faces.size() > 0)
        {
            MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(tm.faces[0]);

            if (chunk.buffer)
                n_triangles = chunk.buffer->getIndexCount() / 3;
        }

        tm.n_triangles = n_triangles;
    }

    this->generate_uvs(geo_scene);

    copy_lightmap_uvs(geo_scene);
}

CMeshSceneNode* MeshNode_Interface::addMeshSceneNode(ISceneNode* parent, scene::ISceneManager* smgr0,GeometryStack* geo_scene)
{
    if(this->m_mesh && smgr0)
    {
        CMeshSceneNode* mesh_node = new scene::CMeshSceneNode(this->m_mesh,parent,smgr0,747);
        mesh_node->SetBox(geo_scene->get_total_geometry()->bbox);
        mesh_node->setVisible(true);
        return mesh_node;
    }

    return NULL;
}

void MeshNode_Interface_Edit::generate_mesh_buffer(GeometryStack* geo_scene, SMesh* mesh)
{
    scene::IMeshBuffer* buffer;

    int b_count=0;
    int i_count=0;
    int v_count=0;
    LineHolder nograph;

    face_to_mb_buffer.clear();

    polyfold* pf = geo_scene->get_total_geometry();

    for(int f_i =0; f_i<pf->faces.size(); f_i++)
    {
        if(pf->faces[f_i].loops.size()==0)
        {
            this->face_to_mb_buffer.push_back(-1);
            continue;
        }

        this->face_to_mb_buffer.push_back(b_count);

        const triangle_holder& th = *geo_scene->get_triangles_for_face(f_i);

        buffer = new scene::CMeshBuffer<video::S3DVertex2TCoords>();

        buffer->getMaterial().setTexture(0, driver->getTexture(pf->faces[f_i].texture_name.c_str()));
        //buffer->Material.setTexture(0,driver->getTexture(pf->faces[f_i].texture_name.c_str()));
        //geo_scene->getMaterialGroupsBase()->apply_material_to_buffer(buffer,pf->faces[f_i].material_group,geo_scene->DynamicLightEnabled());

        make_meshbuffer_from_triangles(th,buffer);
        mesh->addMeshBuffer(buffer);

        b_count++;

        i_count += buffer->getIndexCount();
        v_count += buffer->getVertexCount();

        //pf.faces[f_i].temp_b = false;
    }
     //std::cout<<""<<b_count<<" buffers with "<<v_count<<" vertices and "<<i_count/3<<" triangles... done\n";
}

void MeshNode_Interface::recalc_uvs_for_face(GeometryStack* geo_scene, int brush_i, int face_i, int f_j)
{
    poly_face* f = &geo_scene->elements[brush_i].brush.faces[face_i];

    switch (geo_scene->elements[brush_i].brush.surface_groups[f->surface_group].type)
    {
    case SURFACE_GROUP_CYLINDER:
        recalc_uvs_for_face_cylinder(geo_scene, brush_i, face_i, f_j);
        break;
    case SURFACE_GROUP_SPHERE:
        recalc_uvs_for_face_sphere(geo_scene, brush_i, face_i, f_j);
        break;
    case SURFACE_GROUP_DOME:
        recalc_uvs_for_face_dome(geo_scene, brush_i, face_i, f_j);
        break;
    case SURFACE_GROUP_CUSTOM_UVS_BRUSH:
    case SURFACE_GROUP_CUSTOM_UVS_GEOMETRY:
        recalc_uvs_for_face_custom(geo_scene, brush_i, face_i, f_j);
        break;
    case SURFACE_GROUP_STANDARD:
    default:
        recalc_uvs_for_face_cube(geo_scene, brush_i, face_i, f_j);
        break;
    }
}

void MeshNode_Interface::recalc_uvs_for_face_cube(GeometryStack* geo_scene, int e_i, int f_i, int f_j)
{
    MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_j);

    if(chunk.buffer)
    {
        calculate_meshbuffer_uvs_cube(geo_scene, e_i, f_i, chunk.buffer, chunk.begin_i, chunk.end_i);
    }
}

void MeshNode_Interface::recalc_uvs_for_face_cylinder(GeometryStack* geo_scene, int e_i, int f_i, int f_j)
{
    MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_j);

    if(chunk.buffer)
    {
        calculate_meshbuffer_uvs_cylinder(geo_scene, e_i, f_i, chunk.buffer, chunk.begin_i, chunk.end_i);
    }
}

void MeshNode_Interface::recalc_uvs_for_face_sphere(GeometryStack* geo_scene, int e_i, int f_i, int f_j)
{
    MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_j);

    if(chunk.buffer)
    {
        calculate_meshbuffer_uvs_sphere(geo_scene, e_i, f_i, chunk.buffer, chunk.begin_i, chunk.end_i);
    }
}

void MeshNode_Interface::recalc_uvs_for_face_dome(GeometryStack* geo_scene, int e_i, int f_i, int f_j)
{
    MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_j);

    if(chunk.buffer)
    {
        calculate_meshbuffer_uvs_dome(geo_scene, e_i, f_i, chunk.buffer, chunk.begin_i, chunk.end_i);
    }
}


void MeshNode_Interface::recalc_uvs_for_face_custom(GeometryStack* geo_scene, int e_i, int f_i, int f_j)
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

void MeshNode_Interface::generate_uvs(GeometryStack* geo_scene)
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
            poly_face* f = geo_scene->get_original_brush_face(f_i);
            polyfold* brush = geo_scene->get_original_brush(f_i);

            original_face = pf->faces[f_i].original_face;
            original_brush = pf->faces[f_i].original_brush;

            std::vector<int> surface;

            if(f->temp_b==false)
            {
                switch(brush->surface_groups[ f->surface_group ].type)
                {
                case SURFACE_GROUP_STANDARD:
                    {
                        MeshBuffer_Chunk chunk = get_mesh_buffer_by_face(f_i);

                        original_face = pf->faces[f_i].original_face;
                        original_brush = pf->faces[f_i].original_brush;

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

                    original_face = pf->faces[f_i].original_face;
                    original_brush = pf->faces[f_i].original_brush;

                    if (chunk.buffer)
                    {
                        calculate_meshbuffer_uvs_custom(geo_scene, original_brush, original_face, chunk.buffer, chunk.begin_i, chunk.end_i);
                    }
                    f->temp_b = true;
                } break;
                case SURFACE_GROUP_CYLINDER:
                    {
                        surface = geo_scene->getSurfaceFromFace(f_i);
                        polyfold* brush1 = geo_scene->get_original_brush(f_i);

                        original_brush = pf->faces[f_i].original_brush;

                        Normals_Table calc_normals(brush1, f->surface_group);

                        for(int b_i : surface)
                        {
                            poly_face* ff = geo_scene->get_original_brush_face(b_i);

                            original_face = pf->faces[b_i].original_face;
                            
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
                        surface = geo_scene->getSurfaceFromFace(f_i);
                        polyfold* brush1 = geo_scene->get_original_brush(f_i);

                        original_brush = pf->faces[f_i].original_brush;

                        Normals_Table calc_normals(brush1, f->surface_group);

                        for(int b_i : surface)
                        {
                            poly_face* ff = geo_scene->get_original_brush_face(b_i);

                            original_face = pf->faces[b_i].original_face;

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
                        surface = geo_scene->getSurfaceFromFace(f_i);
                        polyfold* brush1 = geo_scene->get_original_brush(f_i);

                        original_brush = pf->faces[f_i].original_brush;

                        Normals_Table calc_normals(brush1, f->surface_group);

                        for(int b_i : surface)
                        {
                            poly_face* ff = geo_scene->get_original_brush_face(b_i);

                            original_face = pf->faces[b_i].original_face;
                            
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

void MeshNode_Interface_Final::generate_mesh_buffer(GeometryStack* geo_scene, SMesh* mesh)
{
    scene::IMeshBuffer* buffer;
    LineHolder nograph;

    polyfold* pf = geo_scene->get_total_geometry();

	//std::cout<<"geometry uses "<<materials_used.size()<<" material groups\n";

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        pf->faces[f_i].temp_b = false;
    }

	this->face_to_mb_buffer.assign(pf->faces.size(),-1);
	this->face_to_mb_begin.assign(pf->faces.size(),0);
	this->face_to_mb_end.assign(pf->faces.size(),0);

	//std::cout<< pf->faces.size()<<" faces\n\n";

	trianglizer_sphere tri_sphere;

	int tot_v=0;
	int tot_i=0;

	//========loop material groups
	for(int t_i=0; t_i<materials_used.size(); t_i++)
    {
        int total_indices=0;

        std::vector<triangle_holder> triangle_groups;

        for(int f_i : materials_used[t_i].faces)
        {
            if(pf->faces[f_i].loops.size() > 0 && pf->faces[f_i].temp_b==false)
            {
                video::ITexture* tex_j = driver->getTexture(pf->faces[f_i].texture_name.c_str());

                if(tex_j == materials_used[t_i].texture && pf->faces[f_i].material_group == materials_used[t_i].materialGroup)
                {
                    switch(pf->getFaceSurfaceGroup(f_i)->type)
                    {
                    case SURFACE_GROUP_STANDARD:
                    case SURFACE_GROUP_CUSTOM_UVS_BRUSH:
                    case SURFACE_GROUP_CUSTOM_UVS_GEOMETRY:
                        {
                            triangle_holder* th = geo_scene->get_triangles_for_face(f_i);
                            triangle_groups.push_back(*th);

                            this->face_to_mb_buffer[f_i]=t_i;
                            this->face_to_mb_begin[f_i]=total_indices;
                            this->face_to_mb_end[f_i]=total_indices+th->triangles.size()*3;
                            total_indices+=th->triangles.size()*3;

                            pf->faces[f_i].temp_b=true;
                        } break;
                    case SURFACE_GROUP_DOME:
                    case SURFACE_GROUP_SPHERE:
                        {
                            std::vector<int> sfg = geo_scene->getSurfaceFromFace(f_i);

                            tri_sphere.init(geo_scene,pf->getFaceSurfaceGroup(f_i));
                            triangle_holder th;
                            tri_sphere.get_triangles(sfg,th);

                            for(int i=0;i<sfg.size();i++)
                            {
                                int b_i = sfg[i];

                                pf->faces[b_i].temp_b=true;
                                this->face_to_mb_buffer[b_i]=t_i;
                                this->face_to_mb_begin[b_i]=total_indices;
                                this->face_to_mb_end[b_i]=total_indices+th.f_index[i]*3;
                                total_indices+=th.f_index[i]*3;
                            }
                           // std::cout<<sfg.size()<<" faces in sphere\n";

                            triangle_groups.push_back(th);
                        } break;
                    case SURFACE_GROUP_CYLINDER:
                        {
                            std::vector<int> sfg = geo_scene->getSurfaceFromFace(f_i);

                            tri_sphere.init(geo_scene,pf->getFaceSurfaceGroup(f_i));
                            triangle_holder th;
                            tri_sphere.get_triangles(sfg, th);

                            for(int i=0;i<sfg.size();i++)
                            {
                                int b_i = sfg[i];

                                pf->faces[b_i].temp_b=true;
                                this->face_to_mb_buffer[b_i]=t_i;
                                this->face_to_mb_begin[b_i]=total_indices;
                                this->face_to_mb_end[b_i]=total_indices+th.f_index[i]*3;

                                total_indices+=th.f_index[i]*3;
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

       // buffer = new scene::SMeshBuffer();
        buffer = new scene::CMeshBuffer<video::S3DVertex2TCoords>();
        buffer->getMaterial().setTexture(0,materials_used[t_i].texture);
        //geo_scene->getMaterialGroupsBase()->apply_material_to_buffer(buffer,materials_used[t_i].materialGroup,geo_scene->DynamicLightEnabled());

        make_meshbuffer_from_triangles(triangle_groups,buffer);

        //std::cout<<"buffer "<<t_i<<" has "<<buffer->getVertexCount()<<" vertices and "<<buffer->getIndexCount()<<" indices\n";
        tot_i+=buffer->getIndexCount();
        tot_v+=buffer->getVertexCount();
        materials_used[t_i].n_triangles = buffer->getVertexCount() / 3;

        mesh->addMeshBuffer(buffer);
    }   //material groups (unique textures used)
    std::cout<<materials_used.size()<<" buffers with "<<tot_v<<" vertices and "<<tot_i<<" indices\n";
    geo_scene->setFinalMeshDirty(false);
}

void MeshNode_Interface_Final::copy_lightmap_uvs(GeometryStack* geo_scene)
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
        }
    }
}
