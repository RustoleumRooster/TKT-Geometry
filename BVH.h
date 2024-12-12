#pragma once

#ifndef _BVH_H_
#define _BHH_H_

#include <vector>
#include <irrlicht.h>

using namespace irr;
using namespace core;

class LineHolder;

f32 calc_surface_area(f32* aabbMin, f32* aabbMax);

struct aabb_struct
{
public:
	f32 aabbMin[3] = { 1e30,1e30,1e30 };
	f32 aabbMax[3] = { -1e30,-1e30,-1e30 };

	f32 surface_area() { 
		return calc_surface_area(aabbMin,aabbMax);
	}

	void addPoint(vector3df v)
	{
		aabbMin[0] = fmin(aabbMin[0], v.X);
		aabbMin[1] = fmin(aabbMin[1], v.Y);
		aabbMin[2] = fmin(aabbMin[2], v.Z);

		aabbMax[0] = fmax(aabbMax[0], v.X);
		aabbMax[1] = fmax(aabbMax[1], v.Y);
		aabbMax[2] = fmax(aabbMax[2], v.Z);
	}

	void clear()
	{
		aabbMin[0] = aabbMin[1] = aabbMin[2] = 1e30;
		aabbMax[0] = aabbMax[1] = aabbMax[2] = -1e30;
	}

	template<typename T>
	bool overlaps(const T* other, f32 epsilon) const
	{
		bool ret = aabbMax[0] + epsilon > other->aabbMin[0] && aabbMin[0] - epsilon < other->aabbMax[0] &&
			aabbMax[1] + epsilon > other->aabbMin[1] && aabbMin[1] - epsilon < other->aabbMax[1] &&
			aabbMax[2] + epsilon > other->aabbMin[2] && aabbMin[2] - epsilon < other->aabbMax[2];

		return ret;
	}

	bool overlaps(const core::vector3df& r, f32 epsilon) const
	{
		bool ret = aabbMax[0] + epsilon > r.X && aabbMin[0] - epsilon < r.X &&
			aabbMax[1] + epsilon > r.Y && aabbMin[1] - epsilon < r.Y &&
			aabbMax[2] + epsilon > r.Z && aabbMin[2] - epsilon < r.Z;

		return ret;
	}

	template<typename T>
	void grow(T* obj) const
	{
		obj->aabbMin[0] = fmin(obj->aabbMin[0], aabbMin[0]);
		obj->aabbMin[1] = fmin(obj->aabbMin[1], aabbMin[1]);
		obj->aabbMin[2] = fmin(obj->aabbMin[2], aabbMin[2]);

		obj->aabbMax[0] = fmax(obj->aabbMax[0], aabbMax[0]);
		obj->aabbMax[1] = fmax(obj->aabbMax[1], aabbMax[1]);
		obj->aabbMax[2] = fmax(obj->aabbMax[2], aabbMax[2]);
	}

	void addDrawLines(LineHolder& graph) const;
	void textdump();
};

template<typename T>
void swap_indices(T a, T b, std::vector<T>& data)
{
	T tmp = data[a];
	data[a] = data[b];
	data[b] = tmp;
}

struct BVH_intersection_struct
{
	u16 X;
	u16 Y;
};

struct BVH_node
{
	f32 aabbMin[3];
	f32 aabbMax[3];

	u16 left_node = 0xFFFF;
	u16 right_node = 0xFFFF;

	u16 first_prim;
	u16 n_prims;

	u16 id = 0;

	void set_aabb(const aabb_struct& aabb)
	{
		memcpy(&aabbMin, aabb.aabbMin, sizeof(f32) * 3);
		memcpy(&aabbMax, aabb.aabbMax, sizeof(f32) * 3);
	}

	bool isLeafNode() const {
		return (left_node == 0xFFFF && right_node == 0xFFFF);
	}

	template<typename T>
	bool overlaps(const T* other, f32 epsilon) const
	{
		bool ret = aabbMax[0] + epsilon > other->aabbMin[0] && aabbMin[0] - epsilon < other->aabbMax[0] &&
			aabbMax[1] + epsilon > other->aabbMin[1] && aabbMin[1] - epsilon < other->aabbMax[1] &&
			aabbMax[2] + epsilon > other->aabbMin[2] && aabbMin[2] - epsilon < other->aabbMax[2];

		return ret;
	}

