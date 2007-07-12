#ifndef ICL_CHAMPFER_OP
#define ICL_CHAMPFER_OP

#include "iclUnaryOp.h"

namespace icl{
  /// Champfering Unit
  /** TODO comment here 
      \section BENCH Benchmarks
      The champfering operation komplexity is linear to pixel count of an
      image, so a single benchmark for differnt depths are sufficient:
      
      Image-size 320x240 single channel (Pentium M 1.6GHz)
      - icl8u:  approx. 3.6ms
      - icl16s  approx  3.7ms
      - icl32s: approx. 3.7ms
      - icl32f: approx. 3.6ms
      - icl64f: approx. 3.7ms
      
      calculation of the "real-euclidian norm takes much longer" as it runs linear to
      "image-size"*"white-pixel-count" 
      - e.g. icl8u: approx. 6.6<b>sec</b> (using an image with 2193 white pixels)
  */
  class ChampferOp : public UnaryOp{
    public:
    /// Metric to use for horizontal/vertical and diagonal pixels
    enum metric{
      metric_1_1,            /**< h/v and d are handled equally */
      metric_1_2,            /**< city block metric */       
      metric_2_3,            /**< simple approximation of the euclidian metic \f$1:sqrt(2)\f$*/
      metric_7071_10000,     /**< better approximation of the euclidian metic */
      metric_real_euclidian  /**< use the <b>real</b> euclidian distance instead of the high performance matching (not realtime capable)*/
    };

    ChampferOp( metric m = metric_7071_10000 ): m_eMetric(m){}

    virtual ~ChampferOp(){}
    
    virtual void apply(const ImgBase *poSrc, ImgBase **ppoDst);
    
    inline void setMetric( metric m){ m_eMetric = m; }
    
    metric getMetric() const{ return m_eMetric; }

    private:
    metric m_eMetric;
  };
}
#endif
