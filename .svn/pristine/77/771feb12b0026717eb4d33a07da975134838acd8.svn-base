/*
bool GetOrthoClickPoint(dimension2d<u32> viewSize, scene::ICameraSceneNode * camera, int clickx, int clicky, vector3df &hit_vec)
{
    const scene::SViewFrustum* frustum = camera->getViewFrustum();
    const core::vector3df cameraPosition = camera->getAbsolutePosition();

    vector3df vNearLeftDown = frustum->getNearLeftDown();
    vector3df vNearRightDown = frustum->getNearRightDown();
    vector3df vNearLeftUp = frustum->getNearLeftUp();
    vector3df vNearRightUp = frustum->getNearRightUp();

    f32 t_X = (f32)clickx / viewSize.Width;
    f32 t_Y = 1.0 - (f32)clicky / viewSize.Height;

    //screen space: Y is horizontal axis
    vector3df X_vec = (vNearRightDown - vNearLeftDown) * t_X;
    vector3df Y_vec = (vNearLeftUp - vNearLeftDown) * t_Y;
    vector3df target = vNearLeftDown + Y_vec + X_vec;

    hit_vec = target;
    return true;
}

bool GetOrthoScreenCoords(dimension2d<u32> screenSize, scene::ICameraSceneNode * camera, core::vector3df V, core::vector2di &out_coords)
{
    const scene::SViewFrustum* frustum = camera->getViewFrustum();
    const core::vector3df cameraPosition = camera->getAbsolutePosition();

    vector3df vNearLeftDown = frustum->getNearLeftDown();
    vector3df vNearRightDown = frustum->getNearRightDown();
    vector3df vNearLeftUp = frustum->getNearLeftUp();
    vector3df vNearRightUp = frustum->getNearRightUp();

    vector3df ray = frustum->getNearLeftDown() - frustum->getFarLeftDown();
    //ray*=-2;

    core::plane3df aplane(vNearLeftDown,vNearLeftUp,vNearRightDown);
    core::vector3df vIntersect;

    if(aplane.getIntersectionWithLimitedLine(V,V-ray,vIntersect))
    {
        core::vector3df v_X = (vNearRightDown - vNearLeftDown);
        v_X.normalize();
        f32 t_X = v_X.dotProduct(vIntersect-vNearLeftDown);
        t_X /= (vNearRightDown - vNearLeftDown).getLength();
        //f32 t_X = (vIntersect-vNearLeftDown).getLength();
        //t_X*=()

        core::vector3df v_Y = (vNearLeftUp - vNearLeftDown);
        v_Y.normalize();
        f32 t_Y = v_Y.dotProduct(vIntersect-vNearLeftDown);
        t_Y /= (vNearLeftUp - vNearLeftDown).getLength();
        //f32 t_Y = (vIntersect-vNearLeftDown).getLength();

        //std::cout<<v_X.getLength()<<"  "<<v_Y.getLength()<<"\n";
        //std::cout<<t_X*screenSize.Width<<"  "<<(1-t_Y)*screenSize.Height<<"\n";
        out_coords.X = core::round32(t_X*screenSize.Width);
        out_coords.Y = core::round32((1-t_Y)*screenSize.Height);

        return true;
    }
    return false;

}

void CameraPanel::Initialize(scene::ISceneManager* smgr_ ,MyEventReceiver* receiver_, geometry_scene* geo_scene_)
{
    this->smgr=smgr_;
    this->receiver=receiver_;
    this->geo_scene=geo_scene_;
}

CameraPanel::CameraPanel(core::dimension2du wsize, core::vector2di origin)
{
    this->windowSize = wsize;
    this->windowOrigin = origin;
}

bool CameraPanel::IsInWindow(core::vector2di coords)
{
    if(coords.X > windowOrigin.X && coords.X < windowOrigin.X+ windowSize.Width &&
       coords.Y > windowOrigin.Y && coords.Y < windowOrigin.Y+ windowSize.Height)
        return true;
    return false;
}

scene::ICameraSceneNode* CameraPanel3D::getCamera()
{
    if(this->camera == NULL)
    {
        if(this->smgr)
        {
            this->camera = smgr->addCameraSceneNode(0,core::vector3df(-300,400,-300),core::vector3df(0,0,0),-1,false);
            //this->camera->setPosition(core::vector3df(-300,400,-300));
            //this->camera->setTarget(core::vector3df(0,0,0));
        }
    }
    return this->camera;
}

CameraPanel3D::CameraPanel3D(core::dimension2du wsize, core::vector2di origin) :CameraPanel(wsize,origin)
{
}

int CameraPanel3D::Update()
{
    if(!this->smgr || !this->receiver)
        return 0;

    if(receiver->GetMouseState().LeftButtonDown == true)
        {
        mousex=receiver->GetMouseState().Position.X-windowOrigin.X;
        mousey=receiver->GetMouseState().Position.Y-windowOrigin.Y;
        //std::cout<<"window click "<<mousex<<","<<mousey<<"\n";
        }

    if(this->bHasControl==false)
    {
        bDragCamera=false;
        bRotateCamera=false;
        bZoomCamera=false;

        bMouseDown=false;
        rMouseDown=false;
        bHasControl=true;
        //this->getCamera();
    }

    if(receiver->GetMouseState().RightButtonDown == true)
        {
            mousex=receiver->GetMouseState().Position.X-windowOrigin.X;
            mousey=receiver->GetMouseState().Position.Y-windowOrigin.Y;

            if(rMouseDown==false && bMouseDown==false)
            {
            clickx=mousex;
            clicky=mousey;

            vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
            //std::cout<<"click: "<<clickx<<", "<<clicky<<"\n";

            vDragCameraRay = this->getCamera()->getAbsolutePosition()-this->getCamera()->getTarget();

            bRotateCamera = true;
            rMouseDown = true;
            }
            else if (rMouseDown == true && bMouseDown == false)
            {
            if(bRotateCamera)
                {
                int xRot = mousex-clickx;
                float yaw = 0.005 * xRot;
                core::vector3df newCameraRay = vDragCameraRay;
                core::matrix4 MCamRotate;
                MCamRotate.setRotationAxisRadians(yaw,core::vector3df(0,1,0));
                MCamRotate.rotateVect(newCameraRay);

                int yRot = mousey-clicky;
                float pitch = 0.005 * yRot;

                core::vector3df rot_axis = newCameraRay.crossProduct(core::vector3df(0,1,0));
                rot_axis.normalize();

                MCamRotate.setRotationAxisRadians(pitch,rot_axis);
                MCamRotate.rotateVect(newCameraRay);
                this->getCamera()->setPosition(this->getCamera()->getTarget()+newCameraRay);
                this->getCamera()->updateAbsolutePosition();
                }
            }
            else if(rMouseDown == false)
            {
                rMouseDown = true;
            }
        }
        else if(receiver->GetMouseState().RightButtonDown == false)
        {
            bRotateCamera=false;
            bZoomCamera=false;
            rMouseDown = false;
        }

		if(receiver->GetMouseState().LeftButtonDown == true)
        {
            //core::dimension2d<u32> screensize(640, 480);
            core::vector3df hitvec;

            mousex=receiver->GetMouseState().Position.X-windowOrigin.X;
            mousey=receiver->GetMouseState().Position.Y-windowOrigin.Y;

            if(bMouseDown==false && rMouseDown == false)
            {
                clickx=mousex;
                clicky=mousey;

                //std::cout<<"click: "<<clickx<<", "<<clicky<<"\n";

                //Initiate Drag
                if(GetPlaneClickVector(windowSize,this->getCamera(),clickx,clicky,hitvec))
                {
                    vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                    vDragCameraInitialTarget = this->getCamera()->getTarget();

                    vDragCameraOrigin = hitvec;
                    bDragCamera = true;
                    bRotateCamera = false;
                    bZoomCamera = false;

                    //std::cout<<"hit "<<hitvec.X<<","<<hitvec.Y<<","<<hitvec.Z<<"\n";
                    //graph.points.push_back(hitvec);
                    //core::vector2di coords;
                    //GetScreenCoords(screensize,this->getCamera(),hitvec,coords);
                    //std::cout<<coords.X<<" / "<<coords.Y<<"\n";

                }
                bMouseDown=true;
            }
            else if( bMouseDown == true && rMouseDown == false)
            {
                if(bDragCamera )
                {
                    core::vector3df tvec;
                    GetPlaneClickVector(windowSize,this->getCamera(),mousex,mousey,tvec);
                    dragLine = core::line3df(vDragCameraOrigin,tvec);

                    core::vector2di m_coords(mousex,mousey);
                    core::vector2di coords;
                    core::vector3df route = dragLine.getMiddle();

                    if(GetScreenCoords(windowSize,this->getCamera(),route,coords))
                    {
                        if(m_coords.getDistanceFrom(coords)>1)
                        {

                            tvec = dragLine.getVector();
                            tvec *=0.8;

                            this->getCamera()->setPosition(vDragCameraInitialPosition-tvec);
                            this->getCamera()->setTarget(vDragCameraInitialTarget-tvec);
                            this->getCamera()->updateAbsolutePosition();
                            vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                            vDragCameraInitialTarget = this->getCamera()->getTarget();
                        }
                    }
                }
            }
            else if( bMouseDown == true && rMouseDown == true)
            {
                if(bZoomCamera==false)
                {
                    vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                    vDragCameraInitialTarget = this->getCamera()->getTarget();
                    vDragCameraRay = this->getCamera()->getAbsolutePosition()-this->getCamera()->getTarget();

                    bZoomCamera=true;
                    bRotateCamera=false;
                    bDragCamera=false;
                }
                else
                {
                    int ydif = mousey-clicky;
                    float zoom_f = 1.0 - (0.01 * ydif);
                    this->getCamera()->setPosition(vDragCameraInitialTarget+(vDragCameraRay*zoom_f));
                    this->getCamera()->updateAbsolutePosition();
                }
            }
            else if(bMouseDown == false)
            {
                bMouseDown = true;
            }
        }
        else if(receiver->GetMouseState().LeftButtonDown == false)
        {
            if(receiver->GetMouseState().RightButtonDown == false)
                bHasControl=false;

            if(bMouseDown && bDragCamera)
            {
            }
            bMouseDown=false;
            bDragCamera=false;
            bZoomCamera=false;
        }


        if(bMouseDown || rMouseDown)
            return 1;
        else return 0;
}

bool  CameraPanel2D::click_hits_poly(polyfold* brush, core::vector2di v)
{
    if(brush == NULL) return false;

    core::vector3df hit_vec;
    GetOrthoClickPoint(this->windowSize,this->getCamera(),v.X,v.Y,hit_vec);

    core::vector3df look_dir = this->getCamera()->getTarget()-this->getCamera()->getAbsolutePosition();
    look_dir.normalize();
    core::line3df line(hit_vec,hit_vec+look_dir*3000);

    for(int j=0;j<brush->edges.size();j++)
    {
        core::vector3df v0 = brush->getVertex(j,0).V;
        core::vector3df v1 = brush->getVertex(j,1).V;
        core::vector2di w0;
        core::vector2di w1;
        if(GetOrthoScreenCoords(this->windowSize,this->getCamera(),v0,w0) &&
            GetOrthoScreenCoords(this->windowSize,this->getCamera(),v1,w1))
            {
            core::line2di screen_line(w0,w1);

            core::vector2di click_coord(v.X,v.Y);
            core::vector2di r = screen_line.getClosestPoint(click_coord);
            f32 d = r.getDistanceFrom(click_coord);
            if(d<4)
                {
                return true;
                }
            }
    }
    return false;
}


int CameraPanel2D::Update()
{

    if(this->bHasControl==false)
    {
        bDragCamera=false;
        bRotateCamera=false;
        bZoomCamera=false;

        bMouseDown=false;
        rMouseDown=false;
        bHasControl=true;
        this->getCamera();
    }

    if(receiver->GetMouseState().RightButtonDown == true)
        {
            mousex=receiver->GetMouseState().Position.X-windowOrigin.X;
            mousey=receiver->GetMouseState().Position.Y-windowOrigin.Y;

            if(rMouseDown == false)
                {
                    clickx=mousex;
                    clicky=mousey;
                    rMouseDown = true;
                    if(bMouseDown == false && this->geo_scene && this->selected_brush != -1)
                    {
                        if(this->click_hits_poly(&this->geo_scene->elements[this->selected_brush].brush,core::vector2di(clickx,clicky)))
                        {
                            int v0 = this->geo_scene->elements[this->selected_brush].selected_vertex;
                            vDragBrushOriginalPosition = this->geo_scene->elements[this->selected_brush].brush.vertices[v0].V;
                            bDragBrush=true;
                        }
                            //std::cout<<"doube hit\n";
                    }
                }
            else if(rMouseDown == true && bDragBrush == true)
                {
                    if(core::vector2di(clickx,clicky).getDistanceFrom(core::vector2di(mousex,mousey))>4)
                    {
                        int ydif = clicky-mousey;
                        int xdif = clickx-mousex;
                        core::vector3df V=vDragBrushOriginalPosition;
                        V+=this->vVertical*ydif*this->getViewScaling();
                        V+=this->vHorizontal*xdif*this->getViewScaling();
                        f32 X_i = grid_interval*core::round32(V.X/this->grid_interval);
                        f32 Y_i = grid_interval*core::round32(V.Y/this->grid_interval);
                        f32 Z_i = grid_interval*core::round32(V.Z/this->grid_interval);
                        V=core::vector3df(X_i,Y_i,Z_i);
                        core::matrix4 M;
                        int v0 = this->geo_scene->elements[this->selected_brush].selected_vertex;
                        core::vector3df T = V-this->geo_scene->elements[this->selected_brush].brush.vertices[v0].V;
                        if(T.getLength()>grid_interval/2)
                        {
                            M.setTranslation(T);
                            this->geo_scene->elements[this->selected_brush].brush.translate(M);
                        }
                    }
                }
        }
    else if(receiver->GetMouseState().RightButtonDown == false)
        {
            if(bZoomCamera)
            {
                bZoomCamera=false;

                vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                vDragCameraInitialTarget = this->getCamera()->getTarget();
                clickx=mousex;
                clicky=mousey;
                bDragCamera = true;
            }
            if(bDragBrush)
            {
                bDragBrush=false;
            }
            if(rMouseDown==true && mousex == clickx && mousey == clicky)
            {
                core::vector3df hit_vec;
                GetOrthoClickPoint(this->windowSize,this->getCamera(),clickx,clicky,hit_vec);
                //std::cout<<"click "<<hit_vec.X<<","<<hit_vec.Y<<","<<hit_vec.Z<<"\n";
                //core::vector2di w2;
               // GetOrthoScreenCoords(this->windowSize,this->getCamera(),hit_vec,w2);
               // std::cout<<w2.X<<"/"<<w2.Y<<" vs "<<clickx<<","<<clicky<<"\n";
                if(this->geo_scene)
                {
                    bool vertex_click=false;
                    if(this->selected_brush != -1)
                    {
                        if(this->click_hits_poly(&this->geo_scene->elements[this->selected_brush].brush,core::vector2di(clickx,clicky)))
                        {
                            for(int v_i=0;v_i<this->geo_scene->elements[this->selected_brush].brush.vertices.size();v_i++)
                            {
                                core::vector3df v0 = geo_scene->elements[this->selected_brush].brush.vertices[v_i].V;
                                core::vector2di w0;
                                if(GetOrthoScreenCoords(this->windowSize,this->getCamera(),v0,w0))
                                {
                                    f32 d = w0.getDistanceFrom(core::vector2di(clickx,clicky));
                                    if(d<4)
                                        {
                                            this->geo_scene->elements[this->selected_brush].selected_vertex=v_i;
                                            vertex_click=true;
                                        }
                                }
                            }
                        }
                    }
                    if(vertex_click==false)
                    {
                        bool made_selection = false;
                        for(int i=0;i<geo_scene->elements.size();i++)
                        {
                        core::vector3df look_dir = this->getCamera()->getTarget()-this->getCamera()->getAbsolutePosition();
                        look_dir.normalize();
                        core::line3df line(hit_vec,hit_vec+look_dir*3000);
                        //if(geo_scene->elements[i].brush.bbox.intersectsWithLine(line))

                        for(int j=0;j<geo_scene->elements[i].brush.edges.size();j++)
                            {
                                core::vector3df v0 = geo_scene->elements[i].brush.getVertex(j,0).V;
                                core::vector3df v1 = geo_scene->elements[i].brush.getVertex(j,1).V;
                                core::vector2di w0;
                                core::vector2di w1;
                                if(GetOrthoScreenCoords(this->windowSize,this->getCamera(),v0,w0) &&
                                    GetOrthoScreenCoords(this->windowSize,this->getCamera(),v1,w1))
                                    {
                                    core::line2di screen_line(w0,w1);

                                    core::vector2di click_coord(clickx,clicky);
                                    core::vector2di r = screen_line.getClosestPoint(click_coord);
                                    f32 d = r.getDistanceFrom(click_coord);
                                    if(d<4)
                                        {
                                        geo_scene->select(i);
                                        made_selection = true;
                                        this->selected_brush=i;
                                        }
                                    }
                            }
                        }
                        if(made_selection == false)
                        {
                            geo_scene->select(-1);
                            this->selected_brush=-1;
                        }
                    }
                }
            }
            rMouseDown = false;
        }

    if(receiver->GetMouseState().LeftButtonDown == true)
        {

            mousex=receiver->GetMouseState().Position.X-windowOrigin.X;
            mousey=receiver->GetMouseState().Position.Y-windowOrigin.Y;

            if(bMouseDown==false && rMouseDown == false)
            {
                clickx=mousex;
                clicky=mousey;

                //Initiate Drag
                vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                vDragCameraInitialTarget = this->getCamera()->getTarget();

                bDragCamera = true;
                bRotateCamera = false;
                bZoomCamera = false;

                bMouseDown=true;
            }
            else if( bMouseDown == true && rMouseDown == false)
            {
                if(bDragCamera )
                {
                    int ydif = mousey-clicky;
                    int xdif = mousex-clickx;
                    core::vector3df vMove = xdif*this->vHorizontal+ydif*this->vVertical;
                    vMove*=((f32)this->viewSize.Width / this->windowSize.Width);
                    this->getCamera()->setPosition(vDragCameraInitialPosition+vMove);
                    this->getCamera()->setTarget(vDragCameraInitialTarget+vMove);
                    this->getCamera()->updateAbsolutePosition();
                }
            }
            else if( bMouseDown == true && rMouseDown == true)
            {
                if(bZoomCamera==false)
                {
                    vDragCameraInitialPosition = this->getCamera()->getAbsolutePosition();
                    vDragCameraInitialTarget = this->getCamera()->getTarget();
                    vDragCameraRay = this->getCamera()->getAbsolutePosition()-this->getCamera()->getTarget();
                    this->oldViewSize=this->viewSize;

                    bZoomCamera=true;
                    bDragCamera=false;
                }
                else
                {

                    int ydif = mousey-clicky;
                    float zoom_f = 1.0-(0.01 * ydif);

                    core::matrix4 M;
                    M.buildProjectionMatrixOrthoRH(oldViewSize.Width*(zoom_f),oldViewSize.Height*(zoom_f),100,10000);
                    this->camera->setProjectionMatrix(M,true);
                    this->viewSize.Width=oldViewSize.Width*(zoom_f);
                    this->viewSize.Height=oldViewSize.Height*(zoom_f);
                }
            }
            else if(bMouseDown == false)
            {
                clickx=mousex;
                clicky=mousey;
                bMouseDown = true;
            }
        }
        else if(receiver->GetMouseState().LeftButtonDown == false)
        {
            if(receiver->GetMouseState().RightButtonDown == false)
                bHasControl=false;
            if(bZoomCamera)
            {
                bZoomCamera=false;
            }

            if(bMouseDown && bDragCamera)
            {
            }
            bMouseDown=false;
            bDragCamera=false;

        }
}



CameraPanel2D::CameraPanel2D(core::dimension2du wsize, core::vector2di origin) :CameraPanel(wsize,origin)
{
    this->viewSize=wsize;
}


scene::ICameraSceneNode* CameraPanel2D::getCamera()
{
    if(this->camera == NULL)
    {
        if(this->smgr)
        {
            core::matrix4 M;
            M.buildProjectionMatrixOrthoRH(windowSize.Width,windowSize.Height,100,10000);
            this->camera = smgr->addCameraSceneNode(0,core::vector3df(0,1000,0),core::vector3df(0,0,0),-1,false);
            this->camera->setProjectionMatrix(M,true);
            //this->camera->setPosition(core::vector3df(-300,400,-300));
            //this->camera->setTarget(core::vector3df(0,0,0));
        }
    }
    return this->camera;
}

void CameraPanel2D::setAxis(int axis)
{
    if(axis==CAMERA_Y_AXIS)
    {
        this->getCamera()->setPosition(core::vector3df(0,1000,0));
        this->getCamera()->setTarget(core::vector3df(0,0,0));
        this->vHorizontal=core::vector3df(0,0,1);
        this->vVertical=core::vector3df(1,0,0);
    }
    else if(axis==CAMERA_X_AXIS)
    {
        this->getCamera()->setPosition(core::vector3df(1000,0,0));
        this->getCamera()->setTarget(core::vector3df(0,0,0));
        this->vHorizontal=core::vector3df(0,0,-1);
        this->vVertical=core::vector3df(0,1,0);
    }
    else if(axis==CAMERA_Z_AXIS)
    {
        this->getCamera()->setPosition(core::vector3df(0,0,1000));
        this->getCamera()->setTarget(core::vector3df(0,0,0));
        this->vHorizontal=core::vector3df(1,0,0);
        this->vVertical=core::vector3df(0,1,0);
    }
    this->m_axis=axis;
}

void CameraPanel2D::drawGrid(video::IVideoDriver* driver, const video::SMaterial material)
{
    //driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    //driver->setMaterial(material);

    //driver->draw3DLine(core::vector3df(-1000,0,0),core::vector3df(1000,0,0),video::SColor(12,12,0,48));
    //driver->draw3DLine(core::vector3df(0,0,-1000),core::vector3df(0,0,1000),video::SColor(12,12,0,48));
    //driver->draw3DLine(core::vector3df(0,-1000,0),core::vector3df(0,1000,0),video::SColor(12,12,0,48));

    if(m_axis==CAMERA_Y_AXIS)
    {
        int interval = this->grid_interval;
        while(viewSize.Width / interval > 36 || viewSize.Height / interval > 36)
        {
            interval = interval<<1;
        }
        int h_lines = viewSize.Width / interval;
        int v_lines = viewSize.Height / interval;
        core::vector3df vDownLeft = this->getCamera()->getAbsolutePosition();
        vDownLeft.Z-=viewSize.Width/2;
        vDownLeft.X-=viewSize.Height/2;
        int start_x = interval*((int)vDownLeft.Z/interval);
        int start_y = interval*((int)vDownLeft.X/interval);

        video::SColor col;
        for(int i=-1;i<h_lines+2;i++)
        {
            if(start_x+i*interval==0)
                col = video::SColor(200,128,128,128);
            else
                col = video::SColor(200,48,48,48);
            driver->draw3DLine(core::vector3df(vDownLeft.X,0,start_x+i*interval),core::vector3df(vDownLeft.X+viewSize.Height,0,start_x+i*interval),col);
        }

        for(int i=-1;i<v_lines+2;i++)
        {
            if(start_y+i*interval==0)
                col = video::SColor(200,128,128,128);
            else
                col = video::SColor(200,48,48,48);
            driver->draw3DLine(core::vector3df(start_y+i*interval,0,vDownLeft.Z),core::vector3df(start_y+i*interval,0,vDownLeft.Z+viewSize.Width),col);
        }
    }
    else if(m_axis==CAMERA_X_AXIS)
    {
        int interval = this->grid_interval;
        while(viewSize.Width / interval > 36 || viewSize.Height / interval > 36)
        {
            interval = interval<<1;
        }
        int h_lines = viewSize.Width / interval;
        int v_lines = viewSize.Height / interval;
        core::vector3df vDownLeft = this->getCamera()->getAbsolutePosition();
        vDownLeft.Z-=viewSize.Width/2;
        vDownLeft.Y-=viewSize.Height/2;
        int start_x = interval*((int)vDownLeft.Z/interval);
        int start_y = interval*((int)vDownLeft.Y/interval);

        video::SColor col;
        for(int i=-1;i<h_lines+2;i++)
        {
            if(start_x+i*interval==0)
                col = video::SColor(200,128,128,128);
            else
                col = video::SColor(200,48,48,48);
            driver->draw3DLine(core::vector3df(0,vDownLeft.Y,start_x+i*interval),core::vector3df(0,vDownLeft.Y+viewSize.Height,start_x+i*interval),col);
        }

        for(int i=-1;i<v_lines+2;i++)
        {
            if(start_y+i*interval==0)
                col = video::SColor(200,128,128,128);
            else
                col = video::SColor(200,48,48,48);
            driver->draw3DLine(core::vector3df(0,start_y+i*interval,vDownLeft.Z),core::vector3df(0,start_y+i*interval,vDownLeft.Z+viewSize.Width),col);
        }

    }
    else if(m_axis==CAMERA_Z_AXIS)
    {
        int interval = this->grid_interval;
        while(viewSize.Width / interval > 36 || viewSize.Height / interval > 36)
        {
            interval = interval<<1;
        }
        int h_lines = viewSize.Width / interval;
        int v_lines = viewSize.Height / interval;
        core::vector3df vDownLeft = this->getCamera()->getAbsolutePosition();
        vDownLeft.X-=viewSize.Width/2;
        vDownLeft.Y-=viewSize.Height/2;
        int start_x = interval*((int)vDownLeft.X/interval);
        int start_y = interval*((int)vDownLeft.Y/interval);

        video::SColor col;
        for(int i=-1;i<h_lines+2;i++)
        {
            if(start_x+i*interval==0)
                col = video::SColor(200,128,128,128);
            else
                col = video::SColor(200,48,48,48);
            driver->draw3DLine(core::vector3df(start_x+i*interval,vDownLeft.Y,0),core::vector3df(start_x+i*interval,vDownLeft.Y+viewSize.Height,0),col);
        }

        for(int i=-1;i<v_lines+2;i++)
        {
            if(start_y+i*interval==0)
                col = video::SColor(200,128,128,128);
            else
                col = video::SColor(200,48,48,48);
            driver->draw3DLine(core::vector3df(vDownLeft.X,start_y+i*interval,0),core::vector3df(vDownLeft.X+viewSize.Width,start_y+i*interval,0),col);
        }
    }
}
/*
void CameraPanel2D::drawCircle(video::IVideoDriver* driver, const video::SMaterial material,core::vector2di v,int rad,video::SColor col)
{
    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(material);

    core::vector3df V;
    GetOrthoClickPoint(this->windowSize,this->getCamera(),v.X,v.Y,V);

    f32 R = (f32)rad * ((f32)this->viewSize.Width/ (f32)this->windowSize.Width);
    f32 pi = 3.1415926;
    core::vector3df lookdir = this->getCamera()->getTarget()-this->getCamera()->getAbsolutePosition();
    lookdir.normalize();
    core::vector3df up = this->vVertical;
    up.normalize();
    core::vector3df right = this->vHorizontal;

    for(int i=0; i<12; i++)
    {
            core::vector3df V1 = V;
            V1 = V1 + up*R*cos(pi*i/6);
            V1 = V1 + right*R*sin(pi*i/6);

            core::vector3df V2 = V;
            V2 = V2 + up*R*cos(pi*(i+1)/6);
            V2 = V2 + right*R*sin(pi*(i+1)/6);

            driver->draw3DLine(V1,V2,col);
    }
}

void CameraPanel3D::drawAll(video::IVideoDriver* driver)
{
    video::SMaterial someMaterial;
    someMaterial.Lighting = false;
    someMaterial.Thickness = 1.0;
    someMaterial.MaterialType = video::EMT_SOLID;

    video::SMaterial someMaterial_15;
    someMaterial_15.Lighting = false;
    someMaterial_15.Thickness = 1.5;
    someMaterial_15.MaterialType = video::EMT_SOLID;

    video::SMaterial someMaterial_2;
    someMaterial_2.Lighting = false;
    someMaterial_2.Thickness = 2.0;
    someMaterial_2.MaterialType = video::EMT_SOLID;

}

void CameraPanel2D::drawAll(video::IVideoDriver* driver)
{
    video::SMaterial someMaterial;
    someMaterial.Lighting = false;
    someMaterial.Thickness = 1.0;
    someMaterial.MaterialType = video::EMT_SOLID;


    //camera_panel.drawCircle(driver,someMaterial_15,core::vector2di(32,32),4);

    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(someMaterial);

    this->drawGrid(driver,someMaterial);

    if(this->geo_scene)
    {
        for(geo_element geo : this->geo_scene->elements)
        {
            geo.draw_brush(driver,someMaterial);

            if(geo.bSelected && geo.selected_vertex < geo.brush.vertices.size())
            {
                 core::vector2di coords;
                 GetOrthoScreenCoords(this->windowSize,this->getCamera(),geo.brush.vertices[geo.selected_vertex].V,coords);
                // this->drawCircle(driver,someMaterial,coords,6,geo.getColor());
            }

        }

        //driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
        //driver->setMaterial(material);


        //for(geo_element geo : this->geo_scene->elements)
        //    geo.draw_geometry(driver,someMaterial);
    }
}
*/

