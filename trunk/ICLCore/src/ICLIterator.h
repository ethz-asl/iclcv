#ifndef ICLITERATOR_H
#define ICLITERATOR_H

#include "ICLCore.h"

namespace icl{
  /// Iterator class used to iterate over an ICLs ROI pixels
  /**
  The ICLIterator is a utility to iterate line by line over
  all pixels of an ICLs ROI. The following ASCII image 
  shows an images ROI.
  <pre>
    1st pixel
      |
  ....|.................... 
  ....|..ooooooooo......... ---
  ....|..ooooooooo.........  |
  ....|..ooooooooo......... iRoiH
  ....|..ooooooooo.........  |
  ....+->xoooooooo......... ---
  ......................... 
         |-iRoiW-|
  |---------iImageW-------|
  
  </pre>
  
  For image operation like thresholding or filters,
  it is necessary perform calculation for each ROI-
  pixel. To achieve that, the programmer needs to
  Take care about:
     - xoffset
     - yoffset
     - step to jump if right border of the roi 
       is reached (imageW-roiW). current x must be reset
       to the xoffset, and y must be increased by 1
     - check of the last valid pixel position

  the following code example shows how to
  handle image ROIs using the ICLIterator image iterator
  
  <pre>
  void channel_threshold_inplace(ICL8u &im, int iTetta, int iChannel)
  {
      for(ICL8u::iterator p=im.begin(c)  ; p.inRegion() ; p++)
      {
          *p = *p > tetta ? 255 : 0;
      }
     
  }
  </pre>
  The ICLIterator<Type> is defined in the ICL<Type> as iterator.
  This offers an intuitive "stdlib-like" use.

  <h3>Using the ICLIterator as ROW-iterator</h3>
  The ICLIterator can be used as ROW-iterator too. The following example
  will explain usage:
  
  <pre>
  void copy_channel_roi_row_by_row(ICL8u &src, ICL8u &dst, int iChannel)
  {
     for(ICL8u::iterator s=src.begin(iChannel),d=dst.begin(iChannel) ; s.inRegion() ; d.incLine(), s.incLine())
     {
        memcpy(&*d,&*s,s.getROIWidth()*sizeof(iclbyte));
     }
  }
  </pre>

  <h3> Using Nested ICLIterators for Neighborhood operations </h3>

  In addition to the above functionalities, ICLIterators can be used for
  arbitrary image neighborhood operations like convolution, median or
  erosion. The following example explains how to create so called sub-region
  iterators, that work on a symmetrical neighborhood around a higher lever
  ICLIterator.

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

  This code implements a single channel image convolution operation. 


  <h2>Performance:Efficiency</h2>
  There are 3 major ways to access the pixel data of an image.
  - using the (x,y,channel) -operator
  - using the ICLIterator
  - working directly with the channel data

  Each method has its on advantages and disadvantages:
  - the (x,y,channel) operator is very intuitive and it can be used
    to write code whiches functionality is very transparent to 
    other programmers. The disadvantages are:
    - no implicit ROI - support
    - <b>very slow</b>
  - the ICLIterator moves pixel-by-pixel, line-by-line over
    a single image channel. It is highly optimized for processing
    each pixel of an images ROI without respect to the particular
    pixel position in in the image.
    Its advantages are:
     - internal optimized ROI handling
     - direct access to sub-ROIS
     - fast (nearly 10 times faster then the (x,y,channel)-operator
  - the fastest way to process the image data is work directly
    with the data pointer received from image.getData(channel).
    In this case the programmer himself needs to take care about
    The images ROI. This is only recommended, if no ROI-support
    should be provided.

  <h2>Performance:In Values</h2>
  The following example shows use of the different techniques
  to set image data of a single channel image to a static value.
  (Times: 1.4Mhz Pentium-M machine with 512 MB-Ram, SuSe-Linux 9.3)
  <pre>
  // create a VERY large image
  int iW = 10000, iH=10000;
  ICL8u im(iW,iH,1);

  // 1st working with the image data (time: ~210/360ms)
  // pointer style (~210ms)
  for(iclbyte *p= im.getData(0), *d=p+iW*iH ; p<d; ){
     *p++ = 5;
  }
  
  // index style (~360ms)
  iclbyte *pucData = im.getData(0);
  for(int i=0;i<im.getWidth()*im.getHeight();i++){
     pucData[i]=42;
  }

  // 2nd working with the iterator (time: ~280ms) (further implementation ~650ms)
  for(ICL8u::iterator it=im.begin(0) ; it.inRegion() ; it++){
    *it = 42;
  }

  // 3rd working with the (x,y,channel)-operator (time: ~2400)
  for(int x=0;x<im.getWidth();x++){
    for(int y=0;y<im.getHeight();y++){
      im(x,y,0) = 42;
    }
  }

  // for comparison: memset (time: ~140ms)
  memset(pucData,42,im.getWidth()*im.getHeight());
  
  </pre>
  <b>Note</b> Working directly on the image data, is fast for 
  algorithms that are not using the pixels position (x,y) or the
  images ROI.
  
  
  */
  template <class Type>
    class ICLIterator{
    public:
     /// Default Constructor
     /** Creates an ICLIterator object with Type "Type"
         @param ptData pointer to the corresponding channel data
         @param iXPos x offset of the images ROI
         @param iYPos y offset of the images ROI
         @param iImageWidth width of the corresponding image
         @param iROIWidth width of the images ROI
         @param iROIHeight width of the images ROI
     */
    ICLIterator(Type *ptData, int iXPos,int iYPos,int iImageWidth, int iROIWidth, int iROIHeight):
       m_iImageWidth(iImageWidth),
       m_iROIWidth(iROIWidth), 
       m_iROIHeight(iROIHeight), 
       m_iLineStep(m_iImageWidth - m_iROIWidth + 1),
       m_ptDataOrigin(ptData),
       m_ptDataCurr(ptData+iXPos+iYPos*iImageWidth),
       m_ptDataEnd(m_ptDataCurr+iROIWidth+(iROIHeight-1)*iImageWidth),
       m_ptCurrLineEnd(m_ptDataCurr+iROIWidth-1){}

