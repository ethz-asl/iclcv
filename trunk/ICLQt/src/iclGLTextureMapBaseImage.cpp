#include <iclGLTextureMapBaseImage.h>


namespace icl{

  namespace{
    template<class T>
    inline void SAVE_DEL(T *&p){
      if(p){
        delete p;
        p = 0;
      }
    }

    int getCellSize(const Size &imageSize){
      if(imageSize.getDim() < 100) return 16;
      if(imageSize.getDim() < 2500) return 32;
      if(imageSize.getDim() < 10000) return 64;
      if(imageSize.getDim() < 3000000) return 128;
      return 256;
    }
    void append(const ImgBase *src, ImgBase *dst){
      switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: dst->asImg<icl##D>()->append(dst->asImg<icl##D>()); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }
  }
  

  GLTextureMapBaseImage::~GLTextureMapBaseImage(){
    SAVE_DEL(m_po8u);
    SAVE_DEL(m_po16s);
    SAVE_DEL(m_po32s);
    SAVE_DEL(m_po32f);
    SAVE_DEL(m_poChannelBuf);    
  }

  void GLTextureMapBaseImage::setSingleBufferMode(bool useSingleBuffer){
    if(useSingleBuffer!=m_bUseSingleBuffer){
      m_bUseSingleBuffer  = useSingleBuffer;
      SAVE_DEL(m_po8u);
      SAVE_DEL(m_po16s);
      SAVE_DEL(m_po32s);
      SAVE_DEL(m_po32f);
    }
  }
  
  const ImgBase *GLTextureMapBaseImage::adaptChannels(const ImgBase *image){
    WARNING_LOG("memory leak here!");
    ICLASSERT_RETURN_VAL(image && image->getChannels(), 0);
    switch(image->getChannels()){
      case 1: 
      case 3: 
      case 4: return image;
      case 2:  /// add an empty channel
        ensureCompatible(&m_poChannelBuf,image->getDepth(),image->getSize(),1);
        append(image,m_poChannelBuf);
        m_poChannelBuf->swapChannels(0,2);
        return m_poChannelBuf;
      default: // use first 3 channels
        std::vector<int> idxs; idxs.push_back(0);idxs.push_back(1);idxs.push_back(2);
        return const_cast<ImgBase*>(image)->selectChannels(idxs,&m_poChannelBuf);
    }
  }  

