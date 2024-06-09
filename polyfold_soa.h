#pragma once

#include <irrlicht.h>
#include "csg_classes.h"

using namespace std;
struct soa_index;
struct polyfold_soa;

struct polyfold_iterator_base;
struct polyfold_iterator;
struct polyfold_iterator_loop_vertices;
struct polyfold_iterator_face_edges;

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

struct poly_edge2
{
    u32 v0;
    u32 v1;

    void operator=(const poly_edge& other) {
        v0 = other.v0;
        v1 = other.v1;
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

template<typename T>
struct single_indexed_field
{
    soa_single_index index;
    vector<T> data;

    T& operator[](u32 i) { return data[i]; }
    const T& operator[](u32 i)  const { return data[i]; }
};

template<typename T>
struct double_indexed_field
{
    soa_double_index index;
    vector<T> data;

    T& operator[](u32 i) { return data[i]; }
    const T& operator[](u32 i)  const { return data[i]; }
};

template<typename T>
struct triple_indexed_field
{
    soa_triple_index index;
    vector<T> data;

    T& operator[](u32 i)  { return data[i]; }
    const T& operator[](u32 i)  const { return data[i]; }
};

#define FILL_DECLARATIONS() \
template<typename T, typename Z> \
    void fill_indexed_field_convert(const polyfold_soa& soa, index_type<T>& field, const vector<polyfold>& pf_vec, size_t vector_offset); \
  template<typename T> \
    void fill_non_indexed_field(const polyfold_soa& soa, vector<T>& field, const vector<polyfold>& pf_vec, size_t data_offset); \
template<typename T> \
    void fill_indexed_field(const polyfold_soa& soa, index_type<T>& field, const vector<polyfold>& pf_vec, size_t vector_offset); \

class polyfold_soa
{
private:

    struct pf_struct
    {
        u32 n;

        single_indexed_field<poly_vert> vertices;
        single_indexed_field<poly_edge2> edges;

        template<typename T>
        using index_type = single_indexed_field<T>;

        FILL_DECLARATIONS();

    } pf;

    struct face_struct
    {
        soa_index_iterable index;
        vector<vector3df> normal;
        double_indexed_field<u32> edges;

        template<typename T>
        using index_type = double_indexed_field<T>;

        FILL_DECLARATIONS();

    } face;

    struct loop_struct
    {
        soa_index_iterable index;
        triple_indexed_field<u32> vertices;

        template<typename T>
        using index_type = triple_indexed_field<T>;

        FILL_DECLARATIONS();

    } loop;

    void fill_main_indexes(const vector<polyfold>& pf_vec);

public:

    void fill(const vector<polyfold>& pf_vec);

    inline polyfold_iterator get_pf();
    inline polyfold_iterator_loop_vertices pf_loop_vertices();
    inline polyfold_iterator_face_edges pf_face_edges();

    friend struct loop_iterator_base;
    friend struct face_iterator_base;
    friend struct polyfold_iterator_base;

    friend struct loop_iterator;
    friend struct face_iterator;
    friend struct polyfold_iterator;

    friend struct loop_iterator_loop_vertices;
    friend struct face_iterator_loop_vertices;
    friend struct polyfold_iterator_loop_vertices;

    friend struct face_iterator_face_edges;
    friend struct polyfold_iterator_face_edges;
};

struct loop_iterator_base
{
    u32 pf_i;
    u32 idx;
    u32 len;
    u32 i = 0;

    const polyfold_soa& src;

    loop_iterator_base(const polyfold_soa& src, u32 pf_i) : src(src), pf_i(pf_i), idx(0), len(0), i(0)
    {}

    u32 get_index() const { return idx + i; }

    bool end()
    {
        return i == len;
    }

    u32 n_vertices()
    {
        return src.loop.vertices.index.idx2.len[idx + i];
    }

    u32 vertex_index(u32 pf_i, u32 f_i, u32 i)
    {
        u32 v_i = src.loop.vertices.index.idx0.offset[pf_i] + src.loop.vertices.index.idx1.offset[f_i] + src.loop.vertices.index.idx2.offset[get_index()] + i;
        u32 v_j = src.loop.vertices[v_i];
        return src.pf.vertices.index.idx0.offset[pf_i] + v_j;
    }

    poly_vert vertex(u32 pf_i, u32 f_i, u32 i)
    {
        return src.pf.vertices[vertex_index(pf_i, f_i, i)];
    }
};

struct loop_iterator : loop_iterator_base
{
    loop_iterator(const polyfold_soa& src, u32 pf_i) : loop_iterator_base(src, pf_i) {}

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
    u32 pf_vert_offset;

    loop_iterator_loop_vertices(const polyfold_soa& src, u32 pf_i) : loop_iterator_base(src, pf_i), data_index(0), pf_vert_offset(src.pf.vertices.index.idx0.offset[pf_i])
    {}

    u32 get_data_index() { return data_index + src.loop.vertices.index.idx2.offset[get_index()]; }

    inline vector3df get_vertex(u32 j);

    void operator=(index_triple idx_pair)
    {
        idx = idx_pair.offset;
        len = idx_pair.len;
        data_index = idx_pair.data_offset;
        i = 0;
        p_data_index = data_index + src.loop.vertices.index.idx2.offset[get_index()];
    }

    void operator++()
    {
        i++;
        if (i < len)
        {
            p_data_index = data_index + src.loop.vertices.index.idx2.offset[get_index()];
        }
    }
};

struct face_iterator_base
{
    u32 pf_i;
    u32 idx;
    u32 len;
    u32 i;
    const polyfold_soa& src;

    face_iterator_base(const polyfold_soa& src, u32 pf_i) : src(src), pf_i(pf_i), idx(0), len(0), i(0)
    {}

    u32 get_index() const { return idx + i; }

    bool end()
    {
        return i == len;
    }

    vector3df normal()
    {
        return src.face.normal[get_index()];
    }

};

struct face_iterator : face_iterator_base
{
    loop_iterator loop_it;

    face_iterator(const polyfold_soa& src, u32 pf_i) : face_iterator_base(src, pf_i), loop_it(src, pf_i) {}

    void operator=(index_pair idx_pair)
    {
        idx = idx_pair.offset;
        len = idx_pair.len;
        i = 0;
    }
    
    index_pair loops()
    {
        return index_pair{ src.loop.index.offset[get_index()], src.loop.index.len[get_index()] };
    }

    void operator++()
    {
        i++;
    }
};

struct face_iterator_face_edges : face_iterator_base
{
    u32 data_index;
    u32 pf_data_index1;    //pf edges offset
    u32 pf_data_index2;    //pf vert offset
    u32 f_data_index1;
    u32 f_n_edges;

    u32 n_edges()
    {
        return f_n_edges;
    }

    face_iterator_face_edges::face_iterator_face_edges(const polyfold_soa& src, u32 pf_i)
        : face_iterator_base(src, pf_i), data_index(0)
    {}

    u32 get_data_index() { return data_index + src.face.edges.index.idx1.offset[get_index()]; }

    void operator=(index_5 idx_pair)
    {
        idx = idx_pair.offset;
        len = idx_pair.len;
        data_index = idx_pair.data_offset;
        pf_data_index1 = idx_pair.data_offset1;
        pf_data_index2 = idx_pair.data_offset2;
        i = 0;
        f_data_index1 = get_data_index();
        f_n_edges = src.face.edges.index.idx1.len[get_index()];
    }

    u32 edge_index(u32 e_i)
    {
        u32 e_0 = f_data_index1 + e_i;
        u32 v_i = pf_data_index1 + src.face.edges[e_0];
        return v_i;
    }

    poly_vert edge_v0(u32 e_i)
    {
        u32 v_i = edge_index(e_i);
        u32 v = pf_data_index2 + src.pf.edges[v_i].v0;
        return src.pf.vertices[v];
    }

    poly_vert edge_v1(u32 e_i)
    {
        u32 v_i = edge_index(e_i);
        u32 v = pf_data_index2 + src.pf.edges[v_i].v1;
        return src.pf.vertices[v];
    }

    void operator++()
    {
        i++;
        if (i < len)
        {
            f_data_index1 = get_data_index();
            f_n_edges = src.face.edges.index.idx1.len[get_index()];
        }
    }
};

struct face_iterator_loop_vertices : face_iterator_base
{
    loop_iterator_loop_vertices loop_it;
    u32 data_index;

    face_iterator_loop_vertices::face_iterator_loop_vertices(const polyfold_soa& src, u32 pf_i)
        : face_iterator_base(src, pf_i), loop_it(src, pf_i), data_index(0)
    {}

    u32 n_loops()
    {
        return src.loop.index.len[get_index()];
    }

    index_triple loops()
    {
        return index_triple{ src.loop.index.offset[get_index()], src.loop.index.len[get_index()], get_data_index() };
    }

    u32 get_data_index() { return data_index + src.loop.vertices.index.idx1.offset[get_index()]; }

    void operator=(index_triple idx_pair)
    {
        idx = idx_pair.offset;
        len = idx_pair.len;
        data_index = idx_pair.data_offset;
        i = 0;
    }

    void operator++()
    {
        i++;
    }
};

struct polyfold_iterator_base
{
    const polyfold_soa& src;
    u32 len;
    u32 i = 0;

    polyfold_iterator_base(const polyfold_soa& src, u32 len) : src(src), len(len), i(0) {}

    u32 get_index() const { return i; }

    bool end()
    {
        return i == len;
    }

    operator u32 () const
    {
        return get_index();
    }
};

struct polyfold_iterator : polyfold_iterator_base
{
    face_iterator face_it;

    polyfold_iterator(const polyfold_soa& src, u32 len)  :  polyfold_iterator_base(src, len), face_it(src, get_index()) {}

    index_pair faces()
    {
        return index_pair{ src.face.index.offset[get_index()], src.face.index.len[get_index()] };
    }
    
    u32 get_data_index() const {
        return src.loop.vertices.index.idx0.offset[get_index()];
    }

    void operator++()
    {
        i++;
        face_it.pf_i = get_index();
        face_it.loop_it.pf_i = get_index();
    }
};



struct polyfold_iterator_loop_vertices : polyfold_iterator_base
{
    face_iterator_loop_vertices face_it;

    polyfold_iterator_loop_vertices(const polyfold_soa& src, u32 len, u32 data_index) : polyfold_iterator_base(src, len), data_index(data_index), face_it(src,*this) 
    {
    }

    index_triple faces()
    {
        face_it.pf_i = get_index();
        face_it.loop_it.pf_i = get_index();
        face_it.loop_it.pf_vert_offset = src.pf.vertices.index.idx0.offset[get_index()];
        return index_triple{ src.face.index.offset[get_index()], src.face.index.len[get_index()], get_data_index() };
    }

    u32 get_data_index() { return data_index + src.loop.vertices.index.idx0.offset[get_index()]; }

    void operator++()
    {
        i++;
    }

    u32 data_index;
};

struct polyfold_iterator_face_edges : polyfold_iterator_base
{
    face_iterator_face_edges face_it;

    polyfold_iterator_face_edges(const polyfold_soa& src, u32 len, u32 data_index) : polyfold_iterator_base(src, len), data_index(data_index), face_it(src, *this)
    {
    }

    index_5 faces()
    {
        return index_5{ src.face.index.offset[get_index()], src.face.index.len[get_index()], get_data_index(), 
            src.pf.edges.index.idx0.offset[get_index()] , src.pf.vertices.index.idx0.offset[get_index()] };
    }

    u32 get_data_index() { return data_index + src.face.edges.index.idx0.offset[get_index()]; }

    void operator++()
    {
        i++;
    }

    u32 data_index;
};

polyfold_iterator polyfold_soa::get_pf()
{
    return polyfold_iterator(*this, pf.n);
}

polyfold_iterator_loop_vertices polyfold_soa::pf_loop_vertices()
{
    return polyfold_iterator_loop_vertices(*this, pf.n, 0);
}
polyfold_iterator_face_edges polyfold_soa::pf_face_edges()
{
    return polyfold_iterator_face_edges(*this, pf.n, 0);
}

vector3df loop_iterator_loop_vertices::get_vertex(u32 j)
{
    u32 v_j = src.loop.vertices[p_data_index + j];
    return src.pf.vertices[pf_vert_offset + v_j].V;
}

void test_iterators(polyfold_soa&);

