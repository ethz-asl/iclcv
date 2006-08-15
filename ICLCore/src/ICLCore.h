#ifndef ICLCORE_H
#define ICLCORE_H

/** 
\mainpage ICL (Image-Component-Library) : ICLCore 
\section Overview

The ICL is a C++ Image-Library, designed for Computer-Vision tasks. It
supports multi-channel images (class ImgI/Img) with a depth of 8bit integer or 32bit floats.
All channels within the image share a common size and region of 
interest (ROI). This allows to handle color images as 3-channel images for example.

Despite of the different image depth, most methods of an image class have
common code. Hence, the two different pixel formats are implemented by a
template class <b>Img<imagedepth></b>. Methods which are independent on the
image depth are provided by a common interface class, named <b>ImgI</b>. This
allows easy and type-clean wrapping of both template classes within frameworks
such as Neo/NST or TDI.

Hence, the Library provides two basic image classes: 
- <b>ImgI</b>: The <b>abstract interface</b> class providing common, 
  but depth-independent information about the image structure:
  - size (in pixels)
  - channel count  (see <b>channel concept</b>)
  - type of pixels (see <b>data types</b>)
  - color format (see <b>color formats</b>)
  - raw image data access
  - Region of Interest (see <b>Region of Interests</b> (ROI))

  It has no public constructors so it has to be used as interface
  class for the derived template classes Img<Type>.
  Most of the functions in the ImgI class are purely virtual which
  implies, that they are implemented in the derived classes Img<T>.

- <b>Img</b>: The <i>proper</i> image class is implemented as a template,
  where the datatype of each pixel value is the template parameter.
  Internally each Img<T> object holds a std::vector of so called smart 
  pointers (class SmartPtr in the ICLUtils package) to the channel
  data. The Img class provides some additional image information and access
  functions:
  - type-save image data access ( using the functions 
    getData(int channel) and getROIData(int channel)
  - access to single pixel values using the ()-operator
  - access to all Image/all ROI pixels using the ImgIterator 
  (see <b>ImgIterator</b> class reference)
    
In addition to the classes Img and ImgI, the ICLCore package
provides some utility functions, that facilitates working with 
these classes (see icl namespace for more details).
  

@see ImgI, Img

\section Channel-Concept
The Img treats images as a stack of image slices -- <b>channels</b>.  Channels
can be shared by multiple Img images, which is especially important for fast
shallow images copies. Actually, it is possible to freely compose existing
channels (within several "parent images") to another new image.  

Attention: The newly <i>composed</i> image shares its channel data with
the original images, such that modifications will effect all images equally.
In order to get an independent image a deepCopy as well as a detach method
are provided. The latter replaces the "shared" image channel(s) with new 
independent ones. Shared channel data are stored using the boost-like 
shared pointer class SmartPtr, which realizes a garbage collector
automatically releasing <i>unused</i> image channels.

@see Img, ImgChannel

\section Data-Types
Currently the Img provides two different data types:
- <b>iclbyte</b> 8bit unsigned char
- <b>iclfloat</b> 32bit float

Img-classes are predefined for these two types:
- Img<iclfloat> : public ImgI <b>typedef'd to Img32f</b>
- Img<iclbyte> : public ImgI <b>typedef'd to Img8u</b>

Each of these data types has several advantages/disadvantages. The greatest
disadvantage of the iclbyte, is its bounded range (0,1,...,255),
which has the effect, that all information has to be scaled to this
range, and all image processing functions must take care that
no range-overflow occurs during calculation. Furthermore
the limited range may cause loss of information - particular in 
complex systems.
The advantage of integer values is, that computation is faster
than using float values, not at least because of the 4-Times 
larger memory usage.
 
@see Depth, iclbyte, iclfloat

\section Color Formats
An ImgI image provides some information about the (color) format, that
is associated with the image data represented by the images channels. Color
is written in brackets, as not all available formats imply color-information.
The most known color space is probably the RGB color space. 
If an ImgI image has the format <i>formatRGB</i>, than this implies the 
following:
- the image has exactly 3 channels
- the first channel contains RED-Data in range [0,255]
- the second channel contains GREEN-Data in range [0,255]
- the third channel contains BLUE-Data in range [0,255]

All additional implemented Img-Packages may/should use this information. 
The currently available Img formats are member of the enum Format.
A special format: formatMatrix may be used for arbitrary purpose.

@see Format

\section IPP-Optimization
The IPP Intel Performance Primitives is a c-library that contains highly 
optimized and hardware accelerated functions for image processing, and 
other numerical problems for all processors providing the SSE command set, 
i.e. most new Intel and AMD processors.
To provide access to IPP/IPPI functionality, the ImgCore library can be 
compiled with <b>WITH_IPP_OPTIMIZATIONS</b> defined. In this case, the 
following adaptions are performed:
- the icl data types iclfloat and iclbyte are defined as the ipp compatible
  type Ipp32f and Ipp8u.
- the classes Size, Point and Rect are derived from the corresponding ipp-
  structs IppiSize, IppiPoint and IppiRect. Hence the the programme will
  be able to uses the ICL-stucts Size, Point and Rect everywhere whare 
  the ipp-structs are needed
- some of the builtin Img functions, like scaling or converting to another type
  are accelerated using equivalent ipp-function calls.

@see Img, ImgChannel

\section _DEBUG_MACROS_ How to use LOG-Macros in the Img
The ICLUtils package contains the Macros.h header file,
which provides the common debug macros for ICL classes. The debug system
knows 6 different debug levels (0-5). Level depended debug messages
can be written to std::out using the <b>DEBUG_LOG\<LEVEL\></b>-macro.
The set debug level (0 by default) regulates the verboseness of the 
ICL library. The debug levels (0-5) are characterized as follows:

<h3>Img debug levels</h3>
      <table>
         <tr>  
            <td><b>level</b></td>
            <td><b>macro</b></td>
            <td><b>description</b></td>
         </tr><tr>  
            <td><b>level 0</b></td>    
            <td>ERROR_LOG(x)</td>
            <td> <b>(default)</b> Only critical error messages 
                 will be written to std::err. If an exception 
                 that occurred may cause a program crash, then 
                 an error message should be written. 
            </td>
         </tr><tr> 
            <td><b>level 1</b></td>    
            <td>WARNING_LOG(x)</td>
            <td> Also non critical waring will be written (now to 
                 std::out. Exceptions, that are notified with the
                 WARNING_LOG template may not cause a program crash.
            </td>
         </tr><tr>  
            <td><b>level 2</b></td>    
            <td>FUNCTION_LOG(x)</td>
            <td> The next debug level will notify each function call
                 by printing an according message to std::out. Take
                 care to use the FUNCTION_LOG template in each function
                 you implement for the Img
            </td>
         </tr><tr>  
            <td><b>level 3</b></td>    
            <td>SECTION_LOG(x)</td>
            <td> Sections of functions (like initialization-section,
                 working-section or end-section) should be commented 
                 using the SECTION_LOG macro
            </td>
         </tr><tr>  
            <td><b>level 4</b></td>    
            <td>SUBSECTION_LOG(x)</td>
            <td> Subsections of functions may be commented 
                 using the SUBSECTION_LOG macro
            </td>
         </tr><tr>  
            <td><b>level 5</b></td>    
            <td>LOOP_LOG(x)</td>
            <td> Debug messages that occur in long loops, e.g. while
                 iteration over all image pixels, may be written using the
                 LOOP_LOG macro. Note, that these messages can slow down 
                 the iteration time to less then 0.1%.
            </td>
         </tr>
      </table>

The following example will show how to use the DEBUG-Macros 
provided in ImgMacros.h
  <pre>
  int sum_vec(int *piVec, int len){
     FUNCTION_LOG("int *, int");
     ImgASSERT(piVec); // calls ERROR_LOG

     SECTION_LOG("temp. variale allocation");
     int sum = 0;

     SECTION_LOG("starting loop");
     for(int i=0;i<len;i++){
        LOOP_LOG("addition loop, index: " << i << "curr. value: " << sum);
        sum += piVec[i];
        // WARNING_LOG e.g. for range overflow for the int accumulator "sum"
      }
     SECTION_LOG("return sum");
     return sum;
  }
  </pre>
@see ImgMacros.h
*/

