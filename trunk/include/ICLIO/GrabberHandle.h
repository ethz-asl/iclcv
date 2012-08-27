/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/GrabberHandle.h                          **
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

#ifndef ICL_GRABBER_HANDLE_H
#define ICL_GRABBER_HANDLE_H

#include <ICLIO/Grabber.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/SmartPtr.h>
#include <ICLUtils/StringUtils.h>
#include <map>
#include <string>

namespace icl{
  namespace io{
    /** \cond */
    template <class G> class GrabberHandle;
    /** \endcond */
    
    /// Internal storage class for the GrabberHandle wrapper class
    template<class G>
    class GrabberHandleInstance{
      /// only GrabberHandle are allowed to create this structs
      template <class G2> friend class GrabberHandle;
      
      /// creates a new Instance with given id, and Grabber-Pointer
      GrabberHandleInstance(std::string id="", G* ptr = 0):
        id(id),ptr(ptr){}
  
      public:
      /// Destructor
      ~GrabberHandleInstance(){
        ICL_DELETE(ptr);
      }
      /// Grabbers id
      std::string id;
      
      /// Underlying Grabber instance
      G *ptr;
      
      /// mutex protecting ptr
      mutable Mutex mutex;
    };
  
  
    /// Wrapper class for Grabber instances to provide "shared" grabber instances
    /** In general, grabber implementations do not provide shared device functionalities. Once
        having implemented a Grabber class e.g. MyGrabber, the GrabberHandle class can easily 
        be used to make MyGrabber Support shared access to underlying grabbing devices.
        
        \section PROB The Problem
        If we are using an instance of a dc grabber associated with a specific DC-Device, it's not 
        possible to access the grabber and therewith the input image stream from another instance. 
        Hence underlying dc devices can not be opened twice, instantiating another Grabber on
        the same device would cause some serious problems that will cause a segmentation fault 
        (in the best case).
  
        \section ADAPT Adapting an existing Grabber to provide shared device access
        E.g. you have written a grabber class called DemoGrabber, that doesn't provide shared
        access to the underlying grabbing device (we use the actually existing ICLIO/DemoGrabber here).
  
        Firstly, you have to rename the DemoGrabber to another name e.g. DemoGrabberImpl. Subsequent,
        you can implement a new DemoGrabber class by deriving the GrabberHandle<DemoGrabberImpl> template
        class. In your new DemoGrabber implementation you have to implement only the constructor, with
        respect to the following points:
        
        \code
        // new demo grabber class deriving the appropiate GrabberHandle-template
        class DemoGrabber : public GrabberHandle<DemoGrabberImpl>{
           public:
           // Constructor (no explicit call to the super class constructor)
           // but identical parameter list
           DemoGrabber(float maxFPS=30){  
              std::string id = str(maxFPS);   // we need an ID that represents the
                                              // the underlying wrapped device, and that can be 
                                              // associated uniquely 
  
              // initialization of the top level grabber class:
              // 1st: check, if given grabber id is brand new
              // 2nd: ** if the id is new: initialize a real new grabber 
              //         by passing the parameters to the constructor of
              //         of the underlying class
              //      ** if not, then initialize this instance without a new 
              //         grabber, by calling initialize(id). Here, this becomes
              //         a shared grabber copy of the already existing one
              if(isNew(id)){
                 initialize(new DemoGrabberImpl(maxFPS),id);
              }else{
                 initialize(id);
              }
           }
        };
        \endcode
    */
    template<class G>
    class GrabberHandle : public Grabber{
  
      public:
  
      typedef SmartPtr<GrabberHandleInstance<G> > InstancePtr;
      typedef std::map<std::string,InstancePtr> InstanceMap;
  
      protected:
      /// only used internally, returns the GrabberHandles internal ID
      inline std::string getID() const{
        ICLASSERT_RETURN_VAL(!isNull(),"");
        return m_instance->id;
      } 
      
      /// used in derived classes to determine wheter device is shared or new
      static inline bool isNew(const std::string &id){
        Mutex::Locker l(s_mutex);
        return s_instances.find(id) == s_instances.end();
      }
  
      /// used in derived classes to initialize itself as a shared copy 
      inline void initialize(const std::string &id){
        Mutex::Locker l(s_mutex);
        m_instance = s_instances[id];
      }
      
      /// used inderived classes to initialize itself as brand new instance
      inline void initialize(G* g, const std::string &id){
        ICLASSERT_RETURN(isNew(id));
        Mutex::Locker l(s_mutex);
        m_instance = s_instances[id] = InstancePtr(new GrabberHandleInstance<G>(id,g));
      }
      public:
  
      /// Destructor
      inline ~GrabberHandle(){
        if(isNull()) return;
        Mutex::Locker l(s_mutex);
        
        if(m_instance.use_count() == 2){
          // only two remaining instances: m_instance and s_instances[getID()]
       
          // nice C++ code here -> the typename must be added here because the iterator
          // type depends indirectly on the template parameter (or something like that)
          typename InstanceMap::iterator it = s_instances.find(m_instance->id);
          ICLASSERT_RETURN(it != s_instances.end());
          s_instances.erase(it); // releases s_instances[getID()]
          // m_instance itself is released automatically by it's destructor
        }
      }
  
      /// Determine wheter the underlying grabber was created correctly
      inline bool isNull() const{
        return !static_cast<bool>(m_instance);
      }
     
