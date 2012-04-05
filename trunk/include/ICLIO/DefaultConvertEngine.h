/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/DefaultConvertEngine.h                   **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#ifndef ICL_DEFAULT_CONVERT_ENGINE_H
#define ICL_DEFUALT_CONVERT_ENGINE_H
#include <ICLIO/UnicapConvertEngine.h>
#include <vector>

namespace icl{
  

  /// Basic implemenation for the UnicapCovertEngine \ingroup UNICAP_G
  /** providing abilities to convert the following formats:
      - Y444 YUV 4-4-4 interleaved (no sub sampling of U and V channel \n
        (C++ and IPP implementation)
      - UYVY YUV 4-2-2 interleaved (half size subsampling of U and V channel)\n
        (C++ and IPP implementation)
      - YUYV YUV 4-2-2 interleaved (half size subsampling of U and V channel)\n
        (C++ implementation only)
      - Y411 YUV 4-1-1 interleaved (quater size subsampling of U and V channel)\n
        (C++ implementation only) 
      - Y800 8Bit Grab image (no conversion ; just a deep copy)
      
      This convert engine is appropriate to the following camera models:
      - Sony DFW-VL500 cam
      - Apple Computers, Inc., iSight
  **/
  class DefaultConvertEngine : public UnicapConvertEngine{
    public:
    /// Default constructor just passing the UnicapDevice argument to the parent constructor
    DefaultConvertEngine(UnicapDevice *device):UnicapConvertEngine(device){}
    
    /// Empty destructor
    virtual ~DefaultConvertEngine(){}
    
    /// Convertsion function
    /** \copydoc icl::UnicapConvertEngine::cvt(const icl8u*, const ImgParams&, depth, ImgBase**) */
    virtual void cvt(const icl8u *rawData, const ImgParams &desiredParams, depth desiredDepth, ImgBase **ppoDst);

    /// returns whether this engine is able to provide images with given params
    virtual bool isAbleToProvideParams(const ImgParams &desiredParams, depth desiredDepth) const;

    private:
    /// internal used data buffer for conversion
    std::vector<icl8u> m_oCvtBuf;
  };
}

#endif
