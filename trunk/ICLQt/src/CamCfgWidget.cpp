/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/CamCfgWidget.cpp                             **
** Module : ICLQt                                                  **
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

#include <ICLQt/CamCfgWidget.h>

#include <ICLIO/GenericGrabber.h>
#include <ICLQt/ContainerGUIComponents.h>
#include <ICLQt/GUIWidget.h>
#include <ICLQt/ComboHandle.h>
#include <ICLQt/BoxHandle.h>
#include <ICLQt/LabelHandle.h>
#include <ICLQt/CheckBoxHandle.h>
#include <ICLQt/ButtonHandle.h>
#include <ICLUtils/FPSLimiter.h>

#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QBoxLayout>
#include <QtGui/QScrollArea>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtGui/QPushButton>


using namespace icl::utils;
using namespace icl::core;
using namespace icl::io;

namespace icl{
  namespace qt{
    struct VolatileUpdater : public QTimer{
      std::string prop;
      GUI &gui;
      Grabber &grabber;
      VolatileUpdater(int msec, const std::string &prop, GUI &gui, Grabber &grabber):
        prop(prop),gui(gui),grabber(grabber){
        setInterval(msec);
      }
      virtual void timerEvent(QTimerEvent * e){
        LabelHandle &l = gui.get<LabelHandle>("#i#"+prop);
        (**l).setText(grabber.getValue(prop).c_str());
        (**l).update(); 
        QApplication::processEvents();
      }
    };
    
    class CamCfgWidget::Data{
    public:
      bool complex;
      std::string deviceFilter;
      GUI gui;
      GenericGrabber grabber;
      std::vector<GrabberDeviceDescription> foundDevices;
      bool scanScope;
      bool settingUpDevice;
      bool grabbing;
      bool loadParamsScope;
      
      
      Data(bool complex):complex(complex),mutex(QMutex::Recursive),fps(5),fpsLimiter(10,10){
        scanScope = false;
        settingUpDevice = false;
        grabbing = false;
        useFPSLimiter = false;
        end = false;
        loadParamsScope = false;
      }
      
      QScrollArea *scroll;
      GUI propGUI; // contains the dataStore ...
      std::vector<SmartPtr<VolatileUpdater> > timers;
      QMutex mutex;
      
      FPSEstimator fps;
      FPSLimiter fpsLimiter;
      bool useFPSLimiter;
      bool end;
    };
    
