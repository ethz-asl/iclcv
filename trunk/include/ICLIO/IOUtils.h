/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/IOUtils.h                                **
** Module : ICLIO                                                  **
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

#ifndef ICL_IO_UTILS_H
#define ICL_IO_UTILS_H

#include <ICLIO/Grabber.h>

#include <string>
#include <vector>
#include <ICLUtils/Time.h>

namespace icl{
  namespace io{
    
    /// an internally used namespace to avoid symbol naming conflicts \ingroup UTILS_G
    namespace ioutils{
      
      /// converts a Time::value_type (long int) into a string
      std::string time2str(Time::value_type x);
  
      /// cops trailing whitespaces of a string
      std::string skipWhitespaces(const std::string &s);
      
      /// converts a string into an integer
      int ti(const std::string &value);
      
      /// converts a string into a long int
      long tl(const std::string &value);
      
      /// creates a vector of 3 elements v = (a,b,c)
      std::vector<int> vec3(int a, int b, int c);
  
      /// returns whether a given string ends with a given suffix
      bool endsWith(const std::string &s,const std::string &suffix);
      
      /// returns whether a given string begins with a given prefix
      bool startsWith(const std::string &s, const std::string &prefix);
  
      /// analyses a file pattern with hash-characters
      /** This function is e.g. used by the FilennameGenerator to extract a patterns hash count
          e.g. the pattern "image_###.ppm" shall be used to generate filenames like 
          "image_000.ppm", "image_001.ppm" and so on. This function returns the count of found
          hashes and the position in the string where the suffix begins. E.g. if the pattern is
          "image_##.ppm.gz", the hash-count is 2 and the suffix-pos becomes 8.
      **/
      void analyseHashes (const std::string &sFileName, unsigned int& nHashes, std::string::size_type& iPostfixPos);
    }
  } // namespace io
}

#endif
