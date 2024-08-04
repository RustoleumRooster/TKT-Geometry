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

geometry_scene::geometry_scene(video::IVideoDriver* driver_, MyEventReceiver* receiver)
{
    this->driver = driver_;
    this->event_receiver = receiver;
    event_receiver->Register(this);
}

geometry_scene::geometry_scene(scene::ISceneManager* smgr_,video::IVideoDriver* driver_,MyEventReceiver* receiver, video::E_MATERIAL_TYPE base_material_type_, video::E_MATERIAL_TYPE special_material_type_)
{
    this->smgr = smgr_;
    this->driver = driver_;
    this->base_material_type = base_material_type_;
    this->special_material_type = special_material_type_;
    this->event_receiver = receiver;
    event_receiver->Register(this);
    geo_element red;
    red.brush= make_poly_cube(256,256,256);
    red.type=GEO_RED;
    elements.push_back(red);

    this->edit_meshnode_interface.init(smgr,driver,event_receiver,base_material_type,special_material_type);
    this->final_meshnode_interface.init(smgr,driver,event_receiver,base_material_type,special_material_type);
}

geometry_scene::~geometry_scene()
{
    event_receiver->UnRegister(this);
}

void geometry_scene::setMaterialGroupsBase(Material_Groups_Base* base)
{
    material_groups_base = base;
}

Material_Groups_Base* geometry_scene::getMaterialGroupsBase()
{
    return material_groups_base;
}

void geometry_scene::visualizeMaterialGroups()
{
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

    b_Visualize = true;
}

void geometry_scene::showLightMaps()
{

}

void geometry_scene::setLightmapManager(Lightmap_Manager* lm)
{
    lightmap_manager = lm;
}

void geometry_scene::setTexturePickerBase(TexturePicker_Base* texp)
{
    texture_picker_base = texp;
}

