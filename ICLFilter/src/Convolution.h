#ifndef ICLCONVOLUTION_H
#define ICLCONVOLUTION_H

#include <FilterMask.h>

namespace icl {
  
  /// Class for Image convolutions (Img8u, Img32f: IPP + Fallback, all other Types: Fallback only!)
  /**
  The Convolution class provides functionality for any kind of convolution
  filters. As most other filter operations, it operates on the source images
  ROI only. Because the filter mask has to fit into the image at every point
  of the ROI, the ROI is eventually <em>shrinked</em> as described in base
  class Filter.

  <h2>Efficiency (IPP-Optimized)</h2>

  All possible filter operations can be divided in 4 cases, depending on the
  source and destination images depths and the depth of the used filter
  kernel.  While all image depths are supported, the only available kernel 
  depths are depth32f (floating point) and depth32s (32-bit signed integer)
  Note the differences of the following cases:
  
  <h3>case images: depth8u, depth16s, depth32s </h3>
  In this case, an integer kernel is preferred. That means, that an integer
  kernel will be used, if available. Using a float kernel causes a marginal
  decline in performance.

  <h3>case images: depth32f, depth64f </h3>
  In this case, a float kernel is preferred. If it is not available, the
  fallback integer-kernel must be used. As convolution operations of float
  images with integer kernels are not supported by the IPP, the kernel is
  converted internally into a float-kernel. 

  <h3>Benchmarks</h3>
  The IPP-optimized functions are <b>VERY</b> fast in comparison to the 
  fallback C++ implementations. The optimized 3x3 convolution functions
  provided by the IPP are more then 20 times faster. Here are some benchmarks:
  - arbitrary 3x3-convolution 1000x1000 single channel image (IPP-OPTIMIZED)
     - icl8u images & int kernel <b>~11.6ms</b>
     - icl32f images & int kernel <b>~11.1ms</b>
     - icl8u images & icl32f kernel <b>~13.5ms</b>
     - icl32f-image & icl32f kernel <b>~11.3ms</b>
  - fixed 3x3 convolution 1000x1000 single channel sobelx (IPP-OPTIMIZED)
     - icl8u images & int mask <b>~4ms (!!!)</b>
     - icl8u images & icl32f mask <b>~8ms (!!!)</b>
  - arbitrary 3x3-convolution 1000x1000 single channel image (C++-Fallback)
     - icl8u images & int kernel <b>~56ms</b> (further implem. ~81ms) ???
     - icl32f images & int kernel <b>~76ms</b> (further implem. ~370ms)
     - icl8u images & icl32f kernel <b>~135ms</b> (further implem. ~230ms)
     - icl32f-image & icl32f kernel <b>~60ms</b> (further implem. ~60ms)
  
  <h2>Buffering Kernels</h2>
  In some applications the Convolution object has to be created
  during runtime. If the filter-kernel is created elsewhere, and it
  is persistent over the <i>lifetime</i> of the Convolution object,
  it may not be necessary to deeply copy the kernel into an internal buffer
  of the Convolution object. To make the Convolution object just using a
  given kernel pointer, an additional flag <b>iBufferData</b> can be set
  in two Constructors.
  */

