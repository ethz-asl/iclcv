#include <iclCommon.h>
#include <iclMeanShiftTracker.h>
#include <iclMouseHandler.h>


Mutex m;
GUI gui("hsplit");
Grabber *grabber = 0;
Point32f *newPos = 0;
Point32f pos;
std::vector<double> COLOR(3,255);

class Mouse : public MouseHandler{
  virtual void process(const MouseEvent &evt){
    m.lock();
    if(evt.isLeft()){
      if(newPos) *newPos = evt.getPos();
      else newPos = new Point32f(evt.getPos());
    }else if(evt.isRight()){
      std::vector<double> newColor = evt.getColor();
      if(newColor.size() == 3){
        COLOR = newColor;
      }else{
        ERROR_LOG("colors must be given in color image mode!");
      }
      Img32f image(Size(4,3),formatRGB);
      std::fill(image.begin(0),image.end(0),COLOR[0]);
      std::fill(image.begin(1),image.end(1),COLOR[1]);
      std::fill(image.begin(2),image.end(2),COLOR[2]);
      gui["color"] = image;
      gui["color"].update();
    }
    m.unlock();
  }
};


void init(){
  grabber = new GenericGrabber(FROM_PROGARG("-input"));
  grabber->setDesiredDepth(depth32f);
  grabber->setDesiredFormat(formatRGB);
  grabber->setDesiredSize(parse<Size>(pa_subarg<std::string>("-size",0,"VGA")));
  
  GUI controls;
  controls << "image[@handle=kernel@minsize=8x6@label=kernel image]"
           << "image[@handle=color@minsize=8x6@label=current color]"
           << "slider(1,1000,20)[@out=maxCycles@label=max cycles]"
           << "fslider(0.1,5,1.0)[@out=convergence@label=conv. crit.]"
           << "slider(4,200,50)[@out=bandwidth@label=kernel bandwidth]"
           << "combo(epanechnikov,gaussian)[@out=_@handle=kernel-type@label=kernel type]"
           << "combo(color image,weight image)[@out=__@handle=vis@label=shown image]";

  
  
  gui << "draw()[@handle=image@minsize=32x24@label=image stream]" 
      << controls;

  gui.show();

  (*gui.getValue<DrawHandle>("image"))->install(new Mouse());
  (*gui.getValue<ImageHandle>("kernel"))->setRangeMode(ICLWidget::rmAuto);
}

struct ColorDist{
  float r,g,b;
  ColorDist(const std::vector<double> &color):
    r(color.at(0)),g(color.at(1)),b(color.at(2)){}
  static inline float sqr(float x){ return x*x; }
  void operator()(const icl32f src[3], icl32f dst[1]) const{
    *dst = 255.0 - sqrt(sqr(r-src[0])+sqr(g-src[1])+sqr(b-src[2]))/sqrt(3);
  }
};

const Img32f &create_weight_image(const Img32f &image, const std::vector<double> &color){
  static Img32f wi(Size(1,1),1);
  wi.setSize(image.getSize());
  image.reduce_channels<icl32f,3,1,ColorDist>(wi,ColorDist(color));
  return wi;
}


  
void run(){
  const Img32f *image = grabber->grab()->asImg<icl32f>();

  m.lock();
  const Img32f &wi = create_weight_image(*image,COLOR);
 
  static int &maxCycles = gui.getValue<int>("maxCycles");
  static float &convergence = gui.getValue<float>("convergence");
  static ComboHandle &kernelType = gui.getValue<ComboHandle>("kernel-type");
  static ComboHandle &shownImage = gui.getValue<ComboHandle>("vis");
  static int &bandwidth = gui.getValue<int>("bandwidth");
  
  static MeanShiftTracker ms(MeanShiftTracker::epanechnikov, 1);
  
  if(ms.getKernel() != kernelType.getSelectedIndex()||
     ms.getBandwidth() != bandwidth){
    ms.setKernel((MeanShiftTracker::kernelType)kernelType.getSelectedIndex(),bandwidth,bandwidth/2);
    gui["kernel"] = ms.getKernelImage();
    gui["kernel"].update();
  }
  if(newPos){
    pos = *newPos;
    ICL_DELETE(newPos);
  }
  pos = ms.step(wi,pos,maxCycles,convergence);
  m.unlock();

  static ICLDrawWidget &w = **gui.getValue<DrawHandle>("image");
  w.setImage( (shownImage.getSelectedIndex()) ? (&wi) : (image));
  w.lock();
  w.reset();
  w.color(255,0,0,255);
  w.fill(255,0,0,50);
  w.rect(pos.x-bandwidth,pos.y-bandwidth,2*bandwidth+1,2*bandwidth+1);
  w.symsize(20);
  w.sym(pos.x,pos.y, ICLDrawWidget::symPlus);
  w.unlock();
  w.update();

  Thread::msleep(50);
}

  



int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input(2) -k(1) -size(1)",init,run).exec();
}
