#include <irrlicht.h>
#include "reflection.h"
#include "reflected_nodes.h"
#include "CameraPanel.h"
#include "utils.h"
#include "geometry_scene.h"
#include "GUI_tools.h"
#include "my_reflected_nodes.h"
#include "TwoTrianglesNode.h"
#include "CMeshSceneNode.h"
#include "material_groups.h"
#include "my_nodes.h"
#include "ShaderCallbacks.h"
#include "texture_picker.h"
#include "heightmap_projection.h"

using namespace irr;
using namespace video;

extern IrrlichtDevice* device;
extern f32 global_clipping_plane[4];
extern SceneCoordinator* gs_coordinator;

extern float g_time;

//NB: If you add new source files that have reflected scene nodes, you may have to move this
//initialization to ensure that it executes before the initialization of the reflected scene nodes, 
//or they will register ahead of time and get wiped off of the master list
Reflected_SceneNode_Factory Reflected_SceneNode::factory{};

//============================================================================
//  Reflected_LightSceneNode
//

REFLECT_STRUCT2_BEGIN(Reflected_LightSceneNode)
    ALIAS("Light")
    INHERIT_FROM(Reflected_Sprite_SceneNode)
    REFLECT_STRUCT2_MEMBER(enabled)
    REFLECT_STRUCT2_MEMBER(light_radius)
REFLECT_STRUCT2_END()

void Reflected_LightSceneNode::postEdit()
{
    if (my_light)
    {
        my_light->setRadius(light_radius);
        my_light->setVisible(enabled);
    }

    Reflected_SceneNode::postEdit();
}

void Reflected_LightSceneNode::translate(core::matrix4 M)
{
    core::vector3df p = this->getPosition();
    M.translateVect(p);
    if (my_light)
        my_light->setPosition(p);

    Reflected_SceneNode::translate(M);
}

bool Reflected_LightSceneNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    scene::ILightSceneNode* light = smgr->addLightSceneNode(parent, getPosition(), video::SColorf(1.0, 1.0, 1.0), this->light_radius, this->ID);
    this->my_light = light;

    return true;
}


//============================================================================
//  Reflected_SunlightNode
//

REFLECT_STRUCT2_BEGIN(Reflected_SunlightNode)
    ALIAS("Sunlight")
    INHERIT_FROM(Reflected_Sprite_SceneNode)
REFLECT_STRUCT2_END()


//============================================================================
//  Reflected_SimpleEmitterSceneNode
//

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

void Reflected_SimpleEmitterSceneNode::render()
{
    video::IVideoDriver* driver = SceneManager->getVideoDriver();

    if (bSelected)
    {/*
        core::aabbox3df box = core::aabbox3df(getPosition().X - EmitBox.X /2,
                                              getPosition().Y - EmitBox.Y /2,
                                              getPosition().Z - EmitBox.Z /2,
                                              getPosition().X + EmitBox.X /2,
                                              getPosition().Y + EmitBox.Y /2,
                                              getPosition().Z + EmitBox.Z /2);*/
        core::aabbox3df box = core::aabbox3df(Location.X - EmitBox.X / 2,
            Location.Y - EmitBox.Y / 2,
            Location.Z - EmitBox.Z / 2,
            Location.X + EmitBox.X / 2,
            Location.Y + EmitBox.Y / 2,
            Location.Z + EmitBox.Z / 2);

        draw_box(driver, box);
    }

    Reflected_Sprite_SceneNode::render();
}


