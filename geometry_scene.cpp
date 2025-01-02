#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "geometry_scene.h"
#include "edit_env.h"
#include "tolerances.h"
#include "texture_picker.h"
#include "BufferManager.h"
#include "material_groups.h"
#include "BVH.h"
#include <chrono>
#include "custom_nodes.h"
#include "GeometryStack.h"
#include "NodeClassesTool.h"
#include "LightMaps.h"
#include "CMeshSceneNode.h"
#include "initialization.h"
#include "utils.h"

extern SceneCoordinator* gs_coordinator;

#define TIME_HEADER() auto startTime = std::chrono::high_resolution_clock::now();\
    auto timeZero = startTime;\
    auto currentTime = std::chrono::high_resolution_clock::now();\
    float time;
#define START_TIMER() startTime = std::chrono::high_resolution_clock::now(); \

#define PRINT_TIMER(text) currentTime = std::chrono::high_resolution_clock::now(); \
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); \
    std::cout << "---------time (" <<#text<< "): " << time << "\n";\

#define PRINT_TOTAL_TIME(text) currentTime = std::chrono::high_resolution_clock::now(); \
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - timeZero).count(); \
    std::cout << "---------total time(" <<#text<< "): " << time << "\n";

#define printvec(v) v.X<<","<<v.Y<<","<<v.Z

using namespace irr;
using namespace std;

SceneCoordinator::SceneCoordinator(scene::ISceneManager* smgr, video::IVideoDriver* driver, MyEventReceiver* receiver) :
    smgr(smgr),driver(driver),receiver(receiver)
{
    geometry_scene* scene0 = new geometry_scene();
    scene0->initialize(smgr, driver, receiver);
    scene0->set_type(GEO_SOLID);
    //scene0->geoNode()->rebuild_geometry();
    scene0->rename("Main Scene");

    scenes.push_back(reflect::pointer<geometry_scene>{scene0});
}

SceneCoordinator::~SceneCoordinator()
{
}

geometry_scene* SceneCoordinator::current_scene()
{
    return scenes[scene_no];
}

geometry_scene* SceneCoordinator::skybox_scene()
{
    return m_skybox_scene;
}

geometry_scene* SceneCoordinator::get_scene(int n)
{
    if (n < scenes.size())
        return scenes[n];
    else
        return NULL;
}

void SceneCoordinator::swap_scene(int n)
{
    if (n >= scenes.size())
        return;

    current_scene()->disable();

    current_scene()->save_gui_state();

    geometry_scene* scene = scenes[n];

    scene->enable();

    initialize_set_scene(scene);

    scene->restore_gui_state();

    scene_no = n;
}

void SceneCoordinator::add_scene()
{
    geometry_scene* scene = new geometry_scene();

    scene::ISceneManager* new_smgr = smgr->createNewSceneManager();

    scene->initialize(new_smgr, driver, receiver);
    scene->set_type(GEO_SOLID);
    scene->InitializeEmptyScene();
    scene->geoNode()->rebuild_geometry();
    scene->disable();
    scene->rename("new scene");

    scenes.push_back(reflect::pointer<geometry_scene>{scene});
}

void SceneCoordinator::connect_skybox()
{
    m_skybox_scene = NULL;
    Reflected_SkyNode* skynode = NULL;

    for (geometry_scene* scene : this->scenes)
    {
        std::vector<u64> uids = scene->get_reflected_node_uids_by_type("Reflected_SkyNode");

        if (uids.size() > 0)
        {
            m_skybox_scene = scene;
            skynode = (Reflected_SkyNode*)scene->get_reflected_node_by_uid(uids[0]);

            break;
        }
    }

    std::vector<u64> face_uids;

    if (skynode && m_skybox_scene)
    {
        for (geometry_scene* scene : this->scenes)
        {
            std::vector<u64> uids = scene->get_reflected_node_uids_by_type("Reflected_MeshBuffer_Sky_SceneNode");
            for (u64 node_uid : uids)
            {
                face_uids.push_back(node_uid);
            }
        }

        skynode->target.uids = face_uids;

        std::cout << "skybox has " << skynode->target.uids.size() << " faces\n";
    }
    else
    {
        std::cout << "no skybox\n";
    }
    
}

void SceneCoordinator::set_skyox_dirty()
{
    skybox_dirty = true;
}

void SceneCoordinator::cleanup_after_scene_nodes_added_deleted()
{
    if (skybox_dirty)
    {
        connect_skybox();
        skybox_dirty = false;
    }
}

