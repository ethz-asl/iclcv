#include <iclQuick.h>
#include <iclFileReader.h>
#include <iclTestImages.h>
#include <iclConverter.h>

#ifndef __APPLE__
#include <iclPWCGrabber.h>
#endif

#include <iclCC.h>
#include <map>
#include <iclConvolutionOp.h>
#include <iclMedianOp.h>
#include <iclMorphologicalOp.h>
#include <iclBinaryArithmeticalOp.h>
#include <iclUnaryArithmeticalOp.h>
#include <iclFileWriter.h>
#include <iclUnaryCompareOp.h>
#include <iclLUTOp.h>
#include <iclQImageConverter.h>
#include <iclTimer.h>
#include <QPainter>
#include <QImage>
#include <QFont>
#include <QApplication>




namespace icl{
  namespace {
    struct Color{
      // {{{ open

      Color(const float *color){
        for(int i=0;i<4;++i)COLOR[i]=color[i];
      }
      float COLOR[4];
    };

    // }}}

    Color *savedColor = 0;
    Color *savedFill = 0;
    
    float COLOR[4] = {255,0,0,255};
    float FILL[4] = {0,0,0,0};

    void saveColorAndFill(){
      // {{{ open

      if(savedColor) delete savedColor;
      if(savedFill) delete savedFill;
      savedColor = new Color(COLOR);
      savedFill = new Color(FILL);
    }

    // }}}
    void restoreColorAndFill(){
      // {{{ open

      if(savedColor){
        for(int i=0;i<4;i++)COLOR[i]=savedColor->COLOR[i];
        delete savedColor; 
        savedColor = 0;
      }
      if(savedFill){
        for(int i=0;i<4;i++)FILL[i]=savedFill->COLOR[i];
        delete savedFill; 
        savedFill = 0;
      }
    }

    // }}}
    int FONTSIZE = 12;
    string FONTFAMILY = "Times";
    void bresenham(int x0, int x1, int y0, int y1, vector<int> &xs, vector<int> &ys, int maxX, int maxY){
      // {{{ open

      int steep = std::abs(y1 - y0) > std::abs(x1 - x0);
      if(steep){
        swap(x0, y0);
        swap(x1, y1);
      }
      int steep2 = x0 > x1;
      if(steep2){
        swap(x0, x1);
        swap(y0, y1);
      }
      
      int deltax = x1 - x0;
      int deltay = std::abs(y1 - y0);
      int error = 0;
      int ystep = y0 < y1 ? 1 : -1;
      
      for(int x=x0,y=y0;x<=x1;x++){
        if(x>=0 && x<=maxX && y>=0 && y<=maxY){
          if(steep){
            xs.push_back(y); 
            ys.push_back(x);
          }else{
            xs.push_back(x); 
            ys.push_back(y);
          }
        }
        error += deltay;
        if (2*error >= deltax){
          y += ystep;
          error -=deltax;
        }
      }
    }
    // }}}

#ifndef __APPLE__
    static PWCGrabber *G[4] = {0,0,0,0};
    struct PWCReleaser{
      // {{{ open

      ~PWCReleaser(){
        for(int i=0;i<4;i++){
          if(G[i]) delete G[i];
        }
      }
    };

    // }}}
    static PWCReleaser __r;
#endif

    inline ImgQ *prepare_for_binary_op(const ImgQ &a, const ImgQ &b, ImgQ &na, ImgQ &nb){
      // {{{ open

      if(a.getROISize() == b.getROISize() && a.getChannels() == b.getChannels()){
        na = copy(a);
        nb = copy(b);
        return new ImgQ(a.getParams());
      }

    Size sa = a.getROISize(), sb = b.getROISize();
    Size sr(max(sa.width,sb.width), max(sa.height,sb.height) );
    int cr = max(a.getChannels(),b.getChannels());
      
    na = ImgQ(sr,a.getChannels());
    na.setROI(a.getROI());
    a.deepCopyROI(&na);
    na.setFullROI();
    na.setChannels(cr);


    nb = ImgQ(sr,b.getChannels());
    nb.setROI(b.getROI());
    b.deepCopyROI(&nb);
    nb.setFullROI();
    nb.setChannels(cr);
      
    return new ImgQ(na.getParams());
  }

  // }}}
    