	template<>
	bool overlaps<core::vector3df>(const core::vector3df* r, f32 epsilon) const
	{
		bool ret = aabbMax[0] + epsilon > r->X && aabbMin[0] - epsilon < r->X &&
			aabbMax[1] + epsilon > r->Y && aabbMin[1] - epsilon < r->Y &&
			aabbMax[2] + epsilon > r->Z && aabbMin[2] - epsilon < r->Z;

		return ret;
	}

	template<typename T>
	bool overlaps_recursive(const T* other, f32 epsilon, const std::vector<BVH_node>& nodes) const
	{
		if (overlaps(other, epsilon))
		{
			if (isLeafNode())
			{
				return true;
			}
			else if(nodes[left_node].overlaps_recursive(other, epsilon, nodes) || nodes[right_node].overlaps_recursive(other, epsilon, nodes))
			{
				return true;
			}
		}
		return false;
	}

	bool intersect_recursive(const BVH_node* other, f32 epsilon, const std::vector<BVH_node>& nodes, const std::vector<BVH_node>& other_nodes) const
	{
		if (other->overlaps_recursive(this, epsilon, other_nodes))
		{
			return (isLeafNode() || nodes[left_node].intersect_recursive(other,epsilon,nodes,other_nodes) ||
					nodes[right_node].intersect_recursive(other, epsilon, nodes, other_nodes));
		}
		return false;
	}

	template<typename T>
	void find_intersection_with_single_node(const T* other, f32 epsilon, const std::vector<BVH_node>& nodes, std::vector<u16>& results) const
	{
		if (overlaps(other, epsilon))
		{
			if (isLeafNode()) {
				for (u16 i = first_prim; i < first_prim + n_prims; i++) {
					results.push_back(i);
				}
			}
			else {
				nodes[left_node].find_intersection_with_single_node(other, epsilon, nodes, results);
				nodes[right_node].find_intersection_with_single_node(other, epsilon, nodes, results);
			}
		}
	}

	template<typename T>
	void find_nodes_intersection_with_single_node(const T* other, f32 epsilon, const std::vector<BVH_node>& nodes, std::vector<u16>& results) const
	{
		if (overlaps(other, epsilon))
		{
			if (isLeafNode()) {
					results.push_back(id);
			}
			else {
				nodes[left_node].find_nodes_intersection_with_single_node(other, epsilon, nodes, results);
				nodes[right_node].find_nodes_intersection_with_single_node(other, epsilon, nodes, results);
			}
		}
	}

	template<typename T>
	void find_single_intersection_with_single_node(const T* other, f32 epsilon, const std::vector<BVH_node>& nodes, u16* out) const
	{
		if (overlaps(other, epsilon))
		{
			if (isLeafNode()) {
				*out = first_prim;
			}
			else {
				nodes[left_node].find_single_intersection_with_single_node(other, epsilon, nodes, out);
				nodes[right_node].find_single_intersection_with_single_node(other, epsilon, nodes, out);
			}
		}
	}

	template<typename T>
	void find_single_node_intersection_with_single_node(const T* other, f32 epsilon, const std::vector<BVH_node>& nodes, u16* out) const
	{
		if (overlaps(other, epsilon))
		{
			if (isLeafNode()) {
				*out = id;
			}
			else {
				nodes[left_node].find_single_node_intersection_with_single_node(other, epsilon, nodes, out);
				nodes[right_node].find_single_node_intersection_with_single_node(other, epsilon, nodes, out);
			}
		}
	}

	void find_intersection(const BVH_node* other, f32 epsilon, const std::vector<BVH_node>& nodes, const std::vector<BVH_node>& other_nodes, std::vector<u16>& results) const
	{
		if (other->overlaps_recursive(this, epsilon, other_nodes))
		{
			if (isLeafNode()) {
				for (u16 i = first_prim; i < first_prim + n_prims; i++) {
					results.push_back(i);
				}
			}
			else {
				nodes[left_node].find_intersection(other, epsilon, nodes, other_nodes, results);
				nodes[right_node].find_intersection(other, epsilon, nodes, other_nodes, results);
			}
		}
	}