#include "Macros.h"
#include "Size.h"
#include "Point.h"
#include "Rect.h"


#include <string>
#include <vector>
#ifdef WITH_IPP_OPTIMIZATION
#include <ipp.h>
#endif

/// The ICL-namespace
/**
This namespace is dedicated for ICLCore- and all additional Computer-Vision
packages, that are based on the ICLCore classes.
**/
namespace icl {

  //forward declaration
  class ImgI;
  
#ifdef WITH_IPP_OPTIMIZATION
  /// 32Bit floating point type for the ICL 
  typedef Ipp32f iclfloat;

  /// 8Bit unsigned integer type for the ICL
  typedef Ipp8u iclbyte;

#else
  /// 32Bit floating point type for the ICL 
  typedef float iclfloat;

  /// 8Bit unsigned integer type for the ICL 
  typedef unsigned char iclbyte;
#endif

  /// determines the pixel type of an image (8Bit-int or 32Bit-float) 
  enum Depth{
    depth8u  = 8, /**< 8Bit unsigned integer values range {0,1,...255} */
    depth32f = 32 /**< 32Bit floating point values */
  };
  
  /// determines the color-format, that is associated with the images channels 
  enum Format{
    formatRGB, /**< (red,green,blue) colors pace */
    formatHLS, /**< (hue,lightness,saturation) color space (also know as HSI) */
    formatLAB, /**< (lightness,a*,b*) color space */
    formatYUV, /**< (Y,u,v) color space */
    formatGray, /**< n-channel gray image range of values is [0,255] as default */
    formatMatrix /**< n-channel image without a specified color space. */
  };

#ifdef WITH_IPP_OPTIMIZATION
  enum ScaleMode{
    interpolateNN=IPPI_INTER_NN,      /**< nearest neighbor interpolation */
    interpolateLIN=IPPI_INTER_LINEAR, /**< bilinear interpolation */
    interpolateRA=IPPI_INTER_SUPER    /**< region-average interpolation */
  };
#else
  /// for scaling of Img images theses functions are provided
  enum ScaleMode{
    interpolateNN,  /**< nearest neighbor interpolation */
    interpolateLIN, /**< bilinear interpolation */
    interpolateRA   /**< region-average interpolation */
  };
#endif


/* {{{ Global functions */