    inline ImgQ apply_binary_arithmetical_op(const ImgQ &a, const ImgQ &b,  BinaryArithmeticalOp::optype ot){
      // {{{ open

      BinaryArithmeticalOp binop(ot);
      binop.setCheckOnly(true);
      binop.setClipToROI(true);

      if(a.getROISize() == b.getROISize() && a.getChannels() == b.getChannels()){
          ImgBase *res = new ImgQ(a.getParams());
          binop.apply(&a,&b,&res);
          ImgQ x = *(res->asImg<ICL_QUICK_TYPE>());
          delete res;
          return x;
      }
      
      Size sa = a.getROISize(), sb = b.getROISize();
      Size sr(max(sa.width,sb.width), max(sa.height,sb.height) );
      int cr = max(a.getChannels(),b.getChannels());
      
      ImgQ na(sr,a.getChannels());
      na.setROI(a.getROI());
      a.deepCopyROI(&na);
      na.setFullROI();
      na.setChannels(cr);
      
      
      ImgQ nb(sr,b.getChannels());
      nb.setROI(b.getROI());
      b.deepCopyROI(&nb);
      nb.setFullROI();
      nb.setChannels(cr);
      
      ImgBase * res = new ImgQ(na.getParams());
      
      binop.apply(&na,&nb,&res);
      ImgQ x = *(res->asImg<ICL_QUICK_TYPE>());
      delete res;
      return x;
    }

    // }}}
    inline ImgQ apply_unary_arithmetical_op(const ImgQ &image,ICL_QUICK_TYPE val, UnaryArithmeticalOp::optype ot){
      // {{{ open

      ImgBase *dst = 0;
      UnaryArithmeticalOp(ot,val).apply(&image,&dst);
      ImgQ r = *(dst->asImg<ICL_QUICK_TYPE>());
      delete dst;
      return r;
    }

    // }}}

    Timer *TIMER=0;      
  }

  using namespace std;

  ImgQ zeros(int width, int height, int channels){
    // {{{ open

    return ImgQ(Size(width,height),channels);
  }

  // }}}
  ImgQ ones(int width, int height, int channels){
    // {{{ open

    ImgQ a(Size(width,height),channels);
    a.clear(-1,1.0,false);
    return a;
  }

  // }}}
  ImgQ load(const string &filename, format fmt){
    // {{{ open

    FileReader g(filename);
    ImgQ *image = g.grab()->convert<ICL_QUICK_TYPE>();
    if(!image){
      return ImgQ();
    }
    
    ImgQ im(image->getSize(),fmt);
    im.setTime(image->getTime());
    cc(image,&im);
    delete image;
    return im;

  }

  // }}}
  ImgQ create(const string &name, format fmt){
    // {{{ open

    ImgQ *image = TestImages::create(name,fmt,depth32f)->asImg<ICL_QUICK_TYPE>();
    if(!image){
      return ImgQ();
    }
    ImgQ im = *image;
    delete image;
    return im;
  }

  // }}}

  Img8u cvt8u(const ImgQ &image){
    // {{{ open

    Img8u *p = image.convert<icl8u>();
    Img8u r = *p;
    delete p;
    return r;
  }

  // }}}
  Img16s cvt16s(const ImgQ &image){
    // {{{ open

    Img16s *p = image.convert<icl16s>();
    Img16s r = *p;
    delete p;
    return r;

  }

  // }}}
  Img32s cvt32s(const ImgQ &image){
    // {{{ open

    Img32s *p = image.convert<icl32s>();
    Img32s r = *p;
    delete p;
    return r;
  }

  // }}}
  Img32f cvt32f(const ImgQ &image){
    // {{{ open

    Img32f *p = image.convert<icl32f>();
    Img32f r = *p;
    delete p;
    return r;
  }

  // }}}
  Img64f cvt64f(const ImgQ &image){
    // {{{ open

    Img64f *p = image.convert<icl64f>();
    Img64f r = *p;
    delete p;
    return r;
  }

  // }}}

  ImgQ cvt(const Img8u &image){
    // {{{ open

    ImgQ *p = image.convert<ICL_QUICK_TYPE>();
    ImgQ r = *p;
    delete p;
    return r;
  }

  // }}}
  ImgQ cvt(const Img16s &image){
    // {{{ open

    ImgQ *p = image.convert<ICL_QUICK_TYPE>();
    ImgQ r = *p;
    delete p;
    return r;
  }

