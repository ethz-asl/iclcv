/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/LineSampler.cpp                            **
** Module : ICLCore                                                **
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

#include <ICLCore/LineSampler.h>

using namespace icl::utils;

namespace icl{
  namespace core{
  
    Point LineSampler::buf[MAX_LINE_LENGTH];
    Point *LineSampler::cur=0;
    Point *LineSampler::end=0;
  
  
    template<bool steep, bool steep2, int ystep>
    inline void LineSampler::bresenham_templ(int x0, int x1, int y0, int y1, int minX, int maxX, int minY, int maxY, Point *p, int bufSize){
      Point *buf = p;
      int deltax = x1 - x0;
      int deltay = std::abs(y1 - y0);
      int error = 0;
  
      
      if(steep2) p += bufSize-1;
      
      for(int x=x0,y=y0;x<=x1;x++){
        if(steep){
          if( x>=minY && x<maxY && y>=minX && y<=maxX){
            if(steep2){
              *p-- = Point(y,x); 
            }else{
              *p++ = Point(y,x);
            }
          }
        }else{
          if( x>=minX && x<maxX && y>=minY && y<maxY){
            if(steep2){
              *p-- = Point(x,y);
          }else{
              *p++ = Point(x,y);
            }
          }
        }
        error += deltay;
        if (2*error >= deltax){
          y += ystep;
          error -=deltax;
        }
      }
      
      if(steep2){
        init(p+1,buf+bufSize);
      }else{
        init(buf,p);
      }
    }
  
    template<bool steep, bool steep2, int ystep>
    inline void LineSampler::bresenham_templ_2(int x0, int x1, int y0, int y1, Point *p, int bufSize){
      Point *buf = p;
      int deltax = x1 - x0;
      int deltay = std::abs(y1 - y0);
      int error = 0;
      
      
      if(steep2) p += bufSize-1;
      
      for(int x=x0,y=y0;x<=x1;x++){
        if(steep){
          if(steep2){
            *p-- = Point(y,x);
          }else{
            *p++ = Point(y,x);
          }
        }else{
          if(steep2){
            *p-- = Point(x,y);
          }else{
            *p++ = Point(x,y);
          }
        }
        
        error += deltay;
        if (2*error >= deltax){
          y += ystep;
          error -=deltax;
        }
      }
      
      if(steep2){
        init(p+1,buf+bufSize);
      }else{
        init(buf,p);
      }
    }
  
    
    void LineSampler::bresenham(int x0, int x1, int y0, int y1, int minX, int maxX, int minY, int maxY, Point *buf, int bufSize) throw (ICLException){
      bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
      if(steep){
        std::swap(x0, y0);
        std::swap(x1, y1);
      }
      bool steep2 = x0 > x1;
      if(steep2){
        std::swap(x0, x1);
        std::swap(y0, y1);
      }
      if(bufSize <=0){
        if(buf != LineSampler::buf){
          throw ICLException("LineSampler::bresenham got bufSize=0, but buffer is not the internal one");
        }
        bufSize = MAX_LINE_LENGTH;
      }
  
      if(y0 < y1){
        if(steep){
          if(steep2){
            return bresenham_templ<true,true,1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf,bufSize);
          }else{
            return bresenham_templ<true,false,1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf,bufSize);
          }
        }else{
          if(steep2){
            return bresenham_templ<false,true,1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf,bufSize);
          }else{
            return bresenham_templ<false,false,1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf,bufSize);
          }
        }
      }else{
        if(steep){
          if(steep2){
            return bresenham_templ<true,true,-1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf,bufSize);
          }else{
            return bresenham_templ<true,false,-1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf,bufSize);
          }
        }else{
          if(steep2){
            return bresenham_templ<false,true,-1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf,bufSize);
          }else{
            return bresenham_templ<false,false,-1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf,bufSize);
          }
        }
      }
    }
    
    void LineSampler::bresenham(int x0, int x1, int y0, int y1, Point *buf, int bufSize) throw (ICLException){
      bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
      if(steep){
        std::swap(x0, y0);
        std::swap(x1, y1);
      }
      bool steep2 = x0 > x1;
      if(steep2){
        std::swap(x0, x1);
        std::swap(y0, y1);
      }
  
      if(bufSize <=0){
        if(buf != LineSampler::buf){
          throw ICLException("LineSampler::bresenham got bufSize=0, but buffer is not the internal one");
        }
        bufSize = MAX_LINE_LENGTH;
      }
      
      if(y0 < y1){
        if(steep){
          if(steep2){
            return bresenham_templ_2<true,true,1>(x0,x1,y0,y1,buf,bufSize);
          }else{
            return bresenham_templ_2<true,false,1>(x0,x1,y0,y1,buf,bufSize);
          }
        }else{
          if(steep2){
            return bresenham_templ_2<false,true,1>(x0,x1,y0,y1,buf,bufSize);
          }else{
            return bresenham_templ_2<false,false,1>(x0,x1,y0,y1,buf,bufSize);
          }
        }
      }else{
        if(steep){
          if(steep2){
            return bresenham_templ_2<true,true,-1>(x0,x1,y0,y1,buf,bufSize);
          }else{
            return bresenham_templ_2<true,false,-1>(x0,x1,y0,y1,buf,bufSize);
          }
        }else{
          if(steep2){
            return bresenham_templ_2<false,true,-1>(x0,x1,y0,y1,buf,bufSize);
          }else{
            return bresenham_templ_2<false,false,-1>(x0,x1,y0,y1,buf,bufSize);
          }
        }
      }
    }
  } // namespace core
}
