
#include <irrlicht.h>
#include "reflection.h"
#include "reflected_nodes.h"
#include "CameraPanel.h"
#include "utils.h"
#include "geometry_scene.h"
#include "GUI_tools.h"
#include "custom_nodes.h"
#include "USceneNode.h"

using namespace irr;
using namespace gui;

extern IrrlichtDevice* device;
extern SceneCoordinator* gs_coordinator;

video::E_MATERIAL_TYPE Reflected_SceneNode::base_material_type = video::EMT_SOLID;
video::E_MATERIAL_TYPE Reflected_SceneNode::special_material_type = video::EMT_SOLID;

ListReflectedNodes_Base* ListReflectedNodes_Tool::base = NULL;
multi_tool_panel* ListReflectedNodes_Tool::panel = NULL;

std::vector<reflect::TypeDescriptor_Struct*> Reflected_SceneNode_Factory::SceneNode_Types{};

ListReflectedNodesWindow::ListReflectedNodesWindow(IGUIEnvironment* env, IGUIElement* parent,ListReflectedNodes_Base* base_, s32 id,core::rect<s32> rect)
    : IGUIElement(EGUIET_ELEMENT,env,parent,id,rect), base(base_),my_ID(id)
{
    LISTBOX_ID = my_ID+1;
    OK_BUTTON_ID = my_ID+2;

    IGUISkin* skin = Environment->getSkin();
    IGUIFont* font = skin->getFont();
    int itemheight = font->getDimension(L"A").Height + 4;

    core::rect<s32> r(0,0,getRelativePosition().getWidth(),base->typeDescriptors.size()*itemheight+4);

    skin->draw3DSunkenPane(this,skin->getColor(EGDC_3D_FACE),true,true,r);

    IGUIListBox* listbox = Environment->addListBox(r,this,LISTBOX_ID);
    listbox->setDrawBackground(false);

    for(reflect::TypeDescriptor* td : base->typeDescriptors)
    {
        reflect::TypeDescriptor_Struct* typeDesc = (reflect::TypeDescriptor_Struct*) td;

        std::wstring txt(typeDesc->alias,typeDesc->alias+strlen(typeDesc->alias));
        listbox->addItem(txt.c_str());
    }

    listbox->setSelected(-1);

    if(base->selectedDescriptor != NULL)
        for(int i=0;i<base->typeDescriptors.size();i++)
        {
            if(base->typeDescriptors[i] == base->selectedDescriptor)
                listbox->setSelected(i);
        }

    //Environment->addButton(core::rect<s32>(core::vector2di(120,200),core::vector2di(180,228)),this,OK_BUTTON_ID,L"Ok");
}
/*
void ListReflectedNodesWindow::move(core::position2d<s32> new_pos)
{
    IGUIWindow::move(new_pos);
    base->win_pos = getRelativePosition().UpperLeftCorner;
}*/

ListReflectedNodesWindow::~ListReflectedNodesWindow()
{
    //std::cout << "Out of scope (List node classes)\n";
}

bool ListReflectedNodesWindow::OnEvent(const SEvent& event)
{
    if(event.EventType == EET_GUI_EVENT)
    {
        s32 id = event.GUIEvent.Caller->getID();

        switch(event.GUIEvent.EventType)
        {
            case EGET_LISTBOX_CHANGED:
            {
                if(id == LISTBOX_ID)
                {
                    IGUIListBox* listbox = (IGUIListBox*)event.GUIEvent.Caller;
                    int sel = listbox->getSelected();

                    if(sel != -1)
                        this->base->selectedDescriptor = this->base->typeDescriptors[sel];
                    else
                        this->base->selectedDescriptor = NULL;

                    return true;
                }
            }break;
        }

    }
    else if(event.EventType == EET_MOUSE_INPUT_EVENT)
    {
        if(event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
        {
            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
            {
                Environment->removeFocus(this);
            }
            else
            {
                Environment->setFocus(this);
            }
        }
    }

    return IGUIElement::OnEvent(event);
}

void ListReflectedNodesWindow::click_OK()
{
    IGUIListBox* listbox = (IGUIListBox*)getElementFromId(LISTBOX_ID);

    int sel = listbox->getSelected();

    if(sel != -1)
        this->base->selectedDescriptor = this->base->typeDescriptors[sel];
    else
        this->base->selectedDescriptor = NULL;

    this->remove();
}

reflect::TypeDescriptor_Struct* ListReflectedNodes_Base::getSelectedTypeDescriptor()
{
    return selectedDescriptor;
}

void ListReflectedNodes_Base::show()
{
    //ListReflectedNodesWindow* win = new ListReflectedNodesWindow(env,env->getRootGUIElement(),this,-1,core::rect<s32>(140,200,140+196,200+240));
    core::rect<s32> client_rect(core::vector2di(0,0),
                                core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
                                                   this->panel->getClientRect()->getAbsolutePosition().getHeight()));

    ListReflectedNodesWindow* widget = new ListReflectedNodesWindow(env,this->panel->getClientRect(),this,GUI_ID_NODE_PROPERTIES_BASE,client_rect);
   // win->setText(L"Scene Nodes");
    widget->drop();
}

