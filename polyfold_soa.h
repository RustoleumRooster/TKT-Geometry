#pragma once

#include <irrlicht.h>
#include "csg_classes.h"
#include "tolerances.h"

using namespace std;
struct soa_index;
struct polyfold_soa;
struct face_iterator;
struct face_iterator_edges;
struct face_iterator_loop_vertices;
struct face_iterator_edges_loops;

struct index_pair
{
    u32 offset;
    u32 len;
};

struct index_triple
{
    u32 offset;
    u32 len;
    u32 data_offset;
};

struct index_5
{
    u32 offset;
    u32 len;
    u32 data_offset;
    u32 data_offset1;
    u32 data_offset2;
};

struct loop_aabb
{
    f32 min_x = 0;
    f32 max_x = 0;
    f32 min_z = 0;
    f32 max_z = 0;
};

struct edge_info_struct
{
    char topo_group;
    char convexivity;
    char p2;
    char unused;

    void operator=(const poly_edge& e)
    {
        topo_group = e.topo_group;
        convexivity = e.conv;
        p2 = 0;
        unused = 0;
    }
};

struct loop_info_struct
{
    loop_aabb aabb;
    char direction = LOOP_DIRECTION_UNDEF;
    char type = 0;
    char topo_group = -1;
    char depth = 0;

    void operator=(const poly_loop& p)
    {
        aabb.min_x = p.min_x;
        aabb.max_x = p.max_x;
        aabb.min_z = p.min_z;
        aabb.max_z = p.max_z;
        direction = p.direction;
        type = p.type;
        topo_group = p.topo_group;
        depth = p.depth;
    }
};

struct poly_edge2
{
    u32 v0;
    u32 v1;

    void operator=(const poly_edge& other) {
        v0 = other.v0;
        v1 = other.v1;
    }

    void operator+=(u32 offset)
    {
        v0 += offset;
        v1 += offset;
    }
};

struct soa_index_iterable
{
    vector<u32> offset;
    vector<u32> len;
    u32 total_len = 0;

    u32 implied_size() const { return total_len; }

    void resize(u32 newsize)
    {
        offset.resize(newsize);
        len.resize(newsize);
    }
};

struct soa_index
{
    vector<u32> offset;
    vector<u32> len;

    void resize(u32 newsize)
    {
        offset.resize(newsize);
        len.resize(newsize);
    }
};

struct soa_single_index
{
    soa_index idx0;
    u32 total_len = 0;

    u32 implied_size() const { return total_len; }
};

struct soa_double_index
{
    soa_index idx0;
    soa_index idx1;
    u32 total_len = 0;

    u32 implied_size() const { return total_len; }
};

struct soa_triple_index
{
    soa_index idx0;
    soa_index idx1;
    soa_index idx2;
    u32 total_len = 0;

    u32 implied_size() const { return total_len; }
};

struct polyfold_soa_layout
{
    u32 pf_n;
    soa_index_iterable face_index;
    soa_index_iterable loop_index;
};

template<typename T>
struct single_indexed_field
{
    soa_single_index index;
    vector<T> data;

    T& operator[](u32 i) { return data[i]; }
    const T& operator[](u32 i)  const { return data[i]; }

    void resize(const soa_single_index& new_index);
};

template<typename T>
struct double_indexed_field
{
    soa_double_index index;
    vector<T> data;

    T& operator[](u32 i) { return data[i]; }
    const T& operator[](u32 i)  const { return data[i]; }

    void resize(const polyfold_soa& soa, const soa_double_index& new_index);
};

template<typename T>
struct triple_indexed_field
{
    soa_triple_index index;
    vector<T> data;

    T& operator[](u32 i)  { return data[i]; }
    const T& operator[](u32 i)  const { return data[i]; }

    void resize(const polyfold_soa& soa, const polyfold_soa_layout& new_layout, const soa_triple_index& new_data_index);
};

#define FILL_DECLARATIONS() \
template<typename T, typename Z> \
    void fill_indexed_field_convert(const polyfold_soa& soa, index_type<T>& field, const polyfold& pf, size_t vector_offset); \
template<typename T, typename Z> \
    void fill_non_indexed_field(const polyfold_soa& soa, vector<T>& field, const polyfold& pf, size_t data_offset); \
template<typename T> \
    void fill_non_indexed_field(const polyfold_soa& soa, vector<T>& field, const polyfold& pf, size_t data_offset); \
template<typename T> \
    void fill_indexed_field(const polyfold_soa& soa, index_type<T>& field, const polyfold& pf, size_t vector_offset); \
template<typename T> \
    void size_non_indexed_field(const polyfold_soa& soa, vector<T>& field); \



