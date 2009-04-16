#include <iclScene2.h>
#include <iclDrawWidget.h>
#include <iclImg.h>
#include <iclQuick.h>
#include <iclGeomDefs.h>

#include <GL/gl.h>
#include <GL/glu.h>

namespace icl{

  struct Scene2::GLCallback : public ICLDrawWidget3D::GLCallback{
    int cameraIndex;
    Scene2 *parent;
    GLCallback(int cameraIndex,Scene2 *parent):
      cameraIndex(cameraIndex),parent(parent){}
    virtual void draw(){
      parent->render(cameraIndex);
    }
  };

  
  // TODO optimize
  Vec rotate_vector(const Vec &axis, float angle, const Vec &vec){
    angle /= 2;
    float a = cos(angle);
    float sa = sin(angle);
    float b = axis[0] * sa;
    float c = axis[1] * sa;
    float d = axis[2] * sa;
    
    Mat X(a*a + b*b - c*c -d*c , 2*b*c - 2*a*d, 2*a*c + 2*b*d,0,
          2*a*d + 2*d*c, a*a - b*b + c*c -d*d, 2*c*d-2*a*b,0,
          2*b*d - 2*a*c, 2*a*b + 2*c*d, a*a - b*b - c*c +d*d, 0,
          0, 0, 0, 1 );
    
    return X * vec;
  }
  
  struct  Scene2::SceneMouseHandler : public MouseHandler{
    int cameraIndex;
    Scene2 *parent;
    Point32f anchor;
    Camera camSave;
    float speed;
    SceneMouseHandler(int cameraIndex,Scene2 *parent, float maxSceneDim):
      cameraIndex(cameraIndex),parent(parent),speed(maxSceneDim){
    }
    
    virtual void process(const MouseEvent &evt){
      Camera &camera = parent->getCamera(cameraIndex);
      
      if(evt.isPressEvent()){
        anchor = evt.getRelPos();
        camSave = camera;
      }
      if(evt.isDragEvent()){
        Point32f delta = evt.getRelPos()-anchor;
        camera = camSave;      
        if(evt.isLeft()){
          // rotate norm about up (by delta.x)
          Vec norm = camera.getNorm();
          Vec up = camera.getUp();
          norm = rotate_vector(up, delta.x, norm);

          // rotate norm and up about horz (by delta.x)
          norm = rotate_vector(camera.getHorz(), -delta.y, norm);
          up = rotate_vector(camera.getHorz(), -delta.y, up);

          camera.setNorm(norm);
          camera.setUp(up);

        }else if(evt.isMiddle()){
          Vec v = camSave.getUp()*delta.y + camSave.getHorz()*delta.x;
          v[3] = 0;
          camera.translate( v * speed);
        }else if(evt.isRight()){
          camera.translate(camSave.getNorm()*(-delta.y*speed));
          
          // rotate up about norm (by delta.x)
          Vec norm = camera.getNorm();
          Vec up = camera.getUp();
          up = rotate_vector(norm, -3.14*delta.x, up);
          camera.setUp(up);
        }
      }
    }
    void setSpeed(float maxSceneDim){
      speed = maxSceneDim;
    }
  };

  
  struct Scene2::RenderPlugin{
    virtual void color(const GeomColor &c)=0;
    virtual void line(float x1, float y1, float x2, float y2)=0;
    virtual void point(float x, float y, float size)=0;
    virtual void triangle(float x1, float y1, float x2, 
                          float y2, float x3, float y3)=0;    
    virtual void quad(float x1, float y1, float x2, float y2, 
                      float x3, float y3, float x4, float y4)=0;
  };
  
