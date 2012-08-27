/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCV/FoundBlob.h                              **
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

#ifndef FOUND_BLOB_H
#define FOUND_BLOB_H

namespace icl{
  namespace cv{
    
    /// struct that holds all information for a found blob of a color blob searcher \ingroup G_CBS
    template<class BlobRatingType>
    class FoundBlob{
      
      public:
      /// Empty contructor initialized all data with 0
      FoundBlob():m_iX(0),m_iY(0),m_iID(-1),m_tRating(BlobRatingType(0)){}
      
      /// Default constructor given location, id and rating
      FoundBlob(int x, int y, int id, BlobRatingType rating):
        m_iX(x),m_iY(y),m_iID(id),m_tRating(rating){  }
      
      /// x location
      int x(){ return m_iX; }
      
      /// y location
      int y(){ return m_iY; }
      
      /// id
      int id(){ return m_iID; } 
      
      /// rating
      BlobRatingType rating(){ return m_tRating; }    
  
      private:
      
      /// internal storage of the x location
      int m_iX;
  
      /// internal storage of the y location
      int m_iY;
      
      /// internal storage of the blobs id
      int m_iID;
      
      /// internal storage of the blobs rating
      BlobRatingType  m_tRating;
    };
  } // namespace cv
}

#endif
