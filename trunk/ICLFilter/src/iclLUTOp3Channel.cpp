#include "iclLUTOp3Channel.h"
#include <iclException.h>
#include <math.h>

namespace icl{  

  template<class T> 
  LUTOp3Channel<T>::LUTOp3Channel(Plugin *p,icl8u shift):m_poPlugin(0),m_ucShift(shift){
    ICLASSERT_THROW(shift<8,ICLException("invalid shift value ( must be < 8 )"));
    int dim = (int)pow(256>>m_ucShift,3);
    m_oLUT = Img<T>(Size(dim,1),1);
    setPlugin(p);
  }
  
  template<class T>
  LUTOp3Channel<T>::~LUTOp3Channel(){
    setPlugin(0);
  }

  namespace{
    
  
    
#define CAST(x) Cast<srcT,icl8u>::cast(x)
    template<class dstT, class srcT>
    void apply_lut_op_3(const Img<srcT> &src, Img<dstT> &dst, dstT *lut, icl8u shift){
      const unsigned int fac1 = 256>>shift;
      const unsigned int fac2 = fac1*fac1;
      if(src.hasFullROI() && dst.hasFullROI()){
        const srcT * pSrcR = src.getData(0); 
        const srcT * pSrcG = src.getData(1); 
        const srcT * pSrcB = src.getData(2);
        dstT *pDst = dst.getData(0);
        dstT *pDstEnd = pDst+dst.getDim();

        
        if(shift){
          while(pDst < pDstEnd){
            *pDst++  = lut[ (CAST(*pSrcR++)>>shift) + (fac1*(CAST(*pSrcG++)>>shift)) + (fac2*(CAST(*pSrcB++)>>shift)) ];
          }
        }else{
          while(pDst < pDstEnd){
            *pDst++  = lut[ CAST(*pSrcR++) + 256*CAST(*pSrcG++) + 65536*CAST(*pSrcB++) ];
          }
        }

      }else{
        ConstImgIterator<srcT> itSrcR= src.getROIIterator(0);        
        ConstImgIterator<srcT> itSrcG= src.getROIIterator(1);        
        ConstImgIterator<srcT> itSrcB= src.getROIIterator(2);        
        
        ImgIterator<dstT> itDst = dst.getROIIterator(0);
        if(shift){
          while(itDst.inRegion()){
            *itDst++  = lut[ (CAST(*itSrcR++)>>shift) + (fac1*(CAST(*itSrcG++)>>shift)) + (fac2*(CAST(*itSrcB++)>>shift)) ];
            
          }        
        }else{
          while(itDst.inRegion()){
            *itDst++  = lut[ CAST(*itSrcR++) + 256*CAST(*itSrcG++) + 65536*CAST(*itSrcB++) ];
          }        
        }
      }
    }
#undef CAST
  }

  template<class T>
  void LUTOp3Channel<T>::apply(const ImgBase *src, ImgBase **dst){
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(src->getChannels() == 3);
    if(!prepare(dst,getDepth<T>(), src->getSize(),formatMatrix,1, src->getROI(),src->getTime())){
      ERROR_LOG("unable to prepare output image");
    }
    switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: apply_lut_op_3(*(src->asImg<icl##D>()), *((*dst)->asImg<T>()), m_oLUT.getData(0), m_ucShift); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_DEPTH;
    }
  }
  
  template<class T>
  void LUTOp3Channel<T>::setPlugin(Plugin *p){
    if(p){
      if(m_poPlugin) delete m_poPlugin;
      m_poPlugin = p;
      T *lut = m_oLUT.getData(0);

      if(m_ucShift){
        const unsigned int fac1 = 256>>m_ucShift;
        const unsigned int fac2 = fac1*fac1;
        const unsigned int inc = pow(2,m_ucShift);// ? 2<<(m_ucShift-1) : 1;
        for(int r = 0; r < 256; r+=inc){
          for(int g = 0; g < 256; g+=inc){
            for(int b = 0; b < 256; b+=inc){
              lut[(r>>m_ucShift) + (fac1*(g>>m_ucShift)) + (fac2*(b>>m_ucShift)) ] = p->transform( r,g,b );
            }
          }
        } 
      }else{
        for(int r = 0; r < 256; ++r){
          for(int g = 0; g < 256; ++g){
            for(int b = 0; b < 256; ++b){
              lut[r+ 256*g + 65536*b] = p->transform( r,g,b );
            }
          }
        } 
      }
    }else{
      if(m_poPlugin){
        delete m_poPlugin;
        m_poPlugin = 0;
      }        
    }
  }

#define ICL_INSTANTIATE_DEPTH(D) template class LUTOp3Channel<icl##D>;
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  
} // namespace icl

