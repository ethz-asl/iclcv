#include "iclUnicapGrabber.h"
#include <iclImg.h>
#include <string>
#include <unicap.h>
#include <iclConverter.h>
#include <iclCC.h>
#include "iclUnicapDevice.h"

using namespace icl;
using namespace std;

#define MAX_DEVICES 64
#define MAX_FORMATS 64
#define MAX_PROPERTIES 64

namespace icl{

  std::vector<UnicapDevice> &UnicapGrabber::getDeviceList(){
    // {{{ open
    static std::vector<UnicapDevice> s_CurrentDevices;
    s_CurrentDevices.clear();

    for(int i=0;true;++i){
      UnicapDevice d(i);
      if(d.isValid()){
        s_CurrentDevices.push_back(d);
      }else{
        break;
      }
    }
    return s_CurrentDevices;
  }

  // }}}
  

static void new_frame_cb (unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t * buffer, void *usr_data){
  // {{{ open
  printf("callback was called \n");
  volatile int *frame_count = (volatile int *) usr_data;
  *frame_count = *frame_count - 1;
}

// }}}

void capture_frames_dma (unicap_handle_t handle, int nframes){
  // {{{ open

  unicap_format_t format;
  volatile int frame_count;
   
  if (!SUCCESS (unicap_get_format (handle, &format))){
    fprintf (stderr, "Failed to get video format!\n");
    exit (-1);
  }
  
  format.buffer_type = UNICAP_BUFFER_TYPE_SYSTEM;       // (1)
   
  if (!SUCCESS (unicap_set_format (handle, &format))){
    fprintf (stderr, "Failed to set video format!\n");
    exit (-1);
  }
  
  printf("registering callback \n");
  frame_count = nframes;
  unicap_register_callback (handle, UNICAP_EVENT_NEW_FRAME, (unicap_callback_t) new_frame_cb, (void *) &frame_count);   // (2)
  
  printf("starting to capture \n");
  unicap_start_capture (handle); 
  while (frame_count > 0){
    printf("framecount is %d \n",frame_count);
    usleep (1000000);
  }
  printf ("Captured %d frames!\n", nframes);
  
  unicap_stop_capture (handle); // (5)
  
}

// }}}

vector<ImgBase*>  capture_frames (unicap_handle_t handle, int nframes){
  // {{{ open
  vector<ImgBase*> vec;
  unicap_format_t format;
  unicap_data_buffer_t buffer;
  int cframe = 0;
  
  if (!SUCCESS (unicap_get_format (handle, &format))){
    fprintf (stderr, "Failed to get video format!\n");
    exit (-1);
  }
  
  format.buffer_type = UNICAP_BUFFER_TYPE_USER; // (1)
  
  if (!SUCCESS (unicap_set_format (handle, &format))) {
    fprintf (stderr, "Failed to set video format!\n");
    exit (-1);
  }
  
  buffer.data = new unsigned char[format.buffer_size];
  buffer.buffer_size = format.buffer_size;
  
  unicap_start_capture (handle);        // (4)
  unicap_queue_buffer (handle, &buffer);        // (3)
  
  while (cframe < nframes) {
     
    unicap_data_buffer_t *returned_buffer;
    if (!SUCCESS (unicap_wait_buffer (handle, &returned_buffer)))   {
      fprintf (stderr, "Failed to wait for buffer!\n");
      exit (-1);
    }
    /*****************************
    Img8u *imRGB = new Img8u(Size(640,480),formatGray);
    copy(buffer.data,buffer.data+imRGB->getDim(),imRGB->getData(0));
    vec.push_back(imRGB);
    *******************************/
    
    
    
    Img8u *imRGB = new Img8u(Size(640,480),formatRGB);
    icl8u *rgbBuf = new icl8u[imRGB->getDim()*3];
    ippiYUV422ToRGB_8u_C2C3R(buffer.data,
                             imRGB->getWidth()*2,
                             rgbBuf,
                             imRGB->getWidth()*3,
                             imRGB->getROISize()    );
    
    interleavedToPlanar(rgbBuf,imRGB->getSize(),3, imRGB);
    delete [] rgbBuf;
    
    vec.push_back(imRGB);
    
    if (!SUCCESS (unicap_queue_buffer (handle, returned_buffer)))  {
      fprintf (stderr, "Failed to queue buffer!\n");
      exit (-1);
    }
    
    cframe++;
  }
  printf ("Captured %d frames!\n", nframes);
  unicap_stop_capture (handle); // (6)
  delete [] buffer.data;
  return vec;
}

// }}}

// {{{ unicap_device_t

/************************************
typedef struct { 	
    char identifier [128];
    char model_name [128] ;
    char vendor_name [128] ;
    unsigned long long model_id ;
    unsigned int vendor_id ;
    char cpi_layer [1024] ;
    char device [1024] ;
    unsigned int flags ;
}unicap_device_t;
*************************************/

// }}}

// {{{ unicap_format_t

/************************************
typedef struct{
  char identifier[128];
  // default
  unicap_rect_t size;
  
  // min and max extends
  unicap_rect_t min_size;
  unicap_rect_t max_size;
  
  // stepping:
  // 0 if no free scaling available
  int h_stepping;
  int v_stepping;
   
  // array of possible sizes
  unicap_rect_t *sizes;
  int size_count;
   
  int bpp;
  unsigned int fourcc;
  unsigned int flags;
  
  unsigned int buffer_types;    // available buffer types
  int system_buffer_count;
  
  size_t buffer_size;
  
  unicap_buffer_type_t buffer_type;
};
*************************************/

// }}}

// {{{ unicap_property_t
/************************************************************
struct unicap_property_t{
  char identifier[128];         //mandatory
  char category[128];
  char unit[128];               //
  
  // list of properties identifier which value / behaviour may change if this property changes
  char **relations;
  int relations_count;
  
  union
  {
    double value;               // default if enumerated
    char menu_item[128];
  };
    
  union
  {
    unicap_property_range_t range;      // if UNICAP_USE_RANGE is asserted
    unicap_property_value_list_t value_list;    // if UNICAP_USE_VALUE_LIST is asserted
      unicap_property_menu_t menu;
  };
  
  double stepping;
  
  unicap_property_type_enum_t type;
  u_int64_t flags;              // defaults if enumerated
  u_int64_t flags_mask;         // defines capabilities if enumerated
  
  // optional data
  void *property_data;
  size_t property_data_size;
};
*********************************************************************/
// }}}