void SceneCoordinator::rebuild_dirty_meshbuffers()
{
    for (geometry_scene* scene : this->scenes)
    {
        scene->geoNode()->recalculate_final_meshbuffer();
    }
}

ISceneManager* SceneCoordinator::current_smgr()
{
    return scenes[scene_no]->smgr;
}

void geometry_scene::rename(const std::string& new_name)
{
    scene_name = new_name;
}

void geometry_scene::save_gui_state()
{
    ::save_gui_state(saved_gui_state);
}

void geometry_scene::restore_gui_state()
{
    ::restore_gui_state(saved_gui_state);
}

void geometry_scene::InitializeEmptyScene()
{
    if (!geometry_stack)
    {
        geometry_stack = new GeometryStack();
        geometry_stack->initialize(smgr->getRootSceneNode(), smgr, event_receiver);
        

        geo_element red;
        red.brush = make_poly_cube(256, 256, 256);
        red.type = GEO_RED;
        geometry_stack->elements.push_back(red);

        geometry_stack->rebuild_geometry();
    }
}



//====================================================

geometry_scene::geometry_scene()
{
    geometry_stack = NULL;
}

geometry_scene::geometry_scene(video::IVideoDriver* driver_, MyEventReceiver* receiver)
{
    this->driver = driver_;
    this->event_receiver = receiver;
    this->geometry_stack = NULL;

    geometry_stack = new GeometryStack();
    geometry_stack->initialize(NULL, smgr, receiver);
    this->geometry_stack->render_active_brush = false;

    event_receiver->Register(this);
}

geometry_scene::~geometry_scene()
{
    event_receiver->UnRegister(this);

    if (actual_nodes)
        actual_nodes->remove();

    if (editor_nodes)
        editor_nodes->remove();

    if(geometry_stack != NULL)
        geometry_stack->remove();
}

void geometry_scene::enable()
{
    event_receiver->Register(this);
}

void geometry_scene::disable()
{
    event_receiver->UnRegister(this);
}

void geometry_scene::initialize(scene::ISceneManager* smgr_, video::IVideoDriver* driver_, MyEventReceiver* receiver)
{
    this->smgr = smgr_;
    this->driver = driver_;
    this->event_receiver = receiver;

    this->editor_nodes = new USceneNode(smgr->getRootSceneNode(), smgr, 0, vector3df(0, 0, 0));
    this->actual_nodes = new USceneNode(smgr->getRootSceneNode(), smgr, 0, vector3df(0, 0, 0));

    event_receiver->Register(this);
}


void geometry_scene::visualizeMaterialGroups()
{
    /*
    polyfold* pf = get_total_geometry();

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        if (pf->faces[f_i].loops.size() > 0)
        {
            //int buffer_index = edit_meshnode_interface.get_buffer_index_by_face(f_i);
           // 
       
            int buffer_index = edit_meshnode_interface.get_buffer_index_by_face(f_i);

            scene::IMeshBuffer* buffer = (scene::IMeshBuffer*)this->getMeshNode()->getMesh()->getMeshBuffer(buffer_index);

            if (buffer)
            {
                buffer->getMaterial().MaterialType = base_material_type;
                buffer->getMaterial().Lighting = false;
            }

            //video::ITexture* tex = material_groups_base->material_groups[pf->faces[f_i].material_group].texture;

           // if(tex && buffer_index != -1)     
           //     getMeshNode()->SetFaceTexture(buffer_index, tex);
        }
    }

    b_Visualize = true;*/
}

bool geometry_scene::OnEvent(const SEvent& event)
{
    if(event.EventType == irr::EET_USER_EVENT)
    {
        switch(event.UserEvent.UserData1)
        {
            case USER_EVENT_TEXTURE_SELECTED:
            {
                if(this->selected_faces.size()>0)
                    TextureToSelectedFaces();
            }
            break;
        }
        return true;
    }
    return false;
}

void geometry_scene::setSelectedFaces_ShiftAdd(int new_sel)
{
    bool b = false;
    std::vector<int> selection;

    for(int f_i : getSelectedFaces())
    {
        if(f_i == new_sel)
        {
            b = true;
        }
        else
            selection.push_back(f_i);
    }

    if(!b)
        selection.push_back(new_sel);

    setSelectedFaces(selection);
}