  // }}}
  ImgQ cvt(const Img32s &image){
    // {{{ open

    ImgQ *p = image.convert<ICL_QUICK_TYPE>();
    ImgQ r = *p;
    delete p;
    return r;
  }

  // }}}
  ImgQ cvt(const Img32f &image){
    // {{{ open

    ImgQ *p = image.convert<ICL_QUICK_TYPE>();
    ImgQ r = *p;
    delete p;
    return r;
  }

  // }}}
  ImgQ cvt(const Img64f &image){
    // {{{ open

    ImgQ *p = image.convert<ICL_QUICK_TYPE>();
    ImgQ r = *p;
    delete p;
    return r;
  }

  // }}}


  ImgQ pwc(int device, const Size &size, format fmt, bool releaseGrabber){
    // {{{ open
#ifndef __APPLE__
    if(device > 4){
      ERROR_LOG("device must be in 1,2,3 or 4");
      return ImgQ();
    }
    
    if(!G[device]){
      G[device] = new PWCGrabber(Size(640,480),device);
    }
    ImgQ *image = G[device]->grab()->convert<ICL_QUICK_TYPE>();
    ImgQ im(size,fmt);
    im.setTime(image->getTime());
    cc(image,&im);
    delete image;
    if(releaseGrabber){
      delete G[device];
      G[device] = 0;
    }
    return im;
#else
    ImgQ im(size, fmt);
    return label(im,"PWC not supported");
#endif    
  }

  // }}}
  ImgQ ieee(int device,const Size &size, format fmt, bool releaseGrabber){
    // {{{ open

    WARNING_LOG("this function is not yet implemented");
    return ImgQ();
  }

  // }}}
  
  ImgQ filter(const ImgQ &image,const string &filter){
    // {{{ open

    static map<string,UnaryOp*> M;
    bool inited = false;
    if(!inited){
      static icl8u mask[9] = {1,1,1,1,1,1,1,1,1};
      static Size s3x3(3,3);
      M["sobelx"] = new ConvolutionOp(ConvolutionOp::kernelSobelX3x3);
      M["sobely"] = new ConvolutionOp(ConvolutionOp::kernelSobelY3x3);
      M["gauss"] = new ConvolutionOp(ConvolutionOp::kernelGauss3x3);
      M["laplace"] = new ConvolutionOp(ConvolutionOp::kernelLaplace3x3);
      M["median"] = new MedianOp(Size(3,3));
#ifdef WITH_IPP_OPTIMIZATION
      M["dilation"] = new MorphologicalOp(s3x3,(char*)mask, MorphologicalOp::dilate);
      M["erosion"] = new MorphologicalOp(s3x3,(char*)mask, MorphologicalOp::erode);
      M["opening"] = new MorphologicalOp(s3x3,(char*)mask, MorphologicalOp::openBorder);
      M["closing"] = new MorphologicalOp(s3x3,(char*)mask, MorphologicalOp::closeBorder);
#endif
    }    
    UnaryOp* u = M[filter];
    if(!u){
      WARNING_LOG("nothing known about filter type:" << filter);
      return ImgQ();
    }
    ImgBase *dst = 0;
    u->apply(&image,&dst);
    ImgQ *dstQ = dst->convert<ICL_QUICK_TYPE>();
    delete dst;
    ImgQ im = *dstQ;
    delete dstQ;
    return im;    
  } 

  // }}}
  ImgQ copy(const ImgQ &image){
    // {{{ open

    ImgQ *cpy = image.deepCopy()->asImg<ICL_QUICK_TYPE>();
    ImgQ im = *cpy;
    delete cpy;
    return im;
  }

  // }}}
  ImgQ copyroi(const ImgQ &image){
    // {{{ open
    ImgQ *cpy = image.deepCopyROI()->asImg<ICL_QUICK_TYPE>();
    ImgQ im = *cpy;
    delete cpy;
    return im;
  }

  // }}}
  ImgQ norm(const ImgQ &image){
    // {{{ open

    ImgQ *cpy = image.deepCopyROI();
    cpy->normalizeAllChannels(Range<ICL_QUICK_TYPE>(0,255));
    ImgQ x = *cpy;
    delete cpy;
    return x;
  }

  // }}}
  
  void save(const ImgQ &image,const string &filename){
    // {{{ open

    Img<float> bla = Img<float>(Size(5,5),5);
    //    Img<float> blub = bla;
    ImgQ roi = copyroi(image);
    //    roi = copyroi(image);
    FileWriter(filename).write(&roi);
  }