  class Convolution : protected FilterMask {
    public:
    /// this enum contains several predefined convolution kernels
    /** <h3>kernelSobleX</h3>
        The sobel x filter is a combined filter. It performs a symmetrical
        border filter operation in x-direction, followed by a smoothing
        operation in y-direction:
        <pre>

                        ---------     ---        
                       | 1  0 -1 |   | 1 |    ---------
        kernelSobelX = | 2  0 -2 | = | 2 | * | 1  0 -1 |
                       | 1  0 -1 |   | 1 |    ---------
                        ---------     ---
        </pre>           
    
        <h3>kernelSobelY</h3>
        The sobel y filter is essentially equal to the sobel y filter. The
        border detection will run in y-direction, and the smoothing x-direction.

        <pre>

                        ---------     ---        
                       | 1  2  1 |   | 1 |    ---------
        kernelSobelY = | 0  0  0 | = | 0 | * | 1  2  1 |
                       |-1 -2 -1 |   |-1 |    ---------
                        ---------     ---
        </pre>               
    
        <h3>kernelGauss3x3</h3>
        This is a 3x3-Pixel approximation of a 2D Gaussian. It is separable into
        smoothing filters in x- and y-direction.
        
        <pre>

                                 ---------     ---        
                                | 1  2  1 |   | 1 |    ---------
        kernelGauss3x3 = 1/16 * | 2  4  2 | = | 2 | * | 1  2  1 | * (1/4) *(1/4) 
                                | 1  2  1 |   | 1 |    ---------
                                 ---------     ---
        </pre>               

        <h3>kernelGauss5x5</h3>
        This is a 5x5-Pixel approximation of a 2D Gaussian. It is separable into
        smoothing filters in x- and y-direction.
        
        <pre>

                                  ----------------- 
                                 | 2   7  12  7  2 |
                                 | 7  31  52 31  7 |
        kernelGauss5x5 = 1/571 * |12  52 127 52 12 |
                                 | 7  31  52 31  7 |
                                 | 2   7  12  7  2 |
                                  ----------------- 

        </pre>               

        <h3>kernelLaplace</h3>
        The Laplacian kernel is a discrete approximation of the 2nd derivation
        of a 2D function.

        <pre>

                         --------- 
                        | 1  1  1 |
        kernelLaplace = | 1 -8  1 |
                        | 1  1  1 |
                         --------- 
        </pre>    
    
      
    */
    enum kernel { 
      kernelGauss3x3,  /*!< 3x3 approximation of a Gaussian */
      kernelGauss5x5,  /*!< 5x5 approximation of a Gaussian */
      kernelSobelX3x3, /*!< 3x3 sobel x filter */
      kernelSobelX5x5, /*!< 5x5 sobel x filter */
      kernelSobelY3x3, /*!< 3x3 sobel y filter */
      kernelSobelY5x5, /*!< 5x5 sobel y filter */
      kernelLaplace3x3, /*!< 3x3 approximation of the 2nd derivation */
      kernelLaplace5x5, /*!< 5x5 approximation of the 2nd derivation */
      kernelCustom  /*!< used for all other user defined kernels */ 
    };
    
    /// create Convolution object with a fixed predefined filter type
    /** @param eKernel determines the filter type 
        (kernelCustom is not allowed here!)
    */
    explicit Convolution(kernel eKernel);

    /// Default constructor
    Convolution();

    /// Creates a Convolution object with the given custom kernel
    /** Create an instance of the Convolution object, which uses the given kernel.
        If the parameter bBufferData is given, the kernel data is internally buffered
        both as float and as an int array (if possible). If the kernel should not be
        buffered, the pointer to the kernel data is stored directly. In this case
        it is assumed, that the pointer stays valid as long as apply() is called.
        The ownership for pfKernel is <b>not transfered</b> to the Convolution object,
        rather the owner is responsible to release this pointer properly.

        @param pfKernel convolution kernel data
        @param size kernel mask size
        @param bBufferData flag that indicates, if given data should be 
        buffered internally. By default given data will be buffered.
    */
    Convolution(icl32f *pfKernel, const Size& size, bool bBufferData=true);

    /// Creates a Convolution object with the given custom kernel
    /** This constructor behaves essentially like the above one.
        Additionally to the kernel mask itself, a normalization factor is needed.
        The is used to normalize the scalar product of kernel and image mask.
        Usually it is the sum of purely possitive kernel entries or it equals 1.

        @param piKernel convolution kernel data
        @param size kernel mask size
        @param bBufferData flag that indicates, if given data should be 
        buffered internally. By default given data will be buffered.
        @param iNormFactor The NormFactor (Defaultvalue :1)
    */
    Convolution(int *piKernel, const Size& size, 
                int iNormFactor=1, bool bBufferData=true);

    /// Destructor
    ~Convolution();
    
    /// performs the convolution operation on the image
    /** The destination image is automatically set up to correct size and its
        channel count is set to the source images channel count.  
        @param poSrc  source image
        @param ppoDst destination image
    */
    void apply(const ImgBase *poSrc, ImgBase **ppoDst);

    /// performs the convolution operation on the image
    /** The destination image must match the depth of the source image.
        All other parameters, i.e. size, \#channels, etc., is automatically 
        set up w.r.t. the source image.
        @param poSrc source image
        @param poDst destination image
    */
    void apply(const ImgBase *poSrc, ImgBase *poDst);
    
    /// change kernel
    void setKernel (kernel eKernel);
    /// change kernel (and/or normalization factor)
    void setKernel (int *piKernel, const Size& size, int iNormFactor=1, bool bBufferData=true);
    /// change kernel
    void setKernel (icl32f *pfKernel, const Size& size, bool bBufferData=true);

    /// retrieve kernel pointer
    template<typename KernelT> const KernelT* const getKernel() const;

    FilterMask::setClipToROI;
    FilterMask::setCheckOnly;

    private:
    /// internal storage for the sobel-x filter kernels
    static int KERNEL_SOBEL_X_3x3[10], KERNEL_SOBEL_X_5x5[26];
    /// internal storage for the sobel-y filter kernels
    static int KERNEL_SOBEL_Y_3x3[10], KERNEL_SOBEL_Y_5x5[26];
    /// internal storage for the gauss filter kernels
    static int KERNEL_GAUSS_3x3[10], KERNEL_GAUSS_5x5[26];
    /// internal storage for the Laplace filter kernels
    static int KERNEL_LAPLACE_3x3[10], KERNEL_LAPLACE_5x5[26];
  