	void find_intersection_2(const BVH_node* other, f32 epsilon, const std::vector<BVH_node>& nodes, const std::vector<BVH_node>& other_nodes, std::vector<BVH_intersection_struct>& results) const
	{
		if (isLeafNode())
		{
			std::vector<u16> partial_results;
			other->find_intersection(this, epsilon, other_nodes, nodes, partial_results);
			for (u16 v : partial_results)
				for (u16 i = first_prim; i < first_prim + n_prims; i++) {
					results.push_back(BVH_intersection_struct{ i ,v });
				}
		}
		else if (other->overlaps_recursive(this, epsilon, other_nodes))
		{
			nodes[left_node].find_intersection_2(other, epsilon, nodes, other_nodes, results);
			nodes[right_node].find_intersection_2(other, epsilon, nodes, other_nodes, results);
		}
	}

	void find_intersection_nodes(const BVH_node* other, f32 epsilon, const std::vector<BVH_node>& nodes, const std::vector<BVH_node>& other_nodes, std::vector<u16>& results) const
	{
		if (other->overlaps_recursive(this, epsilon, other_nodes))
		{
			if (isLeafNode()) {
				results.push_back(id);
			}
			else {
				nodes[left_node].find_intersection_nodes(other, epsilon, nodes, other_nodes, results);
				nodes[right_node].find_intersection_nodes(other, epsilon, nodes, other_nodes, results);
			}
		}
	}

	void find_intersection_nodes_2(const BVH_node* other, f32 epsilon, const std::vector<BVH_node>& nodes, const std::vector<BVH_node>& other_nodes, std::vector<BVH_intersection_struct>& results) const
	{
		if (isLeafNode())
		{
			std::vector<u16> partial_results;
			other->find_intersection_nodes(this, epsilon, other_nodes, nodes, partial_results);
			for (u16 v : partial_results)
				results.push_back(BVH_intersection_struct{ id,v });
		}
		else if (other->overlaps_recursive(this, epsilon, other_nodes))
		{
			nodes[left_node].find_intersection_nodes_2(other, epsilon, nodes, other_nodes, results);
			nodes[right_node].find_intersection_nodes_2(other, epsilon, nodes, other_nodes, results);
		}
	}

	void find_intersection_nodes_inverse(const BVH_node* other, f32 epsilon, const std::vector<BVH_node>& nodes, const std::vector<BVH_node>& other_nodes, std::vector<u16>& results) const
	{
		if (other->overlaps_recursive(this, epsilon, other_nodes))
		{
			if (isLeafNode()) {

			}
			else {
				nodes[left_node].find_intersection_nodes_inverse(other, epsilon, nodes, other_nodes, results);
				nodes[right_node].find_intersection_nodes_inverse(other, epsilon, nodes, other_nodes, results);
			}
		}
		else results.push_back(id);
	}

	template<typename T>
	void grow(T* obj) const
	{
		obj->aabbMin[0] = fmin(obj->aabbMin[0], aabbMin[0]);
		obj->aabbMin[1] = fmin(obj->aabbMin[1], aabbMin[1]);
		obj->aabbMin[2] = fmin(obj->aabbMin[2], aabbMin[2]);

		obj->aabbMax[0] = fmax(obj->aabbMax[0], aabbMax[0]);
		obj->aabbMax[1] = fmax(obj->aabbMax[1], aabbMax[1]);
		obj->aabbMax[2] = fmax(obj->aabbMax[2], aabbMax[2]);
	}

	f32 surface_area()
	{
		return calc_surface_area(aabbMin, aabbMax);
	}

	f32 length(int axis)
	{
		return aabbMax[axis] - aabbMin[axis];
	}

	void addDrawLines(LineHolder& graph) const;
	void addDrawLines_Recursive(int depth, std::vector<BVH_node>& nodes, int current_depth, LineHolder& graph) const;
	void addDrawLines_Leaves_Recursive(std::vector<BVH_node>& nodes, LineHolder& graph) const;
};

template<typename T, typename NODE_T, typename INDEX_T>
struct BVH_structure_base
{
	std::vector<NODE_T> nodes;
	std::vector<INDEX_T> indices;
	int node_count = 0;


	virtual void grow_prim(const T* data, u16 i, aabb_struct& aabb) = 0;
	virtual core::vector3df prim_position(const T* data, u16 i) = 0;

