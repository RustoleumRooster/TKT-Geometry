
#include <irrlicht.h>
#include "reflection.h"
#include "reflected_nodes.h"
#include "CameraPanel.h"
#include "utils.h"
#include "geometry_scene.h"
#include "GUI_tools.h"
#include "my_reflected_nodes.h"
#include "USceneNode.h"
#include "my_nodes.h"

using namespace irr;
using namespace gui;

extern IrrlichtDevice* device;
extern SceneCoordinator* gs_coordinator;

video::E_MATERIAL_TYPE Reflected_SceneNode::base_material_type = video::EMT_SOLID;
video::E_MATERIAL_TYPE Reflected_SceneNode::special_material_type = video::EMT_SOLID;


//============================================================================
//  Unique Scene Node
//

USceneNode::USceneNode(ISceneNode* parent, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
    ISceneNode(parent,smgr,id,pos), my_UID(random_number())
{
}


//============================================================================
//  Reflected_SceneNode
//

REFLECT_STRUCT2_BEGIN(Reflected_SceneNode)
    ALIAS("Node")
    PLACEABLE(false)
    REFLECT_STRUCT2_MEMBER(my_UID)
    REFLECT_STRUCT2_MEMBER(ID)
    REFLECT_STRUCT2_MEMBER(Location)
    REFLECT_STRUCT2_MEMBER(Rotation)
REFLECT_STRUCT2_END()

Reflected_SceneNode::Reflected_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos)
    : USceneNode(parent,smgr, id, pos), geo_scene(geo_scene)
{
    m_unique_color = makeUniqueColor();

    video::IImage* img = makeSolidColorImage(SceneManager->getVideoDriver(),m_unique_color);
    m_color_tex = SceneManager->getVideoDriver()->addTexture(irr::io::path("hit_color"), img);
    img->drop();
}

void Reflected_SceneNode::OnRegisterSceneNode()
{
    ISceneNode::OnRegisterSceneNode();
}

void Reflected_SceneNode::preEdit()
{
    this->Location = this->getPosition();
    this->Rotation = this->getRotation();
}

void Reflected_SceneNode::postEdit()
{
    this->setPosition(this->Location);
    this->setRotation(this->Rotation);

    total_bounding_box = getMyBoundingBox();

    for (Reflected_SceneNode* n : input_node_ptrs)
    {
        total_bounding_box.addInternalBox(n->getMyBoundingBox());
    }
}

void Reflected_SceneNode::connect_input(Reflected_SceneNode* node)
{
    bool b = false;

    for (u64 uid : input_nodes)
    {
        if (uid == node->UID())
            b = true;
    }

    if (b == false)
    {
        input_nodes.push_back(node->UID());
        rebuild_input_ptrs_list();

        on_connect_node(node);
    }
}

void Reflected_SceneNode::disconnect(Reflected_SceneNode* node) 
{
    std::vector<u64> new_inputs;

    for (u64 uid : input_nodes)
    {
        if (uid != node->UID())
            new_inputs.push_back(uid);

        if (uid == node->UID())
        {
            on_disconnect_node(node);
        }
    }

    input_nodes = new_inputs;

    rebuild_input_ptrs_list();
}

void Reflected_SceneNode::rebuild_input_ptrs_list()
{
    input_node_ptrs.clear();
    for (u64 uid : input_nodes)
    {
        Reflected_SceneNode* node = geo_scene->get_reflected_node_by_uid(uid);
        if (node)
            input_node_ptrs.push_back(node);
    }
}

const core::aabbox3df& Reflected_SceneNode::getBoundingBox() const
{
    return total_bounding_box;
}