  struct ImgQRenderPlugin : public Scene2::RenderPlugin{
    ImgQ &image;
    ImgQRenderPlugin(ImgQ &image):image(image){}
    virtual void color(const GeomColor &c){
      icl::color(c[0],c[1],c[2],c[3]);
      icl::fill(c[0],c[1],c[2],c[3]);
    }
    virtual void line(float x1, float y1, float x2, float y2){
      icl::line(image,x1,y1,x2,y2);
    }
    virtual void point(float x, float y, float size){
      icl::circle(image,x,y,size);
    }
    virtual void triangle(float x1, float y1, float x2, 
                          float y2, float x3, float y3){
      icl::triangle(image,x1,y1,x2,y2,x3,y3);
    }
    virtual void quad(float x1, float y1, float x2, float y2, 
                      float x3, float y3, float x4, float y4){
      icl::triangle(image,x1,y1,x2,y2,x3,y3);
      icl::triangle(image,x3,y3,x4,y4,x1,y1);
    }
  };
  struct ICLDrawWidgetRenderPlugin : public Scene2::RenderPlugin{
    ICLDrawWidget &w;
    ICLDrawWidgetRenderPlugin(ICLDrawWidget &w):w(w){}
    virtual void color(const GeomColor &c){
      w.color(c[0],c[1],c[2],c[3]);
      w.fill(c[0],c[1],c[2],c[3]);
    }
    virtual void line(float x1, float y1, float x2, float y2){
      w.line(x1,y1,x2,y2);
    }
    virtual void point(float x, float y, float size){
      w.ellipse(x-size/2,y-size/2,size,size);
    }
    virtual void triangle(float x1, float y1, float x2, 
                          float y2, float x3, float y3){
      w.triangle(x1,y1,x2,y2,x3,y3);
    }
    virtual void quad(float x1, float y1, float x2, float y2, 
                      float x3, float y3, float x4, float y4){
      w.quad(x1,y1,x2,y2,x3,y3,x4,y4);
    }
  };
  
  
  Scene2::Scene2():m_lightSimulationEnabled(true){}
  Scene2::~Scene2(){}
  Scene2::Scene2(const Scene2 &scene){
    *this = scene;
  }
  Scene2 &Scene2::operator=(const Scene2 &scene){
    clear();
    m_cameras = scene.m_cameras;
    m_projections = scene.m_projections;
    m_objects.resize(scene.m_objects.size());
    for(unsigned int i=0;i<m_objects.size();++i){
      m_objects[i] = scene.m_objects[i]->copy();
    }
    m_mouseHandlers.resize(scene.m_mouseHandlers.size());
    for(unsigned int i=0;i<m_mouseHandlers.size();++i){
      m_mouseHandlers[i] = new SceneMouseHandler(scene.m_mouseHandlers[i]->cameraIndex,this,scene.m_mouseHandlers[i]->speed);
    }
    
    m_glCallbacks.resize(scene.m_glCallbacks.size());
    for(unsigned int i=0;i<m_glCallbacks.size();++i){
      m_glCallbacks[i] = new GLCallback(scene.m_glCallbacks[i]->cameraIndex,this);
    }
    m_lightSimulationEnabled = scene.m_lightSimulationEnabled;
  }
  
  void Scene2::addCamera(const Camera &cam){
    m_cameras.push_back(cam);
  }
  void Scene2::removeCamera(int index){
    ICLASSERT_RETURN(index > 0 && index < m_cameras.size());
    m_cameras.erase(m_cameras.begin()+index);
  }
  Camera &Scene2::getCamera(int camIndex){
    return m_cameras[camIndex];
  }
  const Camera &Scene2::getCamera(int camIndex) const{
    return m_cameras[camIndex];
  }
  
  void Scene2::render(Img32f &image, int camIndex){
    ImgQRenderPlugin p(image);
    render(p,camIndex);
  }
  void Scene2::render(ICLDrawWidget &widget, int camIndex){
    ICLDrawWidgetRenderPlugin p(widget);
    render(p,camIndex);
  }
  
  void Scene2::addObject(Object2 *object){
    m_objects.push_back(object);
  }
  void Scene2::removeObject(int idx, bool deleteObject){
    Object2 *p = m_objects[idx];
    if(deleteObject) delete p;
    m_objects.erase(m_objects.begin()+idx);
  }
  
