
#include <irrlicht.h>
#include <iostream>
#include <fstream>
#include "csg_classes.h"
#include "utils.h"
#include "geometry_scene.h"
#include "edit_env.h"
#include "reflected_nodes.h"
#include "Reflection.h"
#include "CPLYMeshWriter.h"
#include "file_open.h"
#include "CameraPanel.h"
#include "initialization.h"
#include <random>
#include <sstream>

extern IrrlichtDevice* device;

using namespace irr;
using namespace std;

extern irr::video::ITexture* small_circle_tex_add_selected = NULL;
extern irr::video::ITexture* small_circle_tex_add_not_selected = NULL;
extern irr::video::ITexture* small_circle_tex_sub_selected = NULL;
extern irr::video::ITexture* small_circle_tex_sub_not_selected = NULL;
extern irr::video::ITexture* small_circle_tex_red_selected = NULL;
extern irr::video::ITexture* small_circle_tex_red_not_selected = NULL;

extern irr::video::ITexture* med_circle_tex_add_selected = NULL;
extern irr::video::ITexture* med_circle_tex_add_not_selected = NULL;
extern irr::video::ITexture* med_circle_tex_sub_selected = NULL;
extern irr::video::ITexture* med_circle_tex_sub_not_selected = NULL;
extern irr::video::ITexture* med_circle_tex_red_selected = NULL;
extern irr::video::ITexture* med_circle_tex_red_not_selected = NULL;

const int bytesPerPixel = 4; /// red, green, blue
const int fileHeaderSize = 14;
const int infoHeaderSize = 40;

unsigned char* createBitmapFileHeader(int height, int width);
unsigned char* createBitmapInfoHeader(int height, int width);

void generateBitmapImage(unsigned char* image, int height, int width, const char* imageFileName) {

    unsigned char* fileHeader = createBitmapFileHeader(height, width);
    unsigned char* infoHeader = createBitmapInfoHeader(height, width);
    unsigned char padding[3] = { 0, 0, 0 };
    int paddingSize = (4 - (width * bytesPerPixel) % 4) % 4;

    FILE* imageFile = fopen(imageFileName, "wb");

    fwrite(fileHeader, 1, fileHeaderSize, imageFile);
    fwrite(infoHeader, 1, infoHeaderSize, imageFile);

    int i;
    for (i = 0; i < height; i++) {
        fwrite(image + (i * width * bytesPerPixel), bytesPerPixel, width, imageFile);
        fwrite(padding, 1, paddingSize, imageFile);
    }

    fclose(imageFile);
}

unsigned char* createBitmapFileHeader(int height, int width) {
    int fileSize = fileHeaderSize + infoHeaderSize + bytesPerPixel * height * width;

    static unsigned char fileHeader[] = {
        0,0, /// signature
        0,0,0,0, /// image file size in bytes
        0,0,0,0, /// reserved
        0,0,0,0, /// start of pixel array
    };

    fileHeader[0] = (unsigned char)('B');
    fileHeader[1] = (unsigned char)('M');
    fileHeader[2] = (unsigned char)(fileSize);
    fileHeader[3] = (unsigned char)(fileSize >> 8);
    fileHeader[4] = (unsigned char)(fileSize >> 16);
    fileHeader[5] = (unsigned char)(fileSize >> 24);
    fileHeader[10] = (unsigned char)(fileHeaderSize + infoHeaderSize);

    return fileHeader;
}


