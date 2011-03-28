/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/VideoGrabber.cpp                             **
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

#include <ICLIO/VideoGrabber.h>

#undef  HAVE_CONFIG_H // xineutils.h tries to include its own config.h
#define XINE_ENABLE_EXPERIMENTAL_FEATURES
#include <xine.h>
#include <xine/xineutils.h>
#include <ICLIO/File.h>
#include <ICLCore/Img.h>
#include <vector>
#include <ICLCC/CCFunctions.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLUtils/Mutex.h>

namespace icl{

  struct VideoGrabber::XineHandle{
    xine_t              *xine;
    xine_stream_t       *stream;
    xine_video_port_t   *vo_port;
    xine_audio_port_t   *ao_port;
    
    XineHandle(const std::string &filename) throw (InvalidFileException,InvalidFileException):
      xine(0),stream(0),vo_port(0),ao_port(0){
      xine = xine_new();
      xine_config_load(xine,(str(xine_get_homedir())+"/.xine/config").c_str());
      xine_init(xine);
      vo_port = xine_new_framegrab_video_port (xine);
      ao_port = xine_open_audio_driver(xine ,"auto", NULL);
      stream = xine_stream_new(xine, ao_port, vo_port);
      
      if(!xine_open(stream, filename.c_str())){
        throw InvalidFileException("unable to open file stream (filename:" + filename + ")");
      }
      
      if(!xine_play(stream, 0, 0)){
        throw InvalidFileException("unable to play file stream (filename:" + filename + ")");
      }

      /*
          const char * const * oop = xine_list_audio_output_plugins (xine);
          for(const char * const *p=oop;*p;++p){
          DEBUG_LOG("AUDIO OUTPUT PLUGIN " << str(*p) );
          }
      */
    }
    ~XineHandle(){
      xine_close(stream);
      xine_dispose(stream);
      if(ao_port){
        xine_close_audio_driver(xine, ao_port);  
      }
      xine_close_video_driver(xine, vo_port);  
      xine_exit(xine);
    }

    std::string get_4cc() {
      int fourcc = xine_get_stream_info(stream,XINE_STREAM_INFO_VIDEO_FOURCC);    
      char cc4[5]={0};
      memcpy(cc4,&fourcc,4);
      return cc4;
    }
  };
  
  struct VideoGrabber::Data{
    Mutex mutex;
    std::string fileName;
    Size imageSize;

    ImgBase *outputBuffer;

    double frameDuration;
    double fps;
    
    int streamLengthMS;
    bool isSeekable;
    
    float ar;
    
    Data(const std::string &filename, VideoGrabber::XineHandle* xine){
      fileName = filename;
      
      int w = xine_get_stream_info (xine->stream, XINE_STREAM_INFO_VIDEO_WIDTH);
      int h = xine_get_stream_info (xine->stream, XINE_STREAM_INFO_VIDEO_HEIGHT);
      
      int fd = xine_get_stream_info (xine->stream, XINE_STREAM_INFO_FRAME_DURATION);
      frameDuration = double(fd)/90000;
      fps = 1.0/frameDuration;
      
      int xar = xine_get_stream_info(xine->stream,XINE_PARAM_VO_ASPECT_RATIO);
      switch(xar){
        case XINE_VO_ASPECT_SQUARE: ar = 1.0; break;
        case XINE_VO_ASPECT_4_3:    ar = 4./3; break;
        case XINE_VO_ASPECT_ANAMORPHIC: ar = 16./9; break;
        case XINE_VO_ASPECT_DVB: ar = 2.11/1; break;
        default: ar = -1;
      }

      imageSize = Size(w,h);
      
      if(w%8){
        WARNING_LOG("the xine base video grabber plugin might not be able to"
                    " grab frames correctly (frame width needs to be a multiple of 8)");
      }

      int p=0,t=0,l=0;
      int success = xine_get_pos_length(xine->stream,&p,&t,&l);
      if(!success){
        ERROR_LOG("unable to acquire stream length");
        streamLengthMS = 0;
        isSeekable = false;
      }else{
        streamLengthMS = l;
        isSeekable = xine_get_stream_info (xine->stream, XINE_STREAM_INFO_SEEKABLE);
      }
      


      outputBuffer = 0;
    }
    ~Data(){
      ICL_DELETE(outputBuffer);
    }
  };
    