bool Reflected_SimpleEmitterSceneNode::addSelfToScene(USceneNode* parent, scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    //std::cout<<"go!\n";
    core::aabbox3df box = core::aabbox3df(Location.X - EmitBox.X / 2,
        Location.Y - EmitBox.Y / 2,
        Location.Z - EmitBox.Z / 2,
        Location.X + EmitBox.X / 2,
        Location.Y + EmitBox.Y / 2,
        Location.Z + EmitBox.Z / 2);

    //std::cout<<Location.X<<","<<Location.Y<<","<<Location.Z<<"\n";

    scene::IParticleSystemSceneNode* ps = smgr->addParticleSystemSceneNode(false, parent, -1, vector3df(0.0,0.0,0.0), Rotation, vector3df(1.0, 1.0, 1.0));
    /*
    scene::IParticleEmitter* em = ps->createBoxEmitter(core::aabbox3df(-5000,500,-5000,5000,1000,5000),
                                                        core::vector3df(0,0.0,0),
                                                        500,1000,
                                                        video::SColor(255,255,255,255),
                                                        video::SColor(255,255,255,255),
                                                        400,1000);
                                                        */

    scene::IParticleEmitter* em = ps->createBoxEmitter(box,
        core::vector3df(0, 0.0, 0),
        minParticlesPerSecond,
        maxParticlesPerSecond,
        video::SColor(255, 255, 255, 255),
        video::SColor(255, 255, 255, 255),
        lifeTimeMin, lifeTimeMax);
    em->setMinStartSize(core::dimension2df(32, 32));
    em->setMaxStartSize(core::dimension2df(48, 48));
    em->setDirection(core::vector3df(0, 0.09, 0));

    ps->setMaterialFlag(video::EMF_LIGHTING, false);
    video::IVideoDriver* driver = smgr->getVideoDriver();
    ps->setMaterialTexture(0, driver->getTexture("fireball.bmp"));
    //ps->setMaterialTexture(0,this->texture);
    ps->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    ps->setEmitter(em);
    em->drop();

    scene::IParticleAffector* paf = ps->createFadeOutParticleAffector();
    ps->addAffector(paf);
    paf->drop();

    paf = ps->createScaleParticleAffector(particle_scale);
    ps->addAffector(paf);
    paf->drop();

    //paf = ps->createGravityAffector()

  /*
    scene::IParticleSystemSceneNode* ps = smgr->addParticleSystemSceneNode();
    scene::IParticleEmitter* em = ps->createBoxEmitter(core::aabbox3df(-5000,500,-5000,5000,1000,5000),
                                                        core::vector3df(0,0.0,0),
                                                        500,1000,
                                                        video::SColor(255,255,255,255),
                                                        video::SColor(255,255,255,255),
                                                        400,1000);
    ps->setMaterialFlag(video::EMF_LIGHTING, false);
    ps->setMaterialTexture(0,driver->getTexture("sun.jpg"));
    ps->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    em->setMinStartSize(core::dimension2df(16,16));
    em->setMaxStartSize(core::dimension2df(32,32));
    ps->setEmitter(em);
    em->drop();

    scene::IParticleAffector* paf = ps->createFadeOutParticleAffector();
    ps->addAffector(paf);
    paf->drop();

    paf = ps->createScaleParticleAffector(core::dimension2df(64,64));
    ps->addAffector(paf);
    paf->drop();
    */
    return true;
}

//============================================================================
//  Reflected_ReferenceImageNode
//

REFLECT_STRUCT2_BEGIN(Reflected_ReferenceImageNode)
    ALIAS("Reference Image")
    INHERIT_FROM(Reflected_SceneNode)
    REFLECT_STRUCT2_MEMBER(texture)
    REFLECT_STRUCT2_MEMBER(Size)
    REFLECT_STRUCT2_MEMBER(texture_name)
    REFLECT_STRUCT2_MEMBER(ProjectToTerrain)
    REFLECT_STRUCT_MEMBER_ALIAS("Project to Terrain...")
REFLECT_STRUCT2_END()

Reflected_ReferenceImageNode::Reflected_ReferenceImageNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos)
    : Reflected_SceneNode(parent, geo_scene, smgr, id, pos)
{
    Size.X = 512;
    Size.Y = 512;

    setAutomaticCulling(scene::EAC_OFF);

    Buffer = new scene::SMeshBuffer();
    Buffer->Material.Lighting = false;
    Buffer->Material.BackfaceCulling = false;
    Buffer->Material.MaterialType = video::EMT_SOLID;

    Buffer->Vertices.set_used(4);
    Buffer->Indices.set_used(6);

    Buffer->Indices[0] = 0;
    Buffer->Indices[1] = 1;
    Buffer->Indices[2] = 2;
    Buffer->Indices[3] = 0;
    Buffer->Indices[4] = 2;
    Buffer->Indices[5] = 3;

    set_buffer();
}