unsigned char* createBitmapInfoHeader(int height, int width) {
    static unsigned char infoHeader[] = {
        0,0,0,0, /// header size
        0,0,0,0, /// image width
        0,0,0,0, /// image height
        0,0, /// number of color planes
        0,0, /// bits per pixel
        0,0,0,0, /// compression
        0,0,0,0, /// image size
        0,0,0,0, /// horizontal resolution
        0,0,0,0, /// vertical resolution
        0,0,0,0, /// colors in color table
        0,0,0,0, /// important color count
    };

    infoHeader[0] = (unsigned char)(infoHeaderSize);
    infoHeader[4] = (unsigned char)(width);
    infoHeader[5] = (unsigned char)(width >> 8);
    infoHeader[6] = (unsigned char)(width >> 16);
    infoHeader[7] = (unsigned char)(width >> 24);
    infoHeader[8] = (unsigned char)(height);
    infoHeader[9] = (unsigned char)(height >> 8);
    infoHeader[10] = (unsigned char)(height >> 16);
    infoHeader[11] = (unsigned char)(height >> 24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(bytesPerPixel * 8);

    return infoHeader;
}


video::SColor makeUniqueColor()
{
    static int i=0;
    i++;
    return video::SColor(255,i,255,255);
}

irr::video::IImage* makeSolidColorImage(video::IVideoDriver* driver, video::SColor col)
{
    int width = 8;

    video::IImage* img = driver->createImage(irr::video::ECF_A8R8G8B8,core::dimension2d<u32>(width,width));

    for(int i=0;i<width;i++)
        for(int j=0;j<width;j++)
        {
             img->setPixel(i,j,col);
        }

    return img;
}


irr::video::IImage* makeAlphaImage(video::IVideoDriver* driver,video::ITexture* texture, int alpha)
{
    video::IImage* img = driver->createImage(texture,core::vector2di(0,0),texture->getSize());
     for(int i=0;i<texture->getSize().Width;i++)
        for(int j=0;j<texture->getSize().Height;j++)
            {
                video::SColor color = img->getPixel(i,j);
                color.setAlpha(alpha);
                img->setPixel(i,j,color);
            }
    return img;
}


irr::video::IImage* makeCircleImage(video::IVideoDriver* driver,int width, f32 radius, video::SColor col)
{

    video::IImage*     img = driver->createImage(irr::video::ECF_A8R8G8B8,core::dimension2d<u32>(width,width));

    srand(45325);
    video::SColor color=col;
    int r;

        for(int i=0;i<width;i++)
            for(int j=0;j<width;j++)
                {
                        f32 d = sqrt((((f32)width * 0.5)-j-0.5)*(((f32)width * 0.5)-j-0.5)+
                                (((f32)width * 0.5)-i-0.5)*(((f32)width * 0.5)-i-0.5));

                        if(d<radius)
                            color.setAlpha(255);
                        else if(d<radius+0.5)
                            color.setAlpha(128);
                        else if(d<radius+0.1)
                            color.setAlpha(64);
                        else
                            color.setAlpha(0);

                        img->setPixel(i,j,color);
                }
    return img;
}


void MakeCircleImages(video::IVideoDriver* driver)
{
    irr::video::IImage* cimg = makeCircleImage(driver,8,1.5,COLOR_ADD_NOT_SELECTED);
    small_circle_tex_add_not_selected = driver->addTexture(irr::io::path("a circle 0"), cimg);
    cimg->drop();

    cimg = makeCircleImage(driver,8,1.5,COLOR_ADD_SELECTED);
    small_circle_tex_add_selected = driver->addTexture(irr::io::path("a circle 1"), cimg);
    cimg->drop();

    cimg = makeCircleImage(driver,8,1.5,COLOR_SUB_NOT_SELECTED);
    small_circle_tex_sub_not_selected = driver->addTexture(irr::io::path("a circle 2"), cimg);
    cimg->drop();

    cimg = makeCircleImage(driver,8,1.5,COLOR_SUB_SELECTED);
    small_circle_tex_sub_selected = driver->addTexture(irr::io::path("a circle 3"), cimg);
    cimg->drop();

    cimg = makeCircleImage(driver,8,1.5,COLOR_RED_NOT_SELECTED);
    small_circle_tex_red_not_selected = driver->addTexture(irr::io::path("a circle 2"), cimg);
    cimg->drop();

    cimg = makeCircleImage(driver,8,1.5,COLOR_RED_SELECTED);
    small_circle_tex_red_selected = driver->addTexture(irr::io::path("a circle 3"), cimg);
    cimg->drop();

    //==============

    cimg = makeCircleImage(driver,8,2.7,COLOR_ADD_NOT_SELECTED);
    med_circle_tex_add_not_selected = driver->addTexture(irr::io::path("a circle 0"), cimg);
    cimg->drop();

    cimg = makeCircleImage(driver,8,2.7,COLOR_ADD_SELECTED);
    med_circle_tex_add_selected = driver->addTexture(irr::io::path("a circle 1"), cimg);
    cimg->drop();

    cimg = makeCircleImage(driver,8,2.7,COLOR_SUB_NOT_SELECTED);
    med_circle_tex_sub_not_selected = driver->addTexture(irr::io::path("a circle 2"), cimg);
    cimg->drop();

    cimg = makeCircleImage(driver,8,2.7,COLOR_SUB_SELECTED);
    med_circle_tex_sub_selected = driver->addTexture(irr::io::path("a circle 3"), cimg);
    cimg->drop();

    cimg = makeCircleImage(driver,8,2.7,COLOR_RED_NOT_SELECTED);
    med_circle_tex_red_not_selected = driver->addTexture(irr::io::path("a circle 2"), cimg);
    cimg->drop();

    cimg = makeCircleImage(driver,8,2.7,COLOR_RED_SELECTED);
    med_circle_tex_red_selected = driver->addTexture(irr::io::path("a circle 3"), cimg);
    cimg->drop();
}

#define MAT_COUT(M) std::cout<<M[0]<<" "<<M[1]<<" "<<M[2]<<" "<<M[3]<<"\n"; \
					std::cout<<M[4]<<" "<<M[5]<<" "<<M[6]<<" "<<M[7]<<"\n"; \
					std::cout<<M[8]<<" "<<M[9]<<" "<<M[10]<<" "<<M[11]<<"\n"; \
					std::cout<<M[12]<<" "<<M[13]<<" "<<M[14]<<" "<<M[15]<<"\n"; 

REFLECT_STRUCT_BEGIN(Vertex_Struct)
    REFLECT_STRUCT_MEMBER(pos)
    REFLECT_STRUCT_MEMBER(tex_coords_0)
    REFLECT_STRUCT_MEMBER(tex_coords_1)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(Vertex_Buffer_Struct)
    REFLECT_STRUCT_MEMBER(vertices)
    REFLECT_STRUCT_MEMBER(indices)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(Face_Bounding_Rect_Struct)
    REFLECT_STRUCT_MEMBER(v0)
    REFLECT_STRUCT_MEMBER(v1)
    REFLECT_STRUCT_MEMBER(v2)
    REFLECT_STRUCT_MEMBER(v3)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(Face_Info_Struct)
    REFLECT_STRUCT_MEMBER(normal)
    REFLECT_STRUCT_MEMBER(tangent)
    REFLECT_STRUCT_MEMBER(bounding_rect)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(LightMaps_Info_Struct)
    REFLECT_STRUCT_MEMBER(size)
    REFLECT_STRUCT_MEMBER(faces)
    REFLECT_STRUCT_MEMBER(lightmap_block_UL)
    REFLECT_STRUCT_MEMBER(lightmap_block_BR)
REFLECT_STRUCT_END()


REFLECT_STRUCT_BEGIN(Model_Struct)
    REFLECT_STRUCT_MEMBER(vertex_buffers)
    REFLECT_STRUCT_MEMBER(faces_info)
    REFLECT_STRUCT_MEMBER(lightmaps_info)
REFLECT_STRUCT_END()


u64 random_number()
{
    static random_device dev;
    static mt19937 rng(dev());

    std::uniform_int_distribution<u64> dist(0, std::numeric_limits<u64>::max());

    return dist(rng);
}

void save_gui_state(GUI_state_struct& state_struct)
{
    gui::IGUIEnvironment* env = device->getGUIEnvironment();
    gui::IGUIElement* root = env->getRootGUIElement();
    CameraQuad* quad = (CameraQuad*)root->getElementFromId(GUI_ID_CAMERA_QUAD, true);

    if (!quad) return;

    //Top Left
    TestPanel* panel_TL = quad->getPanel(0);

    state_struct.dynamicLight = panel_TL->IsDynamicLight();
    state_struct.viewStyle = ((TestPanel_3D*)panel_TL)->GetViewStyle();

    ICameraSceneNode* camera = panel_TL->getCamera();
    state_struct.tl.my_camera.orthogonal = false;
    state_struct.tl.my_camera.position = camera->getAbsolutePosition();
    state_struct.tl.my_camera.target = camera->getTarget();
    state_struct.tl.my_camera.projM = camera->getProjectionMatrix();

    //Top Right
    TestPanel_2D* panel_TR = (TestPanel_2D*)quad->getPanel(1);
    state_struct.tr.viewSize = panel_TR->getViewSize();

    camera = panel_TR->getCamera();
    state_struct.tr.my_camera.orthogonal = true;
    state_struct.tr.my_camera.position = camera->getAbsolutePosition();
    state_struct.tr.my_camera.target = camera->getTarget();
    state_struct.tr.my_camera.projM = camera->getProjectionMatrix();

    //Bottom Left
    TestPanel_2D* panel_BL = (TestPanel_2D*)quad->getPanel(2);
    state_struct.bl.viewSize = panel_BL->getViewSize();

    camera = panel_BL->getCamera();
    state_struct.bl.my_camera.orthogonal = true;
    state_struct.bl.my_camera.position = camera->getAbsolutePosition();
    state_struct.bl.my_camera.target = camera->getTarget();
    state_struct.bl.my_camera.projM = camera->getProjectionMatrix();

    //Bottom Right
    TestPanel_2D* panel_BR = (TestPanel_2D*)quad->getPanel(3);
    state_struct.br.viewSize = panel_BR->getViewSize();

    camera = panel_BR->getCamera();
    state_struct.br.my_camera.orthogonal = true;
    state_struct.br.my_camera.position = camera->getAbsolutePosition();
    state_struct.br.my_camera.target = camera->getTarget();
    state_struct.br.my_camera.projM = camera->getProjectionMatrix();

    state_struct.is_valid_state = true;
}

void restore_gui_state(const GUI_state_struct& state_struct)
{
    gui::IGUIEnvironment* env = device->getGUIEnvironment();
    gui::IGUIElement* root = env->getRootGUIElement();
    CameraQuad* quad = (CameraQuad*)root->getElementFromId(GUI_ID_CAMERA_QUAD, true);

    if (!quad || !state_struct.is_valid_state) return;

    //Top Left
    ICameraSceneNode* camera = quad->getPanel(0)->getCamera();

    camera->setPosition(state_struct.tl.my_camera.position);
    camera->setTarget(state_struct.tl.my_camera.target);
    camera->setProjectionMatrix(state_struct.tl.my_camera.projM, false);
    camera->updateAbsolutePosition();

    ((TestPanel_3D*)quad->getPanel(0))->SetDynamicLight(state_struct.dynamicLight);

    if(state_struct.viewStyle == PANEL3D_VIEW_RENDER_FINAL)
        ((TestPanel_3D*)quad->getPanel(0))->SetViewStyle(PANEL3D_VIEW_RENDER);
    else
        ((TestPanel_3D*)quad->getPanel(0))->SetViewStyle(state_struct.viewStyle);

    //Top Right
    camera = quad->getPanel(1)->getCamera();
    ((TestPanel_2D*)quad->getPanel(1))->setViewSize(state_struct.tr.viewSize);

    camera->setPosition(state_struct.tr.my_camera.position);
    camera->setTarget(state_struct.tr.my_camera.target);
    camera->setProjectionMatrix(state_struct.tr.my_camera.projM, true);
    camera->updateAbsolutePosition();

    //Bottom Left
    camera = quad->getPanel(2)->getCamera();
    ((TestPanel_2D*)quad->getPanel(2))->setViewSize(state_struct.bl.viewSize);

    camera->setPosition(state_struct.bl.my_camera.position);
    camera->setTarget(state_struct.bl.my_camera.target);
    camera->setProjectionMatrix(state_struct.bl.my_camera.projM, true);
    camera->updateAbsolutePosition();

    //Bottom Right
    camera = quad->getPanel(3)->getCamera();
    ((TestPanel_2D*)quad->getPanel(3))->setViewSize(state_struct.br.viewSize);

    camera->setPosition(state_struct.br.my_camera.position);
    camera->setTarget(state_struct.br.my_camera.target);
    camera->setProjectionMatrix(state_struct.br.my_camera.projM, true);
    camera->updateAbsolutePosition();
}

//=======================================================
//  Open Geometry File
//

Open_Geometry_File::Open_Geometry_File(SceneCoordinator* sc)
{
    scene_coordinator = sc;

    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->Register(this);

    FileSystem = device->getFileSystem();

    File_Open_Tool::show("Open Project");
}

Open_Geometry_File::~Open_Geometry_File()
{
    //std::cout << "Open Geometry File... going out of scope\n";
}

bool Open_Geometry_File::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_USER_EVENT)
    {
        if (event.UserEvent.UserData1 == USER_EVENT_DIRECTORY_SELECTED)
        {
            io::path restore_path = FileSystem->getWorkingDirectory();
            io::path p = File_Open_Tool::getSelectedDir();

            FileSystem->changeWorkingDirectoryTo(p);

            Geometry_Scene_File_IO file_io(FileSystem);
            file_io.ReadFromFiles(scene_coordinator);

            FileSystem->changeWorkingDirectoryTo(restore_path);
            return true;
        }
        else if (event.UserEvent.UserData1 == USER_EVENT_FILE_DIALOGUE_CLOSED)
        {

            MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
            receiver->UnRegisterFromInsideEvent(this);

            delete this;

            return true;
        }
    }
    return false;
}
//=======================================================
//  Save Geometry File
//

