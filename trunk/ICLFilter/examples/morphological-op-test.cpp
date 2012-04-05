/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/examples/morphological-op-test.cpp           **
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

#include <ICLQuick/Quick.h>
#include <ICLFilter/MorphologicalOp.h>
#include <ICLUtils/StringUtils.h>

int main(){
#if 0
  Img8u image = cvt8u(thresh(gray(scale(create("parrot"),280,400)),128));
  Img8u imageG = cvt8u(gray(scale(create("parrot"),280,400)));
  Img8u imageC = cvt8u(scale(create("parrot"),280,400));
  ImgQ image = thresh(gray(scale(create("parrot"),280,400)),128);
  ImgQ imageG = gray(scale(create("parrot"),280,400));
  ImgQ imageC = scale(create("parrot"),280,400);
#endif

  ImgQ imageC_32f(Size(400,150),formatRGB);
  color(255,255,255,255);
  fontsize(45);
  text(imageC_32f,20,20,"ICL rocks!");
  ImgQ imageG_32f = gray(imageC_32f);
  ImgQ image_32f = thresh(imageG_32f,128);
  
  Img8u image = cvt8u(image_32f);
  Img8u imageG = cvt8u(imageG_32f);
  Img8u imageC = cvt8u(imageC_32f);
  
  MorphologicalOp::optype ts[11]={ 
    MorphologicalOp::dilate,
    MorphologicalOp::erode,
    MorphologicalOp::dilate3x3,
    MorphologicalOp::erode3x3,
    MorphologicalOp::dilateBorderReplicate,
    MorphologicalOp::erodeBorderReplicate,
    MorphologicalOp::openBorder,
    MorphologicalOp::closeBorder,
    MorphologicalOp::tophatBorder,
    MorphologicalOp::blackhatBorder,
    MorphologicalOp::gradientBorder
  };
  std::string ns[11]={ 
    "dilate",
    "erode",
    "dilate3x3",
    "erode3x3",
    "dilateBorderReplicate",
    "erodeBorderReplicate",
    "openBorder",
    "closeBorder",
    "tophatBorder",
    "blackhatBorder",
    "gradientBorder"
  };

  
  static Size dilationKernelSize(3,3);
  static std::vector<icl8u> kernel(dilationKernelSize.getDim(),1);

  ImgQ X = zeros(1,1,1);
  
  ImgQ result1 = label(cvt(image),"orig");
  ImgQ result2 = X;

  ImgQ result1G = label(cvt(imageG),"orig");
  ImgQ result2G = X;

  ImgQ result1C = label(cvt(imageC),"orig");
  ImgQ result2C = X;


  for(int i=0;i<11;++i){
    printf("applying %s \n",ns[i].c_str());
    MorphologicalOp mo(ts[i],dilationKernelSize,kernel.data());
    ImgBase *dst = 0;
    ImgBase *dstG = 0;
    ImgBase *dstC = 0;
    
    mo.apply(&image,&dst);
    mo.apply(&imageG,&dstG);
    mo.apply(&imageC,&dstC);
    
    result1 = (result1 %X% label(cvt(dst),ns[i]+str(dst->getROISize())));
    result1G = (result1G %X% label(cvt(dstG),ns[i]+str(dst->getROISize())));
    result1C = (result1C %X% label(cvt(dstC),ns[i]+str(dst->getROISize())));
    
    ICL_DELETE( dst );
    ICL_DELETE( dstG );
    ICL_DELETE( dstC );
    

  }
  show( result1);
  show( result1G);
  show( result1C);
}

