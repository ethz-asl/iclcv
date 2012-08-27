/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/CreateGrabber.h                          **
** Module : ICLIO                                                  **
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

#ifndef ICL_CREATE_GRABBER_H
#define ICL_CREATE_GRABBER_H

#include <ICLCore/Color.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/Size32f.h>

#include <ICLIO/GrabberHandle.h>

namespace icl{
  namespace io{
  
    
    /// Implementation class for the CreateGrabber
    class CreateGrabberImpl : public Grabber{
      public:
      friend class CreateGrabber;
  
      /// default grab function
      virtual const ImgBase* acquireImage();
  
      /// Destructor
      ~CreateGrabberImpl();
      
      private:
      /// Create a CreateGrabber with given max. fps count
      CreateGrabberImpl(const std::string &what);
  
      /// internal image
      const ImgBase *m_image;
    };  
    /** \endcond */
  
    /// Create Grabber class that provides an image from ICL's create function
    /** This grabber can be used as placeholder whenever no senseful Grabber
        is available. It provides an instance of an image that is created with 
        the icl::TestImages::create function */
    class CreateGrabber : public GrabberHandle<CreateGrabberImpl>{
      public:
      
      /// Creates a CreateGrabber instance
      /** allowed values for what can be found in the documentation of
          icl::TestImages::create */
      inline CreateGrabber(const std::string &what="parrot"){
        if(isNew(what)){
          initialize(new CreateGrabberImpl(what),what);
        }else{
          initialize(what);
        }
      }
    };
  } // namespace io
}

#endif
