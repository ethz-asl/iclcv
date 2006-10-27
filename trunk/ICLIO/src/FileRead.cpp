/*
  FileRead.cpp

  Written by: Michael G�tting, Robert Haschke (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include <stdlib.h>
#include <wordexp.h>
#include <dirent.h>
#include <zlib.h>
#include <stdio.h>

#include <fstream>
#include <sstream>

#include <FileRead.h>
#include <Converter.h>

using namespace std;

namespace icl {

  inline void replace_newline (string::value_type& c) {
     if (c == '\n') c = ' ';
  }

  //--------------------------------------------------------------------------
  FileRead::FileRead(string sPattern) throw (ICLException) 
    // {{{ open

  {
    wordexp_t match;
    char **ppcFiles;

    // remove newlines from sPattern
    std::for_each (sPattern.begin(), sPattern.end(), replace_newline);

    // search for file matching the pattern(s)
    switch (wordexp (sPattern.c_str(), &match, WRDE_UNDEF)) {
       case 0: break;
       case WRDE_BADCHAR: 
          throw ICLException ("illegal chars in pattern (|, &, ;, <, >, (, ), {, }");
          break;
       case WRDE_BADVAL:
          throw ICLException ("encountered undefined shell variable");
          break;
       case WRDE_NOSPACE:
          throw ICLException ("out of memory");
          break;
       case WRDE_SYNTAX:
          throw ICLException ("syntax error, e.g. unbalanced parentheses or quotes");
          break;
    }

    ppcFiles = match.we_wordv;
    for (unsigned int i=0; i < match.we_wordc; ++i) {
       bool bGzipped;
       switch (getFileType (ppcFiles[i], bGzipped)) {
         case ioFormatSEQ: readSequenceFile (ppcFiles[i]); break;
         case ioFormatUnknown: break; // skip unknown file types
         default:
            m_vecFileName.push_back(ppcFiles[i]);
            break;
       }
    }
    wordfree(&match);

    this->init ();
  }

// }}}

  //--------------------------------------------------------------------------
  FileRead::FileRead(const string& sPrefix, const string& sType, 
                     int iObjStart, int iObjEnd, 
                     int iImageStart, int iImageEnd) throw (ICLException) 
    // {{{ open 

  {
     ostringstream ossFile;
     ossFile << "." << sType;
     bool bDummy;

     ioformat eFormat = getFileType (ossFile.str(), bDummy);
     if (eFormat == ioFormatSEQ || eFormat == ioFormatUnknown)
        throw ICLException (string("not supported file type ") + sType);

     //---- Build filename ----
     for (int i=iObjStart;i<=iObjEnd;i++) {
        for (int j=iImageStart;j<=iImageEnd;j++) {
           ossFile.clear ();
           ossFile << sPrefix << i << "__" << j << "." << sType;
           m_vecFileName.push_back(ossFile.str());
        }
     }

     this->init ();
  }

// }}}
  
  //--------------------------------------------------------------------------
  FileRead::FileRead(const FileRead& other) : m_poCurImg(0) {
     *this = other;
  }
  
  //--------------------------------------------------------------------------
  FileRead& FileRead::operator=(const FileRead& other) {
    // {{{ open

     if (!this->m_bBuffered) delete this->m_poCurImg;
     this->m_vecFileName = other.m_vecFileName;
     this->m_vecImgBuffer = other.m_vecImgBuffer;
     this->m_bBuffered = other.m_bBuffered;
     this->m_iCurImg = other.m_iCurImg;
     this->m_poCurImg = 0;

     // setup the jpeg error routine once
     jpgCinfo.err = jpeg_std_error(&jpgErr);
     jpgErr.error_exit = icl_jpeg_error_exit;
     return *this;
  }

// }}}

  //--------------------------------------------------------------------------
  FileRead::~FileRead ()
     // {{{ open
  {
     for (ImageBuffer::iterator it=m_vecImgBuffer.begin(), 
             end=m_vecImgBuffer.end(); it != end; ++it)
        delete *it;

     if (!m_bBuffered) delete m_poCurImg;
  }

  // }}}
  
  //--------------------------------------------------------------------------
  void FileRead::init() throw (ICLException) 
     // {{{ open
  {
     m_bBuffered = false;
     m_iCurImg   = 0;
     m_poCurImg  = 0;
     if (m_vecFileName.size () == 0)
        throw ICLException ("empty file list");

     // setup the jpeg error routine once
     jpgCinfo.err = jpeg_std_error(&jpgErr);
     jpgErr.error_exit = icl_jpeg_error_exit;
  }
  // }}}

  //--------------------------------------------------------------------------
  ImgI* FileRead::grab(ImgI* poDst) 
    throw (ICLInvalidFileFormat, FileOpenException, ICLException) {
    // {{{ open 

    if (m_bBuffered) {
       m_poCurImg = m_vecImgBuffer[m_iCurImg];
    } else {
       readImage (m_vecFileName[m_iCurImg], &m_poCurImg);
    }
    
    // forward to next image
    next ();

    //---- Convert to output format ----
    if (poDst) {
       Converter oConv;
       oConv.convert(poDst, m_poCurImg);
       return poDst;
    } else {
      return m_poCurImg;
    }
  }

  // }}}

  //--------------------------------------------------------------------------
  void FileRead::readImage(const string& sFileName, ImgI** ppoDst) 
    throw (ICLInvalidFileFormat, FileOpenException, ICLException) {
    // {{{ open 

    //---- Variable definition ----
    FileInfo oInfo (sFileName);

    // set some defaults
    oInfo.oROI = Rect(); // full ROI

    try {
       switch (oInfo.eFileFormat) {
         case ioFormatPNM:
         case ioFormatICL:
            readHeaderPNM (oInfo);
            ensureCompatible (ppoDst, oInfo.eDepth, oInfo.oImgSize, 
                              oInfo.iNumChannels, oInfo.eFormat, oInfo.oROI);
            readDataPNM (*ppoDst, oInfo);
            break;
         case ioFormatJPG:
            readHeaderJPG (oInfo);
            ensureCompatible (ppoDst, oInfo.eDepth, oInfo.oImgSize, 
                              oInfo.iNumChannels, oInfo.eFormat, oInfo.oROI);
            readDataJPG ((*ppoDst)->asImg<icl8u>(), oInfo);
            break;
         default:
            throw ICLException (string("not supported file type"));
       }
    } catch (ICLException &e) {
       if (oInfo.fp) closeFile (oInfo);
       throw;
    }
    closeFile (oInfo);
  }
  // }}}
  
  //--------------------------------------------------------------------------
  FileRead::FileList FileRead::bufferImages (bool bStopOnError) throw ()
    // {{{ open

  {
     FileList vecErrorList;
     if (m_bBuffered) return vecErrorList; // do not buffer twice

     // if we enter buffering mode and already read images by calling grab
     // we must free the internally allocated image first
     if (m_poCurImg && !m_bBuffered) { delete m_poCurImg; m_poCurImg = 0; }
    
     // clear buffer from previous trial
     for (ImageBuffer::iterator it=m_vecImgBuffer.begin(), 
             end=m_vecImgBuffer.end(); it != end; ++it)
        delete *it;
     m_vecImgBuffer.clear();

     for (FileList::iterator it=m_vecFileName.begin(), end=m_vecFileName.end();
          it != end; ++it) {
        m_poCurImg = 0; // force reallocation of new image pointer
        try {
           readImage (*it, &m_poCurImg);
        } catch (ICLException &e) {
           // in any case, report the error
           vecErrorList.push_back (*it);
           if (bStopOnError) return vecErrorList;
           // create some dummy image to insert to buffer vector
           if (!m_poCurImg) m_poCurImg = imgNew (depth8u, Size(1,1), formatGray);
        }
        m_vecImgBuffer.push_back (m_poCurImg);
     }

     // only on successful reading of all images
     m_bBuffered = true;
     return vecErrorList;
  }

  // }}}
  
  //--------------------------------------------------------------------------
  void FileRead::readSequenceFile (const std::string& sFileName) 
     // {{{ open
  {
     string sFile;
     ifstream streamSeq (sFileName.c_str(),ios::in);

     if(!streamSeq)
     {
        ERROR_LOG("Can't open sequence file: " << sFileName);
     }
      
     while(streamSeq) {
        getline (streamSeq, sFile);
        m_vecFileName.push_back(sFile);
     }
     streamSeq.close();
  }
// }}}

  //--------------------------------------------------------------------------
  void FileRead::readHeaderPNM (FileInfo &oInfo) {
    // {{{ open

    openFile (oInfo, "rb"); // open file for reading
    char acBuf[1024];
    istringstream iss;

    //---- Read the magic number  ----
    if (!gzgets (oInfo.fp, acBuf, 1024) ||
        acBuf[0] != 'P') throw ICLInvalidFileFormat();
    switch (acBuf[1]) {
      case '6': oInfo.eFormat = formatRGB; break;
      case '5': oInfo.eFormat = formatGray; break;
      default: throw ICLInvalidFileFormat();
    }

    //---- Set default values ----
    oInfo.iNumChannels = getChannelsOfFormat(oInfo.eFormat);
    oInfo.iNumImages = 1;
    oInfo.eDepth = depth8u;
        
    // {{{ Read special header info

    do {
       if (!gzgets (oInfo.fp, acBuf, 1024)) throw ICLInvalidFileFormat();
       if (acBuf[0] != '#') break;

       // process comment
       iss.str (acBuf+1);
       string sKey, sValue;
       iss >> sKey;

       if (sKey == "NumFeatures" || sKey == "NumImages") {
          iss >> oInfo.iNumImages;
          oInfo.iNumChannels *= oInfo.iNumImages;
       } else if (sKey == "ROI") {
          iss >> oInfo.oROI.x;
          iss >> oInfo.oROI.y;
          iss >> oInfo.oROI.width;
          iss >> oInfo.oROI.height;
          continue;
       } else if (sKey == "ImageDepth") {
          // ignore image depth for all formats but ICL
          if (!oInfo.eFileFormat == ioFormatICL) return;
          iss >> sValue;
          if (sValue == "depth8u") oInfo.eDepth = depth8u;
          else if (sValue == "depth32f") oInfo.eDepth = depth32f;
          else { ERROR_LOG("Unknown image depth: " + sValue); }
          continue;
       } else if (sKey == "Format") {
          iss >> sValue;
          oInfo.eFormat = translateFormat(sValue.c_str());
       }
       
       //---- Is num channels in depence to the format ----
       if (getChannelsOfFormat(oInfo.eFormat) != oInfo.iNumChannels)
          oInfo.eFormat = formatMatrix;
       
    } while (true);

    // }}}
    
    // read image size
    iss.str (acBuf);
    iss >> oInfo.oImgSize.width;
    iss >> oInfo.oImgSize.height;
    oInfo.oImgSize.height = oInfo.oImgSize.height / oInfo.iNumImages;

    // skip line with maximal pixel value
    if (!gzgets (oInfo.fp, acBuf, 1024)) throw ICLInvalidFileFormat();
  }

  // }}}

  //--------------------------------------------------------------------------
  void FileRead::readDataPNM(ImgI* poImg, FileInfo &oInfo) {
    // {{{ open

    if (oInfo.iNumImages == oInfo.iNumChannels ||
        oInfo.eFileFormat == ioFormatICL) {
       // file format is non-interleaved, i.e. grayscale or icl proprietary
       int iDim = poImg->getDim () * getSizeOf (poImg->getDepth ());
       for (int i=0;i<oInfo.iNumChannels;i++) {
          if (gzread (oInfo.fp, poImg->getDataPtr(i), iDim) != iDim)
             throw ICLInvalidFileFormat ();
       }
    } else if (poImg->getDepth() == depth8u) {
       // file format is interleaved, i.e. RGB or something similar
       Img8u *poImg8u = poImg->asImg<icl8u>();
       int iLines = oInfo.oImgSize.height;
       int iDim   = 3 * oInfo.oImgSize.width;
       icl8u *pcBuf = new icl8u[iDim];
       icl8u *pc;

       for (int i=0;i<oInfo.iNumImages;i++) {
          icl8u *pcR = poImg8u->getData (i*3);
          icl8u *pcG = poImg8u->getData (i*3+1);
          icl8u *pcB = poImg8u->getData (i*3+2);
          for (int l=0; l<iLines; l++) {
             if (gzread (oInfo.fp, pcBuf, iDim) != iDim)
                throw ICLInvalidFileFormat ();
             pc=pcBuf;
             for (int c=0; c<oInfo.oImgSize.width; ++c, ++pcR, ++pcG, ++pcB) {
                *pcR = *pc++;
                *pcG = *pc++;
                *pcB = *pc++;
             } // for cols (deinterleave)
          } // for lines
       } // for images
       delete[] pcBuf;
    } else {
       ERROR_LOG ("This should not happen.");
    }
  }

  // }}}

  //--------------------------------------------------------------------------
  void FileRead::readHeaderJPG (FileInfo &oInfo) {
     // {{{ open

     openFile (oInfo, "rb"); // open file for reading

     /* SETUP ERROR HANDLING:
      *
      * Our example here shows how to override the "error_exit" method so that
      * control is returned to the library's caller when a fatal error occurs,
      * rather than calling exit() as the standard error_exit method does.
      *
      * We use C's setjmp/longjmp facility to return control.  This means that the
      * routine which calls the JPEG library must first execute a setjmp() call to
      * establish the return point.  We want the replacement error_exit to do a
      * longjmp().  But we need to make the setjmp buffer accessible to the
      * error_exit routine.  To do this, we use a private extension of the
      * standard JPEG error handler object.
      */

     if (setjmp(jpgErr.setjmp_buffer)) {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object and signal the error to the caller */
        jpeg_destroy_decompress(&jpgCinfo);
        throw ICLInvalidFileFormat();
     }

     /* Step 1: Initialize the JPEG decompression object. */
     jpeg_create_decompress(&jpgCinfo);

     /* Step 2: specify data source (eg, a file) */
     jpeg_stdio_src(&jpgCinfo, (FILE*) oInfo.fp);

     /* Step 3: read file parameters with jpeg_read_header() */
     (void) jpeg_read_header(&jpgCinfo, TRUE);

     /* Step 4: set parameters for decompression */

     /* Step 5: Start decompressor */
     (void) jpeg_start_decompress(&jpgCinfo);

     /* After jpeg_start_decompress() we have the correct scaled
      * output image dimensions available, as well as the output colormap
      * if we asked for color quantization.
      */

     oInfo.eDepth = depth8u; // can only read depth8u
     oInfo.oImgSize.width  = jpgCinfo.output_width;
     oInfo.oImgSize.height = jpgCinfo.output_height;
     switch (jpgCinfo.out_color_space) {
       case JCS_GRAYSCALE: oInfo.eFormat = formatGray; break;
       case JCS_RGB: oInfo.eFormat = formatRGB; break;
       case JCS_YCbCr: oInfo.eFormat = formatYUV; break;
       default: throw ICLException("unknown color space");
     }
     oInfo.iNumChannels = getChannelsOfFormat (oInfo.eFormat);
  }

  // }}}

  //--------------------------------------------------------------------------
  void FileRead::readDataJPG(Img<icl8u>* poImg, FileInfo &oInfo)
     // {{{ open

  {
     icl8u *pcBuf = 0;
     // update jump context to allow proper throw
     if (setjmp(jpgErr.setjmp_buffer)) {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object and signal the error to the caller */
        jpeg_destroy_decompress(&jpgCinfo);
        if (oInfo.iNumChannels == 3) delete[] pcBuf;
        throw ICLInvalidFileFormat();
     }

     ICLASSERT (jpgCinfo.output_components == oInfo.iNumChannels);
     int iRowDim = jpgCinfo.output_width * jpgCinfo.output_components;
     icl8u *pcR, *pcG, *pcB;

     if (oInfo.iNumChannels == 1) pcBuf = poImg->getData (0);
     else if (oInfo.iNumChannels == 3) {
        pcBuf = new icl8u[iRowDim];
        pcR = poImg->getData (0);
        pcG = poImg->getData (1);
        pcB = poImg->getData (2);
     }
     else {ERROR_LOG ("This should not happen."); return;}

     /* Step 6: while (scan lines remain to be read) */
     /*           jpeg_read_scanlines(...); */
     while (jpgCinfo.output_scanline < jpgCinfo.output_height) {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could ask for
         * more than one scanline at a time if that's more convenient.
         */
        (void) jpeg_read_scanlines(&jpgCinfo, &pcBuf, 1);

        if (oInfo.iNumChannels == 1) pcBuf += iRowDim;
        else { // deinterleave three channel data
           icl8u *pc = pcBuf;
           for (int c=0; c<oInfo.oImgSize.width; ++c, ++pcR, ++pcG, ++pcB) {
              *pcR = *pc++;
              *pcG = *pc++;
              *pcB = *pc++;
           }
        }
     }

     /* Step 7: Finish decompression */
     (void) jpeg_finish_decompress(&jpgCinfo);

     /* At this point you may want to check to see whether any corrupt-data
      * warnings occurred (test whether jpgErr.pub.num_warnings is nonzero).
      */

     /* Step 8: Release JPEG decompression object */
     jpeg_destroy_decompress(&jpgCinfo);
     if (oInfo.iNumChannels == 3) delete[] pcBuf;
  }

  // }}}

  //--------------------------------------------------------------------------
  bool FileRead::findFile (const std::string& sFile,
                           FileList::iterator& itList) {
    // {{{ open

     // search starting from itList to end
     FileList::iterator found = find (itList, m_vecFileName.end(), sFile);
     if (found != m_vecFileName.end()) 
        itList = found;
     // search second part from begin to itList
     else if ( (found = find (m_vecFileName.begin(), itList, sFile)) != itList)
        itList = found;
     else return false; // item not found
     return true;
  }

// }}}

  //--------------------------------------------------------------------------
  void FileRead::removeFiles (const FileList& vecFiles) {
    // {{{ open

     FileList::iterator itList = m_vecFileName.begin();
     for (FileList::const_iterator itDel=vecFiles.begin (), end=vecFiles.end();
          itDel != end; ++itDel) {
        if (findFile (*itDel, itList)) {
           if (m_bBuffered) {
              // delete image at corresponding position
              ImageBuffer::iterator itBuf 
                 = m_vecImgBuffer.begin() + (itList-m_vecFileName.begin());
              delete *itBuf;
              m_vecImgBuffer.erase (itBuf);
           }
           // as well as file name itself
           m_vecFileName.erase (itList);
        }
     }
  }

// }}}

} // namespace icl
