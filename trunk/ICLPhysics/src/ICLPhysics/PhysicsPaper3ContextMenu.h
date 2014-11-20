/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsPaper3ContextMenu.h   **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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

#include <ICLUtils/Function.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Point.h>
#include <string>

namespace icl{
  namespace physics{
    
    class PhysicsPaper3ContextMenu : public utils::Uncopyable{
      struct Data;
      Data *m_data;

      public: 
      typedef utils::Function<void,const std::string&> callback;
      
      PhysicsPaper3ContextMenu();
      
      PhysicsPaper3ContextMenu(const std::string &commaSepEntries);
      
      ~PhysicsPaper3ContextMenu();
      
      void addEntry(const std::string &entry);
      
      void addEntries(const std::string &commaSepEntryList);
      
      void setCallback(callback cb);
      
      void show(const utils::Point &screenPos);
    };
  }
}
