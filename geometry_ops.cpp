#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "geometry_scene.h"
#include "edit_env.h"
#include "tolerances.h"
#include "texture_picker.h"
#include "BufferManager.h"
#include "material_groups.h"
#include "BVH.h"
#include "CMeshSceneNode.h"
#include "reflected_nodes.h"
#include "CameraPanel.h"
#include "LightMaps.h"

#include <chrono>

#include "clip_functions.h"

extern IrrlichtDevice* device;
extern TestPanel* Active_Camera_Window;

extern irr::video::ITexture* small_circle_tex_add_selected;
extern irr::video::ITexture* small_circle_tex_add_not_selected;
extern irr::video::ITexture* small_circle_tex_sub_selected;
extern irr::video::ITexture* small_circle_tex_sub_not_selected;
extern irr::video::ITexture* small_circle_tex_red_selected;
extern irr::video::ITexture* small_circle_tex_red_not_selected;

extern irr::video::ITexture* med_circle_tex_add_selected;
extern irr::video::ITexture* med_circle_tex_add_not_selected;
extern irr::video::ITexture* med_circle_tex_sub_selected;
extern irr::video::ITexture* med_circle_tex_sub_not_selected;
extern irr::video::ITexture* med_circle_tex_red_selected;
extern irr::video::ITexture* med_circle_tex_red_not_selected;

float rebuild_geometry_time = 0.0;
float build_total_geometry_time = 0.0;
float trianglize_time = 0.0;
float generate_meshes_time = 0.0;

void reset_geometry_ops_timers()
{
    rebuild_geometry_time = 0.0;
    trianglize_time = 0.0;
    generate_meshes_time = 0.0;
    build_total_geometry_time = 0.0;
}

#define TIME_HEADER() auto startTime = std::chrono::high_resolution_clock::now();\
    auto timeZero = startTime;\
    auto currentTime = std::chrono::high_resolution_clock::now();\
    float time;
#define START_TIMER() startTime = std::chrono::high_resolution_clock::now(); \

#define PRINT_TIMER(text) currentTime = std::chrono::high_resolution_clock::now(); \
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); \
    std::cout << "---------time (" <<#text<< "): " << time << "\n";\

#define INC_TIMER(thingy) currentTime = std::chrono::high_resolution_clock::now(); \
    thingy += std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); \

#define PRINT_THINGY(thingy)std::cout<<#thingy<<": "<<thingy<<"\n";


#define PRINT_TOTAL_TIME(text) currentTime = std::chrono::high_resolution_clock::now(); \
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - timeZero).count(); \
    std::cout << "---------total time(" <<#text<< "): " << time << "\n";


void print_geometry_ops_timers()
{
    PRINT_THINGY(rebuild_geometry_time)
    PRINT_THINGY(build_total_geometry_time)
   // PRINT_THINGY(trianglize_time)
    PRINT_THINGY(generate_meshes_time)
}

#define printvec(v) v.X<<","<<v.Y<<","<<v.Z

GeometryStack::GeometryStack()
    : USceneNode(NULL, NULL, -1, vector3df(0, 0, 0))
{
}


void GeometryStack::initialize(ISceneNode* parent, scene::ISceneManager* smgr, MyEventReceiver* receiver)
{
    SceneManager = smgr;

    if (parent)
    {
        parent->addChild(this);
        this->drop();
    }

    event_receiver = receiver;

    this->edit_meshnode_interface.init(smgr, device->getVideoDriver(), event_receiver);
    this->final_meshnode_interface.init(smgr, device->getVideoDriver(), event_receiver);
    this->setAutomaticCulling(scene::EAC_OFF);
}

void GeometryStack::initialize(geometry_scene* geo_scene)
{
    initialize(geo_scene->smgr->getRootSceneNode(), geo_scene->smgr, geo_scene->event_receiver);
}

void GeometryStack::initialize(scene::ISceneManager* smgr, MyEventReceiver* receiver)
{
    SceneManager = smgr;
    event_receiver = receiver;
}

GeometryStack::~GeometryStack()
{
   // std::cout << "GeometryStack, going out of scope!\n";
}

void GeometryStack::OnRegisterSceneNode()
{
    if (IsVisible)
    {
        SceneManager->registerNodeForRendering(this);

        core::list<ISceneNode*>::ConstIterator it = Children.begin();
        for (; it != Children.end(); ++it)
            (*it)->OnRegisterSceneNode();
    }
}

