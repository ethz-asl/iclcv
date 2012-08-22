/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQuick/src/Quick.cpp                                 **
** Module : ICLQuick                                               **
** Authors: Christof Elbrechter, Michael Götting                   **
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

#include <ICLQuick/Quick.h>
#include <ICLIO/FileGrabber.h>
#include <ICLIO/TestImages.h>
#include <ICLCC/Converter.h>

#include <ICLIO/GenericGrabber.h>

#include <ICLCC/CCFunctions.h>
#include <map>
#include <list>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLFilter/MorphologicalOp.h>
#include <ICLFilter/BinaryArithmeticalOp.h>
#include <ICLFilter/UnaryArithmeticalOp.h>
#include <ICLIO/FileWriter.h>
#include <ICLFilter/UnaryCompareOp.h>
#include <ICLFilter/LUTOp.h>
#include <ICLUtils/Timer.h>

#ifdef HAVE_QT
#include <ICLQt/QImageConverter.h>
#include <QtGui/QPainter>
#include <QtGui/QImage>
#include <QtGui/QFont>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#endif

#include <ICLCore/Line.h>
#include <ICLUtils/Point32f.h>
#include <ICLQuick/ImgBuffer.h>

#include <fstream>
#include <cstdio>



namespace icl{

  std::string execute_process(const std::string &command){
    char buf[128];
    FILE *f = popen(command.c_str(),"r");
    std::ostringstream out;
    while( !feof(f) ){
      memset(buf,0,128);
      fread(buf,1,127,f);
      out << buf;
    }
    fclose(f);
    return out.str();
  }


#ifdef HAVE_QT
  std::string openFileDialog(const std::string &filter,const std::string &caption, 
                             const std::string &initialDirectory, void *parentWidget) throw (ICLException){
    QString f = QFileDialog::getOpenFileName((QWidget*)parentWidget, caption.c_str(), initialDirectory.c_str(),
                                filter.c_str() );
    if(f.isNull() || !f.length()) throw ICLException("no file selected in openFileDialog or cancel was pressed. This exception must be caught explicitly!");
    return f.toLatin1().data();
  }
  std::string saveFileDialog(const std::string &filter,const std::string &caption, 
                             const std::string &initialDirectory,void *parentWidget) throw (ICLException){
    QString f = QFileDialog::getSaveFileName((QWidget*)parentWidget, caption.c_str(), initialDirectory.c_str(),
                                filter.c_str() );
    if(f.isNull() || !f.length()) throw ICLException("no file selected or saveFileDialog or cancel was pressed.This exception must be caught explicitly!");
    return f.toLatin1().data();
    
  }
#endif


  namespace {


    static std::list<ImgQ> TEMP_IMAGES;
    
    ImgQ &get_temp_image(const ImgParams &params){
      ImgQ * independentOne = 0;
      for(std::list<ImgQ>::iterator it = TEMP_IMAGES.begin(); it != TEMP_IMAGES.end();++it){
        if(it->isIndependent()){
          if(it->getParams() == params){
            //DEBUG_LOG("reusing image with given params");
            return *it;
          }else{
            if(independentOne){
              independentOne = &*it;
            }
          }
        }
      }
      if(independentOne){
        //DEBUG_LOG("reusing independent image (changing parameters)");
        independentOne->setParams(params);
        return *independentOne;
      }else{
        //DEBUG_LOG("creating new image");
        TEMP_IMAGES.push_back(ImgQ(params));
        return TEMP_IMAGES.back();
      }
    }
#if 0
    /// obviously, this is not used!
    ImgQ &get_temp_image(){
      for(std::list<ImgQ>::iterator it = TEMP_IMAGES.begin(); it != TEMP_IMAGES.end();++it){
        if(it->isIndependent()){
          return *it;
        }
      }
      TEMP_IMAGES.push_back(ImgQ());
      return TEMP_IMAGES.back();
    }
#endif

    inline ImgQ get_temp_image(const Size &size, int channels){
      return get_temp_image(ImgParams(size,channels));
    }

#define TEMP_IMG_P(params) get_temp_image(params) 
#define TEMP_IMG_SC(size,channels) get_temp_image(size,channels) 

#define TEMP_IMG_PTR_P(params) new ImgQ(get_temp_image(params))
#define TEMP_IMG_PTR_SC(size,channels) new ImgQ(get_temp_image(size,channels))

    
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