Reflected_ReferenceImageNode::~Reflected_ReferenceImageNode()
{
    if (Buffer)
        Buffer->drop();
}

void Reflected_ReferenceImageNode::set_buffer()
{
    f32 hx = Size.X * 0.5f;
    f32 hz = Size.Y * 0.5f;

    //Local XZ plane, facing +Y. Image top-left (0,0) maps to the -X,+Z corner.
    core::vector3df pos[4] = {
        core::vector3df(-hx, 0, hz),
        core::vector3df(hx, 0, hz),
        core::vector3df(hx, 0, -hz),
        core::vector3df(-hx, 0, -hz) };

    core::vector2df uv[4] = {
        core::vector2df(0, 0),
        core::vector2df(1, 0),
        core::vector2df(1, 1),
        core::vector2df(0, 1) };

    Buffer->BoundingBox.reset(pos[0]);

    for (int i = 0; i < 4; i++)
    {
        Buffer->Vertices[i].Pos = pos[i];
        Buffer->Vertices[i].Normal = core::vector3df(0, 1, 0);
        Buffer->Vertices[i].Color = video::SColor(255, 255, 255, 255);
        Buffer->Vertices[i].TCoords = uv[i];
        Buffer->BoundingBox.addInternalPoint(pos[i]);
    }

    Buffer->setDirty();
}

void Reflected_ReferenceImageNode::OnRegisterSceneNode()
{
    if (IsVisible)
        SceneManager->registerNodeForRendering(this, scene::ESNRP_SOLID);

    Reflected_SceneNode::OnRegisterSceneNode();
}

void Reflected_ReferenceImageNode::draw_outline(video::IVideoDriver* driver)
{
    video::SMaterial lineMaterial;
    lineMaterial.Lighting = false;
    lineMaterial.Thickness = 1.0;
    lineMaterial.MaterialType = video::EMT_SOLID;

    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
    driver->setMaterial(lineMaterial);

    video::SColor col = bSelected ? video::SColor(255, 255, 255, 255) : video::SColor(255, 128, 128, 128);

    for (int i = 0; i < 4; i++)
        driver->draw3DLine(Buffer->Vertices[i].Pos, Buffer->Vertices[(i + 1) % 4].Pos, col);
}

void Reflected_ReferenceImageNode::render()
{
    video::IVideoDriver* driver = SceneManager->getVideoDriver();

    if (!driver)
        return;

    Buffer->Material.setTexture(0, texture ? texture->texture : NULL);
    Buffer->Material.MaterialType = bSelected ? Reflected_SceneNode::special_material_type : video::EMT_SOLID;

    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
    driver->setMaterial(Buffer->Material);
    driver->drawMeshBuffer(Buffer);

    //The outline keeps the node visible and selectable-looking when viewed
    //edge-on in the other 2D views, or when no texture is set
    draw_outline(driver);

    //draw_arrow points along the rotated +Z, so pass the rotation that takes +Z onto the normal
    if (bSelected)
        draw_arrow(driver, getPosition(), get_normal().getHorizontalAngle());

    draw_inputs(driver);
}

core::vector3df Reflected_ReferenceImageNode::get_normal() const
{
    core::vector3df n(0, 1, 0);
    core::matrix4 R;
    R.setRotationDegrees(getRotation());
    R.rotateVect(n);
    return n;
}

void Reflected_ReferenceImageNode::render_special(video::SMaterial& material)
{
    video::IVideoDriver* driver = SceneManager->getVideoDriver();

    if (!driver)
        return;

    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

    material.setTexture(0, m_color_tex);
    material.BackfaceCulling = false;
    driver->setMaterial(material);
    driver->drawMeshBuffer(Buffer);
}

const core::aabbox3df& Reflected_ReferenceImageNode::getMyBoundingBox() const
{
    world_box = Buffer->BoundingBox;
    getRelativeTransformation().transformBoxEx(world_box);
    return world_box;
}

