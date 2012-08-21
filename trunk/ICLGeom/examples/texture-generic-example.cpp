#include <ICLQuick/Common.h>
#include <ICLGeom/Geom.h>

GenericGrabber  grabber;
Scene scene;
GUI gui;
const ImgBase *image = 0;

struct Obj : public SceneObject{
  Obj(){
    Point32f ts[100];
    int idx[100];
    const float ar = float(image->getWidth()) / float(image->getHeight());
    for(int i=0;i<100;++i){
      float angle = (float(i)/100) * 2 *M_PI;
      addVertex(Vec(cos(angle)*5*ar, sin(angle)*5, -2, 1));
      addNormal(Vec(0,0,1,1));
      ts[i] = Point32f(cos(angle)/2+0.5, sin(angle)/2+0.5);
      idx[i] = i;
    }
    bool createTextureOnce = false; // just to make this more explicit
    addTexture(image,100,idx, ts, idx, createTextureOnce);
  }

#if 0  
  // manual version using a GLImg directly
  virtual void customRender(){
    static Img8u lena = create<icl8u>("lena");
    static GLImg image(&lena);
    Vec ps[100];
    Point32f ts[100];
    for(int i=0;i<100;++i){
      float angle = (float(i)/100) * 2 *M_PI;
      ps[i] = Vec(cos(angle)*5, sin(angle)*5, -2, 1);
      ts[i] = Point32f(cos(angle)/2+0.5, sin(angle)/2+0.5);
    }
    image.draw3DGeneric(100, &ps[0][0], &ps[0][1], &ps[0][2],4, ts);
  }
#endif

} * obj = 0;

void init(){

  grabber.init(pa("-i"));
  image = grabber.grab();
  scene.addCamera(Camera());

  obj = new Obj;  
  scene.addObject(obj);
  scene.addObject(SceneObject::cube(0,0,0,100));
  
  gui << Draw3D().handle("draw") << Show();
  
  gui["draw"].link(scene.getGLCallback(0));
  gui["draw"].install(scene.getMouseHandler(0));
}

void run(){
  image = grabber.grab();
  gui["draw"].render();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input|-i(2)",init,run).exec();
}