    inline ImgQ *prepare_for_binary_op(const ImgQ &a, const ImgQ &b, ImgQ &na, ImgQ &nb){
      // {{{ open

      if(a.getROISize() == b.getROISize() && a.getChannels() == b.getChannels()){
        na = copy(a);
        nb = copy(b);
        //return new ImgQ(a.getParams());
        return TEMP_IMG_PTR_P(a.getParams());
      }

    Size sa = a.getROISize(), sb = b.getROISize();
    Size sr(max(sa.width,sb.width), max(sa.height,sb.height) );
    int cr = max(a.getChannels(),b.getChannels());
      
    //    na = ImgQ(sr,a.getChannels());
    na = TEMP_IMG_SC(sr,a.getChannels());
    na.setROI(a.getROI());
    a.deepCopyROI(&na);
    na.setFullROI();
    na.setChannels(cr);


    //nb = ImgQ(sr,b.getChannels());
    nb = TEMP_IMG_SC(sr,b.getChannels());
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
        //ImgBase *res = new ImgQ(a.getParams());
        ImgBase *res = TEMP_IMG_PTR_P(a.getParams());
        binop.apply(&a,&b,&res);
        ImgQ x = *(res->asImg<ICL_QUICK_TYPE>());
        delete res;
        return x;
      }
      
      Size sa = a.getROISize(), sb = b.getROISize();
      Size sr(max(sa.width,sb.width), max(sa.height,sb.height) );
      int cr = max(a.getChannels(),b.getChannels());
      
      //      ImgQ na(sr,a.getChannels());
      ImgQ na = TEMP_IMG_SC(sr,a.getChannels());
      na.setROI(a.getROI());
      a.deepCopyROI(&na);
      na.setFullROI();
      na.setChannels(cr);
      
      
      //      ImgQ nb(sr,b.getChannels());
      ImgQ nb = TEMP_IMG_SC(sr,b.getChannels());
      nb.setROI(b.getROI());
      b.deepCopyROI(&nb);
      nb.setFullROI();
      nb.setChannels(cr);
      
      //      ImgBase * res = new ImgQ(na.getParams());
      ImgBase *res = TEMP_IMG_PTR_P(na.getParams());
      
      binop.apply(&na,&nb,&res);
      ImgQ x = *(res->asImg<ICL_QUICK_TYPE>());
      delete res;
      return x;
    }

    // }}}
    inline ImgQ apply_unary_arithmetical_op(const ImgQ &image,ICL_QUICK_TYPE val, UnaryArithmeticalOp::optype ot){
      // {{{ open

      /* old  !!
          ImgBase *dst = 0;
          UnaryArithmeticalOp(ot,val).apply(&image,&dst);
          ImgQ r = *(dst->asImg<ICL_QUICK_TYPE>());
          delete dst;
          return r;
      */
      ImgQ dst = TEMP_IMG_P(image.getParams());
      UnaryArithmeticalOp(ot,val).apply(&image,bpp(dst));
      return dst;
      
    }

    // }}}

