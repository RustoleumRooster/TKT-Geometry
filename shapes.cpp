
/*
	polyfold pf2 = make_poly_cube(100,300,300);
    polyfold pf = make_poly_cube(200,100,100);
    polyfold pf4 =  make_poly_cube(100,100,100);


    pf.make_concave();
    pf2.make_concave();
    pf4.make_concave();

    //0.785398
    //MAT2.setTranslation(core::vector3df(50,0,-50));
    //pf.translate(MAT2);
   // MAT2.setRotationAxisRadians(0.62,core::vector3df(0,0,1));
   // pf.rotate(MAT2);
   // MAT2.setRotationAxisRadians(-1.1,core::vector3df(0,1,0));
   // pf.rotate(MAT2);


    //MAT2.setRotationAxisRadians(-0.25,core::vector3df(0,0,1));
    //pf.rotate(MAT2);

    //MAT2.setTranslation(core::vector3df(50,-25,-75));
    pf.translate(MAT2);

    //pf2.translate(MAT2);
    MAT.setRotationAxisRadians(0.3,core::vector3df(0,0,1));

    geometry_scene scene;
    pf2.rotate(MAT);
    scene.add(pf2);

    pf.rotate(MAT);
    scene.subtract(pf);

    MAT2.setTranslation(core::vector3df(175,-50,125));
    pf.translate(MAT2);
    //scene.subtract(pf);

    pf2 = make_poly_cube(100,75,75);
    MAT2.setTranslation(core::vector3df(75,-50,125));
    pf2.translate(MAT2);
    pf2.rotate(MAT);
    scene.add(pf2);

    pf2 = make_poly_cube(100,150,75);
    MAT2.setTranslation(core::vector3df(100,0,250));
    pf2.translate(MAT2);
    pf2.rotate(MAT);
    scene.subtract(pf2);
*/


/*

    polyfold cube = make_poly_cube(768,768,768);
    MAT.setTranslation(core::vector3df(-384,0,-384));
    cube.translate(MAT);
    scene.add(cube);

    cube = make_poly_cube(702,702,702);
    MAT.setTranslation(core::vector3df(-351,32,-351));
    cube.translate(MAT);
    scene.subtract(cube);

    cube = make_poly_cube(32,512,512);
    MAT.setTranslation(core::vector3df(-256,0,-256));
    cube.translate(MAT);
    scene.subtract(cube);

    cube = make_poly_cube(128,1000,128);
    MAT.setTranslation(core::vector3df(-500,64,-128));
    cube.translate(MAT);
    scene.subtract(cube);

    MAT.setTranslation(core::vector3df(0,256,-128));
    cube.translate(MAT);
    scene.subtract(cube);

    MAT.setTranslation(core::vector3df(0,128,256));
    cube.translate(MAT);
    scene.subtract(cube);
*/

/*
 core::matrix4 MAT;
    core::matrix4 MAT2;
    MAT2.setRotationAxisRadians(0.4,core::vector3df(0,0,1));

    polyfold cube = make_poly_cube(128,512,512);
    MAT.setTranslation(core::vector3df(0,0,0));
    cube.translate(MAT);
    cube.rotate(MAT2);
    scene.subtract(cube);

    cube = make_poly_cube(128,128,512);
    MAT.setTranslation(core::vector3df(0,0,512));
    cube.translate(MAT);
    cube.rotate(MAT2);
    scene.subtract(cube);

    cube = make_poly_cube(128,128,512);
    MAT.setTranslation(core::vector3df(188,0,-512));
    cube.translate(MAT);
    cube.rotate(MAT2);
    scene.subtract(cube);

    cube = make_poly_cube(128,96,96);
    MAT.setTranslation(core::vector3df(64,0,64));
    cube.translate(MAT);
    cube.rotate(MAT2);
    scene.add(cube);

    cube = make_poly_cube(128,96,96);
    MAT.setTranslation(core::vector3df(352,0,64));
    cube.translate(MAT);
    cube.rotate(MAT2);
    scene.add(cube);

    cube = make_poly_cube(128,96,96);
    MAT.setTranslation(core::vector3df(352,0,320));
    cube.translate(MAT);
    cube.rotate(MAT2);
    scene.add(cube);
*/

/*

    polyfold cube1 = make_poly_cube(128,128,512);
    MAT.setTranslation(core::vector3df(0,0,0));
    cube1.translate(MAT);
    scene.add(cube1);

    polyfold cube2 = make_poly_cube(512,128,128);
    MAT.setRotationAxisRadians(0.7,core::vector3df(0,0,1));
    MAT.setTranslation(core::vector3df(0,0,0));
    cube2.rotate(MAT);
    cube2.translate(MAT);
    scene.add(cube2);
    */

/*

    polyfold cube1 = make_poly_cube(384,256,768);
    MAT.setTranslation(core::vector3df(0,0,0));
    cube1.translate(MAT);
    //MAT.setRotationAxisRadians(1.570796,core::vector3df(0,0,1));
    cube1.rotate(MAT);
    scene.subtract(cube1);

    polyfold cube = make_poly_cube(128,512,256);
    MAT.setTranslation(core::vector3df(-128,384,0));
    cube.translate(MAT);
    cube.rotate(MAT);
    scene.subtract(cube);

    cube = make_poly_cube(768,768,512);
    MAT.setTranslation(core::vector3df(-256,128,-384));
    cube.translate(MAT);
    cube.rotate(MAT);
    scene.subtract(cube);

    cube = make_poly_cube(768,128,128);
    MAT.setTranslation(core::vector3df(384,128,-384));
    cube.translate(MAT);
    cube.rotate(MAT);
    scene.add(cube);

    cube = make_poly_cube(768,128,128);
    MAT.setTranslation(core::vector3df(-256,128,-384));
    cube.translate(MAT);
    cube.rotate(MAT);
    scene.add(cube);

    cube = make_poly_cube(256,256,1024);
    MAT.setTranslation(core::vector3df(-512,128,-384));
    cube.translate(MAT);
    cube.rotate(MAT);
    scene.subtract(cube);
*/