/*
  template<typename T> \
    void combine_non_indexed_field(const polyfold_soa& soa, vector<T>& field, const vector<polyfold_soa&> pf_vec, size_t data_offset); \
template<typename T> \
    void combine_indexed_field(const polyfold_soa& soa, index_type<T>& field, const vector<polyfold_soa&> pf_vec, size_t vector_offset); 
    */

class polyfold_soa
{
public:
//private:

    vector<poly_vert> vertices;
    vector<poly_edge2> edges;
    vector<edge_info_struct> edge_info;

    struct face_struct
    {
        u32 n;
        vector<vector3df> normal;
        vector<matrix4> mat2D;
        single_indexed_field<u32> edges;
        single_indexed_field<u32> vertices;

        template<typename T>
        using index_type = single_indexed_field<T>;

        FILL_DECLARATIONS();

    } face;

    struct loop_struct
    {
        soa_index_iterable index;

        vector<loop_info_struct> loop_info;
        double_indexed_field<u32> vertices;
        
        template<typename T>
        using index_type = double_indexed_field<T>;

        FILL_DECLARATIONS();

        template<typename T>
        void fill_indexed_field_closed_loop(const polyfold_soa& soa, index_type<T>& field, const polyfold& pf, size_t vector_offset);

    } loop;

    void fill_main_indexes(const polyfold& pf);

    void resize_main_indexes(const vector<polyfold_soa*>& pf_vec);

    template<typename T>
    void combine_single_indexed_field(const vector<polyfold_soa*> pf_vec, size_t data_offset, size_t vec_offset);

    template<typename T>
    void combine_non_indexed_field(const vector<polyfold_soa*> pf_vec, size_t data_offset);

    template<typename T>
    void combine_indexed_field_refdata(const vector<polyfold_soa*> pf_vec, size_t data_offset, size_t vec_offset);

    template<typename T,typename Z>
    void offset_into(const vector<polyfold_soa*> pf_vec, size_t data_offset, size_t vec_offset);

    template<typename T>
    void combine_double_indexed_field(const vector<polyfold_soa*> pf_vec, size_t data_offset, size_t vec_offset);

    void resize_single_indexed_field(const soa_single_index& index, size_t vec_offset);

public:

    //===========================================================
    //  CSG FUNCTIONS
    //

    core::vector3df getOrientingNormal(u32 f_i) const
    {
        if (face.normal[f_i].Y < 0)
            return face.normal[f_i] * -1;
        else
            return face.normal[f_i];
    }

    void get2Dmat(u32 f_i, matrix4& mat) const
    {
        mat.buildRotateFromTo(getOrientingNormal(f_i), core::vector3df(0, 1, 0));
    }

    int left_right_test(u32 f_i, u32 p_i, vector2df r,  int n_tries = 2) const;

    inline bool is_closed_loop(u32 f_i, u32 p_i) const;
    inline bool is_point_in_loop(u32 f_i, u32 p_i, core::vector3df v) const;
    inline bool is_point_on_face(u32 f_i, core::vector3df v) const;


    //==========================================================
    //
    //

    void combine(const vector<polyfold_soa*> pf_vec);

    void fill(const polyfold& pf_vec);

    inline face_iterator_edges faces_edges();
    inline face_iterator_edges_loops faces_edges_loops();
    inline face_iterator_loop_vertices faces_loops();

    friend struct loop_iterator_base;
    friend struct polyfold_iterator_base;

    friend struct loop_iterator;
    friend struct face_iterator;

    friend struct loop_iterator_loop_vertices;
    friend struct face_iterator_loop_vertices;
};

struct loop_iterator_base
{
    u32 f_i;
    u32 idx;
    u32 len;
    u32 i = 0;

    const polyfold_soa& src;

    loop_iterator_base(const polyfold_soa& src) : src(src), idx(0), len(0), i(0)
    {}

    u32 get_index() const { return idx + i; }
    
    bool end()
    {
        return i == len;
    }
};

struct loop_iterator : loop_iterator_base
{
    loop_iterator(const polyfold_soa& src) : loop_iterator_base(src) {}

    void operator=(index_pair idx_pair)
    {
        idx = idx_pair.offset;
        len = idx_pair.len;
        i = 0;
    }

    void operator++()
    {
        i++;
    }
};

struct loop_iterator_loop_vertices : loop_iterator_base
{
    u32 data_index;
    u32 p_data_index;
    u32 n_verts;

    loop_iterator_loop_vertices(const polyfold_soa& src) : loop_iterator_base(src), data_index(0), n_verts(0), p_data_index(0)
    {}

