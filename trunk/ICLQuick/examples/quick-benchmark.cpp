/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQuick module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLQuick/Common.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLQt/ConfigEntry.h>

GUI gui("hbox");
GenericGrabber *grabber = 0;

void init(){
  ConfigFile cfg;
  cfg.set("config.threshold",int(3));
  cfg.setRestriction("config.threshold",ConfigFile::KeyRestriction(0,255));
  ConfigFile::loadConfig(cfg);


  gui << "image[@handle=image@minsize=32x24@label=image]";
  
  GUI con("vbox");
  con << "config(embedded)[@label=configuration@minsize=15x15]";
  con << "fps(50)[@handle=fps]";

  gui << con;
  gui.show();
  
  

  grabber = new GenericGrabber(FROM_PROGARG("-input"));
  grabber->setDesiredSize(Size::VGA);
}



void run(){
  ImgQ image = cvt(grabber->grab());

  ImgQ a = gray(image);
  
  static ConfigEntry<int> t("config.threshold");

  ImgQ b = thresh(a,t);

  ImgQ c = thresh(a,2*t);
  
  ImgQ d = binXOR<icl8u>(b,c);
  
  ImgQ e = (b,c,d);
  
  ImgQ f = thresh(b,128);
  
  ImgQ g = (255.0/c+d-b+4*0.3);
  
  ImgQ h = (e%g);

  h.setROI(Rect(300,300,332,221));
  
  ImgQ i = copyroi(h);
  
  static ImageHandle han = gui.getValue<ImageHandle>("image");
  han = i;
  han.update();

  static FPSHandle fps = gui.getValue<FPSHandle>("fps");
  fps.update();
  
  
}


int main(int n,char **ppc){
  return ICLApp(n,ppc,"-input|-i(device,device-params)",init,run).exec();
}
