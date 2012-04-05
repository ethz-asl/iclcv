/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/PWCGrabber.h                             **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Götting, Robert Haschke,  **
**          Andre Justus                                           **
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

#ifndef ICLPWCGRABBER_H
#define ICLPWCGRABBER_H

#include <ICLIO/GrabberHandle.h>
#include <string>

namespace icl{
  
  /// Camera Grabber for Phillips Webcams \ingroup GRABBER_G
  /** The PWCGrabber provides basic abilities for grabbing images from Phillips
  USB Webcams (that are using the PWC Kernel module). The Grabber will create
  an internal grabber thread, that grabs continously images into an internal 
  ring-buffer. This will not slow down the processor performance much, as the
  grabber thread sleeps during that time, that is needed to transfer each
  single frame from the camera. The transfer uses dma (direct memory access) 
  for best performance (i think).
  The received frames have the "YUV-420" format, which means, that the color
  channels U and V each have half X and half Y resolution as the Y-channel. 
  The transformation from the YUV-420 format is not a default part of the
  iclcc color conversion function, because it deals with images, that are
  composed of channels with different sizes. 
  A dedicated conversion function: <i>convertYUV420ToRGB8</i> which is IPP-
  optimized if the HAVE_IPP flag is defined, is used instead.
  ICLs color conversion routines do not support conversion from any to 
  to any format, so not all destination formats are supported directly.
  Directly supported formats are:
   - <b>formatRGB with depth8u</b> In this case <i>convertYUV420ToRGB8</i> is
     used to convert directly into the output buffer image.
   - <b>formatYUV</b> in this case no color conversion is necessary. The output
     data buffer is just filled with: a copy of the Y-channel, and a scaled 
     copy of the U/V channel.
  
  This time, if the destination image has another then one of this formats, 
  the conversion result is buffered into an internal buffer image with formatRGB 
  and depth8u, followed by a conversion call from the buffer into the destination image
  using the Converter.  
  @see Converter
  */
  
  class PWCGrabberImpl : public Grabber {
    public:    
    friend class PWCGrabber;

    private:
    /// Base constructor (recommended)
    /** call init(..) savely after object creation to avoid an exit(-1) call when the 
        desired device was not found **/
    PWCGrabberImpl(void);
    
    /// Deprecated contstructor for direct instantiation of a valid grabber object
    /** use the default contructor instead and call init(..) to get initialization 
        results 
        @param s internal grabbing size for pwc if Size::null, the current size is used!
        @param fFps speed of the internal grabber thread 
        @param iDevice /dev/video - device to use (0..4) **/
    PWCGrabberImpl(const Size &s, float fFps=30, int iDevice = 0); 

    public:
    /// Destructor
    ~PWCGrabberImpl(void);
    
    /// creates a list with all available PWC device indices
    /** The resulting vector contains device indices form 0 to 3
        depending on currently found PWC-devices. E.g. if the
        returned vector is {0}, just a single device was found
        at /dev/video0 
    **/
    static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

    /// retrieve currently used device number
    int getDevice () const {return m_iDevice;}

    /// retrieve currently set frame rate
    float getFrameRate () const {return m_fFps;}

    /// initialisation function
    /** initializes a camera on /dev/video\<iDevice\>. be sure that you call init(..) and init(..) returns true
        before calling grab(..)
        @param s size of grabbed images (if size::null, the current size is used!)
        @param fFps grabbing rate
        @param iDevice USB grabbing device {0,1,2,3}
        @param echoOff if set to true, no warnings and error messages are printed. This feature
                       is used by getDeviceList(), which just tests, if the init function would
                       return true, to estimate if a specific device is valid.
    **/
    bool init(const Size &s,float fFps=30, int iDevice = 0, bool echoOff=false);
    
    /// grabbing function  
    virtual const ImgBase* acquireImage();

    /** @{ @name properties and parameters */
    
    /// interface for the setter function for video device properties 
    /** \copydoc icl::Grabber::setProperty(const std::string&,const std::string&) **/
    virtual void setProperty(const std::string &property, const std::string &value);
    
    /// returns a list of properties, that can be set usingsetProperty
    /** @return list of supported property names **/
    virtual std::vector<std::string> getPropertyList();
    
    /// get type of property
    /** \copydoc icl::Grabber::getType(const std::string &)*/
    virtual std::string getType(const std::string &name);

