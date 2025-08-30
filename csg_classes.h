#ifndef _CSG_CLASSES_H_
#define _CSG_CLASSES_H_

#include "Reflection.h"
#include "BVH.h"
#include "soa.h"

using namespace irr;

class polyfold;


class canonical_brush
{
public:
    int n_quads = 0;
    int length = 0;
    int height = 0;

    std::vector<u16> vertices;
    std::vector<aligned_vec3> uvs;
    std::vector<int> face_ref;
    void visualize(const polyfold*, LineHolder&) const;
    void initialize(int n);
    void init_quad(int n, polyfold* pf, int f, int v0, int v1, int v2, int v3);
    void dump_quad(polyfold* pf, int n);
    int get_real_length(polyfold* pf);
    int get_real_height(polyfold* pf);
    void layout_uvs(f32 len, f32 height);
    void layout_uvs_texture(f32 len, f32 height);
    bool barycentric(vector3df p, vector3df a, vector3df b, vector3df c, f32& u, f32& v, f32& w);
    bool map_point(polyfold* pf, int face_no, vector3df p, aligned_vec3& uv);
    bool map_point(polyfold* pf, int face_no, vector3df p, vector2df& uv);
};

class poly_vert
{
    public:
    irr::core::vector3df V;
    poly_vert() {}
    poly_vert(f32 x, f32 y, f32 z)
    {
        V.X = x;
        V.Y = y;
        V.Z = z;
    }
    poly_vert(core::vector3df v)
    {
        V.X = v.X;
        V.Y = v.Y;
        V.Z = v.Z;
    }

    template<typename T>
    void grow(T* obj) const
    {
        obj->aabbMin[0] = fmin(obj->aabbMin[0], V.X);
        obj->aabbMin[1] = fmin(obj->aabbMin[1], V.Y);
        obj->aabbMin[2] = fmin(obj->aabbMin[2], V.Z);

        obj->aabbMax[0] = fmax(obj->aabbMax[0], V.X);
        obj->aabbMax[1] = fmax(obj->aabbMax[1], V.Y);
        obj->aabbMax[2] = fmax(obj->aabbMax[2], V.Z);
    }
    core::vector3df position() const { return V; }

    REFLECT()
};

#define LOOP_INNER 0
#define LOOP_OUTER 1
#define LOOP_SOLID 0
#define LOOP_HOLLOW 1
#define LOOP_DIRECTION_SOLID 0
#define LOOP_DIRECTION_OPEN 1
#define LOOP_DIRECTION_UNDEF -1
#define LOOP_GHOST_SOLID 2
#define LOOP_GHOST_HOLLOW 3
#define LOOP_UNDEF -1
#define TOP_CONCAVE 0
#define TOP_CONVEX 1
#define TOP_UNDEF -1
#define TOP_FRONT 0
#define TOP_BEHIND 1

#define EDGE_UNDEF -1
#define EDGE_CONCAVE 0
#define EDGE_CONVEX 1
#define EDGE_SIMPLE 2
#define EDGE_COMPLEX 3

#define GEO_ADD 0
#define GEO_SUBTRACT 1

#define GEO_SOLID 0
#define GEO_EMPTY 1
#define GEO_RED 2
#define GEO_SEMISOLID 3
#define GEO_NONSOLID 4

class poly_loop
{
    public:

    std::vector<int> vertices;
    int direction = LOOP_DIRECTION_UNDEF;
    int type=0;
    int topo_group=-1;
    int depth=0;
    char flags=0;   //a temporary variable

    f32 min_x=0;
    f32 max_x=0;
    f32 min_z=0;
    f32 max_z=0;

    void reverse()
    {
        std::vector<int> tvec;
        for(int j = this->vertices.size()-1; j>=0;j--)
        {
            tvec.push_back(this->vertices[j]);
        }
        this->vertices = tvec;
    }

    void copy_properties(const poly_loop& other)
    {
        direction = other.direction;
        type = other.type;
        topo_group = other.topo_group;
        depth = other.depth;
        min_x = other.min_x;
        max_x = other.max_x;
        min_z = other.min_z;
        max_z = other.max_z;
    }

    REFLECT()
};