void geometry_scene::setSelectedNodes_ShiftAdd(Reflected_SceneNode* new_sel)
{
    bool b = false;
    std::vector<Reflected_SceneNode*> selection;

    for(Reflected_SceneNode* n : getSelectedNodes())
    {
        if(n == new_sel)
        {
            b = true;
        }
        else
            selection.push_back(n);
    }

    if(!b)
        selection.push_back(new_sel);

    setSelectedNodes(selection);
}

void geometry_scene::setSelectedFaces(std::vector<int> selection)
{
    if(this->getMeshNode() == NULL || IsEditNode() == false)
        return;

    if(selected_faces.size() == 0 && selection.size() == 0)
        return;

    if(selection.size() == 1 && selected_faces.size()==1 && selection[0] == selected_faces[0])
        selection.clear();

    polyfold* pf = geometry_stack->get_total_geometry();

    for(int f_i=0;f_i<pf->faces.size();f_i++)
    {
        if(pf->faces[f_i].loops.size()>0)
        {
            int buffer_index = geometry_stack->edit_meshnode_interface.get_buffer_index_by_face(f_i);
            scene::IMeshBuffer* buffer = this->getMeshNode()->getMesh()->getMeshBuffer(buffer_index);
            bool bSelected = false;
            for(int f_j : selection)
                if(f_i == f_j)
                    bSelected=true;

            buffer->getMaterial().Lighting = false;


            Material_Groups_Tool::apply_material_to_buffer(buffer, pf->faces[f_i].material_group, -1, bSelected, false);

            if (b_Visualize)
            {
                /// TODO
            }
            
        }
    }

    this->getMeshNode()->copyMaterials();

    this->selected_faces = selection;

    //for(int n: selection)
    //    std::cout<<n<<" ";
    //std::cout << " (selected faces)\n";
}

void geometry_scene::setSelectedNodes(std::vector<Reflected_SceneNode*> selection)
{/*
    auto IsInSelection = [&](int ii) -> bool{
        bool b=false;
        for(int j : selection)
        {
            if(ii==j)
            {
                b=true;
            }
        }
        return b;
    };*/

    auto IsInSelection = [&](Reflected_SceneNode* node) -> bool {
        bool b = false;
        for (Reflected_SceneNode* n: selection)
        {
            if (node == n)
            {
                b = true;
                break;
            }
        }
        return b;
        };

   // for(int i=0; i<this->scene_nodes.size();i++)
    for(ISceneNode* it : this->editor_nodes->getChildren())
    {
        Reflected_SceneNode* node = (Reflected_SceneNode*)it;
        if(IsInSelection(node))
        {
            node->Select();
        }
        else
        {
            node->UnSelect();
        }
    }

    this->selected_scene_nodes = selection;
}


void geometry_scene::selectSurfaceGroup()
{
    if (getSelectedFaces().size() > 0)
    {

        int sg_type = getFaceSurfaceGroup(getSelectedFaces()[0]).type;

        std::vector<int> init_sel;

        for (int i = 0; i < selected_faces.size(); i++)
        {
            if (getFaceSurfaceGroup(selected_faces[i]).type == sg_type)
                init_sel.push_back(selected_faces[i]);
        }

        std::vector<int> unique_faces;

        auto IsUnique = [&](int ii) -> bool {
            bool b = true;

            int brush_ii = geometry_stack->get_total_geometry()->faces[ii].original_brush;
            int face_ii = geometry_stack->get_total_geometry()->faces[ii].original_face;
            int sg_ii = geometry_stack->elements[brush_ii].brush.faces[face_ii].surface_group;

            for (int j : unique_faces)
            {
                int brush_j = geometry_stack->get_total_geometry()->faces[j].original_brush;
                int face_j = geometry_stack->get_total_geometry()->faces[j].original_face;
                int sg_j = geometry_stack->elements[brush_j].brush.faces[face_j].surface_group;

                if (brush_j == brush_ii && sg_j == sg_ii)
                {
                    b = false;
                }
            }
            return b;
        };
    
        for(int i=0; i<init_sel.size(); i++)
        {
            if (IsUnique(init_sel[i]))
                unique_faces.push_back(init_sel[i]);
        }

        //std::cout << unique_faces.size() << " unique faces\n";

        std::vector<int> total_sel;

        for (int i = 0; i < unique_faces.size(); i++)
        {
            std::vector<int> sel = geometry_stack->getSurfaceFromFace(unique_faces[i]);

            for (int j : sel)
                total_sel.push_back(j);
        }
        
        setSelectedFaces(total_sel);
    }
    
}