  // }}}
  void show(const ImgQ &image){
    // {{{ open
    if(image.hasFullROI()){
      TestImages::xv(&image);
    }else{
      ImgQ T = copy(image);
      saveColorAndFill();
      color(255,0,0);
      fill(0,0,0,0);
      rect(T,T.getROI());
      restoreColorAndFill();
      T.setFullROI();
      TestImages::xv(&T);
    }
  }

  // }}}
  void print(const ImgQ &image){
    // {{{ open

    image.print("image");
  }

  // }}}

  ImgQ operator+(const ImgQ &a,const ImgQ &b){
    // {{{ open

    return apply_binary_arithmetical_op(a,b,BinaryArithmeticalOp::addOp);
  }

  // }}}
  ImgQ operator-(const ImgQ &a, const ImgQ &b){
    // {{{ open

    return apply_binary_arithmetical_op(a,b,BinaryArithmeticalOp::subOp);
  }

  // }}}
  ImgQ operator*(const ImgQ &a, const ImgQ &b){
    // {{{ open

    return apply_binary_arithmetical_op(a,b,BinaryArithmeticalOp::mulOp);
  }

  // }}}
  ImgQ operator/(const ImgQ &a, const ImgQ &b){
    // {{{ open

    return apply_binary_arithmetical_op(a,b,BinaryArithmeticalOp::divOp);
  }

  // }}}

  ImgQ operator+(const ImgQ &image, float val){
    // {{{ open

    return apply_unary_arithmetical_op(image,val,UnaryArithmeticalOp::addOp);
  }

  // }}}
  ImgQ operator-(const ImgQ &image, float val){
    // {{{ open

    return apply_unary_arithmetical_op(image,val,UnaryArithmeticalOp::subOp);
  }

  // }}}
  ImgQ operator*(const ImgQ &image, float val){
    // {{{ open

    return apply_unary_arithmetical_op(image,val,UnaryArithmeticalOp::mulOp);
  }

  // }}}
  ImgQ operator/(const ImgQ &image, float val){
    // {{{ open

    return apply_unary_arithmetical_op(image,val,UnaryArithmeticalOp::divOp);
  }

  // }}}

  ImgQ operator+(float val, const ImgQ &image){
    // {{{ open

    return image+val;
  }

  // }}}
  ImgQ operator-(float val, const ImgQ &image){
    // {{{ open
    ImgQ res(image.getROISize(),image.getChannels());
    for(int c=0;c<image.getChannels();++c){
      ConstImgIterator<ICL_QUICK_TYPE> itI = image.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itRes = res.getROIIterator(c);
      for(;itI.inRegion();++itI,++itRes){
        *itRes = val - (*itI);
      }
    }
    return res;
  }

  // }}}
  ImgQ operator*(float val, const ImgQ &image){
    // {{{ open

    return image*val;
  }

  // }}}
  ImgQ operator/(float val, const ImgQ &image){
    // {{{ open
    ImgQ res(image.getROISize(),image.getChannels());
    for(int c=0;c<image.getChannels();++c){
      ConstImgIterator<ICL_QUICK_TYPE> itI = image.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itRes = res.getROIIterator(c);
      for(;itI.inRegion();++itI,++itRes){
        *itRes = ((*itI) == 0) ? 0 :  val / (*itI);
      }
    }
    return res;
  }

  // }}}


  ImgQ cc(const ImgQ& image, format fmt){
    // {{{ open
    ImgQ dst(image.getSize(),fmt);
    ImgQ src = copyroi(image);
    cc(&src,&dst);
    return dst;
  }

  // }}}
  ImgQ rgb(const ImgQ &image){
    // {{{ open

    return cc(image,formatRGB);
  }

  // }}}
  ImgQ hls(const ImgQ &image){
    // {{{ open

    return cc(image,formatHLS);
  }

  // }}}
  ImgQ lab(const ImgQ &image){
    // {{{ open

    return cc(image,formatLAB);
  }

  // }}}
  ImgQ gray(const ImgQ &image){
    // {{{ open

    return cc(image,formatGray);
  }

  // }}}


  ImgQ scale(const ImgQ& image, float factor){
    // {{{ open

    return scale(image,(int)(factor*image.getWidth()), (int)(factor*image.getHeight()));
  }