  void Scene2::clear(bool camerasToo){
    for(unsigned int i=0;i<m_objects.size();++i){
      delete m_objects[i];
    }

    m_objects.clear();
    m_projections.clear();
    
    if(camerasToo){
      m_cameras.clear();
    }      

    for(unsigned int i=0;i<m_mouseHandlers.size();++i){
      delete m_mouseHandlers[i];
    }
    m_mouseHandlers.clear();
    
    for(unsigned int i=0;i<m_glCallbacks.size();++i){
      delete m_glCallbacks[i];
    }
    m_glCallbacks.clear();

  }

  
  static void update_z(Primitive &p,const std::vector<Vec> &ps){
    switch(p.type){
      case Primitive::line: 
        p.z = (ps[p.a][2]+ps[p.b][2])/2.0; 
        break;
      case Primitive::triangle: 
        p.z = (ps[p.a][2]+ps[p.b][2]+ps[p.c][2])/3.0; 
        break;
      case Primitive::quad: 
        p.z = (ps[p.a][2]+ps[p.b][2]+ps[p.c][2]+ps[p.d][2])/4.0; 
        break;
      default:
        p.z = 0;
        break;
    }
  }
  
  static GeomColor adapt_color_by_light_simulation(const Vec &a, const Vec &b, const Vec &c,
                                                   GeomColor col){
    Vec d = normalize3(cross(a-c,b-c));
    
    float fr = 0.3+fabs(d[0])*0.7;
    float fg = 0.3+fabs(d[1])*0.7;
    float fb = 0.3+fabs(d[2])*0.7;
    col[0]*=fr;
    col[1]*=fg;
    col[2]*=fb;
    return col;
  }

  namespace{
    struct ProjectedObject2{
      ProjectedObject2(Object2 *obj=0, std::vector<Vec>* ps=0):
        obj(obj),projections(ps){}
      Object2 *obj;
      std::vector<Vec> *projections;
      
      // NOT reverse ordering
      bool operator<(const ProjectedObject2 &other) const{
        obj->getZ() < other.obj->getZ();
      }
    };
  }

  void Scene2::render(RenderPlugin &renderer, int camIndex){
    Mutex::Locker l(this);
    ICLASSERT_RETURN(camIndex >= 0 && camIndex < (int)m_cameras.size());
    
    if((int)m_projections.size() <= camIndex){
      m_projections.resize(camIndex+1);
    }
    m_projections[camIndex].resize(m_objects.size());
       
    for(unsigned int i=0;i<m_objects.size();++i){
      Object2 *o = m_objects[i];
      m_cameras[camIndex].project(o->getVertices(),m_projections[camIndex][i]);
      std::vector<Primitive> &ps = o->getPrimitives();
      for(unsigned int j=0;j<ps.size();++j){
        update_z(ps[j],m_projections[camIndex][i]);
      }
      o->updateZFromPrimitives();
      std::sort(ps.begin(),ps.end());
    }
    
    std::vector<ProjectedObject2> sorted(m_objects.size());
    for(unsigned int i=0;i<sorted.size();++i){
      sorted[i] = ProjectedObject2(m_objects[i],&m_projections[camIndex][i]);
    }
    std::sort(sorted.begin(),sorted.end());
      
    for(unsigned int i=0;i<sorted.size();++i){  
      Object2 *o = sorted[i].obj;
      std::vector<Primitive> &primitives = o->getPrimitives();
      std::vector<Vec> &ps = *sorted[i].projections;
      std::vector<Vec> &vx = o->m_vertices;
      for(unsigned int j=0;j<primitives.size();++j){

        Primitive &p  = primitives[j];

        if(!o->isVisible(p.type)) continue;      
        
        if(!m_lightSimulationEnabled){
          renderer.color(p.color);
        }
        switch(p.type){
          case Primitive::line:
            if(m_lightSimulationEnabled){
              renderer.color(p.color);
            }
            renderer.line(ps[p.a][0],ps[p.a][1],ps[p.b][0],ps[p.b][1]);
            break;
          case Primitive::triangle:{

            Vec &a = vx[p.a];
            Vec &b = vx[p.b];
            Vec &c = vx[p.c];
            
            if(m_lightSimulationEnabled){
              renderer.color(adapt_color_by_light_simulation(a,b,c,p.color));
            }

            renderer.triangle(ps[p.a][0],ps[p.a][1],ps[p.b][0],ps[p.b][1],ps[p.c][0],ps[p.c][1]);
            break;
          }case Primitive::quad:{

            Vec &a = vx[p.a];
            Vec &b = vx[p.b];
            Vec &c = vx[p.c];
            Vec &d = vx[p.c];

            if(m_lightSimulationEnabled){
              renderer.color(adapt_color_by_light_simulation(a,b,c,p.color));
            }

            renderer.quad(ps[p.a][0],ps[p.a][1],ps[p.b][0],ps[p.b][1],
                          ps[p.c][0],ps[p.c][1],ps[p.d][0],ps[p.d][1]);
            break;
          }default:
            ERROR_LOG("unsupported primitive type");
        }
      }
      if(o->isVisible(Primitive::vertex)){
        for(unsigned int j=0;j<ps.size();++j){
          renderer.color(o->m_vertexColors[j]);
          renderer.point(ps[j][0],ps[j][1],3);
        }
      }
    }
  }
  