void Reflected_SceneNode::draw_box(video::IVideoDriver* driver, core::aabbox3df b)
{
    video::SMaterial someMaterial;
    someMaterial.Lighting = false;
    someMaterial.Thickness = 1.0;
    someMaterial.MaterialType = video::EMT_SOLID;

    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(someMaterial);

    core::vector3df* edges = new core::vector3df[8];
    b.getEdges(edges);


    driver->draw3DLine(edges[0],edges[1],video::SColor(128,255,255,255));
    driver->draw3DLine(edges[0],edges[2],video::SColor(128,255,255,255));
    driver->draw3DLine(edges[0],edges[4],video::SColor(128,255,255,255));
    driver->draw3DLine(edges[2],edges[3],video::SColor(128,255,255,255));
    driver->draw3DLine(edges[2],edges[6],video::SColor(128,255,255,255));
    driver->draw3DLine(edges[6],edges[7],video::SColor(128,255,255,255));
    driver->draw3DLine(edges[6],edges[4],video::SColor(128,255,255,255));
    driver->draw3DLine(edges[4],edges[5],video::SColor(128,255,255,255));
    driver->draw3DLine(edges[1],edges[5],video::SColor(128,255,255,255));
    driver->draw3DLine(edges[5],edges[7],video::SColor(128,255,255,255));
    driver->draw3DLine(edges[3],edges[1],video::SColor(128,255,255,255));
    driver->draw3DLine(edges[3],edges[7],video::SColor(128,255,255,255));

    delete[] edges;
}

void Reflected_SceneNode::draw_inputs(video::IVideoDriver* driver)
{
    if (input_nodes.size() == 0)
        return;

    video::SMaterial someMaterial;
    someMaterial.Lighting = false;
    someMaterial.Thickness = 1.0;
    someMaterial.MaterialType = video::EMT_SOLID;

    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(someMaterial);

    for (Reflected_SceneNode* node : input_node_ptrs)
    {
        driver->draw3DLine(this->Location, node->Location, video::SColor(128, 40, 150, 255));
    }
}

const core::aabbox3df& Reflected_SceneNode::getMyBoundingBox() const
{
    return core::aabbox3df{};
}

void Reflected_SceneNode::draw_arrow(video::IVideoDriver* driver, core::vector3df v, core::vector3df rot)
{
    int len=80;

    core::vector3df z = core::vector3df(0,0,1);
    core::matrix4 M;
    M.setRotationDegrees(rot);
    M.rotateVect(z);

    video::SMaterial someMaterial;
    someMaterial.Lighting = false;
    someMaterial.Thickness = 1.0;
    someMaterial.MaterialType = video::EMT_SOLID;

    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(someMaterial);

    core::vector3df x = z == core::vector3df(0,1,0) ? z.crossProduct(core::vector3df(1,0,0)) : z.crossProduct(core::vector3df(0,1,0));
    core::vector3df y = x.crossProduct(z);

    core::vector3df v0 = v+z*len;
    driver->draw3DLine(v,v0,video::SColor(128,255,255,255));

    core::vector3df v1 = v+z*len*0.75+x*len*0.2;
    driver->draw3DLine(v0,v1,video::SColor(128,255,255,255));

    v1 = v+z*len*0.75-x*len*0.2;
    driver->draw3DLine(v0,v1,video::SColor(128,255,255,255));

    v1 = v+z*len*0.75+y*len*0.2;
    driver->draw3DLine(v0,v1,video::SColor(128,255,255,255));

    v1 = v+z*len*0.75-y*len*0.2;
    driver->draw3DLine(v0,v1,video::SColor(128,255,255,255));
}

vector3df Reflected_SceneNode::get_direction_vector()
{
    vector3df z = vector3df(0, 0, 1);
    matrix4 M;
    M.setRotationDegrees(getRotation());
    M.rotateVect(z);

    return z;
}

void Reflected_SceneNode::translate(core::matrix4 M)
{
    core::vector3df p = this->getPosition();
    M.translateVect(p);
    this->setPosition(p);
    preEdit();
}

void Reflected_SceneNode::rotate(core::vector3df r)
{
    this->setRotation(r);
    preEdit();
}

//============================================================================
//  Reflected_Sprite_SceneNode
//

REFLECT_STRUCT2_BEGIN(Reflected_Sprite_SceneNode)
    ALIAS("Sprite")
    PLACEABLE(false)
    INHERIT_FROM(Reflected_SceneNode)
REFLECT_STRUCT2_END()

void Reflected_Sprite_SceneNode::OnRegisterSceneNode()
{
    SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT);
    Reflected_SceneNode::OnRegisterSceneNode();
}