	f32 find_best_split(const f32* aabbMin, const f32* aabbMax, int& axis, f32& split_out, aabb_struct& sub_L, aabb_struct& sub_R, u16 first_prim, u16 n_prims, const T* data);

	template<int cutoff>
	void recursive_subdivide(u16 n_i, int& N, std::vector<NODE_T>& nodes, const T* data);

	bool subdivide(u16 n_i, int& N, int axis, f32 split, const aabb_struct& sub_L, const aabb_struct& sub_R, std::vector<NODE_T>& nodes, const T* data);
	virtual void assign_node_id(u16 n_i) = 0;

	template<int cutoff>
	void construct(const T* data, int n_prims_, bool(*func)(const T*) = NULL)
	{
		int n_prims = n_prims_;

		if (n_prims == 0)
			return;

		node_count = 0;


		indices.resize(n_prims);

		for (u16 i = 0; i < n_prims; i++)
			indices[i] = i;

		/*
		if (func)
		{
			u16 end_of_list = n_prims;

			for (u16 i = 0; i < end_of_list; i++)
			{
				if (!func(&data[i]))
				{
					swap_indices<INDEX_T>(i, end_of_list - 1, indices);
					end_of_list -= 1;
				}
			}

			n_prims = end_of_list;
		}*/

		int max_nodes = 2 * n_prims - 1;

		nodes.resize(max_nodes);

		aabb_struct aabb_top;


		nodes[0].n_prims = n_prims;
		nodes[0].first_prim = 0;

		for (int i = 0; i < n_prims; i++)
		{
			grow_prim(data, i, aabb_top);
		}

		nodes[0].aabbMin[0] = aabb_top.aabbMin[0];
		nodes[0].aabbMin[1] = aabb_top.aabbMin[1];
		nodes[0].aabbMin[2] = aabb_top.aabbMin[2];

		nodes[0].aabbMax[0] = aabb_top.aabbMax[0];
		nodes[0].aabbMax[1] = aabb_top.aabbMax[1];
		nodes[0].aabbMax[2] = aabb_top.aabbMax[2];

		recursive_subdivide<cutoff>(0, node_count, nodes, data);

		node_count += 1;
	}
};

template<typename T>
struct BVH_structure : BVH_structure_base<T, BVH_node, u16>
{
	virtual void assign_node_id(u16 n_i) { nodes[n_i].id = n_i; }

	void addDrawLines(LineHolder& graph) const;
	void addDrawLines(int depth, LineHolder& graph) const
	{
		if (nodes.size() > 0)
		{
			nodes[0].addDrawLines_Recursive(depth, nodes, 0, graph);
		}
	}
	void addDrawLinesLeaves(LineHolder& graph) const
	{
		if (nodes.size() > 0)
		{
			nodes[0].addDrawLines_Leaves_Recursive(nodes, graph);
		}
	}

	void addDrawLinesByIndex(const std::vector<u16>& nodes_index, LineHolder& graph) const
	{
		if (nodes.size() > 0)
		{
			for (u16 i : nodes_index)
				nodes[i].addDrawLines(graph);
		}
	}

	template<typename W>
	void intersect(const BVH_structure<W>& other, std::vector<u16>& results)
	{
		if (nodes.size() > 0 && other.nodes.size() > 0)
		{
			nodes[0].find_intersection(&other.nodes[0], 0.001, nodes, other.nodes, results);
		}

		for (u16& hit : results)
			hit = indices[hit];
	}

	template<typename W>
	void intersect_2(const BVH_structure<W>& other, std::vector<BVH_intersection_struct>& results)
	{
		if (nodes.size() > 0 && other.nodes.size() > 0)
		{
			nodes[0].find_intersection_2(&other.nodes[0], 0.001, nodes, other.nodes, results);
		}

		for (BVH_intersection_struct& hit : results)
		{
			hit.X = indices[hit.X];
			hit.Y = other.indices[hit.Y];
		}
	}

	template<typename W>
	void intersect_top_level_node(const BVH_structure<W>& other, std::vector<u16>& results)
	{
		if (nodes.size() > 0 && other.nodes.size() > 0)
		{
			nodes[0].find_intersection_with_single_node(&other.nodes[0], 0.001, nodes, results);
		}
	}

	template<typename W>
	void intersect(W& other, std::vector<u16>& results) const
	{
		if (nodes.size() > 0)
		{
			nodes[0].find_intersection_with_single_node(&other, 0.001, nodes, results);
		}

		for (u16& hit : results)
			hit = indices[hit];
	}

