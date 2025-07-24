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
#include "my_reflected_nodes.h"
#include "GeometryStack.h"
#include "NodeClassesTool.h"
#include "LightMaps.h"
#include "CMeshSceneNode.h"
#include "initialization.h"
#include "utils.h"
#include <sstream>

#include "file_open.h"

extern SceneCoordinator* gs_coordinator;
extern IrrlichtDevice* device;

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


//================================================================
// Geometry Scene File IO
//

class SceneCoordinator;

Geometry_Scene_File_IO::Geometry_Scene_File_IO(io::IFileSystem* fs) 
    : FileSystem(fs)
{
}

void Geometry_Scene_File_IO::WriteToFiles(SceneCoordinator* sc)
{
    WriteMetadata(sc, "meta.dat");

    sc->current_scene()->save_gui_state();

    for (int i = 0; i < sc->scenes.size(); i++)
    {
        std::stringstream nodes_name;
        nodes_name << "nodes" << i << ".dat";

        std::stringstream texture_name;
        texture_name << "textures" << i << ".dat";

        std::stringstream serial_name;
        serial_name << "refl_serial" << i << ".dat";

        SerializeSceneNodesToFile(sc->scenes[i], nodes_name.str().c_str());
        WriteTextureNames(sc->scenes[i], texture_name.str().c_str());
        SerializeGeometryToFile(sc->scenes[i], serial_name.str().c_str());
    }
}

void Geometry_Scene_File_IO::ReadFromFiles(SceneCoordinator* sc)
{
    metadata data;

    scene::ISceneManager* smgr = device->getSceneManager();
    IEventReceiver* receiver = device->getEventReceiver();
    video::IVideoDriver* driver = device->getVideoDriver();

    if (ReadMetadata("meta.dat", data))
    {
        //delete all existing scenes
        sc->clear();

        for (int i = 0; i < data.n_scenes; i++)
        {
            geometry_scene* scene0 = new geometry_scene();

            if(i==0)
                scene0->initialize(smgr, driver, (MyEventReceiver*)receiver);
            else
            {
                scene::ISceneManager* new_smgr = smgr->createNewSceneManager();
                scene0->initialize(new_smgr, driver, (MyEventReceiver*)receiver);
            }

            scene0->set_type(GEO_SOLID);
            
            //======== Deserialize ========
            std::stringstream nodes_name;
            nodes_name << "nodes" << i << ".dat";

            std::stringstream texture_name;
            texture_name << "textures" << i << ".dat";

            std::stringstream serial_name;
            serial_name << "refl_serial" << i << ".dat";

            DeserializeGeometryFromFile(scene0, serial_name.str().c_str(), texture_name.str().c_str());
            DeserializeSceneNodesFromFile(scene0, nodes_name.str().c_str());
            //==============================

            //scene0->geoNode()->set_originals();
            scene0->geoNode()->build_total_geometry();
            scene0->geoNode()->generate_meshes();

            sc->scenes.push_back(reflect::pointer<geometry_scene>{scene0});
        }

        //point all tools and GUI to the new main scene
        initialize_set_scene(sc->scenes[0]);

        for (geometry_scene* scene : sc->scenes)
        {
            for (ISceneNode* n : scene->editor_nodes->getChildren())
            {
                Reflected_SceneNode* sn = (Reflected_SceneNode*)n;
                sn->preEdit();
                sn->postEdit();
            }
        }

        gui::IGUIEnvironment* env = device->getGUIEnvironment();
        gui::IGUIElement* root = env->getRootGUIElement();
        CameraQuad* quad = (CameraQuad*)root->getElementFromId(GUI_ID_CAMERA_QUAD, true);

        if (quad)
            quad->set_scene(sc->scenes[0]);

        //load the saved GUI state
        sc->current_scene()->restore_gui_state();
    }
}

void Geometry_Scene_File_IO::AutoLoad(SceneCoordinator* sc, io::path p)
{
    io::path restore_path = FileSystem->getWorkingDirectory();

    FileSystem->changeWorkingDirectoryTo(p);

    io::path project_path = FileSystem->getAbsolutePath(FileSystem->getWorkingDirectory());
    
    File_Open_Tool::get_base()->SetCurrentProjectPath(project_path);

    ReadFromFiles(sc);

    FileSystem->changeWorkingDirectoryTo(restore_path);
}

