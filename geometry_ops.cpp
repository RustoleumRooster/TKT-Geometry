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

#include <chrono>

#include "clip_functions.h"

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

polyfold* geometry_scene::get_total_geometry()
{
    return &this->total_geometry;
}

void geometry_scene::build_total_geometry()
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
        //add_pfold(this->elements[i].geometry,combo);
        polies.push_back(&this->elements[i].geometry);
    }
    //std::cout << "total geometry:\n";

    //combine_polyfolds_accelerated(polies, combo);

    combine_polyfolds(polies, combo);

    //std::cout << combo.vertices.size() << " vertices\n";

    // combine_polyfolds_linear(polies, combo2);
     /*
     for (int f_i = 0; f_i < combo.faces.size(); f_i++)
     {
         for (int p_i = 0; p_i < combo.faces[f_i].loops.size(); p_i++)
         {
             for (int i = 0; i < combo.faces[f_i].loops[p_i].vertices.size(); i++)
             {
                 //std::cout << i << " ";
                 int v_i0 = combo.faces[f_i].loops[p_i].vertices[i];
                 int v_i1 = combo2.faces[f_i].loops[p_i].vertices[i];
                 if (combo.vertices[v_i0].V != combo2.vertices[v_i1].V)
                 {
                     std::cout << "mismatch: "<<printvec(combo.vertices[v_i0].V)<<"  "<<printvec(combo2.vertices[v_i1].V)<<"\n";
                 }
             }
         }
     }*/
     /*
     for (poly_vert v0 : combo.vertices)
     {
         if (combo2.find_point(v0.V) == -1)
             std::cout << "could not find point\n";
     }

     for (poly_edge e0 : combo.edges)
     {
         if (combo2.find_edge(combo.vertices[e0.v0].V, combo.vertices[e0.v1].V) == -1)
             std::cout << "could not find edge\n";
     }

     for (poly_vert v0 : combo2.vertices)
     {
         if (combo.find_point(v0.V) == -1)
             std::cout << "could not find point2\n";
     }

     for (poly_edge e0 : combo2.edges)
     {
         if (combo.find_edge(combo2.vertices[e0.v0].V, combo2.vertices[e0.v1].V) == -1)
             std::cout << "could not find edge2\n";
     }

     if (combo.edges.size() != combo2.edges.size())
         std::cout << "edges size mismatch: "<< combo.edges.size()<<" to " << combo2.edges.size()<<"\n";

     if (combo.vertices.size() != combo2.vertices.size())
     {
         std::cout << "verts size mismatch: " << combo.vertices.size() << " to " << combo2.vertices.size() << "\n";
         for(int v_i=0;v_i<combo.vertices.size(); v_i++)
         {
             for (int v_i2 = 0; v_i2 < combo.vertices.size(); v_i2++)
             {
                 if (is_same_point(combo.vertices[v_i].V, combo.vertices[v_i2].V) && v_i != v_i2)
                 {
                     std::cout << v_i << ", " << v_i2 << " is duplicate\n";
                     std::cout << combo.vertices[v_i].V.X << "," << combo.vertices[v_i].V.Y << "," << combo.vertices[v_i].V.Z << "\n";
                     std::cout << combo.vertices[v_i2].V.X << "," << combo.vertices[v_i2].V.Y << "," << combo.vertices[v_i2].V.Z << "\n";
                 }
             }
         }
     }*/

     // intersections_graph.lines.clear();


      //std::cout << combo2.edges.size() << " edges\n";
      //combo.addDrawLinesEdges(intersections_graph);


    //std::cout << "total geometry created " << this->elements.size() << " elements\n";

    this->total_geometry = combo;
    //for (int i = 0; i < total_geometry.faces.size(); i++)
    //{
    //    total_geometry.calc_tangent(i);
    //}

    if (this->base_type == GEO_EMPTY)
        this->total_geometry.topology = TOP_CONVEX;
    else if (this->base_type == GEO_SOLID)
        this->total_geometry.topology = TOP_CONCAVE;
}

void geometry_scene::set_originals()
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

void geometry_scene::add(polyfold pf)
{
    //this->WriteGeometryToFile("backup.pol");

    geo_element geo;
    geo.brush = pf;
    geo.type = GEO_ADD;

    {
        video::ITexture* tex = texture_picker_base->getCurrentTexture();
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

void geometry_scene::add_plane(polyfold pf)
{
    //this->WriteGeometryToFile("backup.pol");

    geo_element geo;
    geo.brush = pf;
    geo.type = GEO_SEMISOLID;

    {
        video::ITexture* tex = texture_picker_base->getCurrentTexture();
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

void geometry_scene::add_semisolid(polyfold pf)
{
    //this->WriteGeometryToFile("backup.pol");

    geo_element geo;
    geo.brush = pf;
    geo.type = GEO_SEMISOLID;

    {
        video::ITexture* tex = texture_picker_base->getCurrentTexture();
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

void geometry_scene::subtract(polyfold pf)
{
    //this->WriteGeometryToFile("backup.pol");

    geo_element geo;
    geo.brush = pf;
    geo.type = GEO_SUBTRACT;

    {
        video::ITexture* tex = texture_picker_base->getCurrentTexture();
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

void geometry_scene::build_intersecting_target(const polyfold& pf, polyfold& out)
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

void geometry_scene::rebuild_geometry(bool only_build_new_geometry)
{
    //std::cout << "rebuild...\n";

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

    intersections_graph.lines.clear();

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

                clip_poly_accelerated(this->elements[i].geometry, combo, GEO_ADD, this->base_type, results, intersections_graph);
            }
            else if (this->elements[i].type == GEO_SUBTRACT)
            {
                this->elements[i].brush.make_concave();
                this->elements[i].geometry = this->elements[i].brush;

                clip_poly_accelerated(this->elements[i].geometry, combo, GEO_SUBTRACT, this->base_type, results, intersections_graph);
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

        //std::cout << i << ": intersects " << num << " other elements (plane geometry) \n";

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

                    clip_poly_single(this->elements[i].geometry, combo, GEO_ADD, this->base_type, results, nograph);
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

    //GenLightMaps* LM = LightMaps_Tool::getLightmaps();

    //if(LM)
    //    LM->calc_lightmap_tcoords();

    SEvent event;
    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1 = USER_EVENT_GEOMETRY_REBUILT;
    event_receiver->OnEvent(event);
}


void geometry_scene::intersect_active_brush()
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

void geometry_scene::clip_active_brush()
{
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

void geometry_scene::clip_active_brush_plane_geometry()
{/*
    polyfold cube = this->elements[0].brush;

    clip_results results;
    results.n_intersections = 0;

    polyfold no_poly;
    polyfold pf2;
    LineHolder nograph;

    int num = 0;
    for (int j = 1; j < this->elements.size(); j++)
    {
        if (this->elements[j].has_geometry() &&
            (this->elements[j].type == GEO_SUBTRACT || this->elements[j].type == GEO_ADD) &&
            BoxIntersectsWithBox(cube.bbox, this->elements[j].brush.bbox))
        {
            add_pfold(this->elements[j].geometry, pf2);
            num++;
        }
    }

    clip_poly_single(cube, pf2, GEO_ADD, base_type, results, nograph);

    this->elements[0].brush = cube;*/
}