    u32 get_data_index() { return data_index + src.loop.vertices.index.idx1.offset[get_index()]; }

    inline vector3df get_vertex(u32 j);

    inline void begin();

    bool has_verts() const
    {
        return n_verts > 0;
    }

    u32 n_vertices() const
    {
        return n_verts - 1;
    }

    inline bool operator++();

    // CSG FUNCTIONS

    int left_right_test(vector2df r, int n_tries = 2) const
    {
        return src.left_right_test(f_i, i, r, n_tries);
    }

    inline bool is_closed_loop() const
    {
        return src.is_closed_loop(f_i, i);
    }

    inline bool is_point_in_loop(int f_i, core::vector3df v) const
    {
        return src.is_point_in_loop(f_i, i, v);
    }

};

struct face_iterator
{
    u32 len;
    u32 i;
    u32 data_index_0;
    u32 f_n_edges;
    const polyfold_soa& src;

    face_iterator(const polyfold_soa& src, u32 len) : src(src), len(len), i(0), data_index_0(0), f_n_edges(src.face.edges.index.idx0.len[0])
    {}

    u32 get_index() const { return  i; }

    bool end()
    {
        return i == len;
    }

    vector3df normal()
    {
        return src.face.normal[get_index()];
    }

};

struct face_iterator_edges_loops : face_iterator
{
    loop_iterator_loop_vertices loop_it;

    face_iterator_edges_loops(const polyfold_soa& src, u32 len) : face_iterator(src, len), loop_it(src)
    {
        loop_it.idx = src.loop.index.offset[get_index()];
        loop_it.len = src.loop.index.len[get_index()];
        loop_it.data_index = get_data_index();
    }
    u32 n_loops()
    {
        return src.loop.index.len[get_index()];
    }

    loop_iterator_loop_vertices& loops()
    {
        loop_it.begin();
        return loop_it;
    }

    u32 get_data_index() { return src.loop.vertices.index.idx0.offset[get_index()]; }
    u32 n_edges()
    {
        return f_n_edges;
    }

    u32 edge_index(u32 e_i)
    {
        u32 e_0 = data_index_0 + e_i;
        return src.face.edges[e_0];
    }

    poly_vert edge_v0(u32 e_i)
    {
        u32 v_i = edge_index(e_i);
        u32 v = src.edges[v_i].v0;
        return src.vertices[v];
    }

    poly_vert edge_v1(u32 e_i)
    {
        u32 v_i = edge_index(e_i);
        u32 v = src.edges[v_i].v1;
        return src.vertices[v];
    }

    void operator++()
    {
        i++;
        if (i < len)
        {
            data_index_0 = src.face.edges.index.idx0.offset[get_index()];
            f_n_edges = src.face.edges.index.idx0.len[get_index()];
            loop_it.idx = src.loop.index.offset[get_index()];
            loop_it.len = src.loop.index.len[get_index()];
            loop_it.data_index = get_data_index();
        }
    }
};

struct face_iterator_edges : face_iterator
{
    face_iterator_edges(const polyfold_soa& src, u32 len) : face_iterator(src,len)
    {
    }

    u32 n_edges()
    {
        return f_n_edges;
    }

    u32 edge_index(u32 e_i)
    {
        u32 e_0 = data_index_0 + e_i;
        return src.face.edges[e_0];
    }

    poly_vert edge_v0(u32 e_i)
    {
        u32 v_i = edge_index(e_i);
        u32 v = src.edges[v_i].v0;
        return src.vertices[v];
    }

    poly_vert edge_v1(u32 e_i)
    {
        u32 v_i = edge_index(e_i);
        u32 v = src.edges[v_i].v1;
        return src.vertices[v];
    }

    void operator++()
    {
        i++;
        if (i < len)
        {
            data_index_0 = src.face.edges.index.idx0.offset[get_index()];
            f_n_edges = src.face.edges.index.idx0.len[get_index()];
        }
    }
};


struct face_iterator_loop_vertices : face_iterator
{
    loop_iterator_loop_vertices loop_it;

    face_iterator_loop_vertices::face_iterator_loop_vertices(const polyfold_soa& src, u32 n)
        : face_iterator(src, n), loop_it(src)
    {
        loop_it.f_i = get_index();
        loop_it.idx = src.loop.index.offset[get_index()];
        loop_it.len = src.loop.index.len[get_index()];
        loop_it.data_index = get_data_index();
    }

    u32 n_loops() const
    {
        return src.loop.index.len[get_index()];
    }

    loop_iterator_loop_vertices& loops()
    {
        loop_it.begin();
        return loop_it;
    }