/*

     cube = make_poly_cube(128,128,1024);
    MAT.setTranslation(core::vector3df(192,0,-256));
    cube.translate(MAT);
    cube.rotate(MAT);
    scene.subtract(cube);

    cube = make_poly_cube(128,1024,128);
    MAT.setTranslation(core::vector3df(-256,0,192));
    cube.translate(MAT);
    cube.rotate(MAT);
    scene.subtract(cube);

    cube = make_poly_cube(128,128,1024);
    MAT.setTranslation(core::vector3df(128,128,-256));
    cube.translate(MAT);
    cube.rotate(MAT);
    scene.subtract(cube);
*/

/*

     cube = make_poly_cube(128,512,512);
    MAT.setTranslation(core::vector3df(0,0,0));
    cube.translate(MAT);
    scene.subtract(cube);

     cube = make_poly_cube(128,512,512);
    MAT.setTranslation(core::vector3df(-512,0,0));
    cube.translate(MAT);
    scene.subtract(cube);

       cube = make_poly_cube(128,512,512);
    MAT.setTranslation(core::vector3df(-256,0,0));
    cube.translate(MAT);
    scene.add(cube);
*/

/*
 cube = make_cylinder(128,256,16);
    MAT.setTranslation(core::vector3df(0,0,0));
    cube.translate(MAT);
    //scene.subtract(cube);

    cube2 = make_poly_cube(128,512,512);
    MAT.setTranslation(core::vector3df(-256,0,0));
    cube2.translate(MAT);
    scene.subtract(cube2);

    cube = make_cylinder(128,256,16);
    MAT.setTranslation(core::vector3df(0,0,512));
    cube.translate(MAT);
    scene.subtract(cube);

    cube = make_cylinder(128,192,16);
    MAT.setTranslation(core::vector3df(0,0,512));
    cube.translate(MAT);
    scene.add(cube);

    cube = make_poly_cube(128,384,512);
    MAT.setTranslation(core::vector3df(-192,0,0));
    cube.translate(MAT);
    scene.add(cube);

    cube = make_cylinder(128,192,16);
    MAT.setTranslation(core::vector3df(0,0,0));
    cube.translate(MAT);
    //scene.add(cube);

    cube = make_poly_cube(128,512,128);
    MAT.setTranslation(core::vector3df(-256,0,384));
    cube.translate(MAT);
    scene.subtract(cube);
*/

/*
   cube = make_cylinder(64,256,24);
    MAT.setTranslation(core::vector3df(0,0,0));
    cube.translate(MAT);
    scene.add(cube);

    cube = make_cylinder(64,256,16);
    MAT.setTranslation(core::vector3df(0,0,512));
    cube.translate(MAT);
    //scene.subtract(cube);

    cube = make_poly_cube(64,512,512);
    MAT.setTranslation(core::vector3df(-256,0,0));
    cube.translate(MAT);
    scene.subtract(cube);

    cube = make_cylinder(64,224,24);
    MAT.setTranslation(core::vector3df(0,0,0));
    cube.translate(MAT);
    scene.subtract(cube);

   scene.rebuild_geometry();
   */

/*
    pf = scene.get_total_geometry();
    scene.elements.clear();

    scene.add(pf);


    pf2 = pf;
    //MAT.setTranslation(core::vector3df(-256,0,128));
    MAT.setRotationAxisRadians(3.141592653,core::vector3df(0,1,0));
    pf2.rotate(MAT);
    MAT.setTranslation(core::vector3df(0,0,-480));
    pf2.translate(MAT);
    //scene.add(pf2);

    pf2 = pf;
    MAT.setRotationAxisRadians(1.570796326,core::vector3df(0,1,0));
    pf2.rotate(MAT);
    MAT.setTranslation(core::vector3df(240,0,-240));
    pf2.translate(MAT);
    //scene.add(pf2);

    pf2 = pf;
    MAT.setRotationAxisRadians(-1.570796326,core::vector3df(0,1,0));
    pf2.rotate(MAT);
    MAT.setTranslation(core::vector3df(-240,0,-240));
    pf2.translate(MAT);
    //scene.add(pf2);


    cube = make_poly_cube(64,32,512);
    MAT.setTranslation(core::vector3df(-256,0,-496));
    cube.translate(MAT);
    //scene.add(cube);

    cube = make_poly_cube(64,32,512);
    MAT.setTranslation(core::vector3df(224,0,-496));
    cube.translate(MAT);
    //scene.add(cube);

    cube = make_poly_cube(64,512,32);
    MAT.setTranslation(core::vector3df(-256,0,-496));
    cube.translate(MAT);
    //scene.add(cube);

    cube = make_poly_cube(64,512,32);
    MAT.setTranslation(core::vector3df(-256,0,-16));
    cube.translate(MAT);
    //scene.add(cube);

    scene.rebuild_geometry();
*/
