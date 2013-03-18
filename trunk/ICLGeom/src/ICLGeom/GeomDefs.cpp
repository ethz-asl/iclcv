/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/GeomDefs.cpp                       **
** Module : ICLGeom                                                **
** Authors: Matthias Schroeder, Christof Elbrechter                **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLGeom/GeomDefs.h>
#include <limits>

namespace icl{
  using namespace utils;
  
  namespace geom{
    
    float dist_point_linesegment(const Vec &p,const Vec &lineStart,const Vec &lineEnd,Vec *nearestPoint){
      Vec  d1(p-lineStart);
      Vec  d2(lineEnd-lineStart);
      Vec  min_v(lineStart);
      float t = sprod3(d2,d2);
    
      if(t > std::numeric_limits<float>::min()){
        t = sprod3(d1,d2)/t;
        if(t > 1.0){
          d1 = p - (min_v = lineEnd);
        }else if(t > 0.f){
          d1 = p - (min_v = lineStart + d2*t);
        }
      }
    
      if(nearestPoint) *nearestPoint = min_v;
      return norm3(d1);
    }

    float dist_point_triangle(const Vec &p,const Vec &a,const Vec &b,const Vec &c,Vec *nearestPoint){
      Vec  ab = b - a;
      Vec  ac = c - a;
      Vec  n = cross(ab,ac); // not normalized !
      float d = sqrnorm3(n);
    
      // Check if the triangle is degenerated -> measure dist to line segments
      if(fabs(d) < std::numeric_limits<float>::min()){
        Vec  q, qq;
        float d, dd(std::numeric_limits<float>::max());
      
        dd = dist_point_linesegment(p, a, b, &qq);
      
        d = dist_point_linesegment(p, b, c, &q);
        if(d < dd) { dd = d; qq = q; }
      
        d = dist_point_linesegment(p, c, a, &q);
        if(d < dd) { dd = d; qq = q; }
      
        if(nearestPoint) *nearestPoint = qq;
        return dd;
      }
    
      float invD = 1.0 / d;
      Vec  v1v2 = c; v1v2 -= b;
      Vec  v0p = p; v0p -= a;
      Vec  t = cross(v0p,n);
      float aa= sprod3(t,ac) * -invD;
      float bb = sprod3(t,ab) * invD;
      float s01, s02, s12;
    
    
      // Calculate the distance to an edge or a corner vertex
      if(aa < 0){
        s02 = sprod3(ac,v0p) / sqrnorm3(ac);
        if(s02 < 0.0){
          s01 = sprod3(ab,v0p) / sqrnorm3(ab);
          if(s01 <= 0.0){
            v0p = a;
          }else if(s01 >= 1.0){
            v0p = b;
          }else{
            (v0p = a) += (ab *= s01);
          }
        }else if(s02 > 1.0){
          s12 = sprod3(v1v2,( p - b )) / sqrnorm3(v1v2);
          if(s12 >= 1.0){
            v0p = c;
          }else if(s12 <= 0.0){
            v0p = b;
          }else{
            (v0p = b) += (v1v2 *= s12);
          }
        }else{
          (v0p = a) += (ac *= s02);
        }
      }else if(bb < 0.0){// Calculate the distance to an edge or a corner vertex
        s01 = sprod3(ab,v0p) / sqrnorm3(ab);
        if(s01 < 0.0){
          s02 = sprod3(ac,v0p) / sqrnorm3(ac);
          if(s02 <= 0.0){
            v0p = a;
          }else if(s02 >= 1.0){
            v0p = c;
          }else{
            (v0p = a) += (ac *= s02);
          }
        }else if(s01 > 1.0){
          s12 = sprod3(v1v2,( p - b )) / sqrnorm3(v1v2);
          if(s12 >= 1.0){
            v0p = c;
          }else if(s12 <= 0.0){
            v0p = b;
          }else{
            (v0p = b) += (v1v2 *= s12);
          }
        }else{
          (v0p = a) += (ab *= s01);
        }
      }else if(aa+bb > 1.0){  // Calculate the distance to an edge or a corner vertex
        s12 = sprod3(v1v2,( p - b )) / sqrnorm3(v1v2);
        if(s12 >= 1.0){
          s02 = sprod3(ac,v0p) / sqrnorm3(ac);
          if(s02 <= 0.0){
            v0p = a;
          }else if(s02 >= 1.0){
            v0p = c;
          }else{
            (v0p = a) += (ac *= s02);
          }
        }else if(s12 <= 0.0){
          s01 = sprod3(ab,v0p) / sqrnorm3(ab);
          if(s01 <= 0.0){
            v0p = a;
          }else if(s01 >= 1.0){
            v0p = b;
          }else {
            (v0p = a) += (ab *= s01);
          }
        }else{
          (v0p = b) += (v1v2 *= s12);
        }
      }else{// Calculate the distance to an interior point of the triangle
        n *= (sprod3(n,v0p) * invD);
        (v0p = p) -= n;
      }
      if(nearestPoint) *nearestPoint = v0p;
      v0p -= p;
      return norm3(v0p);
    }

  }
}
