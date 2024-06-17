#include <irrlicht.h>
#include "polyfold_soa.h"

using namespace irr;
using namespace core;


void polyfold_soa::resize_main_indexes(const vector<polyfold_soa*>& pf_vec)
{
    u32 n_faces = 0;
    for (polyfold_soa* pf: pf_vec)
    {
        n_faces += pf->face.n;
    }

    face.n = n_faces;

    loop.index.resize(face.n);

    u32 offset_0 = 0;
    u32 total_len = 0;

    for (polyfold_soa* pf : pf_vec)
    {
        for (int i = 0; i < pf->face.n; i++)
        {
            loop.index.offset[offset_0 + i] = total_len;
            loop.index.len[offset_0 + i] = pf->loop.index.len[i];
            total_len += pf->loop.index.len[i];
        }

        offset_0 += pf->face.n;
    }

    loop.index.total_len = total_len;
}


void polyfold_soa::resize_single_indexed_field(const soa_single_index& size_index, size_t vec_offset)
{
    using index_type = single_indexed_field<u32>;

    index_type* src = (index_type*)((char*)(this) + vec_offset);


    soa_single_index new_index;
    new_index.idx0.resize(face.n);

    u32 total_size = 0;
    for (int j = 0; j < face.n; j++)
    {
        new_index.idx0.len[j] = src->index.idx0.len[j] + size_index.idx0.len[j];
        new_index.idx0.offset[j] = total_size;
       
        total_size += src->index.idx0.len[j] + size_index.idx0.len[j];
    }

    new_index.total_len = total_size;

    vector<u32> new_data;

    new_data.resize(new_index.implied_size());

    for (int j = 0; j < face.n; j++)
    {
        u32* mem_src = &src->data.operator[](src->index.idx0.offset[j]);
        u32* mem_dst = &new_data.operator[](new_index.idx0.offset[j]);
        u32 data_len = std::min(src->index.idx0.len[j], new_index.idx0.len[j]);

        memcpy(mem_dst, mem_src, data_len * sizeof(u32));
    }
    
    src->index = std::move(new_index);
    src->data = std::move(new_data);
}

template<typename T>
void polyfold_soa::combine_single_indexed_field(const vector<polyfold_soa*> pf_vec, size_t data_offset, size_t vec_offset)
{
    using index_type = single_indexed_field<u32>;

    index_type* dest = (index_type*)((char*)(this) + vec_offset);

    u32 size = 0;
    for (int i = 0; i < pf_vec.size(); i++)
    {
        polyfold_soa* pf = pf_vec[i];
        index_type* input = (index_type*)((char*)(pf)+vec_offset);

        size += input->index.implied_size();
    }

    dest->data.resize(size);

    dest->index.idx0.resize(face.n);


    u32 c = 0;
    u32 offset_0 = 0;
    for (int i = 0; i < pf_vec.size(); i++)
    {
        polyfold_soa* pf = pf_vec[i];
        index_type* input = (index_type*)((char*)(pf)+vec_offset);

        for (int j = 0; j < pf->face.n; j++)
        {
            u32 input_len = input->index.idx0.len[j];
            //u32 input_offset = input->index.idx0.offset[j];
            
            dest->index.idx0.len[c] = input_len;
            dest->index.idx0.offset[c] = offset_0;
                    
            offset_0 += input_len;
            c++;
        }
    }
    dest->index.total_len = offset_0;

    combine_non_indexed_field<T>(pf_vec, data_offset);
    combine_indexed_field_refdata<T>(pf_vec, data_offset, vec_offset + offsetof(index_type, data));

}

template<typename T, typename Z>
void polyfold_soa::offset_into(const vector<polyfold_soa*> pf_vec, size_t data_offset, size_t vec_offset)
{
    vector<Z>* dest = (vector<Z>*)((char*)(this) + vec_offset);

    u32 size = 0;
    u32 offset_0 = 0;
    for (int i = 0; i < pf_vec.size(); i++)
    {
        polyfold_soa* pf = pf_vec[i];

        vector<Z>* input = (vector<Z>*)((char*)(pf)+vec_offset);
        vector<T>* input_data = (vector<T>*)((char*)(pf)+data_offset);

        for (int j = 0; j < input->size(); j++)
        {
            dest->data()[size + j] += offset_0;
        }

        offset_0 += input_data->size();
        size += input->size();
    }
}

