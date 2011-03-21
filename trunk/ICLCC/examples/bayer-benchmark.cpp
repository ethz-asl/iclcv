/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCC/examples/bayer-benchmark.cpp                     **
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

#include <ICLCC/CCFunctions.h>
#include <ICLCore/Img.h>
#include <ICLIO/FileGrabber.h>
#include <ICLIO/FileWriter.h>
#include <ICLCC/BayerConverter.h>
#include <ICLUtils/Timer.h>

using namespace icl;
using namespace std;

int main() {
  Timer t1,t2,t3,t4,t5;
    
  // Load bayer Image
  try{
    FileGrabber grabber("demoImages/bayer.pgm");
    const ImgBase *bayerImg = grabber.grab();
    
    ImgBase *rgbImg = imgNew(depth8u, bayerImg->getSize(), formatRGB);
    
    // Convert bayer to RGB
    BayerConverter bayerCC(BayerConverter::nearestNeighbor, 
                           BayerConverter::bayerPattern_BGGR, 
                           bayerImg->getSize());
    cout << "100 run's for each interpolation method:" <<  endl;
    // Nearest neighbor
    cout << "Nearest Neighbor:";
    t1.start();
    for (int i=0;i<100;i++) {
      bayerCC.apply(bayerImg->asImg<icl8u>(), &rgbImg);
    }
    t1.stop("Nearest neighbor");
    // Save RGB image
    FileWriter("demoImages/rgbOut_nn.ppm").write(rgbImg);
    
    
    // Bilinear
    cout << "Bilinear:";
    rgbImg->clear();
    bayerCC.setConverterMethod(BayerConverter::bilinear);
    t2.start();
    for (int i=0;i<100;i++) {
      bayerCC.apply(bayerImg->asImg<icl8u>(), &rgbImg);
    }
    t2.stop("Bilinear");
    // Save RGB image
    FileWriter("demoImages/rgbOut_bilinear.ppm").write(rgbImg);
    
    // HQLinear
    cout << "HQLinear:";
    rgbImg->clear();
    bayerCC.setConverterMethod(BayerConverter::hqLinear);
    t3.start();
    for (int i=0;i<100;i++) {
      bayerCC.apply(bayerImg->asImg<icl8u>(), &rgbImg);
    }
    t3.stop("HQLinear");
    // Save RGB image
    FileWriter("demoImages/rgbOut_hqLinear.ppm").write(rgbImg);
    
    // Edge Sense
    cout << "Edge Sense:";
    rgbImg->clear();
    bayerCC.setConverterMethod(BayerConverter::edgeSense);
    t4.start();
    for (int i=0;i<100;i++) {
      bayerCC.apply(bayerImg->asImg<icl8u>(), &rgbImg);
    }
    t4.stop("EdgeSende");
    // Save RGB image
    FileWriter("demoImages/rgbOut_edgeSense.ppm").write(rgbImg);
    
    // Simple
    cout << "Simple:";
    rgbImg->clear();
    bayerCC.setConverterMethod(BayerConverter::simple);
    t5.start();
    for (int i=0;i<100;i++) {
      bayerCC.apply(bayerImg->asImg<icl8u>(), &rgbImg);
    }
    t5.stop("Simple");
    // Save RGB image
    FileWriter("demoImages/rgbOut_simple.ppm").write(rgbImg);  
  }catch(FileNotFoundException &ex){
    ERROR_LOG("this is just a demo benchmark, it needs an 8u- image with\n"
              "bayer pattern BGGR which can be found at ./demoImages/bayer.pgm\n"
              "to be started. Run this benchmark from:\n"
              "$ICL_SOURCE_ROOT/ICLCC/examples");
  }
  return 1;
}
