#pragma once

#include <vector>
#include <irrlicht.h>
#include "BVH.h"
#include "vkModel.h"
#include "csg_classes.h"

struct BVH_node_gpu
{
	alignas(16) f32 aabbMin[4];
	alignas(16) f32 aabbMax[4];
	alignas(16) u32 packing[4];

	u32 left_node = 0xFFFF;
	u32 right_node = 0xFFFF;

	u32 first_prim;
	u32 n_prims;

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

	bool isLeafNode() const {
		return (left_node == 0xFFFF && right_node == 0xFFFF);
	}
	inline void addDrawLines(LineHolder& graph) const;
	inline void addDrawLines_Recursive(int depth, const std::vector<BVH_node_gpu>& nodes, int current_depth, LineHolder& graph) const;

};

struct BVH_structure_triangles : BVH_structure_base<triangle_b, BVH_node_gpu, u32>
{
	std::vector<core::vector3df> positions;
	std::vector<aabb_struct> aabbs;

	virtual void grow_prim(const triangle_b* data, u16 i, aabb_struct& aabb)
	{
		aabbs[i].grow(&aabb);
	}

	virtual core::vector3df prim_position(const triangle_b* data, u16 i)
	{
		return positions[i];
	}

	virtual void assign_node_id(u16 n_i) {}

	void build(const aligned_vec3* verts, const triangle_b* data, int n_prims, bool(*func)(const triangle_b*) = NULL) {

		positions.resize(n_prims);
		aabbs.resize(n_prims);

		for (int i = 0; i < n_prims; i++)
		{
			positions[i] = data[i].position(verts);

			aabbs[i].clear();
			data[i].grow(&aabbs[i], verts);
		}

		BVH_structure_base<triangle_b, BVH_node_gpu, u32>::construct<2>(data, n_prims, func);
	}

	void addDrawLines(int depth, LineHolder& graph) const
	{
		if (nodes.size() > 0)
		{
			nodes[0].addDrawLines_Recursive(depth, nodes, 0, graph);
		}
	}

	inline void addDrawLinesPrims(const aligned_vec3* verts, const aligned_uint* indices, int node_i, LineHolder& graph) const;
};


void BVH_node_gpu::addDrawLines(LineHolder& graph) const
{
	vector3df v0(aabbMin[0], aabbMin[1], aabbMin[2]);
	vector3df v1(aabbMax[0], aabbMin[1], aabbMin[2]);
	vector3df v2(aabbMin[0], aabbMax[1], aabbMin[2]);
	vector3df v3(aabbMin[0], aabbMin[1], aabbMax[2]);

	vector3df v4(aabbMax[0], aabbMax[1], aabbMax[2]);
	vector3df v5(aabbMin[0], aabbMax[1], aabbMax[2]);
	vector3df v6(aabbMax[0], aabbMin[1], aabbMax[2]);
	vector3df v7(aabbMax[0], aabbMax[1], aabbMin[2]);


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

void BVH_node_gpu::addDrawLines_Recursive(int depth, const std::vector<BVH_node_gpu>& nodes, int current_depth, LineHolder& graph) const
{
	if (depth == -1 || depth == current_depth)
		addDrawLines(graph);

	if (!isLeafNode())
	{
		nodes[left_node].addDrawLines_Recursive(depth, nodes, current_depth + 1, graph);
		nodes[right_node].addDrawLines_Recursive(depth, nodes, current_depth + 1, graph);
	}
}

void BVH_structure_triangles::addDrawLinesPrims(const aligned_vec3* verts, const aligned_uint* indices, int node_i, LineHolder& graph) const
{
	if (nodes[node_i].left_node == 0xFFFF && nodes[node_i].right_node == 0xFFFF)
	{
		for (int j = 0; j < std::min(4u, nodes[node_i].n_prims); j++)
		{
			vector3df v0 = verts[indices[nodes[node_i].packing[j] * 3].x].V;
			vector3df v1 = verts[indices[nodes[node_i].packing[j] * 3 + 1].x].V;
			vector3df v2 = verts[indices[nodes[node_i].packing[j] * 3 + 2].x].V;

			graph.lines.push_back(line3df(v0, v1));
			graph.lines.push_back(line3df(v1, v2));
			graph.lines.push_back(line3df(v2, v0));

		}
	}
}