template<typename T>
void polyfold_soa::combine_indexed_field_refdata(const vector<polyfold_soa*> pf_vec, size_t data_offset, size_t vec_offset)
{
    vector<u32>* dest = (vector<u32>*)((char*)(this) + vec_offset);

    u32 size = 0;
    for (int i = 0; i < pf_vec.size(); i++)
    {
        polyfold_soa* pf = pf_vec[i];

        vector<u32>* input = (vector<u32>*)((char*)(pf) + vec_offset);
        size += input->size();
    }
    dest->resize(size);

    size = 0;

    for (int i = 0; i < pf_vec.size(); i++)
    {
        polyfold_soa* pf = pf_vec[i];

        vector<u32>* input = (vector<u32>*)((char*)(pf) + vec_offset);


        for (int j = 0; j < input->size(); j++)
        {
            dest->data()[size + j] = input->data()[j];
        }


        size += input->size();
    }

}

template<typename T>
void polyfold_soa::combine_non_indexed_field(const vector<polyfold_soa*> pf_vec, size_t vec_offset)
{
    vector<T>* dest = (vector<T>*)((char*)(this) + vec_offset);

    u32 size = 0;
    for (int i = 0; i < pf_vec.size(); i++)
    {
        polyfold_soa* pf = pf_vec[i];

        vector<T>* input = (vector<T>*)((char*)(pf) + vec_offset);
        size += input->size();
    }

    dest->resize(size);

    size = 0;
    for (int i = 0; i < pf_vec.size(); i++)
    {
        polyfold_soa* pf = pf_vec[i];

        vector<T>* input = (vector<T>*)((char*)(pf) + vec_offset);
        T* dest_adr = &(dest->data()[size]);
        memcpy(dest_adr, input->data(), sizeof(T) * input->size());

        size += input->size();
    }

}

template<typename T>
void polyfold_soa::combine_double_indexed_field(const vector<polyfold_soa*> pf_vec, size_t data_offset, size_t vec_offset)
{
    using index_type = double_indexed_field<u32>;

    index_type* dest = (index_type*)((char*)(this) + vec_offset);

    u32 size = 0;
    for (int i = 0; i < pf_vec.size(); i++)
    {
        polyfold_soa* pf = pf_vec[i];
        index_type* input = (index_type*)((char*)(pf) + vec_offset);

        size += input->index.implied_size();
    }

    dest->data.resize(size);

    dest->index.idx0.resize(face.n);
    dest->index.idx1.resize(loop.index.implied_size());

    u32 c = 0;
    u32 c2 = 0;
    u32 offset_0 = 0;
    
    for (int i = 0; i < pf_vec.size(); i++)
    {
        polyfold_soa* pf = pf_vec[i];
        index_type* input = (index_type*)((char*)(pf)+vec_offset);

        for (int f_i = 0; f_i < pf->face.n; f_i++)
        {
            u32 p_idx = pf->loop.index.offset[f_i];
            u32 p_len = pf->loop.index.len[f_i];

            dest->index.idx0.offset[c] = offset_0;

            u32 offset_1 = 0;

            for (int p_i = 0; p_i < p_len; p_i++)
            {
                u32 n_verts = input->index.idx1.len[p_idx + p_i];

                dest->index.idx1.len[c2] = n_verts;
                dest->index.idx1.offset[c2] = offset_1;

                offset_0 += n_verts;
                offset_1 += n_verts;

                c2++;
            }

            dest->index.idx0.len[c] = offset_1;

           
            c++;
        }
    }
    dest->index.total_len = offset_0;

    combine_non_indexed_field<T>(pf_vec, data_offset);
    combine_indexed_field_refdata<T>(pf_vec, data_offset, vec_offset + offsetof(index_type, data));

}

