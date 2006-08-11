 /*
  ICL.h

  Written by: Michael G�tting and Christof Elbrechter (2006)
              University of Bielefeld
              AG Neuroinformatik
              {mgoettin,celbrech}@techfak.uni-bielefeld.de
*/

#ifndef ICL_H
#define ICL_H

#include "ICLBase.h"
#include "ICLIterator.h"
#include "ICLAutoPtr.h"
#include <cmath>
#include <algorithm>

using namespace std;

namespace icl {

  /// ICL implements an array of ICLChannel images with an arbitrary number of channels
  /**
  @author Michael Goetting (mgoettin@TechFak.Uni-Bielefeld.de) 
  @author Christof Elbrechter (celbrech@TechFak.Uni-Bielefeld.de)
  **/
template <class Type>
class ICL : public ICLBase
{
 protected:
  
  /// Internally used channel representation
  template <class T> 
  class ICLChannel
    /* {{{ open class definition*/
    {   
      public:
      friend class ICL<iclfloat>;
      friend class ICL<iclbyte>;
     
      private:
      /* {{{ members */

      /// back pointer to the parent ICLBase object
      ICLBase *m_poParent;
      
      /// Array containing image data (row-by-row)
      T *m_ptData;

      /// This flag indicates if data-buffer has to be deleted in the destructor
      bool m_bDeleteData;

      /* }}} */
      public:
      /* {{{ Konstruktor/ Destruktor: */
      //@{ @name Constructors and Destructors
      /// Base constructor creates a channel with specified width,height and optional shared data
      /** This Baseconstructor works in two different ways:
      - if ptData is NULL (default), then the constructor allocates memory for 
        an image channel of the specified width and height. The memory will
        be freed in the ICLChannels constructor        
      - else ptData (not NULL) is used as foreign data, so ptData has to 
        be of size iWidth*iHeight, and it must not be deleted as long as
        the image stays valid. In this case, internally a flag is set,
        that indicates that the memory is not owned by this channel, so
        it has not to be freed in the destuctor.
      
      @param iWidth Image channel width
      @param iHeight Image channel height
      @param ptData pointer to data allocated elsewhere
      **/
      ICLChannel(ICLBase *poParent, T *ptData=0):
        m_poParent(poParent),
        m_ptData(ptData ? ptData : new T[poParent->getDim()]),
        m_bDeleteData(ptData ? 0 : 1)
        {
          FUNCTION_LOG("");
          if(!ptData) clear();
        }
      
      /// Copy constructor 
      /** Copy constructor - generates a copy of the source channel
          @param tSource Source image channel
      **/
      ICLChannel(const ICLChannel<T>& tSrc):
        m_poParent(tSrc.m_poParent),
        m_ptData(new T[tSrc.m_poParent->getDim()]),
        m_bDeleteData(1)
        {
          FUNCTION_LOG("");
          std::copy(tSrc.m_ptData,(tSrc.m_ptData)+m_poParent->getDim(),m_ptData);
        }
      
      /// Destructor
      ~ICLChannel(){
        if(m_bDeleteData == 1)
          {
            FUNCTION_LOG("");
            delete [] m_ptData;
          }
      }

      //@}
      /* }}} */
      /* {{{ basic channel functions: */
      //@{ @name manipulation functions
      /// Set each pixel to a specific value.
      /** @param tValue destination 
      **/
      void clear(Type tValue = 0)
        { 
          FUNCTION_LOG("");
          fill(m_ptData, m_ptData+m_poParent->getDim(), tValue);
        }
      /* }}} */
    };

  /* }}} */
  
  /// internally used storage for the image channels
  vector<ICLAutoPtr<ICLChannel<Type> > > m_vecChannels;
  
  /* {{{ Auxillary function */

  /** Bilinear interpolation for 'subpixel' values.
      @param fX X coordinate of the pixel
      @param fY Y coordinate of the pixel
      @param iChannel Channel index (default 0)
      @return Value of the specified point
  **/
  Type interpolate(float fX,float fY,int iChannel=0) const;

  /* }}} */
                                
 public:
  /* {{{ constructors / destructor: */

  //@{
  /// Creates an image with specified number of channels and size.    
  /** the format of the image will be set to "iclMatrix"
      @param iWidth Width of image
      @param iHeight Height of image
      @param iChannels Number of Channels 
  **/
  ICL(int iWidth=1, int iHeight=1, int iChannels=1);
 
