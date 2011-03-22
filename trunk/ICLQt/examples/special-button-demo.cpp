/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/special-button-demo.cpp                 **
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

GUI gui;
ICLDrawWidget *w = 0;
void init(){
  gui << "draw[@handle=image]";
  gui.show();
  
  w = *gui.getValue<DrawHandle>("image");
  
  ImgQ x = scale(create("parrot"),100,100);
  w->addSpecialButton("im",&x,function((ICLWidget*)w,&ICLWidget::captureCurrentImage));
  
  ImgQ k = zeros(100,100,4);
  color(255,0,0,255);
  font(50);
  text(k,2,2,"FB");
  for(int x=0;x<100;++x){ 
    for(int y=0;y<100;++y){
      k(x,y,3) = k(x,y,0);
    }
  }
  
  w->addSpecialButton("fb",&k,function((ICLWidget*)w,&ICLWidget::captureCurrentFrameBuffer));

  
}

void run(){
  static GenericGrabber grabber(pa("-i"));
  gui["image"] = grabber.grab();
  gui["image"].update();
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(2)",init,run).exec();
}