void Reflected_ReferenceImageNode::postEdit()
{
    if (bFirstEdit)
    {
        //Just created or just deserialized: the pointer may be a stale address
        //from a previous session, so look the texture up by name.
        texture = texture_name.empty() ? NULL : TexturePicker_Tool::get_texture_info(core::stringw(texture_name.c_str()));
        bFirstEdit = false;
    }

    if (texture)
        texture_name = core::stringc(texture->name).c_str();
    else
        texture_name.clear();

    set_buffer();

    Reflected_SceneNode::postEdit();
    updateAbsolutePosition();
}

bool Reflected_ReferenceImageNode::onActionButton(const std::string& name)
{
    if (name == "ProjectToTerrain")
        return Heightmap_Projection_Window::open(geo_scene, this);

    return false;
}

//Bilinear sample of the image's brightness, 0..1. u,v in [0,1], (0,0) = top-left pixel.
static f32 sample_brightness(video::IImage* img, f32 u, f32 v)
{
    core::dimension2du dim = img->getDimension();

    f32 fx = core::clamp(u, 0.0f, 1.0f) * (dim.Width - 1);
    f32 fy = core::clamp(v, 0.0f, 1.0f) * (dim.Height - 1);

    u32 x0 = (u32)floorf(fx);
    u32 y0 = (u32)floorf(fy);
    u32 x1 = core::min_(x0 + 1, dim.Width - 1);
    u32 y1 = core::min_(y0 + 1, dim.Height - 1);

    f32 tx = fx - x0;
    f32 ty = fy - y0;

    auto px = [&](u32 x, u32 y) -> f32 { return img->getPixel(x, y).getAverage() / 255.0f; };

    f32 top = px(x0, y0) * (1.0f - tx) + px(x1, y0) * tx;
    f32 bottom = px(x0, y1) * (1.0f - tx) + px(x1, y1) * tx;

    return top * (1.0f - ty) + bottom * ty;
}

int Reflected_ReferenceImageNode::project_onto_terrain(GeometryStack* geo_stack, const std::vector<int>& brushes, f32 scale, int* n_outside)
{
    if (n_outside)
        *n_outside = 0;

    if (!geo_stack || !texture || !texture->texture)
    {
        std::cout << "Reference Image: no texture set, nothing to project\n";
        return 0;
    }

    video::IVideoDriver* driver = SceneManager->getVideoDriver();
    video::ITexture* tex = texture->texture;

    //Prefer the source file: exact pixel values, no GPU readback
    video::IImage* img = driver->createImageFromFile(tex->getName().getPath());
    if (!img)
        img = driver->createImage(tex, core::position2di(0, 0), tex->getSize());

    if (!img)
    {
        std::cout << "Reference Image: could not read pixels of " << core::stringc(texture->name).c_str() << "\n";
        return 0;
    }

    //Editor nodes hang off a parent at the origin, so relative == world
    core::matrix4 to_local;
    getRelativeTransformation().getInverse(to_local);

    const f32 base_y = getPosition().Y;
    const f32 eps = 1e-4f;

    int n_moved = 0;
    int n_out = 0;

    for (int b : brushes)
    {
        if (b < 0 || b >= (int)geo_stack->elements.size())
            continue;

        polyfold& pf = geo_stack->elements[b].brush;

        //terrain tile vertices = vertices of any grid canonical brush (see make_terrain_grid)
        std::vector<bool> is_tile_vertex(pf.vertices.size(), false);

        for (const surface_group& sg : pf.surface_groups)
        {
            if (sg.type != SURFACE_GROUP_CANONICAL || sg.c_brush.n_cols == 0)
                continue;

            for (u16 v_i : sg.c_brush.vertices)
                if (v_i < pf.vertices.size())
                    is_tile_vertex[v_i] = true;
        }

        bool moved = false;

        for (int v_i = 0; v_i < (int)pf.vertices.size(); v_i++)
        {
            if (!is_tile_vertex[v_i])
                continue;

            core::vector3df p = pf.vertices[v_i].V;
            to_local.transformVect(p);

            //same mapping as set_buffer: u along +X, v = 0 at the +Z edge
            f32 u = p.X / Size.X + 0.5f;
            f32 v = 0.5f - p.Z / Size.Y;

            if (u < -eps || u > 1.0f + eps || v < -eps || v > 1.0f + eps)
            {
                n_out++;
                continue;
            }

            pf.vertices[v_i].V.Y = base_y + sample_brightness(img, u, v) * scale;
            n_moved++;
            moved = true;
        }

        //same as a vertex drag: the next rebuild re-orients the brush and splits bent tiles
        if (moved)
            pf.topology = TOP_UNDEF;
    }

    img->drop();

    if (n_outside)
        *n_outside = n_out;

    std::cout << "Reference Image: projected " << n_moved << " terrain vertices";
    if (n_out > 0)
        std::cout << " (" << n_out << " outside the image, left unchanged)";
    std::cout << "\n";

    return n_moved;
}

