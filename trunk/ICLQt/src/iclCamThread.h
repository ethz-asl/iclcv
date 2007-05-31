#ifndef jfkdjflksdjfdsjfklsdjlkf
#define jfkdjflksdjfdsjfklsdjlkf

#include <iclWidget.h>
#include <iclImgBase.h>
#include <iclPWCGrabber.h>

using namespace icl;
using namespace std;
namespace icl{
class CamThread : public QObject{
  Q_OBJECT
    public:
  inline CamThread(int id, const Size &size=Size(320,240)):
    widget(new ICLWidget(0)),
    grabber(new PWCGrabber(size,30,id)),
    id(id){
    //grabber->setDesiredSize(size);
      widget->setGeometry(10,10,size.width,size.height);
    widget->show();
  }
  inline virtual ~CamThread(){
    delete grabber;
    delete widget;
  }
  
  public slots:
    
  void update(){
    widget->setImage(grabber->grab());
    widget->update();
  }
  
 private:
  ICLWidget *widget;
  PWCGrabber *grabber;
  int id;
};

}


#endif