void geometry_scene::selectionChanged()
{
    SEvent event;
    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1 = USER_EVENT_SELECTION_CHANGED;
    event_receiver->OnEvent(event);
}

surface_group geometry_scene::getFaceSurfaceGroup(int b_i)
{
    int brush_j = geometry_stack->get_total_geometry()->faces[b_i].original_brush;
    int face_j = geometry_stack->get_total_geometry()->faces[b_i].original_face;

    poly_face* f = &geometry_stack->elements[brush_j].brush.faces[face_j];

    return geometry_stack->elements[brush_j].brush.surface_groups[f->surface_group];
}

std::vector<Reflected_SceneNode*> geometry_scene::editor_node_ptrs_from_uid(const std::vector<u64>& selection)
{
    std::vector<Reflected_SceneNode*> ret;

    for (ISceneNode* it : editor_nodes->getChildren())
    {
        Reflected_SceneNode* node = (Reflected_SceneNode*)it;
        for (u64 uid : selection)
        {
            if (uid == node->UID())
            {
                ret.push_back(node);
            }
        }
    }
    return ret;
}

MeshBuffer_Chunk geometry_scene::get_face_buffer_by_uid(u64 uid)
{/*
    for(int f_i = 0; f_i<total_geometry.faces.size(); f_i++)
    {
        poly_face& f = total_geometry.faces[f_i];
        if (f.uid == uid)
        {
           return final_meshnode_interface.get_mesh_buffer_by_face(f_i);
        }
    }
    */
    return MeshBuffer_Chunk();
}

std::vector<int> geometry_scene::getSelectedFaces()
{
    return this->selected_faces;
}

std::vector<Reflected_SceneNode*> geometry_scene::getSelectedNodes()
{
    return this->selected_scene_nodes;
}

std::vector<int> geometry_scene::getBrushSelection()
{
    return this->selected_brushes;
}

void geometry_scene::setBrushSelection(std::vector<int> new_sel)
{
    this->selected_brushes=new_sel;

    for(int i=0; i<geometry_stack->elements.size(); i++)
    {
        geometry_stack->elements[i].bSelected=false;
    }

    for(int i: new_sel)
        geometry_stack->elements[i].bSelected=true;
}

void geometry_scene::setBrushSelection_ShiftAdd(int new_sel)
{
    bool b = false;
    std::vector<int> selection;

    for(int f_i : getBrushSelection())
    {
        if(f_i == new_sel)
        {
            b = true;
        }
        else
            selection.push_back(f_i);
    }

    if(!b)
        selection.push_back(new_sel);

    setBrushSelection(selection);
}

core::vector3df geometry_scene::getSelectedVertex()
{

    bool bSelectedVertex=false;
    core::vector3df ret = core::vector3df(0,0,0);

    for(int p_i :this->selected_brushes)
    {
        if(this->selected_brush_vertex_editing == p_i && geometry_stack->elements[p_i].control_vertex_selected == false && geometry_stack->elements[p_i].selected_vertex < geometry_stack->elements[p_i].brush.vertices.size())
        {
            bSelectedVertex=true;
            ret=geometry_stack->elements[p_i].brush.vertices[ geometry_stack->elements[p_i].selected_vertex].V;
        }
        else if(this->selected_brush_vertex_editing == p_i && geometry_stack->elements[p_i].control_vertex_selected == true && geometry_stack->elements[p_i].selected_vertex < geometry_stack->elements[p_i].brush.control_vertices.size())
        {
            bSelectedVertex = true;
            ret = geometry_stack->elements[p_i].brush.control_vertices[geometry_stack->elements[p_i].selected_vertex].V;
        }
    }
    if(!bSelectedVertex && geometry_stack->elements.size()>0)
    {
        ret=geometry_stack->elements[this->getBrushSelection()[0]].brush.vertices[0].V;
    }
    return ret;
}