  // }}}
  ImgQ scale(const ImgQ& image, int width, int height){
    // {{{ open

    ImgQ *n = image.scaledCopyROI(Size(width,height));
    ImgQ a = *n;
    delete n;
    return a;    
  }

  // }}}
  ImgQ channel(const ImgQ &image, int channel){
    // {{{ open
    const ImgQ *c = image.selectChannel(channel)->asImg<ICL_QUICK_TYPE>();
    const ImgQ a = copy(*c);
    delete c;
    return a;
  }

  // }}}
  ImgQ levels(const ImgQ &image, icl8u levels){
    // {{{ open

    ImgBase *src = image.convertROI(depth8u);
    ImgBase *dst = 0;
    LUTOp(levels).apply(src,&dst);
    ImgQ *a = dst->convert<ICL_QUICK_TYPE>();
    ImgQ b = *a;
    delete a;
    delete src;
    delete dst;
    return b;
  }

  // }}}
  ImgQ thresh(const ImgQ &image, float threshold){
    // {{{ open

    ImgQ r(image.getROISize(),image.getChannels(),image.getFormat());
    r.setTime(image.getTime());
    for(int c=0;c<image.getChannels();++c){
      ConstImgIterator<ICL_QUICK_TYPE> itSrc = image.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itDst = r.getIterator(c);
      for(;itSrc.inRegion();++itSrc,++itDst){
        *itDst = *itSrc < threshold ? 0 : 255;
      }
    }    
    return r;
  }

  // }}}
  ImgQ flipx(const ImgQ& image){
    // {{{ open
    ImgQ r(image.getParams());
    ImgBase *rr = &r;
    flippedCopy(axisVert,&image,&rr);
    return r;
  }

  // }}}
  ImgQ flipy(const ImgQ& image){
    // {{{ open

    ImgQ r(image.getParams());
    ImgBase *rr = &r;
    flippedCopy(axisHorz,&image,&rr);
    return r;
  }

  // }}}

  ImgQ exp(const ImgQ &image){
    // {{{ open

    return apply_unary_arithmetical_op(image,0,UnaryArithmeticalOp::expOp);
  }

  // }}}
  ImgQ ln(const ImgQ &image){
    // {{{ open

    return apply_unary_arithmetical_op(image,0,UnaryArithmeticalOp::lnOp);
  }

  // }}}
  ImgQ sqr(const ImgQ &image){
    // {{{ open

    return apply_unary_arithmetical_op(image,0,UnaryArithmeticalOp::sqrOp);
  }

  // }}}
  ImgQ sqrt(const ImgQ &image){
    // {{{ open

    return apply_unary_arithmetical_op(image,0,UnaryArithmeticalOp::sqrtOp);
  }

  // }}}
  ImgQ abs(const ImgQ &image){
    // {{{ open

    return apply_unary_arithmetical_op(image,0,UnaryArithmeticalOp::absOp);
  }

  // }}}
  ImgQ operator-(const ImgQ &image){
    // {{{ open

    return image*(-1);
  }

  // }}}

  ImgQ operator||(const ImgQ &a, const ImgQ &b){
    // {{{ open

    ImgQ na,nb;
    ImgQ *res = prepare_for_binary_op(a,b,na,nb);
    ImgQ r = *res;
  
    delete res;
    for(int c=0;c<a.getChannels();c++){
      ImgIterator<ICL_QUICK_TYPE> itA = na.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itB = nb.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itR = r.getROIIterator(c);
      for(;itA.inRegion();++itA,++itB, ++itR){
        *itR = *itA || *itB;
      }
    }
    return r;
  }

  // }}}
  ImgQ operator&&(const ImgQ &a, const ImgQ &b){
    // {{{ open

   ImgQ na,nb;
    ImgQ *res = prepare_for_binary_op(a,b,na,nb);
    ImgQ r = *res;
    delete res;
    for(int c=0;c<a.getChannels();c++){
      ImgIterator<ICL_QUICK_TYPE> itA = na.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itB = nb.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itR = r.getROIIterator(c);
      for(;itA.inRegion();++itA,++itB, ++itR){
        *itR = *itA && *itB;
      }
    }
    return r;
  }