  struct VideoGrabber::Params{
    /// one of auto, manual or unlimited
    std::string speedMode;
    
    /** The speed variable is used as follows:
        range 0..100
        (0-49 means slower)
        0   = 1/10 speed
        50  = normal speed
        (51-100 means faster)
        100 = 10x speed 
        */
    int speed; 

    double streamFPS;
    FPSLimiter fpsl;
    FPSLimiter fpslAuto;
    
    int streamOffs;

    Params(double streamFPS):
      streamFPS(streamFPS),
      fpsl(streamFPS,5),
      fpslAuto(streamFPS,5){
      speedMode = "auto";
      speed = 50;
      streamOffs = 0;
    }
    
    
    void setUserSpeed(int value){
      ICLASSERT_RETURN(value >= 0);
      ICLASSERT_RETURN(value <= 100);
      if(value == 50){
        fpsl.setMaxFPS(streamFPS);
      }else if(value < 50){
        fpsl.setMaxFPS( ((0.9/50) * value + 0.1) * streamFPS);
      }else{
        fpsl.setMaxFPS( ((9.0/50) * value - 8.0) * streamFPS);
      }
      speed = value;
    }
    
    void wait(double streamFPS){
      if(speedMode == "unlimited") return;
      if(speedMode == "auto"){
        fpslAuto.wait();
      }else if(speedMode == "manual"){
        fpsl.wait();
      }
    }
  };
  
  
  VideoGrabber::VideoGrabber(const std::string &filename) throw (FileNotFoundException,InvalidFileException):
    m_xine(0),m_data(0),m_params(0){
    
    if(!File(filename).exists()){
      throw FileNotFoundException(filename);
    }
    
    m_xine = new XineHandle(filename);
    m_data = new Data(filename,m_xine);
    m_params = new Params(m_data->fps);
  }

  VideoGrabber::~VideoGrabber(){
    ICL_DELETE(m_xine);
    ICL_DELETE(m_data);
    ICL_DELETE(m_params);
  }  
  
  static void convert_frame(icl8u *data, const Size &size,Img8u *image, const std::string &cc4){
    // u and v channels are swapped -> bug in xine?
    int dim4 = size.getDim()/4;
    icl8u *u = data+dim4*4;
    icl8u *v = u+dim4;
    for(int i=0;i<dim4;++i){
      std::swap(u[i],v[i]);
    }
    convertYUV420ToRGB8(data,size,image);
  }

  void VideoGrabber::pause(){
    xine_set_param(m_xine->stream,XINE_PARAM_SPEED,XINE_SPEED_PAUSE);
  }
  void VideoGrabber::unpause(){ 
    xine_set_param(m_xine->stream,XINE_PARAM_SPEED,XINE_SPEED_NORMAL);
  }

  void VideoGrabber::restart(){
    xine_stop(m_xine->stream);
    //xine_open(stream, mrl);
    xine_play(m_xine->stream,0,0);
  }


