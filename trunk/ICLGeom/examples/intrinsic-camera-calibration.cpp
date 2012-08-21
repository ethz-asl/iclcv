/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/intrinsic-camera-calibration.cpp      **
** Module : ICLGeom                                                **
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

#include <ICLQuick/Common.h>
#include <ICLFilter/WarpOp.h>
#include <ICLUtils/DynMatrix.h>


#include <ICLFilter/LocalThresholdOp.h>
#include <ICLFilter/MorphologicalOp.h>
#include <ICLBlob/RegionDetector.h>
#include <ICLCC/CCFunctions.h>

#include "intrinsic-camera-calibration-tools.h"
#include <ICLAlgorithms/SOM2D.h>
#include <ICLCore/Mathematics.h>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QPushButton>

HSplit gui;
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
  
  static DrawHandle &d = gui.get<DrawHandle>("image");
  static ICLDrawWidget &w = **d;
  ImgQ ps = zeros(gridW,gridH,2);
  
  w.color(255,0,0,255);
  for(int x=0;x<gridW;++x){
    for(int y=0;y<gridH;++y){
      const float *p = som.getNeuron(x,y).prototype.get();
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
  d.render();
  Thread::msleep(1);
}

std::vector<Point32f> sort_points(const std::vector<Point32f> points, int gridW, int gridH, int imageW, int imageH){
  static std::vector<Range32f> initBounds(2,Range32f(0,1));
  static DrawHandle &d = gui.get<DrawHandle>("image");
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
      float *p = const_cast<float*>(som.getNeuron(x,y).prototype.get());
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

    w.color(0,100,255,255);
    for(unsigned int i=0;i<points.size();++i){
      const Point &p = points[i];
      buf[0] = p.x;
      buf[1] = p.y;
      const float *g = som.getWinner(buf).gridpos.get();
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
    w.render();
    
    btn = QMessageBox::question(*gui.get<DrawHandle>("image"),
                                "please confirm ...","Is this tesselation correct?",
                                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if(btn == QMessageBox::Cancel) throw ICLException("cancel pressed");
  }
  return sorted;
}

void optimize_params(){
  Mutex::Locker l(MUTEX);
  
  if(gui.get<bool>("use-stochastic-opt")){
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
  static ImageHandle &h = gui.get<ImageHandle>("state");
  h = good ? iGood : iBad;
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
  static LabelHandle manErr = gui["manErr"];
  
  float manScale = gui["manScale"];
  float manDist = gui["manDist"];
  
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
  static DrawHandle &d = gui.get<DrawHandle>("image");
  static ICLDrawWidget &w = **d;

  static WarpOp warp(MAN_WARP_MAP);
  warp.setWarpMap(MAN_WARP_MAP);
  d = warp.apply(&IMAGE);
  
  w.color(0,100,255,255);
  w.fill(0,100,255,100);
  w.rect(currPos.x-3,currPos.y-3,6,6);

  bool manShowGrid = gui["manShowGrid"];
  
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
  static DrawHandle &d = gui.get<DrawHandle>("image");
  static ICLDrawWidget &w = **d;

  static Img32f grayIm(IMAGE.getSize(),formatGray);
  cc(&IMAGE,&grayIm);
  
  static LocalThresholdOp lt(35,-10,0);
  static int &threshold = gui.get<int>("thresh");
  static int &maskSize = gui.get<int>("mask-size");
  lt.setGlobalThreshold(threshold);
  lt.setMaskSize(maskSize);
  
  static ImgBase *ltIm = 0;
  lt.apply(&grayIm,&ltIm);

  bool useMorph = gui["useMorph"];

  static ImgBase *moIm = 0;
  static MorphologicalOp morph(MorphologicalOp::dilate3x3);
  morph.setClipToROI(false);
  morph.apply(ltIm,&moIm);
  

  static RegionDetector rd(100,50000,0,0);
  rd.setConstraints(gui.get<int>("min-blob-size"),50000,0,0);
  const std::vector<ImageRegion> &rs = rd.detect(useMorph ? moIm : ltIm);
  
  std::string vis = gui["vis"];

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
    w.color(255,0,0,200);
    for(int x=IMAGE.getSize().width-10;x>=0;x-=20){
      for(int y=IMAGE.getSize().height-10;y>=0;y-=20){
        Point32f p = distort_point(x,y);
        w.line(x,y,p.x,p.y);
        w.point(x,y);
      }
    }
  }else if(vis == "warp-map"){
    d = WARP_MAP;
  }

  if(!add){
    w.color(255,0,0,200);
  }
  std::vector<Point32f> pts;
  for(unsigned int i=0;i<rs.size();++i){
    static float &minFormFactor = gui.get<float>("min-form-factor");
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
  static DrawHandle &d = gui.get<DrawHandle>("image");
  
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
  w.color(0,0,0,255);  
  w.fill(0,0,0,255);  
  
  float D = 0.4;
  for(int i=1;i<=W;++i){
    for(int j=1;j<=H;++j){
      w.ellipse(i,j,D,D);
    }
  }

  w.show();
}

void init(){


  CALIB_DATA.nx = pa("-nx",0);
  CALIB_DATA.ny = pa("-ny",0);

  grabber = new GenericGrabber(pa("-input"));
  grabber->grab()->convert(&IMAGE);

  create_empty_warp_map(WARP_MAP);
  create_empty_warp_map(MAN_WARP_MAP);

  
  if(pa("-init")){
    for(int i=0;i<4;++i){
      MAN_DIST_FACTOR[i] = DIST_FACTOR[i] = pa("-init",i);
    }
    update_warp_map();
  }
  
  if(pa("-cp")){
    create_pattern_gui();
  }

  
  gui << Draw().minSize(32,24).handle("image");
  
  GUI controls = VBox().minSize(10,1);
  controls << Image().maxSize(100,2).minSize(5,2).label("state").handle("state")
           << Combo("color,gray,thresh,morph,warp,warp-field,warp-map").handle("vis").label("visualization")
           << Button("off","on").out("grab-loop-val").handle("grab-loop").label("grab loop")
           << FSlider(0.8,2.0,1.1).out("min-form-factor").label("roundness")
           << Slider(10,10000,500).out("min-blob-size").label("min blob size")
           << (VBox().label("local threshold")
               << Slider(2,100,10).out("mask-size").label("mask size")
               << Slider(pa("-t",0),pa("-t",1),-10).out("thresh").label("threshold")
              )
           << Button("no","yes").out("useMorph").label("use morphed image")
           << Button("add").handle("add")
           << Button("no","yes").out("use-stochastic-opt").label("stochastic mode")
           << Button("optimize").handle("optimize")
           << Button("save").handle("save");
  
  GUI manCont = VBox().minSize(10,1);
  manCont <<  ( HBox()
                << FSlider(-100,1500,0,true).out("manDist").label("dist").handle("manDistH")
                << FSlider(-1,1,0,true).out("manScale").label("scale").handle("manScaleH")
                )
          << Label("---").maxSize(100,2).handle("manErr")
          << Button("off","on").maxSize(100,2).out("manShowGrid").label("grid")
          << Button("write").maxSize(100,2).handle("manWrite").handle("manWriteH");

  gui << ( Tab("auto,manual").handle("tab") 
           << controls 
           << manCont 
           )
      << Show();

  gui.registerCallback(manual_adjust_cb,"manDistH,manScaleH");
  gui.registerCallback(man_show_cb,"manWriteH");
  gui["image"].install(mouse);
  
  
  gui.get<ButtonHandle>("grab-loop")->setChecked(true);
  gui.get<ImageHandle>("state")->setMenuEnabled(false);

  CALIB_DATA.data.reserve(10);

  gui["add"].registerCallback(add);
  gui["save"].registerCallback(save_warp_map);
  
}

void run(){

  static DrawHandle &d = gui.get<DrawHandle>("image");
  static ICLDrawWidget &w = **d;
  static bool &grab = gui.get<bool>("grab-loop-val");
  //static ButtonHandle &add = gui.get<ButtonHandle>("add");
  static ButtonHandle &opt = gui.get<ButtonHandle>("optimize");
  
  static TabHandle tab=gui["tab"];
  
  if(grab){
    Mutex::Locker l(MUTEX);
    grabber->grab()->convert(&IMAGE);
  }
  
  w.resetQueue();

  if((*tab)->currentIndex()  ==   0){
    detect_vis(false);
  }else{
    manual_adjust();
  }
  
  d.render();

  if(opt.wasTriggered()){
    DEBUG_LOG("optimizing ...");
    optimize_params();
  }
  
  Thread::msleep(20);
}


int main(int n, char **ppc){
  paex
  ("-nx","count of marker grid points in horizontal direction")
  ("-ny","count of marker grid points in horizontal direction ")
  ("-input","define input device (e.g. -dc 0 or -file 'images/*.ppm'")
  ("-init","defined 4 initial values for distortion factors")
  ("-cp","create an extra widget that shows a calibration pattern")
  ("-thresh-range","define min and max value for\n"
   "local threshold operators slider for threshold adjustment");
  return ICLApp(n,ppc,"-nx(int=5) -ny(int=4) [m]-input|-i(device,device-params) "
                "-cp -init(float=0,float=0,float=0,float=0) "
                "-thresh-range|-t(min=-20,max=20)",init,run).exec();
}
