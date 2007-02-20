#ifndef FIXED_CONVERTER_H
#define FIXED_CONVERTER_H

#include <Converter.h>

namespace icl{

  /// Special converter "producing" images with fixed parameters
  /** This class can be used to convert images with arbitrary params
      and depth to well defined fixed params and depth. Its functionality
      bases on the icl::Converter class which is created as a member of
      the FixedConverter.
      @see icl::Converter
  **/
  class FixedConverter{
    public:
    /// Create a new FixedConverter Object with given destination params and depth
    /** @param p output image parameters
        @param d output image depth
        @param applyToROIOnly decides wheater to apply the conversion on the
                              whole source image or only on the source images ROI
    **/
    FixedConverter(const ImgParams &p, depth d=depth8u, bool applyToROIOnly=false);

    /// Converts the source image into the given destination image
    /** The given destination image pointer is adapted in its depth and parameters
        before apply function of the Converter member is called.
        @param poSrc source image
        @param ppoDst destination image, null is not allowed! If it points to NULL,
                      the a appropriate destination image is created at *ppoDst
                      else it is adapted to the parameters of the FixedConverter object
    **/
    void apply(const ImgBase *poSrc, ImgBase **ppoDst);

    /// set up wheater to apply on the source image or on the source images ROI
    /** @param applytoROIOnly given flag
    **/
    void setApplyToROIOnly(bool applyToROIOnly){ m_oConverter.setApplyToROIOnly(applyToROIOnly); }
    
    /// sets up the output image parameters
    /** @param p output image parameters        
    **/
    void setParams(const ImgParams &p) { m_oParams = p; }

    /// sets up the order to use when applying several conversion internally
    /** @param o new operation order 
        @see Converter 
    **/
    void setOperationOrder(Converter::oporder o){ m_oConverter.setOperationOrder(o); }
    private:

    /// destination image parameters
    ImgParams m_oParams;
    
    /// Converter to apply conversion
    Converter m_oConverter;
    
    /// destination image depth
    depth m_eDepth;
  };

}

#endif 
