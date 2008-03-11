#ifndef ICL_REGION_PCA_INFO_H
#define ICL_REGION_PCA_INFO_H

#include <cmath>

namespace icl{
  /// data-struct to represent local PCA information \ingroup G_RD
  class RegionPCAInfo{
    public:
    
    /// Default Constructor
    RegionPCAInfo(float len1=0, float len2=0, float arc1=0, int cx=0, int cy=0):
    len1(len1),len2(len2),arc1(arc1),arc2(arc1+M_PI/2),cx(cx),cy(cy){}
    
    /// length of first major axis
    float len1; 
    /// length of second major axis
    float len2; 
    /// angle of the first major axis
    float arc1; 
    /// angle of the second major axis (arc1+PI/2)
    float arc2; 
    /// x center of the region
    int cx;
    /// y center of the region
    int cy;
    
    /// null PCAInfo
    static const RegionPCAInfo null;
  };
}

#endif