  // }}}
  ImgQ operator,(const ImgQ &a, const ImgQ &b){
    // {{{ open
    if(a.getSize() == Size::null) return copy(b);
    if(b.getSize() == Size::null) return copy(a);
    ImgQ r = zeros(a.getWidth()+b.getWidth(),max(a.getHeight(),b.getHeight()),max(a.getChannels(),b.getChannels()));
    r.setROI(a.getROI());
    roi(r) = a;

    Rect newroi(Point(r.getROIXOffset()+r.getROIWidth(), r.getROIYOffset()),b.getROISize());
    r.setROI(newroi);

    roi(r) = b;
    r.setFullROI();
    return r;
  }

  // }}}
  ImgQ operator%(const ImgQ &a, const ImgQ &b){
    // {{{ open
    if(a.getSize() == Size::null) return copy(b);
    if(b.getSize() == Size::null) return copy(a);
    ImgQ r = zeros(max(a.getWidth(),b.getWidth()),a.getHeight()+b.getHeight(),max(a.getChannels(),b.getChannels()));
    r.setROI(a.getROI());
    roi(r) = a;
    Rect newroi(Point(r.getROIXOffset(),r.getROIYOffset()+r.getROIHeight()), b.getROISize());
    r.setROI(newroi);
    roi(r) = b;
    r.setFullROI();
    return r;
  }

  // }}}

  ImgQ operator|(const ImgQ &a, const ImgQ &b){
    // {{{ open

    if(a.getChannels() == 0 || a.getROISize() == Size::null) return copy(b);
    if(b.getChannels() == 0 || b.getROISize() == Size::null) return copy(a);
    ImgQ r = zeros(max(a.getWidth(),b.getWidth()),max(a.getHeight(),b.getHeight()),a.getChannels()+b.getChannels());
    r.setROI(a.getROI());
    for(int c=0;c<a.getChannels();c++){
      deepCopyChannelROI (&a,c,a.getROIOffset(),a.getROISize(), &r,c,r.getROIOffset(), r.getROISize());
    }
    r.setROI(b.getROI());
    for(int c=0;c<b.getChannels();c++){
      deepCopyChannelROI (&b,c,b.getROIOffset(),b.getROISize(), &r,c+a.getChannels(),r.getROIOffset(), r.getROISize());
    }
    r.setFullROI();
    return r;
  }

  // }}}

  ImgROI &ImgROI::operator=(float val){
    // {{{ open

    image.clear(-1,val,true);
    return *this;
  }

  // }}}
  ImgROI &ImgROI::operator=(const ImgQ &i){
    // {{{ open
    ICLASSERT_RETURN_VAL(image.getROISize() == i.getROISize(),*this);
    for(int c=0;c<min(image.getChannels(),i.getChannels());++c){
      deepCopyChannelROI(&i,c,i.getROIOffset(),i.getROISize(),
                         &image,c,image.getROIOffset(),image.getROISize());
    }
    return *this;
  }

  // }}}
  ImgROI &ImgROI::operator=(const ImgROI &r){
    // {{{ open
    ICLASSERT_RETURN_VAL(image.getROISize() == r.image.getROISize(),*this);
    for(int c=0;c<min(image.getChannels(),r.image.getChannels());++c){
      deepCopyChannelROI(&(r.image),c,r.image.getROIOffset(),r.image.getROISize(),
                         &image,c,image.getROIOffset(),image.getROISize());
    }
    return *this;
  }

  // }}}

  ImgROI::operator ImgQ(){
    // {{{ open

    return image;
  }

  // }}}
  ImgROI roi(ImgQ &r){
    // {{{ open

    ImgROI roi = { r };
    return roi;
  }

  // }}}
  ImgROI data(ImgQ &r){
    // {{{ open

    ImgROI roi = { r };
    roi.image.setFullROI();
    return roi;
  }

  // }}}

  void color(float r, float g, float b, float a){
    // {{{ open

    COLOR[0] = r;
    COLOR[1] = g<0 ? r : g;
    COLOR[2] = b<0 ? r : b;
    COLOR[3] = a;
  }

  // }}}
  void fill(float r, float g, float b, float a){
    // {{{ open

    FILL[0] = r;
    FILL[1] = g<0 ? r : g;
    FILL[2] = b<0 ? r : b;
    FILL[3] = a;
  }

  // }}}

  void cross(ImgQ &image, int X, int Y){
    // {{{ open

    static const int CROSS_SIZE = 3;
    line(image, X-CROSS_SIZE,Y-CROSS_SIZE,X+CROSS_SIZE,Y+CROSS_SIZE);
    line(image, X-CROSS_SIZE,Y+CROSS_SIZE,X+CROSS_SIZE,Y-CROSS_SIZE);
  }

