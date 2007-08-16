#ifndef ICL_GL_TEXTURE_MAP_PAINT_ENGINE_H
#define ICL_GL_TEXTURE_MAP_PAINT_ENGINE_H

#include <iclGLPaintEngine.h>


namespace icl{

  /** \cond */
  class GLTextureMapBaseImage;
  /** \endcond */
  /// Paint Engine implementation which uses OpenGL's texture mapping abilities for drawing images \ingroup UNCOMMON
  /** In addition to the default PaintEngine, this class can handle an addition so called 
      "shared-image". If this image is set, it is drawn. The use of a shared image avoids a 
      deep copy call of that image internally 
  */
  class GLTextureMapPaintEngine : public GLPaintEngine{
    public:
    /// create a new GLTextureMapPaintEngine with given parent widget
    GLTextureMapPaintEngine(QGLWidget *widget);
    
    /// create a new GLTextureMapPaintEngine with given parent widget and given shared image
    GLTextureMapPaintEngine(QGLWidget *widget, GLTextureMapBaseImage *sharedTMImage);

    /// Destructor
    virtual ~GLTextureMapPaintEngine();

    /// reimplementation of the image drawing function (using texture mapping)
    virtual void image(const Rect &r,ImgBase *image, PaintEngine::AlignMode mode = PaintEngine::Justify);

    /// reimplementation of the image drawing function (using texture mapping)
    virtual void image(const Rect &r,const QImage &image, PaintEngine::AlignMode mode = PaintEngine::Justify);

    /// <b>additional</b> shared image drawing function (draws the internal share image)
    void sharedImage(const Rect &r, PaintEngine::AlignMode mode = PaintEngine::Justify);

    private:
    /// internal utility function which helps to calculate the current image rect
    Rect computeRect(const Rect &rect, const Size &imageSize,PaintEngine::AlignMode mode );

    /// wrapped shared image (or null)
    GLTextureMapBaseImage *m_bSharedImage;
    
  };  
}


#endif