void geo_element::draw_brush(video::IVideoDriver* driver, const video::SMaterial material)
{
    video::SColor col = this->getColor();

    for(int i =0; i<this->brush.edges.size(); i++)
    {
        if(this->brush.edges[i].topo_group != -1)
            driver->draw3DLine(this->brush.getVertex(i,0).V,this->brush.getVertex(i,1).V, col);
        //driver->draw3DLine(this->brush.getVertex(i,0).V,this->brush.getVertex(i,1).V,video::SColor(128,128,128,128));
    }
}
video::SColor geo_element::getColor()
{
   // video::SColor col = video::SColor(128,128,128,128);

    if(this->bSelected==false)
    {
        if(this->type == GEO_ADD)
            return COLOR_ADD_NOT_SELECTED;
        else if(this->type == GEO_SUBTRACT)
            return COLOR_SUB_NOT_SELECTED;
        else if(this->type == GEO_SEMISOLID)
            return COLOR_SEMI_NOT_SELECTED;
        else if(this->type == GEO_RED)
            return COLOR_RED_NOT_SELECTED;
    }
    else if(this->bSelected==true)
    {
        if(this->type == GEO_ADD)
            return COLOR_ADD_SELECTED;
        else if(this->type == GEO_SUBTRACT)
            return COLOR_SUB_SELECTED;
        else if(this->type == GEO_SEMISOLID)
            return COLOR_SEMI_SELECTED;
        else if(this->type == GEO_RED)
            return COLOR_RED_SELECTED;
    }

    return video::SColor(128,128,128,128);
}
void geo_element::draw_geometry(video::IVideoDriver* driver, const video::SMaterial material)
{
    //driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    //driver->setMaterial(material);
    //video::SColor col = video::SColor(255,38,128,1155);
    video::SColor col = video::SColor(255,38,128,155);

    for(int i =0; i<this->geometry.edges.size(); i++)
    {
        if(this->geometry.edges[i].topo_group != -1)
            driver->draw3DLine(this->geometry.getVertex(i,0).V,this->geometry.getVertex(i,1).V,col);
    }
}

scene::CMeshSceneNode* geometry_scene::getMeshNode()
{ 
    if(geometry_stack)
        return geometry_stack->getMeshNode();
    return NULL;
}

bool geometry_scene::IsEditNode() 
{ 
    return geometry_stack->b_isEditNode; 
}

void geometry_scene::MaterialGroupToSelectedFaces()
{
    if (!getMeshNode() || !IsEditNode())
        return;

    Material_Groups_Base* material_groups_base = Material_Groups_Tool::get_base();

    int mg = material_groups_base->getSelected();

    if (mg == -1)
        return;

    bool dirty_mesh = false;

    for (int i : selected_faces)
    {
        int brush_i = geometry_stack->get_total_geometry()->faces[i].original_brush;
        int face_i =geometry_stack->get_total_geometry()->faces[i].original_face;

        if (geometry_stack->get_total_geometry()->faces[i].material_group != mg ||
            geometry_stack->elements[brush_i].brush.faces[face_i].material_group != mg ||
            geometry_stack->elements[brush_i].geometry.faces[face_i].material_group != mg)
        {
            geometry_stack->elements[brush_i].brush.faces[face_i].material_group = mg;
            geometry_stack->elements[brush_i].geometry.faces[face_i].material_group = mg;

            geometry_stack->get_total_geometry()->faces[i].material_group = mg;

            dirty_mesh = true;
        }
    }

    if (b_Visualize)
        visualizeMaterialGroups();

    if (dirty_mesh)
    {
        if (mg == 4)
        {
            setFaceNodeType(selected_faces, "Reflected_MeshBuffer_Sky_SceneNode");
        }
        else if (mg == 5)
        {
            setFaceNodeType(selected_faces, "Reflected_MeshBuffer_Water_SceneNode");
        }
        else
        {
            setFaceNodeType(selected_faces, NULL);
        }

        geometry_stack->setFinalMeshDirty();

        gs_coordinator->cleanup_after_scene_nodes_added_deleted();
    }
}

void geometry_scene::TextureToSelectedFaces()
{
    if(!this->getMeshNode() || !IsEditNode())
        return;

    for(int i: this->selected_faces)
    {
        int brush_i = geometry_stack->get_total_geometry()->faces[i].original_brush;
        int face_i = geometry_stack->get_total_geometry()->faces[i].original_face;

        geometry_stack->elements[brush_i].brush.faces[face_i].texture_name=core::stringw(TexturePicker_Tool::getCurrentTexture()->getName().getPath());
        geometry_stack->elements[brush_i].geometry.faces[face_i].texture_name= TexturePicker_Tool::getCurrentTexture()->getName().getPath();
    }

    for(int i=0; i<this->getMeshNode()->getMesh()->getMeshBufferCount();i++)
    {
        bool b=false;
        for(int j : selected_faces)
        {
            int buffer_index = geometry_stack->edit_meshnode_interface.get_buffer_index_by_face(j);
            if(i==buffer_index)
                b=true;
        }
        if(b)
            getMeshNode()->SetFaceTexture(i, TexturePicker_Tool::getCurrentTexture());
    }

    geometry_stack->setFinalMeshDirty();
}

