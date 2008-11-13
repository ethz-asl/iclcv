#include <iclSize.h>
#include <map>

namespace icl{
  const Size Size::null(0,0);
  const Size Size::QQVGA(160,120);
  const Size Size::CGA(320,200);
  const Size Size::QVGA(320,240);
  const Size Size::HVGA(480,320);
  const Size Size::EGA(640,350);
  const Size Size::VGA(640,480);
  const Size Size::WVGA(800,480);
  const Size Size::SVGA(800,600);
  const Size Size::QHD(960,540);
  const Size Size::DVGA(960,640);
  const Size Size::XGA(1027,768);
  const Size Size::XGAP(1152,864);
  const Size Size::DSVGA(1200,800);
  const Size Size::HD720(1280,720);
  const Size Size::WXGA(1280,800);
  const Size Size::WXGAP(1440,900);
  const Size Size::SXVGA(1280,960);
  const Size Size::SXGA(1280,1024);
  const Size Size::WSXGA(1600,900);
  const Size Size::SXGAP(1400,1050);
  const Size Size::WSXGAP(1600,1050);
  const Size Size::UXGA(1600,1200);
  const Size Size::HD1080(1920,1080);
  const Size Size::WUXGA(1920,1080);
  const Size Size::UD(3840,2160);

  // video formats
  const Size Size::CIF(352,288);
  const Size Size::SIF(360,240);
  const Size Size::SQCIF(128,96);
  const Size Size::QCIF(176,144);
  const Size Size::PAL(768,576);
  const Size Size::NTSC(640,480);

  const Size &Size::fromString(const std::string &name){
    static std::map<std::string,const Size*> m;
    static bool first = true;
    if(first){
      first = false;
#define A(X) m[#X] = &Size::X
      A(null);A(QQVGA);A(CGA);A(QVGA);
      A(HVGA);A(EGA);A(VGA);A(WVGA);A(SVGA);
      A(QHD);A(DVGA);A(XGAP);A(DSVGA);
      A(HD720);A(WXGA);A(WXGAP);A(SXVGA);A(SXGA);
      A(WSXGA);A(SXGAP);A(UXGA);A(HD1080);
      A(WUXGA);A(UD);A(CIF);A(SIF);A(SQCIF);
      A(QCIF);A(PAL);A(NTSC);
#undef A      
    }
    std::map<std::string,const Size*>::iterator it = m.find(name);
    if(it!=m.end()) return *it->second;
    else return Size::null;
  }
}
