/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/RotateOp.h                           **
** Module : ICLFilter                                              **
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

#ifndef ROTATE_OP_H
#define ROTATE_OP_H

#include <ICLFilter/AffineOp.h>

namespace icl{
  
 /// Class to rotate images \ingroup UNARY \ingroup AFFINE
  class RotateOp : public AffineOp {
    public:
    /// Constructor
    RotateOp (double dAngle=0.0, scalemode eInterpolate=interpolateLIN) :
      AffineOp (eInterpolate) {
        setAngle(dAngle);
      }
    
    /// sets the rotation angle
    /**
      @param dAngle angle in degrees (clockwise) 
    */
    void setAngle (double dAngle) {
      AffineOp::reset ();
      AffineOp::rotate (dAngle);
    }
    
    // apply should still be public
    ///applies the rotation
    AffineOp::apply;

    private: // hide the following methods
    AffineOp::translate;
    AffineOp::scale;
  };
}
#endif