bool Geometry_Scene_File_IO::WriteMetadata(SceneCoordinator* sc, std::string fname)
{
    ofstream wf(fname, ios::out | ios::binary);

    if (!wf)
    {
        cout << "Cannot open file\n";
        return false;
    }

    int e = sc->scenes.size();
    wf.write((char*)&e, sizeof(int));

    wf.close();
    if (!wf.good())
    {
        cout << "error writing file\n";
        return false;
    }
    return true;
}

bool Geometry_Scene_File_IO::ReadMetadata(std::string fname, metadata& data)
{
    ifstream rf(fname.c_str(), ios::in | ios::binary);

    if (!rf)
    {
        cout << "Cannot open file\n";
        return false;
    }

    rf.read((char*)&data.n_scenes, sizeof(int));

    rf.close();
    if (!rf.good())
    {
        cout << "error reading file\n";
        return false;
    }
    return true;
}

bool Geometry_Scene_File_IO::WriteTextureNames(geometry_scene* geo_scene, std::string fname)
{
    ofstream wf(fname, ios::out | ios::binary);

    if (!wf)
    {
        cout << "Cannot open file\n";
        return false;
    }

    std::vector<video::ITexture*> textures_used;
    std::vector<std::wstring> texture_paths;

    video::IVideoDriver* driver = device->getVideoDriver();
    GeometryStack* geometry_stack = geo_scene->geometry_stack;

    for (int i = 1; i < geometry_stack->elements.size(); i++)
    {
        for (poly_surface& surface : geometry_stack->elements[i].surfaces)
        {
            //video::ITexture* tex_j = driver->getTexture(geometry_stack->elements[i].brush.faces[f_i].texture_name.c_str());
            video::ITexture* tex_j = driver->getTexture(surface.texture_name.c_str());

            bool b = false;
            for (int j = 0; j < textures_used.size(); j++)
            {
                if (tex_j == textures_used[j])
                {
                    surface.texture_index = j;
                    b = true;
                }
            }
            if (!b)
            {
                textures_used.push_back(tex_j);
                texture_paths.push_back(surface.texture_name.c_str());
                surface.texture_index = texture_paths.size() - 1;

            }
        }
    }

    wf << textures_used.size() << "\n";

    for (int i = 0; i < textures_used.size(); i++)
    {
        std::string str(texture_paths[i].begin(), texture_paths[i].end());
        wf << str.c_str() << "\n";
    }

    wf.close();
    return true;
}

bool Geometry_Scene_File_IO::ReadTextures(io::path fname, std::vector<std::wstring>& texture_paths)
{
    ifstream rf(fname.c_str(), ios::in | ios::binary);

    if (!rf)
    {
        cout << "Cannot open file\n";
        return false;
    }

    std::string line;

    int n_textures;
    getline(rf, line);
    n_textures = core::strtoul10(line.c_str());

    for (int i = 0; i < n_textures; i++)
    {
        getline(rf, line);
        std::wstring str(line.begin(), line.end());
        texture_paths.push_back(str.c_str());
    }

    rf.close();
    return true;
}

bool Geometry_Scene_File_IO::SerializeGeometryToFile(geometry_scene* geo_scene, std::string fname)
{
    ofstream wf(fname, ios::out | ios::binary);

    if (!wf)
    {
        cout << "Cannot open file\n";
        return false;
    }

    reflect::TypeDescriptor_Struct* typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<geometry_scene>::get();

    typeDescriptor->serialize(wf, geo_scene);

    wf.close();
    if (!wf.good())
    {
        cout << "error writing file\n";
        return false;
    }
    return true;
}