	template<typename W>
	void intersect_return_nodes(W& other, std::vector<u16>& results) const
	{
		if (nodes.size() > 0)
		{
			nodes[0].find_nodes_intersection_with_single_node(&other, 0.01, nodes, results);
		}
	}

	template<typename W>
	u16 find_vertex_nocheck(const W& other) const
	{
		u16 res=0;
		if (nodes.size() > 0)
		{
			nodes[0].find_single_intersection_with_single_node(&other, 0.001, nodes, &res);
		}

		return indices[res];
	}

	template<typename W>
	u16 find_vertex_node_nocheck(const W& other) const
	{
		u16 res = 0;
		if (nodes.size() > 0)
		{
			nodes[0].find_single_node_intersection_with_single_node(&other, 0.001, nodes, &res);
		}

		return res;
	}

	template<typename W>
	void intersect_nodes(const BVH_structure<W>& other, std::vector<u16>& results)
	{
		if (nodes.size() > 0 && other.nodes.size() > 0)
		{
			nodes[0].find_intersection_nodes(&other.nodes[0], 0.001, nodes, other.nodes, results);
		}
	}

	template<typename W>
	void intersect_nodes_2(const BVH_structure<W>& other, std::vector<BVH_intersection_struct>& results)
	{
		if (nodes.size() > 0 && other.nodes.size() > 0)
		{
			nodes[0].find_intersection_nodes_2(&other.nodes[0], 0.001, nodes, other.nodes, results);
		}
	}

	template<typename W>
	void intersect_nodes_inv(const BVH_structure<W>& other, std::vector<u16>& results)
	{
		if (nodes.size() > 0 && other.nodes.size() > 0)
		{
			nodes[0].find_intersection_nodes_inverse(&other.nodes[0], 0.001, nodes, other.nodes, results);
		}
	}

	void invalidate()
	{
		indices.clear();
		nodes.clear();
		node_count = 0;
	}

	bool isValid()
	{
		return node_count > 0;
	}

	int nPrims()
	{
		if (nodes.size > 0)
			return nodes[0].n_prims;
		else
			return 0;
	}

	virtual void grow_prim(const T* data, u16 i, aabb_struct& aabb) = 0;
	virtual core::vector3df prim_position(const T* data, u16 i) = 0;

	
	/*
	void rebuild_aabb_recursive(const T* data, u16 N)
	{
		nodes[N].aabbMin[0] = nodes[N].aabbMin[1] = nodes[N].aabbMin[2] = 1e30;
		nodes[N].aabbMax[0] = nodes[N].aabbMax[1] = nodes[N].aabbMax[2] = -1e30;

		if (nodes[N].isLeafNode() == false)
		{
			rebuild_aabb_recursive(data, nodes[N].left_node);
			nodes[nodes[N].left_node].grow(&nodes[N]);

			rebuild_aabb_recursive(data, nodes[N].right_node);
			nodes[nodes[N].right_node].grow(&nodes[N]);
		}
		else
		{
			aabb_struct aabb;
			for (int i = nodes[N].first_prim; i < nodes[N].first_prim + nodes[N].n_prims; i++)
			{
				grow_prim(data, indices[i], aabb);
				nodes[N].set_aabb(aabb);
			}
		}
	}*/
	/*
	template<typename T>
	void grow(T* obj) const
	{
		obj->aabbMin[0] = fmin(obj->aabbMin[0], nodes[0].aabbMin[0]);
		obj->aabbMin[1] = fmin(obj->aabbMin[1], nodes[0].aabbMin[1]);
		obj->aabbMin[2] = fmin(obj->aabbMin[2], nodes[0].aabbMin[2]);

		obj->aabbMax[0] = fmax(obj->aabbMax[0], nodes[0].aabbMax[0]);
		obj->aabbMax[1] = fmax(obj->aabbMax[1], nodes[0].aabbMax[1]);
		obj->aabbMax[2] = fmax(obj->aabbMax[2], nodes[0].aabbMax[2]);
	}*/
	/*
	core::vector3df position(const polyfold*) const { 
		return core::vector3df{ 
			nodes[0].aabbMin[0] + (nodes[0].aabbMax[0] - nodes[0].aabbMin[0]) * 0.5,
			nodes[0].aabbMin[1] + (nodes[0].aabbMax[1] - nodes[0].aabbMin[1]) * 0.5,
			nodes[0].aabbMin[2] + (nodes[0].aabbMax[2] - nodes[0].aabbMin[2]) * 0.5
		};
	}*/
};