Save_Geometry_File::Save_Geometry_File(SceneCoordinator* sc, bool show)
{
    scene_coordinator = sc;

    FileSystem = device->getFileSystem();

    if (show)
    {
        MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
        receiver->Register(this);

        File_Open_Tool::show("Save Project");
    }

}

Save_Geometry_File::~Save_Geometry_File()
{
    //std::cout << "Save Geometry File... going out of scope\n";
}

bool Save_Geometry_File::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_USER_EVENT)
    {
        if (event.UserEvent.UserData1 == USER_EVENT_DIRECTORY_SELECTED)
        {
            io::path restore_path = FileSystem->getWorkingDirectory();
            io::path p = File_Open_Tool::getSelectedDir();
            std::cout << " >>>>> writing files to " << p.c_str() << "\n";

            FileSystem->changeWorkingDirectoryTo(p);

            Geometry_Scene_File_IO file_io(FileSystem);
            file_io.WriteToFiles(scene_coordinator);

            FileSystem->changeWorkingDirectoryTo(restore_path);
            return true;
        }
        else if (event.UserEvent.UserData1 == USER_EVENT_FILE_DIALOGUE_CLOSED)
        {

            MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
            receiver->UnRegisterFromInsideEvent(this);

            delete this;

            return true;
        }
    }
    return false;
}

REFLECT_STRUCT_BEGIN(camera_info_struct)
    REFLECT_STRUCT_MEMBER(orthogonal)
    REFLECT_STRUCT_MEMBER(position)
    REFLECT_STRUCT_MEMBER(target)
    REFLECT_STRUCT_MEMBER(projM)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(camera_panel_3D_info_struct)
    REFLECT_STRUCT_MEMBER(my_camera)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(camera_panel_2D_info_struct)
    REFLECT_STRUCT_MEMBER(my_camera)
    REFLECT_STRUCT_MEMBER(axis)
    REFLECT_STRUCT_MEMBER(viewSize)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(GUI_state_struct)
    REFLECT_STRUCT_MEMBER(tl)
    REFLECT_STRUCT_MEMBER(tr)
    REFLECT_STRUCT_MEMBER(bl)
    REFLECT_STRUCT_MEMBER(br)
    REFLECT_STRUCT_MEMBER(dynamicLight)
    REFLECT_STRUCT_MEMBER(viewStyle)
    REFLECT_STRUCT_MEMBER(is_valid_state)
REFLECT_STRUCT_END()
