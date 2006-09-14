#include "GLPaintEngine.h"
#include <QGLWidget>
#include <Img.h>
#include <QFontMetrics>
#include <QPainter>



namespace icl{

  GLPaintEngine::GLPaintEngine(QGLWidget *widget):
    // {{{ open

    m_poWidget(widget), m_oFont(QFont("Arial",30)){
    
    widget->makeCurrent();
    
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
    QSize sz(widget->size());
    glViewport(0, 0, sz.width(), sz.height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, sz.width(), sz.height(), 0, -999999, 999999);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    memset(m_afFillColor,0,3*sizeof(float));
    for(int i=0;i<4;m_afLineColor[i++]=255);
    memset(m_aiBCI,0,3*sizeof(int));
  }

  // }}}
  GLPaintEngine::~GLPaintEngine(){
    // {{{ open

    glFlush();
  } 

  // }}}
  
 void GLPaintEngine::fontsize(int size){
    // {{{ open
    m_oFont.setPointSize(size);
  }

  // }}}
  void  GLPaintEngine::font(string name, int size, TextWeight weight, TextStyle style){
    // {{{ open
    m_oFont.setFamily(name.c_str());
    m_oFont.setPointSize(size);
    m_oFont.setStyle(style == StyleNormal ? QFont::StyleNormal :
                     style == StyleItalic ? QFont::StyleItalic : QFont::StyleOblique);
    m_oFont.setWeight(weight == Light ? QFont::Light :
                      weight == Normal ? QFont::Normal :
                      weight == DemiBold ? QFont::DemiBold :
                      weight == Bold ? QFont::Bold : QFont::Black);
  }

  // }}}
  void GLPaintEngine::color(int r, int g, int b, int a){
    // {{{ open

    m_afLineColor[0] = (float)r/255.0;
    m_afLineColor[1] = (float)g/255.0;
    m_afLineColor[2] = (float)b/255.0;
    m_afLineColor[3] = (float)a/255.0;
  }

  // }}}
  void GLPaintEngine::fill(int r, int g, int b, int a){
    // {{{ open

    m_afFillColor[0] = (float)r/255.0;
    m_afFillColor[1] = (float)g/255.0;
    m_afFillColor[2] = (float)b/255.0;
    m_afFillColor[3] = (float)a/255.0;
  }

  // }}}
  void GLPaintEngine::line(const Point &a, const Point &b){
    // {{{ open

    glColor4fv(m_afLineColor);
    glBegin(GL_LINES);
    glVertex2f(a.x,a.y);
    glVertex2f(b.x,b.y);
    glEnd();
  }

  // }}}
  void GLPaintEngine::point(const Point &p){
    // {{{ open

    glColor4fv(m_afLineColor);
    glBegin(GL_POINTS);
    glVertex2f((GLfloat)p.x,(GLfloat)p.y);
    glEnd();
  }

  // }}}
  void GLPaintEngine::image(const Rect &r,ImgI *image, AlignMode mode){
    // {{{ open
    Size s = image->getSize();
    setupRasterEngine(r,s,mode);
    setPackAlignment(image->getDepth(),s.width);
    setupPixelTransfer(image->getDepth(),m_aiBCI[0],m_aiBCI[1],m_aiBCI[2]);
  
    GLenum datatype = image->getDepth() == depth8u ? GL_UNSIGNED_BYTE : GL_FLOAT;
    static GLenum CHANNELS[4] = {GL_RED,GL_GREEN,GL_BLUE,GL_ALPHA};
    
    if(image->getChannels() > 1){ 
      for(int i=0;i<4 && i<image->getChannels();i++){
        glColorMask(i==0,i==1,i==2,i==3);
        glDrawPixels(s.width,s.height,CHANNELS[i],datatype,image->getDataPtr(i));
      }
      glColorMask(1,1,1,1);
    }else if(image->getChannels() > 0){
      glColorMask(1,1,1,0);
      glDrawPixels(s.width,s.height,GL_LUMINANCE,datatype,image->getDataPtr(0));
    }
  }

  // }}}
  void GLPaintEngine::image(const Rect &r,const QImage &image, AlignMode mode){
    // {{{ open
    setupPixelTransfer(depth8u,m_aiBCI[0],m_aiBCI[1],m_aiBCI[2]);
    glPixelStorei(GL_UNPACK_ALIGNMENT,4);
    setupRasterEngine(r, Size(image.width(),image.height()),mode);
    glDrawPixels(image.width(),image.height(),GL_RGBA,GL_UNSIGNED_BYTE,image.bits());
  }

