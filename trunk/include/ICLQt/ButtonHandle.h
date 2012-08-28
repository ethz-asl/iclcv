/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/ButtonHandle.h                           **
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
#include <vector>
#include <ICLQt/GUIHandle.h>
#include <ICLUtils/SmartPtr.h>

/**\cond */
class QPushButton;
/**\endcond */


namespace icl{
  namespace qt{
    
    /// Special Utiltiy class for handling Button clicks in the ICL GUI API \ingroup HANDLES
    class ButtonHandle : public GUIHandle<QPushButton>{
      public:
      
      friend class ButtonGUIWidget;
      friend class ToggleButtonGUIWidget;
      
      /// creates a n empty button handle
      ButtonHandle();
  
      /// create a new event with a given button id
      ButtonHandle(QPushButton *b, GUIWidget *w);
  
      /// check if this event/button was triggered
      /** @param reset if set to true the internal boolen variable
                       is set to false, so wasTriggered returns true
                       only if the button was triggered again */
      bool wasTriggered(bool reset=true);
      
      /// trigger this event (sets the internal boolean variable to true)
      void trigger(bool execCallbacks=true){
        *m_triggered = true;
        if(execCallbacks){
          cb();
        }
      }
      
      /// sets the internal boolean variable to false
      void reset();
      
      /// returns this buttons id (uncommon)
      const std::string &getID() const ;
      
      private:
      
      SmartPtr<bool> m_triggered; //!< internal boolean variable
      std::string m_sID; //!< corresponding id
    };  
  } // namespace qt
}

