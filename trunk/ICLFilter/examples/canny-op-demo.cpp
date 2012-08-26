/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/examples/canny-op-demo.cpp                   **
** Module : ICLFilter                                              **
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

#include <ICLCV/Common.h>
#include <ICLFilter/CannyOp.h>
#include <ICLFilter/ConvolutionOp.h>


VSplit gui;


void update();

void init(){
  gui << Image().handle("image").minSize(32,24)
      << (VBox()
          << FSlider(0,2000,10).out("low").label("low").maxSize(100,2).handle("low-handle")
          << FSlider(0,2000,100).out("high").label("high").maxSize(100,2).handle("high-handle")
          <<  ( HBox()  
                << Slider(0,2,0).out("preGaussRadius").handle("pre-gauss-handle").label("pre gaussian radius")
                << Label("time").handle("dt").label("filter time in ms")
                << Button("stopped","running").out("running").label("capture")
                << CamCfg() 
               )
          )
      << Show();
  
  gui.registerCallback(function(update),"low-handle,high-handle,pre-gauss-handle");
  
  update();
}


void update(){
  static Mutex mutex;
  Mutex::Locker l(mutex);

  static GenericGrabber grabber(pa("-i"));
  
  static ImageHandle image = gui["image"];
  static LabelHandle dt = gui["dt"];
  float low = gui["low"];
  float high = gui["high"];
  int preGaussRadius = gui["preGaussRadius"];
  
  CannyOp canny(low,high,preGaussRadius);
  static ImgBase *dst = 0;

  Time t = Time::now();
  canny.apply(grabber.grab(),&dst);
  
  dt = (Time::now()-t).toMilliSecondsDouble();
  
  image = dst;
}

void run(){
  while(!gui["running"].as<bool>()){
    Thread::msleep(100);
  }
  Thread::msleep(1);
  update();
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(2)",init,run).exec();
 
  
}