  void GLTextureMapBaseImage::updateTextures(const ImgBase *imageIn){
    const ImgBase *image = adaptChannels(imageIn);
    ICLASSERT_RETURN(image);

    m_oCurrentImageParams = imageIn->getParams();
    Size s = image->getSize();
    
#define APPLY_FOR(D)                                                    \
    if(m_po##D && !(m_po##D->compatible(image->asImg<icl##D>()))){      \
      SAVE_DEL(m_po##D);                                                \
    }                                                                   \
    if(!m_po##D){                                                       \
      m_po##D = new GLTextureMapImage<icl##D>(s,m_bUseSingleBuffer,image->getChannels(),getCellSize(s)); \
      m_po##D->bci(m_aiBCI[0],m_aiBCI[1],m_aiBCI[2]);                   \
    }                                                                   \
    m_po##D->updateTextures(image->asImg<icl##D>());                    \
    break;  
    
    switch(image->getDepth()){
      case depth8u:
        SAVE_DEL(m_po16s);
        SAVE_DEL(m_po32s);
        SAVE_DEL(m_po32f);
        APPLY_FOR(8u);
      case depth16s:
        SAVE_DEL(m_po8u);
        SAVE_DEL(m_po32s);
        SAVE_DEL(m_po32f);
        APPLY_FOR(16s);
      case depth32s:
        SAVE_DEL(m_po8u);
        SAVE_DEL(m_po16s);
        SAVE_DEL(m_po32f);
        APPLY_FOR(32s);
      case depth32f:
        SAVE_DEL(m_po8u);
        SAVE_DEL(m_po16s);
        SAVE_DEL(m_po32s);
        APPLY_FOR(32f);
      case depth64f:{ // fallback!!
        Img<icl32f> *tmp = image->convert(depth32f)->asImg<icl32f>();
        updateTextures(tmp);
        delete tmp;
      }default:
        ICL_INVALID_DEPTH;
        break;
    }    
  }
  
  Size GLTextureMapBaseImage::getSize() const{
    return m_oCurrentImageParams.getSize();
  }

  depth GLTextureMapBaseImage::getDepth() const{
    if(m_po8u)return depth8u;
    if(m_po16s)return depth16s;
    if(m_po32s)return depth32s;
    if(m_po32f)return depth32f;
    ERROR_LOG("getDepth() must not be called before an image was set!");
    return depth64f; //
  }
  
  int GLTextureMapBaseImage::getChannels() const{
    return m_oCurrentImageParams.getChannels();
  }
                          
  format GLTextureMapBaseImage::getFormat() const{
    return m_oCurrentImageParams.getFormat();  
  }
  
  Rect GLTextureMapBaseImage::getROI() const{
    return m_oCurrentImageParams.getROI();
  }

  std::vector<icl32f>  GLTextureMapBaseImage::getColor(int x, int y) const{
    if(m_po8u) return m_po8u->getColor(x,y);
    if(m_po16s) return m_po16s->getColor(x,y);
    if(m_po32s) return m_po32s->getColor(x,y);
    if(m_po32f) return m_po32f->getColor(x,y);
    return std::vector<icl32f>();
  }

  ImgBase *GLTextureMapBaseImage::deepCopy() const{
    ImgBase *image = 0;
    if(m_po8u) image = m_po8u->deepCopy();
    if(m_po16s) image = m_po16s->deepCopy();
    if(m_po32s) image = m_po32s->deepCopy();
    if(m_po32f) image = m_po32f->deepCopy();
    if(image) image->setFormat(m_oCurrentImageParams.getFormat());
    return image;
  }

  
  Range<icl32f> GLTextureMapBaseImage::getMinMax(int channel) const{
    if(m_po8u)return m_po8u->getMinMax(channel).castTo<icl32f>();
    if(m_po16s)return m_po16s->getMinMax(channel).castTo<icl32f>();
    if(m_po32s)return m_po32s->getMinMax(channel).castTo<icl32f>();
    if(m_po32f)return m_po32f->getMinMax(channel).castTo<icl32f>();
    ERROR_LOG("getMinMax must not be called before an image was set!");
    return Range<icl32f>(0,0);
  }
  

  void GLTextureMapBaseImage::bci(int b, int c, int i){
    m_aiBCI[0]= b;
    m_aiBCI[1]= c;
    m_aiBCI[2]= i;
    if(m_po8u) { m_po8u->bci(b,c,i); return; }
    if(m_po16s) { m_po16s->bci(b,c,i); return; }
    if(m_po32s) { m_po32s->bci(b,c,i); return; }
    if(m_po32f) { m_po32f->bci(b,c,i); return; }
  }

  void GLTextureMapBaseImage::drawTo(const Rect &rect, const Size &windowSize){
    if(m_po8u){
      m_po8u->drawTo(rect,windowSize);
      return;
    }
    if(m_po32f){
      m_po32f->drawTo(rect,windowSize);
      return;
    }
    if(m_po16s){
      m_po16s->drawTo(rect,windowSize);
      return;
    }
    if(m_po32s){
      m_po32s->drawTo(rect,windowSize);
      return;
    }
  }
  void GLTextureMapBaseImage::drawTo3D(float *pCenter, float *pFirstAxis, float *pSecondAxis){
    if(m_po8u){
      m_po8u->drawTo3D(pCenter,pFirstAxis,pSecondAxis);
      return;
    }
    if(m_po16s){
      m_po16s->drawTo3D(pCenter,pFirstAxis,pSecondAxis);
      return;
    }
    if(m_po32s){
      m_po32s->drawTo3D(pCenter,pFirstAxis,pSecondAxis);
      return;
    }
    if(m_po32f){
      m_po32f->drawTo3D(pCenter,pFirstAxis,pSecondAxis);
      return;
    }

  }
    

}