bool Geometry_Scene_File_IO::DeserializeGeometryFromFile(geometry_scene* geo_scene, io::path fname, io::path tex_fname)
{
    ifstream rf(fname.c_str(), ios::in | ios::binary);

    if (!rf)
    {
        cout << "Cannot open file\n";
        return false;
    }

    std::vector<std::wstring> texture_paths;
    if (ReadTextures(tex_fname, texture_paths) == false)
        return false;

    reflect::TypeDescriptor_Struct* typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<geometry_scene>::get();

    typeDescriptor->deserialize(rf, geo_scene);

    GeometryStack* geometry_stack = geo_scene->geometry_stack;
    geometry_stack->initialize(geo_scene);

    for (int i = 1; i < geometry_stack->elements.size(); i++)
        for (poly_surface& surface : geometry_stack->elements[i].surfaces)
        {
            if (surface.texture_index < texture_paths.size())
                surface.texture_name = texture_paths[surface.texture_index].c_str();
            else
                surface.texture_name = "wall.bmp";
        }
    /*
        for (poly_face& face : geometry_stack->elements[i].brush.faces)
        for (int f_i=0; f_i< geometry_stack->elements[i].brush.faces.size(); f_i++)
        {
            if (face.texture_index < texture_paths.size())
                //face.texture_name = texture_paths[face.texture_index].c_str();
                geometry_stack->elements[i].texture_names[f_i] = texture_paths[face.texture_index].c_str();
            else
                //face.texture_name = "wall.bmp";
                geometry_stack->elements[i].texture_names[f_i] = "wall.bmp";
        }*/

    rf.close();
    if (!rf.good())
    {
        cout << "error reading file\n";
        return false;
    }

    for (int i = 0; i < geometry_stack->elements.size(); i++)
    {

        //Brushes
        geometry_stack->elements[i].brush.reduce_edges_vertices();
        geometry_stack->elements[i].brush.recalc_bbox();

        for (int f_i = 0; f_i < geometry_stack->elements[i].brush.faces.size(); f_i++)
        {
            for (int p_i = 0; p_i < geometry_stack->elements[i].brush.faces[f_i].loops.size(); p_i++)
                geometry_stack->elements[i].brush.calc_loop_bbox(f_i, p_i);
        }

        for (poly_face& f : geometry_stack->elements[i].brush.faces)
        {
            geometry_stack->elements[i].brush.calc_center(f);
        }

        //Geometry
        geometry_stack->elements[i].geometry.reduce_edges_vertices();
        geometry_stack->elements[i].geometry.recalc_bbox();

        for (int f_i = 0; f_i < geometry_stack->elements[i].geometry.faces.size(); f_i++)
        {
            for (int p_i = 0; p_i < geometry_stack->elements[i].geometry.faces[f_i].loops.size(); p_i++)
                geometry_stack->elements[i].geometry.calc_loop_bbox(f_i, p_i);
        }

        for (poly_face& f : geometry_stack->elements[i].geometry.faces)
        {
            geometry_stack->elements[i].geometry.calc_center(f);
        }

    }

    geometry_stack->make_index_lists();

    return true;
}

bool Geometry_Scene_File_IO::SerializeSceneNodesToFile(geometry_scene* geo_scene, std::string fname)
{
    ofstream wf(fname, ios::out | ios::binary);

    if (!wf)
    {
        cout << "Cannot open file\n";
        return false;
    }

    wf << "38\n";

    int e = geo_scene->editor_nodes->getChildren().size();
    wf.write((char*)&e, sizeof(int));

    for (ISceneNode* it : geo_scene->editor_nodes->getChildren())
    {
        Reflected_SceneNode* node = (Reflected_SceneNode*)it;
        node->preEdit();
        reflect::TypeDescriptor_Struct* td = node->GetDynamicReflection();
        wf << node->GetDynamicReflection()->name << '\0';

        while (td)
        {
            td->serialize(wf, node);
            td = td->inherited_type;
        }
    }

    wf.close();
    if (!wf.good())
    {
        cout << "error writing file\n";
        return false;
    }
    return true;
}

