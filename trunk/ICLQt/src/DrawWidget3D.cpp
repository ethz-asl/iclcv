/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/DrawWidget3D.cpp                             **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQt/DrawWidget3D.h>
#include <ICLQt/GLImg.h>

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace icl{

  ICLDrawWidget3D::GLCallback::GLCallback(bool useDisplayList){
    // {{{ open

    m_bUseDisplayList=useDisplayList;
    m_bFirst = true;
   
  } 

  // }}}
  
  void ICLDrawWidget3D::GLCallback::draw_extern(ICLDrawWidget3D *widget){
    // {{{ open
    if(m_bFirst && m_bUseDisplayList){
      m_uiListHandle = glGenLists(1);
      glNewList(m_uiListHandle,GL_COMPILE_AND_EXECUTE);
      draw();
      drawSpecial(widget);
      glEndList();
      m_bFirst = false;

    }
    if(m_bUseDisplayList){
      glCallList(m_uiListHandle);
    }else{
      draw();
      drawSpecial(widget);
    }
  }

  // }}}
 
  struct ICLDrawWidget3D::DrawCommand3D{
    // {{{ open
    virtual ~DrawCommand3D(){}
    virtual void execute()=0;
  };

  // }}}
  
  template<void (*func)(void)>
  struct FunctionCommand3D : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    virtual ~FunctionCommand3D(){}
    virtual void execute(){
      func();
    }
  };

  // }}}

  struct Cube3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    Cube3DDrawCommand(float x,float y,float z, float d):x(x),y(y),z(z),d(d){}
    virtual void execute(){
      static GLfloat n[6][3] = {  
        {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
        {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0} };
      static GLint faces[6][4] = {  
        {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
        {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3} };
      static GLfloat v[8][3];  
      bool first = true;
      if(first){
        first = false;
         v[0][0] = v[1][0] = v[2][0] = v[3][0] = -d;
         v[4][0] = v[5][0] = v[6][0] = v[7][0] = d;
         v[0][1] = v[1][1] = v[4][1] = v[5][1] = -d;
         v[2][1] = v[3][1] = v[6][1] = v[7][1] = d;
         v[0][2] = v[3][2] = v[4][2] = v[7][2] = d;
         v[1][2] = v[2][2] = v[5][2] = v[6][2] = -d;
      }
      glTranslatef(x,y,z);
      for(int i=0;i<6;i++) {
        glBegin(GL_QUADS);
        glNormal3fv(&n[i][0]);
        glVertex3fv(&v[faces[i][0]][0]);
        glVertex3fv(&v[faces[i][1]][0]);
        glVertex3fv(&v[faces[i][2]][0]);
        glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
      }
      glTranslatef(-x,-y,-z);
    }
    float x,y,z,d;
  };

  // }}}
  struct SuperCube3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    SuperCube3DDrawCommand(float x,float y,float z, float d):x(x),y(y),z(z),d(d){}

    GLfloat* xx (GLfloat x,GLfloat y, GLfloat z){
      GLfloat* back = new GLfloat[3];
      back[0]=x;
      back[1]=y;
      back[2]=z;
      return back;
    }
    
    virtual void execute(){
      static GLfloat** p = new GLfloat* [40];
      static GLfloat a = 1.0f;
      static GLfloat b = 0.8f;
      static bool first = true;
      if(first){
        first = false;
        //E aussen
        p[0] = xx(-a,a,a);p[1]= xx(-a,-a,a);p[2]= xx(a,-a,a);p[3]= xx(a,a,a);	
        //E innen
        p[4] = xx(-b,b,a);p[5]= xx(-b,-b,a);p[6]= xx(b,-b,a);p[7]= xx(b,b,a);
	
        //F aussen
        p[8] = xx(-a,a,-a);p[9]= xx(-a,-a,-a);p[10]= xx(a,-a,-a);p[11]= xx(a,a,-a);
        //F innne	
        p[12] = xx(-b,b,-a);p[13]= xx(-b,-b,-a);p[14]= xx(b,-b,-a);p[15]= xx(b,b,-a);


        //C innen
        p[16] = xx(-b,a,-b);p[17]= xx(-b,a,b);p[18]= xx(b,a,b);p[19]= xx(b,a,-b);

        //D innen
        p[20] = xx(-b,-a,-b);p[21]= xx(-b,-a,b);p[22]= xx(b,-a,b);p[23]= xx(b,-a,-b);
	
        //A innen
        p[24] = xx(a,b,-b);p[25]= xx(a,b,b);p[26]= xx(a,-b,b);p[27]= xx(a,-b,-b);

        //B innen
        p[28] = xx(-a,b,-b);p[29]= xx(-a,b,b);p[30]= xx(-a,-b,b);p[31]= xx(-a,-b,-b);

        //Innen Wuerfel oben
        p[32] = xx(b,b,b);p[33]= xx(-b,b,b);p[34]= xx(-b,-b,b);p[35]= xx(b,-b,b);

        //Innen Wuerfel unten
        p[36] = xx(b,b,-b);p[37]= xx(-b,b,-b);p[38]= xx(-b,-b,-b);p[39]= xx(b,-b,-b);
      }
    
      static GLfloat nml[][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1},};
      static GLint idx[192] ={0,4,5,1,1,5,6,2,6,2,3,7,3,7,4,0,                     // upper vetices
                              8,12,15,11,15,11,10,14,10,14,13,9,13,9,8,12,         // lower vertices
                              8,16,17,0,17,0,3,18,3,18,19,11,19,11,8,16,           // front vertices
                              1,21,22,2,22,2,10,23,23,10,9,20,9,20,21,1,           // back vertices
                              3,25,26,2,26,2,10,27,27,10,11,24,11,24,25,3,         // right vertices
                              8,28,29,0,0,29,30,1,1,30,31,9,31,9,8,28,             // left vertices
                              36,24,27,39,39,27,26,35,26,35,32,25,32,25,24,36,     // inner fron right
                              28,37,33,29,33,29,30,34,30,34,38,31,38,31,28,37,     // inner back left
                              16,37,33,17,33,17,18,32,18,32,36,19,36,19,16,37,     // inner front left
                              34,21,22,35,22,35,39,23,39,23,20,38,20,38,34,21,     // inner back right
                              5,34,35,6,35,6,7,32,7,32,33,4,33,4,5,34,             // inner uppper
                              37,12,13,38,13,38,39,14,39,14,15,36,15,36,37,12      // inner lower
      };
      static GLint normalIdx[] = {4,4,4,4,5,5,5,5,2,2,2,2,3,3,3,3,0,0,0,0,1,1,1,1,
                                  4,2,5,3,3,5,2,4,0,5,1,4,5,1,4,0,2,1,3,0,0,2,1,3};
      glTranslatef(x,y,z);
      glScalef(d,d,d);
      glBegin(GL_QUADS);
      for(int i=0;i<192;i+=4){
        glNormal3fv(nml[normalIdx[(int)(i/4)]]);
        glVertex3fv(p[idx[i]]);			
        glVertex3fv(p[idx[i+1]]);			
        glVertex3fv(p[idx[i+2]]);				
        glVertex3fv(p[idx[i+3]]);		
      }
      glEnd();	
      glTranslatef(-x,-y,-z);
      glScalef(1/d,1/d,1/d);
    }
    float x,y,z,d;
  };

  // }}}
  struct ImageCube3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    ImageCube3DDrawCommand(float x,float y,float z, float d, const ImgBase *image):x(x),y(y),z(z),d(d){
      glimages.push_back(new GLImg(image));
    }
    ImageCube3DDrawCommand(float x,float y,float z, float d, const ImgBase *images[6]):x(x),y(y),z(z),d(d){
      for(int i=0;i<6;++i){
        glimages.push_back(new GLImg(images[i]));
      }
    }

    ~ImageCube3DDrawCommand(){
      for(unsigned int i=0;i<glimages.size();++i){
        delete glimages[i];
      }
    }
    virtual void execute(){
      
      GLboolean wasLightingEnabled;
      glGetBooleanv(GL_LIGHTING,&wasLightingEnabled);
      if(wasLightingEnabled){
        glDisable(GL_LIGHTING);
      }

      d/=2;
      
      float A[3] = {x+d, y+d, z+d};
      float B[3] = {x+d, y-d, z+d};
      float C[3] = {x-d, y-d, z+d};
      float D[3] = {x-d, y+d, z+d};

      float E[3] = {x+d, y+d, z-d};
      float F[3] = {x+d, y-d, z-d};
      float G[3] = {x-d, y-d, z-d};
      float H[3] = {x-d, y+d, z-d};

      if(glimages.size() == 1){
        glimages[0]->draw3D(A,B,E);
        glimages[0]->draw3D(A,D,E);
        glimages[0]->draw3D(A,D,B);
        
        glimages[0]->draw3D(G,H,C);
        glimages[0]->draw3D(G,H,F);
        glimages[0]->draw3D(G,F,C);
      }else{
        glimages[0]->draw3D(A,B,E);
        glimages[1]->draw3D(A,D,E);
        glimages[2]->draw3D(A,D,B);
        
        glimages[3]->draw3D(G,H,C);
        glimages[4]->draw3D(G,H,F);
        glimages[5]->draw3D(G,F,C);
      }
      d*=2;

      if(wasLightingEnabled){
        glEnable(GL_LIGHTING);
      }
    }
    float x,y,z,d;
    std::vector<GLImg*> glimages;
  };

  // }}}
  struct Image3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    Image3DDrawCommand(float cX,float cY,float cZ,float aX, float aY,float aZ,float bX,float bY,float bZ, const ImgBase *image){
      c[0]=cX; c[1]=cY; c[2]=cZ;
      a[0]=aX; a[1]=aY; a[2]=aZ;
      b[0]=bX; b[1]=bY; b[2]=bZ;
      glimage = new GLImg(image);
    }
    ~Image3DDrawCommand(){
      delete glimage;
    }
    virtual void execute(){
      GLboolean wasLightingEnabled;
      glGetBooleanv(GL_LIGHTING,&wasLightingEnabled);
      if(wasLightingEnabled){
        glDisable(GL_LIGHTING);
      }
      
      glimage->draw3D(c,a,b);
      
      if(wasLightingEnabled){
        glEnable(GL_LIGHTING);
      }    
    }
    float c[3],a[3],b[3];
    GLImg *glimage;
  };

  // }}}
  struct Color3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    Color3DDrawCommand(float r,float g,float b, float a):r(r),g(b),b(b),a(a){}
    virtual void execute(){
      glColor4f(r,g,b,a);
    }
    float r,g,b,a;
  };

  // }}}
  struct Clear3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    virtual void execute(){
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
  };

  // }}}
  struct LookAt3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open
    LookAt3DDrawCommand(float ex, float ey, float ez, float cx, float cy, float cz, float ux, float uy, float uz):
      ex(ex),ey(ey),ez(ez),cx(cx),cy(cy),cz(cz),ux(ux),uy(uy),uz(uz){}
    virtual void execute(){
      glMatrixMode(GL_MODELVIEW);
      gluLookAt(ex,ey,ez,cx,cy,cz,ux,uy,uz);
    }
    float ex,ey,ez,cx,cy,cz,ux,uy,uz;
  };

  // }}}
  struct Frustum3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open
    Frustum3DDrawCommand(float left,float right,float bottom,float top,float zNear,float zFar):
      left(left),right(right),bottom(bottom),top(top),zNear(zNear),zFar(zFar){}
    virtual void execute(){
      glMatrixMode(GL_PROJECTION);
      glFrustum(left,right,bottom,top,zNear,zFar);
    }
    float left,right,bottom,top,zNear,zFar;
  };

  // }}} 
  struct ViewPort3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open
    ViewPort3DDrawCommand(float x,float y, float width,float height):
      x(x),y(y),width(width),height(height){}
    virtual void execute(){
      glViewport(x,y,width,height);
    }
    float x,y,width,height;
  };

  // }}} 
  struct Rotate3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    Rotate3DDrawCommand(float rx,float ry,float rz):rx(rx),ry(ry),rz(rz){}
    virtual void execute(){
      glRotatef(rz,0,0,1);
      glRotatef(rx,1,0,0);
      glRotatef(ry,0,1,0);
    }
    float rx,ry,rz;
  };

  // }}}
  struct Translate3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    Translate3DDrawCommand(float tx,float ty,float tz):tx(tx),ty(ty),tz(tz){}
    virtual void execute(){
      glTranslatef(tx,ty,tz);
    }
    float tx,ty,tz;
  };

  // }}}
  struct Scale3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    Scale3DDrawCommand(float tx,float ty,float tz):tx(tx),ty(ty),tz(tz){}
    virtual void execute(){
      glScalef(tx,ty,tz);
    }
    float tx,ty,tz;
  };

  // }}}
  struct MultMat3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    MultMat3DDrawCommand(float *mat){
      memcpy(this->mat,mat,16*sizeof(float));
    }
    virtual void execute(){
      glMultMatrixf(mat);
    }
    float mat[16];
  };

  // }}}
  struct SetMat3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    SetMat3DDrawCommand(float *mat){
      memcpy(this->mat,mat,16*sizeof(float));
    }
    virtual void execute(){
      glLoadIdentity();
      glMultMatrixf(mat);
    }
    float mat[16];
  };

  // }}}
  struct MatrixMode3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    MatrixMode3DDrawCommand(bool projection){
      m = projection ? GL_PROJECTION : GL_MODELVIEW;
    }
    virtual void execute(){
      glMatrixMode(m);
    }
    
    GLenum m;
  };

  // }}}
  struct Perspective3DCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    Perspective3DCommand(float angle,float aspect,float near,float far):
      angle(angle),aspect(aspect),nearVal(near),farVal(far){}
    virtual void execute(){
      glMatrixMode(GL_PROJECTION);
      gluPerspective(angle,aspect,nearVal,farVal);
    }
	float angle;
	float aspect;
	float nearVal;
	float farVal;
  };

  // }}}
  struct ID3DCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    virtual void execute() { glLoadIdentity(); }
  };

  // }}}
  struct CallbackFunc3DCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    CallbackFunc3DCommand(ICLDrawWidget3D::GLCallbackFunc func, void *data):func(func),data(data){}
    virtual void execute(){
      func(data);
    }
    ICLDrawWidget3D::GLCallbackFunc func;
    void *data;
  };

  // }}}
  struct Callback3DCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open
    Callback3DCommand(ICLDrawWidget3D *widget,ICLDrawWidget3D::GLCallback *cb):
      cb(cb),widget(widget){}
    virtual void execute(){
      cb->draw_extern(widget);
    }
    ICLDrawWidget3D::GLCallback *cb;
    ICLDrawWidget3D *widget;
    
  };

  // }}}
  struct SmartCallback3DCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    SmartCallback3DCommand(ICLDrawWidget3D *widget,SmartPtr<ICLDrawWidget3D::GLCallback> &smartCB):
      m_spCB(smartCB),widget(widget){}
    virtual void execute(){
      m_spCB->draw_extern(widget);
    }
    SmartPtr<ICLDrawWidget3D::GLCallback> m_spCB;
    ICLDrawWidget3D *widget;
  };

  // }}}
  

    /*
        - property "diffuse" value "on|off" (default is on)
        - property "ambient" value "on|off" (default is off)
        - property "specular" value "on|off" (default is off)
        - property: "light-X" value "on|off"
        - property: "light-X-pos" value "{x,y,z,h}"
        - property: "light-X-ambient" value "{r,g,b,factor}"
        - property: "light-X-diffuse" value "{r,g,b,factor}"
        - property: "light-X-specular" value "{r,g,b,factor}"
    */

  struct ICLDrawWidget3D::Properties{
    struct Light{
      Light():on(false){
        std::fill(position,position+4,0);
        std::fill(ambient,ambient+4,0);
        std::fill(diffuse,diffuse+4,0);
        std::fill(specular,specular+4,0);
      }
      bool on;
      GLfloat position[4];
      GLfloat ambient[4];
      GLfloat diffuse[4];
      GLfloat specular[4];
    };
    
    Light lights[4];
    
    bool ambientOn;
    bool diffuseOn;
    bool specularOn;
    bool lightingOn;
    bool colorMaterialOn;
    bool depthTestOn;
    
    Properties():ambientOn(false),diffuseOn(true),
                 specularOn(false),lightingOn(true),
                 colorMaterialOn(true),depthTestOn(true){
      static const GLfloat d[] = {1.0, 1.0, 0.8, 1.0};  /* White diffuse light. */
      static const GLfloat p[] = {1.0, 1.0, 1.0, 0.0};  /* Infinite light location. */

      lights[0].on = true;
      std::copy(d,d+4,lights[0].diffuse);
      std::copy(p,p+4,lights[0].position);
    }

    void activate(bool b, GLenum e){
      if(b) glEnable(e);
      else glDisable(e);
    }
    
    void initGL(){
      activate(lightingOn,GL_LIGHTING);
      activate(colorMaterialOn,GL_COLOR_MATERIAL);
      activate(depthTestOn,GL_DEPTH_TEST);

      static const GLenum ls[4]={
        GL_LIGHT0,GL_LIGHT1,GL_LIGHT2,GL_LIGHT3
      };
      
      for(int i=0;i<4;++i){
        glLightfv(ls[i],GL_POSITION,lights[i].position);
        static const GLfloat null[4] = {0,0,0,0};
        if(lights[i].on){
          glEnable(ls[i]);
          if(diffuseOn){
            glLightfv(ls[i],GL_DIFFUSE,lights[i].diffuse);
          }else{
            glLightfv(ls[i],GL_DIFFUSE,null);
          }
          
          if(ambientOn){
            glLightfv(ls[i],GL_AMBIENT,lights[i].ambient);
          }else{
            glLightfv(ls[i],GL_AMBIENT,null);
          }

          if(specularOn){
            glLightfv(ls[i],GL_SPECULAR,lights[i].specular);
          }else{
            glLightfv(ls[i],GL_SPECULAR,null);
          }
        }else{
          glDisable(ls[i]);
        }
      }
    }
  };
  
  ICLDrawWidget3D::ICLDrawWidget3D(QWidget *parent):ICLDrawWidget(parent),m_properties(new Properties){
    // {{{ open

  }

  // }}}
  ICLDrawWidget3D::~ICLDrawWidget3D(){
    // {{{ open
    delete m_properties;
  }

  // }}}

  void ICLDrawWidget3D::customPaintEvent(PaintEngine *e){
    // {{{ open
    lock();
    glClear(GL_DEPTH_BUFFER_BIT);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    

    
    m_properties->initGL();
    
    //    glEnable(GL_DEPTH_TEST);    
    // GLfloat light_diffuse[] = {1.0, 1.0, 0.8, 1.0};  /* White diffuse light. */
    // GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};  /* Infinite light location. */

    //glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    // glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    //glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    //glEnable(GL_LIGHT0);
    
    //glEnable(GL_LIGHTING);

    //glEnable(GL_COLOR_MATERIAL);
    //    

    glMatrixMode(GL_PROJECTION);
    gluPerspective( 45,  float(width())/height(), 0.1, 100);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(0, 0, -1,   // pos
              0, 0,  0,   // view center point
              1, 0,  0 );// up vector
    

#ifdef DO_NOT_USE_GL_VISUALIZATION
    ERROR_LOG("3D Visualization is not supported without OpenGL!");
#else
    for(unsigned int i=0;i<m_vecCommands3D.size();++i){
      m_vecCommands3D[i]->execute();
    }
    
#endif
    
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    unlock();

    ICLDrawWidget::customPaintEvent(e);
  }

  // }}}
  void ICLDrawWidget3D::reset3D(){
    // {{{ open
    for(unsigned int i=0;i<m_vecCommands3D.size();++i){
      delete m_vecCommands3D[i];
    }
    m_vecCommands3D.clear();
  }

  // }}}
  
  
  void ICLDrawWidget3D::clear3D(){
    m_vecCommands3D.push_back(new Clear3DDrawCommand);
  }
  void ICLDrawWidget3D::cube3D(float x,float y, float z, float d){
    m_vecCommands3D.push_back(new Cube3DDrawCommand(x,y,z,d));
  } 
  void ICLDrawWidget3D::supercube3D(float x,float y, float z, float d){
    m_vecCommands3D.push_back(new SuperCube3DDrawCommand(x,y,z,d));
  }
  void ICLDrawWidget3D::imagecube3D(float cx, float cy, float cz, float d, const ImgBase *image){
    m_vecCommands3D.push_back(new ImageCube3DDrawCommand(cx,cy,cz,d,image));
  }
  void ICLDrawWidget3D::imagecube3D(float cx, float cy, float cz, float d, const ImgBase *images[6]){
    m_vecCommands3D.push_back(new ImageCube3DDrawCommand(cx,cy,cz,d,images));
  }
  void ICLDrawWidget3D::image3D(float cX,float cY,float cZ,float aX, float aY,float aZ,float bX,float bY,float bZ, const ImgBase *image){
    m_vecCommands3D.push_back(new Image3DDrawCommand(cX,cY,cZ,aX,aY,aZ,bX,bY,bZ,image));
  }
  void ICLDrawWidget3D::color3D(float r, float g, float b, float a){
    m_vecCommands3D.push_back(new Color3DDrawCommand(r,g,b,a));
  }
  void ICLDrawWidget3D::lookAt3D(float eyeX, float eyeY, float eyeZ, float cX, float cY, float cZ, float upX, float upY, float upZ){
    m_vecCommands3D.push_back(new LookAt3DDrawCommand(eyeX,eyeY,eyeZ,cX,cY,cZ,upX,upY,upZ));
  }
  void ICLDrawWidget3D::frustum3D(float left,float right,float bottom, float top,float zNear,float zFar){
    m_vecCommands3D.push_back(new Frustum3DDrawCommand(left,right,bottom,top,zNear,zFar));
  }
  void ICLDrawWidget3D::viewport3D(float x,float y, float width, float height){
    m_vecCommands3D.push_back(new ViewPort3DDrawCommand(x,y,width,height));
  }
  void ICLDrawWidget3D::rotate3D(float rx, float ry, float rz){
    m_vecCommands3D.push_back(new Rotate3DDrawCommand(rx,ry,rz));
  }
  void ICLDrawWidget3D::translate3D(float tx, float ty, float tz){
    m_vecCommands3D.push_back(new Translate3DDrawCommand(tx,ty,tz));
  }
  void ICLDrawWidget3D::scale3D(float sx, float sy,float sz){
    m_vecCommands3D.push_back(new Scale3DDrawCommand(sx,sy,sz));
  }
  void ICLDrawWidget3D::setMat3D(float *mat){
    m_vecCommands3D.push_back(new SetMat3DDrawCommand(mat));
  }
  void ICLDrawWidget3D::modelview(){
    m_vecCommands3D.push_back(new MatrixMode3DDrawCommand(false));
  }
  void ICLDrawWidget3D::projection(){
    m_vecCommands3D.push_back(new MatrixMode3DDrawCommand(true));
  }
  
   #ifdef ICL_SYSTEM_WINDOWS
 // for some reason, MinGW does not want to use glPopMatrix as template parameter 
  void glPopMatrix2(void){
	glPopMatrix();
  }
  void glPushMatrix2(void){
	glPushMatrix();
  }
  void ICLDrawWidget3D::pushMatrix(){
    m_vecCommands3D.push_back(new FunctionCommand3D<glPushMatrix2>);
  }
  void ICLDrawWidget3D::popMatrix(){
    m_vecCommands3D.push_back(new FunctionCommand3D<glPopMatrix2>);
  }
  #else
  void ICLDrawWidget3D::pushMatrix(){
    m_vecCommands3D.push_back(new FunctionCommand3D<glPushMatrix>);
  }
  void ICLDrawWidget3D::popMatrix(){
    m_vecCommands3D.push_back(new FunctionCommand3D<glPopMatrix>);
  }
  #endif  
  
  void ICLDrawWidget3D::perspective(float angle, float aspect, float nearVal, float farVal){
    m_vecCommands3D.push_back(new Perspective3DCommand(angle,aspect,nearVal,farVal));
  }
  void ICLDrawWidget3D::id(){
    m_vecCommands3D.push_back(new ID3DCommand);
  }
  void ICLDrawWidget3D::callback(ICLDrawWidget3D::GLCallbackFunc func, void *data){
    m_vecCommands3D.push_back(new CallbackFunc3DCommand(func,data));
  }
  void ICLDrawWidget3D::callback(ICLDrawWidget3D::GLCallback *cb){
    m_vecCommands3D.push_back(new Callback3DCommand(this,cb));
  }
  void ICLDrawWidget3D::callback(SmartPtr<ICLDrawWidget3D::GLCallback> smartCB){
    m_vecCommands3D.push_back(new SmartCallback3DCommand(this,smartCB));
  }

  void ICLDrawWidget3D::setProperty(const std::string &property, const std::string &value){
    if(property == "diffuse"){
      if(value == "on"){
        m_properties->diffuseOn = true;
      }else if(value == "off"){
        m_properties->diffuseOn = false;
      }else{
        ERROR_LOG("unable to set property diffuse to value " << value << "(value can be 'on' or 'off')");
        return;
      }
    }else if(property == "ambient"){
      if(value == "on"){
        m_properties->ambientOn = true;
      }else if(value == "off"){
        m_properties->ambientOn = false;
      }else{
        ERROR_LOG("unable to set property ambient to value " << value << "(value can be 'on' or 'off')");
        return;
      }
    }else if(property == "specular"){
      if(value == "on"){
        m_properties->specularOn = true;
      }else if(value == "off"){
        m_properties->specularOn = false;
      }else{
        ERROR_LOG("unable to set property specular to value " << value << "(value can be 'on' or 'off')");
        return;
      }
    }else if(property == "lighting"){
      if(value == "on"){
        m_properties->lightingOn = true;
      }else if(value == "off"){
        m_properties->lightingOn = false;
      }else{
        ERROR_LOG("unable to set property lighting to value " << value << "(value can be 'on' or 'off')");
        return;
      }
    }else if(property == "color-material"){
      if(value == "on"){
        m_properties->colorMaterialOn = true;
      }else if(value == "off"){
        m_properties->colorMaterialOn = false;
      }else{
        ERROR_LOG("unable to set property color-material to value " << value << "(value can be 'on' or 'off')");
        return;
      }
    }else if(property.length() > 6 && property.substr(0,6) == "light-"){
      std::vector<std::string> ts = tok(property.substr(6),"-");
      if(!ts.size()){
        ERROR_LOG("invalid property " << property);
        return;
      }
      
      int i = parse<int>(ts.front());
      if(i < 0 || i> 4) {
        ERROR_LOG("unable to set property " << property << " to value " << value << "(invalid light index)");
        return;
      }
      Properties::Light &l = m_properties->lights[i];

      if(ts.size() == 1){
        if(value == "on"){
          l.on = true;
        }else if(value == "off"){
          l.on = false;
        }else{
          ERROR_LOG("unable to set property light-" << i << " to value " << value << "(value can be 'on' or 'off')");
          return;
        }
      }else if(ts.size() == 2){
        float *dst = 0;
        if(ts[1] == "ambient"){
          dst = l.ambient;
        }else if(ts[1] == "diffuse"){
          dst = l.diffuse;
        }else if(ts[1] == "specular"){
          dst = l.specular;
        }else if(ts[1] == "pos"){
          dst = l.position;
        }else{
          ERROR_LOG("invalid property " << property);
          return;
        }
        FixedMatrix<float,1,4> color = parse<FixedMatrix<float,1,4> >(value);
        std::copy(color.begin(),color.end(),dst);
      }else{
        ERROR_LOG("invalid property: " << property);
        return;
      }
    }
  }


}