void polyfold_soa::combine(const vector<polyfold_soa*> pf_vec)
{
    resize_main_indexes(pf_vec);

    combine_non_indexed_field<poly_vert>(pf_vec, offsetof(polyfold_soa, vertices));
    combine_non_indexed_field<edge_info_struct>(pf_vec, offsetof(polyfold_soa, edge_info));

    combine_single_indexed_field<poly_edge2>(pf_vec, offsetof(polyfold_soa, edges), offsetof(polyfold_soa, face.edges));
    offset_into<poly_edge2, u32>(pf_vec, offsetof(polyfold_soa, edges), offsetof(polyfold_soa, face.edges.data));
    offset_into<poly_vert, poly_edge2>(pf_vec, offsetof(polyfold_soa, vertices), offsetof(polyfold_soa, edges));

    combine_double_indexed_field<poly_vert>(pf_vec, offsetof(polyfold_soa, vertices), offsetof(polyfold_soa, loop.vertices));
    offset_into<poly_vert, u32>(pf_vec, offsetof(polyfold_soa, vertices), offsetof(polyfold_soa, loop.vertices.data));

    combine_non_indexed_field<loop_info_struct>(pf_vec, offsetof(polyfold_soa, loop.loop_info));
    combine_non_indexed_field<vector3df>(pf_vec, offsetof(polyfold_soa, face.normal));
}

void  polyfold_soa::fill_main_indexes(const polyfold& pf)
{
    face.n = pf.faces.size();

    loop.index.resize(face.n);

    u32 size = 0;

    for (int i = 0; i < face.n; i++)
    {
        loop.index.offset[i] = size;
        loop.index.len[i] = pf.faces[i].loops.size();
        size += pf.faces[i].loops.size();
    }

    loop.index.total_len = size;
}

template<typename T>
void polyfold_soa::loop_struct::fill_indexed_field_closed_loop(const polyfold_soa& soa, double_indexed_field<T>& field, const polyfold& pf, size_t vector_offset)
{
    field.index.idx0.resize(soa.face.n);
    field.index.idx1.resize(soa.loop.index.implied_size());

    u32 cum_f = 0;
    u32 c = 0;

    for (int i = 0; i < pf.faces.size(); i++)
    {
        field.index.idx0.offset[i] = cum_f;

        u32 cum_p = 0;
        for (int j = 0; j < pf.faces[i].loops.size(); j++)
        {
            field.index.idx1.offset[c] = cum_p;

            vector<T>* vec = (vector<T>*)((char*)(&pf.faces[i].loops[j]) + vector_offset);

            u32 size_inc = 0;

            if(vec->size() > 0)
                size_inc = vec->size() + 1; //one extra entry to hold a copy of entry zero

            field.index.idx1.len[c] = size_inc;
            cum_p += size_inc;
            cum_f += size_inc;

            c++;
        }

        field.index.idx0.len[i] = cum_p;
    }

    field.index.total_len = cum_f;
    field.data.resize(field.index.implied_size());

    c = 0;
    for (int i = 0; i < pf.faces.size(); i++)
    {
        for (int j = 0; j < pf.faces[i].loops.size(); j++)
        {
            vector<T>* vec = (vector<T>*)((char*)(&pf.faces[i].loops[j]) + vector_offset);

            for (int k = 0; k < vec->size(); k++)
            {
                field.data[c] = vec->operator[](k);
                c++;
            }

            field.data[c] = vec->operator[](0);
            c++;
        }
    }
}

template<typename T, typename Z>
void polyfold_soa::loop_struct::fill_indexed_field_convert(const polyfold_soa& soa, double_indexed_field<T>& field, const polyfold& pf, size_t vector_offset)
{
    field.index.idx0.resize(soa.face.n);
    field.index.idx1.resize(soa.loop.index.implied_size());

    u32 cum_f = 0;
    u32 c = 0;

    for (int i = 0; i < pf.faces.size(); i++)
    {
        field.index.idx0.offset[i] = cum_f;

        u32 cum_p = 0;
        for (int j = 0; j < pf.faces[i].loops.size(); j++)
        {
            field.index.idx1.offset[c] = cum_p;

            vector<Z>* vec = (vector<Z>*)((char*)(&pf.faces[i].loops[j]) + vector_offset);
            u32 size_inc = vec->size();

            field.index.idx1.len[c] = size_inc;
            cum_p += size_inc;
            cum_f += size_inc;

            c++;
        }

        field.index.idx0.len[i] = cum_p;
    }

    field.index.total_len = cum_f;
    field.data.resize(field.index.implied_size());

    c = 0;
    for (int i = 0; i < pf.faces.size(); i++)
    {
        for (int j = 0; j < pf.faces[i].loops.size(); j++)
        {
            vector<Z>* vec = (vector<Z>*)((char*)(&pf.faces[i].loops[j]) + vector_offset);

            for (int k = 0; k < vec->size(); k++)
            {
                field.data[c] = vec->operator[](k);
                c++;
            }
        }
    }
}