void geometry_scene::drawGraph(LineHolder& graph)
{

    //for (line3df el : intersections_graph.lines)
    //    graph.lines.push_back(el);
    graph.lines.clear();

    for (const line3df& el : special_graph.lines)
        graph.lines.push_back(el);

    for (const vector3df& p : special_graph.points)
        graph.points.push_back(p);
}


void geometry_scene::setRenderType(bool brushes, bool geo, bool loops, bool triangles)
{
    if(geometry_stack)
        geometry_stack->setRenderType(brushes, geo, loops, triangles);
}

void geometry_scene::loadLightmapTextures()
{
    if(geometry_stack)
    Lightmaps_Tool::get_manager()->loadLightmapTextures(geometry_stack);
}

void geometry_scene::save_selection()
{
    saved_selection.brushes = getBrushSelection();
    saved_selection.faces = getSelectedFaces();
    saved_selection.scene_nodes = getSelectedNodes();
}

vector<u64> geometry_scene::get_saved_selection_uids()
{
    vector<u64> ret;
    for (Reflected_SceneNode* node : saved_selection.scene_nodes)
    {
        ret.push_back(node->UID());
    }
    cout << ret.size() << " nodes in selection\n";
    return ret;
}

Reflected_SceneNode* geometry_scene::get_reflected_node_by_uid(u64 uid)
{
    for (ISceneNode* inode : editor_nodes->getChildren())
    {
        Reflected_SceneNode* node = (Reflected_SceneNode*)inode;
        if (node->UID() == uid)
            return node;
    }

    return NULL;
}

std::vector<u64> geometry_scene::get_reflected_node_uids_by_type(const char* node_type_name)
{
    reflect::TypeDescriptor_Struct* tD = NULL;

    if (node_type_name != NULL)
        tD = Reflected_SceneNode_Factory::getNodeTypeDescriptorByName(node_type_name);

    if(tD == NULL)
        return std::vector<u64>{};

    std::vector<u64> ret;

    core::list<ISceneNode*> nodes = editor_nodes->getChildren();

    core::list<scene::ISceneNode*>::Iterator it = nodes.begin();
    for (; it != nodes.end(); ++it)
    {
        Reflected_SceneNode* node = (Reflected_SceneNode*)*it;
        if (node->GetDynamicReflection()->isOfType(tD))
        {
            ret.push_back(node->UID());
        }
    }
    
    return ret;
}

void geometry_scene::setFaceNodeType(const std::vector<int>& faces_i, const char* node_type_name)
{
    reflect::TypeDescriptor_Struct* tD = NULL;
    if (node_type_name != NULL)
        tD = Reflected_SceneNode_Factory::getNodeTypeDescriptorByName(node_type_name);

    reflect::TypeDescriptor_Struct* MeshBufferNode_tD = 
        (reflect::TypeDescriptor_Struct*)(reflect::TypeResolver<Reflected_MeshBuffer_SceneNode>::get());

    core::list<ISceneNode*> nodes = editor_nodes->getChildren();

    std::vector<bool> has_node;
    has_node.assign(faces_i.size(),false);
    int c = 0;
    core::list<scene::ISceneNode*>::Iterator it = nodes.begin();
    for (; it != nodes.end(); ++it)
    {
        Reflected_SceneNode* node = (Reflected_SceneNode*)*it;
        if (node->GetDynamicReflection()->isOfType(MeshBufferNode_tD))
        {
            Reflected_MeshBuffer_SceneNode* meshbuffer_node = (Reflected_MeshBuffer_SceneNode*)node;

            for (int i=0; i< faces_i.size(); i++)
            {
                int f_i = faces_i[i];

                u64 face_uid = geometry_stack->get_total_geometry()->faces[f_i].uid;

                if (meshbuffer_node->get_uid() == face_uid)
                {
                    if (tD == NULL || meshbuffer_node->GetDynamicReflection() != tD)
                    {
                        editor_nodes->removeChild(*it);
                        c++;
                    }
                    else
                    {
                        has_node[i] = true;
                    }
                }
                vector3df pos = geometry_stack->get_total_geometry()->faces[f_i].m_center;
            }
        }
    }
    std::cout << "deleted " << c << "meshbuffer nodes\n";

    c = 0;
    if (tD)
    {
        for (int i = 0; i < faces_i.size(); i++)
        {
            if (!has_node[i])
            {
                int f_i = faces_i[i];
                vector3df pos = geometry_stack->get_total_geometry()->faces[f_i].m_center;
                u64 face_uid = geometry_stack->get_total_geometry()->faces[f_i].uid;

                Reflected_SceneNode* node = ((reflect::TypeDescriptor_SN_Struct*)tD)->create_func(editor_nodes, this, smgr, -1, pos);
                
                node->drop();
                node->preEdit();       
                ((Reflected_MeshBuffer_SceneNode*)node)->set_uid(face_uid);
                node->postEdit();

                c++;
            }
        }

        std::cout << "created " << c << " " << tD->alias << "\n";
    }

    //caller calls
    //gs_coordinator->cleanup_after_scene_nodes_added_deleted();
}