void GeometryStack::render()
{
    video::IVideoDriver* driver = device->getVideoDriver();

    video::SMaterial someMaterial;
    someMaterial.Lighting = false;
    someMaterial.Thickness = 1.0;
    someMaterial.MaterialType = video::EMT_SOLID;

    driver->setMaterial(someMaterial);

    ICameraSceneNode* camera = SceneManager->getActiveCamera();

    if (!camera || !Active_Camera_Window)
        return;

    core::matrix4 trans;
    trans.setbyproduct_nocheck(camera->getProjectionMatrix(), camera->getViewMatrix());

    core::dimension2d<u32> dim = Active_Camera_Window->getImage()->getOriginalSize();

    dim.Width /= 2;
    dim.Height /= 2;

    if (render_active_brush)
    {
        this->elements[0].draw_brush(driver);
    }

    if (render_brushes)
    {
        for (int e_i = 0; e_i < this->elements.size(); e_i++)
        {
            this->elements[e_i].draw_brush(driver);
        }
    }

    if (render_active_brush)
    {
        geo_element* geo = &this->elements[0];
        if (geo->bSelected)
        {
            bool bDrawSelectedVertex = 0 == selected_brush_vertex_editing;
            this->elements[0].draw_brush_vertices(trans, dim, bDrawSelectedVertex, driver);
        }
    }

    if (render_brushes)
    {
        //changed e_i=1 to 0, even though this is normally the red brush,
        //because in the UV editor, there is no red brush
        for (int e_i = 0; e_i < this->elements.size(); e_i++)
        {
            geo_element* geo = &this->elements[e_i];

            if (geo->bSelected)
            {
                bool bDrawSelectedVertex = e_i == selected_brush_vertex_editing;
                this->elements[e_i].draw_brush_vertices(trans, dim, bDrawSelectedVertex, driver);
            }
        }
    }
}

void GeometryStack::set_type(int t)
{
    this->base_type = t;
}

polyfold* GeometryStack::get_total_geometry()
{
    return &this->total_geometry;
}

void GeometryStack::build_total_geometry()
{
    polyfold combo, combo2;
    std::vector<polyfold*> polies;

    if (elements.size() == 1)
    {
        total_geometry = polyfold();
        return;
    }

    for (int i = 1; i < this->elements.size(); i++)
    {
        polies.push_back(&this->elements[i].geometry);
    }

    combine_polyfolds(polies, combo);

    this->total_geometry = combo;
    this->total_geometry.geometry_stack = this;

    if (this->base_type == GEO_EMPTY)
        this->total_geometry.topology = TOP_CONVEX;
    else if (this->base_type == GEO_SOLID)
        this->total_geometry.topology = TOP_CONCAVE;
}
/*
void GeometryStack::set_originals()
{
    for (int i = 1; i < this->elements.size(); i++)
    {
        for (int j = 0; j < this->elements[i].brush.faces.size() && j < this->elements[i].geometry.faces.size(); j++)
        {
            this->elements[i].geometry.faces[j].texture_name = this->elements[i].brush.faces[j].texture_name;
        }
    }
}*/

bool geo_element::has_geometry()
{
    return this->geometry.edges.size() > 2;
}

void GeometryStack::add()
{
    //this->WriteGeometryToFile("backup.pol");

    geo_element geo = elements[0];
    const polyfold& pf = geo.brush;
    geo.type = GEO_ADD;

    for (poly_face& face : geo.brush.faces)
    {
        //face.texture_name = path;
        //face.material_group = new_geometry_material_group;
        face.element_id = element_id_incrementer;
    }

    video::ITexture* tex = TexturePicker_Tool::getCurrentTexture();
    io::path path = tex->getName();

    for (poly_surface& s : geo.surfaces)
    {
        s.texture_name = path;
        s.material_group = new_geometry_material_group;
    }
    geo.reverse_index.resize(pf.faces.size());

    geo.element_id = element_id_incrementer;
    element_id_incrementer++;

    this->elements.push_back(geo);

    if (this->progressive_build)
        this->rebuild_geometry(true);
}

void GeometryStack::add_plane()
{
    //this->WriteGeometryToFile("backup.pol");

    geo_element geo = elements[0];
    const polyfold& pf = geo.brush;
    geo.type = GEO_SEMISOLID;

    for (poly_face& face : geo.brush.faces)
    {
        //face.texture_name = path;
        //face.material_group = new_geometry_material_group;
        face.element_id = element_id_incrementer;
    }

    video::ITexture* tex = TexturePicker_Tool::getCurrentTexture();
    io::path path = tex->getName();

    for (poly_surface& s : geo.surfaces)
    {
        s.texture_name = path;
        s.material_group = new_geometry_material_group;
    }
    geo.reverse_index.resize(pf.faces.size());

    //for (poly_face& face : geo.brush.faces)
    //    face.material_group = 1; ?????

    geo.element_id = element_id_incrementer;
    element_id_incrementer++;

    this->elements.push_back(geo);

    if (this->progressive_build)
        this->rebuild_geometry(true);
}

