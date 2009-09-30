#include <iclCommon.h>
#include <iclWarpOp.h>
#include <iclDynMatrix.h>

#include <QPushButton>
#include <iclLocalThresholdOp.h>
#include <iclRegionDetector.h>
#include <iclMorphologicalOp.h>
#include <iclCC.h>

#include "intrinsic-camera-calibration-tools.h"
#include <iclSOM2D.h>
#include <iclMathematics.h>
#include <QMessageBox>
#include <QFileDialog>

GUI gui("hsplit");
GenericGrabber *grabber = 0;

Img32f IMAGE;
Mutex MUTEX;

CalibrationData CALIB_DATA;
ImgQ WARP_MAP,MAN_WARP_MAP;
double DIST_FACTOR[4];
double MAN_DIST_FACTOR[4];

Point32f currPos;


void create_empty_warp_map(ImgQ &WARP_MAP){
  const Size &size = IMAGE.getSize();
  WARP_MAP.setSize(size);
  WARP_MAP.setChannels(2);

  Channel32f cs[2];
  WARP_MAP.extractChannels(cs);
  
  for(int x=0;x<size.width;++x){
    for(int y=0;y<size.height;++y){
      cs[0](x,y) = x;
      cs[1](x,y) = y;
    }
  }
}

inline Point32f distort_point(int xi, int yi, double *dist=DIST_FACTOR){
  const double &x0 = dist[0];
  const double &y0 = dist[1];
  const double &f = dist[2]/100000000.0;
  const double &s = dist[3];
  
  float x = s*(xi-x0);
  float y = s*(yi-y0);
  float p = 1 - f * (x*x + y*y);
  return Point32f(p*x + x0,p*y + y0);
}

void update_warp_map(ImgQ &WARP_MAP=::WARP_MAP, double *dist=DIST_FACTOR){
  const Size &size = IMAGE.getSize();
  WARP_MAP.setSize(size);
  Channel32f cs[2];
  WARP_MAP.extractChannels(cs);

  double *params = dist;
  DEBUG_LOG("creating warp map with params: " << params[0] << ","<< params[1] << ","<< params[2] << ","<< params[3] );
  
  for(float xi=0;xi<size.width;++xi){
    for(float yi=0;yi<size.height; ++yi){
      Point32f p = distort_point(xi,yi,dist);
      cs[0](xi,yi) = p.x; //p*x + x0; 
      cs[1](xi,yi) = p.y; //p*y + y0; 
    }
  }
}

void vis_som(SOM2D  &som, int gridW, int gridH){
  
  static DrawHandle &d = gui.getValue<DrawHandle>("image");
  static ICLDrawWidget &w = **d;
  ImgQ ps = zeros(gridW,gridH,2);
  
  w.lock();
  w.reset();
  w.color(255,0,0,255);
  for(int x=0;x<gridW;++x){
    for(int y=0;y<gridH;++y){
      float *p = som.getNeuron(x,y).prototype;
      ps(x,y,0) = p[0];
      ps(x,y,1) = p[1];
    }
  }
  for(int x=1;x<gridW;++x){
    for(int y=1;y<gridH;++y){
      w.line(ps(x,y,0),ps(x,y,1),ps(x-1,y,0),ps(x-1,y,1));
      w.line(ps(x,y,0),ps(x,y,1),ps(x,y-1,0),ps(x,y-1,1));
    }
  }
  for(int x=1;x<gridW;++x){
    w.line(ps(x,0,0),ps(x,0,1),ps(x-1,0,0),ps(x-1,0,1));
  }
  for(int y=1;y<gridH;++y){
    w.line(ps(0,y,0),ps(0,y,1),ps(0,y-1,0),ps(0,y-1,1));
  }
  w.unlock();
  d.update();
  Thread::msleep(1);
}