void geometry_scene::addFaceNode(int f_i)
{
    u64 face_uid = geometry_stack->get_total_geometry()->faces[f_i].uid;
    vector3df pos = geometry_stack->get_total_geometry()->faces[f_i].m_center;

    Reflected_MeshBuffer_SceneNode* node = new Reflected_MeshBuffer_SceneNode(editor_nodes, this, smgr, -1, pos);

    node->set_uid(face_uid);

    node->preEdit();
}

void geometry_scene::rebuildSceneGraph()
{

}

void geometry_scene::buildSceneGraph(bool addObjects, int light_mode, bool finalscene)
{
   // std::cout << "rebuilding scene...";
    //std::cout << "edit objects "<< addObjects<< ", final " << finalMesh << ", add lights " << addLights << "\n";

    if(finalscene)
        cout << this->scene_name << ": building Scene Graph (final)\n";
    else
        cout << this->scene_name << ": building Scene Graph (edit)\n";

    //core::list<scene::ISceneNode*> child_list = smgr->getRootSceneNode()->getChildren();
    core::list<scene::ISceneNode*> child_list = actual_nodes->getChildren();

   // if(my_MeshNode)
   //     my_MeshNode->drop();

    bool beginScene = (finalscene && !bSceneInProgress);
    bool endScene = (!finalscene && bSceneInProgress);
    int count = 0;
    if (endScene)
    {
        core::list<scene::ISceneNode*>::Iterator it = child_list.begin();
        for (; it != child_list.end(); ++it)
        {
            int id = (*it)->getID();
            scene::ISceneNode* node = *it;
            if ((*it)->getType() != scene::ESNT_CAMERA)
            {
                actual_nodes->removeChild(*it);
                count++;
            }
            int a = 5;
        }
    }
    
    if(finalscene)
        geometry_stack->recalculate_final_meshbuffer();

    count = 0;
    child_list = editor_nodes->getChildren();
    for (core::list<scene::ISceneNode*>::Iterator it = child_list.begin(); it != child_list.end(); ++it)
    {
        Reflected_SceneNode* node = (Reflected_SceneNode*)(*it);

        if(endScene)
            node->endScene();

       // if (beginScene ||
        //    (light_mode != LIGHTING_UNLIT && node->getType() == ESNT_LIGHT))
        if(beginScene)
        {
            
            if(node->addSelfToScene(actual_nodes, smgr, this))
                count++;
        }
        
    }
    cout << "   actual nodes " << actual_nodes->getChildren().getSize() << "\n";
    //cout << "   editor nodes " << editor_nodes->getChildren().getSize() << "\n";

    child_list = smgr->getRootSceneNode()->getChildren();

    //cout << "   scene total nodes = " << child_list.getSize() << "\n";

    //build the geometry mesh node
    if(geometry_stack)
        geometry_stack->buildSceneNode(finalscene, light_mode);

    if(finalscene)
    {
        editor_nodes->setVisible(false);
        bSceneInProgress = true;
       
    }
    else
    {
        editor_nodes->setVisible(true);
        bSceneInProgress = false;
        
    }

    if(light_mode != LIGHTING_UNLIT)
    {
        b_dynamic_light = true;
    }
    else
    {
        b_dynamic_light = false;
    }

    //this->my_MeshNode->getMesh()->setHardwareMappingHint(scene::EHM_STATIC);

    //my_MeshNode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS,true);

    //my_MeshNode->setVisible(true);

    b_Visualize = false;

    //my_MeshNode->setDebugDataVisible(scene::EDS_NORMALS);
}


