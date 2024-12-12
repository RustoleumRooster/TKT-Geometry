#ifndef _UTILS_H_
#define _UTILS_H_
#include <irrlicht.h>
#include "Reflection.h"
#include "reflected_nodes.h"

using namespace irr;

class geometry_scene;
class LineHolder;
class polyfold;


struct camera_info_struct
{
    bool orthogonal;
    core::vector3df position;
    core::vector3df target;
    //core::vector3df upvec;
    core::matrix4 projM;

    REFLECT()
};

struct camera_panel_3D_info_struct
{
    camera_info_struct my_camera;
    REFLECT()
};

struct camera_panel_2D_info_struct
{
    camera_info_struct my_camera;
    int axis;
    core::dimension2du viewSize;
    REFLECT()
};

struct GUI_state_struct
{
    camera_panel_3D_info_struct tl;
    camera_panel_2D_info_struct tr;
    camera_panel_2D_info_struct bl;
    camera_panel_2D_info_struct br;
    bool dynamicLight;
    int viewStyle;

    REFLECT()
};

struct Vertex_Struct
{
    reflect::vector3 pos;
    reflect::vector2 tex_coords_0;
    reflect::vector2 tex_coords_1;

    REFLECT()
};

struct Vertex_Buffer_Struct
{
    std::vector<Vertex_Struct> vertices;
    std::vector<u16> indices;

    REFLECT()
};

struct Face_Bounding_Rect_Struct
{
    reflect::vector3 v0;
    reflect::vector3 v1;
    reflect::vector3 v2;
    reflect::vector3 v3;

    REFLECT()
};

struct Face_Info_Struct
{
    reflect::vector3 normal;
    reflect::vector3 tangent;
    Face_Bounding_Rect_Struct bounding_rect;

    u16 lightmap_no;
    REFLECT()
};

struct bounding_quad
{
    core::vector3df verts[4];
};

struct LightMaps_Info_Struct
{
    u16 size;
    u16 type;
    std::vector<int> faces;
    std::vector<reflect::vector2u> lightmap_block_UL;
    std::vector<reflect::vector2u> lightmap_block_BR;

    //not reflected
    std::vector<u16> first_triangle;
    std::vector<u16> n_triangles;
    std::vector<bounding_quad> quads;

    REFLECT()
};

struct Model_Struct
{
    std::vector<Vertex_Buffer_Struct> vertex_buffers;
    std::vector<Face_Info_Struct> faces_info;
    std::vector<LightMaps_Info_Struct> lightmaps_info;

    REFLECT()
};

u64 random_number();

bool WriteGUIStateToFile(io::path fname);
bool ReadGUIStateFromFile(io::path fname);

void MakeCircleImages(video::IVideoDriver* driver);
void addDrawLines(polyfold& pf, LineHolder& graph,LineHolder& graph1,LineHolder& graph2);
irr::video::IImage* makeCircleImage(video::IVideoDriver* driver,int width, f32 radius, video::SColor);
irr::video::IImage* makeSolidColorImage(video::IVideoDriver* driver, video::SColor col);
video::SColor makeUniqueColor();

void do_test_loop(polyfold& pf, int f_i, int p_i, LineHolder& graph, LineHolder& graph2);

irr::video::IImage* makeAlphaImage(video::IVideoDriver* driver,video::ITexture* texture, int alpha);

class Open_Geometry_File : public irr::IEventReceiver
{
public:
	Open_Geometry_File(geometry_scene* gs);
	~Open_Geometry_File();

	virtual bool OnEvent(const SEvent& event);

    static void LoadProject(geometry_scene* gs, io::path folder);

private:
	geometry_scene* g_scene = NULL;
	io::IFileSystem* FileSystem = NULL;
};


class Save_Geometry_File : public irr::IEventReceiver
{
public:
	Save_Geometry_File(geometry_scene* gs, bool show = true );
	~Save_Geometry_File();

	virtual bool OnEvent(const SEvent& event);
    bool Export(io::path fname);

private:

    bool export_model(io::path fname);
    bool export_model_2(io::path fname);
    bool WriteModelTextures(std::string fname);

	geometry_scene* g_scene = NULL;
	io::IFileSystem* FileSystem = NULL;
};

void generateBitmapImage(unsigned char* image, int height, int width, const char* imageFileName);

#endif
