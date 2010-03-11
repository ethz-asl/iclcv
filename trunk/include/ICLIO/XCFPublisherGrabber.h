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

#ifdef HAVE_XCF
#ifndef ICL_XCF_PUBLISHER_GRABBER_H
#define ICL_XCF_PUBLISHER_GRABBER_H

#include <ICLIO/XCFGrabberBase.h>
#include <xcf/Subscriber.hpp>
#include <xmltio/Location.hpp>

namespace icl{

  class XCFPublisherGrabber : public XCFGrabberBase{
    public:
    XCFPublisherGrabber(const std::string &streamName, 
                        ::XCF::RecoverLevel l = (::XCF::RecoverLevel)
                        ::XCF::Implementation::Properties::singleton()
                        ->getPropertyAsInt("XCF.Global.RecoverLevel"));
    virtual ~XCFPublisherGrabber();
    
    /// set XCF recover level
    void setRecoverLevel (XCF::RecoverLevel l) {
       m_subscriber->setRecoverLevel (l);
    }

    protected:
    virtual void receive (XCF::CTUPtr& result);
    
    private:
    XCF::SubscriberPtr m_subscriber;
  };
}

#endif // ICL_XCF_PUBLISHER_GRABBER_H

#endif // HAVE_XCF