std::vector<Point32f> sort_points(const std::vector<Point32f> points, int gridW, int gridH, int imageW, int imageH){
  static std::vector<Range32f> initBounds(2,Range32f(0,1));
  static DrawHandle &d = gui.getValue<DrawHandle>("image");
  static ICLDrawWidget &w = **d;

  randomSeed();
  static const float M = 10;
  static const float E_start = 0.8;
  float mx = float(imageW-2*M)/(gridW-1);
  float my = float(imageH-2*M)/(gridH-1);
  float b = M;
  
  SOM2D som(2,gridW,gridH,initBounds,0.5,0.5);
  for(int x=0;x<gridW;++x){
    for(int y=0;y<gridH;++y){
      float *p = som.getNeuron(x,y).prototype;
      p[0] = mx * x + b;
      p[1] = my * y + b;
    }
  }

  URandI ridx(points.size()-1);
  float buf[2];

  std::vector<Point32f> sorted(points.size());  
  QMessageBox::StandardButton btn = QMessageBox::No;

  while(btn != QMessageBox::Yes){
    for(int j=0;j<100;++j){
      som.setEpsilon(E_start * ::exp(-j/30));
      for(int i=0;i<1000;++i){
        unsigned int ridxVal = ridx; 
        //      DEBUG_LOG("idx is " << ridxVal);
        const Point &rnd = points[ridxVal];
        buf[0] = rnd.x;
        buf[1] = rnd.y;
        som.train(buf);
      }
    }
    vis_som(som,gridW,gridH);

    w.lock();
    w.color(0,100,255,255);
    for(unsigned int i=0;i<points.size();++i){
      const Point &p = points[i];
      buf[0] = p.x;
      buf[1] = p.y;
      const float *g = som.getWinner(buf).gridpos;
      int x = ::round(g[0]);
      int y = ::round(g[1]);
      int idx = x + gridW * y;
      if(idx >=0 && idx < gridW*gridH){
        sorted.at(idx) = points.at(i);

        w.text(str(idx),p.x,p.y,-1,-1,12);
      }else{
        ERROR_LOG("could not sort point at index " << i);
      }
    }
    w.unlock();
    w.update();
    
    btn = QMessageBox::question(*gui.getValue<DrawHandle>("image"),
                                "please confirm ...","Is this tesselation correct?",
                                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if(btn == QMessageBox::Cancel) throw ICLException("cancel pressed");
  }
  return sorted;
}

void optimize_params(){
  Mutex::Locker l(MUTEX);
  
  if(gui.getValue<bool>("use-stochastic-opt")){
    calc_distortion_stochastic_search(CALIB_DATA,IMAGE.getWidth(),IMAGE.getHeight(),DIST_FACTOR);  
  }else{
    calc_distortion(CALIB_DATA,IMAGE.getWidth(),IMAGE.getHeight(),DIST_FACTOR);
  }
  
  std::cout << "distortion factors: [ " << DIST_FACTOR[0] << " " << DIST_FACTOR[1] 
            << " " << DIST_FACTOR[2] << "  " << DIST_FACTOR[3]  << " ]" << std::endl;

  update_warp_map();
}

void set_state(bool good){
  static ImgQ iGood(Size(75,25),formatRGB);
  static ImgQ iBad(Size(75,25),formatRGB);
  static bool first = true;
  if(first){
    first = false;
    color(0,255,0,255);
    fill(0,255,0,255);
    fontsize(12);
    text(iGood,20,0,"good");

    color(255,0,0,255);
    fill(255,0,0,255);
    text(iBad,2,0,"searching");
    
  }
  static ImageHandle &h = gui.getValue<ImageHandle>("state");
  h = good ? iGood : iBad;
  h.update();
}





void man_show_cb(){
  std::cout << "distortion factors:[ " << std::endl;
  std::cout << MAN_DIST_FACTOR[0] << " ";
  std::cout << MAN_DIST_FACTOR[1] << " ";
  std::cout << MAN_DIST_FACTOR[2] << " ";
  std::cout << MAN_DIST_FACTOR[3] << " ]" << std::endl;
}

void manual_adjust_cb(){
  Mutex::Locker l(MUTEX);
  gui_LabelHandle(manErr);
  
  gui_float(manScale);
  gui_float(manDist);
  
  MAN_DIST_FACTOR[0] = currPos.x;
  MAN_DIST_FACTOR[1] = currPos.y;
  MAN_DIST_FACTOR[2] = manDist;
  MAN_DIST_FACTOR[3] = manScale < 0 ? (1.0+manScale/2) : 1+manScale; 

  manErr = get_fitting_error(CALIB_DATA,MAN_DIST_FACTOR);
  
  update_warp_map(MAN_WARP_MAP, MAN_DIST_FACTOR);
}

void mouse(const MouseEvent &evt){
  if(evt.isPressEvent() || evt.isDragEvent()){
    currPos = evt.getPos();
  }
  manual_adjust_cb();
}

void manual_adjust(){
  Mutex::Locker l(MUTEX);
  static DrawHandle &d = gui.getValue<DrawHandle>("image");
  static ICLDrawWidget &w = **d;

  static WarpOp warp(MAN_WARP_MAP);
  warp.setWarpMap(MAN_WARP_MAP);
  d = warp.apply(&IMAGE);
  
  w.color(0,100,255,255);
  w.fill(0,100,255,100);
  w.rect(currPos.x-3,currPos.y-3,6,6);
  
  gui_bool(manShowGrid);
  
  static const int NX = 20;
  static const int NY = 15;
  static const float DX = IMAGE.getWidth()/NX;
  static const float DY = IMAGE.getHeight()/NY;
  
  if(manShowGrid){
    w.color(255,0,0,120);
    static std::vector<Point32f> grid;
    if(!grid.size()){
      grid.reserve( (NX+2)*(NY+2));
      for(int y=0;y<=(NY+1);++y){
        for(int x=0;x<=(NX+1);++x){
          grid.push_back(Point32f(x*DX,y*DY));
        }
      }
    }
    w.grid(grid.data(),NX+2,NY+2,true);
  }
}

void detect_vis(bool add=false){
  Mutex::Locker l(MUTEX);
  static DrawHandle &d = gui.getValue<DrawHandle>("image");
  static ICLDrawWidget &w = **d;

  static Img32f grayIm(IMAGE.getSize(),formatGray);
  cc(&IMAGE,&grayIm);
  
  static LocalThresholdOp lt(35,-10,0);
  static int &threshold = gui.getValue<int>("thresh");
  static int &maskSize = gui.getValue<int>("mask-size");
  lt.setGlobalThreshold(threshold);
  lt.setMaskSize(maskSize);
  
  static ImgBase *ltIm = 0;
  lt.apply(&grayIm,&ltIm);

  gui_bool(useMorph);

  static ImgBase *moIm = 0;
  static MorphologicalOp morph(MorphologicalOp::dilate3x3);
  morph.setClipToROI(false);
  morph.apply(ltIm,&moIm);
  

  static RegionDetector rd(100,50000,0,0);
  rd.setRestrictions(gui.getValue<int>("min-blob-size"),50000,0,0);
  const std::vector<icl::Region> &rs = rd.detect(useMorph ? moIm : ltIm);
  
  static std::string &vis = gui.getValue<std::string>("vis");

  if(vis == "color"){
    d  = IMAGE;  
  }else if(vis == "gray"){
    d = grayIm;
  }else if(vis == "thresh"){
    d = ltIm;
  }else if(vis == "morph"){
    d = moIm;
  }else if(vis == "warp"){
    static WarpOp warp(WARP_MAP);
    static ImgBase *waIm = 0;
    warp.apply(&IMAGE,&waIm);
    d = waIm;
  }else if(vis == "warp-field"){
    static Img8u bg(IMAGE.getSize(),1);
    d = bg;
    w.lock();
    w.color(255,0,0,200);
    for(int x=IMAGE.getSize().width-10;x>=0;x-=20){
      for(int y=IMAGE.getSize().height-10;y>=0;y-=20){
        Point32f p = distort_point(x,y);
        w.line(x,y,p.x,p.y);
        w.point(x,y);
      }
    }
    w.unlock();
  }else if(vis == "warp-map"){
    d = WARP_MAP;
  }

  if(!add){
    w.lock();
    w.color(255,0,0,200);
  }
  std::vector<Point32f> pts;
  for(unsigned int i=0;i<rs.size();++i){
    static float &minFormFactor = gui.getValue<float>("min-form-factor");
    bool warpX = (vis == "warp") || (vis == "warp-field") || (vis == "warp-map");
    if(rs[i].getFormFactor() < minFormFactor){
      if(!add && !warpX){
        w.linestrip(rs[i].getBoundary());
        w.text(str(rs[i].getFormFactor()),rs[i].getCOG().x,rs[i].getCOG().y,-1,-1,10);
        w.sym(rs[i].getCOG().x,rs[i].getCOG().y,ICLDrawWidget::symPlus);
      }
      pts.push_back(rs[i].getCOG());
    }
  }
  if(!add){
    w.unlock();
    set_state((int)pts.size() == CALIB_DATA.dim());
  }else{
    DEBUG_LOG("adding data ...");
    if((int)pts.size() != CALIB_DATA.dim()){
      ERROR_LOG("unable to add current state: some markers are missing\n"
                "found " << pts.size() << " searching for " << CALIB_DATA.nx << "x" << CALIB_DATA.ny << "=" << CALIB_DATA.dim());
    }else{

      CALIB_DATA.data.push_back(CalibrationStep());
      IMAGE.deepCopy(&CALIB_DATA.data.back().colorImage);
      moIm->convert(&CALIB_DATA.data.back().image);
      try{
        CALIB_DATA.data.back().points = sort_points(pts,
                                                    CALIB_DATA.nx,
                                                    CALIB_DATA.ny,
                                                    IMAGE.getWidth(),
                                                    IMAGE.getHeight());
      }catch(ICLException &ex){
        std::cout << "aborted ..." << std::endl;
        CALIB_DATA.data.pop_back();
      }
    }
  }
}


void add(void){
  detect_vis(true);
}

void save_warp_map(){
  static DrawHandle &d = gui.getValue<DrawHandle>("image");
  
  std::string defName = str("./warp-map-")+str(WARP_MAP.getSize())+
  "-"+str(DIST_FACTOR[0])+"-"+str(DIST_FACTOR[1])+"-"+str(DIST_FACTOR[2])+"-"+str(DIST_FACTOR[3])+".icl";
  QString name = QFileDialog::getSaveFileName(*d,"save warp-map ... ",defName.c_str(),
                                              "Float Images (*.icl *.pgm *.pnm)");
  if(name != ""){
    try{
      save(WARP_MAP,name.toLatin1().data());
    }catch(const std::exception &ex){
      ERROR_LOG("error while writing file ...");
    }
  }
}
void create_pattern_gui(){
  static ICLDrawWidget w;
  w.setGeometry(QRect(500,500,640,480));
  int W = CALIB_DATA.nx;
  int H = CALIB_DATA.ny;

  Img8u bg(Size(W+1,H+1),1);
  std::fill(bg.begin(0),bg.end(0),255);
  w.setImage(&bg);
  w.lock();
  w.reset();
  w.color(0,0,0,255);  
  w.fill(0,0,0,255);  
  
  float D = 0.4;
  for(int i=1;i<=W;++i){
    for(int j=1;j<=H;++j){
      w.ellipse(i,j,D,D);
    }
  }

  w.unlock();
  w.show();
}

void init(){


  CALIB_DATA.nx = pa_subarg<int>("-nx",0,5);
  CALIB_DATA.ny = pa_subarg<int>("-ny",0,4);

  grabber = new GenericGrabber(FROM_PROGARG_DEF("-input","pwc","0"));
  grabber->setIgnoreDesiredParams(true);
  grabber->grab()->convert(&IMAGE);

  create_empty_warp_map(WARP_MAP);
  create_empty_warp_map(MAN_WARP_MAP);

  
  if(pa_defined("-init")){
    for(int i=0;i<4;++i){
      DIST_FACTOR[0] = pa_subarg<float>("-init",i,0.0f);
      MAN_DIST_FACTOR[0] = pa_subarg<float>("-init",i,0.0f);
    }
    update_warp_map();
  }
  
  if(pa_defined("-cp")){
    create_pattern_gui();
  }
 
  
  gui << "draw[@minsize=32x24@handle=image]";
  GUI controls("vbox[@minsize=10x1]");
  controls << "image[@maxsize=100x2@minsize=5x2label=state@handle=state]";
  controls << "combo(!color,gray,thresh,morph,warp,warp-field,warp-map)[@out=vis@label=visualization]";
  controls << "togglebutton(off,on)[@out=grab-loop-val@handle=grab-loop@label=grab loop]";
  controls << "fslider(0.8,2.0,1.1)[@out=min-form-factor@label=roundness]";
  controls << "slider(10,10000,500)[@out=min-blob-size@label=min blob size]";
  controls << (GUI("vbox[@label=local threshold]") 
               << "slider(2,100,10)[@out=mask-size@label=mask size]"
               << ("slider("+str(pa_subarg<int>("-thresh-range",0,-20))+","+
                   str(pa_subarg<int>("-thresh-range",1,20))+",-10)[@out=thresh@label=threshold]") );
  controls << "togglebutton(!no,yes)[@out=useMorph@label=use morphed image]";
  controls << "button(add)[@handle=add]";
  controls << "togglebutton(no,yes)[@out=use-stochastic-opt@label=stochastic mode]";
  controls << "button(optimize)[@handle=optimize]";
  controls << "button(save)[@handle=save]";

  GUI manCont("vbox[@minsize=10x1]");
  manCont <<  ( GUI("hbox")
                 << "fslider(-100,1500,0,vertical)[@out=manDist@label=dist@handle=manDistH]"
                << "fslider(-1,1,0,vertical)[@out=manScale@label=scale@handle=manScaleH]"
               )
          << "label(---)[@maxsize=100x2@handle=manErr]"
          << "togglebutton(off,on)[@maxsize=100x2@out=manShowGrid@label=grid]"
          << "button(write)[@maxsize=100x2@handle=manWrite@handle=manWriteH]";

  
  gui << ( GUI("tab(auto,manual)[@handle=tab]") << controls << manCont );
  gui.show();

  gui.registerCallback(new GUI::Callback(manual_adjust_cb),"manDistH,manScaleH");
  gui.registerCallback(new GUI::Callback(man_show_cb),"manWriteH");
  gui["image"].install(new MouseHandler(mouse));
  
  
  //  gui.getValue<ButtonHandle>("detect").registerCallback(new GUI::Callback(detect));
  (*gui.getValue<ButtonHandle>("grab-loop"))->setChecked(true);

  //  (*gui.getValue<ImageHandle>("state"))->setFitMode(ICLWidget::fmFit);
  (*gui.getValue<ImageHandle>("state"))->setMenuEnabled(false);

  

  CALIB_DATA.data.reserve(10);

  gui.getValue<ButtonHandle>("add").registerCallback(new GUI::Callback(add));
  gui.getValue<ButtonHandle>("save").registerCallback(new GUI::Callback(save_warp_map));
  
}

void run(){

  static DrawHandle &d = gui.getValue<DrawHandle>("image");
  static ICLDrawWidget &w = **d;
  static bool &grab = gui.getValue<bool>("grab-loop-val");
  //static ButtonHandle &add = gui.getValue<ButtonHandle>("add");
  static ButtonHandle &opt = gui.getValue<ButtonHandle>("optimize");
  
  gui_TabHandle(tab);
  
  if(grab){
    Mutex::Locker l(MUTEX);
    grabber->grab()->convert(&IMAGE);
  }
  
  w.lock();
  w.reset();
  w.unlock();

  if((*tab)->currentIndex()  ==   0){
    detect_vis(false);
  }else{
    manual_adjust();
  }
  
  d.update();

  if(opt.wasTriggered()){
    DEBUG_LOG("optimizing ...");
    optimize_params();
  }
  
  Thread::msleep(20);
}


int main(int n, char **ppc){
  pa_explain("-nx","count of marker grid points in horizontal direction (5 by default)");
  pa_explain("-ny","count of marker grid points in horizontal direction (4 by default)");
  pa_explain("-input","define input device (e.g. -dc 0 or -file 'images/*.ppm'");
  pa_explain("-init","defined 4 initial values for distortion factors");
  pa_explain("-cp","create an extra widget that shows a calibration pattern");
  pa_explain("-thresh-range","define min and max value for local threshold operators slider for threshold adjustment");
  pa_init(n,ppc,"-nx(1) -ny(1) -input(2) -cp -init(4) -thresh-range(2)");
  
  ExecThread x(run);
  QApplication app(n,ppc);
  
  init();

  x.run();
  
  return app.exec();
}