bool Geometry_Scene_File_IO::DeserializeSceneNodesFromFile(geometry_scene* geo_scene, io::path fname)
{
    ifstream rf(fname.c_str(), ios::in | ios::binary);

    if (!rf)
    {
        cout << "Cannot open file\n";
        return false;
    }
    std::string line;

    int magic;
    getline(rf, line);
    magic = core::strtoul10(line.c_str());

    if (magic != 38)
    {
        std::cout << fname.c_str() << " is not a valid node file\n";
        rf.close();
        return false;
    }

    core::list<scene::ISceneNode*> child_list = geo_scene->editor_nodes->getChildren();

    core::list<scene::ISceneNode*>::Iterator it = child_list.begin();
    for (; it != child_list.end(); ++it)
    {
        geo_scene->editor_nodes->removeChild(*it);
    }

    int n_nodes;
    rf.read((char*)&n_nodes, sizeof(int));

    for (int i = 0; i < n_nodes; i++)
    {
        getline(rf, line, '\0');
        reflect::TypeDescriptor_Struct* typeDescriptor = Reflected_SceneNode_Factory::getNodeTypeDescriptorByName(line);
        if (typeDescriptor)
        {
            Reflected_SceneNode* new_node = Reflected_SceneNode_Factory::CreateNodeByTypeName(typeDescriptor->name, geo_scene->editor_nodes, geo_scene, geo_scene->smgr);
            if (new_node)
            {
                reflect::TypeDescriptor_Struct* td = typeDescriptor;
                while (td)
                {
                    td->deserialize(rf, new_node);
                    td = td->inherited_type;

                }
                new_node->drop();
                new_node->postEdit(); //this gets called twice for each node but it's probably OK
            }
            else
            {
                std::cout << "*could not create node*\n";
                rf.close();
                return false;
            }
        }
        else
        {
            std::cout << "*could not find type descriptor*\n";
            rf.close();
            return false;
        }
    }

    rf.close();
    if (!rf.good())
    {
        cout << "error reading file\n";
        return false;
    }
    return true;
}

//================================================================
// Scene Coordinator
//

SceneCoordinator::SceneCoordinator(scene::ISceneManager* smgr, video::IVideoDriver* driver, MyEventReceiver* receiver) :
    smgr(smgr),driver(driver),receiver(receiver)
{
    geometry_scene* scene0 = new geometry_scene();
    scene0->initialize(smgr, driver, receiver);
    scene0->set_type(GEO_SOLID);
    scene0->rename("Main Scene");
    scene0->scene_unique_inc_id = 0;

    unique_id_incrementer = 1;

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

    scene->scene_unique_inc_id = unique_id_incrementer;
    unique_id_incrementer++;

    scenes.push_back(reflect::pointer<geometry_scene>{scene});
}

void SceneCoordinator::connect_skybox(geometry_scene* scene)
{
    m_skybox_scene = scene;
}

void SceneCoordinator::clear()
{
    //delete all geometry scenes
    for (int i = 0; i < this->scenes.size(); i++)
    {
        if (this->scenes[i] != NULL)
        {
            if (i > 0)
            {
                //Let the geometry scene clean up the geometry stack
                this->scenes[i]->geoNode()->grab();
                this->scenes[i]->get_smgr()->drop();
            }

            delete this->scenes[i];
        }
    }

    //std::cout << smgr->getRootSceneNode()->getChildren().getSize() << "nodes in smgr\n";

    this->scenes.clear();
    this->scene_no = 0;
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

void SceneCoordinator::CopyAllMaterials()
{
    for (geometry_scene* scene : scenes)
    {
        if (scene->getMeshNode())
            scene->getMeshNode()->copyMaterials();
    }
}

void SceneCoordinator::SetAllFinalMeshDirty()
{
    for (geometry_scene* scene : scenes)
    {
        if (scene->getMeshNode())
            scene->setFinalMeshDirty();
    }
}

//=================================================================
// Geometry Scene
//

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
        

        geo_element red = make_poly_cube(256, 256, 256);
        red.type = GEO_RED;
        geometry_stack->elements.push_back(red);

        geometry_stack->rebuild_geometry();
    }
}

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

    if (geometry_stack != NULL)
    {
        if (smgr)
            geometry_stack->remove();
        else
            geometry_stack->drop();
    }
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

void geometry_scene::setSelectedFaces(std::vector<int> selection, bool force)
{
    if(this->getMeshNode() == NULL || IsEditNode() == false)
        return;

    if(!force && selected_faces.size() == 0 && selection.size() == 0)
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


            //Material_Groups_Tool::apply_material_to_buffer(buffer, pf->faces[f_i].material_group, -1, bSelected, false);
            Material_Groups_Tool::apply_material_to_buffer(buffer, geometry_stack->surface_by_n(f_i)->material_group, -1, bSelected, false);

            if (b_Visualize)
            {
                /// TODO
            }
            
        }
    }

    this->getMeshNode()->copyMaterials();

    this->selected_faces = selection;

}

