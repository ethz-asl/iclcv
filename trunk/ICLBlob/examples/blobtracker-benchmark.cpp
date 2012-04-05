/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLBlob/examples/blobtracker-benchmark.cpp             **
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

#include <ICLUtils/Timer.h>
#include <ICLCore/Img.h>
#include <ICLCore/Mathematics.h>
#include <ICLBlob/Extrapolator.h>
#include <ICLBlob/PositionTracker.h>
#include <ICLBlob/HungarianAlgorithm.h>

#include <stdio.h>
#include <string>
#include <vector>

using namespace std;
using namespace icl;

typedef std::vector<int> vec;

int main(int n, char  **ppc){
  PositionTracker<int> pt;
  randomSeed();
  
  printf("Benchmarking and Testing the BlobTracker class: \n");
  int ns[] = {2,3,4,5,10,20,50,100,200};
  
  for(int iN = 2;iN<9;iN++){
    int N = ns[iN];
    printf("testing algorithm with N=%d positions \n",N);
    const int K = 100;
    const int D = 10;
    int  *pi = new int[N*K];
    for(int i=0;i<N*K;i++){
      pi[i] = random(static_cast<unsigned int>(10));
    }
    pt.pushData(vec(pi,pi+N),vec(pi,pi+N));    
    Timer t(1); t.start();
    for(int i=0;i<D;i++){
      int *ppp = pi+N*i;
      pt.pushData(vec(ppp,ppp+N),vec(ppp,ppp+N));    
    }
    printf("average time of 10 = %fms \n",float(t.stopSilent())/(10*1000.0));
    delete pi;
  }
  
  return 0;
}