//============================================================================
//  Reflected Water Surface Node
//

REFLECT_STRUCT2_BEGIN(Reflected_WaterSurfaceNode)
    ALIAS("Water Surface")
    REFLECT_STRUCT2_MEMBER(enabled)
    REFLECT_STRUCT2_MEMBER(target)
    REFLECT_STRUCT2_MEMBER(speed)
    REFLECT_STRUCT2_MEMBER(scale)
    INHERIT_FROM(Reflected_Sprite_SceneNode)
REFLECT_STRUCT2_END()

Reflected_WaterSurfaceNode::Reflected_WaterSurfaceNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_Sprite_SceneNode(parent, geo_scene, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("triangle_symbol.png");
    Buffer->Material.setTexture(0, m_texture);
}

Reflected_WaterSurfaceNode::~Reflected_WaterSurfaceNode()
{
    geo_scene->setFinalMeshDirty();
}

bool Reflected_WaterSurfaceNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    if (!enabled)
        return false;

    WaterSurface_SceneNode* node = new WaterSurface_SceneNode(parent, smgr, 77, Location, Rotation, geo_scene);
    node->drop();

    my_node = node;

    //before onSceneInit attaches the buffers, so attach_to_buffer picks these up
    my_node->set_animation(speed, scale);

    
    return true;
}

void Reflected_WaterSurfaceNode::onSceneInit()
{
    if (!enabled)
        return;

    std::vector<Reflected_MeshBuffer_Water_SceneNode*> buffer_nodes;

    resolve_uid_references<Reflected_MeshBuffer_Water_SceneNode>(geo_scene, target, buffer_nodes);

    polyfold* pf = geo_scene->geoNode()->get_total_geometry();

    for (Reflected_MeshBuffer_Water_SceneNode* n : buffer_nodes)
    {
        n->connect_water_sceneNode(my_node);
    }

    geometry_scene* sky_scene = gs_coordinator->skybox_scene();

    if (sky_scene)
    {
        std::vector<u64> uids = sky_scene->get_reflected_node_uids_by_type("Reflected_SkyNode");

        if (uids.size() > 0)
        {
            Reflected_SkyNode* skynode = (Reflected_SkyNode*)sky_scene->get_reflected_node_by_uid(uids[0]);

            if (skynode)
                this->my_node->setSkybox(skynode->underwater_skynode);
        }
    }
}

void Reflected_WaterSurfaceNode::preEdit()
{
    target.old_uids = target.uids;

    Reflected_Sprite_SceneNode::preEdit();
}

void Reflected_WaterSurfaceNode::postEdit()
{
    for (u64 uid : target.old_uids)
    {
        Reflected_SceneNode* node = geo_scene->get_reflected_node_by_uid(uid);

        if (node)
        {
            node->disconnect(this);
            node->postEdit();
        }
    }

    for (u64 uid : target.uids)
    {
        Reflected_SceneNode* node = geo_scene->get_reflected_node_by_uid(uid);

        if (node)
        {
            node->connect_input(this);
            node->postEdit();
        }
    }

    //live update while the final scene is running
    if (my_node)
    {
        my_node->set_animation(speed, scale);

        if (geo_scene->getMeshNode())
            geo_scene->getMeshNode()->copyMaterials();
    }

    Reflected_Sprite_SceneNode::postEdit();
}

void Reflected_WaterSurfaceNode::endScene()
{
    //the WaterSurface_SceneNode is removed with the rest of the final scene
    my_node = NULL;
}

