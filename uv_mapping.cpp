
#include <irrlicht.h>
#include "csg_classes.h"
#include "uv_mapping.h"

using namespace std;

void apply_transform_to_uvs(MeshNode_Interface* mesh_node, const std::vector<int>& surface, int uv_type, matrix4 mat)
{
	for (int b_i : surface)
	{
		MeshBuffer_Chunk chunk = mesh_node->get_mesh_buffer_by_face(b_i);
		std::cout << b_i << ":\n";
		if (chunk.buffer)
		{
			vector<u16> indices;

			for (int i = chunk.begin_i; i < chunk.end_i; i++)
			{
				bool b = false;
				u16 idx = chunk.buffer->getIndices()[i];
				for (u16 j : indices)
				{
					if (j == idx)
					{
						b = true;
						break;
					}
				}

				if (!b)
					indices.push_back(idx);
			}

			for (u16 idx : indices)
			{
				video::S3DVertex2TCoords* vtx = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx];

				f32 V4[4] = { 0,0,0,1 };

				if (uv_type == MAP_UVS_TEXTURE)
				{
					V4[0] = vtx->TCoords.X;
					V4[1] = vtx->TCoords.Y;

					mat.multiplyWith1x4Matrix(V4);

					vtx->TCoords.set(V4[0], V4[1]);
				}
				else if (uv_type == MAP_UVS_LIGHTMAP)
				{
					V4[0] = vtx->TCoords2.X;
					V4[1] = vtx->TCoords2.Y;

					mat.multiplyWith1x4Matrix(V4);

					vtx->TCoords2.set(V4[0], V4[1]);

					std::cout << V4[0]<< "," << V4[1] << "\n";
				}
			}
		}
	}
}