  UnicapGrabber::UnicapGrabber():m_poImage(0){}
  
  const ImgBase* UnicapGrabber::grab(ImgBase *poDst){
    ensureCompatible(&m_poImage,depth8u,Size(640,480),formatRGB);

    std::vector<UnicapDevice> vs = UnicapGrabber::getDeviceList();
    printf("found %d devices\n",vs.size());
    for(unsigned int i=0;i<vs.size();i++){
      printf("Device[%d]::%s \n",i,vs[i].getID().c_str());
      vs[i].listFormats();
      vs[i].listProperties();
    }

    
    return new Img8u(Size(640,480),formatRGB);
    
    /*
        vector<ImgBase*> v=capture_frames(handle,10);
        v[0]->deepCopy(&m_poImage);    
        for(unsigned int i=0;i<v.size();i++){
        delete v[i];
        }
        
        return m_poImage;
    */
  }

}


  /*
      void set_format (unicap_handle_t handle){
      // {{{ open

  unicap_format_t formats[MAX_FORMATS];
  int format_count;
  unicap_status_t status = STATUS_SUCCESS;
  int f = -1;
   
  for (format_count = 0; SUCCESS (status) && (format_count < MAX_FORMATS); format_count++){
    status = unicap_enumerate_formats (handle, NULL, &formats[format_count], format_count);
    if (SUCCESS (status)){
      printf ("%d: %s\n", format_count, formats[format_count].identifier);
    }else{
      break;
    }
  }
  if(!format_count){
    return;
  }
  
  while ((f < 0) || (f >= format_count)){
    printf ("Use Format:  using 3 \n");
    //    scanf ("%d", &f);
    f=3;
  }
  
  if (formats[f].size_count){
    int i;
    int s = -1;
    
    for (i = 0; i < formats[f].size_count; i++){
      printf ("%d: %dx%d\n", i, formats[f].sizes[i].width,
              formats[f].sizes[i].height);
    }
    
    while ((s < 0) || (s >= formats[f].size_count)){
      printf ("Select Size: using 3! \n");
      //scanf ("%d", &s);
      s=3;
    }
    formats[f].size.width = formats[f].sizes[s].width;
    formats[f].size.height = formats[f].sizes[s].height;
  }
  
  if (!SUCCESS (unicap_set_format (handle, &formats[f]))){
    fprintf (stderr, "Failed to set the format!\n");
    exit (-1);
  }
}

// }}}
      
      unicap_handle_t open_device (){
      // {{{ open

  int dev_count;
  int status = STATUS_SUCCESS;
  unicap_device_t devices[MAX_DEVICES];
  unicap_handle_t handle;
  int d = -1;
  
  for (dev_count = 0; SUCCESS (status) && (dev_count < MAX_DEVICES);dev_count++){
    status = unicap_enumerate_devices (NULL, &devices[dev_count], dev_count); // (1)
    if (SUCCESS (status)){
      printf ("%d: %s\n", dev_count, devices[dev_count].identifier);
    }else{
      break;
    }
  }
  
  if (dev_count == 0){
    return NULL;  // no device selected
  }
  while ((d < 0) || (d >= dev_count)){
    printf ("Open Device: \n");
    d=0;
    
    //scanf ("%d", &d);
  }
  unicap_open (&handle, &devices[d]);   // (2)
  return handle;
}

// }}}
      
      void  set_range_property (unicap_handle_t handle){
      // {{{ open

  unicap_property_t properties[MAX_PROPERTIES];
  int property_count;
  int range_ppty_count;
  unicap_status_t status = STATUS_SUCCESS;
  int p = -1;
  double new_value = 0.0;
  
  for (property_count = range_ppty_count = 0; SUCCESS (status) && (property_count < MAX_PROPERTIES); property_count++){
    status = unicap_enumerate_properties (handle, NULL, &properties[range_ppty_count], property_count);       // (1)
    if (SUCCESS (status)){
      if (properties[range_ppty_count].type == UNICAP_PROPERTY_TYPE_RANGE){
        printf ("%d: %s\n", range_ppty_count, properties[range_ppty_count].identifier);
        range_ppty_count++;
      }
    }else{
      break;
    }
  }
  if (range_ppty_count == 0){
    // no range properties
    return;
  }
  while ((p < 0) || (p > range_ppty_count)){
    printf ("Property: ");
    scanf ("%d", &p);
  }
  
  status = unicap_get_property (handle, &properties[p]); 
  if (!SUCCESS (status)){
    fprintf (stderr, "Failed to inquire property '%s'\n",properties[p].identifier);
    exit (-1);
  }
  printf ("Property '%s': Current = %f, Range = [%f..%f]\n",
          properties[p].identifier, properties[p].value,
          properties[p].range.min, properties[p].range.max);
  
  new_value = properties[p].range.min - 1.0f;
  while ((new_value < properties[p].range.min) || (new_value > properties[p].range.max)){
    printf ("New value for property: ");
    scanf ("%lf", &new_value);
  }
  properties[p].value = new_value;
  if (!SUCCESS (unicap_set_property (handle, &properties[p]))){
    fprintf (stderr, "Failed to set property!\n");
    exit (-1);
  }
}

// }}}
  */
