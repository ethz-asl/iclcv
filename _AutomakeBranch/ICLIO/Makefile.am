# libtool libs
lib_LTLIBRARIES=libICLIO.la

libICLIO_la_SOURCES= \
src/iclDemoImageFlowers.cpp \
src/iclDemoImageMacaw.cpp \
src/iclDemoImageWindows.cpp \
src/iclJPEGHandle.cpp \
src/iclFile.cpp \
src/iclFileGrabber.cpp \
src/iclFileGrabberPluginCSV.cpp \
src/iclFileGrabberPluginImageMagick.cpp \
src/iclFileGrabberPluginJPEG.cpp \
src/iclFileGrabberPluginPNM.cpp \
src/iclFileList.cpp \
src/iclFilenameGenerator.cpp \
src/iclFileWriter.cpp \
src/iclFileWriterPluginCSV.cpp \
src/iclFileWriterPluginImageMagick.cpp \
src/iclFileWriterPluginJPEG.cpp \
src/iclFileWriterPluginPNM.cpp \
src/iclGenericGrabber.cpp \
src/iclGrabber.cpp \
src/iclIO.cpp \
src/iclIOUtils.cpp \
src/iclPWCGrabber.cpp \
src/iclSonyFwGrabber.cpp \
src/iclTestImages.cpp

if HAVE_LIBDC_COND
libICLIO_la_SOURCES+= \
src/iclDC.cpp \
src/iclDCDevice.cpp \
src/iclDCDeviceFeatures.cpp \
src/iclDCFrameQueue.cpp \
src/iclDCGrabber.cpp \
src/iclDCGrabberThread.cpp
endif

if HAVE_UNICAP_COND
libICLIO_la_SOURCES+= \
src/iclDefaultConvertEngine.cpp \
src/iclDefaultGrabEngine.cpp \
src/iclUnicapDevice.cpp \
src/iclUnicapFormat.cpp \
src/iclUnicapGrabber.cpp \
src/iclUnicapProperty.cpp
endif

if HAVE_XCF_COND
libICLIO_la_SOURCES+= \
src/iclXCFGrabberBase.cpp \
src/iclXCFMemoryGrabber.cpp \
src/iclXCFPublisher.cpp \
src/iclXCFPublisherGrabber.cpp \
src/iclXCFServerGrabber.cpp \
src/iclXCFUtils.cpp
endif

include_HEADERS= \
src/iclIO.h \
src/iclFileGrabber.h \
src/iclFileGrabberPluginCSV.h \
src/iclFileGrabberPlugin.h \
src/iclFileGrabberPluginJPEG.h \
src/iclFileGrabberPluginPNM.h \
src/iclFileGrabberPluginImageMagick.h \
src/iclGenericGrabber.h \
src/iclGrabber.h \
src/iclFileGrabber.h \
src/iclFile.h \
src/iclFileList.h \
src/iclFilenameGenerator.h \
src/iclFileWriter.h \
src/iclIOUtils.h \
src/iclJPEGHandle.h \
src/iclPWCGrabber.h \
src/iclSonyFwGrabber.h \
src/iclTestImages.h 

if HAVE_LIBDC_COND
include_HEADERS+= \
src/iclDCDeviceFeatures.h \
src/iclDCDevice.h \
src/iclDCDeviceOptions.h \
src/iclDCFrameQueue.h \
src/iclDCGrabber.h \
src/iclDCGrabberThread.h \
src/iclDC.h 
endif

if HAVE_UNICAP_COND
include_HEADERS+= \
src/iclUnicapConvertEngine.h \
src/iclUnicapDevice.h \
src/iclUnicapFormat.h \
src/iclUnicapGrabber.h \
src/iclUnicapGrabEngine.h \
src/iclUnicapProperty.h \
src/iclDefaultConvertEngine.h \
src/iclDefaultGrabEngine.h
endif

if HAVE_XCF_COND
include_HEADERS+= \
src/iclXCFGrabberBase.h \
src/iclXCFMemoryGrabber.h \
src/iclXCFPublisherGrabber.h \
src/iclXCFPublisher.h \
src/iclXCFServerGrabber.h \
src/iclXCFUtils.h
endif

# include other packages here!
AM_CXXFLAGS:=${AM_CXXFLAGS} \
	-I../ICLUtils/src \
	-I../ICLCore/src \
	-I../ICLCC/src 
