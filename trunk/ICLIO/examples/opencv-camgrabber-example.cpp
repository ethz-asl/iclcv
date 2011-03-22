/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLOpenCV/examples/opencv-camgrabber-example.cpp       **
** Module : ICLOpenCV                                              **
** Authors: Christian Groszewski                                   **
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
#include <ICLIO/OpenCVCamGrabber.h>

GUI gui("hsplit");
Mutex mutex;
SmartPtr<OpenCVCamGrabber> cg = 0;

void set_brightness(){
  float f = gui.getValue<float>("bright");
  ostringstream Str;
  Str << f ;
  cg->setProperty("brightness", Str.str());
}

void set_hue(){
  float f = gui.getValue<float>("hue");
  ostringstream Str;
  Str << f ;
  cg->setProperty("hue", Str.str());
}

void set_contrast(){
  float f = gui.getValue<float>("cont");
  ostringstream Str;
  Str << f ;
  cg->setProperty("contrast", Str.str());
}

void set_saturation(){
  float f = gui.getValue<float>("sat");
  ostringstream Str;
  Str << f ;
  cg->setProperty("saturation", Str.str());
}

void set_size(){
  //Mutex::Locker lock(mutex);
  int f = gui["size"];
  ostringstream Str;
  Str << f << "x" << int(0.75*f);
  cg->setProperty("size", Str.str());
}

void printAllProperties(){
  if(cg){
    cout << "brightness: "<<cg->getValue("brightness")<<endl;
    cout << "contrast: "<<cg->getValue("contrast")<<endl;
    cout << "saturation: "<<cg->getValue("saturation")<<endl;
    cout << "hue: "<<cg->getValue("hue")<<endl;
    cout << "width: "<<cg->getValue("size")<<endl;
  }
}

void run(){
  Mutex::Locker lock(mutex);
  gui["image"] = cg->grab();
  gui["image"].update();
  gui_FPSHandle(fps);
  fps.update();
}

void init(){
  if(pa("-input")){
    cg = new OpenCVCamGrabber(parse<int>(pa("-input")));
  }else{
    cg = new OpenCVCamGrabber(0);
  }
  gui << (GUI("vbox")
          <<	"image[@handle=image@minsize=20x20]"
          << "fps(10)[@handle=fps@maxsize=100x2@minsize=8x2]"
          );
  gui << (GUI("vbox[@minsize=20x1]")
          << "slider(0,1800,600)[@out=size@handle=hsize@label=size]"
          << "label(only for cameras)"
          << "fslider(0,1,0.1)[@out=bright@handle=hbright@label=brightness]"
          << "fslider(0,1,0.1)[@out=cont@handle=hcont@label=contrast]"
          << "fslider(0,1,0.1)[@out=sat@handle=hsat@label=saturation]"
          << "fslider(0,1,0.1)[@out=hue@handle=hhue@label=hue]"
          << "button(info)[@out=info@handle=camprops@label=info]"
          );
  gui.show();

  gui["hbright"].registerCallback(set_brightness);
  gui["hcont"].registerCallback(set_contrast);
  gui["hsat"].registerCallback(set_saturation);
  gui["hhue"].registerCallback(set_hue);
  gui["hsize"].registerCallback(set_size);
  gui["camprops"].registerCallback(printAllProperties);
}

int main(int n, char **args){
  return ICLApp(n,args,"-input|-i(device)",init,run).exec();
}

