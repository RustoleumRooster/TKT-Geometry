#include <irrlicht.h>
#include "polyfold_soa.h"

void  polyfold_soa::fill_main_indexes(const vector<polyfold>& pf_vec)
{
    pf.n = pf_vec.size();

    u32 size = pf.n;

    face.index.resize(size);

    size = 0;
    for (int i = 0; i < pf.n; i++)
    {
        face.index.offset[i] = size;
        face.index.len[i] = pf_vec[i].faces.size();
        size += pf_vec[i].faces.size();
    }
    face.index.total_len = size;

    loop.index.resize(size);

    size = 0;
    u32 c = 0;

    for (int i = 0; i < pf.n; i++)
    {
        for (int j = 0; j < pf_vec[i].faces.size(); j++)
        {
            loop.index.offset[c] = size;
            loop.index.len[c] = pf_vec[i].faces[j].loops.size();
            size += pf_vec[i].faces[j].loops.size();
            c++;
        }
    }

    loop.index.total_len = size;
}

template<typename T, typename Z>
void polyfold_soa::face_struct::fill_indexed_field_convert(const polyfold_soa& soa, double_indexed_field<T>& field, const vector<polyfold>& pf_vec, size_t vector_offset)
{
    field.index.idx0.resize(soa.pf.n);
    field.index.idx1.resize(soa.face.index.implied_size());

    u32 cum_pf = 0;
    u32 c = 0;

    for (int i = 0; i < pf_vec.size(); i++)
    {
        field.index.idx0.offset[i] = cum_pf;

        u32 cum_f = 0;
        for (int j = 0; j < pf_vec[i].faces.size(); j++)
        {
            field.index.idx1.offset[c] = cum_f;

            vector<Z>* vec = (vector<Z>*)((char*)(&pf_vec[i].faces[j]) + vector_offset);
            u32 size_inc = vec->size();

            field.index.idx1.len[c] = size_inc;
            cum_f += size_inc;
            cum_pf += size_inc;

            c++;
        }

        field.index.idx0.len[i] = cum_f;
    }

    field.index.total_len = cum_pf;
    field.data.resize(field.index.implied_size());

    c = 0;
    for (int i = 0; i < pf_vec.size(); i++)
    {
        for (int j = 0; j < pf_vec[i].faces.size(); j++)
        {
            vector<Z>* vec = (vector<Z>*)((char*)(&pf_vec[i].faces[j]) + vector_offset);

            for (int k = 0; k < pf_vec[i].faces[j].edges.size(); k++)
            {
                field.data[c] = vec->operator[](k);
                c++;
            }
        }
    }
}

template<typename T>
void polyfold_soa::face_struct::fill_indexed_field(const polyfold_soa& soa, double_indexed_field<T>& field, const vector<polyfold>& pf_vec, size_t vector_offset)
{
    fill_indexed_field_convert<T,T>(soa, field, pf_vec, vector_offset);
}

template<typename T>
void polyfold_soa::face_struct::fill_non_indexed_field(const polyfold_soa& soa, vector<T>& field, const vector<polyfold>& pf_vec, size_t data_offset)
{
    field.resize(soa.face.index.implied_size());

    u32 c = 0;
    for (int i = 0; i < soa.pf.n; i++)
    {
        for (int j = 0; j < pf_vec[i].faces.size(); j++)
        {
            T* r = (T*)((char*)(&pf_vec[i].faces[j]) + data_offset);

            field[c] = *r;
            c++;
        }
    }
}

template<typename T, typename Z>
void polyfold_soa::pf_struct::fill_indexed_field_convert(const polyfold_soa& soa, single_indexed_field<T>& field, const vector<polyfold>& pf_vec, size_t vector_offset)
{
    field.index.idx0.resize(soa.pf.n);

    u32 cum_pf = 0;

    for (int i = 0; i < soa.pf.n; i++)
    {
        field.index.idx0.offset[i] = cum_pf;

        vector<Z>* vec = (vector<Z>*)((char*)(&pf_vec[i]) + vector_offset);
        u32 size_inc = vec->size();

        cum_pf += size_inc;
        field.index.idx0.len[i] = size_inc;
    }

    field.index.total_len = cum_pf;

    field.data.resize(field.index.implied_size());

    u32 c = 0;

    for (int i = 0; i < soa.pf.n; i++)
    {
        vector<Z>* vec = (vector<Z>*)((char*)(&pf_vec[i]) + vector_offset);
        for (int j = 0; j < vec->size(); j++)
        {
            field.data[c] = vec->operator[](j);
            c++;
        }
    }
}

template<typename T>
void polyfold_soa::pf_struct::fill_indexed_field(const polyfold_soa& soa, single_indexed_field<T>& field, const vector<polyfold>& pf_vec, size_t vector_offset)
{
    fill_indexed_field_convert<T,T>(soa, field, pf_vec, vector_offset);
}


