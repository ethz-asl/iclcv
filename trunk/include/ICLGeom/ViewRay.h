/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/ViewRay.h                              **
** Module : ICLGeom                                                **
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

#include <ICLGeom/GeomDefs.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Point32f.h>
#include <iostream>

namespace icl{
  namespace geom{
    /** \cond */
    struct PlaneEquation;
    /** \endcond */
  
    /// This is a view-ray's line equation in parameter form
    /** A view-ray is described by the equation
        \f[ V: \mbox{offset} + \lambda \cdot \mbox{direction} \f]
    */
    struct ViewRay{
      /// Constructor with given offset and direction vector
      explicit ViewRay(const Vec &offset=Vec(), const Vec &direction=Vec(), bool autoNormalizeDirection=false);
      
      /// line offset
      Vec offset;
      
      /// line direction
      Vec direction;
      
      /// calculates line-plane intersection 
      /** @see static Camera::getIntersection function */
      Vec getIntersection(const PlaneEquation &plane) const throw (utils::ICLException);

      /// ray-triangle intersection results 
      enum TriangleIntersection{
        noIntersection,
        foundIntersection,
        wrongDirection,
        degenerateTriangle,
        rayIsCollinearWithTriangle
      };
      
      /// calculates intersection with given triangle
      /** inspired by http://softsurfer.com/Archive/algorithm_0105/algorithm_0105.htm#intersect_RayTriangle() 
          
          The actual intersection point in 3D can optionally be stored into a non-null "intersectionPoint"
          parameter. The parametric triangle coordinats can also optionally be stored in a non-null
          "parametricCoords" parameter. "parametricCoords" is only written if it is not null and if
          the returned intersection result is "foundIntersection"
          */
      TriangleIntersection getIntersectionWithTriangle(const geom::Vec &a, const geom::Vec &b, const geom::Vec &c, 
                                                       geom::Vec *intersectionPoint=0, 
                                                       utils::Point32f *parametricCoords=0) const;
        
      /// calculates the closest distance to the given 3D-Point
      /** for following formula is used:
          <pre>
          ViewRay: o + lambda * v;
          3D-utils::Point: p
          
          distance = sqrt( |p-o|^2 - |(p-o).v|^2 )
          </pre>
      */
      float closestDistanceTo(const Vec &p) const;
      
      /// calculates the closes distance to the given other ViewRay
      /** Here, we use the following formula:
          <pre>
          this ViewRay:  o + lambda * v
          other ViewRay: k + beta * m
          
          distance = | (k-o).(v x m) | 
                     -----------------
                         | v x m |
          </pre>
      */
      float closestDistanceTo(const ViewRay &other) const;
      
      /// evaluates ViewRay' line equation at given lambda
      /** formula : offset + lambda * direction */
      Vec operator()(float lambda) const;
    };
  
    /// ostream operator
    std::ostream &operator<<(std::ostream &s, const ViewRay &vr);
  } // namespace geom
}