    CamCfgWidget::CamCfgWidget(const std::string &deviceFilter,QWidget *parent):
      QWidget(parent), data(new Data(true)){
      
      data->deviceFilter = deviceFilter;
      
      data->gui = HSplit(this);
      data->gui <<  ( VSplit()
                      << Image().handle("image").minSize(8,6).label("preview")
                      << ( VBox().minSize(18,15).maxSize(100,15)
                           << ( HBox().label("devices")
                                << Combo("no devices found").handle("device")
                                << Button("rescan").handle("scan").maxSize(3,8)
                              )
                           << ( HBox()
                                << Combo("no devices found").handle("format").label("format")
                                << Combo("no devices found").handle("size").label("size")
                              )
                           <<  ( HBox().label("control / FPS")
                                 << Button("capture!","stop").handle("capture").out("grabbing")
                                 << Combo("max 1Hz,max 5Hz,max 10Hz,max 15Hz,max 20Hz,max 25Hz,max 30Hz,max 50Hz,max 100Hz,max 120Hz,!no limit").handle("hz").maxSize(4,2)
                                 << Label("--.--").handle("fps")
                               )
                           <<  ( HBox().label("desired params")
                                 << Combo("default,QQVGA,QVGA,VGA,SVGA,XGA,XGAP,UXGA").handle("desired-size").label("size")
                                 << Combo("default,depth8u,depth16s,depth32s,depth32f,depth64f").handle("desired-depth").label("depth")
                                 << Combo("default,formatGray,formatRGB,formatHLS,formatYUV,formatLAB,formatChroma,formatMatrix").handle("desired-format").label("format")
                               )
                         )
                      )
                <<  ( VBox()
                      << VBox().handle("props").minSize(10,1).label("properties")
                      << ( HBox().maxSize(100,2)
                           << Button("load props").handle("load")
                           << Button("save props").handle("save")
                          )
                     );
      
  
      data->gui.create();
  
      setLayout(new QBoxLayout(QBoxLayout::LeftToRight,this));
      layout()->setContentsMargins(2,2,2,2);
      layout()->addWidget(data->gui.getRootWidget());
  
      data->gui.registerCallback(function(this,&icl::qt::CamCfgWidget::callback),"device,scan,format,size,capture,fps,load,save,"
                                 "desired-size,desired-depth,desired-format,hz");
      
      QWidget *w = (*data->gui.get<BoxHandle>("props"));
      w->layout()->setContentsMargins(0,0,0,0);
      w->layout()->addWidget(data->scroll = new QScrollArea(w));
      
      scan();
      start();
    }
  
  
    CamCfgWidget::CamCfgWidget(const std::string &devType, const std::string &devID,QWidget *parent):
      QWidget(parent),data(new Data(false)){
      // how to create the grabber
      
      std::string devText = "";
      bool needDeviceCombo = false;
      if(devType.length()){
        try{
          data->grabber.init(devType,devType+"="+devID);
          devText = devType + " " + devID;
        }catch(const std::exception &ex){
          QMessageBox::critical(0,"Unable to create CamCfgWidget!",
                                ex.what());
        }
      }else{
        data->foundDevices = GenericGrabber::getDeviceList("",false);
        if(!data->foundDevices.size()){
          QMessageBox::critical(0,"Unable to create CamCfgWidget!",
                                QString("no used devices available"));
        
        }else{
          if(data->foundDevices.size() == 1){
            devText = data->foundDevices.front().type + " " + data->foundDevices.front().id;
            data->grabber.init(data->foundDevices.front());
          }else{
            needDeviceCombo = true;
            devText = "";
            data->grabber.init(data->foundDevices.front());
          }
        }
      }
      data->gui = VBox(this);
      if(needDeviceCombo){
        data->gui << Combo("no devices found").handle("device").maxSize(100,2).label("available devices");
      }
      data->gui << VBox().handle("props").minSize(10,1).label("properties for device "+devText)
                << ( HBox().maxSize(100,2)
                     << Button("load props").handle("load")
                     << Button("save props").handle("save")
                   )
                << Create();
  
  
      if(needDeviceCombo){
        ComboHandle devices = data->gui.get<ComboHandle>("device");
        devices.clear();
        for(unsigned int i=0;i<data->foundDevices.size();++i){
          devices.add("[" + data->foundDevices[i].type + "] ID:" + data->foundDevices[i].id);
        }
      }
      
      setLayout(new QBoxLayout(QBoxLayout::LeftToRight,this));
      layout()->setContentsMargins(2,2,2,2);
      layout()->addWidget(data->gui.getRootWidget());
  
      if(needDeviceCombo){
        data->gui.registerCallback(function(this,&icl::qt::CamCfgWidget::callback),"load,save,device");
      }else{
        data->gui.registerCallback(function(this,&icl::qt::CamCfgWidget::callback),"load,save");
      }
      
      QWidget *w = (*data->gui.get<BoxHandle>("props"));
      w->layout()->setContentsMargins(0,0,0,0);
      w->layout()->addWidget(data->scroll = new QScrollArea(w));
  
      callback("device");
    }
  
    const ImgBase *CamCfgWidget::getCurrentImage(){
      QMutexLocker __lock(&data->mutex); 
      if(data->grabber.isNull()) return 0;
      
      const ImgBase *image = data->grabber.grab();
      
      if(data->complex){
        data->gui["image"] = image;
        data->gui["fps"] = data->fps.getFPSString();
      }
  
      return image;
    }
  
    
    CamCfgWidget::~CamCfgWidget(){
      if(data->complex){
        data->end = true;
        Thread::stop();
      }
      delete data;
    }
    
    void CamCfgWidget::setVisible (bool visible){
      if(!visible){
        data->grabbing = false;
      }
      QWidget::setVisible(visible);
    }
    
    static std::string strip(const std::string &s, char a, char b){
      int begin = 0;
      int len = s.length();
      if(s[0] == a) {
        ++begin;
        --len;
      }
      if(s[s.length()-1] == b){
        --len;
      }
      return s.substr(begin,len);
    }
  
    std::string get_combo_content_str(const std::string &p, Grabber &grabber){
      std::string currVal = grabber.getValue(p);
      std::ostringstream s;
      std::vector<std::string> ts = tok(strip(grabber.getInfo(p),'{','}'),"\",\"");
      for(unsigned int i=0;i<ts.size();++i){
        std::string stripped = strip(ts[i],'"','"');
        if(currVal == stripped) s << "!";
        s << stripped;
        if(i < ts.size()-1) s << ',';
      }
      return s.str();
    }
    
    const std::string remove_commas(std::string s){
      for(unsigned int i=0;i<s.length();++i){
        if(s[i] == ',') s[i] = ' ';
      }
      return s;
    }
    