  /// Creates an image with specified size, number of channels and format
  /** @param iWidth width of the image
      @param iHeight height of the image
      @param eFormat (color)-format of the image
      @param iChannels channel count of the image (if -1, then the channel
                       count is calculated from the given format (E.g. if
                       eFormat is formatRGB iChannels is set to 3)
  **/
  ICL(int iWidth, int iHeight, iclformat eFormat, int iChannels = -1);
 
  /// Creates an image with specified size, number of channels, format, using shared data pointers as channel data
  /** @param iWidth width of the image
      @param iHeight height of the image
      @param eFormat (color)-format of the image
      @param iChannels channel count of the image (if -1, then the channel
                       count is calculated from the given format (E.g. if
                       eFormat is formatRGB iChannels is set to 3)
      @param pptData holds a pointer to channel data pointers. pptData must
                     have size iChannels. The data must not be deleted during
                     the "lifetime" of the ICL.
  **/
  ICL(int iWidth, int iHeight, iclformat eFormat, int iChannels, Type** pptData);

  /// Copy constructor
  /** creates a flat copy of the source image
      the new image will contain a flat copy of 
      all channels of the source image.
      @param tSrc Reference of instance to copy 
  **/
  ICL(const ICL<Type>& tSrc);
  
  
  /// Destructor
  ~ICL();
  
  //@}

  /* }}} */
                                              
  /* {{{ class operator */

  //@{ @name class operators

  /// Assign operator (flat copy of channels)
  /** Both images will share their channel data. 
      Use deepCopy() to obtain a copy of an image which is not attached to the 
      source image.      
      @param tSource Reference to source object. 
  **/
  ICL<Type>& operator=(const ICL<Type>& tSource);

  /// pixel access operator
  /** This operator has to be used, to access the pixel data of the image
      e.g. copy of image data:
      <pre>
      ICL8u oA(320,240,1),oB(320,240,1);
      for(int x=0;x<320;x++){
         for(int y=0;y<240;y++){
            oB(x,y,0)=oA(x,y,0);
         }
      }
      </pre>
      <h3>Efficiency</h3>
      Although the ()-operator is compiled inline, and optimized,
      it is very slow. A measurement with a "-O3" binary brought the result
      That pixel access is up to 10 times faster when working
      directly with a channel data pointer. Nevertheless, the ()-operator
      is provided in the ICL-class, as it offers a very intuitive access
      to the pixel data. 
  
      @param iX X-Position of the referenced pixel
      @param iY Y-Position of the referenced pixel
      @param iChannel channel index
  **/
  Type& operator()(int iX, int iY, int iChannel) const
    {
      return m_vecChannels[iChannel]->m_ptData[iX+m_iWidth*iY];
    }

  //@}

  /* }}} */
  
  /* {{{ moving / scaling image data */
  //@{ //@name moving / scaling image data

  /// perform a deep copy (given destination image is resized on demand)
  /** Returns an independent exact copy of the object. If the given destination
      image is not NULL, then deepCopy ensures, that is has a compatible
      size to this, by resizing destination image on demand. 
      <b>WARNING:</b> If the destination image has another depth, then this image,
      the deepCopy will internally call <b>convertTo8Bit</b> or <b>convertTo32Bit</b> 
      depending on the the images and the destination images icldepth.
      <b>The images ROI will not be regarded</b>, to copy just the ROI 
      into another image use deepCopyROI(ICLBase *poDst.) or 
      scaledCopyROI(ICLBase *poDst).
      @param poDst Destination image for the copied data 
                   if NULL, then a new image is created and returned
      @return Pointer to new independent ICL object
  **/
  virtual ICLBase* deepCopy(ICLBase* poDst = NULL) const;

  /// returns a scaled copy of the image data (scaling on demand) (IPP-OPTIMIZED)
  /** the function performs a deep copy of the image data
      into another image. If the given image is not NULL, its size
      is taken to calculate a scaling factor to scale the image into
      the destination. 
      If the count of channels of this image and the destination image
      do not match, then count of destination image channels will be
      adapted to this' count of image channels.
      <b>WARNING:</b> If the destination image has another depth than 
      the image, then internally a temporary buffer is created, to scale 
      and convert the image in two steps. This will hardly <b>slow down 
      performace</b>.
      <b>The images ROI will not be regarded</b>, to copy just the ROI 
      into another image use deepCopyROI(ICLBase *poDst.) or 
      scaledCopyROI(ICLBase *poDst).
      @param poDst destination image (if NULL) than it is created new with
                   identical size of this image.
      @param eScaleMode defines the interpolation mode, that is used for the scaling
                        operation.
                        possible modes are:
                           - interpolateNN  --> nearest neighbor interpolation (fastest)
                           - interpolateLIN  --> bilinear interpolation
                           - interpolateRA  --> region average 
      @see iclscalemode
      @see resize
      @see deepCopy
  **/
  virtual ICLBase* scaledCopy(ICLBase *poDst, iclscalemode eScaleMode=interpolateNN) const;
  
