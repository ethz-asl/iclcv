#include <Logical.h>
#include <Img.h>

namespace icl {

#ifdef WITH_IPP_OPTIMIZATION

  // {{{ ippi-function call templates

  template <typename T, IppStatus (*ippiFunc) (const T*, int, const T*, int, T*, int, IppiSize)>
  inline void ippiAndOrXorCall(const Img<T> *src1, const Img<T> *src2, Img<T> *dst)
  {
    // {{{ open
    ICLASSERT_RETURN( src1 && src2 && dst );
    ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
    ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src1->getChannels()==src2->getChannels());
    ICLASSERT_RETURN( src1->getChannels()==dst->getChannels());
    for (int c=src1->getChannels()-1; c >= 0; --c) {
      ippiFunc (src1->getROIData (c), src1->getLineStep(),
                src2->getROIData (c), src2->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize());
    }
  }
  // }}}

  template <typename T, IppStatus (*ippiFunc) (const T*, int, const T, T*, int, IppiSize)>
  inline void ippiAndOrXorCallC(const Img<T> *src, T value, Img<T> *dst)
  {
    // {{{ open
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels()==dst->getChannels());
    for (int c=src->getChannels()-1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                value,
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize());
    }
  }
  // }}}

  template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize)>
  inline void ippiNotCall(const Img<T> *src, Img<T> *dst)
  {
    // {{{ open
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels()== dst->getChannels());
    for (int c=src->getChannels()-1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize());
    }
  }
  // }}}

  // }}}

  // {{{ function specializations

  void Logical::And (const Img8u *src1, const Img8u *src2, Img8u *dst)
  {
    ippiAndOrXorCall<icl8u,ippiAnd_8u_C1R>(src1,src2,dst);
  }
  void Logical::Or (const Img8u *src1, const Img8u *src2, Img8u *dst)
  {
    ippiAndOrXorCall<icl8u,ippiOr_8u_C1R>(src1,src2,dst);
  }
  void Logical::Xor (const Img8u *src1, const Img8u *src2, Img8u *dst)
  {
    ippiAndOrXorCall<icl8u,ippiXor_8u_C1R>(src1,src2,dst);
  }
  void Logical::Not (const Img8u *src, Img8u *dst)
  {
    ippiNotCall<icl8u,ippiNot_8u_C1R>(src,dst);
  }

  void Logical::AndC (const Img8u *src, const icl8u value, Img8u *dst)
  {
    ippiAndOrXorCallC<icl8u,ippiAndC_8u_C1R>(src,value,dst);
  }
  void Logical::OrC (const Img8u *src, const icl8u value, Img8u *dst)
  {
    ippiAndOrXorCallC<icl8u,ippiOrC_8u_C1R>(src,value,dst);
  }
  void Logical::XorC (const Img8u *src, const icl8u value, Img8u *dst)
  {
    ippiAndOrXorCallC<icl8u,ippiXorC_8u_C1R>(src,value,dst);
  }

  // }}}

