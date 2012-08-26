/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMath/StraightLine2D.h                       **
** Module : ICLMath                                                **
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

#ifndef ICL_STRAIGHT_LINE_2D_H
#define ICL_STRAIGHT_LINE_2D_H

#include <ICLMath/FixedVector.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Exception.h>

namespace icl{
  
  /// A straight line is parameterized in offset/direction form
  /** This formular is used: 
      \f[ L(x) = \vec{o} + x\vec{v} \f]
      
      The template is instantiated for template parameter Pos type
      Point32f and FixedColVector<float,2>
  */
  struct StraightLine2D{
    /// internal typedef 
    typedef FixedColVector<float,2> PointPolar;

    /// internal typedef for 2D points
    typedef FixedColVector<float,2> Pos;
    
    /// creates a straight line from given angle and distance to origin
    StraightLine2D(float angle, float distance);
    
    /// creates a straight line from given 2 points
    StraightLine2D(const Pos &o=Pos(0,0), const Pos &v=Pos(0,0));

    /// creates a straight line from given point32f
    StraightLine2D(const Point32f &o, const Point32f &v);
    
    /// 2D offset vector
    Pos o;
    
    /// 2D direction vector
    Pos v;
    
    /// computes closest distance to given 2D point
    float distance(const Pos &p) const;
    
    /// computes closest distance to given 2D point
    /* result is positive if p is left of this->v
        and negative otherwise */
    float signedDistance(const Pos &p) const;

    /// computes closest distance to given 2D point
    inline float distance(const Point32f &p) const { return distance(Pos(p.x,p.y)); }
    
    /// computes closest distance to given 2D point
    /* result is positive if p is left of this->v
        and negative otherwise */
    float signedDistance(const Point32f &p) const { return signedDistance(Pos(p.x,p.y)); }
    
    /// computes intersection with given other straight line
    /** if lines are parallel, an ICLException is thrown */
    Pos intersect(const StraightLine2D &o) const throw(ICLException);
    
    /// returns current angle and distance
    PointPolar getAngleAndDistance() const;
    
    /// retunrs the closest point on the straight line to a given other point
    Pos getClosestPoint(const Pos &p) const;
  };  
}

#endif