void geometry_scene::clear_scene()
{
    //std::cout<<"clearing scene... ";
    /*
    geo_element active_brush = this->elements[0];
    this->elements.clear();
    this->elements.push_back(active_brush);

    this->build_progress=0;

    this->selected_brushes.clear();
    this->selected_faces.clear();
    this->edit_mesh_buffer_faces.clear();

    if(this->getMeshNode())
    {
        this->getMeshNode()->remove();
    }
    */
    //for(Reflected_SceneNode* node : this->scene_nodes)

    this->selected_brushes.clear();
    this->selected_faces.clear();
    this->selected_scene_nodes.clear();

    core::list<scene::ISceneNode*> child_list = actual_nodes->getChildren();
    core::list<scene::ISceneNode*>::Iterator it = child_list.begin();
    for (; it != child_list.end(); ++it)
    {
        (*it)->remove();
    }

    cout << "actual nodes remaining: " << actual_nodes->getChildren().getSize() << "\n";

    child_list = editor_nodes->getChildren();
    it = child_list.begin();
    for (; it != child_list.end(); ++it)
    {
        (*it)->remove();
    }

    cout << "editor nodes remaining: " << editor_nodes->getChildren().getSize() << "\n";
   

    geometry_stack->clear_scene();

    //this->scene_nodes.clear();

    //this->rebuild_geometry();
}

void geometry_scene::delete_selected_brushes()
{
    std::vector<geo_element> new_elements;

    int removed=0;
    new_elements.push_back(geometry_stack->elements[0]);
    for(int i=1; i< geometry_stack->elements.size(); i++)
    {
        bool b=false;
        for(int j:this->selected_brushes)
            if(j==i)
                b=true;

        if(!b)
            new_elements.push_back(geometry_stack->elements[i]);
        else
            removed++;
    }
    geometry_stack->elements=new_elements;

    this->selected_brushes.clear();

    if(removed > 0)
    {
       // std::cout<<"deleted "<<removed<<" elements\n";
        selectionChanged();
    }
}

void geometry_scene::addSceneLight(core::vector3df pos)
{
    Reflected_SceneNode* a_light = new Reflected_LightSceneNode(editor_nodes,this,smgr,-1,pos);
    a_light->preEdit();
   // scene_nodes.push_back(a_light);
    if(b_dynamic_light)
        a_light->addSelfToScene(actual_nodes,smgr, this);
}

void geometry_scene::addSceneSelectedSceneNodeType(core::vector3df pos)
{
    reflect::TypeDescriptor_Struct* typeDescriptor = Node_Classes_Tool::get_base()->getSelectedTypeDescriptor();
    if(typeDescriptor)
    {
        Reflected_SceneNode* a_thing = Reflected_SceneNode_Factory::CreateNodeByTypeName(typeDescriptor->name, editor_nodes, this, smgr);
        if(a_thing)
        {
            a_thing->drop();
            std::cout<<"added a "<<typeDescriptor->name<<"\n";
            a_thing->setPosition(pos);
            a_thing->preEdit();
            a_thing->postEdit();

            gs_coordinator->cleanup_after_scene_nodes_added_deleted();
        }
    }
}

void geometry_scene::deleteSelectedNodes()
{
    if (selected_scene_nodes.size() == 0)
        return;

    core::list<scene::ISceneNode*> child_list = editor_nodes->getChildren();

    std::cout << editor_nodes->getChildren().size() << " children before\n";
    for (Reflected_SceneNode* n : selected_scene_nodes)
    {
        core::list<scene::ISceneNode*>::Iterator it = child_list.begin();
        for (; it != child_list.end(); ++it)
        {
            Reflected_SceneNode* node = (Reflected_SceneNode*)(*it);
            
            if (node == n)
            {
                reflect::TypeDescriptor_SN_Struct* tD = (reflect::TypeDescriptor_SN_Struct*)node->GetDynamicReflection();

                if (tD->placeable)
                    editor_nodes->removeChild(*it);
                else
                    cout << "cannot delete " << tD->getAlias() << "\n";
            }
        }
    }
    std::cout << editor_nodes->getChildren().size() << " children after\n";

    selected_scene_nodes.clear();

    selectionChanged();

    gs_coordinator->cleanup_after_scene_nodes_added_deleted();

}


REFLECT_STRUCT_BEGIN(geometry_scene)
    REFLECT_STRUCT_MEMBER(base_type)
    REFLECT_STRUCT_MEMBER(geometry_stack)
    REFLECT_STRUCT_MEMBER(scene_name)
    REFLECT_STRUCT_MEMBER(saved_gui_state)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(SceneCoordinator)
    REFLECT_STRUCT_MEMBER(scenes)
REFLECT_STRUCT_END()
