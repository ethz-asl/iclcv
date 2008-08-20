#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([ICL],[3.1],[bugreport@icl.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
CXXFLAGS="$CXXFLAGS -Isrc"


AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([ICLUtils/src/config.h])       

# Checks for programs.
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_SED
AC_CHECK_PROG([HAVE_PKG_CONFIG],[pkg-config],[TRUE],[FALSE])
if test "$HAVE_PKG_CONFIG" = "FALSE" ; then
AC_MSG_ERROR([unable to find 'pkg-config' in PATH (please install and/or adapt path variable])
fi

#########################################################################################
AC_ICL_NORIFY_CHECK([IPP           ])
#########################################################################################

AC_ICL_WITH_ROOT([IPP],[/vol/vision/IPP/5.2])

HAVE_IPP=TRUE
AC_ICL_PUSH_FLAG_VARS
AC_ICL_EXTEND_FLAG_VARS_TMP_FOR([IPP],[sharedlib],[include])
 
AC_CHECK_HEADER([ipp.h],[],[HAVE_IPP=FALSE],[])
AC_CHECK_LIB([ippcore],[ippGetLibVersion],[],[HAVE_IPP=FALSE])
AC_CHECK_LIB([ippi],[ippiGetLibVersion],[],[HAVE_IPP=FALSE])
AC_CHECK_LIB([ipps],[ippsGetLibVersion],[],[HAVE_IPP=FALSE])
AC_CHECK_LIB([ippsr],[ippsrGetLibVersion],[],[HAVE_IPP=FALSE])
AC_CHECK_LIB([ippcv],[ippcvGetLibVersion],[],[HAVE_IPP=FALSE])
AC_CHECK_LIB([ippm],[ippmGetLibVersion],[],[HAVE_IPP=FALSE])
AC_CHECK_LIB([ippcc],[ippccGetLibVersion],[],[HAVE_IPP=FALSE])
AC_CHECK_LIB([guide],[main],[],[HAVE_IPP=FALSE])

AC_ICL_POP_FLAG_VARS

if test "$HAVE_IPP" = "TRUE" ; then
AC_ICL_EXTEND_FLAG_VARS([-L$IPP_ROOT/sharedlib -lippcore -lippi -lipps -lippsr -lippcv -lippm -lippcc -lguide],
                        [-Wl,-rpath=${IPP_ROOT}/sharedlib],
                        [-I$IPP_ROOT/include],
                        [-DHAVE_IPP -DWITH_IPP_OPTIMIZATION])
fi

AM_CONDITIONAL([HAVE_IPP_COND],[test x$HAVE_IPP = xTRUE])



#########################################################################################
AC_ICL_NORIFY_CHECK([libdc1394-2   ])
#########################################################################################

AC_ICL_WITH_ROOT([LIBDC],[/usr])
HAVE_LIBDC=TRUE

AC_ICL_PUSH_FLAG_VARS
AC_ICL_EXTEND_FLAG_VARS_TMP_FOR(LIBDC,lib,include)
        
AC_CHECK_HEADER([dc1394/dc1394.h],[],[HAVE_LIBDC=FALSE])
AC_CHECK_LIB([dc1394],[dc1394_camera_enumerate],[],[HAVE_LIBDC=FALSE])

AC_ICL_POP_FLAG_VARS

if test "$HAVE_LIBDC" = "TRUE" ; then
AC_ICL_EXTEND_FLAG_VARS(
        [-L$LIBDC_ROOT/lib -ldc1394],
        [-Wl,-rpath=$LIBDC_ROOT/lib],
        [-I$LIBDC_ROOT/include],
        [-DHAVE_LIBDC])
fi

AM_CONDITIONAL([HAVE_LIBDC_COND],[test x$HAVE_LIBDC = xTRUE])



#########################################################################################
AC_ICL_NORIFY_CHECK([Unicap        ])
#########################################################################################

AC_ICL_WITH_ROOT([UNICAP],[/usr])
HAVE_UNICAP=TRUE

AC_ICL_PUSH_FLAG_VARS
AC_ICL_EXTEND_FLAG_VARS_TMP_FOR(UNICAP,lib,include/unicap)                
        
        
AC_CHECK_HEADER([unicap.h],[],[HAVE_UNICAP=FALSE])
AC_CHECK_LIB([unicap],[unicap_check_version],[],[HAVE_UNICAP=FALSE])

AC_ICL_POP_FLAG_VARS

if test "$HAVE_UNICAP" = "TRUE" ; then
AC_ICL_EXTEND_FLAG_VARS(
        [-L$UNICAP_ROOT/lib -lunicap -lpthread],
        [-Wl,-rpath=$UNICAP_ROOT/lib],
        [-I$UNICAP_ROOT/include/unicap],
        [-DHAVE_UNICAP])
fi

AM_CONDITIONAL([HAVE_UNICAP_COND],[test x$HAVE_UNICAP = xTRUE])




#########################################################################################
AC_ICL_NORIFY_CHECK([Magick++      ])
#########################################################################################

AC_ICL_WITH_ROOT([IMAGEMAGICK],[/usr])
HAVE_IMAGEMAGICK=TRUE

AC_ICL_PUSH_FLAG_VARS
AC_ICL_EXTEND_FLAG_VARS_TMP_FOR(IMAGEMAGICK,lib,include)                
        
AC_LANG([C++])
AC_CHECK_HEADER([Magick++.h],[],[HAVE_IMAGEMAGICK=FALSE])
AC_CHECK_LIB([Magick++],[InitializeMagick],[],[HAVE_IMAGEMAGICK=FALSE])

AC_ICL_POP_FLAG_VARS

if test "$HAVE_IMAGEMAGICK" = "TRUE" ; then
AC_ICL_EXTEND_FLAG_VARS(
        [-L$IMAGEMAGICK_ROOT/lib -lMagick++],
        [-Wl,-rpath=$IMAGEMGAGICK_ROOT/lib],
        [-I$IMAGEMAGICK_ROOT/include],
        [-DHAVE_IMAGEMAGICK])
fi

AM_CONDITIONAL([HAVE_IMAGEMAGICK_COND],[test x$HAVE_IMAGEMAGICK = xTRUE])


#########################################################################################
AC_ICL_NORIFY_CHECK([libjpeg       ])
#########################################################################################

AC_ICL_WITH_ROOT([LIBJPEG],[/usr])
HAVE_LIBJPEG=TRUE

AC_ICL_PUSH_FLAG_VARS
AC_ICL_EXTEND_FLAG_VARS_TMP_FOR(LIBJPEG,lib,include)                
        
AC_LANG([C++])
AC_CHECK_HEADER([jpeglib.h],[],[HAVE_LIBJPEG=FALSE])
AC_CHECK_LIB([jpeg],[jpeg_std_error],[],[HAVE_LIBJPEG=FALSE])

AC_ICL_POP_FLAG_VARS

if test "$HAVE_LIBJPEG" = "TRUE" ; then
AC_ICL_EXTEND_FLAG_VARS(
        [-L$LIBJPEG_ROOT/lib -lMagick++],
        [-Wl,-rpath=$LIBJPEG_ROOT/lib],
        [-I$LIBJPEG_ROOT/include],
        [-DHAVE_LIBJPEG])
fi

AM_CONDITIONAL([HAVE_LIBJPEG_COND],[test x$HAVE_LIBJPEG = xTRUE])




#########################################################################################
AC_ICL_NORIFY_CHECK([libz          ])
#########################################################################################

AC_ICL_WITH_ROOT([LIBZ],[/usr])
HAVE_LIBZ=TRUE

AC_ICL_PUSH_FLAG_VARS
AC_ICL_EXTEND_FLAG_VARS_TMP_FOR(LIBZ,lib,include)                
        
AC_LANG([C++])
AC_CHECK_HEADER([zlib.h],[],[HAVE_LIBZ=FALSE])
AC_CHECK_LIB([z],[zlibVersion],[],[HAVE_LIBZ=FALSE])

AC_ICL_POP_FLAG_VARS

if test "$HAVE_LIBZ" = "TRUE" ; then
AC_ICL_EXTEND_FLAG_VARS(
        [-L$LIBZ_ROOT/lib -lMagick++],
        [-Wl,-rpath=$LIBZ_ROOT/lib],
        [-I$LIBZ_ROOT/include],
        [-DHAVE_LIBZ])
fi

AM_CONDITIONAL([HAVE_LIBZ_COND],[test x$HAVE_LIBZ = xTRUE])



#########################################################################################
AC_ICL_NORIFY_CHECK([SVS           ])
#########################################################################################

AC_ICL_WITH_ROOT([SVS],[/usr])
HAVE_SVS=TRUE

AC_ICL_PUSH_FLAG_VARS
AC_ICL_EXTEND_FLAG_VARS_TMP_FOR(SVS,bin,src)                
        
AC_LANG([C])
AC_CHECK_HEADER([svs.h],[],[HAVE_SVS=FALSE])
AC_CHECK_LIB([svs],[svsEngineVersion],[],[HAVE_SVS=FALSE],[-lsvscalc])
AC_CHECK_LIB([svscalc],[main],[],[HAVE_SVS=FALSE],[-lsvs])

AC_ICL_POP_FLAG_VARS

if test "$HAVE_SVS" = "TRUE" ; then
AC_ICL_EXTEND_FLAG_VARS(
        [-L$SVS_ROOT/bin -lsvs -lsvscalc],
        [-Wl,-rpath=$SVS_ROOT/lib],
        [-I$SVS_ROOT/src],
        [-DHAVE_SVS])
fi

AM_CONDITIONAL([HAVE_SVS_COND],[test x$HAVE_SVS = xTRUE])



#########################################################################################
AC_ICL_NORIFY_CHECK([Qt            ])
#########################################################################################

AC_ICL_WITH_ROOT([QT],[/usr])
HAVE_QT=TRUE

AC_ICL_PUSH_FLAG_VARS
AC_ICL_EXTEND_FLAG_VARS_TMP_FOR(QT,lib/,include/qt4)                
        
AC_LANG([C++])
AC_CHECK_FILE([$QT_ROOT/lib/pkgconfig/QtCore.pc],[],[HAVE_QT=FALSE])
AC_CHECK_FILE([$QT_ROOT/lib/pkgconfig/QtOpenGL.pc],[],[HAVE_QT=FALSE])
AC_CHECK_FILE([$QT_ROOT/lib/pkgconfig/QtGui.pc],[],[HAVE_QT=FALSE])
AC_CHECK_FILE([$QT_ROOT/lib/pkgconfig/QtXml.pc],[],[HAVE_QT=FALSE])

QT_PACKAGES="QtCore QtOpenGL QtGui QtXml"
QT_LIBS_l=`pkg-config --libs-only-l $QT_PACKAGES`
QT_LIBS_L=`pkg-config --libs-only-L $QT_PACKAGES`
QT_LIBS_RPATH=`echo $QT_LIBS_L | sed "s|-L|-Wl,-rpath=-L|g"`
QT_CFLAGS_I=`pkg-config --cflags-only-I $QT_PACKAGES`
QT_CFLAGS_other=`pkg-config --cflags-only-other $QT_PACKAGES`

AC_ICL_EXTEND_FLAG_VARS([$QT_LIBS_L $QT_LIBS_l],
                        [$QT_LIBS_RPATH],
                        [$QT_CFLAGS_I],
                        [$QT_CLFAGS_other $QT_CFLAGS_I])

AC_CHECK_HEADER([QtCore],[],[HAVE_QT=FALSE])
AC_CHECK_HEADER([QtGui],[],[HAVE_QT=FALSE])
AC_CHECK_HEADER([QtOpenGL],[],[HAVE_QT=FALSE])
AC_CHECK_HEADER([QtXml],[],[HAVE_QT=FALSE])
AC_CHECK_LIB([QtCore],[main],[],[HAVE_QT=FALSE])
AC_CHECK_LIB([QtGui],[main],[],[HAVE_QT=FALSE])
AC_CHECK_LIB([QtOpenGL],[main],[],[HAVE_QT=FALSE])
AC_CHECK_LIB([QtXml],[main],[],[HAVE_QT=FALSE])

AC_ICL_POP_FLAG_VARS

if test "$HAVE_QT" = "TRUE" ; then
AC_ICL_EXTEND_FLAG_VARS(
        [$QT_LIBS_L $QT_LIBS_l],
        [$QT_LIBS_RPATH],
        [$QT_CFLAGS_I],
        [$QT_CLFAGS_other -DHAVE_QT])
fi

AM_CONDITIONAL([HAVE_QT_COND],[test x$HAVE_QT = xTRUE])

#########################################################################################
AC_ICL_NORIFY_CHECK([XCF           ])
#########################################################################################

AC_ICL_WITH_ROOT([XCF],[/vol/xcf])
HAVE_XCF=TRUE

AC_ICL_PUSH_FLAG_VARS
AC_ICL_EXTEND_FLAG_VARS_TMP_FOR(XCF,lib/,include/)                
        
AC_LANG([C++])
AC_CHECK_FILE([$XCF_ROOT/lib/pkgconfig/xcf.pc],[],[HAVE_XCF=FALSE])

export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$XCF_ROOT/lib/pkgconfig"

XCF_LIBS_l=`pkg-config --libs-only-l xcf`
XCF_LIBS_L=`pkg-config --libs-only-L xcf`
XCF_LIBS_RPATH=`echo $XCF_LIBS_L | sed "s|-L|-Wl,-rpath=-L|g"`
XCF_CFLAGS_I=`pkg-config --cflags-only-I xcf`
XCF_CFLAGS_other=`pkg-config --cflags-only-other xcf`

AC_ICL_EXTEND_FLAG_VARS([$XCF_LIBS_L $XCF_LIBS_l],
                        [$XCF_LIBS_RPATH],
                        [$XCF_CFLAGS_I],
                        [$XCF_CLFAGS_other $XCF_CFLAGS_I])

AC_CHECK_HEADER([xcf/xcf.hpp],[],[HAVE_XCF=FALSE])
AC_CHECK_LIB([xcf],[main],[],[HAVE_XCF=FALSE])

AC_ICL_POP_FLAG_VARS

if test "$HAVE_XCF" = "TRUE" ; then
AC_ICL_EXTEND_FLAG_VARS(
        [$XCF_LIBS_L $XCF_LIBS_l],
        [$XCF_LIBS_RPATH],
        [$XCF_CFLAGS_I],
        [$XCF_CLFAGS_other -DHAVE_XCF])
fi

AM_CONDITIONAL([HAVE_XCF_COND],[test x$HAVE_XCF = xTRUE])


#########################################################################################
## Summary ...                        ###################################################
#########################################################################################

AC_MSG_NOTICE([+---------------------------------------+])
AC_MSG_NOTICE([| Summary                               |])
AC_MSG_NOTICE([+---------------------------------------+])
AC_ICL_NOTIFY_SUPPORT([IPP],          [IPP      ])
AC_ICL_NOTIFY_SUPPORT([LIBDC],        [libdc1394])
AC_ICL_NOTIFY_SUPPORT([UNICAP],       [Unicap   ])
AC_ICL_NOTIFY_SUPPORT([QT],           [Qt       ])
AC_ICL_NOTIFY_SUPPORT([LIBJPEG],      [libjpeg  ])
AC_ICL_NOTIFY_SUPPORT([LIBZ],         [libz     ])
AC_ICL_NOTIFY_SUPPORT([SVS],          [SVS      ])
AC_ICL_NOTIFY_SUPPORT([XCF],          [XCF      ])
AC_MSG_NOTICE([+---------------------------------------+])




# Checks for library functions.
AC_CONFIG_FILES([Makefile ICLCore/Makefile ICLUtils/Makefile ICLCC/Makefile ICLIO/Makefile])

AC_OUTPUT

