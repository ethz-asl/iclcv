#ifndef ICL_FILENAME_GENERATOR_H
#define ICL_FILENAME_GENERATOR_H

#include <iclShallowCopyable.h>
#include <string>

namespace icl{
  /** \cond */
  class FilenameGeneratorImpl;
  struct FilenameGeneratorImplDelOp { static void delete_func(FilenameGeneratorImpl *i); };
  /** \endcond */

  /// Utility class for generating a stream of filenames \ingroup UTILS_G
  /** This list can have a finite or an infinite size. The class provides functions to
      get the next filename (which inplicitly increases the internal counter), to get 
      the count of remaining filenames and to reset the internal counter to first value.\n
      The FilenameGenerator class extends the ShallowCopyable class interface to provide
      cheap-copies using reference counting.
  **/
  class FilenameGenerator : public ShallowCopyable<FilenameGeneratorImpl,FilenameGeneratorImplDelOp>{
    public:
    static const int INFINITE_FILE_COUNT;
    /// Null constructor
    FilenameGenerator();

    /// Destructor
    ~FilenameGenerator();
    /// generate a new filename list with given maxFiles
    /** if the maxFile count is reached, the filegenerator is resetted internally
        and will produce start counting from the beginning on again. If
        maxFiles is -1, there is no stop criterion 
        example:
        <pre>
        pattern = image_#.ppm 
        maxFiles = 10
        list = { image_0.ppm, image_1.ppm, ..., image_10.ppm }  
        
        pattern = image_##.ppm
        maxFiles = 10
        list = { image_00.ppm, image_01.ppm, ..., image_10.ppm }  

        pattern = image_#####.ppm.gz
        maxFiles = -1
        list = { image_00000.ppm.gz, image_00001.ppm.gz, ... }
        </pre>
    **/
    FilenameGenerator(const std::string &pattern, int maxFiles=-1);
    
    /// generate a filename list with given prefix,postfix and object/image index range
    /** example: 
        <pre>
        prefix = "image_"
        postfix = ".ppm"
        objectStart = 0
        objectEnd = 5
        imageStart = 10
        imageEnd = 20
        
        list = { image_0__10.ppm, image_0__11.ppm, ..., image_0__20.ppm,
                 image_1__10.ppm, image_1__11.ppm, ..., image_1__20.ppm,
                 ...
                 image_5__10.ppm, image_5__11.ppm, ..., image_5__20.ppm }
        </pre>
    */
    FilenameGenerator(const std::string& prefix, 
                      const std::string& postfix,  
                      int objectStart, int objectEnd, 
                      int imageStart, int imageEnd);
    
    /// returns the next file from the list
    std::string next();

    /// returns the next file without incrementing the internal counter (preview of next filename)
    std::string showNext();
    
    /// returns the number of files left (-1) if the FileList's length is infinite
    int filesLeft() const;
    
    /// must be called if if files left is < 0
    void reset();
    
    /// returns a list of all files (only if the count if files if finite)
    std::vector<std::string> getList();
    
    /// shows all files to if( the filelist is finite, the 10 first files are show);
    void show();
    
    
    
  };
}

#endif
