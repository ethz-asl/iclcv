/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICLSONYFWGRABBER_H
#define ICLSONYFWGRABBER_H

#ifdef WIN32
#ifdef WITH_SONYIIDC

#include <ICLIO/Grabber.h>
#include <sony/iidcapi.h>
#include <iostream>
#include <vector>
#include <string>

#define SONY_GAIN 470
#define SONY_SHUTTER 562
#define SONY_WHITEBALANCE_U 45
#define SONY_WHITEBALANCE_V 12
#define SONY_HUE 15

#define GET_FORMAT(formatindex) ((formatindex>>8)&0x07)
#define GET_MODE(formatindex) ((formatindex>>12)&0x07)
#define GET_COLORCODING(formatindex) (formatindex & 0x0f)

// TODO THIS MUST NOT BE LOCATED IN A .h FILE !!
using namespace std;

namespace icl {

  /// Grabber implementation for grabbing on our Camera-Head \ingroup GRABBER_G
  class SonyFwGrabber : public Grabber {

    public:
    SonyFwGrabber();
    ~SonyFwGrabber();

    bool init();
    bool initTrigger();

    /// grabbing function  
    /** \copydoc icl::Grabber::grab(icl::ImgBase**)  **/    
    virtual const ImgBase* grabUD(ImgBase **poDst=0);

    void grabStereo (ImgBase*& ppoDstLeft, ImgBase*& ppoDstRight);
    void grabStereoTrigger (ImgBase*& leftImage, ImgBase*& rightImage);

    /** @{ @name properties and parameters */
    
    /// interface for the setter function for video device properties 
    /** \copydoc icl::Grabber::setProperty(const std::string&,const std::string&) **/
    virtual void setProperty(const std::string &property, const std::string &value);

    /// returns a list of properties, that can be set using setProperty
    /** @return list of supported property names **/
    virtual std::vector<std::string> getPropertyList();

    /// returns the current value of a property or a parameter
    virtual std::string getValue(const std::string &name);
    /** @} */

    void setDevice(int dev) {
      if ((dev <= m_lNumCameras) && (dev >=0))
        m_iDevice = dev; 
    }
    inline icl::Size getSize() { return icl::Size(m_iWidth, m_iHeight); }

    private:
    /// current grabbing width
    int m_iWidth;
    /// current grabbing height
    int m_iHeight;
    /// current grabbing device
    int m_iDevice;
    /// current grabbing fps value
    float m_fFps;
    /// current number of devices
    long m_lNumCameras;
    /// image data buffers for grabbing
    BYTE ***m_pppImgBuffer;
    /// camera handels
    HIIDC m_hCamera[10];

    void GetCamAllString(long camIndex, char *strCamera);
    void copyGrabbingBuffer (int iDevice, ImgBase *poDst);

  };

}

#endif //SONYIIDC
#endif //WIN32
#endif //SONYFWGRABBER_H