    /// 2nd Constructor to create sub-regions of an ICL-image
    /** This 2nd constructor creates a sub-region iterator, which may be
        used e.g. for arbitrary neighborhood operations like 
        linear filters, medians, ...
        See the ICLIterator description for more detail.        
        @param roOrigin reference to source Iterator Object
        @param iROIWidth width of the images ROI
        @param iROIHeight width of the images ROI
    */

    ICLIterator(const ICLIterator<Type> &roOrigin,int iROIWidth, int iROIHeight):
       m_iImageWidth(roOrigin.m_iImageWidth),
       m_iROIWidth(iROIWidth),
       m_iROIHeight(iROIHeight),
       m_iLineStep(m_iImageWidth - m_iROIWidth + 1),
       m_ptDataOrigin(roOrigin.m_ptDataOrigin),
       m_ptDataCurr(roOrigin.m_ptDataCurr-(iROIWidth/2)-(iROIHeight/2)*m_iImageWidth),
       m_ptDataEnd(m_ptDataCurr+iROIWidth+(iROIHeight-1)*m_iImageWidth),
       m_ptCurrLineEnd(m_ptDataCurr+iROIWidth-1){}
    
    /// retuns a reference of the current pixel value
    /** changes on *p (p is of type ICLIterator) will effect
        the image data       
    */
    inline Type &operator*() const
       {
          return *m_ptDataCurr;
       }
    
    /// moves to the next iterator position
    /** The image ROI will be scanned line by line
        beginning on the bottom left iterator.
       <pre>

           +--'I' is the first invalid iterator
           |  (p.inRegion() will become false)
    .......V.................
    .......I.................
    .......++++++++X<---------- last valid pixel
    .......+++++++++.........
    .......+++++++++.........
    .......9++++++++.........
    ....+->0++-->++8.........
    ....|....................
        |
       begin here

       </pre>
       
       In most cases The ++ operator will just increase the
       current x position and update the reference to the
       current pixel data. If the end of a line is reached, then
       the position is set to the beginning of the next line.
    */
    inline void operator ++(int i)
       {
         (void)i;
         if ( m_ptDataCurr == m_ptCurrLineEnd )
           {
             m_ptDataCurr += m_iLineStep;
             m_ptCurrLineEnd += m_iImageWidth;
           }
         else
           {
             m_ptDataCurr++;
           }
       }
    
    /// to check if iterator is still inside the ROI
    /** @see operator++ */
    inline bool inRegion() const
       {
          return m_ptDataCurr < m_ptDataEnd;          
       }

    /// returns the length of each row processed by this iterator
    /** @return row length 
     */
    inline int getROIWidth() const
       {
          return m_iROIWidth;
       }
    
    inline int getROIHeight() const
       {
          return m_iROIHeight;
       }
    
    /// moved the pixel vertically forward
    /** current x value is hold, the current y-value is
        incremented by iLines
        @param iLines amount of lines to jump over
    */
    inline void incRow(int iLines=1) 
       {
          m_ptDataCurr += iLines * m_iImageWidth;
          m_ptCurrLineEnd += iLines * m_iImageWidth;
       }

    /// returns the current x position of the iterator (image-coordinates)
    /** @return current x position*/
    inline int x()
       {
          return (m_ptDataCurr-m_ptDataOrigin) % m_iImageWidth;
       }

    /// returns the current y position of the iterator (image-coordinates)
    /** @return current y position*/
    inline int y()
       {
          return (m_ptDataCurr-m_ptDataOrigin) / m_iImageWidth;
       }       
    private:
    /// corresponding images width
    int m_iImageWidth;
    
    /// corresponding images ROI width
    int m_iROIWidth;

    /// corresponding images ROI height
    int m_iROIHeight;

    /// result of m_iImageWidth - m_iROIWidth
    int m_iLineStep;

    /// pointer to the image data pointer (bottom left pixel)
    Type *m_ptDataOrigin;

    /// pointer to the current data element
    Type *m_ptDataCurr;

    /// pointer to the first invalid pixel of ptDataOrigin
    Type *m_ptDataEnd;

    /// pointer to the first invalid pixel of the current line
    Type *m_ptCurrLineEnd;

    
  };
}
#endif
