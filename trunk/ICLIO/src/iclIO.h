#ifndef ICLIO_H
#define ICLIO_H

#include <string>
#include <iclImgBase.h>
#include <iclException.h>
#include <jerror.h>
#include <jpeglib.h>
#include <setjmp.h>
/*
  IO.h

  Written by: Michael G�tting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

/**
\mainpage ICLIO (Input/Ouput) package
\section Overview

The ICLIO package provides the complete input and  output functions supported by the ICL. Currently the following subpackages are included in the IO 
library:
- <b>FileReader</b>: The FileReader could be used to load (pgm, ppm, pnm, jpg, icl) files from a file or a sequence of files. 

- <b>FileWriter</b>: The provides the same file formats as the FileReader. But now the ICL images are written to a file or a file sequence.

- <b>PWCGrabber</b>: The PWC Grabber (Phillips Webcam Grabber) supports various webcams chipsets. For a detailed overview of the supported webcams look at <a href="http://www.saillard.org/linux/pwc/">www.saillard.org</a>.

A detailed description of the provided functions in each package is included in
the class description.

*/


extern "C" {
}

/// Provide some common functionality for all file accessing classes

namespace icl {

  struct FileInfo;

  /// Determine the supported file formats for load and save functions
  enum ioformat {
     ioFormatUnknown = -2,
     ioFormatSEQ = -1, //< file list
     ioFormatPNM, //< PNM file format (gray/pgm or rgb/ppm
     ioFormatICL, //< proprietary format, equals pnm for icl8u, but allows icl32f as well
     ioFormatJPG, //< JPG image format
     ioFormatCSV  //< comma seperated value
  };

  enum GrabMode1394 {
    MONO8_640x480 = 0,
    MONO8_800x600,
    MONO8_1024x768,
    MONO8_1280x960,
    MONO8_1600x1200,
    MONO16_640x480,
    MONO16_800x600,
    MONO16_1024x768,
    MONO16_1280x960,
    MONO16_1600x1200,
    RGB8_640x480,
    RGB8_800x600,
    RGB8_1024x768,
    RGB8_1280x960,
    RGB8_1600x1200,
    YUV422_320x240,
    YUV422_640x480,
    YUV422_800x600,
    YUV422_1024x768,
    YUV422_1280x960,
    YUV422_1600x1200
  };
  
  /// Check for supported file type
  ioformat getFileType (const std::string &sFileName, bool& bGzipped);

  /// Count Hashes directly before file suffix
  void analyseHashes (const std::string &sFileName, unsigned int& nHashes, 
                      std::string::size_type& iSuffixPos);
  /// open given file
  void openFile (FileInfo& oInfo, const char *pcMode) throw (FileOpenException);
  /// close given file
  void closeFile (FileInfo& oInfo);


  struct FileInfo {
     depth       eDepth;
     format      eFormat;
     Time        timeStamp;
     int         iNumImages;
     int         iNumChannels;
     Size        oImgSize;
     Rect        oROI;
     std::string sFileName;
     ioformat    eFileFormat;
     bool        bGzipped;
     void*       fp;

     FileInfo (const std::string& sFileName) : 
        sFileName (sFileName),
        eFileFormat(getFileType (sFileName, bGzipped)),
        fp(0){}
  };

  void icl_jpeg_error_exit (j_common_ptr cinfo);
  struct icl_jpeg_error_mgr : jpeg_error_mgr {
     jmp_buf setjmp_buffer;	/* for return to caller */
  };

} //namespace icl

#endif