  const ImgBase *VideoGrabber::acquireImage(){
    Mutex::Locker lock(m_data->mutex);
  
    m_params->wait(m_data->fps);

    xine_video_frame_t f={0,0,0,0,0,0,0,0,0,0,0};
    int success = xine_get_next_video_frame (m_xine->vo_port,&f);
    if(!success){
      restart();
      success = xine_get_next_video_frame (m_xine->vo_port,&f);
    }
    if(!success){
      throw ICLException("unable to read from stream [" +str(__FUNCTION__)+ "]");
    }
    
    if(f.colorspace != XINE_IMGFMT_YV12){
      throw ICLException("invalid xine colorspace (currently only XINE_IMGFMT_YV12 is allowed) ["+str(__FUNCTION__)+"]");
    }
    
    m_params->streamOffs = f.pos_time;
    Size size(f.width,f.height);    

    ensureCompatible(&m_data->outputBuffer,depth8u,size,formatRGB);
    convert_frame(f.data,size,m_data->outputBuffer->asImg<icl8u>(),m_xine->get_4cc());
    xine_free_video_frame (m_xine->vo_port,&f);
    
    return m_data->outputBuffer;
  }
  
  std::vector<std::string> VideoGrabber::getPropertyList(){
    static const std::string ps="speed-mode speed stream-pos stream-length volume is-seekable";
    return tok(ps," ");
  }

  std::string VideoGrabber::getType(const std::string &name){
    if(name == "speed-mode"){
      return "menu";
    }else if(name == "speed" || name == "stream-pos" || name == "volume"){
      return "range";
    }else if(name == "stream-length" || name == "is-seekable"){
      return "info";
    }
    return "undefined";
  }
   
  std::string VideoGrabber::getInfo(const std::string &name){
    if(name == "speed-mode"){
      return "{\"auto\",\"manual\",\"unlimited\"}";
    }else if(name == "speed"){
      return "[0,100]:1";
    }else if(name == "stream-pos"){
      return "[0,"+getValue("stream-length")+"]:1";
    }else if(name == "stream-length"){
      return "";
    }else if(name == "volume"){
      return "[0,100]:1";
    }
    return "undefined";
  }

  std::string VideoGrabber::getValue(const std::string &name){
    if(name == "speed-mode"){
      return m_params->speedMode;
    }else if(name == "speed"){
      return str(m_params->speed);
    }else if(name == "stream-pos"){
      return str(m_params->streamOffs);
    }else if(name == "stream-length"){
      return str(m_data->streamLengthMS)+" ms";
    }else if(name == "is-seekable"){
      return m_data->isSeekable ? "yes" : "no";
    }else if(name == "volume"){
      return str(xine_get_param(m_xine->stream,XINE_PARAM_AUDIO_VOLUME));
    }
    return "undefined";
  }

  void VideoGrabber::setProperty(const std::string &name, const std::string &value){
    Mutex::Locker l(m_data->mutex);
    if(name == "speed-mode"){
      m_params->speedMode = value;
    }else if(name == "speed"){
      m_params->setUserSpeed(parse<int>(value));
    }else if(name == "stream-pos"){
      DEBUG_LOG("here: new streap pos is " << value);
      int streamPos = parse<int>(value);
      xine_stop(m_xine->stream);
      int success = xine_play(m_xine->stream,0,streamPos);
      if(!success){
        int err = xine_get_error (m_xine->stream);
        /*
            switch(err){
            case XINE_ERROR_NONE: 
            break;
            case 
            case 
            #define XINE_ERROR_NO_INPUT_PLUGIN         1
            #define XINE_ERROR_NO_DEMUX_PLUGIN         2
            #define XINE_ERROR_DEMUX_FAILED            3
            #define XINE_ERROR_MALFORMED_MRL           4
            #define XINE_ERROR_INPUT_FAILED            5
            }
         */
        ERROR_LOG("found this xine error code: " << err);
      }
    }else if(name == "stream-length"){
      ERROR_LOG("stream-length is an info-variable, that cannot be set up externally!");
    }else if(name == "volume"){
      int vol = parse<int>(value);
      ICLASSERT_RETURN(vol >= 0);
      ICLASSERT_RETURN(vol <= 100);
      xine_set_param (m_xine->stream,XINE_PARAM_AUDIO_VOLUME,vol);
    }
  }
    
  int VideoGrabber::isVolatile(const std::string &propertyName){
    return propertyName == "stream-pos" ? (int)(1000.0*m_data->frameDuration) : 0;
  }
}

