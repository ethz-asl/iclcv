/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCC/examples/colorspace-online-demo.cpp              **
** Module : ICLCC                                                  **
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
#include <ICLCore/CCFunctions.h>
GUI gui;
GenericGrabber grabber;

void init(){

  gui << Image().handle("image")
      << Combo("Gray,RGB,HLS,YUV,LAB,Chroma,Matrix").handle("fmt").maxSize(100,3)
      << Show();
  
  grabber.init(pa("-i"));

}

void run(){
  //grabber.useDesired(parse<format>(gui["fmt"]));
  //gui["image"] = grabber.grab();
  const ImgBase *image = grabber.grab();
  static Img8u dst;
  dst.setFormat(parse<format>(gui["fmt"]));
  cc(image,&dst);
  gui["image"] = dst;
}

int main(int n, char **args){
  return ICLApplication(n,args,"[m]-i|-input(2)",init,run).exec();
}