    /// get information of a property valid values
    /** \copydoc icl::Grabber::getInfo(const std::string &)*/
    virtual std::string getInfo(const std::string &name);

    /// returns the current value of a property or a parameter
    virtual std::string getValue(const std::string &name);
    /** @} */

    /** @{ @name additional special functions for PWC-Param access **/

    /// restores user settings
    /** @return success value */
    bool restoreUserSettings();
    
    /// saves user settings
    /** @return success value */
    bool saveUserSettings();

    /// sets the current gain level
    /** @param iGainValue gain level in range [0..65535]
        @return success value */
    bool setGain(signed int iGainValue);

    /// sets for whitebalance mode 
    /** possible modes are
        - 0 indoor
        - 1 outdoor
        - 2 "fl-tube"
        - 4 auto
        @param mode mode specifier
        @return if successful
    */
    bool setWhiteBalance(int mode) {return setWhiteBalance(mode, -1, -1);}
    /// manually sets white balance for red and blue channel
    /**
        @param manual_red definition of manual red white balance value 
               left unregarded if -1
        @param manual_blue definition of manual blue white balance value 
               left unregarded if -1
        @return if successful
    */
    bool setWhiteBalance(int manual_red, int manual_blue)
       {return setWhiteBalance (3, manual_red, manual_blue);}

    /// sets new grabbing size
    /** internally the grabber is released and created new(this lasts
        less then a second )
        @param size new size ( one of (160x120,320x240 or 640x480)
        @return if successful
    */
    bool setGrabbingSize(const Size &size);

    
    /// sets the compression factor
    /** @param level value in {0=no compression, 1,2,3=highest compression]
        @return if successful */
    bool setCompression(int level);
    
    /// sets the shutterspeed
    /** @param level speed in range [-1,65535] where -1 is auto 
        @return if successful */
    bool setShutterSpeed(int level);


    /// sets led state
    /** @param time value between 0 and 25000
        @param on sets LED state to on, if true, or to off if false 
        @return if successful **/
    bool setLED(bool on, int time);


    /** @} **/
    
    private:   
    
    /// internal release function (for destructor and setting new size)
    void releaseAll();
    /// internal white balance setting
    bool setWhiteBalance(int mode, int manual_red, int manual_blue);
    
    /// current grabbing width
    int m_iWidth;
    
    /// current grabbing height
    int m_iHeight;
    
    /// current grabbing device
    int m_iDevice;
    
    /// current grabbing fps value
    float m_fFps;

    /// current Img8u image buffer
    Img8u *m_poRGB8Image;
  };


  /// Phillips Webcam grabber using pwc interface \ingroup GRABBER_G \ingroup DC_G
  /** for more details: @see PWCGrabberImpl */
  class PWCGrabber : public GrabberHandle<PWCGrabberImpl>{
    static inline std::string create_id(int dev){
      return std::string("device-")+str(dev);
    }
    
    public:
    /// returns a list of available pwc devices 
    /** @see PWCGrabberImpl for more details*/
    static inline const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan){
      return PWCGrabberImpl::getDeviceList(rescan);
    }
    
    /// creates a new PWCGrabber instance
    /** @see PWCGrabberImpl for more details */
    inline PWCGrabber(const Size &s, float fFps=30, int iDevice = 0){
      std::string id = create_id(iDevice);
      if(isNew(id)){
        initialize(new PWCGrabberImpl(s,fFps,iDevice),id);
      }else{
        initialize(id);
      }
    }
    /// empty constructor (initialize late using init())
    /** @see PWCGrabberImpl for more details */
    inline PWCGrabber(){}
    
    /// save initialization of the PWCGrabber instance
    /** @see PWCGrabberImpl for more details */
    inline bool init(const Size &s,float fFps=30, int iDevice = 0, bool echoOff=false){
      std::string id = create_id(iDevice);
      if(isNew(id)){
        initialize(new PWCGrabberImpl,id);
        Mutex::Locker l(m_instance->mutex);
        return m_instance.get()->ptr->init(s,fFps,iDevice,echoOff);
      }else{
        initialize(id);
        // resetting the parameters
        Mutex::Locker l(m_instance->mutex);
        m_instance.get()->ptr->setGrabbingSize(s);
        return true;
      }
    }
  };
  
}

#endif