f32 Reflected_SceneNode::getDistanceFromCamera(TestPanel* viewPanel)
{
    core::vector3df cam_pos = viewPanel->getCamera()->getAbsolutePosition();
    return cam_pos.getDistanceFrom(getPosition());
}

Reflected_Sprite_SceneNode::Reflected_Sprite_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos)
    : Reflected_SceneNode(parent, geo_scene, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("sun.jpg");

    Buffer = new scene::SMeshBuffer();
    Buffer->Material.Lighting = false;
    Buffer->Material.setTexture(0,m_texture);
    //Buffer->Material.MaterialType = video::EMT_TRANSPARENT_ADD_COLOR;
    Buffer->Material.MaterialType = video::EMT_SOLID;
    //MaterialType = video::EMT_SOLID;

    Buffer->Vertices.set_used(4);
    Buffer->Indices.set_used(6);

    Buffer->Indices[0] = (u16)0;
    Buffer->Indices[1] = (u16)2;
    Buffer->Indices[2] = (u16)1;
    Buffer->Indices[3] = (u16)0;
    Buffer->Indices[4] = (u16)3;
    Buffer->Indices[5] = (u16)2;

    Buffer->BoundingBox.addInternalPoint(pos);

    this->UnSelect();
}


core::rect<s32> Reflected_Sprite_SceneNode::GetVisibleRectangle(TestPanel* viewPanel)
{
	ICameraSceneNode* camera = viewPanel->getCamera();

    const core::matrix4 &m = camera->getViewFrustum()->getTransform( video::ETS_VIEW );

    f32 f;

    f = 0.5f * icon_size;//particle.size.Width;
    const core::vector3df horizontal ( m[0] * f, m[4] * f, m[8] * f );

    f = -0.5f * icon_size;//particle.size.Height;
    const core::vector3df vertical ( m[1] * f, m[5] * f, m[9] * f );

    core::vector3df c = getPosition() - horizontal - vertical;
    core::vector3df d = getPosition() + horizontal + vertical;

    core::vector2di C,D;

    viewPanel->GetScreenCoords(c,C);
    viewPanel->GetScreenCoords(d,D);

    core::rect<s32> ret{C,D};

    return ret;
}

void Reflected_Sprite_SceneNode::Select()
{
    Buffer->Material.MaterialType = Reflected_SceneNode::special_material_type;
    bSelected=true;
}

void Reflected_Sprite_SceneNode::UnSelect()
{
    Buffer->Material.MaterialType = Reflected_SceneNode::base_material_type;
    bSelected=false;
}

void Reflected_Sprite_SceneNode::set_buffer()
{
    ICameraSceneNode* camera = SceneManager->getActiveCamera();

    if(camera)
    {
         const core::matrix4 &m = camera->getViewFrustum()->getTransform( video::ETS_VIEW );
	const core::vector3df view ( -m[2], -m[6] , -m[10] );

	f32 f;

    f = 0.5f * icon_size;//particle.size.Width;
    const core::vector3df horizontal ( m[0] * f, m[4] * f, m[8] * f );

    f = -0.5f * icon_size;//particle.size.Height;
    const core::vector3df vertical ( m[1] * f, m[5] * f, m[9] * f );

    Buffer->Vertices[0].Pos = getPosition() + horizontal + vertical;
    Buffer->Vertices[0].Color = video::SColor(255,255,255,255);
    Buffer->Vertices[0].Normal = view;
    Buffer->Vertices[0].TCoords = core::vector2df(1,1);

    Buffer->Vertices[1].Pos = getPosition() + horizontal - vertical;
    Buffer->Vertices[1].Color = video::SColor(255,255,255,255);
    Buffer->Vertices[1].Normal = view;
    Buffer->Vertices[1].TCoords = core::vector2df(1,0);

    Buffer->Vertices[2].Pos = getPosition() - horizontal - vertical;
    Buffer->Vertices[2].Color = video::SColor(255,255,255,255);
    Buffer->Vertices[2].Normal = view;
    Buffer->Vertices[2].TCoords = core::vector2df(0,0);

    Buffer->Vertices[3].Pos = getPosition() - horizontal + vertical;
    Buffer->Vertices[3].Color = video::SColor(255,255,255,255);
    Buffer->Vertices[3].Normal = view;
    Buffer->Vertices[3].TCoords = core::vector2df(0,1);

    }
}

