    Open_Geometry_File::LoadProject(&scene, "../projects/cubetest");


    cube = scene.elements[0].brush;
    cube.topology = TOP_CONVEX;

   // scene.rebuild_geometry();

    //======
    //cube = scene.elements[2].brush;
    //cube.make_convex();
    //if(op_type == GEO_SUBTRACT)

    cube.make_convex();

    pf2 = *scene.get_total_geometry();
    
    //pf2 = scene.elements[1].geometry;

   // addDrawLines(pf2, graph, graph2, graph3);

    //std::cout<<"Begin Testing:\n\n";

    polyfold no_poly;
    polyfold combo, combo2, combo3, cube2, cube3;

    no_poly.topology = TOP_CONCAVE;
    clip_results results;
    results.n_intersections=0;
    
   // cube.build_vertices_BVH();
    scene.build_intersecting_target(cube, combo);
    
    //START_TIMER()

    reset_testclip_timers();
    reset_geometry_ops_timers();

    std::cout << "N: " << cube.edges.size() * combo.faces.size() << "\n";
    std::cout << "M: " << combo.edges.size() * cube.faces.size() << "\n\n";

    for (int i = 0; i < 100; i++)
    {
        cube2 = cube;
        combo2 = combo;
        clip_poly_no_acceleration(cube2, combo2, GEO_ADD, GEO_SOLID, results, nograph);
        //std::cout << "ok\n";
    }

    for (int i = 0; i < 100; i++)
    {
        cube3 = cube;
        combo3 = combo;
        clip_poly_accelerated(cube3, combo3, GEO_ADD, GEO_SOLID, results, nograph);
        //std::cout << "ok\n";
    }

    //addDrawLines(combo2, graph, graph2, graph3);

    for (int i = 0; i < combo3.vertices.size(); i++)
    {
        //if (combo3.find_point(combo2.vertices[i].V) == -1)
        {
          //  graph.points.push_back(combo3.vertices[i].V);
          //  std::cout << i << ": " << combo2.vertices[i].V.X << "," << combo2.vertices[i].V.Y << "," << combo2.vertices[i].V.Z << "\n";
        }
    }
    /*
    std::cout << cube.vertices.size() << "\n";
    for (int i = 0; i < 100; i++)
    {
        //cutoff ~200 vertices
        cube.build_edges_BVH();
        cube.build_vertices_BVH();
        cube.reduce_edges_vertices_accelerated(cube,nograph);
       // cube.reduce_edges_vertices();
       // combo2 = combo;
       // clip_poly(cube2, combo2, GEO_ADD, GEO_SOLID, results, nograph);
      //  scene.rebuild_geometry();
    }*/

    print_testclip_timers();
   // print_geometry_ops_timers();

    //PRINT_TIMER(test_time);
    //cube = scene.elements[0].brush;

    
   // combine_polyfolds(std::vector<polyfold*>{&cube, & combo}, pf2);

    addDrawLines(combo3, graph, graph2, graph3);
   // cube.faces[4].loops[0].direction = LOOP_DIRECTION_UNDEF;
    //cube.set_loop_solid(1, 0);
   // cube.set_loop_solid(3, 2);
    
    //cube.faces[5].flip_normal();
   // cube.faces[0].loops[0].reverse();
   // do_test_loop(pf2, 0, 1, graph, graph2);
   // do_test_loop(pf2, 500, 0, graph, graph2);
    //combine_polyfolds_linear(std::vector<polyfold*>{&pf2}, combo);
    //combine_polyfolds_accelerated(std::vector<polyfold*>{&pf2}, combo);

   // cube.addDrawLinesFacesByIndex(std::vector<u16>{0}, graph2);
   // combo.addDrawLinesFacesByIndex(std::vector<u16>{1}, graph2);

    //

    //

    
    
    

   // cube.vertices_BVH.addDrawLinesLeaves(graph);
    /*
    int dupcount = 0;
    for (int i = 0; i < cube.vertices_BVH.node_count; i++)
    {
        if (cube.vertices_BVH.nodes[i].isLeafNode() && cube.vertices_BVH.nodes[i].n_prims > 1)
            dupcount++;
    }
    std::cout << dupcount << " dups\n";*/
    /*
    std::vector<vector3df> test_results;

    scene::CMeshSceneNode* mesh_node = NULL;
    srand(34567);

    std::cout << cube.vertices.size() << "\n";
    for (int i = 0; i < 32; i++)
    {
        int v_i = rand() % cube.vertices.size();

        vector3df avec = cube.vertices[v_i].V;
        cube.vertices.push_back(poly_vert(avec.X,avec.Y,avec.Z));
    }


    std::cout << cube.vertices.size() << "\n";
    test_results.clear();

    START_TIMER()

    cube.build_vertices_BVH();*/
    /*
    dupcount = 0;
    for (int i = 0; i < cube.vertices_BVH.node_count; i++)
    {
        if (cube.vertices_BVH.nodes[i].isLeafNode() && cube.vertices_BVH.nodes[i].n_prims > 1)
            dupcount+= cube.vertices_BVH.nodes[i].n_prims - 1;
    }
    std::cout << dupcount << " dups\n";
    std::cout << "ok...\n";*/
    /*
    for (int i = 0; i < cube.vertices_BVH.node_count; i++)
    {
        BVH_node& node = cube.vertices_BVH.nodes[i];
        if (node.isLeafNode())
        {
            u16 v_i = cube.vertices_BVH.indices[node.first_prim];
            test_results.push_back(cube.vertices[v_i].V);
        }
    }
    std::cout << test_results.size() << "\n";
    PRINT_TIMER(build_bvh)

    test_results.clear();

    START_TIMER()

    for (int i = 0; i < cube.vertices.size(); i++)
    {
        bool b = false;
        for (int j = 0; j < test_results.size(); j++)
        {
            if (is_same_point(test_results[j], cube.vertices[i].V))
            {
                b = true;
                break;
            }
        }
        if (!b)
            test_results.push_back(cube.vertices[i].V);
    }
    std::cout << test_results.size() << "\n";
    PRINT_TIMER(linear_search)*/
    /*
    std::vector<int> test_results;
    test_results.resize(cube.vertices.size());

    START_TIMER()

    for (int i = 0; i < cube.vertices.size(); i++)
    {
        int v_i = rand() % cube.vertices.size();

        int v_j = cube.find_point(cube.vertices[v_i].V);
        test_results[i] = v_j;
    }

    PRINT_TIMER(linear_search)

    START_TIMER()

    for (int i = 0; i < cube.vertices.size(); i++)
    {
        int v_i = rand() % cube.vertices.size();

        std::vector<u16> hits;
        cube.vertices_BVH.intersect(cube.vertices[v_i].V, hits);

        for (u16 v_j : hits)
            if (is_same_point(cube.vertices[v_i].V, cube.vertices[v_j].V))
                test_results[i] = v_j;
    }

    PRINT_TIMER(accelerated_search)
    */
    /*
    cube.build_edges_BVH();
    cube.build_faces_BVH();

   // cube.faces_BVH.addDrawLinesLeaves(graph);
    std::vector<u16> nfaces0;
    std::vector<u16> nfaces1;
    int e_0 = 62;
    std::vector<u16> hits;

    aabb_struct aabb;
    cube.edges[e_0].grow(&aabb, &cube);
    cube.faces_BVH.intersect(aabb,hits);

    aabb.addDrawLines(graph2);
    for (u16 f_i : hits)
    {
        std::cout << f_i << "\n";
        
        for (int e_i : cube.faces[f_i].edges)
        {
            if (e_i == e_0)
                nfaces0.push_back(f_i);
        }
    }
    cube.addDrawLinesFacesByIndex(hits, graph);

    std::cout << "----\n";
    for (int f_i = 0; f_i < cube.faces.size(); f_i++)
    {
        for (int e_i : cube.faces[f_i].edges)
        {
            if (e_i == e_0)
            {
                std::cout << f_i << "\n";
                nfaces1.push_back(f_i);
            }
        }
    }

std::cout << nfaces0.size() << " / " << nfaces1.size() << "\n\n";
*/
/*
    pf = pf2;
    scene::CMeshSceneNode* mesh_node = NULL;
    //===========
    
    scene::SMeshBuffer* buffer;
    scene::SMesh* mesh = new scene::SMesh;

    buffer = new scene::SMeshBuffer();
	buffer->Material.Lighting = false;
	buffer->Material.setTexture(0, driver->getTexture("wall.bmp"));
    //for(int f_i =0 ;f_i<pf.faces.size(); f_i++)
    {
        int f_i = 0;
        //std::cout << f_i << " " << pf.faces[f_i].loops.size()<<"\n";
        //if(f_i==0)
        {
            for (int p_i = 0; p_i < pf.faces[f_i].loops.size(); p_i++)
            {
                std::cout << p_i << ": ";
                std::cout << pf.faces[f_i].loops[p_i].depth << "\n";
            }
        buffer = new scene::SMeshBuffer();
        buffer->Material.Lighting = false;
        buffer->Material.setTexture(0, driver->getTexture("wall.bmp"));

        pf.trianglize(f_i,buffer,graph,graph2);

        mesh->addMeshBuffer(buffer);
        //std::cout<<buffer->getIndexCount()<<"\n";
        //for(int i=0;i<buffer->getIndexCount();i+=3)
        //    std::cout<<buffer->Indices[i]<<" "<<buffer->Indices[i+1]<<" "<<buffer->Indices[i+2]<<"\n";
        }
    }*/
   // mesh_node = new scene::CMeshSceneNode(mesh,smgr->getRootSceneNode(),smgr,747);
   //// mesh_node->setWireFrame(true);
   // mesh_node->SetBox(pf.bbox);
   // mesh->drop();
    
    //=============

    //scene.ExportFinalMesh("final_mesh.msh");

    //addDrawLines(pf, graph,graph2,graph3);
    //std::cout << graph4.lines.size() << " lines\n";

    //C:\Projects\tkt - projects\test_4

    