    Timer *TIMER=0;      
  }

  using namespace std;
  
  template <class T>
  Img<T> zeros(int width, int height, int channels){
    // {{{ open
    Img<T> &image = *ImgBuffer::instance()->get<T>(Size(width,height),channels);
    image.clear(-1,T(0),false);
    return image;
  }

  // }}}

  template <class T>
  Img<T> ones(int width, int height, int channels){
    // {{{ open
    Img<T> &image = *ImgBuffer::instance()->get<T>(Size(width,height),channels);
    image.clear(-1,T(1),false);
    return image;
  }

  // }}}




  template<class T>
  Img<T> load(const string &filename){
    FileGrabber g(filename);
    const ImgBase *grabbedImage = 0;
    try{
      grabbedImage = g.grab();
    }catch(const ICLException &ex){
      ERROR_LOG("exception: "  << ex.what());
    }
    if(!grabbedImage){
      return Img<T>();
    }
    Img<T> buf = *ImgBuffer::instance()->get<T>(grabbedImage->getParams());
    grabbedImage->convert(&buf);
    return buf;
  }


  
  template<class T>
  Img<T> load(const string &filename, format fmt){
    // {{{ open

    FileGrabber g(filename);
    const ImgBase *gi  = 0;
    try{
      gi = g.grab();
    }catch(const ICLException &ex){
      ERROR_LOG("exception: "  << ex.what());
    }
    if(!gi){
      return Img<T>();
    }
    Img<T> buf = *ImgBuffer::instance()->get<T>(gi->getSize(),getChannelsOfFormat(fmt));
    buf.setFormat(fmt);
    cc(gi,&buf);

    return buf;
  }

  // }}}
  

  template<class T>
  Img<T> create(const string &name, format fmt){
    // {{{ open
    depth d = getDepth<T>();
    Img<T> *image = TestImages::create(name,fmt,d)->asImg<T>();
    if(!image){
      ERROR_LOG("unable to create test image: \"" << name << "\"");
      return Img<T>();
    }
    Img<T> im = *image;
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

  
  ImgQ cvt(const ImgBase *image){
    ICLASSERT_RETURN_VAL(image, ImgQ() );
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return cvt(*(image->asImg<icl##D>()));
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: 
        ICL_INVALID_DEPTH;
    }
    return ImgQ();
  }

  ImgQ cvt(const ImgBase &image){
    return cvt(&image);
  }



  template<class T>
  Img<T> grab(const std::string &dev, const std::string &devSpec, 
            const Size &size, format fmt, bool releaseGrabber){
    static std::map<std::string,SmartPtr<GenericGrabber> > grabbers;
    
    SmartPtr<GenericGrabber> g;
    std::string id;
    if(devSpec.substr(0,dev.length()) != (dev+"=")){
      id = dev+dev+"="+devSpec;
    }else{
      id = dev+devSpec;
    }
    std::map<std::string,SmartPtr<GenericGrabber> >::iterator it = grabbers.find(id);
    if(it != grabbers.end()){
      g = it->second;
    }else{
      g = new GenericGrabber(dev,devSpec);
      if(!releaseGrabber){
        grabbers[id] = g;
      }
    }
    Img<T> back;
    if(size != Size::null){
      g->useDesired(size);
      g->useDesired(fmt);
      g->useDesired(getDepth<T>());
      back = *g->grab()->asImg<T>();
    }else{
      const ImgBase *image = g->grab();
      back.setSize(image->getSize());
      back.setFormat(fmt);
      cc(image,&back);
    }
    return back;
  }

  


  
  template<class T>
  Img<T> filter(const Img<T> &image,const string &filter){
    // {{{ open

    static map<string,UnaryOp*> M;
    if(!M.size()){
      static Size s3x3(3,3);
      M["sobely"] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::sobelX3x3),true);
      M["sobelx"] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::sobelY3x3),true);
      M["gauss"] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::gauss3x3),true);
      M["laplace"] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::laplace3x3),true);
      M["median"] = new MedianOp(Size(3,3));
      M["dilation"] = new MorphologicalOp(MorphologicalOp::dilate);
      M["erosion"] = new MorphologicalOp(MorphologicalOp::erode);
      M["opening"] = new MorphologicalOp(MorphologicalOp::openBorder);
      M["closing"] = new MorphologicalOp(MorphologicalOp::closeBorder);

    }    
    UnaryOp* u = M[filter];
    if(!u){
      WARNING_LOG("nothing known about filter type:" << filter);
      return Img<T>();
    }
    u->setClipToROI(true);
    u->setCheckOnly(true);
    
    Img<T> &buf = *ImgBuffer::instance()->get<T>(image.getSize()-Size(2,2),image.getChannels());
    buf.setFormat(image.getFormat());    
    
    u->apply(&image,bpp(buf));
    return buf;
  } 
  // }}}

  
  template<class T>
  Img<T> blur(const Img<T> &image, int maskRadius){
    if(maskRadius == 1) return filter(image,"gauss");
    std::vector<int> k(maskRadius*2+1);
    const float sigma2 = 2*(maskRadius/2*maskRadius/2);
    int sum = 0;
    for(unsigned int i=0;i<k.size();++i){
      float d = ((int)i)-maskRadius;
      k[i] = 255.0 * ::exp( - d*d / sigma2);
      sum += k[i];
    }
    ConvolutionOp c_horz(ConvolutionKernel(k.data(),Size(k.size(),1),iclMax(1,sum),false));
    ConvolutionOp c_vert(ConvolutionKernel(k.data(),Size(1,k.size()),iclMax(1,sum),false));

    const ImgBase *result = c_horz.apply(c_vert.apply(&image));
    if(result->getDepth() == getDepth<T>()){
      switch(getDepth<T>()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return *result->asImg<T>();
        ICL_INSTANTIATE_ALL_DEPTHS
        default:
        ICL_INVALID_DEPTH;
      }
#undef ICL_INSTANTIATE_DEPTH
    }else{
      Img<T> resultT;
      switch(getDepth<T>()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: result->convert(&resultT); break;
        ICL_INSTANTIATE_ALL_DEPTHS
        default:
        ICL_INVALID_DEPTH;
      }
#undef ICL_INSTANTIATE_DEPTH
      return resultT;
    }
    return Img<T>(); // just to avoid complains by the compiler
  }


  template<class T>
  Img<T> copy(const Img<T> &image){
    // {{{ open
    Img<T> &cpy = *ImgBuffer::instance()->get<T>(image.getParams());
    image.deepCopy(&cpy);
    return cpy;
  }

  // }}}
  template<class T>
  Img<T> copyroi(const Img<T> &image){
    // {{{ open
    Img<T> &cpy = *ImgBuffer::instance()->get<T>(image.getROISize(),image.getChannels());
    cpy.setFormat(image.getFormat());
    cpy.setTime(image.getTime());
    image.deepCopyROI(&cpy);
    return cpy;
  }




  // }}}
  template<class T>
  Img<T> norm(const Img<T> &image){
    // {{{ open

    Img<T> cpy = copy(image);
    cpy.normalizeAllChannels(Range<T>(0,255));
    return cpy;
  }

  // }}}
  
  template<class T>
  void save(const Img<T> &image,const string &filename){
    // {{{ open
    FileWriter(filename).write(&image);
  }

  // }}}
  
  namespace{
    string g_sShowCommand = "icl-xv -input %s -delete";
    string g_sRmCommand = "";
    int g_iMsecBeforeDelete = 0;
  }

  void showSetup(const string &showCommand, const string &rmCommand, int msecBeforeDelete){
    g_sShowCommand = showCommand;
    g_sRmCommand = rmCommand;
    g_iMsecBeforeDelete = msecBeforeDelete;
  }  
  
  template<class T>
  void show(const Img<T> &image){
    // {{{ open
    
    if(image.hasFullROI()){
      if(image.getFormat()==formatMatrix && image.getChannels()==1){
        Img<T> tmp = image;
        tmp.setFormat(formatGray);
        TestImages::show(&tmp,g_sShowCommand, g_iMsecBeforeDelete,g_sRmCommand);
      }else if(image.getFormat() == formatMatrix && image.getChannels()==3){
        Img<T> tmp = image;
        tmp.setFormat(formatRGB);

        TestImages::show(&tmp,g_sShowCommand, g_iMsecBeforeDelete,g_sRmCommand);       
      }else{
        TestImages::show(&image,g_sShowCommand, g_iMsecBeforeDelete,g_sRmCommand);
      }
    }else{
      Img<T> Ti = copy(image);
      if(image.getFormat()==formatMatrix && image.getChannels()==1){
        Ti.setFormat(formatGray);
      }else if(image.getFormat()==formatMatrix && image.getChannels()==3){
        Ti.setFormat(formatRGB);
      }
      saveColorAndFill();
      color(255,0,0);
      fill(0,0,0,0);
#warning "unable to visualize ROI in show!"
      //      rect(Ti,Ti.getROI());
      restoreColorAndFill();
      Ti.setFullROI();
      
      TestImages::show(&Ti,g_sShowCommand, g_iMsecBeforeDelete,g_sRmCommand);
    }
  }



  // }}}

  template<class T>
  void print(const Img<T> &image){
    // {{{ open

    image.print("image");
  }

  // }}}