void GeometryStack::add_semisolid()
{
    //this->WriteGeometryToFile("backup.pol");

    geo_element geo = elements[0];
    const polyfold& pf = geo.brush;
    geo.type = GEO_SEMISOLID;

    //geo.brush.generate_uids();

    for (poly_face& face : geo.brush.faces)
    {
        //face.material_group = new_geometry_material_group;
        face.element_id = element_id_incrementer;
    }


    video::ITexture* tex = TexturePicker_Tool::getCurrentTexture();
    io::path path = tex->getName();

    for (poly_surface& s : geo.surfaces)
    {
        s.texture_name = path;
        s.material_group = new_geometry_material_group;
    }
    geo.reverse_index.resize(pf.faces.size());

    geo.element_id = element_id_incrementer;
    element_id_incrementer++;

    this->elements.push_back(geo);

    if (this->progressive_build)
        this->rebuild_geometry(true);
}

void GeometryStack::subtract()
{
    //this->WriteGeometryToFile("backup.pol");

    geo_element geo = elements[0];
    const polyfold& pf = geo.brush;
    geo.type = GEO_SUBTRACT;

    for (poly_face& face : geo.brush.faces)
    {
        //face.material_group = new_geometry_material_group;
        face.element_id = element_id_incrementer;
    }

    video::ITexture* tex = TexturePicker_Tool::getCurrentTexture();
    io::path path = tex->getName();

    //geo.texture_names.assign(pf.faces.size(), path);
    for (poly_surface& s : geo.surfaces)
    {
        s.texture_name = path;
        s.material_group = new_geometry_material_group;
    }
    geo.reverse_index.resize(pf.faces.size());

    geo.element_id = element_id_incrementer;
    element_id_incrementer++;

    this->elements.push_back(geo);

    if (this->progressive_build)
        this->rebuild_geometry(true);
}

void GeometryStack::build_intersecting_target(const polyfold& pf, polyfold& out)
{
    // A function useful for testing
    //

    std::vector<polyfold*> polies;

    int num = 0;
    for (int j = 1; j < elements.size(); j++)
    {

        if (this->elements[j].has_geometry() &&
            (this->elements[j].type == GEO_SUBTRACT || this->elements[j].type == GEO_ADD) &&
            BoxIntersectsWithBox(pf.bbox, this->elements[j].brush.bbox))
        {
            polies.push_back(&elements[j].geometry);

            num++;
        }
    }

    if (num > 0)
    {
        combine_polyfolds(polies, out);
    }
}