template<typename T>
void polyfold_soa::loop_struct::fill_indexed_field(const polyfold_soa& soa, double_indexed_field<T>& field, const polyfold& pf, size_t vector_offset)
{
    fill_indexed_field_convert<T,T>(soa, field, pf, vector_offset);
}

template<typename T, typename Z>
void polyfold_soa::loop_struct::fill_non_indexed_field(const polyfold_soa& soa, vector<T>& field, const polyfold& pf, size_t data_offset)
{
    field.resize(soa.loop.index.implied_size());

    u32 c = 0;
    for (int i = 0; i < pf.faces.size(); i++)
    {
        for (int j = 0; j < pf.faces[i].loops.size(); j++)
        {
            Z* r = (Z*)((char*)(&pf.faces[i].loops[j]) + data_offset);

            field[c] = *r;
            c++;
        }
    }
}

template<typename T>
void polyfold_soa::loop_struct::fill_non_indexed_field(const polyfold_soa& soa, vector<T>& field, const polyfold& pf, size_t data_offset)
{
    fill_non_indexed_field<T, T>(soa, field, pf, data_offset);
}

template<typename T, typename Z>
void polyfold_soa::face_struct::fill_indexed_field_convert(const polyfold_soa& soa, single_indexed_field<T>& field, const polyfold& pf, size_t vector_offset)
{
    field.index.idx0.resize(soa.face.n);

    u32 cum_f = 0;

    for (int i = 0; i < soa.face.n; i++)
    {
        field.index.idx0.offset[i] = cum_f;

        vector<Z>* vec = (vector<Z>*)((char*)(&pf.faces[i]) + vector_offset);
        u32 size_inc = vec->size();

        cum_f += size_inc;
        field.index.idx0.len[i] = size_inc;
    }

    field.index.total_len = cum_f;

    field.data.resize(field.index.implied_size());

    u32 c = 0;

    for (int i = 0; i < soa.face.n; i++)
    {
        vector<Z>* vec = (vector<Z>*)((char*)(&pf.faces[i]) + vector_offset);
        for (int j = 0; j < vec->size(); j++)
        {
            field.data[c] = vec->operator[](j);
            c++;
        }
    }
}

template<typename T>
void polyfold_soa::face_struct::fill_indexed_field(const polyfold_soa& soa, single_indexed_field<T>& field, const polyfold& pf, size_t vector_offset)
{
    fill_indexed_field_convert<T,T>(soa, field, pf, vector_offset);
}


template<typename T>
void polyfold_soa::face_struct::fill_non_indexed_field(const polyfold_soa& soa, vector<T>& field, const polyfold& pf, size_t data_offset)
{
    field.resize(soa.face.n);

    u32 c = 0;
    for (int i = 0; i < soa.face.n; i++)
    {
        T* r = (T*)((char*)(&pf.faces[i]) + data_offset);

        field[c] = *r;
        c++;
    }
}

template<typename T>
void polyfold_soa::face_struct::size_non_indexed_field(const polyfold_soa& soa, vector<T>& field)
{
    field.resize(soa.face.n);
}

void  polyfold_soa::fill(const polyfold& pf)
{
    fill_main_indexes(pf);

    vertices = pf.vertices;

    edges.resize(pf.edges.size());
    edge_info.resize(pf.edges.size());

    for (int i = 0; i < edges.size(); i++)
    {
        edges[i] = pf.edges[i];
        edge_info[i] = pf.edges[i];
    }

    loop.fill_indexed_field_closed_loop<u32>(*this, loop.vertices, pf, offsetof(poly_loop, vertices));

    loop.fill_non_indexed_field<loop_info_struct,poly_loop>(*this, loop.loop_info, pf, 0);

    face.fill_indexed_field<u32>(*this, face.edges, pf, offsetof(poly_face, edges));
    face.fill_non_indexed_field<vector3df>(*this, face.normal, pf, offsetof(poly_face, m_normal));
    face.fill_indexed_field<u32>(*this, face.vertices, pf, offsetof(poly_face, vertices));

    face.size_non_indexed_field<matrix4>(*this, face.mat2D);

    for (int i = 0; i < face.n; i++)
        get2Dmat(i, face.mat2D[i]);

}