#else

  // {{{ C++ fallback functions

  template <typename T>
  void fallbackLogicalNot(const Img<T> *src, Img<T> *dst)
    // {{{ open
  {
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
    for(int c=src->getChannels()-1; c >= 0; --c) {
      ConstImgIterator<T> itSrc = src->getROIIterator(c);
      ImgIterator<T>      itDst = dst->getROIIterator(c);
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
        *itDst = ~*itSrc ;
      }
    }
  }
  // }}}

  template <typename T,class LogicalOp>
  void fallbackLogicalAndOrXor(const Img<T> *src1, const Img<T> *src2, Img<T> *dst,const LogicalOp &op)
    // {{{ open
  {
    ICLASSERT_RETURN( src1 && src2 && dst );
    ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
    ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
    ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );
    for(int c=src1->getChannels()-1; c >= 0; --c) {
      ConstImgIterator<T> itSrc1 = src1->getROIIterator(c);
      ConstImgIterator<T> itSrc2 = src2->getROIIterator(c);
      ImgIterator<T>      itDst  =  dst->getROIIterator(c);
      for(;itSrc1.inRegion(); ++itSrc1, ++itSrc2, ++itDst){
        *itDst = op(*itSrc1,*itSrc2);
      }
    }
  }
  // }}}

  template <typename T,class LogicalOp>
  void fallbackLogicalAndOrXorC(const Img<T> *src, const T value, Img<T> *dst,const LogicalOp &op)
    // {{{ open
  {
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
    for(int c=src->getChannels()-1; c >= 0; --c) {
      ConstImgIterator<T> itSrc = src->getROIIterator(c);
      ImgIterator<T>      itDst = dst->getROIIterator(c);
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
        *itDst = op(*itSrc,value);
      }
    }
  }
  // }}}

  // }}}

  // {{{ C++ fallback Logical Operators
  template <typename T> class AndOp {
  public:
    inline T operator()(T val1,T val2) const { return val1 & val2; }
  };

  template <typename T> class OrOp {
  public:
    inline T operator()(T val1,T val2) const { return val1 | val2; }
  };

  template <typename T> class XorOp {
  public:
    inline T operator()(T val1,T val2) const { return val1 ^ val2; }
  };
  // }}}

  // {{{ C++ fallback function specializations
  void Logical::And (const Img8u *src1, const Img8u *src2, Img8u *dst)
  {
    fallbackLogicalAndOrXor<icl8u>(src1, src2,dst,AndOp<icl8u>());
  }
  void Logical::Or (const Img8u *src1, const Img8u *src2, Img8u *dst)
  {
    fallbackLogicalAndOrXor<icl8u>(src1, src2,dst,OrOp<icl8u>());
  }
  void Logical::Xor (const Img8u *src1, const Img8u *src2, Img8u *dst)
  {
    fallbackLogicalAndOrXor<icl8u>(src1, src2,dst,XorOp<icl8u>());
  }
  void Logical::Not (const Img8u *src, Img8u *dst)
  {
    fallbackLogicalNot<icl8u>(src, dst);
  }

  void Logical::AndC (const Img8u *src, const icl8u value, Img8u *dst)
  {
    fallbackLogicalAndOrXorC<icl8u>(src, value,dst,AndOp<icl8u>());
  }
  void Logical::OrC (const Img8u *src, const icl8u value, Img8u *dst)
  {
    fallbackLogicalAndOrXorC<icl8u>(src, value,dst,OrOp<icl8u>());
  }
  void Logical::XorC (const Img8u *src, const icl8u value, Img8u *dst)
  {
    fallbackLogicalAndOrXorC<icl8u>(src, value,dst,XorOp<icl8u>());
  }
  // }}}

#endif

  // {{{ ImgBase* versions
  
  void Logical::Not (const ImgBase *poSrc, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN(poSrc->getDepth() == depth8u);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    Not(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
  }
  // }}}

  void Logical::And (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc1->getDepth() == depth8u);
    ICLASSERT_RETURN( poSrc2->getDepth() == depth8u);
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    And(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
  }
  // }}}

  void Logical::Or (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc1->getDepth() == depth8u);
    ICLASSERT_RETURN( poSrc2->getDepth() == depth8u);
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    Or(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
  }
  // }}}

  void Logical::Xor (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );
    ICLASSERT_RETURN( poSrc1->getDepth() == depth8u);
    ICLASSERT_RETURN( poSrc2->getDepth() == depth8u);
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    Xor(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
  }
  // }}}

  void Logical::AndC (const ImgBase *poSrc, const icl8u value, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth8u);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    AndC(poSrc->asImg<icl8u>(),value,(*ppoDst)->asImg<icl8u>());
  }
  // }}}

  void Logical::OrC (const ImgBase *poSrc, const icl8u value, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth8u);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    OrC(poSrc->asImg<icl8u>(),value,(*ppoDst)->asImg<icl8u>());
  }
  // }}}

  void Logical::XorC (const ImgBase *poSrc, const icl8u value, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth8u);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    XorC(poSrc->asImg<icl8u>(),value,(*ppoDst)->asImg<icl8u>());
  }
  // }}}
  
// }}}
}