void GeometryStack::rebuild_geometry(bool only_build_new_geometry)
{

    TIME_HEADER()

    LineHolder nograph;

    polyfold no_poly;

    int build_start;

    if (only_build_new_geometry)
        build_start = this->build_progress + 1;
    else
        build_start = 1;

    for (int i = build_start; i < this->elements.size(); i++)
        this->elements[i].geometry = no_poly;

    //intersections_graph.lines.clear();

    START_TIMER()

    for (int i = build_start; i < this->elements.size(); i++)
    {
        if (!(this->elements[i].type == GEO_SUBTRACT || this->elements[i].type == GEO_ADD))
            continue;

        std::vector<polyfold*> polies;
        polyfold combo, combo2;
        std::vector<int> touched_brushes;
        int num = 0;
        for (int j = 1; j < i; j++)
        {

            if (this->elements[j].has_geometry() &&
                (this->elements[j].type == GEO_SUBTRACT || this->elements[j].type == GEO_ADD) &&
                BoxIntersectsWithBox(this->elements[i].brush.bbox, this->elements[j].brush.bbox))
            {
                touched_brushes.push_back(j);

                polies.push_back(&elements[j].geometry);

                num++;
            }
        }

        if (num > 0)
        {
            combine_polyfolds(polies, combo);
        }

        if (this->base_type == GEO_EMPTY)
            combo.topology = TOP_CONVEX;
        else if (this->base_type == GEO_SOLID)
            combo.topology = TOP_CONCAVE;

        clip_results results;
        results.n_intersections = 0;

        if (num >= 0)
        {
            if (this->elements[i].type == GEO_ADD)
            {
                this->elements[i].brush.make_convex();
                this->elements[i].geometry = this->elements[i].brush;

                clip_poly_accelerated(this->elements[i].geometry, combo, GEO_ADD, this->base_type, results, nograph);
            }
            else if (this->elements[i].type == GEO_SUBTRACT)
            {
                this->elements[i].brush.make_concave();
                this->elements[i].geometry = this->elements[i].brush;

                clip_poly_accelerated(this->elements[i].geometry, combo, GEO_SUBTRACT, this->base_type, results, nograph);
            }
        }

        if (num == 0)
        {
            if ((this->elements[i].type == GEO_ADD && this->base_type == GEO_EMPTY) ||
                (this->elements[i].type == GEO_SUBTRACT && this->base_type == GEO_SOLID))
            {
                if (this->elements[i].type == GEO_ADD)
                {
                    this->elements[i].brush.make_convex();
                }
                else if (this->elements[i].type == GEO_SUBTRACT)
                {
                    this->elements[i].brush.make_concave();
                }

                this->elements[i].geometry = this->elements[i].brush;
            }
            else
            {
                this->elements[i].geometry = no_poly;
            }
        }
        else if(results.n_intersections == 0 && combo.faces.size() > 0 && this->elements[i].brush.vertices.size() > 0)
        {
            vector3df v0 = this->elements[i].brush.vertices[0].V;
            int RES = combo.classify_point(v0, nograph);

            if(this->elements[i].type == GEO_ADD && RES == TOP_FRONT)
            {
                this->elements[i].brush.make_convex();
                this->elements[i].geometry = this->elements[i].brush;
            }
            else if(this->elements[i].type == GEO_SUBTRACT && RES == TOP_BEHIND)
            {

                this->elements[i].brush.make_concave();
                this->elements[i].geometry = this->elements[i].brush;
            }
            else
            {
                this->elements[i].geometry = no_poly;
            }
        }

        if (results.n_intersections > 0)
        {
            results.n_intersections = 0;

            for (int j : touched_brushes)
            {
                this->elements[j].geometry.faces.clear();
                this->elements[j].geometry.edges.clear();
                this->elements[j].geometry.vertices = combo.vertices;

                for (int jj = 0; jj < combo.faces.size(); jj++)
                {
                    if (get_element_index_by_id(combo.faces[jj].element_id) == j)
                    {
                        this->elements[j].geometry.faces.push_back(combo.faces[jj]);
                    }
                }

                for (int jj = 0; jj < this->elements[j].geometry.faces.size(); jj++)
                {
                    this->elements[j].geometry.faces[jj].surface_group = this->elements[j].brush.faces[jj].surface_group;
                }

                this->elements[j].geometry.reduce_edges_vertices();
            }
        }
    } //add, subtract brushes

    for (int i = build_start; i < this->elements.size(); i++)
    {
        if (this->elements[i].type != GEO_SEMISOLID)
            continue;

        std::vector<polyfold*> polies;
        polyfold combo;

        int num = 0;
        for (int j = 1; j < i; j++)
        {
            if (this->elements[j].has_geometry() &&
                (this->elements[j].type == GEO_SUBTRACT || this->elements[j].type == GEO_ADD) &&
                BoxIntersectsWithBox(this->elements[i].brush.bbox, this->elements[j].brush.bbox))
            {
                polies.push_back(&elements[j].geometry);
                num++;
            }
        }

        combine_polyfolds(polies, combo);

        if (this->base_type == GEO_EMPTY)
            combo.topology = TOP_CONVEX;
        else if (this->base_type == GEO_SOLID)
            combo.topology = TOP_CONCAVE;

        clip_results results;
        results.n_intersections = 0;

        if (num >= 0)
        {
            if (this->elements[i].type == GEO_SEMISOLID)
            {
                if (this->elements[i].brush.topology == TOP_CONVEX ||
                    this->elements[i].brush.topology == TOP_CONCAVE)
                {
                    this->elements[i].brush.make_convex();
                    this->elements[i].geometry = this->elements[i].brush;

                    clip_poly_accelerated_single(this->elements[i].geometry, combo, GEO_ADD, this->base_type, results, nograph);
                }
                else //Neither Convex or Concave, ie plane geometry
                {
                    std::cout << "utilizing Plane Geometry\n";
                    this->elements[i].geometry = this->elements[i].brush;
                    clip_poly_accelerated_single(this->elements[i].geometry, combo, GEO_ADD, this->base_type, results, nograph);
                }
            }
        }

        if (num == 0 || results.n_intersections == 0)
        {
            if (this->elements[i].type == GEO_SEMISOLID && this->base_type == GEO_EMPTY)
            {

            }
            else
            {
                //this->elements[i].geometry = no_poly;
            }
        }
    }// semisolid brushes

    /*
    for (int i = 1; i < this->elements.size(); i++)
    {
        for (int j = 0; j < this->elements[i].brush.faces.size() && j < this->elements[i].geometry.faces.size(); j++)
        {
            this->elements[i].geometry.faces[j].texture_name = this->elements[i].brush.faces[j].texture_name;
        }
    }*/

    this->build_progress = this->elements.size() - 1;

    INC_TIMER(rebuild_geometry_time)

    START_TIMER()

    build_total_geometry();

    INC_TIMER(build_total_geometry_time)

    make_index_lists();

    START_TIMER()

    generate_meshes();

    INC_TIMER(generate_meshes_time)

    loops_graph.lines.clear();
    for (geo_element el : elements)
    {
        el.geometry.addDrawLinesEdges(loops_graph);
    }

    SEvent event;
    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1 = USER_EVENT_GEOMETRY_REBUILT;
    event_receiver->OnEvent(event);
}