void Reflected_WaterSurfaceNode::connect_sky_sceneNode(MySkybox_SceneNode* node)
{
    my_node->setSkybox(node);
}


void Reflected_MeshBuffer_Water_SceneNode::on_connect_node(Reflected_SceneNode* node)
{
    Reflected_WaterSurfaceNode* water_node = dynamic_cast<Reflected_WaterSurfaceNode*>(node);

    if (water_node)
    {
        this->set_connected(true);
    }
}

void Reflected_MeshBuffer_Water_SceneNode::on_disconnect_node(Reflected_SceneNode* node)
{
    Reflected_WaterSurfaceNode* water_node = dynamic_cast<Reflected_WaterSurfaceNode*>(node);

    if (water_node)
    {
        this->set_connected(false);
    }
}

//===========================================================
// Reflected Skybox Node
//

REFLECT_STRUCT2_BEGIN(Reflected_SkyNode)
    ALIAS("Skybox Node")
    INHERIT_FROM(Reflected_Sprite_SceneNode)
    REFLECT_STRUCT2_MEMBER(enabled)
    REFLECT_STRUCT2_MEMBER(target)
REFLECT_STRUCT2_END()

Reflected_SkyNode::Reflected_SkyNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos) :
    Reflected_Sprite_SceneNode(parent, geo_scene, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("triangle_symbol.png");
    Buffer->Material.setTexture(0, m_texture);

    gs_coordinator->connect_skybox(geo_scene);
}

Reflected_SkyNode::~Reflected_SkyNode()
{
    gs_coordinator->connect_skybox(NULL);
    gs_coordinator->SetAllFinalMeshDirty();
}

bool Reflected_SkyNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    if (!enabled)
        return false;

    regular_skynode = new MySkybox_SceneNode(parent, smgr, -1, Location, Rotation, geo_scene);
    regular_skynode->WaterReflectionPass = false;
    regular_skynode->drop();


    underwater_skynode = new MySkybox_SceneNode(parent, smgr, -1, Location, Rotation, geo_scene);
    underwater_skynode->WaterReflectionPass = true;
    underwater_skynode->drop();

    return true;
}

void Reflected_SkyNode::onSceneInit()
{
    if (!enabled)
        return;

    int c = 0;

    for (geometry_scene* a_geo_scene : gs_coordinator->scenes)
    {
        if (a_geo_scene->IsEditNode())
            continue;

        std::vector<Reflected_MeshBuffer_Sky_SceneNode*> buffer_nodes;

        //TODO: find a better way of doing this
        //resolve_uid_references<Reflected_MeshBuffer_Sky_SceneNode>(a_geo_scene, target, buffer_nodes);
        resolve_match_all_references<Reflected_MeshBuffer_Sky_SceneNode>(a_geo_scene, buffer_nodes);

        if (buffer_nodes.size() > 0)
        {
            RenderSky_SceneNode* renderNode = new RenderSky_SceneNode((USceneNode*)a_geo_scene->ActualNodes(), a_geo_scene->get_smgr(), -1, Location, Rotation, geo_scene);
            renderNode->drop();
            renderNode->set_skynode(regular_skynode);
        }

        for (Reflected_MeshBuffer_Sky_SceneNode* n : buffer_nodes)
        {
            n->connect_sky_sceneNode(regular_skynode, underwater_skynode);
        }

        std::vector<Reflected_WaterSurfaceNode*> waterNodes;

        //resolve_uid_references<Reflected_WaterSurfaceNode>(a_geo_scene, target, waterNodes);
        resolve_match_all_references<Reflected_WaterSurfaceNode>(a_geo_scene, waterNodes);

        for (Reflected_WaterSurfaceNode* n : waterNodes)
        {
            n->connect_sky_sceneNode(underwater_skynode);
        }
    }
}

void Reflected_SkyNode::preEdit()
{
    //target.old_uids = target.uids;

    Reflected_Sprite_SceneNode::preEdit();
}