enum
{
    SURFACE_GROUP_STANDARD = 0,
    SURFACE_GROUP_CYLINDER,
    SURFACE_GROUP_SPHERE,
    SURFACE_GROUP_DOME,
    SURFACE_GROUP_CANONICAL,
    SURFACE_GROUP_CUSTOM_UVS_BRUSH,
    SURFACE_GROUP_CUSTOM_UVS_GEOMETRY
};

struct point_texcoord
{
    int vertex;
    core::vector2df texcoord;
    REFLECT()
};

class surface_group
{
public:
    int type;
    core::vector3df point = core::vector3df(0,0,0);
    core::vector3df vec = core::vector3df(0,0,0);
    core::vector3df vec1 = core::vector3df(0,0,0);
    //std::vector<core::vector2df> texcoords;
    std::vector<point_texcoord> texcoords;
    canonical_brush c_brush;
    int height;
    int radius;
    //int n_columns;
    //int n_rows;
    REFLECT()
};

struct face_index
{
    int brush;
    int face;
};

class poly_face
{
    public:
    std::vector<int> edges;
    std::vector<int> vertices;
    std::vector<poly_loop> loops;
    core::vector3df m_normal = core::vector3df(0,0,0);
    core::vector3df m_center = core::vector3df(0,0,0); // not reflected
    core::vector3df m_tangent = core::vector3df(0, 0, 0); //not reflected
    core::rectf bbox2d; //not reflected

    int face_id = 0;
    int element_id = 0;
    int surface_group=0;
    int surface_no = 0;

    core::vector3df uv_origin = core::vector3df(0,0,0);
    bool bFlippedNormal=false; //To be removed. Only retained for compatability
    int topo_group = 0;

    int row = 0;    //for cylinders and spheres
    int column = 0; 

    //**any new variables may need to be manually added to copy_properties below**

    core::matrix4 uv_mat;
    core::matrix4 uv_mat0;  //a temp variable
    bool temp_b=false;      //a temp variable

    poly_face()
    {
        uv_mat.setTranslation(core::vector3df(0,0,0));
        uv_mat.setScale(core::vector3df(1,1,1));
        uv_mat.setRotationAxisRadians(0,core::vector3df(0,0,1));
    }

    void copy_properties(const poly_face& other)
    {
        m_normal = other.m_normal;
        m_center = other.m_center;
        element_id = other.element_id;
        face_id = other.face_id;
        surface_group = other.surface_group;
        surface_no = other.surface_no;
        uv_origin = other.uv_origin;
        uv_mat = other.uv_mat;
        column = other.column;
        row = other.row;
    }

    core::matrix4 get2Dmat() const;

    core::vector3df getOrientingNormal() const;

    void get3DBoundingQuad(vector3df* points, int v0_idx = 0) const;

    void addVertex(int new_v)
    {
         for(int v_i : this->vertices)
            if(new_v == v_i)
                return;
         this->vertices.push_back(new_v);
    }

    void addEdge(int e)
    {
        for(int e_i : edges)
            if(e_i == e)
                return;
        edges.push_back(e);
    }

    void clear()
    {
        edges.clear();
        vertices.clear();
        loops.clear();
    }
    void flip_normal()
    {
        m_normal*=-1;
    }

    template<typename T>
    void grow(T* obj, const poly_vert* verts) const
    {
        for (int v_i : vertices)
        {
            verts[v_i].grow(obj);
        }
        
    }
    core::vector3df position(const poly_vert* verts) const {
        return m_center;
    }

    face_index get_index()
    {
        return face_index{ element_id, face_id };
    }

    REFLECT()
};

class poly_edge
{
    public:
    int v0=-1;
    int v1=-1;
    int topo_group=0;
    int conv=TOP_UNDEF;
    int p2; //a temporary variable used in loop calculations

    poly_edge(int v0_, int v1_)
    {
        v0 = v0_;
        v1 = v1_;
    }

    poly_edge(int v0_, int v1_, int top)
    {
        v0 = v0_;
        v1 = v1_;
        topo_group=top;
    }

    template<typename T>
    void grow(T* obj, const poly_vert* verts) const
    {
        verts[v0].grow(obj);
        verts[v1].grow(obj);
    }

    core::vector3df position(const poly_vert* verts) const;
};

class LineHolder
{
public:
    std::vector<core::line3df> lines;
    std::vector<core::vector3df> points;
};

class triangle
{
public:
    int A = -1;
    int B = -1;
    int C = -1;
};