#define ICL_INSTANTIATE_DEPTH(D)                                   \
  template Img<icl##D> zeros(int,int,int);                         \
  template Img<icl##D> ones(int,int,int);                          \
  template Img<icl##D> load(const std::string&);                   \
  template Img<icl##D> load(const std::string&,format);            \
  template Img<icl##D> create(const std::string&,format);          \
  template Img<icl##D> grab(const std::string&,const std::string&, \
                            const Size&,format,bool);              \
  template Img<icl##D> filter(const Img<icl##D>&,                  \
                              const std::string&);                 \
  template Img<icl##D> blur(const Img<icl##D>&,int);               \
  template Img<icl##D> copy(const Img<icl##D>&);                   \
  template Img<icl##D> copyroi(const Img<icl##D>&);                \
  template void show(const Img<icl##D>&);                          \
  template void save(const Img<icl##D>&,const string&);            \
  template void print(const Img<icl##D>&);                         \
  template Img<icl##D> norm(const Img<icl##D>&);
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

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
    //    ImgQ res(image.getROISize(),image.getChannels());

    ImgQ res = TEMP_IMG_SC(image.getROISize(),image.getChannels());
    
    for(int c=0;c<image.getChannels();++c){
      const ImgIterator<ICL_QUICK_TYPE> itI = image.beginROI(c);
      const ImgIterator<ICL_QUICK_TYPE> itIEnd = image.endROI(c);
      ImgIterator<ICL_QUICK_TYPE> itRes = res.beginROI(c);
      for(;itI != itIEnd ;++itI,++itRes){
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
    //ImgQ res(image.getROISize(),image.getChannels());
    
    ImgQ res = TEMP_IMG_SC(image.getROISize(),image.getChannels());
    
    for(int c=0;c<image.getChannels();++c){
      const ImgIterator<ICL_QUICK_TYPE> itI = image.beginROI(c);
      const ImgIterator<ICL_QUICK_TYPE> itIEnd = image.endROI(c);
      ImgIterator<ICL_QUICK_TYPE> itRes = res.beginROI(c);
      for(;itI != itIEnd;++itI,++itRes){
        *itRes = ((*itI) == 0) ? 0 :  val / (*itI);
      }
    }
    return res;
  }

  // }}}


  ImgQ cc(const ImgQ& image, format fmt){
    // {{{ open
    //    ImgQ dst(image.getSize(),fmt);
    ImgQ dst = TEMP_IMG_SC(image.getROISize(),getChannelsOfFormat(fmt));
    dst.setFormat(fmt);

    ImgQ src = image;//copyroi(image); // just a shallow copy here!
    if(src.getFormat() == formatMatrix && src.getChannels()==1) src.setFormat(formatGray);
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
  
  // TODO ...
  
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

    //ImgQ r(image.getROISize(),image.getChannels(),image.getFormat());
    ImgQ r = TEMP_IMG_SC(image.getROISize(),image.getChannels());
    r.setFormat(image.getFormat());
    r.setTime(image.getTime());
    for(int c=0;c<image.getChannels();++c){
      const ImgIterator<ICL_QUICK_TYPE> itSrc = image.beginROI(c);
      const ImgIterator<ICL_QUICK_TYPE> itSrcEnd = image.endROI(c);
      ImgQ::iterator itDst = r.begin(c);
      for(;itSrc != itSrcEnd ;++itSrc,++itDst){
        *itDst = 255*(*itSrc > threshold);
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
      ImgIterator<ICL_QUICK_TYPE> itA = na.beginROI(c);
      const ImgIterator<ICL_QUICK_TYPE> itAEnd = na.endROI(c);
      ImgIterator<ICL_QUICK_TYPE> itB = nb.beginROI(c);
      ImgIterator<ICL_QUICK_TYPE> itR = r.beginROI(c);
      for(;itA != itAEnd;++itA,++itB, ++itR){
        *itR = 255*( (*itA>0) || (*itB>0) );
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
      ImgIterator<ICL_QUICK_TYPE> itA = na.beginROI(c);
      const ImgIterator<ICL_QUICK_TYPE> itAEnd = na.endROI(c);
      ImgIterator<ICL_QUICK_TYPE> itB = nb.beginROI(c);
      ImgIterator<ICL_QUICK_TYPE> itR = r.beginROI(c);
      for(;itA != itAEnd;++itA,++itB, ++itR){
        *itR = 255* ((*itA>0) && (*itB>0) );
      }
    }
    return r;
  }

  // }}}

  template<class T>
  ImgQ binOR(const ImgQ &a, const ImgQ &b){
    // {{{ open
    ImgQ na,nb;
    ImgQ *res = prepare_for_binary_op(a,b,na,nb);
    ImgQ r = *res;
    delete res;
    for(int c=0;c<a.getChannels();c++){
      ImgIterator<ICL_QUICK_TYPE> itA = na.beginROI(c);
      const ImgIterator<ICL_QUICK_TYPE> itAEnd = na.endROI(c);
      ImgIterator<ICL_QUICK_TYPE> itB = nb.beginROI(c);
      ImgIterator<ICL_QUICK_TYPE> itR = r.beginROI(c);
      for(;itA != itAEnd;++itA,++itB, ++itR){
        T val = clipped_cast<ICL_QUICK_TYPE,T>(*itA) | clipped_cast<ICL_QUICK_TYPE,T>(*itB);
        *itR = clipped_cast<T,ICL_QUICK_TYPE>(val);
      }
    }
    return r;
  }

  // }}}

  template<class T>
  ImgQ binXOR(const ImgQ &a, const ImgQ &b){
    // {{{ open
    ImgQ na,nb;
    ImgQ *res = prepare_for_binary_op(a,b,na,nb);
    ImgQ r = *res;
    delete res;
    for(int c=0;c<a.getChannels();c++){
      ImgIterator<ICL_QUICK_TYPE> itA = na.beginROI(c);
      const ImgIterator<ICL_QUICK_TYPE> itAEnd = na.endROI(c);
      ImgIterator<ICL_QUICK_TYPE> itB = nb.beginROI(c);
      ImgIterator<ICL_QUICK_TYPE> itR = r.beginROI(c);
      for(;itA != itAEnd;++itA,++itB, ++itR){
        T val = clipped_cast<ICL_QUICK_TYPE,T>(*itA) ^ clipped_cast<ICL_QUICK_TYPE,T>(*itB);
        *itR = clipped_cast<T,ICL_QUICK_TYPE>(val);
      }
    }
    return r;
  }

  // }}}
  
  template<class T>
  ImgQ binAND(const ImgQ &a, const ImgQ &b){
    // {{{ open
    ImgQ na,nb;
    ImgQ *res = prepare_for_binary_op(a,b,na,nb);
    ImgQ r = *res;
    delete res;
    for(int c=0;c<a.getChannels();c++){
      ImgIterator<ICL_QUICK_TYPE> itA = na.beginROI(c);
      const ImgIterator<ICL_QUICK_TYPE> itAEnd = na.endROI(c);
      ImgIterator<ICL_QUICK_TYPE> itB = nb.beginROI(c);
      ImgIterator<ICL_QUICK_TYPE> itR = r.beginROI(c);
      for(;itA != itAEnd;++itA,++itB, ++itR){
        T val = clipped_cast<ICL_QUICK_TYPE,T>(*itA) & clipped_cast<ICL_QUICK_TYPE,T>(*itB);
        *itR = clipped_cast<T,ICL_QUICK_TYPE>(val);
      }
    }
    return r;
  }

  // }}}

  template ImgQ binOR<icl8u>(const ImgQ&, const ImgQ&);
  template ImgQ binOR<icl16s>(const ImgQ&, const ImgQ&);
  template ImgQ binOR<icl32s>(const ImgQ&, const ImgQ&);

  template ImgQ binAND<icl8u>(const ImgQ&, const ImgQ&);
  template ImgQ binAND<icl16s>(const ImgQ&, const ImgQ&);
  template ImgQ binAND<icl32s>(const ImgQ&, const ImgQ&);

  template ImgQ binXOR<icl8u>(const ImgQ&, const ImgQ&);
  template ImgQ binXOR<icl16s>(const ImgQ&, const ImgQ&);
  template ImgQ binXOR<icl32s>(const ImgQ&, const ImgQ&);


  ImgQ operator,(const ImgQ &a, const ImgQ &b){
    // {{{ open
    if(a.getSize() == Size::null) return copy(b);
    if(b.getSize() == Size::null) return copy(a);
    //ImgQ r = zeros();
    ImgQ r = TEMP_IMG_SC(Size(a.getWidth()+b.getWidth(),max(a.getHeight(),b.getHeight())),max(a.getChannels(),b.getChannels()));
    
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

  void colorinfo(float color[4], float fill[4]){
    // {{{ open
    memcpy(color,COLOR,4*sizeof(float));
    memcpy(fill,FILL,4*sizeof(float));
  }
  // }}}

  void cross(ImgQ &image, int X, int Y){
    // {{{ open

    static const int CROSS_SIZE = 3;
    line(image, X-CROSS_SIZE,Y-CROSS_SIZE,X+CROSS_SIZE,Y+CROSS_SIZE);
    line(image, X-CROSS_SIZE,Y+CROSS_SIZE,X+CROSS_SIZE,Y-CROSS_SIZE);
  }

  // }}}
  
  static void simple_rect(ImgQ &image, int x, int y, int w, int h){
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

  void rect(ImgQ &image, int x_in, int y_in, int w_in, int h_in, int r){
    // {{{ open
    if(!r){
      icl::simple_rect(image,x_in,y_in,w_in,h_in);
      return;
    }
    const Rect rect(x_in,y_in,w_in,h_in);

    float c[4],f[4];
    colorinfo(c,f);
    color(f[0],f[1],f[2],f[3]);
    
    if(r > iclMin(rect.width/2,rect.height/2)){
      r = iclMin(rect.width/2,rect.height/2);
    }
    
    icl::simple_rect(image,rect.x+r,rect.y, rect.width-2*r, rect.height);
    icl::simple_rect(image,rect.x,rect.y+r,r,rect.height-2*r);
    icl::simple_rect(image,rect.x+rect.width-r,rect.y+r,
                     r,rect.height-2*r);
    
    color(f[0],f[1],f[2],f[3]);
    
    int xr = rect.x+rect.width-r;
    int yb = rect.y+rect.height-r-1;
    int k,x,s,ybry;
    for(int y=0;y<r;++y){
      // upper left
      k = r-y;
      s = ::sqrt(r*r - k*k);
      ybry = yb+(r-y);
      x = r - s;
      
      icl::line(image,x+rect.x,y+rect.y,r+rect.x,y+rect.y);
      
      // upper right
      x = s;
      icl::line(image,xr,y+rect.y,xr+x-1,y+rect.y);
      
      // lower left
      x = r - s;
      icl::line(image,rect.x+x,ybry,rect.x+r,ybry);
      
      // lower right
      x = s;
      icl::line(image,xr,ybry,xr+x-1,ybry);
    }
    
    // Draw Boundary ...
    color(c[0],c[1],c[2],c[3]);
    
    icl::line(image,rect.x+r, rect.y, xr, rect.y);
    icl::line(image,rect.x+r, yb+r, xr, yb+r);
    icl::line(image,rect.x, rect.y+r, rect.x, yb);
    icl::line(image,rect.x+rect.width-1, rect.y+r, rect.x+rect.width-1, yb);
    
    for(int y=0;y<r;++y){
      // upper left
      k = r-y;
      s = ::sqrt(r*r - k*k);
      ybry = yb+(r-y);
      x = r - s;
      
      icl::pix(image, x+rect.x, y+rect.y);
      icl::pix(image, y+rect.x, x+rect.y);
      
      // upper right
      x = s;
      icl::pix(image,xr+x-1,y+rect.y);
      if(y!=r-1) icl::pix(image,xr+(r-y-1),(r-x)+rect.y);
      
      
      // lower left
      x = r - s;
      icl::pix(image,rect.x+x,ybry);
      icl::pix(image,rect.x+y,yb+(r-x));
      
      // lower right
      x = s;
      icl::pix(image,xr+x-1,ybry);
      if(y!=r-1) icl::pix(image,xr+(r-y)-1,yb+x);
    }
  }
  // }}}




  namespace{
    void draw_circle_outline(ImgQ &image, int xcenter, int ycenter, int radius){
      // {{{ open

      float outline = 2*M_PI*radius;
      int nc = iclMin(3,image.getChannels());
      float A = COLOR[3]/255;
      int maxx = image.getWidth()-1;
      int maxy = image.getHeight()-1;
      for(float f=0;f<2*M_PI;f+=1/outline){
        for(int c=0;c<nc;c++){
          int x = (int)round(xcenter+cos(f)*radius);
          if(x<0 || x > maxx) continue;
          int y = (int)round(ycenter+sin(f)*radius);
          if(y<0 || y > maxy) continue;
          ICL_QUICK_TYPE &v = image(x,y,c);
          v=(1.0-A)*v + A*COLOR[c];
        }
      }
    }

    // }}}

    inline bool lessPt(const Point &a, const Point &b){
      // {{{ open

      return a.y<b.y;
    }

    // }}}

    void hline(ImgQ &image, int x1, int x2, int y, bool useFillColor){
      // {{{ open
      if( y < 0 || y >= image.getHeight()) return;
      if(x1 > x2) std::swap(x1,x2);
      const float *color = useFillColor ? FILL : COLOR;
      float A = color[3]/255.0;
      int cMax = iclMin(image.getChannels(),3);
      int xEnd = iclMin(x2,image.getWidth());
      for(int x=iclMax(x1,0);x<=xEnd;++x){
        for(int c=0;c<cMax; ++c){
          float &v = image(x,y,c);
          v=(1.0-A)*v + A*color[c];
        }
      }
    }

    // }}}
    inline void hlinef(ImgQ &image, float x1, float x2, float y,bool useFillColor){
      // {{{ open

      hline(image,(int)round(x1), (int)round(x2), (int)round(y), useFillColor);
    }

    // }}}
    void vline(ImgQ &image, int x, int y1, int y2,bool useFillColor){
      // {{{ open
      if( x < 0 || x >= image.getWidth()) return;
      if(y1 > y2) std::swap(y1,y2);
      const float *color = useFillColor ? FILL : COLOR;
      float A = color[3]/255.0;
      int cMax = iclMin(image.getChannels(),3);
      int yEnd = iclMin(y2,image.getHeight()-1);
      for(int y=iclMax(y1,0);y<=yEnd;++y){
        for(int c=0;c<cMax; ++c){
          float &v = image(x,y,c);
          v=(1.0-A)*v + A*color[c];
        }
      }
    }

    // }}}

  }
  
  void circle(ImgQ &image, int xoffs, int yoffs, int radius) {
    // {{{ open
    
    float rr = radius*radius;
    int h = image.getHeight();
    int w = image.getWidth();
    
    int ystart = iclMax(-radius,-yoffs);
    int yend = iclMin(radius,h-yoffs);
    int nc = iclMin(image.getChannels(),3);
    for(int dy = ystart;dy<=yend;++dy){
      int y = dy+yoffs;
      int dx = (int)round(::sqrt(rr-dy*dy));
      float A = FILL[3]/255;
      int xend = iclMin(xoffs+dx,w-1);
      for(int x=iclMax(0,xoffs-dx);x<=xend;++x){
        for(int c=0;c<nc;++c){
          ICL_QUICK_TYPE &v = image(x,y,c);
          v=(1.0-A)*v + A*FILL[c];
        }
      }
    }
    draw_circle_outline(image,xoffs,yoffs,radius);


    /** OLD Qt implementation
        int n = 0;
        char ** ppc = 0;
        if(!qApp){
        new QApplication(n,ppc);
        }
        static QSize *br = new QSize(radius * 2, radius * 2);
        
        QImage img(br->width()+2,br->height()+2,QImage::Format_ARGB32_Premultiplied);
        img.fill(0);
        
        QPainter painter(&img);
        painter.setPen(QColor(255,255,255,254));
        painter.setBrush(QColor(255, 255, 255, 254));
        painter.drawEllipse(QRectF(0, 0, radius * 2, radius * 2));
        painter.end();
        
        // then transfer the rendered image into the given image
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
     **/
  }

  // }}}

  void line(ImgQ &image, int x1, int y1, int x2, int y2){
    // {{{ open
    if(x1 == x2) { vline(image,x1,y1,y2,false); return; }
    if(y1 == y2) { hline(image,x1,x2,y1,false); return; }
    std::vector<int> xs,ys;
    Line l(Point(x1,y1), Point(x2,y2));
    l.sample(xs,ys,Rect(0,0,image.getWidth(), image.getHeight()));
    float A = COLOR[3]/255.0;
    for(vector<int>::iterator itX=xs.begin(), itY=ys.begin(); itX != xs.end(); ++itX, ++itY){
      for(int c=0;c<image.getChannels() && c<3; ++c){
        //        if(*itX >= 0 && *itX < image.getWidth() && *itY >= 0 && *itY <= image.getHeight()){
        float &v = image(*itX,*itY,c);
        v=(1.0-A)*v + A*COLOR[c];
        //      }
      }
    }
  }

  // }}}

  void linestrip(ImgQ &image, const std::vector<Point> &pts, bool closeLoop){
    /// {{{ open
    if(!pts.size()) return;
    for(unsigned int i=0;i<pts.size()-1;++i){
      line(image,pts[i],pts[i+1]);
    }
    if(closeLoop){
      line(image,pts.front(),pts.back());
    }    
  }
  // }}}
  
  void triangle(ImgQ &image,int x1, int y1, int x2, int y2, int x3, int y3 ){
    // {{{ open

    // *  the coordinates of vertices are (A.x,A.y), (B.x,B.y), (C.x,C.y); 
    //we assume that A.y<=B.y<=C.y (you should sort them first)
    //* dx1,dx2,dx3 are deltas used in interpolation
    //* horizline draws horizontal segment with coordinates (S.x,Y), (E.x,Y)
    //* S.x, E.x are left and right x-coordinates of the segment we have to draw
    //* S=A means that S.x=A.x; S.y=A.y; 
    if(FILL[3] != 0){
      if(x1==x2 && y1 == y2){ line(image,x1,y1,x3,y3); return; }
      if(x1==x3 && y1 == y3){ line(image,x1,y1,x2,y2); return; }
      if(x2==x3 && y2 == y3){ line(image,x1,y1,x3,y3); return; }
      
      vector<Point> v(3);
      v[0] = Point(x1,y1);
      v[1] = Point(x2,y2);
      v[2] = Point(x3,y3);
      std::sort(v.begin(),v.end(),lessPt);
      
      Point A = v[0];
      Point B = v[1];
      Point C = v[2];
      
      float dx1,dx2,dx3;
      
      if (B.y-A.y > 0){
        dx1=float(B.x-A.x)/(B.y-A.y);
      }else{
        dx1=B.x - A.x;
      }
      if (C.y-A.y > 0){
        dx2=float(C.x-A.x)/(C.y-A.y);
      }else{
        dx2=0;
      }
      if (C.y-B.y > 0){
        dx3=float(C.x-B.x)/(C.y-B.y);
      }else{
        dx3=0;
      }
      
      Point32f S = Point32f(A.x,A.y);
      Point32f E = Point32f(A.x,A.y);
      if(dx1 > dx2) {
        for(;S.y<=B.y;S.y++,E.y++,S.x+=dx2,E.x+=dx1){
          hlinef(image,S.x,E.x,S.y,true);
        }
        E=Point32f(B.x,B.y);
        for(;S.y<=C.y;S.y++,E.y++,S.x+=dx2,E.x+=dx3){
          hlinef(image,S.x,E.x,S.y,true);
        }
      }else {
        for(;S.y<=B.y;S.y++,E.y++,S.x+=dx1,E.x+=dx2){
          hlinef(image,S.x,E.x,S.y,true);
        }
        S=Point32f(B.x,B.y);
        for(;S.y<=C.y;S.y++,E.y++,S.x+=dx3,E.x+=dx2){
          hlinef(image,S.x,E.x,S.y,true);
        }
      }
    }
    if(COLOR[3] != 0){
      line(image,x1,y1,x2,y2);
      line(image,x1,y1,x3,y3);
      line(image,x2,y2,x3,y3);
    }
  }

  // }}}

  void text(ImgQ &image, int xoffs, int yoffs, const string &text){
    // {{{ open
    // first rendering the text 
#ifdef HAVE_QT
    int n = 0;
    char ** ppc = 0;
    if(!qApp){
      new QApplication(n,ppc);
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
#else
    (void)image;(void)xoffs; (void)yoffs; (void)text; 
    ERROR_LOG("this feature is not supported without Qt-Support");
#endif
  }

  // }}}

  void pix(ImgQ &image, int x, int y){
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
 
  void pix(ImgQ &image, const vector<vector<Point> > &pts){
    // {{{ open

    for(vector<vector<Point> >::const_iterator it = pts.begin();it!=pts.end();++it){
      pix(image,*it);
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


}