void Reflected_Sprite_SceneNode::render()
{
    video::IVideoDriver* driver = SceneManager->getVideoDriver();
	ICameraSceneNode* camera = SceneManager->getActiveCamera();

	if (!camera || !driver)
		return;

    set_buffer();

    core::matrix4 mat;

	driver->setTransform(video::ETS_WORLD, mat);
	driver->setMaterial(Buffer->Material);

    //driver->draw3DBox(
    //    getBoundingBox(),
    //    video::SColor(255, 190, 128, 128));

	driver->drawVertexPrimitiveList(Buffer->getVertices(), 4,
		Buffer->getIndices(), 2, video::EVT_STANDARD, EPT_TRIANGLES,Buffer->getIndexType());

    if(bSelected)
        draw_arrow(driver,getPosition(),getRotation());

    draw_inputs(driver);

}

void Reflected_Sprite_SceneNode::render_special(video::SMaterial& material)
{
    video::IVideoDriver* driver = SceneManager->getVideoDriver();
	ICameraSceneNode* camera = SceneManager->getActiveCamera();

	if (!camera || !driver)
		return;

    set_buffer();

    core::matrix4 mat;

	driver->setTransform(video::ETS_WORLD, mat);

	material.setTexture(0,m_color_tex);
	driver->setMaterial(material);

	driver->drawVertexPrimitiveList(Buffer->getVertices(), 4,
		Buffer->getIndices(), 2, video::EVT_STANDARD, EPT_TRIANGLES,Buffer->getIndexType());

}

const core::aabbox3df& Reflected_Sprite_SceneNode::getMyBoundingBox() const
{
    return Buffer->BoundingBox;
}

//============================================================================
//  Reflected_Model_SceneNode
//

REFLECT_STRUCT2_BEGIN(Reflected_Model_SceneNode)
    ALIAS("Model")
    REFLECT_STRUCT2_MEMBER(Scale)
    INHERIT_FROM(Reflected_SceneNode)
REFLECT_STRUCT2_END()

Reflected_Model_SceneNode::Reflected_Model_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos)
: Reflected_SceneNode(parent, geo_scene, smgr, id, pos)
{

    Mesh = SceneManager->getMesh("baby_shark.3ds");


    //Set all the vertices to white
    for (u32 i=0; i<Mesh->getMeshBufferCount(); ++i)
		{
			scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
			for(int j=0;j<mb->getVertexCount();j++)
                ((video::S3DVertex*)mb->getVertices())[j].Color = video::SColor(255,255,255,255);
		}

    m_texture = SceneManager->getVideoDriver()->getTexture("wall.bmp");

    //setMaterialTexture(0, SceneManager->getVideoDriver()->getTexture("wall.bmp"));
    //setMaterialTexture(0, tex);
	//setMaterialFlag(video::EMF_LIGHTING, false);
	//setMaterialType(video::EMT_SOLID);

	((scene::SMeshBuffer*)Mesh->getMeshBuffer(0))->Material.MaterialType = video::EMT_SOLID;
	((scene::SMeshBuffer*)Mesh->getMeshBuffer(0))->Material.Lighting = false;

	//((scene::SMeshBuffer*)Mesh->getMeshBuffer(0))->Material.setTexture(0,tex);
	//((scene::SMeshBuffer*)Mesh->getMeshBuffer(0))->Material.setTexture(0,);
}

const core::aabbox3df& Reflected_Model_SceneNode::getMyBoundingBox() const
{
    return Box;
}

void Reflected_Model_SceneNode::OnRegisterSceneNode()
{
    SceneManager->registerNodeForRendering(this, scene::ESNRP_SOLID);

    Reflected_SceneNode::OnRegisterSceneNode();
}