void GeometryStack::make_index_lists()
{
    n_total_faces = 0;
    for (int i = 1; i < elements.size(); i++)
    {
        n_total_faces += elements[i].brush.faces.size();
    }

    element_id_by_face_n.resize(n_total_faces);
    face_j_by_face_n.resize(n_total_faces);

    int cc = 0;
    for (int i = 1; i < elements.size(); i++)
    {
        for (int j = 0; j < elements[i].surfaces.size(); j++)
        {
            elements[i].surfaces[j].face_index_offset = cc;
        }

        for (int j = 0; j < elements[i].brush.faces.size(); j++)
        {
            element_id_by_face_n[cc] = elements[i].element_id;
            face_j_by_face_n[cc] = j;
            elements[i].reverse_index[j] = cc;
            cc++;
        }
    }

    element_by_element_id.assign(element_id_incrementer, -1);
    for (int i = 1; i < elements.size(); i++)
    {
        int j = elements[i].element_id;
        element_by_element_id[j] = i;
    }
}


void GeometryStack::intersect_active_brush()
{
    polyfold cube = this->elements[0].brush;
    cube.make_convex();

    clip_results results;
    results.n_intersections = 0;

    polyfold no_poly;
    polyfold pf2;
    LineHolder nograph;

    int num = 0;
    std::vector<polyfold*> polies;
    for (int j = 1; j < this->elements.size(); j++)
    {
        if (this->elements[j].has_geometry() &&
            (this->elements[j].type == GEO_SUBTRACT || this->elements[j].type == GEO_ADD) &&
            BoxIntersectsWithBox(cube.bbox, this->elements[j].brush.bbox))
        {
            polies.push_back(&elements[j].geometry);
            num++;
        }
    }
    combine_polyfolds(polies, pf2);

    clip_poly_accelerated(cube, pf2, GEO_SUBTRACT, base_type, results, nograph);

    if (results.n_intersections == 0)
    {
        this->elements[0].brush = pf2;
    }
    else
    {
        polyfold res;
        combine_polyfolds(std::vector<polyfold*>{&pf2, & cube}, res);
        res.remove_empty_faces();
        this->elements[0].brush = res;
        this->elements[0].brush.topology = TOP_CONVEX;
    }
}

void GeometryStack::clip_active_brush()
{
    if (this->elements[0].brush.topology != TOP_CONVEX &&
        this->elements[0].brush.topology != TOP_CONCAVE)
    {
        clip_active_brush_plane_geometry();
        return;
    }

    polyfold cube = this->elements[0].brush;
    cube.make_concave();

    clip_results results;
    results.n_intersections = 0;

    polyfold no_poly;
    polyfold pf2;
    LineHolder nograph;

    std::vector<polyfold*> polies;
    int num = 0;
    for (int j = 1; j < this->elements.size(); j++)
    {
        if (this->elements[j].has_geometry() &&
            (this->elements[j].type == GEO_SUBTRACT || this->elements[j].type == GEO_ADD) &&
            BoxIntersectsWithBox(cube.bbox, this->elements[j].brush.bbox))
        {
            polies.push_back(&elements[j].geometry);
            num++;
        }
    }
    combine_polyfolds(polies, pf2);

    clip_poly_accelerated(cube, pf2, GEO_ADD, base_type, results, nograph);

    if (results.n_intersections == 0)
    {
        this->elements[0].brush = cube;
    }
    else
    {
        polyfold res;
        combine_polyfolds(std::vector<polyfold*>{&pf2, & cube}, res);
        res.remove_empty_faces();
        this->elements[0].brush = res;
        this->elements[0].brush.topology = TOP_CONCAVE;
    }
}

void GeometryStack::clip_active_brush_plane_geometry()
{
    polyfold cube = this->elements[0].brush;

    clip_results results;
    results.n_intersections = 0;

    polyfold no_poly;
    polyfold pf2;
    LineHolder nograph;

    std::vector<polyfold*> polies;
    int num = 0;
    for (int j = 1; j < this->elements.size(); j++)
    {
        if (this->elements[j].has_geometry() &&
            (this->elements[j].type == GEO_SUBTRACT || this->elements[j].type == GEO_ADD) &&
            BoxIntersectsWithBox(cube.bbox, this->elements[j].brush.bbox))
        {
            polies.push_back(&elements[j].geometry);
            num++;
        }
    }
    combine_polyfolds(polies, pf2);

    clip_poly_accelerated_single(cube, pf2, GEO_ADD, base_type, results, nograph);

    for (const poly_vert& v : pf2.control_vertices)
    {
        cube.control_vertices.push_back(v);
    }

    this->elements[0].brush = cube;
}