void ListReflectedNodes_Base::initialize(std::wstring name_, int my_id, IGUIEnvironment* env_, geometry_scene* g_scene_,multi_tool_panel* panel_ )
{
    typeDescriptors = Reflected_SceneNode_Factory::getAllTypes();

    //tool_base::initialize(name_,my_id,env_,g_scene_,panel_);
}

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
    }
}

void Reflected_SceneNode::disconnect(Reflected_SceneNode* node) 
{
    std::vector<u64> new_inputs;

    for (u64 uid : input_nodes)
    {
        if (uid != node->UID())
            new_inputs.push_back(uid);
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
    return core::aabbox3df();
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

const core::aabbox3df& Reflected_Sprite_SceneNode::getBoundingBox() const
{
    return Buffer->getBoundingBox();
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

const core::aabbox3df& Reflected_Model_SceneNode::getBoundingBox() const
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

Reflected_MeshBuffer_SceneNode::Reflected_MeshBuffer_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_Sprite_SceneNode(parent,geo_scene,smgr,id,pos)
{
    m_texture = device->getVideoDriver()->getTexture("color_square_icon.png");
    Buffer->Material.setTexture(0, m_texture);
}

bool Reflected_MeshBuffer_SceneNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    /*
    polyfold* pf = geo_scene->geoNode()->get_total_geometry();

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        if (pf->faces[f_i].uid == this->face_uid)
        {
            std::cout << "found it: "<<f_i<<"\n";
        }
    }*/
    return false;
}

void Reflected_MeshBuffer_SceneNode::restore_original_texture()
{
    polyfold* pf = geo_scene->geoNode()->get_total_geometry();

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        if (pf->faces[f_i].uid == this->face_uid)
        {
            int brush_i = pf->faces[f_i].original_brush;
            int face_i = pf->faces[f_i].original_face;

            const core::stringw& texture_name = geo_scene->geoNode()->elements[brush_i].brush.faces[face_i].texture_name;

            video::ITexture* texture = device->getVideoDriver()->getTexture(texture_name);
            
            MeshBuffer_Chunk chunk = geo_scene->geoNode()->final_meshnode_interface.get_mesh_buffer_by_face(f_i);

            IMeshBuffer* buffer = chunk.buffer;

            if (buffer)
                buffer->getMaterial().setTexture(0, texture);
        }
    }
}

IMeshBuffer* Reflected_MeshBuffer_SceneNode::get_mesh_buffer()
{
    polyfold* pf = geo_scene->geoNode()->get_total_geometry();

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        if (pf->faces[f_i].uid == get_uid())
        {
            MeshBuffer_Chunk chunk = geo_scene->geoNode()->final_meshnode_interface.get_mesh_buffer_by_face(f_i);

            IMeshBuffer* buffer = chunk.buffer;

            return buffer;
        }
    }

    return NULL;
}

void Reflected_MeshBuffer_SceneNode::set_uid(u64 uid)
{
    face_uid = uid;
}

u64 Reflected_MeshBuffer_SceneNode::get_uid()
{
    return face_uid;
}


Reflected_MeshBuffer_Sky_SceneNode::Reflected_MeshBuffer_Sky_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_MeshBuffer_SceneNode(parent,geo_scene,smgr,id,pos)
{
    m_texture = device->getVideoDriver()->getTexture("color_square_icon_nogood.png");
    Buffer->Material.setTexture(0, m_texture);
}

Reflected_MeshBuffer_Sky_SceneNode::~Reflected_MeshBuffer_Sky_SceneNode()
{
    gs_coordinator->set_skyox_dirty();
}