    static void create_property_gui(GUI &gui,GenericGrabber &grabber,const GUI::ComplexCallback &cb,  
                                    std::vector<SmartPtr<VolatileUpdater> > &timers){
      gui = VBox();
      std::ostringstream ostr;
      std::vector<std::string> propertyList = grabber.getPropertyList();
      
      
      for(unsigned int i=0;i<propertyList.size();++i){
        const std::string &p = propertyList[i];
        const std::string pp = remove_commas(p);
        if(p == "size" || p == "format") continue; // these two are handled elsewhere
        
        std::string pt = grabber.getType(p);
  
        //      std::cout << "property " << i << " is " << p << " (type is " << pt << ")" << std::endl; 
        
        if(pt == "range"){
          // todo check stepping ...
          std::string handle="#r#"+p;
          SteppingRange<float> r = parse<SteppingRange<float> >(grabber.getInfo(p));
          std::string c = grabber.getValue(p);
          gui << FSlider(r,parse<float>(c)).handle(handle).minSize(12,2).label(pp);
          ostr << '\1' << handle;
        }else if(pt == "menu" || pt == "value-list" || pt == "valueList"){
          std::string handle = (pt == "menu" ? "#m#" : "#v#")+p;
          gui << Combo(get_combo_content_str(p,grabber)).handle(handle).minSize(12,2).label(pp);
          ostr << '\1' << handle;
        }else if(pt == "command"){
          std::string handle = "#c#"+p;
          ostr << '\1' << handle;
          gui << Button(pp).handle(handle).minSize(12,2);
        }else if(pt == "info"){
          std::string handle = "#i#"+p;
          ostr << '\1' << handle;
          gui << Label("unknown").handle(handle).minSize(12,2).label(p);
          int volatileness = grabber.isVolatile(p);
          if(volatileness){
            timers.push_back(new VolatileUpdater(volatileness,p,gui,grabber));
          }
        }else{
          ERROR_LOG("unable to create GUI-component for property \"" << p << "\" (unsupported property type: \"" + pt+ "\")");
        }
      }
    
      gui.show();
      
      std::string cblist = ostr.str();
      if(cblist.size() > 1){
        gui.registerCallback(cb,cblist.substr(1),'\1');
      }
      for(unsigned int i=0;i<timers.size();++i){
        timers[i]->start();
      }
    }
    
    static void process_callback(char t, const std::string &property, GenericGrabber &grabber, GUI &gui){
      switch(t){
        case 'r': 
        case 'm': 
        case 'v': 
          grabber.setProperty(property,gui[str("#")+t+"#"+property]);
          break;
        case 'c': 
          grabber.setProperty(property,"");
          break;
        default:
          ERROR_LOG("invalid callback ID type char: \"" << t << "\"");
      }
    }
    
