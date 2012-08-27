/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCV/DefaultColorBlobSearcher.h               **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

#ifndef DEF_COLOR_BLOB_SEARCHER_H
#define DEF_COLOR_BLOB_SEARCHER_H

#include <ICLCV/ColorBlobSearcher.h>
#include <ICLCV/PixelRatingGroup.h>
#include <ICLUtils/FastMedianList.h>
#include <stdlib.h>
#include <vector>



namespace icl{
  namespace cv{
  
    /// Default implementation of the ColorBlobSearcher interface \ingroup G_CBS
    class DefaultColorBlobSearcher : public ColorBlobSearcher<icl8u,bool,float>{
      public:
      /// combination strategie for combining pixelratings
      enum RatingCombinationType { 
        rctOR, /// !< pixel-wise or 
        rctAND /// !< pixel-wise and
      };
      /// center estimation strategy
      enum CenterEstimationType { 
        cetMean,  /// !< center is estimated using the pixel mean
        cetMedian /// !< center is estimated using the pixel median 
      };
  
      /// Default constructor with given image size
      DefaultColorBlobSearcher(const Size &imageSize);
      
      /// Destructor
      virtual ~DefaultColorBlobSearcher();
      
      /// internal type definition
      typedef vector<FastMedianList> fmlVec;
  
      /// internal type definition
      typedef vector<CenterEstimationType> cetVec;
  
      /// add a new reference color based sub searcher
      /** creates n new RGB reference color and threshold based blob searchers where n is
          rs.size() == gs.size() == bs.size().
          and given RatingCombinationType as well as CenterEstimationType. \n
          Example:
          <pre>
          rs={255,0}
          gs={0,0}
          bs={0,200}
          thresholds = {10,255,10}
          
          Creates to RGB-Pixel-Ratings:
          1st:  reference color 255,0,0 (rs[0],gs[0] and bs[0]) with threshold (10,255,10)
          2nd:  reference color 0,0,200 (rs[1],gs[0] and bs[1]) also with threshold (10,255,10) 
          
          The Pixel classification funcition of the first rating is:
          f(r,g,b) = abs(r-255)<10 && abs(g-0) <255 && abs(b-0)<10
          which is equal to f(r,g,b) = r in [246,255] && b in [190,210]
          </pre>
      */
      int addSubSearcher(const vector<icl8u> &rs,
                         const vector<icl8u> &gs, 
                         const vector<icl8u> &bs,
                         const icl8u thresholds[3],
                         RatingCombinationType rct=rctOR,
                         CenterEstimationType cet=cetMedian);
  
      /// just passing to the parent class
      virtual const FoundBlobVector &search(Img8u *poImage, Img8u *poMask);
  
      /// returns the current image size of this 
      const Size &getImageSize() const;
  
      /// sets the current image size
      void setImageSize(const Size &size);
      protected:
      
      /// internally used function
      virtual void prepareForNewImage(Img8u *poImage, Img8u *poMask);
  
      /// internally used function
      virtual void storeResult(int iPRIndex, int x, int y, bool rating);
  
      /// internally used function
      virtual void evaluateResults(FoundBlobVector &resultDestination);
  
      /// internally used function
      virtual void pixelRatingAdded(pixelrating *pr);
  
      /// internally used function
      virtual void pixelRatingRemoved(int index);
      
      private:
      /// internal storage of the current image size    
      Size m_oImageSize;
  
      /// internally used median list 
      fmlVec m_vecXMedianLists;
  
      /// internally used median list 
      fmlVec m_vecYMedianLists;
  
      /// internally center list
      cetVec m_vecCet;
      
    };
  } // namespace cv
}
#endif

