/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/PylonUtils.h                             **
** Module : ICLIO                                                  **
** Authors: Viktor Richter                                         **
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

#ifndef ICL_PYLON_UTILS_H
#define ICL_PYLON_UTILS_H

#include <pylon/PylonIncludes.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Exception.h>
#include <ICLCore/CoreFunctions.h>

namespace icl {
  namespace pylon {

    /// Buffer, registered to the Pylon-drivers StreamGrabber \ingroup GIGE_G
    template <typename T>
    class PylonGrabberBuffer {
      private:
        T *m_pBuffer;
        Pylon::StreamBufferHandle m_hBuffer;

      public:
        PylonGrabberBuffer(size_t size) : m_pBuffer(NULL) {
          m_pBuffer = new T[size];
          if (!m_pBuffer)
            throw icl::ICLException("Not enough memory to allocate image buffer");
        }
        ~PylonGrabberBuffer(){
          if (m_pBuffer)
            delete[] m_pBuffer;
          }
        T* getBufferPointer(void) {
          return m_pBuffer;
        }
        Pylon::StreamBufferHandle getBufferHandle(void) {
          return m_hBuffer;
        }
        void setBufferHandle(Pylon::StreamBufferHandle hBuffer) {
          m_hBuffer = hBuffer;
        }
    };

    /// A buffer holding image information and timestamp \ingroup GIGE_G
    template <typename T>
    class TsBuffer {
      public:
        /// Buffer for image information
        T* m_Buffer;
        /// Buffer for image-timestamp
        uint64_t m_Timestamp;
        /// holdss the size of m_Buffer
        size_t m_Size;

        /// Constructor allocates required memory
        TsBuffer(size_t size)  : m_Timestamp(0), m_Size(size){
          m_Buffer = new T[m_Size];
          if (!m_Buffer)
            throw icl::ICLException("Not enough memory to allocate image buffer");
        }
        /// Frees allocated memory
        ~TsBuffer(){
          if (m_Buffer){
            delete[] m_Buffer;
          }
        }
        /// uses icl::copy to write buffer-data to m_Buffer
        /**
        * casts buffer to internal type and copies m_Size blocks
        * to m_Buffer.
        */
        void copy(void* buffer){
          T* tmp = (T*) buffer;
          icl::copy(tmp, tmp + m_Size, m_Buffer);
        }
    };

    /// Utility Structure \ingroup GIGE_G
    /**
     * This struct is used to initialize and terminate the pylon environment.
     * It intializes Pylon on creation and terminates it on destruction.
     * Uses a static counter to ensure initialization only on first and
     * termination on last call.
     */
    struct PylonAutoEnv{
      public:
        /// Initializes Pylon environment if not already done.
        PylonAutoEnv();
        /// Terminates Pylon environment when (calls to term) == (calls to init).
        ~PylonAutoEnv();
        /// Initializes the Pylon environment.
        /** @return whether Pylon::PylonInitialize() actually was called. */
        static bool initPylonEnv();
        /// terminates the Pylon environment.
        /** @return whether Pylon::PylontTerminate() actually was called. */
        static bool termPylonEnv();
    };

    /// Utility Structure \ingroup GIGE_G
    /**
    * This struct is used to realize easy interruprion of Pylon grabbing
    * processes.
    */
    struct Interruptable{
        /// Virtual destructor
        virtual ~Interruptable() {}
        /// starts the acquisition
        virtual void acquisitionStart() = 0;
        /// stops the acquisition
        virtual void acquisitionStop() = 0;
        /// starts grabbing
        virtual void grabbingStart() = 0;
        /// stops grabbing
        virtual void grabbingStop() = 0;
    };

    /// Utility Structure \ingroup GIGE_G
    /**
    * This struct is used to stop the acquisition and restart it
    * on destruction. It also locks the mutex lock so this can't be
    * created twice at the same time.
    */
    struct AcquisitionInterruptor{
      private:
        /// A pointer to the PylonGrabberImpl that is to be stopped.
        Interruptable* m_Interu;

      public:
        /// stops the acquisiton
        /**
        * @param i The Interruptable to stop.
        * @param mock Whether to mock the stopping. This can be used to
        *        create 'shallow' AcquisitionInterruptors on conditions.
        */
        AcquisitionInterruptor(Interruptable* i, bool mock=false);
        /// Starts acquisition if stopped before.
        ~AcquisitionInterruptor();
    };

    /// Utility Structure \ingroup GIGE_G
    /**
    * This struct is used to stop grabbing and restart it on destruction.
    * This should not be created while image acquisition is still active.
    */
    struct GrabbingInterruptor{
    private:
      /// A pointer to the Interruptable that needs to be stopped.
      Interruptable* m_Interu;

    public:
      /// Constructor calls grabbingStop().
      /**
      * @param i The Interruptable to stop.
      * @param mock Whether to mock the stopping. This can be used to
      *        create 'shallow' GrabbingInterruptors on conditions.
      */
      GrabbingInterruptor(Interruptable* i, bool mock=false);
      /// Destructor calls grabbingStart().
      ~GrabbingInterruptor();
    };

    /// Prints help-information to std::cout
    void printHelp();
    /// Uses args to find demanded device
    Pylon::CDeviceInfo getDeviceFromArgs(std::string args) throw(ICLException);
    /// Uses args to find out which BufferChannel to use.
    int channelFromArgs(std::string args);
    /// Returns a list of available Pylon devices.
    /** @param filter if provided will be used to filter available devices */
    Pylon::DeviceInfoList_t
    getPylonDeviceList(Pylon::DeviceInfoList_t* filter=NULL);

  } //namespace
} //namespace

#endif