TexturePicker_Base* geometry_scene::getTexturePickerBase()
{
    return texture_picker_base;
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

void geometry_scene::setSelectedNodes_ShiftAdd(int new_sel)
{
    bool b = false;
    std::vector<int> selection;

    for(int f_i : getSelectedNodes())
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

    polyfold* pf = get_total_geometry();

    for(int f_i=0;f_i<pf->faces.size();f_i++)
    {
        if(pf->faces[f_i].loops.size()>0)
        {
            int buffer_index = edit_meshnode_interface.get_buffer_index_by_face(f_i);
            scene::IMeshBuffer* buffer = this->getMeshNode()->getMesh()->getMeshBuffer(buffer_index);
            bool bSelected = false;
            for(int f_j : selection)
                if(f_i == f_j)
                    bSelected=true;

            buffer->getMaterial().Lighting = false;


            material_groups_base->apply_material_to_buffer(buffer, pf->faces[f_i].material_group, DynamicLightEnabled(), bSelected);

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

void geometry_scene::setSelectedNodes(std::vector<int> selection)
{
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
    };

    for(int i=0; i<this->scene_nodes.size();i++)
    {
        if(IsInSelection(i))
        {
            this->scene_nodes[i]->Select();
        }
        else
        {
            this->scene_nodes[i]->UnSelect();
        }
    }

    this->selected_scene_nodes = selection;
}

std::vector<int> geometry_scene::getSurfaceFromFace(int b_i)
{
    int brush_j = get_total_geometry()->faces[b_i].original_brush;
    int face_j = get_total_geometry()->faces[b_i].original_face;

    poly_face* f = &elements[brush_j].brush.faces[face_j];
    std::vector<int> sel;
    int sg = f->surface_group;
    for(int i=0; i<get_total_geometry()->faces.size(); i++)
    {
        if(get_total_geometry()->faces[i].original_brush == brush_j)
        {
            int face_i = get_total_geometry()->faces[i].original_face;

            if(elements[brush_j].brush.faces[face_i].surface_group == sg &&
               elements[brush_j].geometry.faces[face_i].loops.size() > 0)
                sel.push_back(i);
        }
    }
    return sel;

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

            int brush_ii = get_total_geometry()->faces[ii].original_brush;
            int face_ii = get_total_geometry()->faces[ii].original_face;
            int sg_ii = elements[brush_ii].brush.faces[face_ii].surface_group;

            for (int j : unique_faces)
            {
                int brush_j = get_total_geometry()->faces[j].original_brush;
                int face_j = get_total_geometry()->faces[j].original_face;
                int sg_j = elements[brush_j].brush.faces[face_j].surface_group;

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
            std::vector<int> sel = getSurfaceFromFace(unique_faces[i]);

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
    int brush_j = get_total_geometry()->faces[b_i].original_brush;
    int face_j = get_total_geometry()->faces[b_i].original_face;

    poly_face* f = &elements[brush_j].brush.faces[face_j];

    return elements[brush_j].brush.surface_groups[f->surface_group];
}

std::vector<int> geometry_scene::getSelectedFaces()
{
    return this->selected_faces;
}

std::vector<int> geometry_scene::getSelectedNodes()
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

    for(int i=0; i<elements.size(); i++)
    {
        this->elements[i].bSelected=false;
    }

    for(int i: new_sel)
        this->elements[i].bSelected=true;
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
        if(this->selected_brush_vertex_editing==p_i && this->elements[p_i].selected_vertex< this->elements[p_i].brush.vertices.size())
        {
            bSelectedVertex=true;
            ret=this->elements[p_i].brush.vertices[ this->elements[p_i].selected_vertex].V;
        }
    }
    if(!bSelectedVertex && this->elements.size()>0)
    {
        ret=this->elements[this->getBrushSelection()[0]].brush.vertices[0].V;
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


void geometry_scene::set_type(int t)
{
    this->base_type=t;
}

void geometry_scene::MaterialGroupToSelectedFaces()
{
    if (!material_groups_base || !getMeshNode() || !IsEditNode() || !get_total_geometry())
        return;

    int mg = material_groups_base->getSelected();

    if (mg == -1)
        return;

    for (int i : selected_faces)
    {
        int brush_i = get_total_geometry()->faces[i].original_brush;
        int face_i =get_total_geometry()->faces[i].original_face;

        elements[brush_i].brush.faces[face_i].material_group = mg;
        elements[brush_i].geometry.faces[face_i].material_group = mg;

        get_total_geometry()->faces[i].material_group = mg;
    }

    if (b_Visualize)
        visualizeMaterialGroups();

    setFinalMeshDirty();
}

void geometry_scene::TextureToSelectedFaces()
{
    if(!getTexturePickerBase() || !this->getMeshNode() || !IsEditNode())
        return;

    for(int i: this->selected_faces)
    {
        int brush_i = this->get_total_geometry()->faces[i].original_brush;
        int face_i = this->get_total_geometry()->faces[i].original_face;

        this->elements[brush_i].brush.faces[face_i].texture_name=core::stringw(texture_picker_base->getCurrentTexture()->getName().getPath());
        this->elements[brush_i].geometry.faces[face_i].texture_name=texture_picker_base->getCurrentTexture()->getName().getPath();
    }

    for(int i=0; i<this->getMeshNode()->getMesh()->getMeshBufferCount();i++)
    {
        bool b=false;
        for(int j : selected_faces)
        {
            int buffer_index = edit_meshnode_interface.get_buffer_index_by_face(j);
            if(i==buffer_index)
                b=true;
        }
        if(b)
            getMeshNode()->SetFaceTexture(i,texture_picker_base->getCurrentTexture());
    }

    setFinalMeshDirty();
}

poly_face* geometry_scene::get_original_brush_face(int f_i)
{
    int brush_j = total_geometry.faces[f_i].original_brush;
    int face_j =  total_geometry.faces[f_i].original_face;

    poly_face* f = &elements[brush_j].brush.faces[face_j];

    return f;
}


polyfold* geometry_scene::get_original_brush(int f_i)
{
    int brush_j =  total_geometry.faces[f_i].original_brush;

    polyfold* pf = &elements[brush_j].brush;

    return pf;
}

polyfold geometry_scene::get_intersecting_geometry(polyfold pf)
{
    polyfold combo;
    std::vector<int> touched_brushes;
    int num=0;
    std::vector<polyfold*> polies;
    for(int j=1; j<this->elements.size(); j++)
    {
        if(this->elements[j].has_geometry() &&
         BoxIntersectsWithBox(pf.bbox,this->elements[j].brush.bbox))
        {
            for(int f_j =0; f_j<this->elements[j].geometry.faces.size(); f_j++)
            {
                this->elements[j].geometry.faces[f_j].original_brush=j;
                this->elements[j].geometry.faces[f_j].original_face=this->elements[j].geometry.faces[f_j].surface_group;
            }
            touched_brushes.push_back(j);

            //add_pfold(this->elements[j].geometry,combo);
            polies.push_back(&elements[j].geometry);
            num++;
        }
    }
    combine_polyfolds(polies, combo);

    return combo;
}

void geometry_scene::drawGraph(LineHolder& graph)
{

    //for (line3df el : intersections_graph.lines)
    //    graph.lines.push_back(el);
    graph.lines.clear();

    for (const line3df& el : special_graph.lines)
        graph.lines.push_back(el);
}

triangle_holder* geometry_scene::get_triangles_for_face(int f_i)
{
    return &total_geometry_triangles[f_i];
}

void geometry_scene::trianglize_total_geometry()
{
    LineHolder nograph;
    polyfold* pf = get_total_geometry();

    total_geometry_triangles.clear();

    total_geometry_triangles.resize(pf->faces.size());

    for (int i = 0; i < pf->faces.size(); i++)
    {
        if (pf->faces[i].loops.size() > 0)
        {
            pf->trianglize(i, total_geometry_triangles[i], NULL, nograph, nograph);
        }
    }
}

void geometry_scene::generate_meshes()
{
    trianglize_total_geometry();

    edit_meshnode_interface.generate_mesh_node(this);

    final_meshnode_interface.refresh_material_groups(this);
    final_meshnode_interface.generate_mesh_node(this);
}

void geometry_scene::rebuildSceneGraph()
{

}

void geometry_scene::buildSceneGraph(bool finalMesh, bool addObjects, bool addLights, bool finalscene)
{
   // std::cout << "rebuilding scene...";
    //std::cout << "edit objects "<< addObjects<< ", final " << finalMesh << ", add lights " << addLights << "\n";

    core::list<scene::ISceneNode*> child_list = smgr->getRootSceneNode()->getChildren();

    if(my_MeshNode)
        my_MeshNode->drop();

    core::list<scene::ISceneNode*>::Iterator it = child_list.begin();
    for(;it != child_list.end();++it)
    {
        int id = (*it)->getID();
        if( (*it)->getType() != scene::ESNT_CAMERA )
            smgr->getRootSceneNode()->removeChild(*it);
    }

    for(Reflected_SceneNode* node : this->scene_nodes)
    {
        node->onClear();

        if(addObjects)
        {
            node->setParent(smgr->getRootSceneNode());
            node->setUnlit(!addLights);
        }

        if (finalscene ||
           (addLights && node->getType() == ESNT_LIGHT))
            node->addSelfToScene(smgr);
    }

    if (final_mesh_dirty)
    {
        //std::cout << "regenerating final mesh...\n";
        final_meshnode_interface.refresh_material_groups(this);
        final_meshnode_interface.generate_mesh_node(this);
    }

    polyfold* pf = get_total_geometry();

    if(finalMesh) //final mesh
    {
        b_isEditNode = false;
        this->my_MeshNode = final_meshnode_interface.addMeshSceneNode(smgr,this);

        for(int i = 0; i< my_MeshNode->getMesh()->getMeshBufferCount(); i++)
        {
            scene::IMeshBuffer* buffer = my_MeshNode->getMesh()->getMeshBuffer(i);
            int f_i = final_meshnode_interface.getMaterialsUsed()[i].faces[0];
            material_groups_base->apply_material_to_buffer(buffer,pf->faces[f_i].material_group,addLights,false);
            
        }
        my_MeshNode->copyMaterials();
    }
    else //edit mesh
    {
        b_isEditNode = true;
        this->my_MeshNode = edit_meshnode_interface.addMeshSceneNode(smgr,this);

        for(int f_i = 0; f_i< pf->faces.size(); f_i++)
        {
            if(pf->faces[f_i].loops.size() >0)
            {
                int buffer_index = edit_meshnode_interface.get_buffer_index_by_face(f_i);
                scene::IMeshBuffer* buffer = this->getMeshNode()->getMesh()->getMeshBuffer(buffer_index);

                material_groups_base->apply_material_to_buffer(buffer,pf->faces[f_i].material_group,addLights,false);

                video::ITexture* tex_j = driver->getTexture(pf->faces[f_i].texture_name.c_str());

                buffer->getMaterial().setTexture(0, tex_j);
            }
        }
        my_MeshNode->copyMaterials();
    }

    this->my_MeshNode->setMaterialFlag(video::EMF_LIGHTING, false);

    if(addLights)
    {
        b_dynamic_light = true;
    }
    else
    {
        b_dynamic_light = false;
    }

    this->my_MeshNode->getMesh()->setHardwareMappingHint(scene::EHM_STATIC);

    //my_MeshNode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS,true);

    my_MeshNode->setVisible(true);

    b_Visualize = false;

    //my_MeshNode->setDebugDataVisible(scene::EDS_NORMALS);
}


void geometry_scene::clear_scene()
{
    //std::cout<<"clearing scene... ";

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

    for(Reflected_SceneNode* node : this->scene_nodes)
    {
        node->remove();
        node->drop();
    }
    this->scene_nodes.clear();

    this->rebuild_geometry();
}

void geometry_scene::delete_selected_brushes()
{
    std::vector<geo_element> new_elements;

    int removed=0;
    new_elements.push_back(this->elements[0]);
    for(int i=1; i<this->elements.size(); i++)
    {
        bool b=false;
        for(int j:this->selected_brushes)
            if(j==i)
                b=true;

        if(!b)
            new_elements.push_back(this->elements[i]);
        else
            removed++;
    }
    this->elements=new_elements;

    this->selected_brushes.clear();

    if(removed > 0)
    {
       // std::cout<<"deleted "<<removed<<" elements\n";
        selectionChanged();
    }
}

reflect::TypeDescriptor_Struct* geometry_scene::getChooseNodeType()
{
    return this->choose_reflected_node_base->getSelectedTypeDescriptor();
}

void geometry_scene::addSceneLight(core::vector3df pos)
{
    Reflected_SceneNode* a_light = new Reflected_LightSceneNode(smgr,-1,pos);
    a_light->preEdit();
    scene_nodes.push_back(a_light);
    if(b_dynamic_light)
        a_light->addSelfToScene(smgr);
}

void geometry_scene::addSceneSelectedSceneNodeType(core::vector3df pos)
{
    reflect::TypeDescriptor_Struct* typeDescriptor = choose_reflected_node_base->getSelectedTypeDescriptor();
    if(typeDescriptor)
    {
        //Reflected_SceneNode* a_thing = CreateNodeByTypeName(typeDescriptor->name, smgr);
        Reflected_SceneNode* a_thing = Reflected_SceneNode_Factory::CreateNodeByTypeName(typeDescriptor->name, smgr);
        if(a_thing)
        {
            std::cout<<"added a "<<typeDescriptor->name<<"\n";
            a_thing->setPosition(pos);
            a_thing->preEdit();
            scene_nodes.push_back(a_thing);
            //typeDescriptor->dump(a_thing,0);
        }
    }
}

void geometry_scene::deleteSelectedNodes()
{
    std::vector<Reflected_SceneNode*> new_nodes;
    int removed=0;
    for(int i=0;i<scene_nodes.size();i++)
    {
        Reflected_SceneNode* node = scene_nodes[i];

        bool b=false;
        for(int j : getSelectedNodes())
            if(i==j)
                b=true;

        if(b)
        {
            removed++;
            node->remove();
            node->drop();
        }
        else
            new_nodes.push_back(node);
    }
    std::cout<<"deleted "<<removed<<" nodes\n";

    selected_scene_nodes = std::vector<int>{};

    scene_nodes = new_nodes;

    if(removed > 0)
        selectionChanged();
}

std::vector<Reflected_SceneNode*> geometry_scene::getSceneNodes()
{
    return scene_nodes;
}

REFLECT_STRUCT_BEGIN(geo_element)
    REFLECT_STRUCT_MEMBER(type)
    REFLECT_STRUCT_MEMBER(brush)
    REFLECT_STRUCT_MEMBER(geometry)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(geometry_scene)
    REFLECT_STRUCT_MEMBER(base_type)
    REFLECT_STRUCT_MEMBER(elements)
REFLECT_STRUCT_END()