void geometry_scene::setSelectedNodes(std::vector<Reflected_SceneNode*> selection)
{
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

            polyfold* pf = geometry_stack->get_total_geometry();

            face_index face_idx_0 = pf->index_face(ii);
            int sg_ii = geometry_stack->get_brush_face(face_idx_0)->surface_group;

            for (int j : unique_faces)
            {

                face_index face_idx_1 = pf->index_face(j);
                int sg_j = geometry_stack->get_brush_face(face_idx_1)->surface_group;

                if (face_idx_0.brush == face_idx_1.brush && sg_j == sg_ii)
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

        std::vector<int> total_sel;

        for (int i = 0; i < unique_faces.size(); i++)
        {
            std::vector<int> sel = geometry_stack->get_total_geometry()->getSurfaceFromFace(unique_faces[i]);

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

surface_group geometry_scene::getFaceSurfaceGroup(int f_i)
{

    int element_id = geometry_stack->get_total_geometry()->faces[f_i].element_id;
    geo_element* element = geometry_stack->get_element_by_id(element_id);
    int face_j = geometry_stack->get_total_geometry()->faces[f_i].face_id;

    poly_face* f = &element->brush.faces[face_j];

    return element->brush.surface_groups[f->surface_group];
}

std::vector<std::pair<int, int>> geometry_scene::getSelectedBlocks()
{
    polyfold* pf = geometry_stack->get_total_geometry();

    std::vector<std::pair<int, int>> ret;

    for (int f_i : selected_faces)
    {
        pf->faces[f_i].temp_b = false;
    }

    for (int f_i : selected_faces)
    {
        if (pf->faces[f_i].temp_b)
            continue;

        vector<int> surface;

        surface_group& sfg = *pf->getFaceSurfaceGroup(f_i);
        int sfg_i = pf->getFaceSurfaceGroupNo(f_i);

        switch (sfg.type)
        {
        case SURFACE_GROUP_CYLINDER:
        case SURFACE_GROUP_DOME:
        case SURFACE_GROUP_SPHERE:
            surface = pf->getSurfaceFromFace(f_i);
            break;
        default:
            surface = vector<int>{ f_i };
            break;
        }

        for (int f_j : surface)
        {
            pf->faces[f_j].temp_b = true;
        }

        int i = geometry_stack->edit_meshnode_interface.get_material_group_by_face(f_i);
        int j = geometry_stack->edit_meshnode_interface.get_lm_block_by_face(f_i);

        ret.push_back(std::pair<int, int>{i, j});
    }

    return ret;
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
        if(geometry_stack->selected_brush_vertex_editing == p_i && geometry_stack->elements[p_i].control_vertex_selected == false && geometry_stack->elements[p_i].selected_vertex < geometry_stack->elements[p_i].brush.vertices.size())
        {
            bSelectedVertex=true;
            ret=geometry_stack->elements[p_i].brush.vertices[ geometry_stack->elements[p_i].selected_vertex].V;
        }
        else if(geometry_stack->selected_brush_vertex_editing == p_i && geometry_stack->elements[p_i].control_vertex_selected == true && geometry_stack->elements[p_i].selected_vertex < geometry_stack->elements[p_i].brush.control_vertices.size())
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

void geo_element::draw_brush(video::IVideoDriver* driver)
{
    video::SColor col = this->getColor();

    for(int i =0; i<this->brush.edges.size(); i++)
    {
        if(this->brush.edges[i].topo_group != -1)
            driver->draw3DLine(this->brush.getVertex(i,0).V,this->brush.getVertex(i,1).V, col);
    }
}
video::SColor geo_element::getColor()
{

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
        //int element_id = geometry_stack->get_total_geometry()->faces[i].element_id;
        //int face_i = geometry_stack->get_total_geometry()->faces[i].face_id;
        //geo_element* element = geometry_stack->get_element_by_id(element_id);

        if (geometry_stack->surface_by_n(i)->material_group != mg)
        {
            geometry_stack->surface_by_n(i)->material_group = mg;
            dirty_mesh = true;
        }

        /*if (geometry_stack->get_total_geometry()->faces[i].material_group != mg ||
            element->brush.faces[face_i].material_group != mg ||
            element->geometry.faces[face_i].material_group != mg)
        {
            element->brush.faces[face_i].material_group = mg;
            element->geometry.faces[face_i].material_group = mg;

            geometry_stack->get_total_geometry()->faces[i].material_group = mg;

            dirty_mesh = true;
        }
        */
    }

    if (b_Visualize)
        visualizeMaterialGroups();

    if (dirty_mesh)
    {
        const Material_Group& material = material_groups_base->material_groups[mg];

        if (material.create_meshBuffer_Node)
        {
            setFaceNodeType(selected_faces, material.meshBuffer_Node_typeName);
        }
        else
        {
            setFaceNodeType(selected_faces, NULL);
        }

        geometry_stack->setFinalMeshDirty();
    }
}

void geometry_scene::TextureToSelectedFaces()
{
    if(!this->getMeshNode() || !IsEditNode())
        return;

    for(int i: this->selected_faces)
    {
        //int element_id = geometry_stack->get_total_geometry()->faces[i].element_id;
       // int face_i = geometry_stack->get_total_geometry()->faces[i].face_id;
       // geo_element& element = geometry_stack->get_element_by_id(element_id);

        //element.surfaces[]
        poly_surface* surface = geometry_stack->surface_by_n(i);
        surface->texture_name = TexturePicker_Tool::getCurrentTexture()->getName().getPath();

        //element.texture_names[face_i] = TexturePicker_Tool::getCurrentTexture()->getName().getPath();

       // element.brush.faces[face_i].texture_name=core::stringw(TexturePicker_Tool::getCurrentTexture()->getName().getPath());
       // element.geometry.faces[face_i].texture_name= TexturePicker_Tool::getCurrentTexture()->getName().getPath();
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

void geometry_scene::set_selected_brush_vertex_editing(int brush_i)
{
    if (geometry_stack)
        geometry_stack->selected_brush_vertex_editing = brush_i;
}

void geometry_scene::drawGraph(LineHolder& graph)
{
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
        Lightmaps_Tool::get_manager()->loadLightmapTextures(this);
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

std::vector<Reflected_SceneNode*> geometry_scene::get_reflected_nodes_by_type(const char* node_type_name)
{
    reflect::TypeDescriptor_Struct* tD = NULL;

    if (node_type_name != NULL)
        tD = Reflected_SceneNode_Factory::getNodeTypeDescriptorByName(node_type_name);

    if (tD == NULL)
        return std::vector<Reflected_SceneNode*>{};

    std::vector<Reflected_SceneNode*> ret;

    core::list<ISceneNode*> nodes = editor_nodes->getChildren();

    core::list<scene::ISceneNode*>::Iterator it = nodes.begin();
    for (; it != nodes.end(); ++it)
    {
        Reflected_SceneNode* node = (Reflected_SceneNode*)*it;
        if (node->GetDynamicReflection()->isOfType(tD))
        {
            ret.push_back(node);
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

    polyfold* pf = geometry_stack->get_total_geometry();

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

                int face_id = pf->faces[f_i].face_id;
                int element_id = pf->faces[f_i].element_id;

                if (meshbuffer_node->get_face_id() == element_id &&
                    meshbuffer_node->get_element_id() == face_id)
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
                vector3df pos = pf->faces[f_i].m_center;
            }
        }
    }

    c = 0;
    if (tD)
    {
        for (int i = 0; i < faces_i.size(); i++)
        {
            if (!has_node[i])
            {
                int f_i = faces_i[i];
                vector3df pos = pf->faces[f_i].m_center;

                int face_id = pf->faces[f_i].face_id;
                int element_id = pf->faces[f_i].element_id;

                Reflected_SceneNode* node = ((reflect::TypeDescriptor_SN_Struct*)tD)->create_func(editor_nodes, this, smgr, -1, pos);
                
                node->drop();
                node->preEdit();       
                ((Reflected_MeshBuffer_SceneNode*)node)->set_face(element_id, face_id);
                node->postEdit();

                c++;
            }
        }
    }

    //caller calls
    //gs_coordinator->cleanup_after_scene_nodes_added_deleted();
}

void geometry_scene::addFaceNode(int f_i)
{
    polyfold* pf = geometry_stack->get_total_geometry();

    int face_id = pf->faces[f_i].face_id;
    int element_id = pf->faces[f_i].element_id;
    vector3df pos = pf->faces[f_i].m_center;

    Reflected_MeshBuffer_SceneNode* node = new Reflected_MeshBuffer_SceneNode(editor_nodes, this, smgr, -1, pos);

    node->set_face(element_id, face_id);

    node->preEdit();
}

void geometry_scene::rebuildSceneGraph()
{

}

void geometry_scene::buildSceneGraph(bool addObjects, int light_mode, bool finalscene)
{
    if(finalscene)
        cout << this->scene_name << ": building Scene Graph (final)\n";
    else
        cout << this->scene_name << ": building Scene Graph (edit)\n";

    core::list<scene::ISceneNode*> child_list = actual_nodes->getChildren();

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
        }
    }
    
   // if(finalscene)
   //     geometry_stack->recalculate_final_meshbuffer();

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

    if (endScene)
        geoNode()->getMeshNode()->copyMaterials();

    if (finalscene)
        cout << "(Scene has " << actual_nodes->getChildren().getSize() << " nodes)\n";

    child_list = smgr->getRootSceneNode()->getChildren();

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

    b_Visualize = false;
}

void geometry_scene::beginScene()
{
    core::list<scene::ISceneNode*> child_list = editor_nodes->getChildren();
    for (core::list<scene::ISceneNode*>::Iterator it = child_list.begin(); it != child_list.end(); ++it)
    {
        Reflected_SceneNode* node = (Reflected_SceneNode*)(*it);

        node->onSceneInit();
    }

    //Caller calls
    //geoNode()->getMeshNode()->copyMaterials();
}


void geometry_scene::clear_scene()
{
    this->selected_brushes.clear();
    this->selected_faces.clear();
    this->selected_scene_nodes.clear();

    core::list<scene::ISceneNode*> child_list = actual_nodes->getChildren();
    core::list<scene::ISceneNode*>::Iterator it = child_list.begin();
    for (; it != child_list.end(); ++it)
    {
        (*it)->remove();
    }

    child_list = editor_nodes->getChildren();
    it = child_list.begin();
    for (; it != child_list.end(); ++it)
    {
        (*it)->remove();
    }
  
    geometry_stack->clear_scene();
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
        selectionChanged();
    }
}

void geometry_scene::addSceneLight(core::vector3df pos)
{
    Reflected_SceneNode* a_light = new Reflected_LightSceneNode(editor_nodes,this,smgr,-1,pos);
    a_light->drop();
    a_light->preEdit();

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

            //gs_coordinator->cleanup_after_scene_nodes_added_deleted();
        }
    }
}

void geometry_scene::deleteSelectedNodes()
{
    if (selected_scene_nodes.size() == 0)
        return;

    core::list<scene::ISceneNode*> child_list = editor_nodes->getChildren();

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
                    {
                        editor_nodes->removeChild(*it);
                    }
                else
                    cout << "cannot delete " << tD->getAlias() << "\n";
            }
        }
    }

    selected_scene_nodes.clear();

    selectionChanged();

   // gs_coordinator->cleanup_after_scene_nodes_added_deleted();
}

REFLECT_STRUCT_BEGIN(geometry_scene)
    REFLECT_STRUCT_MEMBER(base_type)
    REFLECT_STRUCT_MEMBER(geometry_stack)
    REFLECT_STRUCT_MEMBER(scene_name)
    REFLECT_STRUCT_MEMBER(saved_gui_state)
    REFLECT_STRUCT_MEMBER(scene_unique_inc_id)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(SceneCoordinator)
    REFLECT_STRUCT_MEMBER(scenes)
    REFLECT_STRUCT_MEMBER(unique_id_incrementer)
REFLECT_STRUCT_END()