  // }}}
  void GLPaintEngine::rect(const Rect &r){
    // {{{ open

    glColor4fv(m_afFillColor);
    glBegin(GL_QUADS);
    glVertex2f((GLfloat)r.x,(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.bottom());
    glVertex2f((GLfloat)r.x,(GLfloat)r.bottom());
    glEnd();
    
    glColor4fv(m_afLineColor);
    glBegin(GL_LINE_STRIP);
    glVertex2f((GLfloat)r.x,(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.bottom());
    glVertex2f((GLfloat)r.x,(GLfloat)r.bottom());
    glVertex2f((GLfloat)r.x,(GLfloat)r.y);
    glEnd();
    
    point(Point(r.right(),r.top()));
    
  }

  // }}}
  void GLPaintEngine::ellipse(const Rect &r){
    // {{{ open
    glColor4fv(m_afFillColor);
    GLfloat w2 = 0.5*(r.width);
    GLfloat h2= 0.5*(r.height);
    GLfloat cx = r.x+w2;
    GLfloat cy = r.y+h2;
    static const GLint NSTEPS = 32;
    static const GLfloat D_ARC = (2*M_PI)/NSTEPS;
    glBegin(GL_POLYGON);
    for(int i=0;i<NSTEPS;i++){
      float arc = i*D_ARC;
      glVertex2f(cx+std::cos(arc)*w2,cy+std::sin(arc)*h2);
    }
    glEnd();
    
    glColor4fv(m_afLineColor);
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<NSTEPS;i++){
      float arc = i*D_ARC;
      glVertex2f(cx+std::cos(arc)*w2,cy+std::sin(arc)*h2);
    }
    glVertex2f(cx+std::cos(float(0))*w2,cy+std::sin(float(0))*h2);
    glEnd();
  }

  // }}}
  void GLPaintEngine::text(const Rect &r, const string text, AlignMode mode){
    // {{{ open
    QFontMetrics m(m_oFont);
    QRect br = m.boundingRect(text.c_str());
    QImage img;
    
    img = QImage(br.width()+2,br.height()+6,QImage::Format_ARGB32);
    img.fill(0);
    QPainter painter(&img);
    painter.setFont(m_oFont);
    painter.setPen(QColor( (int)(m_afLineColor[2]*255),
                           (int)(m_afLineColor[1]*255),
                           (int)(m_afLineColor[0]*255),
                           std::min(254,(int)(m_afLineColor[3]*255)) ));
    painter.drawText(QPoint(0,img.height()-4),text.c_str());
    painter.end();
    
    setupPixelTransfer(depth8u,0,0,0);
    glPixelStorei(GL_UNPACK_ALIGNMENT,4);
    

    if(mode == NoAlign){
      // specialized for no alligned text rendering: 2*img.height() makes the text origin be
      // lower left and not upper left
      setupRasterEngine(Rect(r.x,r.y,img.width(),2*img.height()), Size(img.width(),img.height()),mode);
    }else{
      setupRasterEngine(r, Size(img.width(),img.height()),mode);
    }
    
    glDrawPixels(img.width(),img.height(),GL_RGBA,GL_UNSIGNED_BYTE,img.bits());
  }

  // }}}
  
  void GLPaintEngine::bci(int brightness, int contrast, int intensity){
    // {{{ open
    m_aiBCI[0]=brightness;
    m_aiBCI[1]=contrast;
    m_aiBCI[2]=intensity;
  }

  // }}}

  void GLPaintEngine::getColor(int *piColor){
    // {{{ open

    for(int i=0;i<4;piColor[i]=(int)m_afLineColor[i],i++);
  }

  // }}}
  
  void GLPaintEngine::getFill(int *piColor){
    // {{{ open

    for(int i=0;i<4;piColor[i]=(int)m_afFillColor[i],i++);
  }

  // }}}
  

  void GLPaintEngine::setupRasterEngine(const Rect& r, const Size &s, AlignMode mode){
    // {{{ open

    switch(mode){
      case NoAlign:
        glPixelZoom(1.0,-1.0);
        glRasterPos2i(r.x,r.y-r.height+s.height);
        break;
      case Centered:
        glRasterPos2i(r.x+(r.width-s.width)/2,r.y+(r.height-s.height)/2);
        glPixelZoom(1.0,-1.0);      
        break;
      case Justify:
        glPixelZoom((GLfloat)r.width/s.width,-(GLfloat)r.height/s.height);
        glRasterPos2i(r.x,(int)(r.y-r.height+(s.height*(GLfloat)r.height/s.height)));
        break;
    }
  }

  // }}}
  void GLPaintEngine::setPackAlignment(depth d, int linewidth){
    // {{{ open
    if(d==depth8u){
      if(linewidth%2) glPixelStorei(GL_UNPACK_ALIGNMENT,1);
      else if(linewidth%4) glPixelStorei(GL_UNPACK_ALIGNMENT,2);
      else if(linewidth%8) glPixelStorei(GL_UNPACK_ALIGNMENT,4);
      else  glPixelStorei(GL_UNPACK_ALIGNMENT,8);
    }else{
      if(linewidth%2) glPixelStorei(GL_UNPACK_ALIGNMENT,4);
      else glPixelStorei(GL_UNPACK_ALIGNMENT,8);
    }
  }

  // }}}
  void GLPaintEngine::setupPixelTransfer(depth d, int brightness, int contrast, int intensity){
    // {{{ open

    (void)contrast; (void)intensity;
    float fBiasRGB =(float)brightness/255.0;
    float fScaleRGB = d == depth8u ? 1.0 : 1.0/255;
    
    glPixelTransferf(GL_RED_SCALE,fScaleRGB);
    glPixelTransferf(GL_GREEN_SCALE,fScaleRGB);
    glPixelTransferf(GL_BLUE_SCALE,fScaleRGB);
    glPixelTransferf(GL_RED_BIAS,fBiasRGB);
    glPixelTransferf(GL_GREEN_BIAS,fBiasRGB);
    glPixelTransferf(GL_BLUE_BIAS,fBiasRGB);
  }

  // }}}
}
