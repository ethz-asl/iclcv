/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/examples/convolution-op-demo.cpp             **
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

#include <ICLQt/Common.h>
#include <ICLFilter/ConvolutionOp.h>
#include <sstream>

HSplit gui;
GenericGrabber *grabber = 0;

void init(){
  std::string filters = "custom,sobelX3x3,sobelY3x3,sobelX5x5,sobelY5x5,!gauss3x3,gauss5x5,laplace3x3,laplace5x5";
  gui << ( VBox().label("controls")
           << Combo("QQVGA,QVGA,HVGA,!VGA,SVGA,HD720,WXGA,SXGAP,HD1080").label("source size").handle("src-size")
           << Combo("depth8u,depth16s,depth32s,depth32f,depth64f").label("source depth").handle("src-depth")
           << Combo("full,ul,ur,ll,rl,center").label("source roi").handle("src-roi")
           << Combo(filters).label("kernel-type").handle("kernel-type")
           << Button("off","on").label("force unsigned").out("force-unsigned")
           << Button("off","on").label("clip to ROI").out("clip-to-roi")
           << Fps(10).handle("fps").label("FPS")
           << Label("--").handle("apply-time").label("apply time")
           )
      << ( VSplit()
           << Draw().minSize(16,12).handle("src").label("source image")
           << Draw().minSize(16,12).handle("dst").label("result image")
           )
      << Show();
}

Rect get_roi(const std::string &info,const Rect &full){
  if(info == "full"){
    return full;
  }
  Size size = full.getSize()/2;

  if(info == "center"){
    return Rect(Point(full.width/4,full.height/4),size);
  }

  Point offs;
  if(info[0] == 'l') offs.y = full.height/2;
  if(info[1] == 'r') offs.x = full.width/2;
  return Rect(offs,size);
}

ConvolutionKernel::fixedType get_kernel(const std::string &name){
#define IF(X) if(name == #X) return ConvolutionKernel::X
  IF(sobelX3x3);IF(sobelY3x3);IF(sobelX5x5);IF(sobelY5x5);IF(gauss3x3);IF(gauss5x5);IF(laplace3x3);IF(laplace5x5);
#undef IF
  return ConvolutionKernel::custom;
}

void run(){
  static DrawHandle src = gui.get<DrawHandle>("src");
  static DrawHandle dst = gui.get<DrawHandle>("dst");

  static ComboHandle srcSize = gui.get<ComboHandle>("src-size");
  static ComboHandle srcDepth = gui.get<ComboHandle>("src-depth");
  static ComboHandle srcROI = gui.get<ComboHandle>("src-roi");
  static ComboHandle kernel = gui.get<ComboHandle>("kernel-type");
  
  static bool &forceUnsigned = gui.get<bool>("force-unsigned");
  static bool &clipToROI = gui.get<bool>("clip-to-roi");

  static FPSHandle fps = gui.get<FPSHandle>("fps");
  static LabelHandle applyTime = gui.get<LabelHandle>("apply-time");
  
  static GenericGrabber grabber(pa("-i"));

  static ImgBase *dstImage = 0;
  
  grabber.useDesired(parse<Size>(srcSize.getSelectedItem()));
  grabber.useDesired(parse<depth>(srcDepth.getSelectedItem()));
  
  const ImgBase *grabbedImage = grabber.grab();
  Rect roi = get_roi(srcROI.getSelectedItem(),grabbedImage->getImageRect());
  const ImgBase *roiedImage = grabbedImage->shallowCopy(roi);
  
  ConvolutionOp conv(ConvolutionKernel(get_kernel(kernel.getSelectedItem())),forceUnsigned);
  conv.setClipToROI(clipToROI);
  
  Time t = Time::now();
  conv.apply(roiedImage,&dstImage);
  applyTime = (Time::now()-t).toStringFormated("%Ss %#ms %-us");
  ostringstream sstr; sstr << *dstImage;
  
  src = roiedImage;
  src->color(255,0,0,255);
  src->fill(0,0,0,0);
  src->rect(roi.enlarged(-1));
  
  dst = dstImage;
  dst->color(255,0,0,255);
  dst->text(sstr.str(),10,10,-1,-1,7);
  
  src.render();
  dst.render();
  fps.render();
  
  delete roiedImage;
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params)",init,run).exec();
}

/// there is an error when getting info from 16 bit image ???
/// add text field with output image string