void Reflected_MeshBuffer_Sky_SceneNode::postEdit()
{
    gs_coordinator->set_skyox_dirty();

    Reflected_MeshBuffer_SceneNode::postEdit();
}

Reflected_MeshBuffer_Water_SceneNode::Reflected_MeshBuffer_Water_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_MeshBuffer_SceneNode(parent, geo_scene, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("color_square_icon_nogood.png");
    Buffer->Material.setTexture(0, m_texture);
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
    REFLECT_STRUCT2_MEMBER(face_uid)
REFLECT_STRUCT2_END()

REFLECT_STRUCT2_BEGIN(Reflected_MeshBuffer_Sky_SceneNode)
    ALIAS("Sky Mesh Buffer")
    PLACEABLE(false)
    INHERIT_FROM(Reflected_MeshBuffer_SceneNode)
    REFLECT_STRUCT2_MEMBER(OK)
REFLECT_STRUCT2_END()

REFLECT_STRUCT2_BEGIN(Reflected_MeshBuffer_Water_SceneNode)
    ALIAS("Water Mesh Buffer")
    PLACEABLE(false)
    INHERIT_FROM(Reflected_MeshBuffer_SceneNode)
    REFLECT_STRUCT2_MEMBER(OK)
REFLECT_STRUCT2_END()

REFLECT_STRUCT2_BEGIN(Reflected_LightSceneNode)
    ALIAS("Light")
    INHERIT_FROM(Reflected_Sprite_SceneNode)
    REFLECT_STRUCT2_MEMBER(enabled)
    REFLECT_STRUCT2_MEMBER(light_radius)
REFLECT_STRUCT2_END()

REFLECT_STRUCT2_BEGIN(Reflected_SkyNode)
    ALIAS("Sky Node")
    INHERIT_FROM(Reflected_Sprite_SceneNode)
    REFLECT_STRUCT2_MEMBER(enabled)
    REFLECT_STRUCT2_MEMBER(target)
REFLECT_STRUCT2_END()

REFLECT_STRUCT2_BEGIN(Reflected_WaterSurfaceNode)
    ALIAS("Water Surface")
    REFLECT_STRUCT2_MEMBER(enabled)
    REFLECT_STRUCT2_MEMBER(target)
    INHERIT_FROM(Reflected_Sprite_SceneNode)
REFLECT_STRUCT2_END()

REFLECT_STRUCT2_BEGIN(Reflected_PointNode)
    ALIAS("Point Node")
    INHERIT_FROM(Reflected_Sprite_SceneNode)
REFLECT_STRUCT2_END()

REFLECT_STRUCT2_BEGIN(Reflected_TestNode)
    ALIAS("Test Node")
    INHERIT_FROM(Reflected_Sprite_SceneNode)
    REFLECT_STRUCT2_MEMBER(bEnabled)
    REFLECT_STRUCT2_MEMBER(nParticles)
    REFLECT_STRUCT2_MEMBER(options)
    REFLECT_STRUCT2_MEMBER(velocity)
    REFLECT_STRUCT2_MEMBER(scale)
    REFLECT_STRUCT2_MEMBER(my_vec)
    REFLECT_STRUCT2_MEMBER(vec2)
    REFLECT_STRUCT2_MEMBER(color)
REFLECT_STRUCT2_END()

REFLECT_STRUCT2_BEGIN(Reflected_SimpleEmitterSceneNode)
    ALIAS("Particle Emitter")
    INHERIT_FROM(Reflected_Sprite_SceneNode)

    REFLECT_STRUCT2_MEMBER(EmitBox)
    REFLECT_STRUCT2_MEMBER(texture)
    REFLECT_STRUCT2_MEMBER(particle_scale)
    REFLECT_STRUCT2_MEMBER(minParticlesPerSecond)
    REFLECT_STRUCT2_MEMBER(maxParticlesPerSecond)
    REFLECT_STRUCT2_MEMBER(lifeTimeMin)
    REFLECT_STRUCT2_MEMBER(lifeTimeMax)

    REFLECT_STRUCT2_MEMBER(bool_A)
    REFLECT_STRUCT2_MEMBER(bool_B)
    REFLECT_STRUCT2_MEMBER(color)
    REFLECT_STRUCT2_MEMBER(Vector_A)
    REFLECT_STRUCT2_MEMBER(Vector_B)
    REFLECT_STRUCT2_MEMBER(Vector_C)
REFLECT_STRUCT2_END()


