/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCore/LineSampler.h                          **
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

#ifndef ICL_LINE_SAMPLER_H
#define ICL_LINE_SAMPLER_H

#include <ICLUtils/Point.h>
#include <ICLUtils/Rect.h>
#include <ICLUtils/Exception.h>

namespace icl{
  namespace core{
    /// Utility class for efficient line sampling (providing only static functions)
    /** The LineSampler class is a bit different from the Line class, which is also provided 
        in the ICLCore package. The Line class is a really basic line structure, that ALSO provides
        a function to sample itsef into an image pixel grid. The LineSampler class uses a lot of
        optimizations to enhance sampling performance. This is achieved by a set of optimizations:
        
        - avoid data copies and dynamic memory allocations: Internally, a static (of given) buffer is 
          used to store the sampled point set. This provides a high performance benefit, however
          it also entails the main drawback: The LineSampler class cannot be instatiated, as it this 
          would mean, that this sampling buffers was either used by several instances, or that each
          instance would need an own buffer, which would nullify all the performance benefits, we got
          before
        - limiting the maximum line length: by these means, we can use a fixed size data buffer
        - internally we use a lot of templates, to suppress if-then statements if possible
        
        The facilitate buffer allocation and things like that, the SampledLine class should be used. 
        This class provides an Object-oriented handling of sampled lines
        
        \section PERF Performance Comparison
        In comparison to the Line.sample function, the LineSampler is nearly 30 times faster.
        Sampling a short line (from (23,40) to (20,20)) with given bounding rect 1 Million times 
        lasts about 120ms (compiled with -O4 and -march=native on a 2Ghz Core-2-duo)
    */
    class LineSampler{
      public:
      
      // maximum number of line pixels
      static const int MAX_LINE_LENGTH = 10000;
  
      private:
      /// Internel data pointers (wrapped shallowly)
      static utils::Point *cur, *end;
      
      /// internal initialization function
      static inline void init(utils::Point *cur, utils::Point *end){
        LineSampler::cur = cur;
        LineSampler::end = end;
      }
      
      /// internal sampling function using bresenham line sampling algorithm
      template<bool steep, bool steep2, int ystep>
      static void bresenham_templ_2(int x0, int x1, int y0, int y1, utils::Point *p, int bufSize);
  
      /// internal sampling function using bresenham line sampling algorithm
      template<bool steep, bool steep2, int ystep>
      static void bresenham_templ(int x0, int x1, int y0, int y1, int minX, int maxX, int minY, int maxY, utils::Point *p, int bufSize);
  
      /// internal sampling function using bresenham line sampling algorithm
      static void bresenham(int x0, int x1, int y0, int y1, int minX, int maxX, int minY, int maxY, utils::Point *p, int bufSize) throw (utils::ICLException);
  
      /// internal sampling function using bresenham line sampling algorithm
      static void bresenham(int x0, int x1, int y0, int y1, utils::Point *p, int bufSize) throw (utils::ICLException);
  
      /// internal point buffer
      static utils::Point buf[MAX_LINE_LENGTH];
      
      static utils::Point *get_buf(utils::Point *userBuf){
        return userBuf ? userBuf : LineSampler::buf;
      }
      LineSampler(){}
      
      public:
      
      static void getPointers(utils::Point **cur, utils::Point **end){
        *cur = LineSampler::cur;
        *end = LineSampler::end;
      }
  
      /// create a SampledLine instance (only one instance is valid at a time)
      static void init(int aX, int aY, int bX, int bY, utils::Point *userBuf=0, int bufSize=0) throw (utils::ICLException){ 
        bresenham(aX,bX,aY,bY,get_buf(userBuf),bufSize); 
      }
  
      /// create a SampledLine instance (only one instance is valid at a time) with given boundig rect parameters
      static void init(int aX, int aY, int bX, int bY, int minX, int minY, int maxX, int maxY, utils::Point *userBuf=0, int bufSize=0) throw (utils::ICLException){
        bresenham(aX,bX,aY,bY,minX,maxX,minY,maxY,get_buf(userBuf),bufSize);
      }
      /// create a SampledLine instance (only one instance is valid at a time)
      static void init(const utils::Point &a, const utils::Point &b, utils::Point *userBuf=0, int bufSize=0) throw (utils::ICLException){ 
        bresenham(a.x,b.x,a.y,b.y,get_buf(userBuf),bufSize); 
      }
  
      /// create a SampledLine instance (only one instance is valid at a time) with given boundig rect parameters
      static void init(const utils::Point &a, const utils::Point &b, 
                       const utils::Rect &bounds, utils::Point *userBuf=0, int bufSize=0) throw (utils::ICLException){
        bresenham(a.x,b.x,a.y,b.y,bounds.x,bounds.y,bounds.right(),bounds.bottom(), get_buf(userBuf),bufSize);
      }
      
      /// gets the next valid point (this function is not overflow-safe)
      /** calls to next must be protected by using hasNext() before*/
      static inline const utils::Point &next(){ return *cur++; }
      
      /// returns whether this line has remaining points, that have not yet been extracted using next()
      static inline bool hasNext() { return cur != end; }
  
      /// returns the number of remaining points in this line
      static inline int remaining() { return (int)(end-cur); }
    };
  
  } // namespace core
}

#endif