class triangle_holder
{
public:
    std::vector<core::vector3df> vertices;
    std::vector<triangle> triangles;
    std::vector<int> f_index;

    int get_point_or_add(core::vector3df);
};

//template<typename T> class BVH_structure_pf;

struct poly_intersection_info
{
    std::vector<BVH_intersection_struct> faces_faces;
    std::vector<BVH_intersection_struct> faces_edges;
    std::vector<BVH_intersection_struct> edges_faces;
};

class GeometryStack;
class geo_element;
class poly_surface;

class polyfold
{
    public:

    std::vector<poly_edge> edges;
    std::vector<poly_face> faces;
    std::vector<poly_vert> vertices;
    std::vector<poly_vert> control_vertices;
    std::vector<surface_group> surface_groups;
    core::aabbox3df bbox;
    //u64 uid;

    BVH_structure_pf<poly_face> faces_BVH;
    BVH_structure_pf<poly_edge> edges_BVH;
    BVH_structure_simple<poly_vert> vertices_BVH;

    int topology = -1; //0 concave, 1 convex, -1 unknown
    int n_mesh_buffers=0;

    GeometryStack* geometry_stack = NULL;

    void draw(video::IVideoDriver* driver, const video::SMaterial material, bool);
    poly_vert getVertex(int,int,int) const;
    poly_vert getVertex(int,int) const;

    void calc_center(int f_i);
    void calc_center(poly_face& face);
    void calc_normal(int f_i);
    void calc_tangent(int f_i);

    //void generate_uids();

    void recalc_bbox();
    void recalc_bbox_and_loops();

    bool is_closed_loop(int f_i, int p_i) const;
    bool is_clockwise_loop(int f_i, int p_i) const;
    void calc_loop_bbox(int f_i, int p_i);
    void calc_loop_bbox(poly_face& face, poly_loop &loop);
    void set_loop_solid(int f_i, int p_i);
    void set_loop_open(int f_i, int p_i);

    int left_right_test(int f_i, int p_i, f32 vx, f32 vz, bool is_open = false, int = 2) const;

    bool is_point_on_face(int f_i, core::vector3df v) const;
    bool is_point_in_loop(int f_i, int p_i, core::vector3df v) const;

    bool point_is_on_edge(int, core::vector3df v );
    bool point_is_coplanar(int face, core::vector3df v );

    std::vector<int> get_connecting_edges(int edge_no) const;
    int get_edge_loop_no(int face_i, int e_i) const;
    std::vector<int> get_vert_loop_no(int face_i, int v_i) const;

    void merge_faces(); //currently unused

    int get_opposite_end(int e_i, int v_i) const;
    //========================================
    // BVH functions
    
    void build_faces_BVH();
    void build_edges_BVH();
    void build_vertices_BVH();

    void addDrawLinesEdges(LineHolder& graph) const;
    void addDrawLinesEdgesByIndex(std::vector<u16> edges_i, LineHolder& graph) const;
    void addDrawLinesFacesByIndex(std::vector<u16> faces_i, LineHolder& graph) const;

    //========================================
    // Trianglize
    void trianglize(int face_i, triangle_holder&, scene::SMeshBuffer*, LineHolder &graph, LineHolder&);

    //========================================
    // Helpful functions for clipping
    bool bisect_edge(int e_i,int v_i, int g1, int g2);
    void remove_empty_faces(std::vector<poly_surface>& surfaces);

    template<bool bAccelerate>
    void finalize_clipped_poly(const polyfold& pf, LineHolder& graph);

    //========================================
    // Classify topology of points and edges

    int classify_point(int face_i, core::vector3df v0, core::vector3df v1, LineHolder&);
    int classify_point(core::vector3df v0, LineHolder& graph);
    int classify_point_edge(int v_i, core::vector3df v0, core::vector3df v1);
    int classify_point_face(int v_i, core::vector3df point);

    template<bool bAccelerate>
        void classify_edges(LineHolder& graph);

    template<bool bAccelerate>
        void classify_edge(int edge);

    int edge_classification(int edge);

    //========================================
    //Defined in clip_poly
    bool get_point_in_loop(int face_i, int loop_i, core::vector3df& out, LineHolder& graph) const;
    bool get_facial_point(int face_i, int edge_i, int loop_i, core::vector3df & ret, LineHolder &graph) const;
    bool get_facial_point(int face_i, int loop_i, core::vector3df& ret,  LineHolder& graph) const;