  /// copies the image data in the images ROI into the destination images ROI (IPP-OPTIMIZED)
  /** This function will copy the content of the images ROI into the
      destination images ROI. The ROIs must have equal dimensions - an error will
      exit the program otherwise. If the given destination image is NULL (by default),
      deepCopyROI will create a new image, with identical channel count, depth and
      format. This image has the size of the source images ROI-size, and will contain
      the source images' ROI data.

      <h3>Efficiency</h3>
      The copy calls of the ROIs are performed line by line, using <b>memcpy</b>.
      This will speedup performance for huge ROIs (width>100) by up to 50%. 

      <h3>Depth conversion</h3>
      The deep copy function supports IPP-OPTIMIZED depth conversion.
      
  **/
  virtual ICLBase *deepCopyROI(ICLBase *poDst = NULL) const;
  
  /// scales the image data in the image ROI into the destination images ROI (IPP-OPTIMIZED)
  /** This function copies ROI data from one image into the ROI of another one. If the source
      and destination ROIs have different sizes the moved data is scaled.
      If the destination image is NULL or the ROIs have identical sized, deepCopyROI is called.
  
      <h3>Efficiency</h3>
      The IPP-OPTIMIZED implementation is <b>very fast</b> - as the used ippResize-function
      uses as well source ROI as destination ROI by default. <b>Note</b> that the non
      OPTIMIZED function is just a fallback implementation to provided identical functionality
      to the optimized implementation. As scaledCopy, this fallback implementation uses
      a temporary image buffer, to perform the operation.
      @param poDst destination image (if NULL) than it is created new with
                   identical size of this images ROI size.
      @param eScaleMode defines the interpolation mode, that is used for the scaling
                        operation.
                        possible modes are:
                           - interpolateNN  --> nearest neighbor interpolation (fastest)
                           - interpolateLIN  --> bilinear interpolation
                           - interpolateRA  --> region average 
  **/
  virtual ICLBase *scaledCopyROI(ICLBase *poDst = NULL, iclscalemode eScaleMode=interpolateNN) const;
                  
  /* }}} */
                                         
  /* {{{ class organization / channel management */

  //@{ //@name organization and channel management
  
  /// Makes the image channels inside the ICL independent from other ICL.
  /** @param iIndex index of the channel, that should be detached.
      (If iIndex is an legal channel index only the corresponding channel will 
      be detached. If the legal index channel is set to -1 the whole ICL 
      becomes independent.)
  **/
  virtual void detach(int iIndex = -1);
  
  /// Removes a specified channel.
  /** @param iChannel Index of channel to remove
  **/
  virtual void removeChannel(int iChannel);
  
  /// Append channels of external ICL to the existing ICL. 
  /** Both objects will share their data (cheap copy). 
      @param poSrc source ICL<Type>
  **/
  void append(ICL<Type>* poSrc);
  
  /// Appends the channel iChannel of an external image to the current image
  /** Both objects will share their data (cheap copy). 
      @param iChannel Channel index to append
      @param poSrc ICL<Type> that contains the source channel 
  **/
  void appendChannel(int iChannel, 
                     ICL<Type> *poSrc);

  /// Swap channel A and B
  /** @param iIndexA Index of channel A;
      @param iIndexB Index of channel B
  **/
  virtual void swapChannels(int iIndexA, int iIndexB);
  
  /// Replace the channel A of this image with the channel B another image. 
  /** Both images must have the same width and height.
      @param iThisIndexA Channel to replace
      @param iOtherIndexB Channel to replace with
      @param poOtherICL Image pointer that contains the new channel
  **/
  void replaceChannel(int iThisIndexA, 
                      int iOtherIndexB, 
                      ICL<Type>* poOtherICL);