void GeometryStack::clear_scene()
{
    geo_element active_brush = this->elements[0];
    this->elements.clear();
    this->elements.push_back(active_brush);

    this->build_progress = 0;

    if (this->my_MeshNode)
    {
        this->my_MeshNode->remove();
        this->my_MeshNode = NULL;
    }

    this->rebuild_geometry();
}

polyfold GeometryStack::get_intersecting_geometry(const polyfold& pf)
{
    polyfold combo;

    int num = 0;
    std::vector<polyfold*> polies;
    for (int j = 1; j < this->elements.size(); j++)
    {
        if (this->elements[j].has_geometry() &&
            BoxIntersectsWithBox(pf.bbox, this->elements[j].brush.bbox))
        {

            polies.push_back(&elements[j].geometry);
            num++;
        }
    }
    combine_polyfolds(polies, combo);

    return combo;
}

void GeometryStack::recalculate_final_meshbuffer()
{
    if (final_mesh_dirty)
    {
        std::cout << "recalculating meshbuffers\n";
        //final_meshnode_interface.refresh_material_groups(this);
        edit_meshnode_interface.refresh_material_groups(this);
        edit_meshnode_interface.generate_lightmap_info(this);

        final_meshnode_interface.generate_mesh_node(this);
        final_meshnode_interface.generate_lightmap_info(this);
        final_mesh_dirty = false;
    }
}

triangle_holder* GeometryStack::get_triangles_for_face(int f_i)
{
    return &total_geometry_triangles[f_i];
}

void GeometryStack::setRenderType(bool brushes, bool geo, bool loops, bool triangles)
{
    render_brushes = brushes;
    render_geometry = geo;
    render_loops = loops;
    render_triangles = triangles;
}


void GeometryStack::trianglize_total_geometry()
{
    LineHolder nograph;
    polyfold* pf = get_total_geometry();

    total_geometry_triangles.clear();

    total_geometry_triangles.resize(pf->faces.size());

    for (int i = 0; i < pf->faces.size(); i++)
    {
        if (pf->faces[i].loops.size() > 0)
        {
            pf->trianglize(i, total_geometry_triangles[i], NULL, nograph, nograph);
        }
    }
}

geo_element* GeometryStack::get_element_by_id(int id)
{
    for (geo_element& el : elements)
    {
        if (el.element_id == id)
            return &el;
    }
}

int GeometryStack::get_element_index_by_id(int id)
{
    for (int i = 0; i < elements.size(); i++)
    {
        if (elements[i].element_id == id)
            return i;
    }

    return -1;
}

poly_face* GeometryStack::get_brush_face(face_index idx)
{
    return &get_element_by_id(idx.brush)->brush.faces[idx.face];
}

poly_face* GeometryStack::get_geometry_face(face_index idx)
{
    return &get_element_by_id(idx.brush)->geometry.faces[idx.face];
}

polyfold* GeometryStack::get_brush(face_index idx)
{
    return &get_element_by_id(idx.brush)->brush;
}

polyfold* GeometryStack::get_geometry(face_index idx)
{
    return &get_element_by_id(idx.brush)->geometry;
}

int GeometryStack::get_element_index(face_index idx)
{
    return get_element_index_by_id(idx.brush);
}

int GeometryStack::n_faces()
{
    return n_total_faces;
}
poly_face* GeometryStack::brush_face_by_n(int n)
{
    int i = element_id_by_face_n[n];
    int j = face_j_by_face_n[n];
    return &elements[element_by_element_id[i]].brush.faces[j];
}
poly_face* GeometryStack::geo_face_by_n(int n)
{
    int i = element_id_by_face_n[n];
    int j = face_j_by_face_n[n];
    return &elements[element_by_element_id[i]].geometry.faces[j];
}
polyfold* GeometryStack::brush_by_n(int n)
{
    int i = element_id_by_face_n[n];
    return &elements[element_by_element_id[i]].brush;
}
polyfold* GeometryStack::geo_by_n(int n)
{
    int i = element_id_by_face_n[n];
    return &elements[element_by_element_id[i]].geometry;
}

int GeometryStack::element_id_by_n(int n)
{
    return element_id_by_face_n[n];
}

int GeometryStack::face_no_by_n(int n)
{
    return face_j_by_face_n[n];
}

geo_element* GeometryStack::element_by_n(int n)
{
    int i = element_id_by_face_n[n];
    return &elements[element_by_element_id[i]];
}

