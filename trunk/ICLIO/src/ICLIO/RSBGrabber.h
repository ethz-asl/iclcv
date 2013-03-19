/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/RSBGrabber.h                           **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
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


#pragma once

#include <ICLIO/Grabber.h>

#if !defined(HAVE_RSB) || !defined(HAVE_PROTOBUF)
#warning "This header should only be included if HAVE_RSB and HAVE_PROTOBUF are defined and available in ICL"
#endif

namespace icl{
  namespace io{
    
    /// Grabber implementation for RSB based image transfer
    class RSBGrabber : public Grabber{
        struct Data;  //!< pimpl type
        Data *m_data; //!< pimpl pointer

      public:

        /// empty constructor (creates a null instance)
        RSBGrabber();

        /// Destructor
        ~RSBGrabber();

        /// main constructor with given scope and comma separated transportList
        /** supported transports are socket, spread and inprocess. Please note, that
          the spread-transport requires spread running. */
        RSBGrabber(const std::string &scope, const std::string &transportList="spread");

        /// deferred intialization with given scope and comma separated transportList
        /** supported transports are socket, spread and inprocess. Please note, that
          the spread-transport requires spread running. */
        void init(const std::string &scope, const std::string &transportList="spread");

        /// grabber-interface
        virtual const core::ImgBase *acquireImage();

        /// returns whether this grabber has not jet been initialized
        inline bool isNull() const { return !m_data; }

        /// returns a list of all available rsb streams
        static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);

    };

  } // namespace io
}