  /// sets the channel count to a new value
  /** This function works only on demand, that means, that
      channels will only be created/deleted, if the new 
      channel count differs from the current.
      @param iNewNumChannels new channel count
      @see resize
  **/
  virtual void setNumChannels(int iNewNumChannels);

  /// creates a hole new ICL internally
  /** Change the number of ICL channels and the size. The function works
      on demand: If the image has already the correct size and number of
      channels, nothing is done at all. This allows you to call renew ()
      always when you want to ensure a specific image size.
      If the size must be adapted: 
      <b> All the data within the ICL will be lost. </b> 
      @param iNewWidth New image width (if < 0, the orignal width is used)
      @param iNewHeight New image height (if < 0, the orignal height is used)
      @param iNewNumChannel New channel number (if < 0, the orignal 
             channel count is used)
  **/
  virtual void renew(int iNewWidth, int iNewHeight,int iNewNumChannel);

  /// resizes the image to new values
  /** operation is performed on demand - if image
      has already size iNewWidth,iNewHeight, then
      nothing is done at all. For resizing
      operation with scaling of the image data use scale.
      <b>Note:</b> The ROI of the image is set to the hole
      image using delROI(), notwithstanding if a resize
      operation was performed or not.
      @param iNewWidth new image width (if < 0, the orignal width is used)
      @param iNewHeight new image height (if < 0, the orignal height is used)
      @see scale
  **/
  virtual void resize(int iNewWidth, int iNewHeight);
  
  
  //@}

  /* }}} */
                                                     
  /* {{{ Type conversion iclbyte/iclfloat */
  //@{ @name type conversion
  
  /// Return a copy of the object with depth 32 bit. (IPP-OPTIMIZED)
  /** If the given destination image poDst is not NULL, than it's size is
      adapted to the images size on demand.
      @param poDst destination image (if NULL, then a new image is created) 
      @return Copy of the object with depth 32 bit 
  **/
  virtual ICL32f *convertTo32Bit(ICL32f* poDst = NULL) const ;
 
  /// Return a copy of the object with depth 8 bit (IPP-OPTIMIZED)
  /** <b>Waring: Information may be lost!</b>
      If the given destination image poDst is not NULL, than it's size is
      adapted to the images size on demand.
      @param poDst destination image (if NULL, then a new image is created) 
      @return Copy of the object with depth 8 bit 
  **/
  virtual ICL8u *convertTo8Bit(ICL8u* poDst = NULL) const;

   //@}
  /* }}} */
                                              
  /* {{{ Getter Functions */

  //@{ @name getter functions
  
  /// Returns max pixel value of channel iChannel (IPP-OPTIMIZED)
  /** @param iChannel Index of channel
  **/
  Type getMax(int iChannel) const;
  
  /// Returns min pixel value of channel iChannel (IPP-OPTIMIZED)
  /** @param iChannel Index of channel 
  **/
  Type getMin(int iChannel) const;
  
  /// Returns min and max pixel value of channel iChannel (IPP-OPTIMIZED)
  /** @param iChannel Index of channel
      @param rtMin reference to store the min value 
      @param rtMax reference to store the max value
  **/
  void getMinMax(int iChannel, Type &rtMin, Type &rtMax) const;

  /// Returns pointer to the specified channel data
  /** This method provides
      direct access to the channel data memory.
      @param iChannel Channel to get data from
  **/
  Type* getData(int iChannel) const
    { 
      return (m_vecChannels[iChannel]->m_ptData);
    }
  
  /// Returns a pointer to the end of the channel data
  /** This method provides
      direct access to the channel data memory.
      @param iChannel Channel to get data from
  **/
  Type* getDataEnd(int iChannel) const
    { 
      return getData(iChannel)+getDim();
    }

  
  /// return the raw- data pointer of an image channel
  /** This function is inherited from the base class ICLBase
      @param iChannel determines the channel which's dataptr should
                      be returned
  **/
  virtual void* getDataPtr(int iChannel) const
    {
      return getData(iChannel);
    }
  
  //@}

  /* }}} */
  
  /* {{{ basic image manipulation: */

  //@{ @name basic image manipulations