void Reflected_Model_SceneNode::render()
{
    video::IVideoDriver* driver = SceneManager->getVideoDriver();

	if (!Mesh || !driver)
		return;

    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

    for (u32 i=0; i<Mesh->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
		if (mb)
		{
			//const video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];

            video::SMaterial& material = mb->getMaterial();

            if(bSelected)
                material.MaterialType = Reflected_SceneNode::special_material_type;
            else
                material.MaterialType = video::EMT_SOLID;

            material.setTexture(0,m_texture);

            driver->setMaterial(material);
            driver->drawMeshBuffer(mb);
		}
	}

    draw_inputs(driver);
}

void Reflected_Model_SceneNode::render_special(video::SMaterial& material)
{
    video::IVideoDriver* driver = SceneManager->getVideoDriver();

	if (!Mesh || !driver)
		return;

    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

    for (u32 i=0; i<Mesh->getMeshBufferCount(); ++i)
		{
			scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
			if (mb)
			{
                material.setTexture(0,m_color_tex);

                driver->setMaterial(material);
                driver->drawMeshBuffer(mb);
			}
		}
}

void Reflected_Model_SceneNode::preEdit()
{
    Scale = getScale();
    Reflected_SceneNode::preEdit();
}

void Reflected_Model_SceneNode::postEdit()
{
    setScale(Scale);
    Reflected_SceneNode::postEdit();
}

void Reflected_Model_SceneNode::setUnlit(bool unlit)
{
    //this->Mesh->setMaterialFlag(video::EMF_LIGHTING,!unlit);
}

//===================================================================
// Reflected MeshBuffer Node
//

Reflected_MeshBuffer_SceneNode::Reflected_MeshBuffer_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_Sprite_SceneNode(parent, geo_scene, smgr, id, pos), element_id(0), face_id(0)
{
    m_texture = device->getVideoDriver()->getTexture("color_square_icon.png");
    Buffer->Material.setTexture(0, m_texture);
}

bool Reflected_MeshBuffer_SceneNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    return false;
}

void Reflected_MeshBuffer_SceneNode::endScene()
{
    restore_original_texture();
}

void Reflected_MeshBuffer_SceneNode::restore_original_texture()
{
    polyfold* pf = geo_scene->geoNode()->get_total_geometry();

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        //if (pf->faces[f_i].uid == this->face_uid)
        if (pf->faces[f_i].element_id == this->element_id &&
            pf->faces[f_i].face_id == this->face_id)
        {
            int face_i = pf->faces[f_i].face_id;
            //geo_element* element = geo_scene->geoNode()->get_element_by_id(pf->faces[f_i].element_id);

            poly_surface* surface = geo_scene->geoNode()->surface_by_n(face_i);

            video::ITexture* texture = device->getVideoDriver()->getTexture(surface->texture_name.c_str());
            
            MeshBuffer_Chunk chunk = geo_scene->geoNode()->final_meshnode_interface.get_mesh_buffer_by_face(f_i);

            IMeshBuffer* buffer = chunk.buffer;

            if (buffer)
                buffer->getMaterial().setTexture(0, texture);

            //TODO: copy materials ?????
        }
    }

    //called by caller
    //geoNode()->getMeshNode()->copyMaterials();
}

IMeshBuffer* Reflected_MeshBuffer_SceneNode::get_mesh_buffer()
{
    polyfold* pf = geo_scene->geoNode()->get_total_geometry();

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
       // if (pf->faces[f_i].uid == get_uid())
        if (pf->faces[f_i].element_id == this->element_id &&
            pf->faces[f_i].face_id == this->face_id)
        {
            MeshBuffer_Chunk chunk = geo_scene->geoNode()->final_meshnode_interface.get_mesh_buffer_by_face(f_i);

            IMeshBuffer* buffer = chunk.buffer;

            return buffer;
        }
    }

    return NULL;
}

void Reflected_MeshBuffer_SceneNode::set_face(int element, int face)
{
    face_id = face;
    element_id = element;
}

/*
void Reflected_MeshBuffer_SceneNode::set_uid(u64 uid)
{
    face_uid = uid;
}

u64 Reflected_MeshBuffer_SceneNode::get_uid()
{
    return face_uid;
}
*/

//=================================================
//Reflected_MeshBuffer_Sky_SceneNode
//

