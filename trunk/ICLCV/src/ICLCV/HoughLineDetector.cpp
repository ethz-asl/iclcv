/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/HoughLineDetector.cpp                  **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLCV/HoughLineDetector.h>
#include <ICLMath/DynMatrixUtils.h>
#include <ICLFilter/ConvolutionOp.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::filter;

namespace icl{
  namespace cv{
  
  
    HoughLineDetector::HoughLineDetector(float dRho, float dR, const Range32f &rRange, float rInhibitionRange, float rhoInhibitionRange,
                                         bool gaussianInhib, bool blurHoughSpace,bool dilateEntries,bool blurredSampling){
      init(dRho, dR, rRange, rInhibitionRange, rhoInhibitionRange, gaussianInhib, blurHoughSpace, dilateEntries, blurredSampling);
    }

    void HoughLineDetector::init(float dRho, float dR, const utils::Range32f &rRange, 
                                 float rInhibitionRange, float rhoInhibitionRange,
                                 bool gaussianInhibition,
                                 bool blurHoughSpace,
                                 bool dilateEntries,
                                 bool blurredSampling){
      m_dRho = dRho;
      m_rRange = rRange;
      m_rInhib = rInhibitionRange;
      m_rhoInhib = rhoInhibitionRange;
      m_gaussianInhibition = gaussianInhibition;
      m_blurHoughSpace = blurHoughSpace;
      m_dilateEntries = dilateEntries;
      m_blurredSampling = blurredSampling;
      
      
      m_w = ceil(2*M_PI/dRho);
      m_h = (rRange.maxVal-rRange.minVal)/dR;
      
      m_image.setChannels(1);
      m_image.setSize(Size(m_w, m_h));
      m_lut = m_image[0];
  
      m_mr = (m_h-1)/(rRange.maxVal-rRange.minVal);
      m_br = -rRange.minVal * m_mr;
      
      m_mrho = (m_w-1)/(2*M_PI);
  
      if(gaussianInhibition){
        /// create inhibition image
        float dx = m_rhoInhib/(2*M_PI) * float(m_w);
        float dy = m_rInhib/(m_rRange.maxVal-m_rRange.minVal) * float(m_h);
  
        int w = 2*dx, h=2*dy;
        if(dx >0 && dy >0){
          m_inhibitImage = Img32f(Size(2*dx,2*dy),1);
          Channel32f I = m_inhibitImage[0];
          
          Point32f c(I.getWidth()/2,I.getHeight()/2);
          
          for(int x=0;x<I.getWidth();++x){
            for(int y=0;y<I.getHeight();++y){
              float r = (c-Point32f(x,y)).transform(2./w,2./h).norm();
              I(x,y) = 1.0 - exp(-r*r);
            }
          }
        }
      }
    }

  void HoughLineDetector::add(const Img8u &binaryImage){
    ICLASSERT_THROW(binaryImage.getChannels() == 1, ICLException("HoughLineDetector::add: can only work with 1 channel images"));
    const Channel8u c = binaryImage[0];
    for(int y=0;y<c.getHeight();++y){
      for(int x=0;x<c.getWidth();++x){
        if(c(x,y)) add_intern(x,y);
      }
    }
  }

    void HoughLineDetector::add_intern(float x, float y){
      if(m_dilateEntries){
        add_intern2(x,y);
        add_intern2(x-1,y);
        add_intern2(x+2,y);
        add_intern2(x,y-1);
        add_intern2(x,y+1);
      }else{
        add_intern2(x,y);
      }
    }
    
    void HoughLineDetector::add_intern2(float x, float y){
      if(m_blurredSampling){
        for(float rho=0;rho<2*M_PI;rho+=m_dRho){
          incLutBlurred(rho,r(rho,x,y));
        }
      }else{
        for(float rho=0;rho<2*M_PI;rho+=m_dRho){
          incLut(rho,r(rho,x,y));
        }
      }
    }
  
    void HoughLineDetector::clear(){
      std::fill(m_lut.begin(),m_lut.end(),0);
    }
  
    static inline int mult_float_int(const int &a, const float &b){
      return round(a*b);
    }
      
    void HoughLineDetector::apply_inhibition(const Point &p){
      float dx = m_rhoInhib/(2*M_PI) * float(m_w);
      float dy = m_rInhib/(m_rRange.maxVal-m_rRange.minVal) * float(m_h);
      const Rect r(p.x-dx,p.y-dy,2*dx,2*dy);  
  
      if(m_gaussianInhibition){
        Channel32f c0 = m_inhibitImage[0];
        for(int x=r.x;x<r.right();++x){
          for(int y=r.y;y<r.bottom();++y){
            if(!m_inhibitImage.getImageRect().contains(x-r.x,y-r.y)){
            }else{
              cyclicLUT(x,y) *= c0(x-r.x,y-r.y);
            }
          }
        }
      }else{
        for(int x=r.x;x<r.right();++x){
          for(int y=r.y;y<r.bottom();++y){
            cyclicLUT(x,y)=0;
          }
        }
      }
    }
  
    void HoughLineDetector::blur_hough_space_if_necessary(){ 
      if(m_blurHoughSpace){
        ImgBase *image = 0;
        ConvolutionOp co( (ConvolutionKernel(ConvolutionKernel::gauss3x3)) );
        co.setClipToROI(false);
        co.apply(&m_image,&image);
        m_image = *image->asImg<icl32s>();
        ICL_DELETE(image);
        m_image.setFullROI();
        m_lut = m_image[0];
      }
    }
    
    std::vector<StraightLine2D> HoughLineDetector::getLines(int max) {
      blur_hough_space_if_necessary();
      
      std::vector<StraightLine2D> ls;
      ls.reserve(max);
      
      for(int i=0;i<max;++i){
        Point p(-1,-1);
        int m = m_image.getMax(0,&p);
        if(m == 0) return ls;
        ls.push_back(StraightLine2D(getRho(p.x),getR(p.y)));
        apply_inhibition(p);
  
      }
      
      return ls;
    }
  
    
    std::vector<StraightLine2D> HoughLineDetector::getLines(int max, std::vector<float> &significances){
      blur_hough_space_if_necessary();
      
      std::vector<StraightLine2D> ls;
      ls.reserve(max);
      significances.clear();
      significances.reserve(max);
      
      int firstMax = -1;
      for(int i=0;i<max;++i){
        Point p(-1,-1);
        int m = m_image.getMax(0,&p);
        if(!i) firstMax = m;
        significances.push_back(float(m)/firstMax);
        if(m == 0) return ls;
        ls.push_back(StraightLine2D(getRho(p.x),getR(p.y)));
        
        apply_inhibition(p);
  
      }
      
      return ls;
    }
  } // namespace cv
}
