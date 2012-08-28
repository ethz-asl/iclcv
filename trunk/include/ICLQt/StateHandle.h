/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/StateHandle.h                            **
** Module : ICLQt                                                  **
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

#pragma once

#include <string>
#include <ICLQt/GUIHandle.h>
#include <ICLUtils/StringUtils.h>

namespace icl{
  namespace qt{
  
    /** \cond */
    class ThreadedUpdatableTextView;
    /** \endcond */
  
  
    /// Class for GUI-Label handling a so called state component\ingroup HANDLES
    /** You can add a state component as an alternative std::cout for debugging
        or system state messages
        @see GUI */
    class StateHandle : public GUIHandle<ThreadedUpdatableTextView>{
      int maxLen;
      public:
      
      /// Create an empty handle
      StateHandle(){}
      
      /// Create a new LabelHandle
      StateHandle(ThreadedUpdatableTextView *l, GUIWidget *w, int maxLen):
        GUIHandle<ThreadedUpdatableTextView>(l,w),maxLen(maxLen){}
      
      /// appends a string
      void append(const std::string &text);
      
      /// appends anything in std::ostream manner
      template<class T>
      StateHandle &operator<<(const T &t){
        append(str(t));
        return *this;
      }
      
      /// erases all lines
      void clear();
      
      /// sets max line length (odd lines are removed)
      void setMaxLen(int maxLen);
      
      /// returns curren max line length
      int getMaxLen() const;
  
      private:
      /// utitlity function
      ThreadedUpdatableTextView *text() { return **this; }
  
      /// utitlity function
      const ThreadedUpdatableTextView *text() const{ return **this; }
      
      /// utility function
      void removeOldLines();
    };
  
   
    
   
  } // namespace qt
}                               