/*

polyfold clip_poly2(polyfold& pf, polyfold& pf2, int rule, int rule2, LineHolder &graph, int pass_no)
{
    pf.classify_edges(graph);
    pf2.classify_edges(graph);

    polyfold pf4 = pf;
    polyfold pf5 = pf2;
    polyfold ret;

    //pf5.clear_ghost_loops();
    //pf5.reduce_edges_vertices2();

    std::cout<<"Creating new geometry...\n";

    for(poly_edge &edge : pf4.edges)
    {
     edge.topo_group=2;
    }

    for(poly_edge &edge : pf5.edges)
    {
     edge.topo_group=2;
    }
    int num=0;

    //intersections and bisect - pf4

    for(int face_j=0;face_j<pf2.faces.size();face_j++)
    {
        poly_face f2=pf2.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);
        core::vector3df ipoint;


        for(int e_i=0; e_i<pf4.edges.size(); e_i++)
        {
            core::vector3df v0=pf4.getVertex(e_i,0).V;
            core::vector3df v1=pf4.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf4.edges[e_i].topo_group != -1
               && pf2.is_point_on_face(face_j,ipoint))
            {
                int new_v = pf4.get_point_or_add(ipoint);
                pf4.bisect_edge(e_i,new_v,2,2);
            }
        }
    }
    //intersections and bisect - pf5

    for(int face_j=0;face_j<pf.faces.size();face_j++)
    {
        poly_face f=pf.faces[face_j];
        core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf5.edges.size(); e_i++)
        {
            core::vector3df v0=pf5.getVertex(e_i,0).V;
            core::vector3df v1=pf5.getVertex(e_i,1).V;
            if(line_intersects_plane(f_plane,v0,v1,ipoint) && pf5.edges[e_i].topo_group != -1
               && pf.is_point_on_face(face_j,ipoint))
            {
                int new_v = pf5.get_point_or_add(ipoint);
                pf5.bisect_edge(e_i,new_v,2,2);
            }
        }
    }

    for(int v_i=0; v_i<pf5.vertices.size(); v_i++)
    {
        if(pf4.find_point(pf5.vertices[v_i].V) == -1)
           {
            for(int e_i=0; e_i<pf4.edges.size(); e_i++)
                {
                    if(pf4.point_is_on_edge(e_i,pf5.vertices[v_i].V))
                    {
                        //std::cout<<"bonus vertex!\n";
                        int new_v = pf4.get_point_or_add(pf5.vertices[v_i].V);
                        pf4.bisect_edge(e_i,new_v,2,2);
                    }
                }
           }
    }

    for(int v_i=0; v_i<pf4.vertices.size(); v_i++)
    {
        if(pf5.find_point(pf4.vertices[v_i].V) == -1)
           {
            for(int e_i=0; e_i<pf5.edges.size(); e_i++)
                {
                    if(pf5.point_is_on_edge(e_i,pf4.vertices[v_i].V))
                    {
                        //std::cout<<"bonus vertex!\n";
                        int new_v = pf5.get_point_or_add(pf4.vertices[v_i].V);
                        pf5.bisect_edge(e_i,new_v,2,2);
                    }
                }
           }
    }

    for(poly_edge &edge : pf4.edges)
        edge.perimeter=0;

    for(poly_edge &edge : pf5.edges)
        edge.perimeter=0;

    //initial topology - pf4
    for(int face_j=0;face_j<pf2.faces.size();face_j++)
    {
        poly_face f2=pf2.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf4.edges.size(); e_i++)
        {
            core::vector3df v0 = pf4.getVertex(e_i,0).V;
            core::vector3df v1 = pf4.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf4.edges[e_i].topo_group != -1
               && pf2.is_point_on_face(face_j,ipoint))
            {
                int new_v = pf4.get_point_or_add(ipoint);

                if(new_v == pf4.edges[e_i].v0)
                {
                    v0 = pf4.getVertex(e_i,0).V;
                    v1 = pf4.getVertex(e_i,1).V;
                }
                else
                {
                    v1 = pf4.getVertex(e_i,0).V;
                    v0 = pf4.getVertex(e_i,1).V;
                }

               // std::cout<<"v0: "<<v0.X<<","<<v0.Y<<","<<v0.Z<<"\n";
               // std::cout<<"v1: "<<v1.X<<","<<v1.Y<<","<<v1.Z<<"\n";
               // graph.points.push_back(ipoint);

                int RES = pf2.classify_point(face_j,v0,v1);


                if(RES == TOP_FRONT)
                    pf4.edges[e_i].topo_group=0;
                else if(RES == TOP_BEHIND)
                    pf4.edges[e_i].topo_group=1;

                new_v = pf5.get_point_or_add(ipoint);
                pf5.faces[face_j].addVertex(new_v);
            }
        }
    }

    //catch special cases
    for(int e_i=0; e_i<pf4.edges.size(); e_i++)
        {
            if(pf4.edges[e_i].topo_group == 2)
                {
                core::vector3df v0 = pf4.getVertex(e_i,0).V;
                core::vector3df v1 = pf4.getVertex(e_i,1).V;
                int RES = TOP_UNDEF;
                int v_0 = pf2.find_point(v0);
                int v_1 = pf2.find_point(v1);

                if(v_0 != -1)
                {
                    RES = pf2.classify_point_point_special(v_0,v1);
                }
                if(v_1 != -1)
                {
                    RES = pf2.classify_point_point_special(v_1,v0);
                }

                if(RES == TOP_FRONT)
                    pf4.edges[e_i].topo_group=0;
                else if(RES == TOP_BEHIND)
                    pf4.edges[e_i].topo_group=1;
                }
        }

    //initial topology - pf5
    for(int face_j=0;face_j<pf.faces.size();face_j++)
    {
        poly_face f=pf.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f.m_center,f.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf5.edges.size(); e_i++)
        {
            core::vector3df v0 = pf5.getVertex(e_i,0).V;
            core::vector3df v1 = pf5.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf5.edges[e_i].topo_group != -1
               && pf.is_point_on_face(face_j,ipoint))
            {
                int new_v = pf5.get_point_or_add(ipoint);

                //graph.points.push_back(ipoint);
                if(new_v == pf5.edges[e_i].v0)
                {
                    v0 = pf5.getVertex(e_i,0).V;
                    v1 = pf5.getVertex(e_i,1).V;
                }
                else
                {
                    v1 = pf5.getVertex(e_i,0).V;
                    v0 = pf5.getVertex(e_i,1).V;
                }

                int RES = pf.classify_point(face_j,v0,v1);

                if(RES == TOP_FRONT)
                    pf5.edges[e_i].topo_group=0;
                else if(RES == TOP_BEHIND)
                    pf5.edges[e_i].topo_group=1;

                new_v = pf4.get_point_or_add(ipoint);
                pf4.faces[face_j].addVertex(new_v);
            }
        }
    }


    //common topology
    for(int face_i=0;face_i<pf.faces.size();face_i++)
        for(int face_j=0;face_j<pf2.faces.size();face_j++)
        {
            poly_face f=pf.faces[face_i];
            poly_face f2=pf2.faces[face_j];

            plane res;

            core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
            core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);


            if(f_plane.getIntersectionWithPlane(f2_plane,res.point,res.norm))
            {

                polyfold verts;
                for(int v_i : pf4.faces[face_i].vertices)
                {
                    core::vector3df v = pf4.vertices[v_i].V;
                    if(fabs(f2_plane.getDistanceTo(v))<0.01 && pf2.is_point_on_face(face_j,v))
                    {
                        verts.get_point_or_add(v);
                    }
                }

                for(int v_j : pf5.faces[face_j].vertices)
                {
                    core::vector3df v = pf5.vertices[v_j].V;
                    if(fabs(f_plane.getDistanceTo(v))<0.01 && pf.is_point_on_face(face_i,v))
                    {
                        verts.get_point_or_add(v);
                    }
                }
                if(verts.vertices.size()>0)
                {
                    sort_inline_vertices(verts);
                    for(int i=0;i<verts.vertices.size()-1;i++)
                    {
                        core::vector3df r = verts.vertices[i].V + (verts.vertices[i+1].V - verts.vertices[i].V)*0.5;
                        if(pf.is_point_on_face(face_i,r) && pf2.is_point_on_face(face_j,r))
                        {
                            int v0 = pf4.get_point_or_add(verts.vertices[i].V);
                            int v1 = pf4.get_point_or_add(verts.vertices[i+1].V);
                            int w0 = pf5.get_point_or_add(verts.vertices[i].V);
                            int w1 = pf5.get_point_or_add(verts.vertices[i+1].V);

                            //if(face_j==1)
                            {
                            //graph.points.push_back(verts.vertices[i].V);
                            //graph.points.push_back(verts.vertices[i+1].V);
                            //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                            }

                            int new_e;

                            if(pf4.find_edge(v0,v1)==-1)
                            {
                            //std::cout<<"add "<<v0<<","<<v1<<"\n";
                            new_e = pf4.get_edge_or_add(v0,v1,3);
                            pf4.edges[new_e].perimeter = 1;
                            pf4.faces[face_i].addEdge(new_e);

                            //graph.points.push_back(verts.vertices[i].V);
                            //graph.points.push_back(verts.vertices[i+1].V);

                            //new_e = pf5.get_edge_or_add(w0,w1,3);
                            //pf5.faces[face_j].addEdge(new_e);
                            }
                            else if(pf4.find_edge(v0,v1) != -1)
                            {
                            new_e = pf4.find_edge(v0,v1);
                           // if(pf4.edges[new_e].topo_group==2)
                                pf4.edges[new_e].topo_group=3;
                            }

                            if(pf5.find_edge(w0,w1)==-1)
                            {
                            new_e = pf5.get_edge_or_add(w0,w1,3);
                            pf5.edges[new_e].perimeter = 1;
                            pf5.faces[face_j].addEdge(new_e);
                            }
                            else
                            {
                            new_e = pf5.get_edge_or_add(w0,w1,3);
                            pf5.faces[face_j].addEdge(new_e);
                            }

                        }   //TESTING
                        else if((pf5.find_edge(verts.vertices[i].V,verts.vertices[i+1].V) != -1)&&
                                pass_no == CLIP_FIRST_PASS)
                        {

                        }
                        //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                    }
                }
            }
        }

    //========================================


    if(pf4.apply_topology_groups(rule) == false)
    {
      //  if(rule == rule2) //needs fixing...
        if(false)
        {
            for(int e_i = 0; e_i<pf4.edges.size(); e_i++)
            {
                if(pf4.edges[e_i].topo_group==2)
                    pf4.edges[e_i].topo_group=!rule;
            }
        }
        else
        {
            for(int e_i = 0; e_i<pf4.edges.size(); e_i++)
            {
                if(pf4.edges[e_i].topo_group==2)
                    pf4.edges[e_i].topo_group=rule;
            }

        }
    }

    pf5.apply_topology_groups(!rule);

    LineHolder nograph;

    for(int f_i=0; f_i<pf4.faces.size();f_i++)
    {
        pf4.calc_loops4(f_i,nograph);
        for(int p_i=0; p_i< pf4.faces[f_i].loops.size(); p_i++)
            pf4.set_loop_solid(f_i,p_i);
       // pf4.sort_loops(pf4.faces[f_i]);
    }

    for(int f_i=0; f_i<pf5.faces.size();f_i++)
    {
        pf5.calc_loops4(f_i,nograph);
        for(int p_i=0; p_i< pf5.faces[f_i].loops.size(); p_i++)
            pf5.set_loop_solid(f_i,p_i);
        //pf5.sort_loops(pf5.faces[f_i]);
    }

    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        pf4.sort_loops(f_i);
    }

    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
        {
            pf4.faces[f_i].loops[p_i].topo_group = LOOP_UNDEF;
        }

    num=0;

    //"kissing" loops
    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        core::plane3df f_plane = core::plane3df(pf4.faces[f_i].m_center,pf4.faces[f_i].m_normal);

        if(pf4.faces[f_i].loops.size()>0)
        {
            for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
            {
                if(pf4.faces[f_i].loops[p_i].vertices.size()>0)
                {
                    for(int f_j=0; f_j < pf5.faces.size(); f_j++)
                    {
                        if(fabs(f_plane.getDistanceTo(pf5.faces[f_j].m_center))<0.001 &&
                           fabs(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal))>0.999)
                            for(int p_j=0; p_j < pf5.faces[f_j].loops.size(); p_j++)
                            {
                                //std::cout<<"loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                if(pass_no == CLIP_FIRST_PASS)
                                {
                                    if(is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j])
                                   && pf4.faces[f_i].loops[p_i].type==LOOP_INNER)
                                    {
                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                                    num++;
                                    goto LoopHasBeenCulled;
                                    }
                                }
                                else if( pf4.faces[f_i].loops[p_i].type==LOOP_INNER &&
                                        (is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j]) ||
                                         is_contained_loop(pf4, pf4.faces[f_i].loops[p_i],pf5,f_j,p_j)))
                                {
                                    if(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
                                    {
                                        //std::cout<<"solid loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                        pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                                        num++;
                                        goto LoopHasBeenCulled;
                                    }
                                    else
                                    {
                                        //std::cout<<"hollow loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                        pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                                        num++;
                                        goto LoopHasBeenCulled;
                                    }
                                }
                            }
                    }
                }
                LoopHasBeenCulled:
                int z=0;
            }
        }
    }
    std::cout<<" "<<num<<" kissing loops\n";

    //return object
    //if(false)
    {
        for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        {
            poly_face f;
            f.m_normal = pf4.faces[f_i].m_normal;
            f.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;

            for(int p_i=0; p_i<pf4.faces[f_i].loops.size(); p_i++)
            {
                if(pf4.faces[f_i].loops[p_i].vertices.size() > 0 && pf4.faces[f_i].loops[p_i].type == LOOP_INNER)
                {
                    poly_loop new_loop;
                    for(int v_i : pf4.faces[f_i].loops[p_i].vertices)
                    {
                        int v = ret.get_point_or_add(pf4.vertices[v_i].V);
                        new_loop.vertices.push_back(v);
                        f.addVertex(v);
                    }

                    if(pf4.faces[f_i].loops[p_i].topo_group == LOOP_UNDEF)
                    {
                        std::vector<int> tempv = pf4.faces[f_i].loops[p_i].vertices;
                        tempv.push_back(tempv[0]);

                        bool exclude=false;
                        bool include=false;


                        for(int i=0; i < tempv.size()-1; i++)
                        {
                            core::vector3df v0=pf4.vertices[tempv[i]].V;
                            core::vector3df v1=pf4.vertices[tempv[i+1]].V;

                            if(pf4.edges[pf4.get_edge_or_add(tempv[i],tempv[i+1],!rule)].topo_group == !rule )
                            {
                                exclude = true;
                            }
                            else if(pf4.edges[pf4.get_edge_or_add(tempv[i],tempv[i+1],!rule)].topo_group == rule )
                            {
                                include = true;
                            }
                            int new_e = ret.get_edge_or_add(ret.get_point_or_add(v0),ret.get_point_or_add(v1),0);
                            f.addEdge(new_e);
                        }

                        if(exclude && !include )
                        {
                        //    std::cout<<"hollow\n";
                            pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                        }
                        else if(include && !exclude )
                        {
                        //    std::cout<<"solid\n";
                            pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                        }
                        else if((!exclude && !include ) || (exclude && include ))
                        {
                            std::cout<<" odd loop: "<<f_i<<", "<<p_i<<" ";
                            core::vector3df v0=pf4.vertices[tempv[0]].V;
                            core::vector3df v1;
                            int e_i = pf4.find_edge(tempv[0],tempv[1]);

                            if(e_i != -1 && pf4.get_facial_point3(f_i,e_i,p_i,v1,nograph))
                            {
                                int res = pf2.classify_point(v0,v1);
                                if (res == rule)
                                {
                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                                    std::cout<<" solid\n";
                                    exclude = false;
                                }
                                else
                                {
                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                                    std::cout<<" hollow\n";
                                    exclude = false;
                                }
                            }
                            else  pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                        }
                    }

                    new_loop.depth = pf4.faces[f_i].loops[p_i].depth;
                    new_loop.type = pf4.faces[f_i].loops[p_i].type;
                    new_loop.topo_group = pf4.faces[f_i].loops[p_i].topo_group;
                    ret.calc_loop_bbox(f,new_loop);

                    if(new_loop.topo_group != LOOP_UNDEF)
                    {
                        if(new_loop.topo_group == LOOP_SOLID || pass_no == CLIP_FIRST_PASS)
                        {
                            f.loops.push_back(new_loop);
                        }
                    }
                    //else std::cout<<"no loop!\n";
                }
                else if(pf4.faces[f_i].loops[p_i].vertices.size() > 0 && pf4.faces[f_i].loops[p_i].type == LOOP_OUTER
                        && pf4.faces[f_i].loops[p_i].depth%2 == 1)
                {
                    poly_loop new_loop;
                    for(int v_i : pf4.faces[f_i].loops[p_i].vertices)
                    {
                        int v = ret.get_point_or_add(pf4.vertices[v_i].V);
                        new_loop.vertices.push_back(v);
                        f.addVertex(v);
                    }

                    new_loop.type = pf4.faces[f_i].loops[p_i].type;
                    new_loop.depth = pf4.faces[f_i].loops[p_i].depth;
                    new_loop.topo_group = LOOP_HOLLOW;
                    new_loop.type = LOOP_INNER;
                    ret.calc_loop_bbox(f,new_loop);
                    //std::cout<<"hollow loop!\n";
                    f.loops.push_back(new_loop);
                }
            }

            ret.calc_center(f);
            if(f.loops.size() > 0)
            {
                ret.faces.push_back(f);
            }
        }

        if(pass_no == CLIP_SECOND_PASS)
        {
            ret.reduce_edges_vertices2();
            ret.recalc_bbox_and_loops();
        }
        else
        {
            ret.reduce_edges_vertices2();

            if(ret.vertices.size()>0)
            {
                ret.bbox.reset(ret.vertices[0].V);
                for(poly_vert v : ret.vertices)
                    ret.bbox.addInternalPoint(v.V);
            }
        }
    }

    return ret;
}
*/