  namespace {
    struct GLMatrix{
      FixedMatrix<float,4,4> mt;
      GLMatrix(const Mat &m):mt(m.transp()){}
      
      operator const float*() const{
        return mt.data();
      }
      
    };
  }
  
  void Scene2::render(int camIndex){
    Mutex::Locker l(this);
    ICLASSERT_RETURN(camIndex >= 0 && camIndex < (int)m_cameras.size());
    Camera &cam = m_cameras[camIndex];

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Vec vc = cam.getPos()+cam.getNorm();
    gluLookAt(cam.getPos()[0],cam.getPos()[1],cam.getPos()[2],
              vc[0],vc[1],vc[2],
              cam.getUp()[0],cam.getUp()[1],cam.getUp()[2]);
    // glLoadMatrixf(GLMatrix(cam.getCoordinateSystemTransformationMatrix()));
    // DEBUG_LOG("modelview matrix:\n" << GLMatrix(cam.getCoordinateSystemTransformationMatrix()).mt);
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(GLMatrix(cam.getProjectionMatrix()));
    

    GLfloat viewport[4];
    glGetFloatv(GL_VIEWPORT,viewport);
    float ar = viewport[2]/viewport[3];
    if(ar>1){
      glScalef(1.0/ar,1,1);
    }else{
      glScalef(1.0,ar,1);
    }
    

    GLenum flags[4] = {GL_DEPTH_TEST,GL_LINE_SMOOTH,GL_POINT_SMOOTH,GL_POLYGON_SMOOTH};
    GLboolean on[4] = {0,0,0,0};
    for(int i=0;i<4;++i){
      glGetBooleanv(flags[i],on+i);
      glEnable(flags[i]);
    }


    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);

