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

void  polyfold_soa::fill_vertices_and_loops(const vector<polyfold>& pf_vec)
{
    
    face.index.resize(pf.n);
    loop.vertices.index.idx0.resize(pf.n);
    pf.vertices.index.idx0.resize(pf.n);

    loop.vertices.index.idx1.resize(face.index.implied_size());
    face.normal.resize(face.index.implied_size());

    loop.vertices.index.idx2.resize(loop.index.implied_size());
    
    u32 size = 0;
    u32 c = 0;
    u32 c2 = 0;
    u32 cum_pf = 0;
    u32 cum_f = 0;
    u32 cum_p = 0;

    for (int i = 0; i < pf.n; i++)
    {
        loop.vertices.index.idx0.offset[i] = cum_pf;

        cum_f = 0;
        for (int j = 0; j < pf_vec[i].faces.size(); j++)
        {


            loop.vertices.index.idx1.offset[c] = cum_f;

            cum_p = 0;

            for (int k = 0; k < pf_vec[i].faces[j].loops.size(); k++)
            {
                loop.vertices.index.idx2.offset[c2] = cum_p;

                u32 size_inc = pf_vec[i].faces[j].loops[k].vertices.size();

                cum_pf += size_inc;
                cum_f += size_inc;
                cum_p += size_inc;

                loop.vertices.index.idx2.len[c2] = size_inc;

                c2++;
            }

            loop.vertices.index.idx1.len[c] = cum_p;

            c++;
        }

        loop.vertices.index.idx0.len[i] = cum_f;
    }
    loop.vertices.index.total_len = cum_pf;

    loop.vertices.data.resize(loop.vertices.index.implied_size());

    
    cum_pf = 0;

    for (int i = 0; i < pf.n; i++)
    {
        pf.vertices.index.idx0.offset[i] = cum_pf;
        cum_pf += pf_vec[i].vertices.size();
        pf.vertices.index.idx0.len[i] = pf_vec[i].vertices.size();
    }

    c = 0;
    for (int i = 0; i < pf.n; i++)
    {
        for (int j = 0; j < pf_vec[i].faces.size(); j++)
        {
            for (int k = 0; k < pf_vec[i].faces[j].loops.size(); k++)
            {
                for (int z = 0; z < pf_vec[i].faces[j].loops[k].vertices.size(); z++)
                {
                    loop.vertices[c] = pf_vec[i].faces[j].loops[k].vertices[z];
                    c++;
                }
            }
        }
    }

    size = 0;
    for (int i = 0; i < pf.n; i++)
    {
        
        size += pf_vec[i].vertices.size();
    }

    pf.vertices.data.resize(size);

    c = 0;
    for (int i = 0; i < pf.n; i++)
    {

        for (int j = 0; j < pf_vec[i].vertices.size(); j++)
        {
            pf.vertices[c] = pf_vec[i].vertices[j];
            c++;
        }
    }
}

void polyfold_soa::fill_edges(const vector<polyfold>& pf_vec)
{
    u32 size = pf.n;

    size = 0;
    for (int i = 0; i < pf_vec.size(); i++)
    {
        size += pf_vec[i].edges.size();
    }

    pf.edges.data.resize(size);

    u32 c = 0;
    for (int i = 0; i < pf_vec.size(); i++)
    {
        for (int j = 0; j < pf_vec[i].edges.size(); j++)
        {
            pf.edges[c].v0 = pf_vec[i].edges[j].v0;
            pf.edges[c].v1 = pf_vec[i].edges[j].v1;
            
            c++;
        }
    }

    pf.edges.index.idx0.resize(pf.n);
    face.edges.index.idx0.resize(pf.n);

    u32 cum = 0;

    for (int i = 0; i < pf_vec.size(); i++)
    {
        pf.edges.index.idx0.offset[i] = cum;
        cum += pf_vec[i].edges.size();

        pf.edges.index.idx0.len[i] = pf_vec[i].edges.size();
    }

    size = 0;

    for (int i = 0; i < pf_vec.size(); i++)
    {
        size += pf_vec[i].faces.size();
    }

    face.edges.index.idx1.resize(size);


    u32 cum_pf = 0;
    u32 cum_f = 0;
    c = 0;
    u32 c2 = 0;

    for (int i = 0; i < pf_vec.size(); i++)
    {
        face.edges.index.idx0.offset[i] = cum_pf;

        cum_f = 0;
        for (int j = 0; j < pf_vec[i].faces.size(); j++)
        {
            face.edges.index.idx1.offset[c2] = cum_f;

            u32 size_inc = pf_vec[i].faces[j].edges.size();

            face.edges.index.idx1.len[c2] = size_inc;
            cum_f += size_inc;
            cum_pf += size_inc;

            c2++;
        }

        face.edges.index.idx0.len[i] = cum_f;
    }

    size = 0;
    for (int i = 0; i < pf_vec.size(); i++)
    {
        for (int j = 0; j < pf_vec[i].faces.size(); j++)
        {
            size += pf_vec[i].faces[j].edges.size();
        }
    }

    face.edges.data.resize(size);

    c = 0;
    for (int i = 0; i < pf_vec.size(); i++)
    {
        for (int j = 0; j < pf_vec[i].faces.size(); j++)
        {
            for (int k = 0; k < pf_vec[i].faces[j].edges.size(); k++)
            {
                face.edges[c] = pf_vec[i].faces[j].edges[k];
                c++;
            }
        }
    }
}

/*

    for (auto pf = soa.pf_loop_vertices(); pf.end() == false; ++pf)
        {
            auto& f = pf.face_it;
            for (f = pf.faces(); f.end() == false; ++f)
            {
                auto& loop = f.loop_it;
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