    void CamCfgWidget::callback(const std::string &source){
      QMutexLocker __lock(&data->mutex);
  
      if(source.length()>3 && source[0] == '#'){
        process_callback(source[1],source.substr(3),data->grabber,data->propGUI);
      }else if(source == "scan"){
        scan();
      }else if(data->scanScope || data->settingUpDevice){
        return;
      }else if(source == "device"){
        data->settingUpDevice = true;
        QWidget *w = data->scroll->widget();
        
        if(w){
          w->setVisible(false);
          data->scroll->setWidget(0);
          for(unsigned int i=0;i<data->timers.size();++i){
            data->timers[i]->stop();
          }
          data->timers.clear();
          delete w;
        }
  
        if(data->complex){
          ComboHandle &fmt = data->gui.get<ComboHandle>("format");
          ComboHandle &siz = data->gui.get<ComboHandle>("size");
          fmt.clear();
          siz.clear();
        }
        
        try{
          if(data->complex || data->foundDevices.size() > 1){
            if(!data->loadParamsScope){
              data->grabber.init(data->foundDevices.at((int)data->gui["device"]));
              if(data->grabber.isNull()){
                ERROR_LOG("unable to initialize grabber");
              }
            }
            //if(data->complex){
            //  data->grabber.setUseDesiredParams(data->gui.getValue<CheckBoxHandle>("desired-use").isChecked());
            //}
            if(data->complex){
              
            }
          }
          
          create_property_gui(data->propGUI,data->grabber,function(this,&icl::qt::CamCfgWidget::callback), data->timers);
          data->scroll->setWidget(data->propGUI.getRootWidget());
  
          if(data->complex){
            ComboHandle &fmt = data->gui.get<ComboHandle>("format");
            ComboHandle &siz = data->gui.get<ComboHandle>("size");
          
            std::vector<std::string> fmts = tok(strip(data->grabber.getInfo("format"),'{','}'),"\",",false);
            std::vector<std::string> sizes = tok(strip(data->grabber.getInfo("size"),'{','}'),",");
            
            for(unsigned int i=0;i<fmts.size();++i){
              fmt.add(strip(fmts[i],'"','"'));
            }
            
            for(unsigned int i=0;i<sizes.size();++i){
              siz.add(strip(sizes[i],'"','"'));
            }
            
            fmt.setSelectedItem(data->grabber.getValue("format"));
            siz.setSelectedItem(data->grabber.getValue("size"));
          }
          
        }catch(const std::exception &x){
          ComboHandle &fmt = data->gui.get<ComboHandle>("format");
          ComboHandle &siz = data->gui.get<ComboHandle>("size");
  
          fmt.add("no devices found");
          siz.add("no devices found");
        }
        
        // fill format and size combos
        data->settingUpDevice = false;
        
        if(data->complex){
          callback("desired-format");
          callback("desired-size");
          callback("desired-depth");
        }
      }else if(source == "hz"){
        std::string v = data->gui["hz"];
        if(v.length() < 5) {
          ERROR_LOG("invalid FPS-value [this should not happen]");
        }else{
          if(v == "no limit"){
            data->useFPSLimiter = false;
          }else{
            v = v.substr(4,v.length()-6);
            data->useFPSLimiter = true;
            data->fpsLimiter.setMaxFPS(parse<int>(v));
          }
        }
      }else if(source == "format" || source == "size"){
        if(data->grabber.isNull()) return;
        data->grabber.setProperty(source,data->gui[source]);
        //}else if(source == "desired-use"){
       // if(data->grabber.isNull()) return;
       // data->grabber.setUseDesiredParams(data->gui.getValue<CheckBoxHandle>(source).isChecked());
      }else if(source == "desired-format"){
        if(data->grabber.isNull()) return;
        std::string fmt = data->gui[source];
        if(fmt == "default"){
          data->grabber.ignoreDesired<format>();
        }else{
          data->grabber.useDesired(parse<format>(fmt));
        }
      }else if(source == "desired-size"){
        std::string size = data->gui[source];
        if(size == "default"){
          data->grabber.ignoreDesired<Size>();
        }else{
          data->grabber.useDesired(parse<Size>(size));
        }
      }else if(source == "desired-depth"){
        std::string d = data->gui[source];
        if(d == "default"){
          data->grabber.ignoreDesired<core::depth>();
        }else{
          data->grabber.useDesired(utils::parse<core::depth>(d));
        }
      }else if(source == "save"){
        if(data->grabber.isNull()){
          QMessageBox::information(this,"No device selected!","You can only save properties if a device is selected");
          return;
        }
        QString s = QFileDialog::getSaveFileName(this,"Save device properties ...","","XML-files (*.xml)");
        if(s.isNull() || s=="")return;
        try{
          data->grabber.saveProperties(s.toLatin1().data(),false);
        }catch(ICLException &e){
          ERROR_LOG(e.what());
        }
      }else if(source == "load"){
        if(data->grabber.isNull()){
          QMessageBox::information(this,"No device selected!","You can only load properties if a device is selected");
          return;
        }
        QString s = QFileDialog::getOpenFileName(this,"Load device properties ...","","XML-files (*.xml)");
        if(s.isNull() || s == "") return;
        data->grabber.loadProperties(s.toLatin1().data(),false);
        data->loadParamsScope = true;
        callback("device");
        data->loadParamsScope = false;
      }
    }
  
    void CamCfgWidget::scan(){
      data->scanScope = true;
      
      ComboHandle &devices = data->gui.get<ComboHandle>("device");
      devices.clear();
      data->foundDevices = GenericGrabber::getDeviceList(data->deviceFilter);
      if(data->foundDevices.size()){
        for(unsigned int i=0;i<data->foundDevices.size();++i){
          devices.add("[" + data->foundDevices[i].type + "]:" + data->foundDevices[i].description);
        }
      }else{
        devices.add("no devices found");
      }
      data->scanScope = false;
      callback("device");
    }
  
    void CamCfgWidget::run(){
      bool &b = data->gui.get<bool>("grabbing");
      data->mutex.lock();
      while(1){
        if(data->end){
          break;
        }
        while(!b){
          data->gui["fps"] = str("--.--");
          data->grabbing = false;
          data->mutex.unlock();
          Thread::msleep(100);
          data->mutex.lock();
        }
        data->grabbing = true;
  
        if(!data->grabber.isNull()){
          data->gui["image"] = data->grabber.grab();
          data->gui["fps"] = data->fps.getFPSString();
          data->mutex.unlock();
          if(data->useFPSLimiter){
            data->fpsLimiter.wait();
          }
          Thread::msleep(0);
          data->mutex.lock();
        }else{
          data->mutex.unlock();
          Thread::msleep(100);
          data->mutex.lock();
        }
        
      }
    }
  } // namespace qt
}


