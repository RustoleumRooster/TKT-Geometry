
#include <irrlicht.h>
#include "edit_classes.h"
#include "reflection.h"
#include "create_primitives.h"
#include "geometry_scene.h"

geometry_scene* GeometryFactory::g_scene = NULL;
irr::gui::IGUIEnvironment* GeometryFactory::env = NULL;
GeometryFactory::Cube GeometryFactory::cube{128,128,128};
GeometryFactory::Cylinder GeometryFactory::cylinder{128,64,16};
GeometryFactory::Sphere GeometryFactory::sphere{0,128,8,8,true};
GeometryFactory::Plane GeometryFactory::plane{256,256};
GeometryFactory::Cone GeometryFactory::cone{256,128,8};

void EditCubeWindow::click_OK()
{
    GeometryFactory::CreateCube(this);
    this->remove();
}

void EditCylinderWindow::click_OK()
{
    GeometryFactory::CreateCylinder(this);
    this->remove();
}

void EditSphereWindow::click_OK()
{
    GeometryFactory::CreateSphere(this);
    this->remove();
}

void EditPlaneWindow::click_OK()
{
    GeometryFactory::CreatePlane(this);
    this->remove();
}

void EditConeWindow::click_OK()
{
    GeometryFactory::CreateCone(this);
    this->remove();
}


void GeometryFactory::CreatePlane(EditPlaneWindow* win)
{
    win->write(&plane);

    core::matrix4 M;
    M.setTranslation(g_scene->geoNode()->elements[0].brush.vertices[0].V);
    g_scene->geoNode()->elements[0].brush = make_poly_plane(plane.length,plane.width);
    g_scene->geoNode()->elements[0].brush.translate(M);
}

void GeometryFactory::CreateCube(EditCubeWindow* win)
{
    win->write(&cube);

    core::matrix4 M;
    M.setTranslation(g_scene->geoNode()->elements[0].brush.vertices[0].V);
    g_scene->geoNode()->elements[0].brush = make_poly_cube(cube.height,cube.length,cube.width);
    g_scene->geoNode()->elements[0].brush.translate(M);
}

void GeometryFactory::CreateCylinder(EditCylinderWindow* win)
{
    win->write(&cylinder);

    core::matrix4 M;
    
    g_scene->geoNode()->elements[0].brush = make_cylinder(cylinder.height,cylinder.radius,cylinder.nSides,cylinder.radius_type.value);
    if (cylinder.align_to_side)
    {
        M.setRotationRadians(vector3df(0,PI / cylinder.nSides,0));
        g_scene->geoNode()->elements[0].brush.rotate(M);
    }
    M.setTranslation(g_scene->geoNode()->elements[0].brush.vertices[0].V);
    g_scene->geoNode()->elements[0].brush.translate(M);
}

void GeometryFactory::CreateSphere(EditSphereWindow* win)
{
    win->write(&sphere);

    core::matrix4 M;
    M.setTranslation(g_scene->geoNode()->elements[0].brush.vertices[0].V);
    g_scene->geoNode()->elements[0].brush = make_sphere(sphere.radius,sphere.nSides,sphere.nZenSides,sphere.simplify);
    g_scene->geoNode()->elements[0].brush.translate(M);
}


void GeometryFactory::CreateCone(EditConeWindow* win)
{
    win->write(&cone);

    core::matrix4 M;
    M.setTranslation(g_scene->geoNode()->elements[0].brush.vertices[0].V);
    g_scene->geoNode()->elements[0].brush = make_cone(cone.height,cone.radius,cone.nSides);
    g_scene->geoNode()->elements[0].brush.translate(M);
}


void GeometryFactory::MakePlaneWindow()
{
    EditPlaneWindow* win = new EditPlaneWindow(env,env->getRootGUIElement(),-1,core::rect<s32>(140,200,16+196,64+196));
    win->setText(L"Make a plane");

    reflect::TypeDescriptor_Struct* typeDesc = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<Plane>::get();
    win->Show(typeDesc, &plane);
    win->drop();
}

void GeometryFactory::MakeCubeWindow()
{
    EditCubeWindow* win = new EditCubeWindow(env,env->getRootGUIElement(),-1,core::rect<s32>(140,200,16+196,64+196));
    win->setText(L"Make a cube");

    reflect::TypeDescriptor_Struct* typeDesc = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<Cube>::get();
    win->Show(typeDesc, &cube);
    win->drop();
}

void GeometryFactory::MakeCylinderWindow()
{
    EditCylinderWindow* win = new EditCylinderWindow(env,env->getRootGUIElement(),-1,core::rect<s32>(140,200,16+196,64+196));
    win->setText(L"Make a cylinder");

    reflect::TypeDescriptor_Struct* typeDesc = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<Cylinder>::get();
    win->Show(typeDesc, &cylinder);
    win->drop();
}

void GeometryFactory::MakeSphereWindow()
{
    EditSphereWindow* win = new EditSphereWindow(env,env->getRootGUIElement(),-1,core::rect<s32>(140,200,16+196,64+196));
    win->setText(L"Make a sphere");

    reflect::TypeDescriptor_Struct* typeDesc = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<Sphere>::get();
    win->Show(typeDesc, &sphere);
    win->drop();
}

void GeometryFactory::MakeConeWindow()
{
    EditConeWindow* win = new EditConeWindow(env,env->getRootGUIElement(),-1,core::rect<s32>(140,200,16+196,64+196));
    win->setText(L"Make a cone");

    reflect::TypeDescriptor_Struct* typeDesc = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<Cone>::get();
    win->Show(typeDesc, &cone);
    win->drop();
}

REFLECT_MULTI_STRUCT_BEGIN(GeometryFactory::SphereOptions)
    REFLECT_MULTI_STRUCT_LABEL("sphere")
    REFLECT_MULTI_STRUCT_LABEL("dome")
REFLECT_MULTI_STRUCT_END()

REFLECT_MULTI_STRUCT_BEGIN(GeometryFactory::RadiusOptions)
    REFLECT_MULTI_STRUCT_LABEL("to edge")
    REFLECT_MULTI_STRUCT_LABEL("to corner")
REFLECT_MULTI_STRUCT_END()

REFLECT_STRUCT_BEGIN(GeometryFactory::Cube)
    REFLECT_STRUCT_MEMBER(height)
    REFLECT_STRUCT_MEMBER(width)
    REFLECT_STRUCT_MEMBER(length)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(GeometryFactory::Cylinder)
    REFLECT_STRUCT_MEMBER(height)
    REFLECT_STRUCT_MEMBER(radius)
    REFLECT_STRUCT_MEMBER(nSides)
    REFLECT_STRUCT_MEMBER(radius_type)
    REFLECT_STRUCT_MEMBER(align_to_side)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(GeometryFactory::Sphere)
    REFLECT_STRUCT_MEMBER(type)
    REFLECT_STRUCT_MEMBER(radius)
    REFLECT_STRUCT_MEMBER(nSides)
    REFLECT_STRUCT_MEMBER(nZenSides)
    REFLECT_STRUCT_MEMBER(simplify)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(GeometryFactory::Plane)
    REFLECT_STRUCT_MEMBER(length)
    REFLECT_STRUCT_MEMBER(width)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(GeometryFactory::Cone)
    REFLECT_STRUCT_MEMBER(height)
    REFLECT_STRUCT_MEMBER(radius)
    REFLECT_STRUCT_MEMBER(nSides)
REFLECT_STRUCT_END()
