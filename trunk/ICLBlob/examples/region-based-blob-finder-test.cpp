/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLBlob/examples/region-based-blob-finder-test.cpp     **
** Module : ICLBlob                                                **
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

#include <ICLBlob/RegionBasedBlobSearcher.h>
#include <ICLQuick/Quick.h>
#include <ICLBlob/RegionDetector.h>

vector<icl8u> vec3(icl8u r, icl8u g, icl8u b){
  vector<icl8u> v(3);
  v[0] = r;
  v[1] = g;
  v[2] = b;
  return v;
} 


int main(){
  ImgQ A = scale(create("parrot"),0.2);

  A = gray(A);
  A = levels(A,5);
  A = filter(A,"median");

  RegionBasedBlobSearcher R;


  FMCreator *fmc[] = {
    FMCreator::getDefaultFMCreator(A.getSize(),formatRGB,vec3(10,200,10), vec3(180,180,180)),
    FMCreator::getDefaultFMCreator(A.getSize(),formatRGB,vec3(200,10,10), vec3(180,180,180)),
    FMCreator::getDefaultFMCreator(A.getSize(),formatRGB,vec3(10,10,200), vec3(180,180,180))
  };
  
  RegionFilter *rf[] = { new RegionFilter( new Range<icl8u>(200,255),       // value range
                                           new Range<icl32s>(10,1000000) ),   // size range
                         new RegionFilter( new Range<icl8u>(200,255),       // value range
                                           new Range<icl32s>(10,1000000) ),   // size range
                         new RegionFilter( new Range<icl8u>(200,255),       // value range
                                           new Range<icl32s>(10,1000000) ) };   // size range


  for(int i=0;i<3;++i){
    R.add(fmc[i],rf[i]);
  }

  
  R.extractRegions(&A);
 
  A =rgb(A);
  

  color(255,0,0,200);
  pix(A,R.getCOGs());
    
  
  color(0,255,0,100);
  const std::vector<std::vector<Point> > &boundaries = R.getBoundaries();
  for(unsigned int i=0;i<boundaries.size();i++){
    pix(A,boundaries[i]);
  }
  
  show(A);
}
