/*********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLIO/OpenCVCamGrabber.h                       **
 ** Module : ICLIO                                                  **
 ** Authors: Christian Groszewski, Christof Elbrechter, V. Richter  **
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
#include <ICLCore/OpenCV.h>
#include <ICLIO/GrabberHandle.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Mutex.h>

#ifdef HAVE_OPENCV
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#endif
namespace icl{
  namespace io{

    /// Grabber Implementation for using an OpenCV based camera source
    class OpenCVCamGrabberImpl : public Grabber{
      private:
        /// Wrapped Device struct
        CvCapture *cvc;
        ///number of device
        int device;
        ///
        utils::Mutex m_mutex;
        ///Buffer for imagescaling
        core::ImgBase *m_buffer;
      public:

        /// grab function grabs an image (destination image is adapted on demand)
        /** @copydoc icl::io::Grabber::grab(core::ImgBase**) **/
        virtual const core::ImgBase *acquireImage();

        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);

        /// Constructor creates a new OpenCVCamGrabber instance from a given device
        /** @param dev device to use
          */
        OpenCVCamGrabberImpl(int dev=0) throw (utils::ICLException);

        /// Destructor
        ~OpenCVCamGrabberImpl();
    };


    /// Grabber class that uses OpenCV's grabbing function to grab camera images
    class OpenCVCamGrabber : public GrabberHandle<OpenCVCamGrabberImpl>{
      public:
        /// Creates new OpenCV based grabber
        /** @param dev specifies the device index
             (0 chooses any available device automatically)
          you can also use
          opencv's so called 'domain offsets': current values are:
          - 100 MIL-drivers (proprietary)
          - 200 V4L,V4L2 and VFW,
          - 300 Firewire,
          - 400 TXYZ (proprietary)
          - 500 QuickTime
          - 600 Unicap
          - 700 Direct Show Video Input
          (e.g. device ID 301 selects the 2nd firewire device)
       */
        inline OpenCVCamGrabber(int dev){
          std::string id = utils::str(dev);
          if(isNew(id)){
            initialize(new OpenCVCamGrabberImpl(dev),id);
          }else{
            initialize(id);
          }
        }

        // returns a list of all valid device IDs
        /** Internally, for each device index i=0,1,2,...,
          a grabber-instance is created. If any of these creation trys returns an error,
          no further devices are tested.
          @param rescan if this params is a positive or zero integer, it defines the
          last device ID that is tried internally */

        /// simpler version of getDeviceListN detecting a maxinum of 100 devices
        static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);
    };
    

  } // namespace io
}