Reflected_MeshBuffer_Sky_SceneNode::Reflected_MeshBuffer_Sky_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_MeshBuffer_SceneNode(parent,geo_scene,smgr,id,pos)
{
    m_texture = device->getVideoDriver()->getTexture("color_square_icon_nogood.png");
    Buffer->Material.setTexture(0, m_texture);
}

Reflected_MeshBuffer_Sky_SceneNode::~Reflected_MeshBuffer_Sky_SceneNode()
{
    
}

void Reflected_MeshBuffer_Sky_SceneNode::set_connected(bool b)
{
    this->is_connected = b;

    if (this->is_connected)
    {
        m_texture = device->getVideoDriver()->getTexture("color_square_icon_good.png");
        Buffer->Material.setTexture(0, m_texture);
    }
    else
    {
        m_texture = device->getVideoDriver()->getTexture("color_square_icon_nogood.png");
        Buffer->Material.setTexture(0, m_texture);
    }
}

void Reflected_MeshBuffer_Sky_SceneNode::postEdit()
{
    Reflected_MeshBuffer_SceneNode::postEdit();
}

void Reflected_MeshBuffer_Sky_SceneNode::connect_sky_sceneNode(MySkybox_SceneNode* node_regular, MySkybox_SceneNode* node_underwater)
{
    polyfold* pf = geo_scene->geoNode()->get_total_geometry();

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        //if (pf->faces[f_i].uid == this->get_uid())
        if (pf->faces[f_i].element_id == this->element_id &&
            pf->faces[f_i].face_id == this->face_id)
        {
            MeshBuffer_Chunk chunk = geo_scene->geoNode()->final_meshnode_interface.get_mesh_buffer_by_face(f_i);

            IMeshBuffer* buffer = chunk.buffer;

            node_regular->attach_to_buffer(buffer);
            node_underwater->attach_to_buffer(buffer);
        }
    }

    //called by caller
    //geoNode()->getMeshNode()->copyMaterials();
}

//=================================================
//Reflected_MeshBuffer_Water_SceneNode
//

Reflected_MeshBuffer_Water_SceneNode::Reflected_MeshBuffer_Water_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_MeshBuffer_SceneNode(parent, geo_scene, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("color_square_icon_nogood.png");
    Buffer->Material.setTexture(0, m_texture);
}

Reflected_MeshBuffer_Water_SceneNode::~Reflected_MeshBuffer_Water_SceneNode()
{
    //gs_coordinator->set_skyox_dirty();
}

void Reflected_MeshBuffer_Water_SceneNode::postEdit()
{
    //gs_coordinator->set_skyox_dirty();

    Reflected_MeshBuffer_SceneNode::postEdit();
}

void Reflected_MeshBuffer_Water_SceneNode::connect_water_sceneNode(WaterSurface_SceneNode* node)
{
    polyfold* pf = geo_scene->geoNode()->get_total_geometry();

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        //if (pf->faces[f_i].uid == this->get_uid())
        if (pf->faces[f_i].element_id == this->element_id &&
            pf->faces[f_i].face_id == this->face_id)
        {
            MeshBuffer_Chunk chunk = geo_scene->geoNode()->final_meshnode_interface.get_mesh_buffer_by_face(f_i);

            IMeshBuffer* buffer = chunk.buffer;

            node->attach_to_buffer(buffer);
            node->set_z_depth(this->getAbsolutePosition().Y);

            return;
        }
    }

    //called by caller
    //geoNode()->getMeshNode()->copyMaterials();
}

void Reflected_MeshBuffer_Water_SceneNode::set_connected(bool b)
{
    this->is_connected = b;

    if (this->is_connected)
    {
        m_texture = device->getVideoDriver()->getTexture("color_square_icon_good.png");
        Buffer->Material.setTexture(0, m_texture);
    }
    else
    {
        m_texture = device->getVideoDriver()->getTexture("color_square_icon_nogood.png");
        Buffer->Material.setTexture(0, m_texture);
    }
}


//
//============================================================================
//  REFLECTED NODE FACTORY CLASS
//
//

namespace reflect 
{
    TypeDescriptor_SN_Struct::TypeDescriptor_SN_Struct(void (*init)(TypeDescriptor_Struct*)) : reflect::TypeDescriptor_Struct(init)
    {
        Reflected_SceneNode_Factory::addType(this);
    }
}

