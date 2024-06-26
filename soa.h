#pragma once

#include <vector>
#include <irrlicht.h>

using namespace std;

template<typename T>
struct soa_struct
{
	vector<T> data;
	vector<u16> offset;
	vector<u16> len;

	//template<typename AOS>
	//void fill_data(const vector<AOS>& src, size_t offset);

	template<typename AOS>
	void fill_data(AOS*, u32 n, u32(*length)(AOS*, u32), T(*item)(AOS*, u32, u32));
};

template<typename T, typename M>
struct soa_struct_2
{
	vector<T> data0;
	vector<M> data1;
	vector<u16> offset;
	vector<u16> len;

	//template<typename AOS>
	//void fill_data(const vector<AOS>& src, size_t offset0, size_t offset1);

	template<typename AOS>
	void fill_data(AOS* src, u32 n, u32(*length)(AOS*, u32), T(*item0)(AOS*, u32, u32), M(*item1)(AOS*, u32, u32));
};


template<typename T>
template<typename AOS>
inline void soa_struct<T>::fill_data(AOS* src, u32 n, u32(*length)(AOS*, u32), T(*item)(AOS*, u32, u32))
{
	u32 size = 0;

	offset.resize(n);
	len.resize(n);

	for (u32 i = 0; i < n; i++)
	{
		offset[i] = size;
		len[i] = length(src, i);
		size += length(src, i);
	}

	data.resize(size);

	u32 c = 0;

	for (u32 i = 0; i < n; i++)
		for (u32 j = 0; j < length(src, i); j++)
		{
			data[c] = item(src, i, j);
			c++;
		}
}

template<typename T, typename M>
template<typename AOS>
inline void soa_struct_2<T, M>::fill_data(AOS* src, u32 n, u32(*length)(AOS*, u32), T(*item0)(AOS*, u32, u32), M(*item1)(AOS*, u32, u32))
{
	u32 size = 0;

	offset.resize(n);
	len.resize(n);

	for (u32 i = 0; i < n; i++)
	{
		offset[i] = size;
		len[i] = length(src, i);
		size += length(src, i);
	}

	data0.resize(size);
	data1.resize(size);

	u32 c = 0;

	for (u32 i = 0; i < n; i++)
		for (u32 j = 0; j < length(src, i); j++)
		{
			data0[c] = item0(src, i, j);
			data1[c] = item1(src, i, j);
			c++;
		}
}