  // }}}
  
  void rect(ImgQ &image, int x, int y, int w, int h){
    // {{{ open

    line(image,x,y,x+w-1,y);
    line(image,x,y,x,y+h-1);
    line(image,x+w-1,y+h-1,x+w-1,y);
    line(image,x+w-1,y+h-1,x,y+h-1);
    float A = FILL[3]/255;
    if(! A) return;
    for(int i=x+1;i<x+w-1;i++){   
      for(int j=y+1;j<y+h-1;j++){
        if(i>=0 && j>=0 && i<image.getWidth() && j<image.getHeight()){
          for(int c=0;c<image.getChannels() && c<3; ++c){
            float &v = image(i,j,c);
            v=(1.0-A)*v + A*FILL[c];
          }
        }
      }
    }
  }

  // }}}

  void circle(ImgQ &image, int xoffs, int yoffs, int radius) {
    // {{{ open
		// first render the circle
    int n = 0;
    char ** ppc = 0;
    static QApplication  *QAPP = 0;
    if(!QAPP){
      QAPP = new QApplication(n,ppc);
    }
    static QSize *br = new QSize(radius * 2, radius * 2);
    
    QImage img(br->width()+2,br->height()+2,QImage::Format_ARGB32_Premultiplied);
    img.fill(0);

    QPainter painter(&img);
    painter.setPen(QColor(255,255,255,254));
		painter.setBrush(QColor(255, 255, 255, 254));
		painter.drawEllipse(QRectF(xoffs - radius, yoffs - radius, xoffs + radius, yoffs + radius));
    painter.end();
    
    QImageConverter qic(&img);
    const Img8u &t = *(qic.getImg<icl8u>());
    for(int c=0;c<image.getChannels() && c<3; ++c){
      for(int x=0;x<t.getWidth();x++){
        for(int y=0;y<t.getHeight();y++){
          int ix = x+xoffs-radius;
          int iy = y+yoffs-radius;
          if(ix >= 0 && iy >= 0 && ix < image.getWidth() && iy < image.getHeight() ){
            ICL_QUICK_TYPE &v = image(ix,iy,c);
            float A = (((float)t(x,y,c))/255.0) * (FILL[3]/255);
            v=(1.0-A)*v + A*FILL[c];
          }
        }
      }
    }

  }

  // }}}

  void line(ImgQ &image, int x1, int y1, int x2, int y2){
    // {{{ open

    std::vector<int> xs,ys;
    bresenham(x1,x2,y1,y2,xs,ys,image.getWidth()-1,image.getHeight()-1);
    float A = COLOR[3]/255.0;
    for(vector<int>::iterator itX=xs.begin(), itY=ys.begin(); itX != xs.end(); ++itX, ++itY){
      for(int c=0;c<image.getChannels() && c<3; ++c){
        if(*itX >= 0 && *itX < image.getWidth() && *itY >= 0 && *itY <= image.getHeight()){
          float &v = image(*itX,*itY,c);
          v=(1.0-A)*v + A*COLOR[c];
        }
      }
    }
  }

  // }}}

  void text(ImgQ &image, int xoffs, int yoffs, const string &text){
    // {{{ open
    // first rendering the text 
    int n = 0;
    char ** ppc = 0;
    static QApplication  *QAPP = 0;
    if(!QAPP){
      QAPP = new QApplication(n,ppc);
    }
    QFont f(FONTFAMILY.c_str(),FONTSIZE,QFont::DemiBold);
    QFontMetrics m(f);
    QSize br = m.size(Qt::TextSingleLine,text.c_str());
    
    QImage img(br.width()+2,br.height()+2,QImage::Format_ARGB32_Premultiplied);
    img.fill(0);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(f);
    painter.setPen(QColor(255,255,255,254));   
    painter.drawText(QPoint(1,img.height()-m.descent()-1),text.c_str());
    painter.end();
    
    QImageConverter qic(&img);
    const Img8u &t = *(qic.getImg<icl8u>());
    for(int c=0;c<image.getChannels() && c<3; ++c){
      for(int x=0;x<t.getWidth();x++){
        for(int y=0;y<t.getHeight();y++){
          int ix = x+xoffs;
          int iy = y+yoffs;
          if(ix >= 0 && iy >= 0 && ix < image.getWidth() && iy < image.getHeight() ){
            ICL_QUICK_TYPE &v = image(ix,iy,c);
            float A = (((float)t(x,y,c))/255.0) * (COLOR[3]/255);
            v=(1.0-A)*v + A*COLOR[c];
          }
        }
      }
    }
  }