void Reflected_SceneNode_Factory::addType(reflect::TypeDescriptor_SN_Struct* typeDesc)
{
    SceneNode_Types.push_back(typeDesc);
}

int Reflected_SceneNode_Factory::getNumTypes()
{
    return SceneNode_Types.size();
}

reflect::TypeDescriptor_Struct* Reflected_SceneNode_Factory::getNodeTypeDescriptorByName(std::string name)
{
    for (reflect::TypeDescriptor_Struct* typeDesc : SceneNode_Types)
    {
        if (typeDesc->name == name)
            return typeDesc;
    }
    return NULL;
}

Reflected_SceneNode* Reflected_SceneNode_Factory::CreateNodeByTypeName(std::string name, USceneNode* parent, geometry_scene* geo_scene, ISceneManager* smgr)
{
    for (reflect::TypeDescriptor_Struct* typeDesc : SceneNode_Types)
    {
        if (typeDesc->name == name)
            return ((reflect::TypeDescriptor_SN_Struct*)typeDesc)->create_func(parent, geo_scene, smgr, -1, core::vector3df(0, 0, 0));
    }
    return NULL;
}

int Reflected_SceneNode_Factory::getTypeNum(reflect::TypeDescriptor_Struct* td)
{
    for (int i = 0; i < SceneNode_Types.size(); i++)
    {
        if (SceneNode_Types[i] == td)
            return i;
    }
    return 0;
}


REFLECT_STRUCT_BEGIN(reflect::vector2)
    REFLECT_STRUCT_MEMBER(X)
    REFLECT_STRUCT_MEMBER(Y)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(reflect::vector2i)
    REFLECT_STRUCT_MEMBER(X)
    REFLECT_STRUCT_MEMBER(Y)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(reflect::vector2u)
    REFLECT_STRUCT_MEMBER(X)
    REFLECT_STRUCT_MEMBER(Y)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(reflect::vector3)
    REFLECT_STRUCT_MEMBER(X)
    REFLECT_STRUCT_MEMBER(Y)
    REFLECT_STRUCT_MEMBER(Z)
REFLECT_STRUCT_END()

REFLECT_MULTI_STRUCT_BEGIN(reflect::SomeOptions)
    REFLECT_MULTI_STRUCT_LABEL("default")
    REFLECT_MULTI_STRUCT_LABEL("Option A")
    REFLECT_MULTI_STRUCT_LABEL("Option B")
REFLECT_MULTI_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN(reflect::color3)
    REFLECT_STRUCT_MEMBER(Blue)
    REFLECT_STRUCT_MEMBER(Green)
    REFLECT_STRUCT_MEMBER(Red)
REFLECT_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN(reflect::uid_reference)
    REFLECT_STRUCT_MEMBER(uids)
REFLECT_STRUCT_END()

//=======================================================
// CUSTOM NODE REFLECT DECLARATIONS
//

REFLECT_STRUCT2_BEGIN(Reflected_MeshBuffer_SceneNode)
    ALIAS("Mesh Buffer")
    PLACEABLE(false)
    INHERIT_FROM(Reflected_Sprite_SceneNode)
    //REFLECT_STRUCT2_MEMBER(face_uid)
    REFLECT_STRUCT2_MEMBER(element_id)
    REFLECT_STRUCT2_MEMBER(face_id)
REFLECT_STRUCT2_END()

REFLECT_STRUCT2_BEGIN(Reflected_MeshBuffer_Sky_SceneNode)
    ALIAS("Sky Mesh Buffer")
    PLACEABLE(false)
    INHERIT_FROM(Reflected_MeshBuffer_SceneNode)
    REFLECT_STRUCT2_MEMBER(is_connected)
REFLECT_STRUCT2_END()

REFLECT_STRUCT2_BEGIN(Reflected_MeshBuffer_Water_SceneNode)
    ALIAS("Water Mesh Buffer")
    PLACEABLE(false)
    INHERIT_FROM(Reflected_MeshBuffer_SceneNode)
    REFLECT_STRUCT2_MEMBER(is_connected)
REFLECT_STRUCT2_END()

