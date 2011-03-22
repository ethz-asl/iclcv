/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/examples/generic-grab-example.cpp                **
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

#include <ICLQuick/Common.h>

GUI gui("vbox");
GenericGrabber *grabber = 0;
std::string params[] = {"","0","0","*","*.ppm",""};
Mutex mutex;

void change_grabber(){
  Mutex::Locker l(mutex);
  gui_ComboHandle(source);

  std::string newType = source.getSelectedItem();
  int idx = source.getSelectedIndex();

  if(!grabber || grabber->getType() != newType){
    ICL_DELETE( grabber );
    try{
      grabber = new GenericGrabber(newType,newType+"="+params[idx],false);
    }catch(...){}
    if(grabber->isNull()){
      ICL_DELETE( grabber );
    }
  }
}

void init(){
  gui << "image[@minsize=32x24@handle=image]" 
      << "combo(null,pwc,dc,unicap,file,demo)[@label=source@out=_@handle=source]";
  
  gui.show();
  
  gui.registerCallback(change_grabber,"source");

  if(pa("-input")){
    grabber = new GenericGrabber(pa("-i"));
  }
}

void run(){
  Mutex::Locker l(mutex);
  
  if(grabber){
    gui["image"] = grabber->grab();
    gui["image"].update();
  }else{
    Thread::msleep(20);
  }
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(device=unicap,device-params=*)",init,run).exec();
}