void Reflected_SkyNode::postEdit()
{

    //gs_coordinator->connect_skybox();

    /*
    for (u64 uid : target.old_uids)
    {
        Reflected_SceneNode* node = geo_scene->get_reflected_node_by_uid(uid);

        if (node)
        {
            node->disconnect(this);
        }
    }

    for (u64 uid : target.uids)
    {
        Reflected_SceneNode* node = geo_scene->get_reflected_node_by_uid(uid);

        if (node)
        {
            node->connect_input(this);
        }
    }
    */

    Reflected_Sprite_SceneNode::postEdit();
}

void Reflected_SkyNode::endScene()
{
    /*
    for (u64 uid : target.uids)
    {
        Reflected_SceneNode* node = geo_scene->get_reflected_node_by_uid(uid);
        Reflected_MeshBuffer_SceneNode* face_node = dynamic_cast<Reflected_MeshBuffer_SceneNode*>(node);

        if (face_node)
        {
            face_node->restore_original_texture();
        }
    }
    */

    //called by caller
    //geoNode()->getMeshNode()->copyMaterials();
}

//==========================================================
// Clouds Meshbuffer Node
//

void CloudsShaderCallBack::OnSetConstants(video::IMaterialRendererServices* services,
    s32 userData)
{
    video::IVideoDriver* driver = services->getVideoDriver();

    core::matrix4 Proj = driver->getTransform(video::ETS_PROJECTION);
    services->setPixelShaderConstant("P", Proj.pointer(), 16);

    core::matrix4 MV = driver->getTransform(video::ETS_VIEW);
    core::matrix4 World = driver->getTransform(video::ETS_WORLD);
    MV *= World;

    services->setPixelShaderConstant("MV", MV.pointer(), 16);

    scene::ICameraSceneNode* cam = device->getSceneManager()->getActiveCamera();
    core::vector3df pos = device->getSceneManager()->getActiveCamera()->getAbsolutePosition();

    services->setVertexShaderConstant("mCamPos", reinterpret_cast<f32*>(&pos), 3);
    services->setPixelShaderConstant("Time", reinterpret_cast<f32*>(&g_time), 1);

    s32 TextureLayerID = 0;
    services->setPixelShaderConstant("myTexture", &TextureLayerID, 1);

    TextureLayerID = 1;
    services->setPixelShaderConstant("myTexture2", &TextureLayerID, 1);

    TextureLayerID = 2;
    services->setPixelShaderConstant("myTexture3", &TextureLayerID, 1);

}

Reflected_MeshBuffer_Clouds_SceneNode::Reflected_MeshBuffer_Clouds_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos)
    :Reflected_MeshBuffer_SceneNode(parent, geo_scene, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("color_square_icon_good.png");
    Buffer->Material.setTexture(0, m_texture);
}

Reflected_MeshBuffer_Clouds_SceneNode::~Reflected_MeshBuffer_Clouds_SceneNode()
{

}

bool Reflected_MeshBuffer_Clouds_SceneNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    return false;
}

void Reflected_MeshBuffer_Clouds_SceneNode::onSceneInit()
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

            buffer->getMaterial().setTexture(0, CloudsShaderCallBack::texture0);
            buffer->getMaterial().setTexture(1, CloudsShaderCallBack::texture1);
            buffer->getMaterial().setTexture(2, CloudsShaderCallBack::texture2);

            Material_Groups_Base* material_groups_base = Material_Groups_Tool::get_base();

            E_MATERIAL_TYPE cloud_material = (E_MATERIAL_TYPE)material_groups_base->Material_Clouds_Type;
            buffer->getMaterial().MaterialType = cloud_material;
        }
    }
}

REFLECT_STRUCT2_BEGIN(Reflected_MeshBuffer_Clouds_SceneNode)
    ALIAS("Clouds Mesh Buffer")
    PLACEABLE(false)
    INHERIT_FROM(Reflected_MeshBuffer_SceneNode)
REFLECT_STRUCT2_END()

//==========================================================
// Area Light Meshbuffer Node
//

Reflected_MeshBuffer_AreaLight_SceneNode::Reflected_MeshBuffer_AreaLight_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos)
    :Reflected_MeshBuffer_SceneNode(parent, geo_scene, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("color_square_icon_good.png");
}

