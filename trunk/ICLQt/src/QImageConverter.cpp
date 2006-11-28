#include <QImageConverter.h>

#include <QImage>
#include <QVector>
#include <Img.h>
#include <ICLCore.h>
#include <map>

#ifdef WITH_IPP_OPTIMIZATION
#include <ippi.h>
#include <ipps.h>
#endif

namespace icl{

  // {{{ hidden memory management data and functions

  namespace qimageconverter{
    static int g_iRC(0);
    static QVector<QRgb> g_qvecPalette;
    
    static std::vector<icl32f> g_vecBuffer32f;
    static std::map<int,std::vector<icl32f> > g_mapBuffer32f;
    static std::vector<icl8u> g_vecBuffer8u;
    static std::map<int,std::vector<icl8u> > g_mapBuffer8u;

    static Img8u g_ImgBuffer8u;    
    
    void inc(){
      // {{{ open

      g_iRC++;
    }

    // }}}
    void dec(){
      // {{{ open

      g_iRC--;
      if(!g_iRC){
        g_qvecPalette.clear();
        g_vecBuffer32f.clear();
        g_vecBuffer8u.clear();
        for(std::map<int,std::vector<icl32f> >::iterator it = g_mapBuffer32f.begin();
            it!=g_mapBuffer32f.end(); ++it){
          (*it).second.clear();
        }
        for(std::map<int,std::vector<icl8u> >::iterator it = g_mapBuffer8u.begin();
            it!=g_mapBuffer8u.end(); ++it){
          (*it).second.clear();
        }
      }
    }

    // }}}
  
    void ensureQImage(QImage *&qimage, int w, int h, QImage::Format f){
      // {{{ open

    if(!g_qvecPalette.size()){ for(int i=0;i<255;++i)g_qvecPalette.push_back(qRgb(i,i,i)); }
    
    if(!qimage){
      qimage = new QImage(w,h,f);
      if(f == QImage::Format_Indexed8){
        qimage->setColorTable(g_qvecPalette);
      }
    }
    else{
      if(qimage->width() != w || qimage->height() != h || qimage->format() != f){
        *qimage = QImage(w,h,f);
        if(f == QImage::Format_Indexed8){
          qimage->setColorTable(g_qvecPalette);
        }
      }
    }    
  }

  // }}}
    icl32f *getBuffer32f(unsigned int size, int id=0){
      // {{{ open
    if(!id){
      if(g_vecBuffer32f.size() < size) g_vecBuffer32f.resize(size);
      return &(g_vecBuffer32f[0]);
    }else{
      std::vector<icl32f> &ref = g_mapBuffer32f[id];
      if(ref.size() < size) ref.resize(size);
      return &(ref[0]);
    }
  }

  // }}}
    icl8u *getBuffer8u(unsigned int size,int id=0){
      // {{{ open
    if(!id){
      if(g_vecBuffer8u.size() < size) g_vecBuffer8u.resize(size);
      return &(g_vecBuffer8u[0]);
    }else{
      std::vector<icl8u> &ref = g_mapBuffer8u[id];
      if(ref.size() < size) ref.resize(size);
      return &(ref[0]);
    }
  }

  // }}}

    void copy_8u_C3P3R_function(const icl8u* src, int srcstep, icl8u** dsts, int dststep, const Size &size){
#ifdef WITH_IPP_OPTIMIZATION
      ippiCopy_8u_C3P3R(src,srcstep, dsts,dststep, size);
#else
      icl8u *d1 = dsts[0];
      icl8u *d2 = dsts[1];
      icl8u *d3 = dsts[2];
      icl8u *d4 = dsts[3];
      int srcLineWrap = srcstep-4*size.width*sizeof(icl8u);
      int dstLineWrap = dststep-size.width*sizeof(icl8u);
      if(!dstLineWrap && !srcLineWrap){
        for(const icl8u* srcEnd=src+size.getDim()*4; src<srcEnd;++d1,++d2,++d3,++d4){
          *d1 = *src++;
          *d2 = *src++;
          *d3 = *src++;
          *d4 = *src++;
        }
      }else{
        for(int y=0;y<size.height;y++){
          for(int x=0;x<size.width;x++){            
            *d1 = *src++;
            *d2 = *src++;
            *d3 = *src++;
            *d4 = *src++;
          }
          d1+=dstLineWrap;
          d2+=dstLineWrap;
          d3+=dstLineWrap;
          d4+=dstLineWrap;
          src+=srcLineWrap;
        }        
      }
#endif
    }
    
    
    void copy_8u_P4C4R_function(const icl8u **srcs, int srcstep, icl8u *dst, int dststep, const Size &size){
      // {{{ open

#ifdef WITH_IPP_OPTIMIZATION
      ippiCopy_8u_P4C4R(srcs,srcstep,dst,dststep, size);
#else
      const icl8u *s1 = srcs[0];
      const icl8u *s2 = srcs[1];
      const icl8u *s3 = srcs[2];
      const icl8u *s4 = srcs[3];
      int dstLineWrap = dststep-4*size.width*sizeof(icl8u);
      int srcLineWrap = srcstep-size.width*sizeof(icl8u);
      if(!dstLineWrap && !srcLineWrap){
        for(icl8u* dstEnd=dst+size.getDim()*4; dst<dstEnd;++s1,++s2,++s3,++s4){
          *dst++ = *s1;
          *dst++ = *s2;
          *dst++ = *s3;
          *dst++ = *s4;        
        }
      }else{
        for(int y=0;y<size.height;y++){
          for(int x=0;x<size.width;x++){            
            *dst++ = *s1++;
            *dst++ = *s2++;
            *dst++ = *s3++;
            *dst++ = *s4++; 
          }
          dst+=dstLineWrap;
          s1+=srcLineWrap;
          s2+=srcLineWrap;
          s3+=srcLineWrap;
          s4+=srcLineWrap;
        }        
      }
#endif
    }

