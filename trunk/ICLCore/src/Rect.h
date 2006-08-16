#ifndef ICL_RECT_H
#define ICL_RECT_H

namespace icl{
#ifndef WITH_IPP_OPTIMIZATION
  /// fallback implementation for the IppiRect struct, defined in the ippi lib
  struct IppiRect {
    /// xpos
    int x;
    
    /// ypos
    int y;

    /// width
    int width;

    /// height
    int height;
  };
#else
#include <ipp.h>
#endif
  
  /// Rectangle class of the ICL used e.g. for the Images ROI-rect
  class Rect : public IppiRect{
    public:
    /// creates a (0,0,0,0) Rect
    Rect(){
      this->x = 0;
      this->y = 0;
      this->width = 0;
      this->height = 0;
    }
    
    /// creates a defined Rect
    Rect(int x, int y, int width, int height){
      this->x = x;
      this->y = y;
      this->width = width;
      this->height = height;
    }
    
    /// creates a new Rect with specified offset and size
    Rect(const Point &p, const Size &s){
      this->x = p.x;
      this->y = p.y;
      this->width = s.width;
      this->height = s.height;
    } 
    
    /// create a deep copy of a rect
    Rect(const Rect &r){
      this->x = r.x;
      this->y = r.y;
      this->width = r.width;
      this->height = r.height;
    }
    /// returns (x || y || width || height) as bool
    operator bool() const{
      return x || y || width || height;
    }

    /// checks if two rects are equal
    bool operator==(const Rect &s) const {
      return x==s.x && y==s.y && width==s.width && height==s.height;
    }

    /// checks if two rects are not equal
    bool operator!=(const Rect &s) const {
      return x!=s.x || y!= s.y || width!=s.width || height!=s.height;
    }

    /// scales all parameters of the rect by a double value
    Rect operator*(double d) const {
      return Rect((int)(d*x),(int)(d*y),(int)(d*width),(int)(d*height));
    }
    
    /// adds a size to the rects size
    Rect& operator+=(const Size &s){
      width+=s.width; height+=s.height; return *this;
    }
    
    /// substracs a size to the rects size
    Rect& operator-=(const Size &s){
      width-=s.width; height-=s.height; return *this;
    }
    
    /// adds a Point to the rects offset
    Rect& operator+=(const Point &p){
      x+=p.x; y+=p.y; return *this;
    }

    /// substracts a Point to the rects offset
    Rect& operator-=(const Point &p){
      x-=p.x; y-=p.y; return *this;
    }
    
    /// scales all rect params inplace
    Rect& operator*=(double d){
      x=(int)((float)x*d); 
      y=(int)((float)y*d);
      width=(int)((float)width*d); 
      height=(int)((float)height*d); 
      return *this;
    };
    
    /// returns width*height
    int getDim() const {return width*height;}

    /// intersection of two Rects (NOT IMPLEMENTED)
    Rect operator&(const Rect &r) const {
      printf("ERROR!!! Rect::intersection operator & is not yet implemented \n");
      return Rect();
    }
    
    /// union of two Rects (NOT IMPLEMENTED)
    Rect operator|(const Rect &r) const {
      printf("ERROR!!! Rect::union operaotr | is not yet implemented \n");
      return Rect();
    }
    
    /// rects with negative sizes are normalized to Positive sizes (NOT IMPLEMENTED)
    /** e.g. the rect (5,5,-5,-5) is normalized to (0,0,5,5) */
    Rect nomalized(){
      printf("ERROR!!! Rect::normalized is not yet implemented \n");
      return Rect();
    }
    
    /// returns if a Rect containes another rect (NOT IMPLEMENTED)
    bool contains(const Rect &r){
      printf("ERROR!!! Rect::contains is not yet implemented \n");
      return Rect();
    }
  };

} // namespace icl

#endif // ICL_RECT_H
