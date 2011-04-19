/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/configurable-gui-example.cpp            **
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

#include <ICLQuick/Common.h>
#include <ICLUtils/Configurable.h>

GUI gui("hbox");


struct C : public Configurable{
  C(){
    addProperty("general.x","range:slider","[0,100]",50);
    addProperty("general.y","range:spinbox","[0,10]",4);
    addProperty("general.z","command","");
    setConfigurableID("c");
  }
};

struct B : public Configurable, public Thread{
  C c;
  B(){
    addProperty("time","info","",Time::now().toString(),100);
    addProperty("general.f","flag","",true);
    addChildConfigurable(&c);
    start();
  }
  
  virtual void run(){
    while(true){
      Thread::msleep(100);
      setPropertyValue("time",Time::now().toString());
    }
  }
};

struct A : public Configurable{
  B b;
  A(){
    addProperty("general.my range","range:slider","[0,100]",50);
    addProperty("general.spinner-property","range:spinbox","[0,10]",4);
    addProperty("general.some menu property","menu","a,b,c,d","a");
    addProperty("general.hey its a flag","flag","",true);

    setConfigurableID("a");
    
    addChildConfigurable(&b,"b");
  }
} a;


void init(){
  gui << "prop(a)[@label=properties of a]" 
      << "prop(a)[@label=also properties of a]" 
      << "prop(c)[@label=properties of c only]" 
      << "!show";
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init).exec();
}
