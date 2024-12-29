#ifndef _CREATE_PRIMITIVES_H_
#define _CREATE_PRIMITIVES_H_

#include "edit_classes.h"
#include <irrlicht.h>

using namespace irr;

class GeometryFactory;


class EditCubeWindow : public  EditWindow
{
public:
    EditCubeWindow(gui::IGUIEnvironment* env, gui::IGUIElement* parent, s32 id,core::rect<s32> rect)
    : EditWindow(env,parent,NULL,id,rect){}

private:
    virtual void click_OK();
};

class EditCylinderWindow : public  EditWindow
{
public:
    EditCylinderWindow(gui::IGUIEnvironment* env, gui::IGUIElement* parent,s32 id,core::rect<s32> rect)
    : EditWindow(env,parent,NULL,id,rect){}

private:
    virtual void click_OK();
};

class EditSphereWindow : public  EditWindow
{
public:
    EditSphereWindow(gui::IGUIEnvironment* env, gui::IGUIElement* parent,s32 id,core::rect<s32> rect)
    : EditWindow(env,parent,NULL,id,rect){}

private:
    virtual void click_OK();
};

class EditPlaneWindow : public  EditWindow
{
public:
    EditPlaneWindow(gui::IGUIEnvironment* env, gui::IGUIElement* parent,s32 id,core::rect<s32> rect)
    : EditWindow(env,parent,NULL,id,rect){}

private:
    virtual void click_OK();
};

class EditConeWindow : public  EditWindow
{
public:
    EditConeWindow(gui::IGUIEnvironment* env, gui::IGUIElement* parent,s32 id,core::rect<s32> rect)
    : EditWindow(env,parent,NULL,id,rect){}

private:
    virtual void click_OK();
};


class geometry_scene;

class GeometryFactory
{

    struct SphereOptions {
        int value;
        REFLECT_MULTI_CHOICE()
        };

    struct RadiusOptions {
        int value;
        REFLECT_MULTI_CHOICE()
    };

    struct Cube{
        int height;
        int width;
        int length;
        REFLECT()
        };

    struct Cylinder {
        int height;
        int radius;
        int nSides;
        RadiusOptions radius_type;
        bool align_to_side;
        REFLECT()
        };

    struct Sphere {
        SphereOptions type;
        int radius;
        int nSides;
        int nZenSides;
        bool simplify;
        REFLECT()
        };

    struct Plane {
        int length;
        int width;
        REFLECT()
        };

    struct Cone
        {
        int height;
        int radius;
        int nSides;
        REFLECT()
        };

public:

    static Cube cube;
    static Cylinder cylinder;
    static Sphere sphere;
    static Plane plane;
    static Cone cone;

    static gui::IGUIEnvironment* env;
    static geometry_scene* g_scene;

    static void initialize(gui::IGUIEnvironment* env_, geometry_scene* g_scene_)
    {
        env=env_;
        g_scene= g_scene_;
    }

    static void set_scene(geometry_scene* g_scene_)
    {
        g_scene = g_scene_;
    }

    static void MakeCubeWindow();
    static void MakeCylinderWindow();
    static void MakeSphereWindow();
    static void MakePlaneWindow();
    static void MakeConeWindow();

    static void CreateCube(EditCubeWindow*);
    static void CreateCylinder(EditCylinderWindow*);
    static void CreateSphere(EditSphereWindow*);
    static void CreatePlane(EditPlaneWindow*);
    static void CreateCone(EditConeWindow*);
};

#endif