class poly_vert;

template<typename T>
struct BVH_structure_pf : BVH_structure<T>
{
	std::vector<core::vector3df> positions;
	std::vector<aabb_struct> aabbs;

	virtual void grow_prim(const T* data, u16 i, aabb_struct& aabb)
	{
		aabbs[i].grow(&aabb);
	}

	virtual core::vector3df prim_position(const T* data, u16 i)
	{
		return positions[i];
	}

	//void build(poly_vert* verts, const T* data, int n_prims, bool(*func)(const T*) = NULL) {
	void build(poly_vert * verts, const T * data, int n_prims, bool(*func)(const T*) = NULL) {
		
		positions.resize(n_prims);
		aabbs.resize(n_prims);

		for (int i = 0; i < n_prims; i++)
		{
			positions[i] = data[i].position(verts);

			aabbs[i].clear();
			data[i].grow(&aabbs[i],verts);
		}

		BVH_structure<T>::construct<1>(data, n_prims, func);
	}
};

template<typename T>
struct BVH_structure_simple : BVH_structure<T>
{
	virtual void grow_prim(const T* data, u16 i, aabb_struct& aabb)
	{
		data[i].grow(&aabb);
	}

	virtual core::vector3df prim_position(const T* data, u16 i)
	{
		return data[i].position();
	}

	void build(const T* data, int n_prims) {

		BVH_structure<T>::construct<1>(data, n_prims);
	}
	
	u16 find(core::vector3df v)
	{
		std::vector<u16> results;
		nodes[0].find_intersection_with_single_node(v, 0.001, nodes, results);
		return results[0];
	}
};

template<typename T>
void BVH_structure<T>::addDrawLines(LineHolder& graph) const
{
	for (int i = 0; i < node_count; i++)
	{
		if (nodes[i].left_node == 0xFFFF)
		{
			vector3df v0(nodes[i].aabbMin[0], nodes[i].aabbMin[1], nodes[i].aabbMin[2]);
			vector3df v1(nodes[i].aabbMax[0], nodes[i].aabbMin[1], nodes[i].aabbMin[2]);
			vector3df v2(nodes[i].aabbMin[0], nodes[i].aabbMax[1], nodes[i].aabbMin[2]);
			vector3df v3(nodes[i].aabbMin[0], nodes[i].aabbMin[1], nodes[i].aabbMax[2]);

			vector3df v4(nodes[i].aabbMax[0], nodes[i].aabbMax[1], nodes[i].aabbMax[2]);
			vector3df v5(nodes[i].aabbMin[0], nodes[i].aabbMax[1], nodes[i].aabbMax[2]);
			vector3df v6(nodes[i].aabbMax[0], nodes[i].aabbMin[1], nodes[i].aabbMax[2]);
			vector3df v7(nodes[i].aabbMax[0], nodes[i].aabbMax[1], nodes[i].aabbMin[2]);

			graph.lines.push_back(core::line3df(v0, v1));
			graph.lines.push_back(core::line3df(v0, v2));
			graph.lines.push_back(core::line3df(v0, v3));

			graph.lines.push_back(core::line3df(v4, v5));
			graph.lines.push_back(core::line3df(v4, v6));
			graph.lines.push_back(core::line3df(v4, v7));

			graph.lines.push_back(core::line3df(v1, v7));
			graph.lines.push_back(core::line3df(v2, v5));
			graph.lines.push_back(core::line3df(v3, v6));

			graph.lines.push_back(core::line3df(v1, v6));
			graph.lines.push_back(core::line3df(v2, v7));
			graph.lines.push_back(core::line3df(v3, v5));
		}
	}
}



inline void BVH_node::addDrawLines_Recursive(int depth, std::vector<BVH_node>& nodes, int current_depth, LineHolder& graph) const
{
	if (depth == -1 || depth == current_depth)
		addDrawLines(graph);

	if (!isLeafNode())
	{
		nodes[left_node].addDrawLines_Recursive(depth, nodes, current_depth + 1, graph);
		nodes[right_node].addDrawLines_Recursive(depth, nodes, current_depth + 1, graph);
	}
}