/*

polyfold clip_poly3(polyfold& pf, polyfold& pf2, int rule, int rule2, LineHolder &graph, int pass_no)
{
    pf.classify_edges(graph);
    pf2.classify_edges(graph);

    polyfold pf4 = pf;
    polyfold pf5 = pf2;
    polyfold ret;

    std::cout<<"Creating new geometry...\n";

    if(pass_no==CLIP_SECOND_PASS)
    {
        pf5.clear_ghost_loops();
        pf5.reduce_edges_vertices2();
    }

    for(poly_edge &edge : pf4.edges)
    {
     edge.topo_group=2;
    }

    for(poly_edge &edge : pf5.edges)
    {
     edge.topo_group=2;
    }
    int num=0;

    //intersections and bisect - pf4
    for(int face_j=0;face_j<pf5.faces.size();face_j++)
    {
        poly_face f2=pf5.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);
        core::vector3df ipoint;


        for(int e_i=0; e_i<pf4.edges.size(); e_i++)
        {
            core::vector3df v0=pf4.getVertex(e_i,0).V;
            core::vector3df v1=pf4.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf4.edges[e_i].topo_group != -1
               && pf5.is_point_on_face(face_j,ipoint))
            {
                int new_v = pf4.get_point_or_add(ipoint);
                pf4.bisect_edge(e_i,new_v,2,2);
                //graph.points.push_back(ipoint);
               // graph.points.push_back(v1);
            }
        }
    }

    //intersections and bisect - pf5
    for(int face_j=0;face_j<pf.faces.size();face_j++)
    {
        poly_face f=pf.faces[face_j];
        core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf5.edges.size(); e_i++)
        {
            core::vector3df v0=pf5.getVertex(e_i,0).V;
            core::vector3df v1=pf5.getVertex(e_i,1).V;
            if(line_intersects_plane(f_plane,v0,v1,ipoint) && pf5.edges[e_i].topo_group != -1
               && pf.is_point_on_face(face_j,ipoint))
            {
                int new_v = pf5.get_point_or_add(ipoint);
                pf5.bisect_edge(e_i,new_v,2,2);
            }
        }
    }

    for(int v_i=0; v_i<pf5.vertices.size(); v_i++)
    {
        if(pf4.find_point(pf5.vertices[v_i].V) == -1)
           {
            for(int e_i=0; e_i<pf4.edges.size(); e_i++)
                {
                    if(pf4.point_is_on_edge(e_i,pf5.vertices[v_i].V))
                    {
                        //std::cout<<"bonus vertex!\n";
                        int new_v = pf4.get_point_or_add(pf5.vertices[v_i].V);
                        pf4.bisect_edge(e_i,new_v,2,2);
                    }
                }
           }
    }

    for(int v_i=0; v_i<pf4.vertices.size(); v_i++)
    {
        if(pf5.find_point(pf4.vertices[v_i].V) == -1)
           {
            for(int e_i=0; e_i<pf5.edges.size(); e_i++)
                {
                    if(pf5.point_is_on_edge(e_i,pf4.vertices[v_i].V))
                    {
                        //std::cout<<"bonus vertex!\n";
                        int new_v = pf5.get_point_or_add(pf4.vertices[v_i].V);
                        pf5.bisect_edge(e_i,new_v,2,2);
                    }
                }
           }
    }

    for(poly_edge &edge : pf4.edges)
        edge.perimeter=0;

    for(poly_edge &edge : pf5.edges)
        edge.perimeter=0;

    //initial topology - pf4
    for(int face_j=0;face_j<pf2.faces.size();face_j++)
    {
        poly_face f2=pf2.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf4.edges.size(); e_i++)
        {
            core::vector3df v0 = pf4.getVertex(e_i,0).V;
            core::vector3df v1 = pf4.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf4.edges[e_i].topo_group != -1
               && pf5.is_point_on_face(face_j,ipoint))
            {
                int new_v = pf4.get_point_or_add(ipoint);

                if(new_v == pf4.edges[e_i].v0)
                {
                    v0 = pf4.getVertex(e_i,0).V;
                    v1 = pf4.getVertex(e_i,1).V;
                }
                else
                {
                    v1 = pf4.getVertex(e_i,0).V;
                    v0 = pf4.getVertex(e_i,1).V;
                }

               // std::cout<<"v0: "<<v0.X<<","<<v0.Y<<","<<v0.Z<<"\n";
               // std::cout<<"v1: "<<v1.X<<","<<v1.Y<<","<<v1.Z<<"\n";
               // graph.points.push_back(ipoint);

                int RES = pf2.classify_point(face_j,v0,v1);


                if(RES == TOP_FRONT)
                    pf4.edges[e_i].topo_group=0;
                else if(RES == TOP_BEHIND)
                    pf4.edges[e_i].topo_group=1;

                new_v = pf5.get_point_or_add(ipoint);
                pf5.faces[face_j].addVertex(new_v);
            }
        }
    }

    //initial topology - pf5
    for(int face_j=0;face_j<pf.faces.size();face_j++)
    {
        poly_face f=pf.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f.m_center,f.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf5.edges.size(); e_i++)
        {
            core::vector3df v0 = pf5.getVertex(e_i,0).V;
            core::vector3df v1 = pf5.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf5.edges[e_i].topo_group != -1
               && pf.is_point_on_face(face_j,ipoint))
            {
                int new_v = pf5.get_point_or_add(ipoint);

                //graph.points.push_back(ipoint);
                if(new_v == pf5.edges[e_i].v0)
                {
                    v0 = pf5.getVertex(e_i,0).V;
                    v1 = pf5.getVertex(e_i,1).V;
                }
                else
                {
                    v1 = pf5.getVertex(e_i,0).V;
                    v0 = pf5.getVertex(e_i,1).V;
                }

                int RES = pf.classify_point(face_j,v0,v1);

                if(RES == TOP_FRONT)
                    pf5.edges[e_i].topo_group=0;
                else if(RES == TOP_BEHIND)
                    pf5.edges[e_i].topo_group=1;

                new_v = pf4.get_point_or_add(ipoint);
                pf4.faces[face_j].addVertex(new_v);
            }
        }
    }

    //common topology
    for(int face_i=0;face_i<pf.faces.size();face_i++)
        for(int face_j=0;face_j<pf2.faces.size();face_j++)
        {
            poly_face f=pf.faces[face_i];
            poly_face f2=pf2.faces[face_j];

            plane res;

            core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
            core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);

            if(f_plane.getIntersectionWithPlane(f2_plane,res.point,res.norm))
            {

                polyfold verts;
                for(int v_i : pf4.faces[face_i].vertices)
                {
                    core::vector3df v = pf4.vertices[v_i].V;
                    if(fabs(f2_plane.getDistanceTo(v))<0.01 && pf5.is_point_on_face(face_j,v))
                    {
                        verts.get_point_or_add(v);
                    }
                }

                for(int v_j : pf5.faces[face_j].vertices)
                {
                    core::vector3df v = pf5.vertices[v_j].V;
                    if(fabs(f_plane.getDistanceTo(v))<0.01 && pf.is_point_on_face(face_i,v))
                    {
                        verts.get_point_or_add(v);
                    }
                    if(face_j==12 && face_i==0)
                    {
                    //std::cout<<"12 vs "<<face_i<<"\n";
                   // if(pf2.is_point_on_face(face_j,v))
                    //    graph.points.push_back(pf5.vertices[v_j].V);
                    }

                }
                if(verts.vertices.size()>0)
                {
                    sort_inline_vertices(verts);
                    for(int i=0;i<verts.vertices.size()-1;i++)
                    {
                        //graph.points.push_back(verts.vertices[i].V);
                        //graph.points.push_back(verts.vertices[i+1].V);
                        core::vector3df r = verts.vertices[i].V + (verts.vertices[i+1].V - verts.vertices[i].V)*0.5;
                        if(pf.is_point_on_face(face_i,r) && pf5.is_point_on_face(face_j,r))
                        {
                            int v0 = pf4.get_point_or_add(verts.vertices[i].V);
                            int v1 = pf4.get_point_or_add(verts.vertices[i+1].V);
                            int w0 = pf5.get_point_or_add(verts.vertices[i].V);
                            int w1 = pf5.get_point_or_add(verts.vertices[i+1].V);

                            //if(face_j==1)
                            {
                            //graph.points.push_back(verts.vertices[i].V);
                            //graph.points.push_back(verts.vertices[i+1].V);
                            //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                            }

                            int new_e;

                            if(pf4.find_edge(v0,v1)==-1)
                            {
                            //std::cout<<"add "<<v0<<","<<v1<<"\n";
                            new_e = pf4.get_edge_or_add(v0,v1,3);
                            pf4.edges[new_e].perimeter = 1;
                            pf4.faces[face_i].addEdge(new_e);

                            //graph.points.push_back(verts.vertices[i].V);
                            //graph.points.push_back(verts.vertices[i+1].V);

                            //new_e = pf5.get_edge_or_add(w0,w1,3);
                            //pf5.faces[face_j].addEdge(new_e);
                            }
                            else if(pf4.find_edge(v0,v1) != -1)
                            {
                            new_e = pf4.find_edge(v0,v1);
                           // if(pf4.edges[new_e].topo_group==2)
                                pf4.edges[new_e].topo_group=3;
                            }

                            if(pf5.find_edge(w0,w1)==-1)
                            {
                            new_e = pf5.get_edge_or_add(w0,w1,3);
                            pf5.edges[new_e].perimeter = 1;
                            pf5.faces[face_j].addEdge(new_e);
                            }
                            else
                            {
                            new_e = pf5.get_edge_or_add(w0,w1,3);
                            pf5.faces[face_j].addEdge(new_e);
                            }

                        }   //TESTING
                        else if((pf5.find_edge(verts.vertices[i].V,verts.vertices[i+1].V) != -1)&&
                                pass_no == CLIP_FIRST_PASS)
                        {

                        }
                        //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                    }
                }
            }
        }

    //========================================

    if(pf4.apply_topology_groups(rule) == false)
    {
        if(false)
        {
            for(int e_i = 0; e_i<pf4.edges.size(); e_i++)
            {
                if(pf4.edges[e_i].topo_group==2)
                    pf4.edges[e_i].topo_group=!rule;
            }
        }
        else
        {
            for(int e_i = 0; e_i<pf4.edges.size(); e_i++)
            {
                if(pf4.edges[e_i].topo_group==2)
                    pf4.edges[e_i].topo_group=rule;
            }

        }
    }

    pf5.apply_topology_groups(!rule);

    LineHolder nograph;
    for(int f_i=0; f_i<pf4.faces.size();f_i++)
    {
        pf4.calc_loops4(f_i,nograph);
        for(int p_i=0; p_i< pf4.faces[f_i].loops.size(); p_i++)
            pf4.set_loop_solid(f_i,p_i);
    }

    for(int f_i=0; f_i<pf5.faces.size();f_i++)
    {
        pf5.calc_loops4(f_i,nograph);
    }



    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        pf4.sort_loops(f_i);
    }

     for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
        {
            pf4.faces[f_i].loops[p_i].topo_group = LOOP_UNDEF;
        }

    num=0;

    {
        //cull loops which are kissing
        for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        {
            core::plane3df f_plane = core::plane3df(pf4.faces[f_i].m_center,pf4.faces[f_i].m_normal);

            if(pf4.faces[f_i].loops.size()>0)
            {
                for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
                {
                    if(pf4.faces[f_i].loops[p_i].vertices.size()>0)
                    {
                        for(int f_j=0; f_j < pf5.faces.size(); f_j++)
                        {
                            if(fabs(f_plane.getDistanceTo(pf5.faces[f_j].m_center))<0.001 &&
                               fabs(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal))>0.999)
                                for(int p_j=0; p_j < pf5.faces[f_j].loops.size(); p_j++)
                                {
                                    //std::cout<<"loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                    if(pass_no == CLIP_FIRST_PASS)
                                    {
                                        if(is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j]))
                                     //  && pf4.faces[f_i].loops[p_i].type==LOOP_INNER)
                                        {

                                            if(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0 && pf4.faces[f_i].loops[p_i].type==LOOP_OUTER)
                                            {

                                            }
                                            else if(pf4.faces[f_i].loops[p_i].type==LOOP_INNER)
                                            {
                                                //std::cout<<"1 hollow loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                                pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                                                num++;
                                                goto LoopHasBeenCulled;
                                            }
                                        }
                                    }
                                    else if(pass_no == CLIP_SECOND_PASS)
                                    {
                                        if( pf4.faces[f_i].loops[p_i].type==LOOP_INNER &&
                                            (is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j]) ||
                                             is_contained_loop(pf4, pf4.faces[f_i].loops[p_i],pf5,f_j,p_j)))
                                        {
                                            if(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
                                            {
                                                //std::cout<<"2 solid loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                                pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                                                num++;
                                                goto LoopHasBeenCulled;
                                            }
                                            else
                                            {
                                                //std::cout<<"2 hollow loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                                pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                                                num++;
                                                goto LoopHasBeenCulled;
                                            }
                                        }
                                        else if( pf4.faces[f_i].loops[p_i].type==LOOP_OUTER && pf4.faces[f_i].loops[p_i].depth%2 == 1 &&
                                                (is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j]) ||
                                             is_contained_loop(pf4, pf4.faces[f_i].loops[p_i],pf5,f_j,p_j)) &&
                                                pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
                                        {
                                                //std::cout<<"2 solid outer loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                                pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                                                num++;
                                                goto LoopHasBeenCulled;
                                        }
                                    }
                                }
                        }
                    }
                    LoopHasBeenCulled:
                    int z=0;
                }
            }
        }
        std::cout<<" "<<num<<" kissing loops\n";
    }

    //if(false)
    {
        for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        {
            poly_face f;
            f.m_normal = pf4.faces[f_i].m_normal;
            f.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;

            for(int p_i=0; p_i<pf4.faces[f_i].loops.size(); p_i++)
            {
                if(pf4.faces[f_i].loops[p_i].vertices.size() > 0 && pf4.faces[f_i].loops[p_i].type == LOOP_INNER)
                {
                    poly_loop new_loop;
                    for(int v_i : pf4.faces[f_i].loops[p_i].vertices)
                    {
                        int v = ret.get_point_or_add(pf4.vertices[v_i].V);
                        new_loop.vertices.push_back(v);
                        f.addVertex(v);
                    }

                    if(pf4.faces[f_i].loops[p_i].topo_group == LOOP_UNDEF)
                    {
                        std::vector<int> tempv = pf4.faces[f_i].loops[p_i].vertices;
                        tempv.push_back(tempv[0]);

                        bool exclude=false;
                        bool include=false;


                        for(int i=0; i < tempv.size()-1; i++)
                        {
                            core::vector3df v0=pf4.vertices[tempv[i]].V;
                            core::vector3df v1=pf4.vertices[tempv[i+1]].V;

                            if(pf4.edges[pf4.get_edge_or_add(tempv[i],tempv[i+1],!rule)].topo_group == !rule )
                            {
                                exclude = true;
                            }
                            else if(pf4.edges[pf4.get_edge_or_add(tempv[i],tempv[i+1],!rule)].topo_group == rule )
                            {
                                include = true;
                            }
                            int new_e = ret.get_edge_or_add(ret.get_point_or_add(v0),ret.get_point_or_add(v1),0);
                            f.addEdge(new_e);
                        }

                        if(exclude && !include )
                        {
                        //    std::cout<<"hollow\n";
                            pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                        }
                        else if(include && !exclude )
                        {
                        //    std::cout<<"solid\n";
                            pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                        }
                        else if((!exclude && !include ) || (exclude && include ))
                        {
                            std::cout<<"odd loop: "<<f_i<<", "<<p_i<<" ";
                            core::vector3df v0=pf4.vertices[tempv[0]].V;
                            core::vector3df v1;
                            int e_i = pf4.find_edge(tempv[0],tempv[1]);

                            if(e_i != -1 && pf4.get_facial_point3(f_i,e_i,p_i,v1,nograph))
                            {
                                int res = pf2.classify_point(v0,v1);
                                if (res == rule)//  || res == TOP_UNDEF)
                                {
                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                                    std::cout<<" solid\n";
                                    exclude = false;
                                }
                                else
                                {
                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                                    std::cout<<" hollow\n";
                                    exclude = false;
                                }
                            }
                            else  pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                        }
                    }

                    new_loop.type = pf4.faces[f_i].loops[p_i].type;
                    new_loop.depth = pf4.faces[f_i].loops[p_i].depth;
                    new_loop.topo_group = pf4.faces[f_i].loops[p_i].topo_group;
                    ret.calc_loop_bbox(f,new_loop);

                    if(new_loop.topo_group != LOOP_UNDEF)
                    {
                        if(pass_no == CLIP_SECOND_PASS && new_loop.topo_group == LOOP_SOLID)
                        {
                            bool b=false;
                            for(int p_j=0;p_j<pf4.faces[f_i].loops.size();p_j++)
                                if(pf4.faces[f_i].loops[p_j].type==LOOP_OUTER  && pf4.faces[f_i].loops[p_j].topo_group==LOOP_SOLID &&
                                   is_identical_loop(pf4,pf4.faces[f_i].loops[p_j],ret,new_loop))
                                   {
                                        b=true;
                                   }
                            if(!b)
                                f.loops.push_back(new_loop);
                        }
                        else if(pass_no == CLIP_FIRST_PASS)
                        {
                            f.loops.push_back(new_loop);
                        }
                    }
                }
                else if(pf4.faces[f_i].loops[p_i].vertices.size() > 0 && pf4.faces[f_i].loops[p_i].type == LOOP_OUTER
                        && pf4.faces[f_i].loops[p_i].depth%2 == 1 && pf4.faces[f_i].loops[p_i].topo_group != LOOP_SOLID)
                {
                    poly_loop new_loop;
                    for(int v_i : pf4.faces[f_i].loops[p_i].vertices)
                    {
                        int v = ret.get_point_or_add(pf4.vertices[v_i].V);
                        new_loop.vertices.push_back(v);
                        f.addVertex(v);
                    }

                    new_loop.type = pf4.faces[f_i].loops[p_i].type;
                    new_loop.depth = pf4.faces[f_i].loops[p_i].depth;
                    new_loop.topo_group = LOOP_HOLLOW;
                    new_loop.type = LOOP_INNER;
                    ret.calc_loop_bbox(f,new_loop);
                    f.loops.push_back(new_loop);
                }
            }

            ret.calc_center(f);
            if(f.loops.size() > 0)
            {
                ret.faces.push_back(f);
            }
        }

        if(pass_no == CLIP_SECOND_PASS)
        {
            ret.reduce_edges_vertices2();
            ret.recalc_bbox_and_loops();
        }
        else
        {
            ret.reduce_edges_vertices2();

            if(ret.vertices.size()>0)
            {
                ret.bbox.reset(ret.vertices[0].V);
                for(poly_vert v : ret.vertices)
                    ret.bbox.addInternalPoint(v.V);
            }
        }
    }

    return ret;
}
*/