    /// storage of the kernel data
    float *pfKernel;
    /// storage of the kernel data
    int   *piKernel;
    int    iNormFactor; // normalization factor for integer kernel
    
    /// indicates that data is buffered
    bool   m_bBuffered;

    /// kernel type
    kernel m_eKernel;
    depth  m_eKernelDepth;

    /// checks whether float array can be interpreted as int
    bool isConvertableToInt (float *pfData, int iLen);
    /// copy external int kernel to internal float buffer
    void copyIntToFloatKernel (int iDim);
    /// make major changes for both setKernel() versions
    void cleanupKernels (depth newDepth, const Size& size, bool bBufferData);
    /// create kernel buffers
    void bufferKernel (float *pfKernel);
    void bufferKernel (int *piKernel);
    /// release kernel buffers
    void releaseBuffers ();

    /// array of image-type selective convolution methods (assigned during setKernel calls)
    void (Convolution::*aMethods[depthLast+1])(const ImgBase *poSrc, ImgBase *poDst);
    /// static array of image- and kernel-type selective generic convolution methods
    static void (Convolution::*aGenericMethods[depthLast+1][2])(const ImgBase *poSrc, ImgBase *poDst);

#ifdef WITH_IPP_OPTIMIZATION 
    template<typename T, IppStatus (IPP_DECL *)(const T*, int, T*, int, IppiSize, const Ipp32s*, IppiSize, IppiPoint, int)>
    void ippGenericConvIntKernel (const ImgBase *poSrc, ImgBase *poDst);
    template<typename T, IppStatus (IPP_DECL *)(const T*, int, T*, int, IppiSize, const Ipp32f*, IppiSize, IppiPoint)>
    void ippGenericConvFloatKernel (const ImgBase *poSrc, ImgBase *poDst);
    template<typename T>
    void ippFixedConv (const ImgBase *poSrc, ImgBase *poDst);
    template<typename T>
    void ippFixedConvMask (const ImgBase *poSrc, ImgBase *poDst);

    /// function pointers for ipp fixed convolution, with and without mask size parameter
#define ICL_INSTANTIATE_DEPTH(T) \
    IppStatus (IPP_DECL *pFixed ## T)(const Ipp ## T* pSrc, int srcStep, Ipp ## T* pDst, int dstStep, IppiSize roiSize); \
    IppStatus (IPP_DECL *pFixedMask ## T)(const Ipp ## T* pSrc, int srcStep, Ipp ## T* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask);
    ICL_INSTANTIATE_DEPTH(8u)
    ICL_INSTANTIATE_DEPTH(16s)
    ICL_INSTANTIATE_DEPTH(32f)
#undef ICL_INSTANTIATE_DEPTH

    template<typename T> IppStatus (IPP_DECL *getIppFixedMethod() const)(const T*, int, T*, int, IppiSize);
    template<typename T> IppStatus (IPP_DECL *getIppFixedMaskMethod() const)(const T*, int, T*, int, IppiSize, IppiMaskSize);
    
    /// set ipp methods for fixed kernels
    void setIPPFixedMethods(kernel);
#endif // IPP available

    template<typename ImgT, typename KernelT, bool bUseFactor>
    void cGenericConv (const ImgBase *poSrc, ImgBase *poDst);

    /// initMethods initializes method array aMethods with dummyConvMethod
    void initMethods ();
    void dummyConvMethod (const ImgBase *poSrc, ImgBase *poDst) {}
  };


  /// Convolution using the ROI of an ICL image as its kernel
  /** Sometimes it is useful to use the ROI of an ICL image directly as the
      convolution kernel, e.g. for template matching. Because the ROI may be
      smaller than the image itself, the DynamicConvolution class maintains
      an internal buffer poKernelBuf of this ROI only. Its first channel is
      directly set as the (unbuffered) kernel data of the underlying Convolution
      class.
  */
  class DynamicConvolution : protected Convolution {
  public:
     DynamicConvolution (const ImgBase* poKernel = 0);
     ~DynamicConvolution ();

     void setKernel (const ImgBase* poKernel);
     Convolution::setClipToROI;
     Convolution::setCheckOnly;
     Convolution::apply;
  private:
     icl::Img<icl::icl32f> *poKernelBuf;
  };

  template<> inline const int*   const Convolution::getKernel<int>()   const {return piKernel;}
  template<> inline const float* const Convolution::getKernel<float>() const {return pfKernel;}

}

#endif