    // }}}
  }
  
  using namespace qimageconverter;

  // }}}

  // {{{ defines

#define IDX8U 0
#define IDX32F 1
#define IDXQ 2

#define STU m_eStates[IDX8U]  
#define STF m_eStates[IDX32F]  
#define STQ m_eStates[IDXQ]  

#define CHECK ICLASSERT_RETURN_VAL( m_poImgBuffer8u || m_poImgBuffer32f || m_poQImageBuffer , 0); \
              ICLASSERT_RETURN_VAL( STU == given || STF == given || STQ == given , 0);

  // }}}
  // {{{ Constructors / Destructor

  QImageConverter::QImageConverter():
    // {{{ open

    m_poImgBuffer8u(0),
    m_poImgBuffer32f(0),
    m_poQImageBuffer(0){
    STU=STF=STQ=undefined;
    inc();
  }

  // }}}
  
  QImageConverter::QImageConverter(const ImgBase *image):
    // {{{ open

    m_poImgBuffer8u(0),
    m_poImgBuffer32f(0),
    m_poQImageBuffer(0)
  {
    setImage(image);
    inc();
  }

  // }}}
  
  QImageConverter::QImageConverter(const QImage *qimage):
    // {{{ open

    m_poImgBuffer8u(0),
    m_poImgBuffer32f(0),
    m_poQImageBuffer(0)
  {
    setQImage(qimage);
    inc();
  }

  // }}}

  QImageConverter::~QImageConverter(){
    // {{{ open

    if(m_poImgBuffer8u) delete m_poImgBuffer8u;
    if(m_poImgBuffer32f) delete m_poImgBuffer32f;
    if(m_poQImageBuffer) delete m_poQImageBuffer;
    dec();
  }

  // }}}

  // }}}
  // {{{ image-getter

  const QImage *QImageConverter::getQImage(){
    // {{{ open

    CHECK;
    if(STQ <= uptodate) return m_poQImageBuffer;
    if(STU <= uptodate) return img8uToQImage(m_poImgBuffer8u,m_poQImageBuffer);
    else return img32fToQImage(m_poImgBuffer32f,m_poQImageBuffer);
  }

  // }}}

  const ImgBase *QImageConverter::getImage(){
    // {{{ open

    CHECK;
    if(STU <= uptodate) return m_poImgBuffer8u;
    if(STF <= uptodate) return m_poImgBuffer32f;
    else return qimageToImg8u(m_poQImageBuffer,m_poImgBuffer8u);
  }

  // }}}

  const Img8u *QImageConverter::getImg8u(){
    // {{{ open

    CHECK;
    if(STU <= uptodate) return m_poImgBuffer8u;
    if(STF <= uptodate) return m_poImgBuffer32f->deepCopy(m_poImgBuffer8u)->asImg<icl8u>();
    else return qimageToImg8u(m_poQImageBuffer,m_poImgBuffer8u);
  }

  // }}}

  const Img32f *QImageConverter::getImg32f(){
    // {{{ open

    CHECK;
    if(STF <= uptodate) return m_poImgBuffer32f;
    if(STU <= uptodate) return m_poImgBuffer8u->deepCopy(m_poImgBuffer32f)->asImg<icl32f>();
    else return qimageToImg32f(m_poQImageBuffer,m_poImgBuffer32f);
  }

  // }}}

  // }}}
  // {{{ image-setter