core::stringw GeometryStack::face_texture_by_n(int n)
{
    int i = element_id_by_face_n[n];
    int j = face_j_by_face_n[n];
    int surface_no = elements[element_by_element_id[i]].brush.faces[j].surface_no;
    return elements[element_by_element_id[i]].surfaces[surface_no].texture_name;
}

poly_surface* GeometryStack::surface_by_n(int n)
{
    int i = element_id_by_face_n[n];
    int j = face_j_by_face_n[n];
    int surface_no = elements[element_by_element_id[i]].brush.faces[j].surface_no;
    return &elements[element_by_element_id[i]].surfaces[surface_no];
}

void  GeometryStack::GetGeometryLoopLines(LineHolder& lines0, LineHolder& lines1)
{
    for (const geo_element& el : elements)
    {
        const polyfold& pf = el.geometry;

        for (const poly_face& face : pf.faces)
        {
            for (const poly_loop& p : face.loops)
            {
                LineHolder& line = (p.depth % 2 == 0) ? lines0 : lines1;
                int v0, v1;

                for (int i = 0; i < p.vertices.size() -1; i++)
                {
                    v0 = p.vertices[i];
                    v1 = p.vertices[i + 1];
                    line.lines.push_back(core::line3df(pf.vertices[v0].V, pf.vertices[v1].V));
                }

                v0 = p.vertices[p.vertices.size()-1];
                v1 = p.vertices[0];
                line.lines.push_back(core::line3df(pf.vertices[v0].V, pf.vertices[v1].V));
            }
        }
    }

}

void GeometryStack::buildSceneNode(bool finalMesh, int light_mode)
{
    video::IVideoDriver* driver = device->getVideoDriver();

    polyfold* pf = get_total_geometry();

    if (my_MeshNode)
        my_MeshNode->remove();
    my_MeshNode = NULL;

    

    if (finalMesh) //final mesh
    {
        recalculate_final_meshbuffer();

        b_isEditNode = false;
        this->my_MeshNode = final_meshnode_interface.addMeshSceneNode(this, SceneManager, this);

        for (int i = 0; i < my_MeshNode->getMesh()->getMeshBufferCount(); i++)
        {
            scene::IMeshBuffer* buffer = my_MeshNode->getMesh()->getMeshBuffer(i);
            //int f_i = final_meshnode_interface.getMaterialsUsed()[i].faces[0];
            //Material_Groups_Tool::apply_material_to_buffer(buffer, pf->faces[f_i].material_group, light_mode, false, true);
            int mg = final_meshnode_interface.getMaterialsUsed()[i].materialGroup;
            Material_Groups_Tool::apply_material_to_buffer(buffer, mg, light_mode, false, true);

        }

        my_MeshNode->copyMaterials();
    }
    else //edit mesh
    {
        b_isEditNode = true;
        this->my_MeshNode = edit_meshnode_interface.addMeshSceneNode(this, SceneManager, this);

        for (int f_i = 0; f_i < pf->faces.size(); f_i++)
        {
            if (pf->faces[f_i].loops.size() > 0)
            {
                int buffer_index = edit_meshnode_interface.get_buffer_index_by_face(f_i);
                scene::IMeshBuffer* buffer = this->my_MeshNode->getMesh()->getMeshBuffer(buffer_index);

                //Material_Groups_Tool::apply_material_to_buffer(buffer, pf->faces[f_i].material_group, light_mode, false, false);
                Material_Groups_Tool::apply_material_to_buffer(buffer, surface_by_n(f_i)->material_group, light_mode, false, false);

               // video::ITexture* tex_j = driver->getTexture(pf->faces[f_i].texture_name.c_str());
                video::ITexture* tex_j = driver->getTexture(face_texture_by_n(f_i).c_str());

                buffer->getMaterial().setTexture(0, tex_j);
            }
        }

        my_MeshNode->copyMaterials();
    }

    if (render_triangles)
        my_MeshNode->setWireFrame(true);

   // if(final_meshnode_interface.getMaterialsUsed().size() > 0)
   //     Lightmaps_Tool::get_manager()->loadLightmapTextures(this);

    my_MeshNode->setMaterialFlag(video::EMF_LIGHTING, false);

    my_MeshNode->getMesh()->setHardwareMappingHint(scene::EHM_STATIC);

    my_MeshNode->setVisible(true);
}

void GeometryStack::generate_meshes()
{
    trianglize_total_geometry();

    edit_meshnode_interface.generate_mesh_node(this);
    final_meshnode_interface.generate_mesh_node(this);
    final_meshnode_interface.generate_lightmap_info(this);

}