  // }}}

  inline void pix(ImgQ &image, int x, int y){
    // {{{ open
    float A = COLOR[3]/255.0;
    if(x>=0 && y>=0 && x<image.getWidth() && y<image.getHeight()){
      for(int c=0;c<image.getChannels() && c<3; ++c){
        float &v = image(x,y,c);
        v=(1.0-A)*v + A*COLOR[c];
      }
    }
  }

  // }}}

  void pix(ImgQ &image, const vector<Point> &pts){
    // {{{ open

    for(vector<Point>::const_iterator it = pts.begin();it!=pts.end();++it){
      pix(image,it->x,it->y);
    }
  } 

  // }}}
 
  ImgQ label(const ImgQ &imageIn, const string &text){
    // {{{ open
    ImgQ image = copy(imageIn);
        
    float _COLOR[4] = { COLOR[0],COLOR[1],COLOR[2],COLOR[3] };
    int _FONTSIZE = FONTSIZE;
    string _FONTFAMILY = FONTFAMILY;
 
    FONTSIZE = 10;
    FONTFAMILY = "Arial";
    COLOR[3]=255;

    std::fill(COLOR,COLOR+3,float(1.0));
    icl::text(image,6,6,text);
    
    std::fill(COLOR,COLOR+3,float(255.0));
    icl::text(image,5,5,text);    
    
    std::copy((float*)_COLOR,_COLOR+4,COLOR);
    FONTSIZE = _FONTSIZE;
    FONTFAMILY = _FONTFAMILY;
    return image;    
  }

  // }}}

  void font(int size, const string &family){
    // {{{ open

    FONTSIZE = size;
    FONTFAMILY = family;
  }

  // }}}

  void fontsize(int size){
    // {{{ open

    FONTSIZE = size;
  }

  // }}}
  
  void tic(){
    // {{{ open

    if(!TIMER){
      TIMER = new Timer();
      TIMER->start();
    }else{
      printf("timer was already running: \n");
      TIMER->stop();
      delete TIMER;
      TIMER = new Timer();
      TIMER->start();
    }
  }

  // }}}
  void toc(){
    // {{{ open

    if(!TIMER){
      printf("could not stop timer: call tic first! \n");
    }else{
      TIMER->stop();
      delete TIMER;
      TIMER = 0;
    }
  }

  // }}}

  template<class T>
  ImgBasePtrPtr<T>::~ImgBasePtrPtr(){
    // {{{ open
    if(!r){
      ERROR_LOG("Result image is NULL");
      if(o) delete o;
      return;
    }
    if(r && !o){
      ERROR_LOG("Result image is NULL");
      delete r;
      return;
    }
    /// r and o is given !!
    if(r != rbef ){
      ERROR_LOG("Detected a pointer reallocation in Implicit Img<T> to ImgBase** cast operation\n"
                "This warning implicates, that a local ImgBase* was reallocated instead of using\n"
                "the given Img<T>. To enshure a maximum compability, the new result images data\n"
                "will be converted interanlly into the given image. To avoid this warning and to\n"
                "enhance performance, DO NOT USE THE \"bpp(..)\"-FUNCTION\n");
      printf("convert: \n");
      r->convert(o);
      printf("del \n");
      delete r;
      printf("ret \n");
      return;
    }
    if(o->getParams() != r->getParams()){
      // shallow copy back
      ImgBase *poBase = o;
      
      r->shallowCopy(&poBase);
      //      *o = *r;
      delete r;
      return;
    }
    
    delete r;
  }

  // }}}
  template<class T>
  ImgBasePtrPtr<T>::ImgBasePtrPtr(Img<T> &i){
    r = new Img<T>(i);
    o = &i;
    rbef = r;
  }
  
  template<class T>
  ImgBasePtrPtr<T>::ImgBasePtrPtr(Img<T> *i){
    // {{{ open

    if(!i){ 
      o = 0; 
      r = 0;
      rbef = 0;
    }else{
      r = new Img<T>(*i);
      o = i;
      rbef = r;
    }
  }

  // }}}

#define ICL_INSTANTIATE_DEPTH(D) template struct ImgBasePtrPtr<icl##D>;
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  
}
