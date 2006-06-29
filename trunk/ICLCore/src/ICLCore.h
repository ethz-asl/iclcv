#ifndef ICLCORE_H
#define ICLCORE_H

// comment in to ensure IPP optimizations
// the flag may defined as compiler option also
//#define WITH_IPP_OPTIMIZATION


/** 
\mainpage Image-Component-Library (ICLCore) 
\section Overview
The ICL is a C++ Image-Library, designed for Computer-Vision Tasks.
It consists of two main classes:
- <b>ICLBase</b>: The <b>base</b> image- class providing basic 
  information about the image structure:
  - size (in pixels)
  - channel count  (see <b>Channel-Concept</b>)
  - type of pixels (see <b>Data-Types</b>)
  - (color)-format (see <b>(Color)-Formats</b>)
  - raw image data

  It has no public constructors so it has to be used as interface
  class for the derived template classes ICL<Type>
- <b>ICL</b>: The <i>working</i> image class implemented as a template,
  where the datatype of each pixel value is the template parameter.
  This class provides some additional image information:
  - type-save image data
  - Region of Interest (see <b>Region of Interests</b> (ROI))
  
@see ICLBase, ICL


\section Channel-Concept
The ICL treats images as a stack of image slices -- <b>channels</b>.
An ICLChannel can be a part of several images at the same time, 
which enables the programmer to compose existing ICLChannels 
(located as member of some existing ICL) to another new image.
The <i>composed</i> new ICL-Image will share the 
image data with the original ICL, so modifications on it 
will effect as well the original images, as the composed one.
If it is necessary the image is independent from other images, you
may call the detach-method, which replaces the image channels with
new independent ones.

@see ICL, ICLChannel

\section Data-Types
This time the ICL is optimized for two different data types:
- <b>iclbyte</b> 8bit unsigned integer
- <b>iclfloat</b> 32bit float

ICL-classes are predefined for these to types:
- ICL<iclfloat> : public ICLBase
- ICL<iclbyte> : public ICLBase

Each of these data types has several advantages/disadvantages. The greatest
disadvantage of the iclbyte, is its bounded range to {0,1,...,255},
which has the effect, that all information has to be scaled to these
range, and all image processing functions must take care that
no range-overflow occurs during calculation. Furthermore
the limited range may cause loss of information - particular in 
complex systems.
The advantage of integer values is, that computation is faster
than using float values, not at least because of the 4-Times 
larger memory usage.
 
@see icldepth, iclbyte, iclfloat

\section Color-Formats
An ICLBase image provides some information about the (Color)-format, that
is associated with the image data represented by the images channels. Color
is written in brackets, as not all available formats imply color-information.
The most known Color-Space is probably the RGB-color-space. 
If the an ICLBase image has the format formatRGB, than this implies the following:
- the image has exactly 3 channels
- the first channel contains RED-Data in range [0,255]
- the second channel contains GREEN-Data in range [0,255]
- the third channel contains BLUE-Data in range [0,255]

All additional implemented ICL-Packages may use this information. The currently 
available icl-formats are member of the struct iclformat.
A special format: formatMatrix may be used for arbitrary purpose.

@see iclformat

\section IPP-Optimization
The IPP Intel Performance Primitives is a c-library that contains highly optimized
and hardware accelerated functions for image processing, and other numerical problems.
To provide access to IPP/IPPI functionalities, the ICLCore library can be 
compiled with <b>WITH_IPP_OPTIMIZATIONS</b> defined. In this case, the following
adaption are performed:
- the icl data types iclfloat and iclbyte are defined as the ipp compatible
  type Ipp32f and Ipp8u.
- some of the builtin ICL functions, like scaling or converting to another type
  are accelerated using equivalent ipp-function calls.
- some additional <i>ipp-compability functions</i> are included into the class interface
  of ICLChannel and ICL<Type>.

@see ICL, ICLChannel
*/

#include "ICLMacros.h"
#ifdef WITH_IPP_OPTIMIZATION
#include <ipp.h>
#endif

/// The ICL-namespace
/**
This namespace is dedicated for ICLCore- and
all additional Computer-Vision packages, that 
are based on the ICLCore classes.
**/
namespace ICL {

  //forward declaration
  class ICLBase;
  
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
  enum icldepth{
    depth8u, /**< 8Bit unsigned integer values range {0,1,...255} */
    depth32f /**< 32Bit floating point values */
  };
  
  /// determines the color-format, that is associated with the images channels 
  enum iclformat{
    formatRGB, /**< (red,green,blue)-colors pace */
    formatHLS, /**< (hue,lightness,saturation)-colors pace (also know as HSI) */
    formatLAB, /**< (lightness,a*,b*)-colors pace */
    formatYUV, /**< (Y,u,v)-colors pace */
    formatGray, /**< n-channel gray image range of values is [0,255] as default */
    formatMatrix /**< n-channel image without a specified colors pace. */
  };

#ifdef WITH_IPP_OPTIMIZATION
  enum iclscalemode{
    interpolateNN=IPPI_INTER_NN,      /**< nearest neighbor interpolation */
    interpolateLIN=IPPI_INTER_LINEAR, /**< bilinear interpolation */
    interpolateRA=IPPI_INTER_SUPER    /**< region-average interpolation */
  };
#else
  /// for scaling of ICL images theses functions are provided
  enum iclscalemode{
    interpolateNN,  /**< nearest neighbor interpolation */
    interpolateLIN, /**< bilinear interpolation */
    interpolateRA   /**< region-average interpolation */
  };

#endif
  /* {{{ Global functions */

  /// ensures that an image has the specified depth
  /** This function will delete the original image pointed by (*ppoImage)
      and create a new one with identical parameters, if the given depth
      parameter is not the images depth.
      @param ppoImage pointer to the image-pointer
      @param eDepth destination depth of the image
  **/
  void iclEnsureDepth(ICLBase **ppoImage, icldepth eDepth);

  /// ensures that two images have the same size, channel count, depth, and format
  /** @param ppoDst points the destination ICLBase*. If the images depth has to be
                    converted, then a new ICLCore* is created, at (*ppoDst).
      @param poSrc source image. All params of this image are extracted to define
                   the destination parameters for *ppoDst.  
  **/
  void iclEnsureCompatible(ICLBase **ppoDst, ICLBase *poSrc);
  
  /// determines the count of channels, for each color format
  /** @param eFormat source format which channel count should be returned
      @return channel count of format eFormat
  **/
  int iclGetChannelsOfFormat(iclformat eFormat);

  
  
  /// call iclGetDepth<T> inside of an ICL function to get associated Depth as int
  /**
  @return depth associated with the Type value
  **/
  template<class T> 
  static icldepth iclGetDepth(){
    return depth8u;
  }

  /// specialized function for depth8u
  template<> 
  static icldepth iclGetDepth<iclbyte>(){
    return depth8u;
  }
  
  /// specialized function for depth32f
  template<> 
  static icldepth iclGetDepth<iclfloat>(){
    return depth32f;
  }
  
}

#endif