    u32 get_data_index() const { return src.loop.vertices.index.idx0.offset[get_index()]; }

    void operator++()
    {
        i++;
        if (i < len)
        {
            loop_it.f_i = get_index();
            loop_it.idx = src.loop.index.offset[get_index()];
            loop_it.len = src.loop.index.len[get_index()];
            loop_it.data_index = get_data_index();
        }
    }

    //CSG FUNCTIONS

    bool is_point_on_face(core::vector3df v) { return src.is_point_on_face(get_index(), v); }
};

vector3df loop_iterator_loop_vertices::get_vertex(u32 j)
{
    u32 v_i = src.loop.vertices[p_data_index + j];
    return src.vertices[v_i].V;
}

inline face_iterator_edges_loops polyfold_soa::faces_edges_loops()
{
    return face_iterator_edges_loops(*this, face.n);
}

face_iterator_edges polyfold_soa::faces_edges()
{
    return face_iterator_edges(*this, face.n);
}

face_iterator_loop_vertices polyfold_soa::faces_loops()
{
    return face_iterator_loop_vertices(*this, face.n);
}

void loop_iterator_loop_vertices::begin()
{
    i = 0;
    p_data_index = data_index + src.loop.vertices.index.idx1.offset[idx];
    n_verts = src.loop.vertices.index.idx1.len[idx];
}

bool loop_iterator_loop_vertices::operator++()
{
    if (i < len)
    {
        p_data_index = data_index + src.loop.vertices.index.idx1.offset[idx + i];
        n_verts = src.loop.vertices.index.idx1.len[idx + i];

        i++;

        return true;
    }

    return false;
}
/*
bool loop_iterator_loop_vertices::is_closed_loop() const
{
    return src.loop.loop_info[get_index()].depth % 2 == 0;
}

bool loop_iterator_loop_vertices::is_point_in_loop(int f_i, core::vector3df v) const
{
    core::vector3df V = v;

    src.face.mat2D[f_i].rotateVect(V);

    f32 small_number = LOOP_BBOX_SMALL_NUMBER;
    if (V.X < src.loop.loop_info[get_index()].aabb.min_x - small_number || V.X > src.loop.loop_info[get_index()].aabb.max_x + small_number ||
        V.Z < src.loop.loop_info[get_index()].aabb.min_z - small_number || V.Z > src.loop.loop_info[get_index()].aabb.max_z + small_number)
    {
        return false;
    }

    return (this->left_right_test(vector2df(V.X,V.Z), f_i) == 1);
}

bool face_iterator_loop_vertices::is_point_on_face(core::vector3df v)
{
    int n = 0;

    for (int p_i = 0; p_i < n_loops(); p_i++)
    {
        if (this->faces[f_i].loops[p_i].vertices.size() > 0 && is_point_in_loop(f_i, p_i, v) == true)
        {
            if (this->is_closed_loop(f_i, p_i))
                n++;
            else
                n--;
        }
    }
    return n > 0;
}*/
    

bool polyfold_soa::is_closed_loop(u32 f_i, u32 p_i) const
{
    u32 p_idx = loop.index.offset[f_i] + p_i;

    return loop.loop_info[p_idx].depth % 2 == 0;
}

bool polyfold_soa::is_point_in_loop(u32 f_i, u32 p_i, core::vector3df v) const
{
    core::vector3df V = v;

    face.mat2D[f_i].rotateVect(V);

    u32 p_idx = loop.index.offset[f_i] + p_i;

    f32 small_number = LOOP_BBOX_SMALL_NUMBER;
    if (V.X < loop.loop_info[p_idx].aabb.min_x - small_number || V.X > loop.loop_info[p_idx].aabb.max_x + small_number ||
        V.Z < loop.loop_info[p_idx].aabb.min_z - small_number || V.Z > loop.loop_info[p_idx].aabb.max_z + small_number)
    {
        return false;
    }

    return (left_right_test(f_i, p_i, vector2df(V.X, V.Z)) == 1);
}

bool polyfold_soa::is_point_on_face(u32 f_i, core::vector3df v) const
{
    u32 p_idx = loop.index.offset[f_i];
    u32 p_len = loop.index.len[f_i];

    int n = 0;

    for (u32 p_i = 0; p_i < p_len; p_i++)
    {
        if (is_point_in_loop(f_i, p_i, v) == true)
        {
            if (this->is_closed_loop(f_i, p_i))
                n++;
            else
                n--;
        }
    }
    return n > 0;
}

void test_iterators(polyfold_soa&);

void combine_polyfolds_soa(const std::vector<polyfold_soa*>& polies, polyfold_soa& res);

