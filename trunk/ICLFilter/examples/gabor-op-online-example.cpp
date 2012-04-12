/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/examples/gabor-op-online-example.cpp         **
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

#include <ICLQuick/Common.h>
#include <ICLUtils/FPSEstimator.h>
#include <ICLFilter/GaborOp.h>
GUI gui;
GenericGrabber grabber;

inline bool is_equal(const float *a, const float *b, unsigned int n){
  for(unsigned int i=0;i<n;i++){
    if(a[i] != b[i]) return false;
  }
  return true;
}
inline vector<float> vec1(float f) { 
  return vector<float>(1,f);
}

void init(){
  gui = GUI("hbox");
  GUI params("vbox[@label=Gabor Parameters@minsize=15x20]");
  params  << "fslider(0.1,100,20)[@label=Wave-Length -Lambda-@minsize=15x2@out=lambda]"
          << "fslider(0,3.15,0)[@label=Wave-Angle -Theta-@minsize=15x2@out=theta]"
          << "fslider(0,50,0)[@label=Phase-Offset -Psi-@minsize=15x2@out=psi]"
          << "fslider(0.01,10,0.5)[@label=Elipticity -Gamma-@minsize=15x2@out=gamma]"
          << "fslider(0.1,18,20)[@label=Gaussian Std-Dev. -Sigma-@minsize=15x2@out=sigma]"
          << "slider(3,50,10)[@label=Width@minsize=15x2@out=width]"
          << "slider(3,50,10)[@label=Height@minsize=15x2@out=height]";
  
  GUI maskNfps("hbox");
  maskNfps << "image[@minsize=15x15@label=Gabor Mask@handle=mask]"
           << "fps(10)[handle=fps]"; 
  
  GUI sidebar("vbox");
  sidebar << maskNfps << params;
  
  gui << "image[@minsize=32x24@label=Result Image@handle=image]" << sidebar;
  
  gui.show();

  grabber.init(pa("-i"));
  grabber.useDesired(parse<Size>(pa("-size")));
  grabber.useDesired(parse<format>(*pa("-format")));
  grabber.useDesired(parse<depth>(*pa("-depth")));

}

void run(){
  float &lambda = gui.getValue<float>("lambda");
  float &theta = gui.getValue<float>("theta");
  float &psi = gui.getValue<float>("psi");
  float &gamma = gui.getValue<float>("gamma");
  float &sigma = gui.getValue<float>("sigma");
  int &width = gui.getValue<int>("width");
  int &height = gui.getValue<int>("height");
  
  float saveParams[] = {0,0,0,0,0};
  Size saveSize = Size::null;
  
  ImgBase *resultImage = 0;
  
  SmartPtr<GaborOp> g;
  
  while(1){
    float params[] = {lambda,theta,psi,gamma,sigma};
    Size size = Size(width,height);
    
    
    if(!is_equal(params,saveParams,5) || size != saveSize || !g){
      g = new GaborOp(size,vec1(lambda),vec1(theta),vec1(psi),vec1(sigma),vec1(gamma));
      Img32f m = g->getKernels()[0].detached();
      m.normalizeAllChannels(Range<float>(0,255));
      gui["mask"] = m;
    }
    saveSize = size;
    memcpy(saveParams,params,5*sizeof(float));
    
    g->apply(grabber.grab(),&resultImage);
    resultImage->normalizeAllChannels(Range<icl64f>(0,255));
    
    gui["image"] = resultImage;
    gui["fps"].render();
  }
}



int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) -format(format=rgb) -depth(depth=depth32f) -size(size=VGA)",init,run).exec();
}