  void QImageConverter::setImage(const ImgBase *image){
    // {{{ open

    ICLASSERT_RETURN( image );

    if(image->getDepth() == depth8u){
      if(m_poImgBuffer8u && STU != given) delete m_poImgBuffer8u;
      STU = given;
      STF = undefined;
      STQ = undefined;
      m_poImgBuffer8u = image->asImg<icl8u>();
    }else{
      if(m_poImgBuffer32f && STF != given) delete m_poImgBuffer32f;
      STF = given;
      STU = undefined;
      STQ = undefined;
      m_poImgBuffer32f = image->asImg<icl32f>();
    }
  }

  // }}}

  void QImageConverter::setQImage(const QImage *qimage){
    // {{{ open

    ICLASSERT_RETURN( qimage );
    STU=STF=undefined;
    if(m_poQImageBuffer && STQ != given) delete m_poQImageBuffer;
    STQ = given;
    m_poQImageBuffer = const_cast<QImage*>(qimage);
    
  } 

  // }}}

  // }}}

  // {{{ XXXToYYY-Functions

  const QImage *QImageConverter::img32fToQImage(Img32f *image, QImage *&qimage){
    // {{{ open
    ICLASSERT_RETURN_VAL( image && image->getChannels() > 0, 0);
    image->convertTo<icl8u>(&g_ImgBuffer8u);
    return img8uToQImage(&g_ImgBuffer8u,qimage);
  }
    // }}}
  
  const QImage *QImageConverter::img8uToQImage(Img8u *image, QImage *&qimage){
    // {{{ open

    // {{{ variable declaration

    ICLASSERT_RETURN_VAL( image && image->getChannels() > 0, 0);
   
    int c = image->getChannels();
    int w = image->getWidth();
    int h = image->getHeight();
    int dim = c*w*h;
    const Size &s = image->getSize();
    int step = image->getLineStep();
    const icl8u *ap[4];
    // }}}
    

    switch(image->getChannels()){
      case 1:
        ensureQImage(qimage,w,h,QImage::Format_Indexed8);
        icl::copy(image->getData(0),image->getData(0)+w*h,qimage->bits());
        break;
      case 2:{
        ap[0] = image->getData(0); //using b and r channel
        ap[1] = getBuffer8u(dim,0);
        ap[2] = image->getData(1);
        ap[3] = getBuffer8u(dim,1);
        ensureQImage(qimage,w,h,QImage::Format_RGB32);
        copy_8u_P4C4R_function(ap,step,qimage->bits(),4*step,s);
        break;
      }        
      case 3:
        ap[0] = image->getData(2); // qt byter order bgra
        ap[1] = image->getData(1);
        ap[2] = image->getData(0);
        ap[3] = getBuffer8u(dim,0);
        ensureQImage(qimage,w,h,QImage::Format_RGB32);
        copy_8u_P4C4R_function(ap,step,qimage->bits(),4*step,s);
        break;
      default:
        ap[0] = image->getData(2); // qt byter order bgra
        ap[1] = image->getData(1);
        ap[2] = image->getData(0);
        ap[3] = image->getData(3);
        ensureQImage(qimage,w,h,QImage::Format_RGB32);
        copy_8u_P4C4R_function(ap,step,qimage->bits(),4*step,s);
        break;
    }
    STQ=uptodate;
    return qimage;
  }

  // }}}

  const Img8u *QImageConverter::qimageToImg8u(QImage *qimage, Img8u *&image){
    // {{{ open

    ICLASSERT_RETURN_VAL(qimage && !qimage->isNull() ,0);
    ensureCompatible((ImgBase**)&image,depth8u,Size(qimage->width(),qimage->height()),3);
    icl8u *ap[3] = { image->getData(0), image->getData(1), image->getData(2) };
    copy_8u_C3P3R_function(qimage->bits(),image->getLineStep()*3, ap, image->getLineStep(),image->getSize());

    STU = uptodate;
    return image;
  }

  // }}}

  const Img32f *QImageConverter::qimageToImg32f(QImage *qimage, Img32f *&image){
    // {{{ open

    ICLASSERT_RETURN_VAL(qimage && !qimage->isNull() ,0);  
    ensureCompatible((ImgBase**)&image,depth32f,Size(qimage->width(),qimage->height()),3);
    
    int dim = image->getDim();
    icl8u *buf = getBuffer8u(image->getDim()*3);
    icl8u *ap[] = { buf,buf+dim,buf+2*dim };
    copy_8u_C3P3R_function(qimage->bits(),3*dim, ap, dim,image->getSize());
    icl::copy(ap[0],ap[0]+dim,image->getData(0));
    icl::copy(ap[1],ap[1]+dim,image->getData(1));
    icl::copy(ap[2],ap[2]+dim,image->getData(2));

    STF = uptodate;
    return image;  
  }

  // }}}

  // }}}
  
}

