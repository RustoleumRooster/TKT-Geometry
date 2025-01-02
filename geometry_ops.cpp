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

#include <chrono>

#include "clip_functions.h"

extern IrrlichtDevice* device;

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


void GeometryStack::initialize(scene::ISceneManager* smgr, MyEventReceiver* receiver)
{
    SceneManager = smgr;
    event_receiver = receiver;
}

GeometryStack::~GeometryStack()
{
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

    if(render_active_brush)
        this->elements[0].draw_brush(driver, someMaterial);

    if (render_brushes)
    {

        for (int e_i = 0; e_i < this->elements.size(); e_i++)
        {
            this->elements[e_i].draw_brush(driver, someMaterial);
        }

        for (int e_i = 0; e_i < this->elements.size(); e_i++)
        {
            geo_element* geo = &this->elements[e_i];
            /*
            if (geo->bSelected)
            {

                core::vector2di coords;
                for (int i = 0; i < geo->brush.vertices.size(); i++)
                {
                    GetScreenCoords(geo->brush.vertices[i].V, coords);
                    coords.X -= 4;
                    coords.Y -= 4;
                    if (geo_scene->selected_brush_vertex_editing == e_i && geo->control_vertex_selected == false && geo->selected_vertex == i)
                    {
                        if (geo->type == GEO_ADD)
                            driver->draw2DImage(med_circle_tex_add_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        else if (geo->type == GEO_SUBTRACT)
                            driver->draw2DImage(med_circle_tex_sub_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        else if (geo->type == GEO_RED)
                            driver->draw2DImage(med_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                    }
                    else
                    {
                        if (geo->type == GEO_ADD)
                            driver->draw2DImage(small_circle_tex_add_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        else if (geo->type == GEO_SUBTRACT)
                            driver->draw2DImage(small_circle_tex_sub_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        else if (geo->type == GEO_RED)
                            driver->draw2DImage(small_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                    }
                }
                for (int i = 0; i < geo->brush.control_vertices.size(); i++)
                {
                    GetScreenCoords(geo->brush.control_vertices[i].V, coords);
                    coords.X -= 4;
                    coords.Y -= 4;
                    if (geo_scene->selected_brush_vertex_editing == e_i && geo->control_vertex_selected == true && geo->selected_vertex == i)
                    {
                        if (geo->type == GEO_ADD)
                            driver->draw2DImage(med_circle_tex_add_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        else if (geo->type == GEO_SUBTRACT)
                            driver->draw2DImage(med_circle_tex_sub_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        else if (geo->type == GEO_RED)
                            driver->draw2DImage(med_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                    }
                    else
                    {
                        if (geo->type == GEO_ADD)
                            driver->draw2DImage(small_circle_tex_add_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        else if (geo->type == GEO_SUBTRACT)
                            driver->draw2DImage(small_circle_tex_sub_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        else if (geo->type == GEO_RED)
                            driver->draw2DImage(small_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                    }
                }
            }*/
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

    if (this->base_type == GEO_EMPTY)
        this->total_geometry.topology = TOP_CONVEX;
    else if (this->base_type == GEO_SOLID)
        this->total_geometry.topology = TOP_CONCAVE;
}

void GeometryStack::set_originals()
{
    for (int i = 1; i < this->elements.size(); i++)
    {
        for (int j = 0; j < this->elements[i].brush.faces.size() && j < this->elements[i].geometry.faces.size(); j++)
        {
            this->elements[i].geometry.faces[j].texture_name = this->elements[i].brush.faces[j].texture_name;
            this->elements[i].geometry.faces[j].original_brush = i;
            this->elements[i].geometry.faces[j].original_face = j;
        }
    }
}

bool geo_element::has_geometry()
{
    return this->geometry.edges.size() > 2;
}

void GeometryStack::add(polyfold pf)
{
    //this->WriteGeometryToFile("backup.pol");

    geo_element geo;
    geo.brush = pf;
    geo.type = GEO_ADD;

    geo.brush.generate_uids();

    {
        video::ITexture* tex = TexturePicker_Tool::getCurrentTexture();
        io::path path = tex->getName();

        for (poly_face& face : geo.brush.faces)
        {
            face.texture_name = path;
            face.material_group = new_geometry_material_group;
        }
    }

    this->elements.push_back(geo);

    if (this->progressive_build)
        this->rebuild_geometry(true);
}

void GeometryStack::add_plane(polyfold pf)
{
    //this->WriteGeometryToFile("backup.pol");

    geo_element geo;
    geo.brush = pf;
    geo.type = GEO_SEMISOLID;

    geo.brush.generate_uids();

    {
        video::ITexture* tex = TexturePicker_Tool::getCurrentTexture();
        io::path path = tex->getName();

        for (poly_face& face : geo.brush.faces)
        {
            face.texture_name = path;
            face.material_group = new_geometry_material_group;
        }
    }

    for (poly_face& face : geo.brush.faces)
        face.material_group = 1;

    this->elements.push_back(geo);

    if (this->progressive_build)
        this->rebuild_geometry(true);
}

void GeometryStack::add_semisolid(polyfold pf)
{
    //this->WriteGeometryToFile("backup.pol");

    geo_element geo;
    geo.brush = pf;
    geo.type = GEO_SEMISOLID;

    geo.brush.generate_uids();

    {
        video::ITexture* tex = TexturePicker_Tool::getCurrentTexture();
        io::path path = tex->getName();

        for (poly_face& face : geo.brush.faces)
        {
            face.texture_name = path;
            face.material_group = new_geometry_material_group;
        }
    }

    this->elements.push_back(geo);

    if (this->progressive_build)
        this->rebuild_geometry(true);
}

void GeometryStack::subtract(polyfold pf)
{
    //this->WriteGeometryToFile("backup.pol");

    geo_element geo;
    geo.brush = pf;
    geo.type = GEO_SUBTRACT;

    geo.brush.generate_uids();

    {
        video::ITexture* tex = TexturePicker_Tool::getCurrentTexture();
        io::path path = tex->getName();

        for (poly_face& face : geo.brush.faces)
        {
            face.texture_name = path;
            face.material_group = new_geometry_material_group;
        }
    }

    this->elements.push_back(geo);

    if (this->progressive_build)
        this->rebuild_geometry(true);
}

void GeometryStack::build_intersecting_target(const polyfold& pf, polyfold& out)
{
    std::vector<polyfold*> polies;

    int num = 0;
    for (int j = 1; j < elements.size(); j++)
    {

        if (this->elements[j].has_geometry() &&
            (this->elements[j].type == GEO_SUBTRACT || this->elements[j].type == GEO_ADD) &&
            BoxIntersectsWithBox(pf.bbox, this->elements[j].brush.bbox))
        {
            for (int f_j = 0; f_j < this->elements[j].geometry.faces.size(); f_j++)
            {
                //this->elements[j].geometry.faces[f_j].original_brush = j;
                //this->elements[j].geometry.faces[f_j].original_face = this->elements[j].geometry.faces[f_j].surface_group;
            }

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
                for (int f_j = 0; f_j < this->elements[j].geometry.faces.size(); f_j++)
                {
                    this->elements[j].geometry.faces[f_j].original_brush = j;
                    this->elements[j].geometry.faces[f_j].original_face = this->elements[j].geometry.faces[f_j].surface_group;
                }

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
                    if (combo.faces[jj].original_brush == j)
                    {
                        this->elements[j].geometry.faces.push_back(combo.faces[jj]);
                    }
                }

                for (int jj = 0; jj < this->elements[j].geometry.faces.size(); jj++)
                {
                    this->elements[j].geometry.faces[jj].surface_group = this->elements[j].geometry.faces[jj].original_face;
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

        std::vector<int> touched_brushes;
        int num = 0;
        for (int j = 1; j < i; j++)
        {
            if (this->elements[j].has_geometry() &&
                (this->elements[j].type == GEO_SUBTRACT || this->elements[j].type == GEO_ADD) &&
                BoxIntersectsWithBox(this->elements[i].brush.bbox, this->elements[j].brush.bbox))
            {
                for (int f_j = 0; f_j < this->elements[j].geometry.faces.size(); f_j++)
                {
                    this->elements[j].geometry.faces[f_j].original_brush = j;
                    this->elements[j].geometry.faces[f_j].original_face = this->elements[j].geometry.faces[f_j].surface_group;
                }
                touched_brushes.push_back(j);

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
                    std::cout << "Plane Geometry\n";
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
                this->elements[i].geometry = no_poly;
            }
        }
    }// semisolid brushes

    for (int i = 1; i < this->elements.size(); i++)
    {
        for (int j = 0; j < this->elements[i].brush.faces.size() && j < this->elements[i].geometry.faces.size(); j++)
        {
            this->elements[i].geometry.faces[j].texture_name = this->elements[i].brush.faces[j].texture_name;
            this->elements[i].geometry.faces[j].original_brush = i;
            this->elements[i].geometry.faces[j].original_face = j;
        }
    }

    this->build_progress = this->elements.size() - 1;

    INC_TIMER(rebuild_geometry_time)

    START_TIMER()

    build_total_geometry();

    INC_TIMER(build_total_geometry_time)

    START_TIMER()

    generate_meshes();

    INC_TIMER(generate_meshes_time)

    loops_graph.lines.clear();
    for (geo_element el : elements)
    {
        el.geometry.addDrawLinesEdges(loops_graph);
    }

    //GenLightMaps* LM = LightMaps_Tool::getLightmaps();

    //if(LM)
    //    LM->calc_lightmap_tcoords();

    SEvent event;
    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1 = USER_EVENT_GEOMETRY_REBUILT;
    event_receiver->OnEvent(event);
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

    this->elements[0].brush = cube;
}

void GeometryStack::clear_scene()
{
    geo_element active_brush = this->elements[0];
    this->elements.clear();
    this->elements.push_back(active_brush);

    this->build_progress = 0;

    if (this->getMeshNode())
    {
        this->getMeshNode()->remove();
    }

    this->rebuild_geometry();
}

polyfold GeometryStack::get_intersecting_geometry(polyfold pf)
{
    polyfold combo;
    std::vector<int> touched_brushes;
    int num = 0;
    std::vector<polyfold*> polies;
    for (int j = 1; j < this->elements.size(); j++)
    {
        if (this->elements[j].has_geometry() &&
            BoxIntersectsWithBox(pf.bbox, this->elements[j].brush.bbox))
        {
            for (int f_j = 0; f_j < this->elements[j].geometry.faces.size(); f_j++)
            {
                this->elements[j].geometry.faces[f_j].original_brush = j;
                this->elements[j].geometry.faces[f_j].original_face = this->elements[j].geometry.faces[f_j].surface_group;
            }
            touched_brushes.push_back(j);

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
        final_meshnode_interface.refresh_material_groups(this);
        final_meshnode_interface.generate_mesh_node(this);
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

std::vector<int> GeometryStack::getSurfaceFromFace(int b_i)
{
    int brush_j = get_total_geometry()->faces[b_i].original_brush;
    int face_j = get_total_geometry()->faces[b_i].original_face;

    poly_face* f = &elements[brush_j].brush.faces[face_j];
    std::vector<int> sel;
    int sg = f->surface_group;
    for (int i = 0; i < get_total_geometry()->faces.size(); i++)
    {
        if (get_total_geometry()->faces[i].original_brush == brush_j)
        {
            int face_i = get_total_geometry()->faces[i].original_face;

            if (elements[brush_j].brush.faces[face_i].surface_group == sg &&
                elements[brush_j].geometry.faces[face_i].loops.size() > 0)
                sel.push_back(i);
        }
    }
    return sel;

}

void GeometryStack::buildSceneNode(bool finalMesh, int light_mode)
{
    video::IVideoDriver* driver = device->getVideoDriver();

    polyfold* pf = get_total_geometry();

    if (my_MeshNode)
        my_MeshNode->remove();
    my_MeshNode = NULL;

    if (final_mesh_dirty)
    {
        std::cout << "recalculating meshbuffers\n";
        final_meshnode_interface.refresh_material_groups(this);
        final_meshnode_interface.generate_mesh_node(this);
    }

    if (finalMesh) //final mesh
    {
        b_isEditNode = false;
        this->my_MeshNode = final_meshnode_interface.addMeshSceneNode(this, SceneManager, this);

        for (int i = 0; i < my_MeshNode->getMesh()->getMeshBufferCount(); i++)
        {
            scene::IMeshBuffer* buffer = my_MeshNode->getMesh()->getMeshBuffer(i);
            int f_i = final_meshnode_interface.getMaterialsUsed()[i].faces[0];
            Material_Groups_Tool::apply_material_to_buffer(buffer, pf->faces[f_i].material_group, light_mode, false, true);

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

                Material_Groups_Tool::apply_material_to_buffer(buffer, pf->faces[f_i].material_group, light_mode, false, false);

                video::ITexture* tex_j = driver->getTexture(pf->faces[f_i].texture_name.c_str());

                buffer->getMaterial().setTexture(0, tex_j);
            }
        }

        my_MeshNode->copyMaterials();
    }

    my_MeshNode->setMaterialFlag(video::EMF_LIGHTING, false);

    my_MeshNode->getMesh()->setHardwareMappingHint(scene::EHM_STATIC);

    my_MeshNode->setVisible(true);
}

void GeometryStack::generate_meshes()
{
    trianglize_total_geometry();

    edit_meshnode_interface.generate_mesh_node(this);

    final_meshnode_interface.refresh_material_groups(this);
    final_meshnode_interface.generate_mesh_node(this);
}

poly_face* GeometryStack::get_original_brush_face(int f_i)
{
    int brush_j = total_geometry.faces[f_i].original_brush;
    int face_j = total_geometry.faces[f_i].original_face;

    poly_face* f = &elements[brush_j].brush.faces[face_j];

    return f;
}


polyfold* GeometryStack::get_original_brush(int f_i)
{
    int brush_j = total_geometry.faces[f_i].original_brush;

    polyfold* pf = &elements[brush_j].brush;

    return pf;
}

#include "GeometryStack.h"




REFLECT_STRUCT_BEGIN(geo_element)
    REFLECT_STRUCT_MEMBER(type)
    REFLECT_STRUCT_MEMBER(brush)
    REFLECT_STRUCT_MEMBER(geometry)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(GeometryStack)
    REFLECT_STRUCT_MEMBER(base_type)
    REFLECT_STRUCT_MEMBER(elements)
REFLECT_STRUCT_END()
