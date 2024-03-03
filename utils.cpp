
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
#include "LightMaps.h"

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

void addDrawLines(polyfold& pf, LineHolder& graph,LineHolder& graph2,LineHolder& graph3)
{
    //if(false)
    for(int f_i=0; f_i < pf.faces.size(); f_i++)
        {
           // if(f_i==0 )//|| f_i==2)
           //if(f_i==29)
            //if (f_i == 25 || f_i == 26)
            {
                //std::cout << f_i << " has " << pf.faces[f_i].vertices.size() << " v\n";
                for (int p_i = 0; p_i < pf.faces[f_i].loops.size(); p_i++)
                {
                    //std::cout << f_i << " and " << p_i << "\n";
                    //std::cout<<f_i<<", "<<p_i<<": "<<pf.faces[f_i].loops[p_i].vertices.size()<<" verts\n";
                    //if(p_i==0)
                    //if(pf.faces[f_i].loops[p_i].vertices.size()>0 && pf.faces[f_i].loops[p_i].type == LOOP_INNER)

                    //if(p_i==1)
                    if (pf.faces[f_i].loops[p_i].vertices.size() > 0)//&& pf.faces[f_i].loops[p_i].type == LOOP_INNER)
                    {

                        std::vector<int> tempv = pf.faces[f_i].loops[p_i].vertices;
                        tempv.push_back(tempv[0]);

                        //std::cout << f_i << " / " << p_i << ": " << pf.faces[f_i].loops[p_i].depth << "\n";

                        //if(pf.faces[f_i].loops[p_i].type==LOOP_INNER)
                        for (int i = 0; i < tempv.size() - 1; i++)
                        {

                            //std::cout<<tempv[i]<<" "<<tempv[i+1]<<", ";
                            core::vector3df v0 = pf.vertices[tempv[i]].V;
                            core::vector3df v1 = pf.vertices[tempv[i + 1]].V;

                            if(pf.is_closed_loop(f_i,p_i))
                               // if (pf.faces[f_i].bFlippedNormal)

                            //if (pf.faces[f_i].loops[p_i].depth % 2 == 0)
                                //if(pf.faces[f_i].loops[p_i].topo_group==LOOP_UNDEF)
                                // if(pf.faces[f_i].loops[p_i].topo_group==LOOP_GHOST_SOLID)
                                //if(pf.faces[f_i].loops[p_i].type==LOOP_INNER)
                            {
                                graph.lines.push_back(core::line3df(v0, v1));
                            }
                            //else if(pf.faces[f_i],pf.faces[f_i].loops[p_i].topo_group==LOOP_UNDEF)
                            //else if(pf.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                            else //if(pf.faces[f_i].loops[p_i].topo_group==LOOP_GHOST_SOLID)
                            {
                                //graph.points.push_back(v0);
                                graph2.lines.push_back(core::line3df(v0, v1));
                            }
                            //else
                            {
                                // graph3.lines.push_back(core::line3df(v0,v1));
                            }
                        }
                    }
                }
            }
        }
    //std::cout<<"huh?\n";

    if(false)
    for(int e_i=0; e_i < pf.edges.size(); e_i++)
    {
        core::vector3df v0=pf.getVertex(e_i,0).V;
        core::vector3df v1=pf.getVertex(e_i,1).V;
        //if(e_i==28)
        {
            //if(pf.edges[e_i].p2== 1 || pf.edges[e_i].p2== 2)
            //if(pf.edges[e_i].topo_group==3 )// || pf.edges[e_i].topo_group==3)
            {
                graph.lines.push_back(core::line3df(v0,v1));
            }
        }
    }

    if(false)
    for(int e_i=0; e_i < pf.edges.size(); e_i++)
    {
        core::vector3df v0=pf.getVertex(e_i,0).V;
        core::vector3df v1=pf.getVertex(e_i,1).V;
        //if(e_i==48 || e_i== 49)// || e_i==41)
        {
            //std::cout<<pf.edges[33].topo_group<<" topo\n";
            //if(true)
            if(pf.edges[e_i].topo_group==3 )// || pf.edges[e_i].topo_group==3)
            {
                graph.lines.push_back(core::line3df(v0,v1));
                //std::cout<<"edge "<<e_i<< "("<<my_poly.edges[e_i].topo_group<<") :   "<<v0.X<<","<<v0.Y<<","<<v0.Z<<" to "<<v1.X<<","<<v1.Y<<","<<v1.Z<<"\n";
            }
            else if(pf.edges[e_i].topo_group==0)// || pf.edges[e_i].topo_group==3)
            {
                graph2.lines.push_back(core::line3df(v0,v1));
            }
            else if(pf.edges[e_i].topo_group==1)// || pf.edges[e_i].topo_group==3)
            {
                graph2.lines.push_back(core::line3df(v0,v1));
            }
        }
    }


   //std::cout<<pf.vertices.size()<<"\n";
    if(false)
    for(int v_i=0;v_i<pf.vertices.size();v_i++)
        {
            core::vector3df v0 = pf.vertices[v_i].V;
            graph.points.push_back(v0);
         //   std::cout<<v_i<<":  "<<v0.X<<","<<v0.Y<<","<<v0.Z<<"\n";
        }

    if(false)
    for(int f_i=0; f_i < pf.faces.size(); f_i++)
    {
        //core::vector3df v0 =pf.faces[f_i].m_normal;
        //std::cout<<f_i<<": "<<v0.X<<","<<v0.Y<<","<<v0.Z<<"\n";
    }

    if(false)
    for(int f_i=0; f_i < pf.faces.size(); f_i++)
    {
        ///if(f_i==5)
        {

            for(int v_i : pf.faces[f_i].vertices)
            {
                graph.points.push_back(pf.vertices[v_i].V);
            }
            //std::cout<<"n edges = "<<pf.faces[f_i].edges.size()<<"\n";
            for(int i=0;i<pf.faces[f_i].edges.size();i++)
            {
                //if(i==0)
                {
                    int e_i = pf.faces[f_i].edges[i];
                   // std::cout<<i<<" "<<e_i<<" ("<<pf.edges[e_i].topo_group<<") : "<<pf.edges[e_i].v0<<" "<<pf.edges[e_i].v1<<"\n";
                    //if(e_i==13)
                   {
                    core::vector3df v0=pf.getVertex(e_i,0).V;
                    core::vector3df v1=pf.getVertex(e_i,1).V;
                    //std::cout<<pf.edges[e_i].v0<<" = "<<v0.X<<","<<v0.Y<<","<<v0.Z<<"\n";
                    //std::cout<<pf.edges[e_i].v1<<" = "<<v1.X<<","<<v1.Y<<","<<v1.Z<<"\n";
                    //graph.points.push_back(v0);
                    //graph.points.push_back(v1);
                    //if(true)
                    if(pf.edges[e_i].topo_group==3 )//|| pf.edges[e_i].topo_group==3)// || pf.edges[e_i].topo_group==3)
                    {
                        graph.lines.push_back(core::line3df(v0,v1));
                        //std::cout<<"edge "<<e_i<< "("<<my_poly.edges[e_i].topo_group<<") :   "<<v0.X<<","<<v0.Y<<","<<v0.Z<<" to "<<v1.X<<","<<v1.Y<<","<<v1.Z<<"\n";
                    }
                    else if(pf.edges[e_i].topo_group==0)// || pf.edges[e_i].topo_group==3)
                    {
                        graph2.lines.push_back(core::line3df(v0,v1));
                    }
                    else if(pf.edges[e_i].topo_group==1)// || pf.edges[e_i].topo_group==3)
                    {
                        graph3.lines.push_back(core::line3df(v0,v1));
                    }
                   }
                }
            }
        }
       //std::cout<<" a \n";
    }

    if(false)
    for(int f_i=0; f_i < pf.faces.size(); f_i++)
    {
        core::vector3df w0 = pf.faces[f_i].m_center;
        core::vector3df w1 = w0 +  pf.faces[f_i].m_normal*10;
        graph2.lines.push_back(core::line3df(w0,w1));

        for(int i=0;i<pf.faces[f_i].edges.size();i++)
        {
                int e_i = pf.faces[f_i].edges[i];
                core::vector3df v0=pf.getVertex(e_i,0).V;
                core::vector3df v1=pf.getVertex(e_i,1).V;
                graph.lines.push_back(core::line3df(v0,v1));
        }
    }



    if(false)
    for(int e_i=0; e_i < pf.edges.size(); e_i++)
    {
        core::vector3df v0=pf.getVertex(e_i,0).V;
        core::vector3df v1=pf.getVertex(e_i,1).V;
        if(e_i%2 )
        {
            graph.lines.push_back(core::line3df(v0,v1));
        }
        else
            graph2.lines.push_back(core::line3df(v0,v1));
    }

    LineHolder nograph;
    //pf.classify_edges(nograph);

    if(false)
    for(int e_i=0; e_i < pf.edges.size(); e_i++)
    {
        core::vector3df v0=pf.getVertex(e_i,0).V;
        core::vector3df v1=pf.getVertex(e_i,1).V;
        //if(e_i == 16)
            {
           // std::cout<<e_i<<" edge!\n";
            //if(pf.edges[e_i].topo_group==-1)// || pf.edges[e_i].topo_group==3)
            if(pf.edges[e_i].topo_group == -1) continue;

            //if(pf.classify_edge2(e_i)== TOP_CONVEX)
            if(pf.edges[e_i].conv == EDGE_CONVEX )//|| pf.edges[e_i].conv == EDGE_CONCAVE)
            //if(true)
            {
                graph2.lines.push_back(core::line3df(v0,v1));
               // std::cout<<"edge "<<e_i<< "("<<pf.edges[e_i].topo_group<<") :   "<<v0.X<<","<<v0.Y<<","<<v0.Z<<" to "<<v1.X<<","<<v1.Y<<","<<v1.Z<<"\n";
            }
            //else if(pf.edges[e_i].topo_group==0 )// || pf.edges[e_i].topo_group==0)
            //else  if(pf.classify_edge(e_i)== TOP_CONCAVE)
            else if(pf.edges[e_i].conv == EDGE_SIMPLE)
            {
                graph3.lines.push_back(core::line3df(v0,v1));
                //std::cout<<"edge "<<e_i<< "("<<pf.edges[e_i].topo_group<<") :   "<<v0.X<<","<<v0.Y<<","<<v0.Z<<" to "<<v1.X<<","<<v1.Y<<","<<v1.Z<<"\n";
                //std::cout<<pf.find_edge(pf.getVertex(e_i,1).V,pf.getVertex(e_i,0).V)<<"\n";
            }
            else if(pf.edges[e_i].conv == EDGE_COMPLEX)
            {
                //std::cout<<"could not classify edge "<<e_i<<"\n";
                graph.lines.push_back(core::line3df(v0,v1));
            }
           // std::cout<<"next...\n";
        }
    }
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
    //std::cout<<texture->getSize().Width<<","<<texture->getSize().Height<<"\n";
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


    //std::cout<<"Generating Static Texture \n";

    video::IImage*     img = driver->createImage(irr::video::ECF_A8R8G8B8,core::dimension2d<u32>(width,width));

    srand(45325);
    video::SColor color=col;
    int r;

        for(int i=0;i<width;i++)
            for(int j=0;j<width;j++)
                {
                        f32 d = sqrt((((f32)width * 0.5)-j-0.5)*(((f32)width * 0.5)-j-0.5)+
                                (((f32)width * 0.5)-i-0.5)*(((f32)width * 0.5)-i-0.5));

                        //color.setRed(r);
                        //color.setGreen(r);
                        //color.setBlue(r);
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

void do_test_loop(polyfold& pf, int f_i, int p_i, LineHolder& graph, LineHolder& graph2)
{
    if(f_i >= pf.faces.size() || p_i >= pf.faces[f_i].loops.size())
        return;

    poly_face* f = &pf.faces[f_i];
    /*
    //pf.faces[f_i].loops[p_i].reverse();

    // unit vectors
    core::vector3df fr(f->m_normal);
    core::vector3df to(vector3df(0,1,0));
    fr.normalize();
    to.normalize();

    // axis multiplication by sin
    core::vector3df vs(to.crossProduct(fr));

    // axis of rotation
    core::vector3df v(vs);
    v.normalize();

    // cosine angle
    f32 ca = fr.dotProduct(to);

    std::cout << ca << " !!!\n";
    std::cout << vs.X << "," << vs.Y << "," << vs.Z << "\n";
   
   */
    if(f->loops[p_i].vertices.size()==0)
    return;

   // pf.faces[f_i].loops[p_i].reverse();
    //pf.set_loop_solid(f_i, p_i);
   // f->flip_normal();
   // f->m_normal *= -1;
   // 
   // std::cout << f->bFlippedNormal << " ??\n";

   // f->bFlippedNormal = true;
   // f->m_normal.Y *= -1;

    pf.calc_loop_bbox(*f, f->loops[0]);

    core::matrix4 R = f->get2Dmat();

    MAT_COUT(R);

    //R.setRotationCenter(vector3df(0, 0, 0), f->m_center);

    std::vector<int> tempv = f->loops[p_i].vertices;
    tempv.push_back(tempv[0]);

    for(int i=0; i < tempv.size()-1; i++)
       {

        core::vector3df v0=pf.vertices[tempv[i]].V;
        core::vector3df v1=pf.vertices[tempv[i+1]].V;

       // std::cout<<v0.X<<" "<<v0.Z<<" to "<<v1.X<<" "<<v1.Z<<"\n";
        R.rotateVect(v0);
        R.rotateVect(v1);
        v0.Y=0;
        v1.Y=0;
            {
                graph.lines.push_back(core::line3df(v0,v1));
            }

        }
    std::vector<core::vector3df> test_points;

    //-128.001,45.2558,-445.256
    core::vector3df av = core::vector3df(53.9966,0,64.0015);
    //R.rotateVect(av);
    //test_points.push_back(av);

    f32 s = 16;

   /* vector3df zvec = vector3df(14.344, 766.905, 8.57017);
    R.rotateVect(zvec);
    test_points.push_back(zvec);

    zvec = vector3df(16.7073, 766.905, -0.25);
    R.rotateVect(zvec);
    test_points.push_back(zvec);

    zvec = vector3df(31.4178, 765.81, 8.41837);
    R.rotateVect(zvec);
    test_points.push_back(zvec);*/
    

    for (int i = 0; i < pf.faces[f_i].loops[p_i].vertices.size(); i++)
    {
        int v_0 = pf.faces[f_i].loops[p_i].vertices[i];
        int v_1 = i + 1 < pf.faces[f_i].loops[p_i].vertices.size() ?
            pf.faces[f_i].loops[p_i].vertices[i + 1] : pf.faces[f_i].loops[p_i].vertices[0];

        core::vector3df v0 = pf.vertices[v_0].V;
        core::vector3df v1 = pf.vertices[v_1].V;

        core::vector3df r = v1 - v0;

        core::vector3df v3 = r.crossProduct(pf.faces[f_i].getOrientingNormal());
        v3.normalize();
        v3 *= 0.25;

        core::vector3df ret = v0 + (v1 - v0) * 0.5 - v3;

        //R.rotateVect(ret);
        test_points.push_back(ret);
    }

   // 

    if(false)
    for(int i=0; i< f->loops[p_i].vertices.size(); i++)
    {
        int v_i = f->loops[p_i].vertices[i];
        core::vector3df v0= pf.vertices[v_i].V;
        R.rotateVect(v0);
        test_points.push_back(v0);
        test_points.push_back(v0+core::vector3df(s,0,0));
        test_points.push_back(v0+core::vector3df(-s,0,0));
        test_points.push_back(v0+core::vector3df(0,0,s));
        test_points.push_back(v0+core::vector3df(0,0,-s));

        if (i + 1< f->loops[p_i].vertices.size())
        {
            int v_j = f->loops[p_i].vertices[i+1];
            core::vector3df v1 = pf.vertices[v_j].V;
            R.rotateVect(v1);
            core::vector3df v2 = v1 - v0;
            v2.normalize();
            test_points.push_back(v0 + v2 * s);
            test_points.push_back(v0 - v2 * s);
        }
    }

   //f32 testz = (f->loops[p_i].max_z - f->loops[p_i].min_z) * 0.5 + f->loops[p_i].min_z;
   //f32 testx = f->loops[p_i].min_x - 10;
   //test_points.push_back(vector3df(testx,0,testz));

    graph.lines.push_back(core::line3df(core::vector3df(f->loops[p_i].min_x,0, f->loops[p_i].min_z), 
        core::vector3df(f->loops[p_i].min_x, 0, f->loops[p_i].max_z)));
    graph.lines.push_back(core::line3df(core::vector3df(f->loops[p_i].min_x, 0, f->loops[p_i].min_z),
        core::vector3df(f->loops[p_i].max_x, 0, f->loops[p_i].min_z)));
    graph.lines.push_back(core::line3df(core::vector3df(f->loops[p_i].max_x, 0, f->loops[p_i].max_z),
        core::vector3df(f->loops[p_i].min_x, 0, f->loops[p_i].max_z)));
    graph.lines.push_back(core::line3df(core::vector3df(f->loops[p_i].max_x, 0, f->loops[p_i].max_z),
        core::vector3df(f->loops[p_i].max_x, 0, f->loops[p_i].min_z)));

    core::vector3df center = f->m_center;
    R.rotateVect(center);
    center.Y = 0;

    for (core::line3df& line : graph.lines)
    {
        line.end -= center;
        line.start -= center;
    }

    for(core::vector3df v : test_points)
    {
        core::vector3df V = v;
        //R.rotateVect(V);
        f32 small_number = 0.01;
        bool bIn = false;
        bool bOutOfZone = false;

        

        if(V.X < f->loops[p_i].min_x - small_number || V.X > f->loops[p_i].max_x + small_number ||
            V.Z < f->loops[p_i].min_z - small_number || V.Z > f->loops[p_i].max_z + small_number)
            {
            bOutOfZone= true;
            //std::cout << "out of zone\n";
            bIn = false;
            }
        
        //if(!bOutOfZone)
        {
            //if(pf.is_closed_loop(f_i,p_i))
            { 
                //bIn = (pf.left_right_test(f_i, p_i, V.X,V.Z)==1);
                bIn = pf.is_point_in_loop(f_i, p_i, V);
                std::cout << bIn << "\n";
            }
            //else
              //  std::cout<<"loop not closed\n";
        }

        R.rotateVect(V);

        V.Y=0;

        V -= center;

        if(bIn)
            graph.points.push_back(V);
        else
            graph2.points.push_back(V);
    }
}

bool  geometry_scene::WriteTextures(std::string fname)
{
     ofstream wf(fname,ios::out | ios::binary);

    if(!wf)
    {
        cout<<"Cannot open file\n";
        return false;
    }

    std::vector<video::ITexture*> textures_used;
    std::vector<std::wstring> texture_paths;

    for(int i=1;i<this->elements.size();i++)
	{
        for(int f_i =0 ;f_i<this->elements[i].brush.faces.size(); f_i++)
        {
            video::ITexture* tex_j = this->driver->getTexture(this->elements[i].brush.faces[f_i].texture_name.c_str());

            bool b=false;
            for(int j=0;j<textures_used.size();j++)
            {
                if(tex_j == textures_used[j])
                {
                    this->elements[i].brush.faces[f_i].texture_index = j;
                    b=true;
                }
            }
            if(!b)
            {
                textures_used.push_back(tex_j);
                texture_paths.push_back(this->elements[i].brush.faces[f_i].texture_name.c_str());
                this->elements[i].brush.faces[f_i].texture_index = texture_paths.size()-1;
    
            }
        }
	}

	wf<<textures_used.size()<<"\n";

	for(int i=0;i<textures_used.size();i++)
    {
        std::string str(texture_paths[i].begin(),texture_paths[i].end());
        wf<<str.c_str()<<"\n";
    }

    wf.close();
    return true;
}

bool geometry_scene::ReadTextures(io::path fname, std::vector<std::wstring>& texture_paths)
{
    ifstream rf(fname.c_str(),ios::in | ios::binary);

    if(!rf)
    {
        cout<<"Cannot open file\n";
        return false;
    }

    std::string line;

    int n_textures;
    getline(rf,line);
    n_textures = core::strtoul10(line.c_str());

   // std::cout<<n_textures<<" textures used:\n";

    for(int i=0;i<n_textures;i++)
    {
        getline(rf,line);
        std::wstring str(line.begin(),line.end());
        texture_paths.push_back(str.c_str());
    }

    rf.close();
    return true;
}


bool geometry_scene::WriteSceneNodesToFile(std::string fname)
{
    ofstream wf(fname,ios::out | ios::binary);

    if(!wf)
    {
        cout<<"Cannot open file\n";
        return false;
    }

    wf<<"38\n";

    int e = this->scene_nodes.size();
    wf.write((char*)&e,sizeof(int));

    for(Reflected_SceneNode* node : this->scene_nodes)
    {
        node->preEdit();
        reflect::TypeDescriptor_Struct* td = node->GetDynamicReflection();
        wf<<node->GetDynamicReflection()->name<<'\0';
        while(td)
        {
            //td->dump(node,0);
            td->serialize(wf,node);
            td = td->inherited_type;
        }
    }

    wf.close();
    if(!wf.good())
    {
        cout<<"error writing file\n";
        return false;
    }
    return true;

}
bool geometry_scene::ReadSceneNodesFromFile(io::path fname)
{
     ifstream rf(fname.c_str(),ios::in | ios::binary);

    if(!rf)
    {
        cout<<"Cannot open file\n";
        return false;
    }
    std::string line;

    int magic;
    getline(rf,line);
    magic = core::strtoul10(line.c_str());

    if(magic != 38)
    {
        std::cout<<fname.c_str()<<" is not a valid node file\n";
        rf.close();
        return false;
    }

    for(Reflected_SceneNode* node : this->scene_nodes)
    {
        node->remove();
        node->drop();
    }
    this->scene_nodes.clear();

    int n_nodes;
    rf.read((char*)&n_nodes,sizeof(int));

    //std::cout<<"reading "<<n_nodes<<" nodes\n";
    for(int i=0;i<n_nodes;i++)
    {
        getline(rf,line,'\0');
        reflect::TypeDescriptor_Struct* typeDescriptor = Reflected_SceneNode_Factory::getNodeTypeDescriptorByName(line);
        if(typeDescriptor)
        {
            //Reflected_SceneNode* new_node = ((reflect::TypeDescriptor_SN_Struct*)typeDescriptor)->create_func(smgr, -1, core::vector3df(0, 0, 0));
            Reflected_SceneNode* new_node = Reflected_SceneNode_Factory::CreateNodeByTypeName(typeDescriptor->name,this->smgr);
            if(new_node)
            {
                reflect::TypeDescriptor_Struct* td = typeDescriptor;
                while(td)
                {
                    td->deserialize(rf,new_node);
                    //td->dump(new_node,0);
                    td = td->inherited_type;
                }
                new_node->postEdit();
                this->scene_nodes.push_back(new_node);
            }
            else
                {
                    std::cout<<"*could not create node*\n";
                    rf.close();
                    return false;
                }
        }
        else
        {
            std::cout<<"*could not find type descriptor*\n";
            rf.close();
            return false;
        }
    }

    rf.close();
    if(!rf.good())
    {
        cout<<"error reading file\n";
        return false;
    }
    return true;
}

bool geometry_scene::Write2(std::string fname)
{
    ofstream wf(fname,ios::out | ios::binary);

    if(!wf)
    {
        cout<<"Cannot open file\n";
        return false;
    }

    reflect::TypeDescriptor_Struct* typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<geometry_scene>::get();

    typeDescriptor->serialize(wf,this);

    wf.close();
    if(!wf.good())
    {
        cout<<"error writing file\n";
        return false;
    }
    return true;
}

bool geometry_scene::Read2(io::path fname,io::path tex_fname)
{
    ifstream rf(fname.c_str(),ios::in | ios::binary);

    if(!rf)
    {
        cout<<"Cannot open file\n";
        return false;
    }

    std::vector<std::wstring> texture_paths;
    if(ReadTextures(tex_fname,texture_paths)==false)
        return false;

    reflect::TypeDescriptor_Struct* typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<geometry_scene>::get();

    typeDescriptor->deserialize(rf,this);

    //typeDescriptor->dump(this,0);

   // std::cout << "surface groups:\n";
    for (int j = 1; j < elements.size(); j++)
    {
        //std::cout << j << ":\n";
        //for (int i = 0; i < this->elements[j].brush.surface_groups.size(); i++)
        //    std::cout << i << " " << this->elements[j].brush.surface_groups[i].type << "\n";
    }


    for(int i=1;i<elements.size();i++)
        for(poly_face& face : elements[i].brush.faces)
        {
            if(face.texture_index < texture_paths.size())
                    face.texture_name = texture_paths[face.texture_index].c_str();
                else
                    face.texture_name="wall.bmp";
        }

    rf.close();
    if(!rf.good())
    {
        cout<<"error reading file\n";
        return false;
    }

    for(int i=0;i<this->elements.size();i++)
    {
        //Brushes
        this->elements[i].brush.reduce_edges_vertices();
        this->elements[i].brush.recalc_bbox();

        for (int f_i = 0; f_i < this->elements[i].brush.faces.size(); f_i++)
        {
            for (int p_i = 0; p_i < this->elements[i].brush.faces[f_i].loops.size(); p_i++)
                this->elements[i].brush.calc_loop_bbox(f_i, p_i);
        }

        for (poly_face& f : this->elements[i].brush.faces)
        {
            this->elements[i].brush.calc_center(f);
        }

        //Geometry
        this->elements[i].geometry.reduce_edges_vertices();
        this->elements[i].geometry.recalc_bbox();

        for (int f_i = 0; f_i < this->elements[i].geometry.faces.size(); f_i++)
        {
            for (int p_i = 0; p_i < this->elements[i].geometry.faces[f_i].loops.size(); p_i++)
                this->elements[i].geometry.calc_loop_bbox(f_i, p_i);
        }

        for (poly_face& f : this->elements[i].geometry.faces)
        {
            this->elements[i].geometry.calc_center(f);
        }

    }

   // std::cout<<"Loaded Geometry\n\n";

    return true;
}

bool geometry_scene::ExportFinalMesh(std::string fname)
{
    io::IFileSystem* fs = this->smgr->getFileSystem();
    io::IWriteFile* io = fs->createAndWriteFile(fname.c_str());

    scene::IMeshWriter* mesh_writer = smgr->createMeshWriter(scene::EMWT_IRR_MESH);

    //mesh_writer->writeMesh(io, this->getFinalMeshNode()->getMesh());

    delete mesh_writer;
    return true;
}

REFLECT_STRUCT_BEGIN(camera_info_struct)
    REFLECT_STRUCT_MEMBER(orthogonal)
    REFLECT_STRUCT_MEMBER(position)
    REFLECT_STRUCT_MEMBER(target)
   // REFLECT_STRUCT_MEMBER(upvec)
    //REFLECT_STRUCT_MEMBER(fovy)
   // REFLECT_STRUCT_MEMBER(aspect)
   // REFLECT_STRUCT_MEMBER(near)
   // REFLECT_STRUCT_MEMBER(far)
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
REFLECT_STRUCT_END()


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
    REFLECT_STRUCT_MEMBER(bounding_rect)
    REFLECT_STRUCT_MEMBER(lightmap_resolution)
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

bool ReadGUIStateFromFile(io::path fname)
{
    gui::IGUIEnvironment* env = device->getGUIEnvironment();
    gui::IGUIElement* root = env->getRootGUIElement();
    CameraQuad* quad = (CameraQuad*)root->getElementFromId(GUI_ID_CAMERA_QUAD, true);

    if (!quad) return false;

    ifstream rf(fname.c_str(), ios::in | ios::binary);

    if (!rf)
    {
        cout << "Cannot open file " << fname.c_str() << "\n";
        return false;
    }

    GUI_state_struct state_struct;

    reflect::TypeDescriptor_Struct* typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<GUI_state_struct>::get();

    typeDescriptor->deserialize(rf, &state_struct);

    //Top Left
    ICameraSceneNode* camera = quad->getPanel(0)->getCamera();

    camera->setPosition(state_struct.tl.my_camera.position);
    camera->setTarget(state_struct.tl.my_camera.target);
    camera->setProjectionMatrix(state_struct.tl.my_camera.projM, false);
    camera->updateAbsolutePosition();

    ((TestPanel_3D*)quad->getPanel(0))->SetDynamicLight(state_struct.dynamicLight);
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

    rf.close();
    if (!rf.good())
    {
        cout << "error reading file\n";
        return false;
    }

   // std::cout << "Loaded GUI state\n";

    return true;
}

bool WriteGUIStateToFile(io::path fname)
{
    gui::IGUIEnvironment* env = device->getGUIEnvironment();
    gui::IGUIElement* root = env->getRootGUIElement();
    CameraQuad* quad = (CameraQuad*)root->getElementFromId(GUI_ID_CAMERA_QUAD, true);

    if (!quad) return false;

    GUI_state_struct state_struct;

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
    TestPanel_2D* panel_TR = (TestPanel_2D * )quad->getPanel(1);
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

    //Write File
    ofstream wf(fname.c_str(), ios::out | ios::binary);

    if (!wf)
    {
        cout << "Cannot open file " << fname.c_str() << "\n";
        return false;
    }

    reflect::TypeDescriptor_Struct* typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<GUI_state_struct>::get();

    typeDescriptor->serialize(wf, &state_struct);

    wf.close();
    if (!wf.good())
    {
        cout << "error writing file\n";
        return false;
    }
    return true;
}

//=======================================================
//  Open Geometry File
//

Open_Geometry_File::Open_Geometry_File(geometry_scene* gs)
{
    g_scene = gs;

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
            std::cout << " >>>>> reading files from " << p.c_str() << "\n";

            FileSystem->changeWorkingDirectoryTo(p);

            g_scene->Read2("refl_serial.dat", "textures.txt");
            g_scene->ReadSceneNodesFromFile("nodes.dat");
            
            g_scene->set_originals();
            g_scene->build_total_geometry();
            g_scene->generate_meshes();

            ReadGUIStateFromFile("gui_state.dat");

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

void Open_Geometry_File::LoadProject(geometry_scene* gs, io::path folder)
{
    
    io::IFileSystem* FS = device->getFileSystem();

    if (FS && gs)
    {
        
        io::path restore_path = FS->getWorkingDirectory();
        FS->changeWorkingDirectoryTo(folder);
        
        gs->Read2("refl_serial.dat", "textures.txt");
        gs->ReadSceneNodesFromFile("nodes.dat");

        gs->set_originals();
        gs->build_total_geometry();
        gs->generate_meshes();

        ReadGUIStateFromFile("gui_state.dat");

        FS->changeWorkingDirectoryTo(restore_path);

    }

}


//=======================================================
//  Save Geometry File
//

Save_Geometry_File::Save_Geometry_File(geometry_scene* gs)
{
    g_scene = gs;

    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->Register(this);

    FileSystem = device->getFileSystem();

    File_Open_Tool::show("Save Project");
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

            g_scene->WriteSceneNodesToFile("nodes.dat");
            g_scene->WriteTextures("textures.txt");
            g_scene->Write2("refl_serial.dat");
            WriteGUIStateToFile("gui_state.dat");
            export_model("model.dat");
            export_model_2("model2.dat");
            WriteModelTextures("model_textures.txt");

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

bool Save_Geometry_File::export_model(io::path fname)
{
    SMesh* mesh = g_scene->final_meshnode_interface.getMesh();

    Model_Struct model{};

    if (mesh)
    {
        int n_buffers = mesh->getMeshBufferCount();

        model.vertex_buffers.resize(n_buffers);

        for (int i = 0; i < n_buffers; i++)
        {
            CMeshBuffer<video::S3DVertex2TCoords>* mesh_buffer = (CMeshBuffer<video::S3DVertex2TCoords>*)mesh->getMeshBuffer(i);

           // std::cout << g_scene->final_meshnode_interface.getMaterialsUsed()[i].texture->getName().getPath().c_str() << "\n";

            model.vertex_buffers[i].vertices.resize(mesh_buffer->getVertexCount());

            for (int j = 0; j < mesh_buffer->getVertexCount(); j++)
            {
                model.vertex_buffers[i].vertices[j].pos = mesh_buffer->Vertices[j].Pos;
                model.vertex_buffers[i].vertices[j].tex_coords_0 = mesh_buffer->Vertices[j].TCoords;
                model.vertex_buffers[i].vertices[j].tex_coords_1 = mesh_buffer->Vertices[j].TCoords2;
            }

            model.vertex_buffers[i].indices.resize(mesh_buffer->getIndexCount());

            for (int j = 0; j < mesh_buffer->getIndexCount(); j++)
            {
                model.vertex_buffers[i].indices[j] = mesh_buffer->Indices[j];
            }
        }

        std::vector<TextureMaterial> materials_used = g_scene->final_meshnode_interface.getMaterialsUsed();

        for (int m_i = 0; m_i < materials_used.size(); m_i++)
        {
            for (int i = 0; i < materials_used[m_i].faces.size(); i++)
            {
                int f_i = materials_used[m_i].faces[i];

                MeshBuffer_Chunk chunk = g_scene->final_meshnode_interface.get_mesh_buffer_by_face(f_i);
                int buffer_no = g_scene->final_meshnode_interface.get_buffer_index_by_face(f_i);

                core::vector3df verts[4];

                g_scene->get_total_geometry()->calc_tangent(f_i);
                g_scene->get_total_geometry()->faces[f_i].get3DBoundingQuad(verts);

                core::vector3df u_vec = verts[1] - verts[0];
                core::vector3df v_vec = verts[3] - verts[0];

                u_vec.normalize();
                v_vec.normalize();

                if (i < materials_used[m_i].blocks.size())
                {
                    core::rect<u16> rect = materials_used[m_i].blocks[i];

                    rect.UpperLeftCorner.X += 1;
                    rect.UpperLeftCorner.Y += 1;
                    rect.LowerRightCorner.X -= 1;
                    rect.LowerRightCorner.Y -= 1;

                    for (int j = chunk.begin_i; j < chunk.end_i; j++)
                    {
                        u16 idx = chunk.buffer->getIndices()[j];
                        video::S3DVertex2TCoords* vtx = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx];
                        core::vector3df V = vtx->Pos;

                        float_t u = core::vector3df(V - verts[0]).dotProduct(u_vec) / core::vector3df(verts[1] - verts[0]).getLength();
                        float_t v = core::vector3df(V - verts[0]).dotProduct(v_vec) / core::vector3df(verts[3] - verts[0]).getLength();

                        u = fabs(u) < 0.00001 ? 0 : u;
                        v = fabs(v) < 0.00001 ? 0 : v;

                        core::vector2df tex_coord = core::vector2df(
                            (rect.UpperLeftCorner.X + u * rect.getWidth()) / materials_used[m_i].lightmap_size,
                            (rect.UpperLeftCorner.Y + v * rect.getHeight()) / materials_used[m_i].lightmap_size
                        );

                        //std::cout << buffer_no << ", " << idx << " ("<< model.vertex_buffers[buffer_no].vertices.size() <<" )  ";
                        //std::cout << tex_coord.X << "," << tex_coord.Y << "\n";
                        model.vertex_buffers[buffer_no].vertices[idx].tex_coords_1 = tex_coord;
                    }
                }
            }
        }

        ofstream wf(fname.c_str(), ios::out | ios::binary);

        if (!wf)
        {
            cout << "Cannot open file\n";
            return false;
        }

        reflect::TypeDescriptor_Struct* typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<Model_Struct>::get();

        typeDescriptor->serialize(wf, &model);

        wf.close();
        if (!wf.good())
        {
            cout << "error writing file\n";
            return false;
        }

        return true;
        
    }

    return false;
}


bool Save_Geometry_File::export_model_2(io::path fname)
{
    SMesh* mesh = g_scene->edit_meshnode_interface.getMesh();

    Model_Struct model{};

    if (mesh)
    {
        int n_buffers = mesh->getMeshBufferCount();

        model.vertex_buffers.resize(n_buffers);
        model.faces_info.resize(n_buffers);

        std::vector<int> face_number_ref;
        face_number_ref.resize(g_scene->get_total_geometry()->faces.size());

        int c = 0;
        for (int f_i = 0; f_i < g_scene->get_total_geometry()->faces.size(); f_i++)
        {
            
            if (g_scene->get_total_geometry()->faces[f_i].loops.size() > 0)
            {
                face_number_ref[f_i] = c;

                int idx = g_scene->edit_meshnode_interface.get_buffer_index_by_face(f_i);
                CMeshBuffer<video::S3DVertex2TCoords>* mesh_buffer = (CMeshBuffer<video::S3DVertex2TCoords>*)mesh->getMeshBuffer(idx);

                model.vertex_buffers[c].vertices.resize(mesh_buffer->getVertexCount());

                for (int j = 0; j < mesh_buffer->getVertexCount(); j++)
                {
                    model.vertex_buffers[c].vertices[j].pos = mesh_buffer->Vertices[j].Pos;
                    model.vertex_buffers[c].vertices[j].tex_coords_0 = mesh_buffer->Vertices[j].TCoords;
                }

                model.vertex_buffers[c].indices.resize(mesh_buffer->getIndexCount());

                for (int j = 0; j < mesh_buffer->getIndexCount(); j++)
                {
                    model.vertex_buffers[c].indices[j] = mesh_buffer->Indices[j];
                }

                model.faces_info[c].normal = g_scene->get_total_geometry()->faces[f_i].m_normal;
                model.faces_info[c].lightmap_resolution = g_scene->get_total_geometry()->faces[f_i].lightmap_res - 2;

                core::vector3df verts[4];

                g_scene->get_total_geometry()->calc_tangent(f_i);
                g_scene->get_total_geometry()->faces[f_i].get3DBoundingQuad(verts);

                model.faces_info[c].bounding_rect.v0 = verts[0];
                model.faces_info[c].bounding_rect.v1 = verts[1];
                model.faces_info[c].bounding_rect.v2 = verts[2];
                model.faces_info[c].bounding_rect.v3 = verts[3];

                core::vector3df u_vec = verts[1] - verts[0];
                core::vector3df v_vec = verts[3] - verts[0];

                u_vec.normalize();
                v_vec.normalize();

                for (int j = 0; j < mesh_buffer->getVertexCount(); j++)
                {
                    core::vector3df V = mesh_buffer->Vertices[j].Pos;
                   
                    float_t u = core::vector3df(V - verts[0]).dotProduct(u_vec) / core::vector3df(verts[1] - verts[0]).getLength();
                    float_t v = core::vector3df(V - verts[0]).dotProduct(v_vec) / core::vector3df(verts[3] - verts[0]).getLength();

                    u = fabs(u) < 0.00001 ? 0 : u;
                    v = fabs(v) < 0.00001 ? 0 : v;

                    model.vertex_buffers[c].vertices[j].tex_coords_1 = core::vector2df(u, v);
                }
                c++;
            }
        }

        model.lightmaps_info.resize(g_scene->edit_meshnode_interface.getMaterialsUsed().size());

        std::vector<TextureMaterial> materials_used = g_scene->edit_meshnode_interface.getMaterialsUsed();

        for (int i = 0; i < materials_used.size(); i++)
        {
            model.lightmaps_info[i].faces.resize(materials_used[i].faces.size());
            model.lightmaps_info[i].lightmap_block_UL.resize(materials_used[i].faces.size());
            model.lightmaps_info[i].lightmap_block_BR.resize(materials_used[i].faces.size());
            model.lightmaps_info[i].size = materials_used[i].lightmap_size;

            //std::cout << "faces = " << materials_used[i].lightmap_size<<", " << materials_used[i].faces.size() << "\n";
            for (int j = 0; j < materials_used[i].faces.size(); j++)
            {
                //std::cout << face_number_ref[materials_used[i].faces[j]] << ", ";
                model.lightmaps_info[i].faces[j] = face_number_ref[materials_used[i].faces[j]];
                model.lightmaps_info[i].lightmap_block_UL[j].X = materials_used[i].blocks[j].UpperLeftCorner.X + 1;
                model.lightmaps_info[i].lightmap_block_UL[j].Y = materials_used[i].blocks[j].UpperLeftCorner.Y + 1;
                model.lightmaps_info[i].lightmap_block_BR[j].X = materials_used[i].blocks[j].LowerRightCorner.X - 1;
                model.lightmaps_info[i].lightmap_block_BR[j].Y = materials_used[i].blocks[j].LowerRightCorner.Y - 1;
            }
            //std::cout << "\n";
        }

        ofstream wf(fname.c_str(), ios::out | ios::binary);

        if (!wf)
        {
            cout << "Cannot open file\n";
            return false;
        }

        reflect::TypeDescriptor_Struct* typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<Model_Struct>::get();

        typeDescriptor->serialize(wf, &model);

        wf.close();
        if (!wf.good())
        {
            cout << "error writing file\n";
            return false;
        }

        return true;

    }

    return false;
}


bool Save_Geometry_File::WriteModelTextures(std::string fname)
{
    ofstream wf(fname, ios::out | ios::binary);

    if (!wf)
    {
        cout << "Cannot open file\n";
        return false;
    }

    SMesh* mesh = g_scene->final_meshnode_interface.getMesh();
    std::vector<std::string> texture_paths;

    if (mesh)
    {
        int n_buffers = mesh->getMeshBufferCount();

        for (int i = 0; i < n_buffers; i++)
        {
            std::string tex_name{ g_scene->final_meshnode_interface.getMaterialsUsed()[i].texture->getName().getPath().c_str() };

            texture_paths.push_back(tex_name);
        }
    }

    wf << texture_paths.size() << "\n";

    for (int i = 0; i < texture_paths.size(); i++)
    {
        wf << texture_paths[i].c_str() << "\n";
    }

    wf.close();
    return true;
}