  /// perform a scaling operation of the images (keeping the data) (IPP-OPTIMIZED)
  /** Scaling the channels is only performed on demand.
      @param iNewWidth destination width for the scaling operation 
                       (if set to -1 then the original width is used)
      @param iNewHeight destination height for the scaling operation
                       (if set to -1 then the original height is used)
      @param eScaleMode defines the interpolation mode, that is used for the scaling
                          operation. (if interpolateNon, the image becomes black)
                          possible modes are:
                           - interpolateNN   --> nearest neighbor interpolation (fastest)
                           - interpolateLIN  --> bilinear interpolation
                           - interpolateRA   --> region average 
      @see iclscalemode
      @see resize
  **/
  virtual void scale(int iNewWidth, int iNewHeight, iclscalemode eScaleMode=interpolateNN);
 
 
  
  /// Sets the pixels of one or all channels to a specified value
  /** @param iChannel Channel to fill with zero (default: -1 = all)
      @param tValue destination value (default: 0)
   **/
  void clear(int iChannel = -1, Type tValue = 0);
  
  /// Scale the channel min/ max range to the new range tMin, tMax.
  /** @param fMin new mininum value for the channel
      @param fMax new maximum value for the channel
      @param iChannel channel index (if set to -1, then operation is 
                      performed on all channels)
  **/
  virtual void scaleRange(float fMin=0.0, float fMax=255.0, int iChannel = -1); 

  /// Scales pixel values from given min/max values to new min/max values.
  /** Values exceeding the given range are set to the new min/max values.
      For an automatic scaling use the results of  min(),max() as as arguments.
      (Defining a range allows to compare different images.)
      @param tNewMin destination minimum value
      @param tNewMax destination maximum value
      @param tMin current minimum value
      @param tMax current maximum value
      @param iChannel channel index (if set to -1, then operation is 
                      performed on all channels)
  **/
  virtual void scaleRange(float tNewMin, float tNewMax, float tMin, float tMax, int iChannel=-1);


  //@}

/* }}} */
                                       
  /* {{{ pixel-access using pixel-iterator */

  //@{ @name pixel access using roi iterator                                
  /// type definition for roi iterator
  typedef ICLIterator<Type> iterator;

  /// returns the iterator for the image roi
  /** The following example taken from ICLIterator.h will show
      the iterator usage:
      <pre>
      void channel_convolution_3x3(ICL32f &src, ICL32f &dst,iclfloat *pfMask, int iChannel)
      { 
         for(ICL32f::iterator s=src.begin(iChannel) d=dst.begin() ; s.inRegion() ; s++,d++)
         {
            iclfloat *m = pfMask;
            (*d) = 0;
            for(ICL32f::iterator sR(s, 3, 3); sR.inRegion(); sR++,m++)
            {
               (*d) += (*sR) * (*m);
            }
         }  
      }
      </pre>
      <b>Note:</b> The performance hints illustrated in the
      ICLIterator documentation.
      @param iChannel selected channel index
      @return roi-iterator
      @see ICLIterator
      @see end
  */
  inline iterator begin(int iChannel)
    {
      FUNCTION_LOG("begin(" << iChannel << ")");
      return iterator(getData(iChannel),m_vecROI[0],m_vecROI[1],m_iWidth,m_vecROI[2],m_vecROI[3]);
    }
  //@}
  /* }}} */
                                               
#ifdef WITH_IPP_OPTIMIZATION
                                     
  /* {{{ IPP-compability functions */

  //@{ @name IPP-compability-functions prefix:ipp
  /// returns the image pointer to the bottom left corner of the images ROI
  /** if IPP functions are using image ROIs, than the initial data pointer
      needs to point not the bottom left pixel of the image, but the bottom left
      pixel of the images ROI:
      <pre>

      image: (o=roi)              
      ..............                    ..............x<--getDataEnd()
      ....oooooooo..                    ....oooooooo..
      ....oooooooo..       ippData(): ----->xooooooo..
      ..............                    ..............
      ..............  getDataBegin(): ->x.............
      </pre>
  
      @param iChannel selects a specific channel
      @return "ROI'ed" image data pointer
  */
  Type *ippData(int iChannel) const
    {
      FUNCTION_LOG("ippData(" << iChannel << ")");
      return getData(iChannel)+m_vecROI[0]+m_vecROI[1]*m_iWidth;
    }