  /// creates a new ImgI by abstacting about the depth parameter
  /** This function is essention for the abstaction mechanism about 
      Img images underlying depth. In many cases you might have an
      ImgI*, wich must be initialized with parameters width, height,
      channel count and - which is the problem - the depth. The
      default solution is to insert an if statement. Look at the 
      following Example, that shows the implementation of a class
      construktor.
      <pre>
      class Foo{
         public:
         Foo(...,Depth eDepth,...):
             poImage(eDepth==depth8u ? new Img8u(...) : new Img32f(...)){
         }
         private:
         ImgI *poImage;         
      };
      </pre>
      This will work, but the "?:"-statement make the code hardly readable.
      The following code extract will show the advantage of using the iclNew
      function:
      <pre>
      class Foo{
         public:
         Foo(...,Depth eDepth,...):
             poImage(iclNew(eDepth,...)){
         }
         private:
         ImgI *poImage;         
      };
      The readability of the code is much better.
      </pre>
  
      @param eDepth depth of the image that should be created
      @param s size of the new image
      @param eFormat format of the new image
      @param iChannels channel count of the new image. If < 0,
                       then the channel count associated with 
                       eFormat is used
      @return the new ImgI* with underlying Img<Type>, where
              Type is depending on the first parameter eDepth
  **/
  ImgI *imgNew(Depth eDepth=depth8u, 
               const Size& s=Size(1,1),
               Format eFormat=formatMatrix,
               int iChannels = -1,
               const Rect &oROI=Rect());


  
  /// ensures that an image has the specified depth
  /** This function will delete the original image pointed by (*ppoImage)
      and create a new one with identical parameters, if the given depth
      parameter is not the images depth. If the fiven image pointer
      (*ppoImage) is NULL, then a new image is created with size 1x1,
      one channel and specified depth.
      @param ppoImage pointer to the image-pointer
      @param eDepth destination depth of the image
  **/
  void ensureDepth(ImgI **ppoImage, Depth eDepth);

  /// ensures that two images have the same size, channel count, depth, format and ROI
  /** If the given dst image image is 0 than it is created as a clone  (deep copy) of
      of poSrc.
      @param ppoDst points the destination ImgI*. If the images depth has to be
                    converted, then a new ImgCore* is created, at (*ppoDst).
      @param poSrc source image. All params of this image are extracted to define
                   the destination parameters for *ppoDst.  
  **/
  void ensureCompatible(ImgI **ppoDst, ImgI *poSrc);

  /// ensures that two images have the same size, channel count, depth, format and ROI
  /** If the given dst image image is 0 than it is created as a clone  (deep copy) of
      of poSrc.
      @param ppoDst points the destination ImgI*. If the images depth has to be
                    converted, then a new ImgCore* is created, at (*ppoDst).
      @param eDepth destination depth
      @param iWidth destination width
      @param iHeight destination height
      @param eFormat destination format
      @param iChannelCount destination channel count. (If -1, then the channel count
                           is extracted from the given eFormat
      @param poROIoffset destination ROI offset
      @param poROIsize   destination ROI size
      If the ROI parameters are not given, the ROI will comprise the whole image.
  **/
  void ensureCompatible(ImgI **ppoDst,
                        Depth eDepth, 
                        const Size& s,
                        Format eFormat, 
                        int iChannelCount=-1,
                        const Rect &r=Rect());
  
  /// determines the count of channels, for each color format
  /** @param eFormat source format which channel count should be returned
      @return channel count of format eFormat
  **/
  int getChannelsOfFormat(Format eFormat);


  /// returns a string representation of an Format enum
  /** @param eFormat Format enum which string repr. is asked 
      @return string representation of eFormat
  **/
  string translateFormat(Format eFormat);  
  
  /// returns an Format enum, specified by a string 
  /** This functions implements the opposite direction to the above function,
      which means, that:
      <pre>
      iclTranslateFormat(iclTranslateFormat(x)) == x
      </pre>
      If x is a string or an Format enum.
      @param sFormat string representation of the format 
                     which should be returned
      @return Format, that corresponds to sFormat
  **/
  Format translateFormat(string sFormat);

  /// call iclGetDepth<T> inside of an Img function to get associated Depth as int
  /**
  @return depth associated with the Type value
  **/
  template<class T> 
  static Depth getDepth(){
    return depth8u;
  }

  /// specialized function for depth8u
  template<> 
  static Depth getDepth<iclbyte>(){
    return depth8u;
  }
  
  /// specialized function for depth32f
  template<> 
  static Depth getDepth<iclfloat>(){
    return depth32f;
  }

  /// determine the sizeof value of an Img deph
  /**
     @return sizeof value associated with the Type value
  **/
  int getSizeOf(Depth eDepth);

}

/* }}} */

#endif