inline void BVH_node::addDrawLines_Leaves_Recursive(std::vector<BVH_node>& nodes, LineHolder& graph) const
{
	if ( isLeafNode() )
	{
		addDrawLines(graph);
	}
	else
	{
		nodes[left_node].addDrawLines_Leaves_Recursive(nodes, graph);
		nodes[right_node].addDrawLines_Leaves_Recursive(nodes, graph);
	}
}

template<typename T, typename NODE_T, typename INDEX_T>
template<int cutoff>
void BVH_structure_base<T, NODE_T, INDEX_T>::recursive_subdivide(u16 n_i, int& N, std::vector<NODE_T>& nodes, const T* data)
{
	if (nodes[n_i].n_prims <= cutoff)
		return;

	aabb_struct sub_L;
	aabb_struct sub_R;
	int axis;
	f32 split;

	f32 best_split = find_best_split(nodes[n_i].aabbMin, nodes[n_i].aabbMax, axis, split, sub_L, sub_R, nodes[n_i].first_prim, nodes[n_i].n_prims, data);

	//if (best_split <= nodes[n_i].surface_area() * nodes[n_i].n_prims)
	if(best_split < 1e30)
	{
		bool divide = subdivide(n_i, N, axis, split, sub_L, sub_R, nodes, data);
		if (divide)
		{
			recursive_subdivide<cutoff>(nodes[n_i].left_node, N, nodes, data);
			recursive_subdivide<cutoff>(nodes[n_i].right_node, N, nodes, data);
		}
	}
}

template<typename T, typename NODE_T, typename INDEX_T>
f32 BVH_structure_base<T, NODE_T, INDEX_T>::find_best_split(const f32* aabbMin, const f32* aabbMax, int& axis, f32& split_out, aabb_struct& sub_L, aabb_struct& sub_R, u16 first_prim, u16 n_prims, const T* data)
{
	f32 best = 1e30;

	f32 len;

	int n_left, n_right;
	f32 left_cost, right_cost;
	if (n_prims > 3)
	{
		for (int j = 0; j < 3; j++)
		{
			len = (aabbMax[j] - aabbMin[j]);

			for (int k = 1; k < 4; k++)
			{
				f32 split = (len * (f32(k) / 4.0));
				n_left = n_right = 0;

				aabb_struct aabb_L, aabb_R;

				for (u16 i = first_prim; i < first_prim + n_prims; i++)
				{
					core::vector3df v = prim_position(data, indices[i]);
					f32 t = ((f32*)(&v))[j] - aabbMin[j];

					if (t < split + 0.001)
					{
						n_left++;
						grow_prim(data, indices[i], aabb_L);
					}
					else
					{
						n_right++;
						grow_prim(data, indices[i], aabb_R);
					}
				}

				left_cost = n_left * aabb_L.surface_area();
				right_cost = n_right * aabb_R.surface_area();

				if (left_cost + right_cost < best && n_left !=0 && n_right != 0)
				{
					axis = j;
					split_out = split;
					sub_L = aabb_L;
					sub_R = aabb_R;
					best = left_cost + right_cost;
				}
			}
		}
	}/*
	else if( n_prims == 2)
	{
		core::vector3df v0 = prim_position(data, indices[first_prim]);
		core::vector3df v1 = prim_position(data, indices[first_prim + 1]);

		for (int j = 0; j < 3; j++)
		{
			if (((f32*)(&v1))[j] - ((f32*)(&v0))[j] > 0.001)
			{
				f32 split = ((f32*)(&v0))[j];

				aabb_struct aabb_L, aabb_R;

				grow_prim(data, indices[first_prim], aabb_L);
				grow_prim(data, indices[first_prim+1], aabb_R);

				left_cost = aabb_L.surface_area();
				right_cost = aabb_R.surface_area();

				axis = j;
				split_out = split - aabbMin[j];
				sub_L = aabb_L;
				sub_R = aabb_R;
				best = left_cost + right_cost;

				return best;
			}
			else if (((f32*)(&v0))[j] - ((f32*)(&v1))[j] > 0.001)
			{
				f32 split = ((f32*)(&v1))[j];

				aabb_struct aabb_L, aabb_R;

				grow_prim(data, indices[first_prim], aabb_R);
				grow_prim(data, indices[first_prim + 1], aabb_L);

				left_cost = aabb_L.surface_area();
				right_cost = aabb_R.surface_area();

				axis = j;
				split_out = split - aabbMin[j];
				sub_L = aabb_L;
				sub_R = aabb_R;
				best = left_cost + right_cost;

				return best;
			}
		}
	}*/
	else
	{
		for (int j = 0; j < 3; j++)
		{
			for (u16 k = first_prim; k < first_prim + n_prims; k++)
			{
				core::vector3df v0 = prim_position(data, indices[k]);
				f32 split = ((f32*)(&v0))[j];

				n_left = n_right = 0;

				aabb_struct aabb_L, aabb_R;

				for (u16 i = first_prim; i < first_prim + n_prims; i++)
				{
					core::vector3df v = prim_position(data, indices[i]);
					f32 t = ((f32*)(&v))[j];

					if (t < split + 0.001)
					{
						n_left++;
						grow_prim(data, indices[i], aabb_L);
					}
					else
					{
						n_right++;
						grow_prim(data, indices[i], aabb_R);
					}
				}

				left_cost = n_left * aabb_L.surface_area();
				right_cost = n_right * aabb_R.surface_area();

				if (left_cost + right_cost < best && n_left !=0 && n_right !=0)
				{
					axis = j;
					split_out = split - aabbMin[j];
					sub_L = aabb_L;
					sub_R = aabb_R;
					best = left_cost + right_cost;
				}
			}
		}
	}

	return best;
}