      /// calles underlying grabber's grab function
      virtual inline const ImgBase* acquireImage(){
        ICLASSERT_RETURN_VAL(!isNull(),0);
        Mutex::Locker l(m_instance->mutex);
        return m_instance->ptr->acquireImage();
      }
      /// calles underlying grabber's setProperty function
      virtual inline void setProperty(const std::string &property, const std::string &value){      
        ICLASSERT_RETURN(!isNull());
        Mutex::Locker l(m_instance->mutex);
        m_instance->ptr->setProperty(property,value);
      }
      /// calles underlying grabber's getPropertyList function
      virtual inline std::vector<std::string> getPropertyList(){
        ICLASSERT_RETURN_VAL(!isNull(),std::vector<std::string>());
        Mutex::Locker l(m_instance->mutex);
        return m_instance->ptr->getPropertyList();
      }
      /// calles underlying grabber's supportsProperty function
      virtual inline  bool supportsProperty(const std::string &property){
        ICLASSERT_RETURN_VAL(!isNull(),false);
        Mutex::Locker l(m_instance->mutex);
        return m_instance->ptr->supportsProperty(property);
      }
      /// calles underlying grabber's getType function
      virtual inline std::string getType(const std::string &name){
        ICLASSERT_RETURN_VAL(!isNull(),"undefined");
        Mutex::Locker l(m_instance->mutex);
        return m_instance->ptr->getType(name);
      }
      /// calles underlying grabber's getInfo function
      virtual inline std::string getInfo(const std::string &name){
        ICLASSERT_RETURN_VAL(!isNull(),"undefined");
        Mutex::Locker l(m_instance->mutex);
        return m_instance->ptr->getInfo(name);
      }
      /// calles underlying grabber's getValue function
      virtual inline std::string getValue(const std::string &name){
        ICLASSERT_RETURN_VAL(!isNull(),"undefined");
        Mutex::Locker l(m_instance->mutex);
        return m_instance->ptr->getValue(name);
      }
  
      virtual inline int isVolatile(const std::string &propertyName){
        ICLASSERT_RETURN_VAL(!isNull(),0);
        Mutex::Locker l(m_instance->mutex);
        return m_instance->ptr->isVolatile(propertyName);
      }
  
  
      /// internally set a desired format
      virtual void setDesiredFormatInternal(format fmt){
        ICLASSERT_RETURN(!isNull());
        Mutex::Locker l(m_instance->mutex);
        m_instance->ptr->setDesiredFormatInternal(fmt);
      }
  
      /// internally set a desired format
      virtual void setDesiredSizeInternal(const Size &size){
        ICLASSERT_RETURN(!isNull());
        Mutex::Locker l(m_instance->mutex);
        m_instance->ptr->setDesiredSizeInternal(size);
      }
  
      /// internally set a desired format
      virtual void setDesiredDepthInternal(depth d){
        ICLASSERT_RETURN(!isNull());
        Mutex::Locker l(m_instance->mutex);
        m_instance->ptr->setDesiredDepthInternal(d);
      }
  
      /// returns the desired format
      virtual format getDesiredFormatInternal() const{
        ICLASSERT_RETURN_VAL(!isNull(),(format)-1);
        Mutex::Locker l(m_instance->mutex);
        return m_instance->ptr->getDesiredFormatInternal();
      }
  
      /// returns the desired format
      virtual depth getDesiredDepthInternal() const{
        ICLASSERT_RETURN_VAL(!isNull(),(depth)-1);
        Mutex::Locker l(m_instance->mutex);
        return m_instance->ptr->getDesiredDepthInternal();
      }
  
      /// returns the desired format
      virtual Size getDesiredSizeInternal() const{
        ICLASSERT_RETURN_VAL(!isNull(),Size::null);
        Mutex::Locker l(m_instance->mutex);
        return m_instance->ptr->getDesiredSizeInternal();
      }
  
      /// passes registered callback to the internal pointer
      virtual void registerCallback(callback cb){
        ICLASSERT_RETURN(!isNull());
        Mutex::Locker l(m_instance->mutex);
        return m_instance->ptr->registerCallback(cb);
      }
       
      /// passes registered callback to the internal pointer
      virtual void removeAllCallbacks(){
        ICLASSERT_RETURN(!isNull());
        Mutex::Locker l(m_instance->mutex);
        return m_instance->ptr->removeAllCallbacks();
      }
  
      /// returns all current instances available
      static inline const InstanceMap &getInstanceMap() { return s_instances; }
      
      /// locks the static instance map (obtainable using getInstanceMap())
      static inline void lockInstanceMap() { s_mutex.lock(); }
  
      /// un-locks the static instance map (obtainable using getInstanceMap())
      static inline void unlockInstanceMap() { s_mutex.unlock(); }
    
      protected:
      /// internal instance pointer
      InstancePtr m_instance;
      
      private:
  
      /// list of all instances
      static InstanceMap s_instances;
      
      /// static mutex protecting the instance map
      static Mutex s_mutex;
    };
  
    /** \cond */
    template<class G> std::map<std::string,icl::SmartPtr<GrabberHandleInstance<G> > > GrabberHandle<G>::s_instances;
    template<class G> Mutex GrabberHandle<G>::s_mutex; 
    /** \endcond */
  } // namespace io
}

#endif