    //Defined in clip2
    void sort_loops_inner(int f_i);

    //========================================
    //Defined in calc_loops
    void calc_loops(int f_i,LineHolder& graph);
    void sort_loops(int f_i);
    void do_loops(int f_i, int e_0, LineHolder& graph);
    void add_loop_from_edges(int face_i, std::vector<int> e_vec, bool is_left);
    std::vector<int> get_loop_going_left(int f_i, int v_0, int e_0, int v_1,LineHolder& graph);
    std::vector<int> get_loop_going_right(int f_i, int v_0, int e_0, int v_1,LineHolder& graph);
    int get_next_loop_edge_going_right(int f_i, int e_i, int v_i);
    int get_next_loop_edge_going_left(int f_i, int e_i, int v_i);
    std::vector<int> get_all_connected_edges(int f_i, int e_0) const;
    std::vector<int> search_connected_edges(polyfold &pf, int f_i, int e_0) const;
    void get_all_edges_from_point(int e_i, int v, std::vector<int>& ret) const;
    void get_all_edges_from_point_accelerated(int e_0, int v, std::vector<int>& ret) const;
    std::vector<int> get_edges_from_point(int f_i, int v) const;
    std::vector<int> get_edges_from_point(int f_i, int e_0, int v) const;

    bool is_inner_loop(int f_i, int p_i, int p_j);
    //std::vector<int> get_edges_from_2edges(int f_i, int e_0, int e_1);
    //std::vector<int> get_all_loop_edges(int f_i, int p_i);
    //void copy_loops(polyfold& pf, int f_i);
    
    //========================================
    //Core Functions
    int get_point_or_add(core::vector3df);
    int get_edge_or_add(int,int,int);
    int find_edge(int, int) const;
    int find_edge(int f_i, int v0, int v1) const;
    int find_edge(core::vector3df v0, core::vector3df v1) const;
    
    bool edge_exists(int,int) const;
    int find_point(core::vector3df) const;

    int find_edge_accelerated(core::vector3df v0, core::vector3df v1, std::vector<u16>& hits) const;
    int find_edge_accelerated(int, int, std::vector<u16>& hits) const;
    int find_point_accelerated(core::vector3df) const;

    //========================================
    //An important function. Eliminate all excess vertices and rebuild edges from loops only
    void reduce_edges_vertices();
    void reduce_edges_vertices_accelerated(const polyfold& pf, LineHolder& graph);
    void deduplicate_edges_vertices_accelerated();

    template<bool bAccelerate>
    void rebuild_faces_from_loops(const polyfold& pf);

    template<bool bAccelerate>
    void rebuild_loops_copy_verts(const polyfold& pf);

    void simplify_edges(); //currently unused

    bool getSurfaceVectors(int f_i,core::vector3df &a, core::vector3df &b);
    surface_group* getFaceSurfaceGroup(int f_i);
    int getFaceSurfaceGroupNo(int f_i);

    //========================================
    //Topology functions
    void make_convex();
    void make_convex2(LineHolder& graph);
    void make_concave();
    void recalc_faces();

    void rotate(core::matrix4 MAT);
    void translate(core::matrix4 MAT);

    //========================================
    // Total Geometry functions 

    std::vector<int> getSurfaceFromFace(int);
    std::vector<int> getConnectedSurfaceFromFace(int);
    face_index index_face(int f_i);

    void operator=(const polyfold& other)
    {
        edges = other.edges;
        faces = other.faces;
        surface_groups = other.surface_groups;
        vertices = other.vertices;
        control_vertices = other.control_vertices;
        topology = other.topology;
        bbox = other.bbox;
        faces_BVH = other.faces_BVH;
        //uid = other.uid;
    }

    REFLECT()
};

bool BoxIntersectsWithBox(const core::aabbox3d<f32>& A, const core::aabbox3d<f32>& B);

void combine_polyfolds_accelerated(const std::vector<polyfold*>& polies, polyfold& res);
void combine_polyfolds_linear(const std::vector<polyfold*>& polies, polyfold& res);
void combine_polyfolds(const std::vector<polyfold*>& polies, polyfold& res);


bool line_intersects_plane(const core::plane3df&, const core::vector3df& v0, const core::vector3df& v1, core::vector3df& ipoint);
void sort_inline_vertices(polyfold& pf);


#endif