template<typename T, typename NODE_T, typename INDEX_T>
bool BVH_structure_base<T, NODE_T, INDEX_T>::subdivide(u16 n_i, int& N, int axis, f32 split, const aabb_struct& sub_L, const aabb_struct& sub_R, std::vector<NODE_T>& nodes, const T* data)
{
	int j = nodes[n_i].n_prims;
	int count = 0;

	for (int i = nodes[n_i].first_prim; i < nodes[n_i].first_prim + j;)
	{
		core::vector3df v = prim_position(data, indices[i]);
		f32 t = ((f32*)(&v))[axis] - nodes[n_i].aabbMin[axis];

		if (t < split + 0.001)
		{
			count++;
			i++;
		}
		else
		{
			swap_indices<INDEX_T>(i, nodes[n_i].first_prim + j - 1, indices);
			j -= 1;
		}
	}

	if (count == 0 || count == nodes[n_i].n_prims)
		return false;

	u16 left_i = nodes[n_i].left_node = ++N;
	u16 right_i = nodes[n_i].right_node = ++N;

	nodes[left_i].first_prim = nodes[n_i].first_prim;
	nodes[left_i].n_prims = count;
	nodes[left_i].left_node = 0xFFFF;
	nodes[left_i].right_node = 0xFFFF;
	nodes[left_i].aabbMin[0] = sub_L.aabbMin[0];
	nodes[left_i].aabbMin[1] = sub_L.aabbMin[1];
	nodes[left_i].aabbMin[2] = sub_L.aabbMin[2];
	nodes[left_i].aabbMax[0] = sub_L.aabbMax[0];
	nodes[left_i].aabbMax[1] = sub_L.aabbMax[1];
	nodes[left_i].aabbMax[2] = sub_L.aabbMax[2];
	assign_node_id(left_i);
	//nodes[left_i].id = left_i;

	nodes[right_i].first_prim = nodes[n_i].first_prim + count;
	nodes[right_i].n_prims = nodes[n_i].n_prims - count;
	nodes[right_i].left_node = 0xFFFF;
	nodes[right_i].right_node = 0xFFFF;
	nodes[right_i].aabbMin[0] = sub_R.aabbMin[0];
	nodes[right_i].aabbMin[1] = sub_R.aabbMin[1];
	nodes[right_i].aabbMin[2] = sub_R.aabbMin[2];
	nodes[right_i].aabbMax[0] = sub_R.aabbMax[0];
	nodes[right_i].aabbMax[1] = sub_R.aabbMax[1];
	nodes[right_i].aabbMax[2] = sub_R.aabbMax[2];
	assign_node_id(right_i);
	//nodes[right_i].id = right_i;

	return true;
}



#endif

