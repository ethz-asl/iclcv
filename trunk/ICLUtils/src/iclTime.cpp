#include <iclTime.h>
#include <sstream>
// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifdef SYSTEM_WINDOWS
#   include <time.h>
#   include <sys/timeb.h>
#else
#   include <sys/time.h>
#endif

namespace icl {

   const Time Time::null(0);
   
   Time::Time() :
      _usec(0)
   {
   }

   Time
   Time::now()
   {
#ifdef SYSTEM_WINDOWS
      struct timeb tb;
      ftime(&tb);
      return Time(tb.time * static_cast<value_type>(1000000) + tb.millitm * static_cast<value_type>(1000));
#else
      struct timeval tv;
      gettimeofday(&tv, 0);
      return Time(tv.tv_sec * static_cast<value_type>(1000000) + tv.tv_usec);
#endif
   }

   Time
   Time::seconds(value_type t)
   {
      return Time(t * static_cast<value_type>(1000000));
   }

   Time
   Time::milliSeconds(value_type t)
   {
      return Time(t * static_cast<value_type>(1000));
   }

   Time
   Time::microSeconds(value_type t)
   {
      return Time(t);
   }

   Time::value_type
   Time::toSeconds() const
   {
      return _usec / 1000000;
   }

   Time::value_type
   Time::toMilliSeconds() const
   {
      return _usec / 1000;
   }

   Time::value_type
   Time::toMicroSeconds() const
   {
      return _usec;
   }

   double
   Time::toSecondsDouble() const
   {
      return _usec / 1000000.0;
   }

   double
   Time::toMilliSecondsDouble() const
   {
      return _usec / 1000.0;
   }

   double
   Time::toMicroSecondsDouble() const
   {
      return static_cast<double>(_usec);
   }

   std::string
   Time::toString() const{
     return toStringFormated("%x %H:%M:%S:%#",32);
   }
  /*
      time_t time = static_cast<long>(_usec / 1000000);

      struct tm* t;
#ifdef SYSTEM_WINDOWS
      t = localtime(&time);
#else
      struct tm tr;
      localtime_r(&time, &tr);
      t = &tr;
#endif

      char buf[32];
      strftime(buf, sizeof(buf), "%x %H:%M:%S", t);

     std::string buf = toStringFormated("%x %H:%M:%S:",32);
     std::ostringstream os;
     os << buf << ":";
     os.fill('0');
     os.width(3);
     os << static_cast<long>(_usec % 1000000 / 1000);
      return os.str();
      */
  
  
  std::string Time::toStringFormated(const std::string &pattern,unsigned int bufferSize) const{
    
    std::ostringstream os;

    for(unsigned int i=0;i<pattern.length()-1;++i){
      if(pattern[i]=='%'){
        ++i;
        switch(pattern[i]){
          case '*': os << (_usec % 1000000); break; 
          case '#': os << (_usec % 1000000 / 1000); break;
          case '-': os << (_usec % 1000); break;
          default:
            os << pattern[i-1];
            os << pattern[i];
        }
      }else{
        os << pattern[i];
      }
    }
    
    
    time_t time = static_cast<long>(_usec / 1000000);
    
    struct tm* t;
#ifdef SYSTEM_WINDOWS
    t = localtime(&time);
#else
    struct tm tr;
    localtime_r(&time, &tr);
    t = &tr;
#endif
    
    char *buf = new char[bufferSize];
    strftime(buf,bufferSize,os.str().c_str(),t);
    std::string s(buf);
    delete [] buf;
    return s;
  }

   Time::Time(value_type usec) :
      _usec(usec)
   {
   }

   std::ostream&
   operator<<(std::ostream& out, const Time& tm)
   {
      return out << tm.toMicroSeconds() / 1000000.0;
   }

}