//This function temporarily located in this file
std::vector<int> polyfold::getSurfaceFromFace(int f_i)
{
    face_index f = index_face(f_i);
    geo_element* element = geometry_stack->get_element_by_id(f.brush);

    int sg = element->brush.faces[f.face].surface_group;

    std::vector<int> sel;

    for (int i = 0; i < faces.size(); i++)
    {

        if (faces[i].element_id == f.brush)
        {

            int face_i = faces[i].face_id;

            if (element->brush.faces[face_i].surface_group == sg &&
                element->geometry.faces[face_i].loops.size() > 0)
                sel.push_back(i);
        }
    }
    return sel;
}


void geo_element::draw_brush_vertices(const core::matrix4& trans, core::dimension2du view_dim, bool bDrawSelectedVertex, video::IVideoDriver* driver)
{
    video::ITexture* med_circle;
    video::ITexture* small_circle;

    if (this->type == GEO_ADD || this->type == GEO_SEMISOLID )
    {
        med_circle = med_circle_tex_add_selected;
        small_circle = small_circle_tex_add_selected;
    }
    else if (this->type == GEO_SUBTRACT)
    {
        med_circle = med_circle_tex_sub_selected;
        small_circle = small_circle_tex_sub_selected;
    }
    else if (this->type == GEO_RED)
    {
        med_circle = med_circle_tex_red_selected;
        small_circle = small_circle_tex_red_selected;
    }
    else
        return;

    for (int i = 0; i < this->brush.vertices.size(); i++)
    {
        f32 transformedPos[4] = { this->brush.vertices[i].V.X,
            this->brush.vertices[i].V.Y,
            this->brush.vertices[i].V.Z,
            1.0f };

        trans.multiplyWith1x4Matrix(transformedPos);

        const f32 zDiv = transformedPos[3] == 0.0f ? 1.0f :
            core::reciprocal(transformedPos[3]);

        vector2di coords = vector2di(
            view_dim.Width + core::round32(view_dim.Width * (transformedPos[0] * zDiv)),
            view_dim.Height - core::round32(view_dim.Height * (transformedPos[1] * zDiv)));

        coords.X -= 4;
        coords.Y -= 4;

        if (this->bSelected && bDrawSelectedVertex && this->control_vertex_selected == false && this->selected_vertex == i)
        {
            driver->draw2DImage(med_circle, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
        }
        else
        {
            driver->draw2DImage(small_circle, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
        }
    }
    for (int i = 0; i < this->brush.control_vertices.size(); i++)
    {
        f32 transformedPos[4] = { this->brush.control_vertices[i].V.X,
            this->brush.control_vertices[i].V.Y,
            this->brush.control_vertices[i].V.Z,
            1.0f };

        trans.multiplyWith1x4Matrix(transformedPos);

        const f32 zDiv = transformedPos[3] == 0.0f ? 1.0f :
            core::reciprocal(transformedPos[3]);

        vector2di coords = vector2di(
            view_dim.Width + core::round32(view_dim.Width * (transformedPos[0] * zDiv)),
            view_dim.Height - core::round32(view_dim.Height * (transformedPos[1] * zDiv)));

        coords.X -= 4;
        coords.Y -= 4;

        if (this->bSelected && bDrawSelectedVertex && this->control_vertex_selected == true && this->selected_vertex == i)
        {
            driver->draw2DImage(med_circle, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
        }
        else
        {
            driver->draw2DImage(small_circle, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
        }
    }
}

REFLECT_STRUCT_BEGIN(lightmap_info_struct)
    REFLECT_STRUCT_MEMBER(width)
    REFLECT_STRUCT_MEMBER(height)
    REFLECT_STRUCT_MEMBER(bFlipped)
    REFLECT_STRUCT_MEMBER(bOverrideSize)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(poly_surface)
    REFLECT_STRUCT_MEMBER(my_faces)
    REFLECT_STRUCT_MEMBER(material_group)
    REFLECT_STRUCT_MEMBER(surface_type)
    REFLECT_STRUCT_MEMBER(texture_index)
    REFLECT_STRUCT_MEMBER(face_index_offset)
    REFLECT_STRUCT_MEMBER(lightmap_info)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(geo_element)
    REFLECT_STRUCT_MEMBER(type)
    REFLECT_STRUCT_MEMBER(element_id)
    REFLECT_STRUCT_MEMBER(brush)
    REFLECT_STRUCT_MEMBER(geometry)
    REFLECT_STRUCT_MEMBER(surfaces)
    REFLECT_STRUCT_MEMBER(reverse_index)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(GeometryStack)
    REFLECT_STRUCT_MEMBER(base_type)
    REFLECT_STRUCT_MEMBER(elements)
    REFLECT_STRUCT_MEMBER(element_id_incrementer)
REFLECT_STRUCT_END()