/*
void do_merge_faces(polyfold& pf)
{
    int n_faces_merged=0;
    try_again_merge_faces:

    polyfold positive;
    polyfold negative;

    positive.vertices = pf.vertices;
    negative.vertices = pf.vertices;

    for(int f_i = 0; f_i <pf.faces.size(); f_i++)
    {
        poly_face f_p;
        poly_face f_n;

        f_p.m_center        = pf.faces[f_i].m_center;
        f_p.m_normal        = pf.faces[f_i].m_normal;
        f_p.bFlippedNormal  = pf.faces[f_i].bFlippedNormal;
        f_n.m_center        = pf.faces[f_i].m_center;
        f_n.m_normal        = pf.faces[f_i].m_normal;
        f_n.bFlippedNormal  = pf.faces[f_i].bFlippedNormal;

        for(int f_j = f_i+1; f_j <pf.faces.size(); f_j++)
        {
            core::plane3df f_plane = core::plane3df(pf.faces[f_i].m_center,pf.faces[f_i].m_normal);

            if(fabs(f_plane.getDistanceTo(pf5.faces[f_j].m_center))<0.001 &&
               pf.faces[f_i].m_normal.dotProduct(pf.faces[f_j].m_normal) > 0.999)
            {
                for(int p=0; p < pf.faces[f_j].loops.size(); p++)
                {
                    if(pf.faces[f_j].loops[p].vertices.size()>0)
                    {
                        if(pf.faces[f_j].loops[p].topo_group == LOOP_SOLID)
                            f_p.loops.push_back(pf.faces[f_j].loops[p]);
                        else if(pf.faces[f_j].loops[p].topo_group == LOOP_HOLLOW)
                            f_n.loops.push_back(pf.faces[f_j].loops[p]);
                    }
                }
                pf.faces[f_j].loops.clear();
            }
        }

        if(f_p.loops.size()>0 || f_n.loops.size()>0)
        {
            positive.faces.push_back(f_p);
            negative.faces.push_back(f_n);
        }
    }

    pf.faces.clear();

    for(int f_i = 0; f_i <positive.faces.size(); f_i++)
    {
        poly_face f;

        f.m_center        = positive.faces[f_i].m_center;
        f.m_normal        = positive.faces[f_i].m_normal;
        f.bFlippedNormal  = positive.faces[f_i].bFlippedNormal;

        positive.calc_loops4(f_i,nograph);
        positive.sort_loops(f_i);

        negative.calc_loops4(f_i,nograph);
        negative.sort_loops(f_i);

        for(int p=0;p<positive.faces[i].loops.size();p++)
        {
            if(positive.faces[i].loops[p].type==LOOP_OUTER)
            {
                poly_loop loop;
                for(int v_i=0;v_i<positive.faces[i].loops[p].vertices.size();v_i++)
                {
                    core::vector3df V = positive.vertices[positive.faces[i].loops[p].vertices[v_i]].V;
                    int v = result.get_point_or_add(V);
                    loop.vertices.push_back(v);
                }
                loop.topo_group=LOOP_SOLID;
                f.loops.push_back(loop);
            }
        }
     }
}*/
