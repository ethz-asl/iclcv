/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCV/PixelRatingGroup.h                       **
** Module : ICLCV                                                  **
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

#pragma once

#include <ICLCV/PixelRating.h>
#include <vector>

using std::vector;

namespace icl{
  namespace cv{
    
    /// Provides abilities for individual combinations of pixleratings \ingroup G_CBS
    template<class PixelType,class RatingType>
    class PixelRatingGroup : public PixelRating<PixelType,RatingType>{
  
      protected:
      /// internaly used type definition for the containded pixelratings
      typedef PixelRating<PixelType,RatingType> pixelrating;
      
      /// internally used type definition for then member vector of pixelratings 
      typedef std::vector<pixelrating*> PixelRatingVector;
      
      public:
      /// overwritten rate-function, calls combineRatings with all sub-results
      virtual RatingType rate(PixelType a, PixelType b, PixelType c){
        for(unsigned int i=0;i<m_vecPR.size();++i){
          m_vecSubResults[i] = m_vecPR[i]->rate(a,b,c);
        }
        return combineRatings(m_vecSubResults);
      }
      /// deletes all contained pixelratings
      virtual ~PixelRatingGroup(){
        for(unsigned int i=0;i<m_vecPR.size();++i){
          delete m_vecPR[i];
        }
      }
      
      /// this function has to be idividualized
      virtual RatingType combineRatings(const vector<RatingType> &vec ){
        (void)vec;
        return RatingType();
      }
      
      /// adds a new pixelrating to the group (which takes possession of it)
      virtual void addPR(pixelrating *p){
        m_vecSubResults.push_back(0);
        m_vecPR.push_back(p);
      }
      
      /// removed and deletes the pixelrating at index
      int removePR(int index){
        if(index == -1){
          for(unsigned int i=0;i<m_vecPR.size();i++){
            delete m_vecPR[i];
          }
          m_vecPR.clear();
          m_vecSubResults.clear();
          return 1;
        }
        if(index < (int)m_vecPR.size() ) {
          m_vecSubResults.erase(m_vecSubResults.begin()+index);    
          delete *(m_vecPR.begin()+index);
          m_vecPR.erase(m_vecPR.begin()+index);
          return 1;
        }else{
          return 0;
        }
      }
  
      int getNumPR(){
        return m_vecPR.size();
      }
      
      protected:
      vector<RatingType> m_vecSubResults;
      PixelRatingVector m_vecPR;
    };
  } // namespace cv
}