    for(unsigned int i=0;i<m_objects.size();++i){
      Object2 *o = m_objects[i];
      std::vector<Vec> &ps = o->m_vertices;
      for(unsigned int j=0;j<o->m_primitives.size();++j){
        Primitive &p = o->m_primitives[j];
        if(!o->isVisible(p.type)) continue; 
        glColor4fv(((p.color)/255.0).begin());
        switch(p.type){
          case Primitive::line:
            glBegin(GL_LINES);
            glVertex3fv(ps[p.a].data());              
            glVertex3fv(ps[p.b].data());
            glEnd();

            break;
          case Primitive::triangle:{
            glBegin(GL_TRIANGLES);
            Vec &a = ps[p.a];
            Vec &b = ps[p.b];
            Vec &c = ps[p.c];
            
            glNormal3fv(normalize(cross(a-c,b-c)).data());

            glVertex3fv(a.data());              
            glVertex3fv(b.data());
            glVertex3fv(c.data());
            glEnd();
            break;
          }case Primitive::quad:{
            glBegin(GL_QUADS);
            Vec &a = ps[p.a];
            Vec &b = ps[p.b];
            Vec &c = ps[p.c];
            Vec &d = ps[p.d];
            
            glNormal3fv(normalize(cross(a-c,b-c)).data());

            glVertex3fv(a.data());              
            glVertex3fv(b.data());
            glVertex3fv(c.data());
            glVertex3fv(d.data());
            glEnd();
            break;
           }default:
            ERROR_LOG("unsupported primitive type");
        }
      }
      glPointSize(3);
      glBegin(GL_POINTS);
      if(o->isVisible(Primitive::vertex)){
        for(unsigned int j=0;j<ps.size();++j){
          glColor3fv(((o->m_vertexColors[j])/255.0).data());
          glVertex3fv(ps[j].data());
        }
      }
      glEnd();
      glPointSize(1);
    }

    for(int i=0;i<4;++i){
      if(!on[i]){
        glDisable(flags[i]);
      }
    }
  }



  MouseHandler *Scene2::getMouseHandler(int camIndex){
    ICLASSERT_RETURN_VAL(camIndex >= 0 && camIndex < (int)m_cameras.size(),0);
    // search for already exsiting mouse handler for given camera
    
    for(unsigned int i=0;i<m_mouseHandlers.size();++i){
      if(m_mouseHandlers[i]->cameraIndex == camIndex){
        m_mouseHandlers[i]->setSpeed(getMaxSceneDim());
        return m_mouseHandlers[i];
      }
    }
    m_mouseHandlers.push_back(new SceneMouseHandler(camIndex,this, getMaxSceneDim()));
    return m_mouseHandlers.back();
  }

  ICLDrawWidget3D::GLCallback *Scene2::getGLCallback(int camIndex){
    ICLASSERT_RETURN_VAL(camIndex >= 0 && camIndex < (int)m_cameras.size(),0);

    // search for already exsiting mouse handler for given camera
    for(unsigned int i=0;i<m_glCallbacks.size();++i){
      if(m_glCallbacks[i]->cameraIndex == camIndex){
        return m_glCallbacks[i];
      }
    }
    m_glCallbacks.push_back(new GLCallback(camIndex,this));
    return m_glCallbacks.back();
  }


  void Scene2::setLightSimulationEnabled(bool enabled){
    m_lightSimulationEnabled = enabled;
  }
  bool Scene2::getLightSimulationEnabled() const{
    return m_lightSimulationEnabled;
  }
  
  float Scene2::getMaxSceneDim() const{
    Range32f rangeXYZ[3]={Range32f::limits(),Range32f::limits(),Range32f::limits()};
    for(int i=0;i<3;++i){
      std::swap(rangeXYZ[i].minVal,rangeXYZ[i].maxVal);
    }
    for(unsigned int i=0;i<m_objects.size();++i){
      Object2 *o = m_objects[i];
      std::vector<Vec> &ps = o->m_vertices;
      for(unsigned int j=0;j<ps.size();++j){
        for(int k=0;k<3;++k){
          if(ps[j][k] < rangeXYZ[k].minVal) rangeXYZ[k].minVal = ps[j][k];
          if(ps[j][k] > rangeXYZ[k].maxVal) rangeXYZ[k].maxVal = ps[j][k];
        }
      }
      return iclMax(iclMax(rangeXYZ[1].getLength(),rangeXYZ[2].getLength()),rangeXYZ[0].getLength());
    }
  }


  
}