  /// returns the data pointer (in respect to the images roi) as iclbyte*
  /** When implementing ipp-accelerated template functions, you may need
      the functions ippData8u and ippData32f to get type-save data pointers.
      Regard the following example:
      <pre>
      template<class T>
      void scale_image_with_ipp(ICL<T> *a, ICL<T> *b)
      {
         for(int c=0;c<a->getChannels();c++)
         {
            if(a->getDepth() == depth8u)
            {
               ippScale_8u_C1R(a->ippData(c),...);           
            }
            else
            {
               ippScale_32f_C1R(a->ippData(c),...);                  
            }
         }
      }
      </pre>
      This looks fine first, but the compiler will complain about wrong types!
      Although due to dynamic type checking (if(a->getDepth()...) no error
      would occur during runtime, the functions ippScale_8u_... and ippScale_32f_...
      are compiled for template type T=iclfloat and T=iclbyte. So if T is iclbyte
      the ippScale_32f_...-call is not allowed, and is not compilable.
      The following code example will explain, how the functions ippData8u and 
      ippData32f can be used to avoid these complications:
     
      <pre>
      template<class T>
      void scale_image_with_ipp(ICL<T> *a, ICL<T> *b)
      {
         for(int c=0;c<a->getChannels();c++)
         {
            if(a->getDepth() == depth8u)
            {
               ippScale_8u_C1R(a->ippData8u(c),...);           
            }
            else
            {
               ippScale_32f_C1R(a->ippData32(c),...);                  
            }
         }
      }
      </pre>
      This example causes no compile errors at all, and also, 
      due to dynamic type checking (if(a->getDepth()...), 
      no runtime error will occur.
      
      It is strongly recommended to use ICLBase class to avoid these problems.
      As ICLBase is not a template, it's not necessary to implement functions
      as templates:
      <pre>
      
      void scale_image_with_ipp(ICLBase *a, ICLBase *b)
      {
         if(a->getDepth() != b->getDepht())
         {
            error or type conversion....
         }
         for(int c=0;c<a->getChannels();c++)
         {
            if(a->getDepth() == depth8u)
            {
               ippScale_8u_C1R(a->asIcl8u()->ippData(c),...);           
            }
            else
            {
               ippScale_32f_C1R(a->asIcl32f()->ippData(c),...);                  
            }
         }
      }
      </pre>
      
      @param iChannel selects a specific channel
      @return data pointer casted to iclbyte* (without type check)
  
  **/
  virtual Ipp8u *ippData8u(int iChannel) const
    {
      FUNCTION_LOG("ippData8u(" << iChannel << ")");
      return reinterpret_cast<iclbyte*>(ippData(iChannel));
    }
  
  /// returns the data pointer (in respect to the images roi) as iclfloat*
  /** This function behaves essentially like the above function
      @param iChannel selects a specific channel
      @return data pointer casted to iclbyte* (without type check)
  **/
  virtual Ipp32f *ippData32f(int iChannel) const
    {
      FUNCTION_LOG("ippData32f(" << iChannel << ")");
      return reinterpret_cast<iclfloat*>(ippData(iChannel));
    }
  /// returns the roi size in Ippi compatible format IppiSize
  /** @return roi size of the channel
  **/
  virtual IppiSize ippROISize() const
    {
      FUNCTION_LOG("");
      IppiSize oSize = {m_vecROI[2],m_vecROI[3]};
      return oSize; 
    }

  /// returns the roi offset in Ippi compatible format IppiPoint
  /** @return roi offset of the channel
  **/
  virtual IppiPoint ippROIOffset() const
    {
      FUNCTION_LOG("");
      IppiPoint oOffset = {m_vecROI[0],m_vecROI[1]};
      return oOffset;
    }

  /// returns the roi-rect of this channel in Ippi compatible format IppiRect
  /** @return roi-rect of the channel
  **/
  virtual IppiRect ippROI() const
    {
      IppiRect oRoi = {m_vecROI[0],m_vecROI[1],m_vecROI[2],m_vecROI[3]};
      return oRoi;
    }

  /// returns the line width in bytes of the image
  /** @return "step" of image line
  **/
  virtual int ippStep() const
    {
      FUNCTION_LOG("");
      return (m_eDepth == depth8u ? sizeof(iclbyte) : sizeof(iclfloat)) * getWidth();
    }
  
  /// returns the size of the image in Ippi compatible format IppiSize
  /** @return size of the image  
  **/
  virtual IppiSize ippSize() const
    {
      FUNCTION_LOG("");
      IppiSize oSize = {getWidth(),getHeight()};
      return oSize; 
    }

 //@}
  /* }}} */
                                       
#endif
  
                                       
};// class
  
  
} //namespace icl

#endif //ICL_H