template<typename T>
void polyfold_soa::pf_struct::fill_non_indexed_field(const polyfold_soa& soa, vector<T>& field, const vector<polyfold>& pf_vec, size_t data_offset)
{
    field.resize(soa.pf.n);

    u32 c = 0;
    for (int i = 0; i < soa.pf.n; i++)
    {
        T* r = (T*)((char*)(&pf_vec[i]) + data_offset);

        field[c] = *r;
        c++;
    }
}

template<typename T, typename Z>
void polyfold_soa::loop_struct::fill_indexed_field_convert(const polyfold_soa& soa, triple_indexed_field<T>& field, const vector<polyfold>& pf_vec, size_t vector_offset)
{
    field.index.idx0.resize(soa.pf.n);
    field.index.idx1.resize(soa.face.index.implied_size());
    field.index.idx2.resize(soa.loop.index.implied_size());

    u32 c = 0;
    u32 c2 = 0;
    u32 cum_pf = 0;

    for (int i = 0; i < soa.pf.n; i++)
    {
        field.index.idx0.offset[i] = cum_pf;

        u32 cum_f = 0;
        for (int j = 0; j < pf_vec[i].faces.size(); j++)
        {


            field.index.idx1.offset[c] = cum_f;

            u32 cum_p = 0;

            for (int k = 0; k < pf_vec[i].faces[j].loops.size(); k++)
            {
                field.index.idx2.offset[c2] = cum_p;

                vector<Z>* vec = (vector<Z>*)((char*)(&pf_vec[i].faces[j].loops[k]) + vector_offset);

                u32 size_inc = vec->size();

                cum_pf += size_inc;
                cum_f += size_inc;
                cum_p += size_inc;

                field.index.idx2.len[c2] = size_inc;

                c2++;
            }

            field.index.idx1.len[c] = cum_p;

            c++;
        }

        field.index.idx0.len[i] = cum_f;
    }
    field.index.total_len = cum_pf;

    field.data.resize(field.index.implied_size());

    c = 0;
    for (int i = 0; i < soa.pf.n; i++)
    {
        for (int j = 0; j < pf_vec[i].faces.size(); j++)
        {
            for (int k = 0; k < pf_vec[i].faces[j].loops.size(); k++)
            {
                vector<Z>* vec = (vector<Z>*)((char*)(&pf_vec[i].faces[j].loops[k]) + vector_offset);

                for (int z = 0; z < vec->size(); z++)
                {
                    field.data[c] = vec->operator[](z);
                    c++;
                }
            }
        }
    }
}

template<typename T>
void polyfold_soa::loop_struct::fill_indexed_field(const polyfold_soa& soa, triple_indexed_field<T>& field, const vector<polyfold>& pf_vec, size_t vector_offset)
{
    fill_indexed_field_convert<T,T>(soa, field, pf_vec, vector_offset);
}

template<typename T>
void polyfold_soa::loop_struct::fill_non_indexed_field(const polyfold_soa& soa, vector<T>& field, const vector<polyfold>& pf_vec, size_t data_offset)
{
    field.resize(soa.loop.index.implied_size());

    u32 c = 0;
    for (int i = 0; i < soa.pf.n; i++)
    {
        for (int j = 0; j < pf_vec[i].faces.size(); j++)
        {
            for (int k = 0; k < pf_vec[i].faces[j].loops.size(); k++)
            {
                T* r = (T*)((char*)(&pf_vec[i].faces[j].loops[k]) + data_offset);

                field[c] = *r;
                c++;
            }
        }
    }
}

void  polyfold_soa::fill(const vector<polyfold>& pf_vec)
{
    fill_main_indexes(pf_vec);

    loop.fill_indexed_field<u32>(*this, loop.vertices, pf_vec, offsetof(poly_loop, vertices));

    pf.fill_indexed_field<poly_vert>(*this, pf.vertices, pf_vec, offsetof(polyfold, vertices));
    pf.fill_indexed_field_convert<poly_edge2, poly_edge>(*this, pf.edges, pf_vec, offsetof(polyfold, edges));

    face.fill_indexed_field<u32>(*this, face.edges, pf_vec, offsetof(poly_face, edges));
    face.fill_non_indexed_field<vector3df>(*this, face.normal, pf_vec, offsetof(poly_face, m_normal));
}

/*

    for (auto pf = soa.pf_loop_vertices(); pf.end() == false; ++pf)
        {
            auto& f = pf.face_it;
            for (f = pf.faces(); f.end() == false; ++f)
            {
                auto& loop = f.loop_it;

                if(f.n_loops() > 0)
                    for (loop = f.loops(); loop.end() == false; ++loop)
                    {
                        results[c] = vector3df(0, 0, 0);

                        for (int i = 0; i < loop.n_vertices(); i++)
                        {
                            results[c] += loop.get_vertex(i);
                        }
                        c++;
                    }
            }
        }


    for (auto pf = soa.pf_face_edges(); pf.end() == false; ++pf)
        {
            auto& f = pf.face_it;
            for (f = pf.faces(); f.end() == false; ++f)
            {
                for (int e_i = 0; e_i < f.n_edges(); e_i++)
                {
                    results[c] = f.edge_v1(e_i).V;
                    c++;
                }
            }
        }

*/