Reflected_MeshBuffer_AreaLight_SceneNode::~Reflected_MeshBuffer_AreaLight_SceneNode()
{

}

bool Reflected_MeshBuffer_AreaLight_SceneNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    return false;
}

void Reflected_MeshBuffer_AreaLight_SceneNode::onSceneInit()
{
 
}

REFLECT_STRUCT2_BEGIN(Reflected_MeshBuffer_AreaLight_SceneNode)
    ALIAS("Area Light")
    PLACEABLE(false)
    REFLECT_STRUCT2_MEMBER(intensity)
    INHERIT_FROM(Reflected_MeshBuffer_SceneNode)
REFLECT_STRUCT2_END()

//==========================================================
// Animated Texture Meshbuffer Node
//

Reflected_MeshBuffer_AnimatedTexture_SceneNode::Reflected_MeshBuffer_AnimatedTexture_SceneNode(USceneNode* parent, geometry_scene* geo_scene, irr::scene::ISceneManager* smgr, int id, const core::vector3df& pos)
    :Reflected_MeshBuffer_SceneNode(parent, geo_scene, smgr, id, pos)
{
    m_texture = device->getVideoDriver()->getTexture("color_square_icon_good.png");
}

Reflected_MeshBuffer_AnimatedTexture_SceneNode::~Reflected_MeshBuffer_AnimatedTexture_SceneNode()
{

}

bool Reflected_MeshBuffer_AnimatedTexture_SceneNode::addSelfToScene(USceneNode* parent, irr::scene::ISceneManager* smgr, geometry_scene* geo_scene)
{
    return false;
}

void Reflected_MeshBuffer_AnimatedTexture_SceneNode::onSceneInit()
{
    polyfold* pf = geo_scene->geoNode()->get_total_geometry();

    for (int f_i = 0; f_i < pf->faces.size(); f_i++)
    {
        if (pf->faces[f_i].element_id == this->element_id &&
            pf->faces[f_i].face_id == this->face_id)
        {
            MeshBuffer_Chunk chunk = geo_scene->geoNode()->final_meshnode_interface.get_mesh_buffer_by_face(f_i);

            IMeshBuffer* buffer = chunk.buffer;

            buffer->getMaterial().setTexture(2, AnimatedLightmapShaderCallback::texture0);

            Material_Groups_Base* material_groups_base = Material_Groups_Tool::get_base();

            E_MATERIAL_TYPE animated_material;

            if (material_groups_base->lighting_mode == LIGHTING_LIGHT_ONLY)
                animated_material = (E_MATERIAL_TYPE)material_groups_base->Material_AnimatedLightOnly_Type;
            else
                animated_material = (E_MATERIAL_TYPE)material_groups_base->Material_AnimatedLightmap_Type;

            buffer->getMaterial().MaterialType = animated_material;

            set_shader_params(buffer);
        }
    }
}

void Reflected_MeshBuffer_AnimatedTexture_SceneNode::set_shader_params(IMeshBuffer* buffer)
{
    //read back by AnimatedLightmapShaderCallback::OnSetMaterial
    buffer->getMaterial().MaterialTypeParam = speed;
    buffer->getMaterial().MaterialTypeParam2 = scale;
    buffer->getMaterial().Shininess = core::clamp(ambient, 0.0f, 1.0f);
}

void Reflected_MeshBuffer_AnimatedTexture_SceneNode::postEdit()
{
    IMeshBuffer* buffer = get_mesh_buffer();

    if (buffer)
    {
        set_shader_params(buffer);

        if (geo_scene->getMeshNode())
            geo_scene->getMeshNode()->copyMaterials();
    }

    Reflected_MeshBuffer_SceneNode::postEdit();
}

REFLECT_STRUCT2_BEGIN(Reflected_MeshBuffer_AnimatedTexture_SceneNode)
    ALIAS("Animated Texture")
    PLACEABLE(false)
    REFLECT_STRUCT2_MEMBER(speed)
    REFLECT_STRUCT2_MEMBER(scale)
    REFLECT_STRUCT2_MEMBER(ambient)
    INHERIT_FROM(Reflected_MeshBuffer_SceneNode)
REFLECT_STRUCT2_END()