/*
template<typename T>
void single_indexed_field<T>::resize(const soa_single_index& new_index)
{
    vector<T> new_data;

    new_data.resize(new_index.implied_size());

    for (int i = 0; i < index.idx0.offset.size() && i < new_index.idx0.offset.size(); i++)
    {
        size_t size = std::min(new_index.idx0.len[i], index.idx0.len[i]);

        if(size > 0)
            memcpy(new_data[new_index.idx0.offset[i]], data[index.idx0.offset[i]], size);
    }

    data = new_data;
    index = new_index;
}

template<typename T>
void double_indexed_field<T>::resize(const polyfold_soa& soa, const soa_double_index& new_index)
{
    vector<T> new_data;

    new_data.resize(new_index.implied_size());

    for (int pf_i = 0; pf_i < soa.pf_n; pf_i++)
    {
        u32 f_idx = soa.faces.offset[pf_i];
        u32 f_len = soa.faces.len[pf_i];

        u32 pf0 = soa.loop_vertices_index.idx0.offset[pf_i];

        for (int f_i = 0; f_i < f_len; f_i++)
        {
            u32 p_idx = soa.loops.offset[f_idx + f_i];
            u32 p_len = soa.loops.len[f_idx + f_i];
            u32 f0 = pf0 + soa.loop_vertices_index.idx1.offset[f_idx + f_i];
        }
    }

    data = new_data;
    index = new_index;
}

template<typename T>
void triple_indexed_field<T>::resize(const polyfold_soa& soa, const polyfold_soa_layout& new_layout, const soa_triple_index& new_data_index)
{
    index.idx0.resize(new_layout.pf.n);
    index.idx1.resize(new_layout.face_index.implied_size());
    index.idx2.resize(new_layout.loop_index.implied_size());

    vector<T> new_data;
    new_data.resize(new_layout.loop_index.implied_size());

    u32 cum_pf = 0;

    for (int pf_i = 0; pf_i < soa.pf_n; pf_i++)
    {
        u32 f_idx = soa.faces.offset[pf_i];
        u32 f_len = soa.faces.len[pf_i];

        u32 f2_idx = new_layout.face_index.offset[pf_i];
        u32 f2_len = new_layout.face_index.len[pf_i];

        u32 f_small_len = std::min(f_len, f2_len);
        
        u32 cum_f = 0;
        index.idx0.offset[pf_i] = cum_pf;

        //u32 pf0 = new_data_index.idx0.offset[pf_i];

        for (int f_i = 0; f_i < f_small_len; f_i++)
        {
            u32 p_idx = soa.loops.offset[f_idx + f_i];
            u32 p_len = soa.loops.len[f_idx + f_i];

            u32 p2_idx = new_layout.loop_index.offset[f_idx + f_i];
            u32 p2_len = new_layout.loop_index.len[f_idx + f_i];

            u32 p_small_len = std::min(p_len, p2_len);

            //u32 f0 = pf0 + new_data_index.idx1.offset[f_idx + f_i];

            index.idx1.offset[c] = cum_f;
            u32 cum_p = 0;

            for (int p_i = 0; p_i < p_small_len; p_i++)
            {
               // u32 p0 = f0 + new_data_index.idx2.offset[p_idx + p_i];

                //u32 size_inc = new_data_index.idx2.len[p_idx + p_i]
                cum_pf += size_inc;
                cum_f += size_inc;
                cum_p += size_inc;

            }

            index.idx1.len[c] = cum_p;

            c++;
        }

        index.idx0.len[pf_i] = cum_f;

    }

}*/
/*
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
    field.index.total_len = cum_pf;*/

/*
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
*/