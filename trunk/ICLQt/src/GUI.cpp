/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/GUI.cpp                                      **
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

#include <ICLUtils/StrTok.h>
#include <ICLUtils/SteppingRange.h>
#include <ICLUtils/Size.h>
#include <ICLCore/CoreFunctions.h>

#include <ICLQt/GUI.h>
#include <ICLQt/GUIWidget.h>
#include <ICLQt/GUIDefinition.h>
#include <ICLQt/GUISyntaxErrorException.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Array2D.h>
#include <ICLQt/Widget.h>
#include <ICLIO/File.h>

#include <QtGui/QColorDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QRadioButton>
#include <QtGui/QWidget>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QSlider>
#include <QtGui/QLCDNumber>
#include <QtGui/QLineEdit>
#include <QtGui/QIntValidator>
#include <QtGui/QDoubleValidator>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QTabBar>
#include <QtGui/QMainWindow>
#include <QtGui/QDockWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QApplication>
#include <QtGui/QSplitter>
#include <QtGui/QScrollArea>

#include <ICLQt/ProxyLayout.h>

#include <ICLQt/ButtonHandle.h>
#include <ICLQt/BoxHandle.h>
#include <ICLQt/TabHandle.h>
#include <ICLQt/BorderHandle.h>
#include <ICLQt/ButtonGroupHandle.h>
#include <ICLQt/LabelHandle.h>
#include <ICLQt/StateHandle.h>
#include <ICLQt/SliderHandle.h>
#include <ICLQt/FSliderHandle.h>
#include <ICLQt/IntHandle.h>
#include <ICLQt/FloatHandle.h>
#include <ICLQt/StringHandle.h>
#include <ICLQt/ComboHandle.h>
#include <ICLQt/SpinnerHandle.h>
#include <ICLQt/ImageHandle.h>
#include <ICLQt/DrawHandle.h>
#include <ICLQt/DrawHandle3D.h>
#include <ICLQt/DispHandle.h>
#include <ICLQt/FPSHandle.h>
#include <ICLQt/CheckBoxHandle.h>
#include <ICLQt/MultiDrawHandle.h>
#include <ICLQt/SplitterHandle.h>
#include <ICLQt/ColorHandle.h>
#include <QtGui/QCheckBox>
#include <QtGui/QCleanlooksStyle>
#include <QtCore/QTimer>

#include <QtGui/QFileDialog>

#include <ICLQt/ConfigFileGUI.h>
#include <ICLQt/CamCfgWidget.h>
#include <ICLUtils/StringUtils.h>
#include <ICLQt/ToggleButton.h>

#include <ICLQt/Widget.h>
#include <ICLQt/DrawWidget.h>
#ifdef HAVE_OPENGL
#include <ICLQt/DrawWidget3D.h>
#endif
#include <ICLQt/ThreadedUpdatableSlider.h>
#include <ICLQt/ThreadedUpdatableTextView.h>
#include <ICLQt/ColorLabel.h>
#include <ICLUtils/Configurable.h>
#include <ICLCC/Color.h>

#include <map>
#include <set>

using namespace std;
using namespace icl;

namespace icl{
  namespace{
    struct VolatileUpdater : public QTimer{
      std::string prop;
      GUI &gui;
      Configurable &conf;
      LabelHandle *l;
      VolatileUpdater(int msec, const std::string &prop, GUI &gui, Configurable &conf):
        prop(prop),gui(gui),conf(conf),l(0){
        setInterval(msec);
      }
      virtual void timerEvent(QTimerEvent * e){
        if(!l){
          l = &gui.getValue<LabelHandle>("#i#"+prop);
        }
        (***l).setText(conf.getPropertyValue(prop).c_str());
        (***l).update(); 
        QApplication::processEvents();
      }
    };
  }
  
  static const std::string &gen_params(){
    // {{{ open

    static std::string op = 
    string("general params are: \n")+
    string("\t@size=WxH     (W and H are positive integers) set min and max size of that widget\n")+
    string("\t@minsize=WxH  (W and H are positive integers) set min. size of that widget\n")+
    string("\t@maxsize=WxH  (W and H are positive integers) set max. size of that widget\n")+
    string("\t@handle=NAME  if defined, the componets handle is allocated with id NAME\n")+
    string("\t              (all size parameters are defined in cells of 15x15 pixles)\n")+
    string("\t@label=L      L is the label of this component\n")+
    string("\t@out=LIST     LIST is a comma-seperated list of output names\n")+
    string("\t@inp=LIST     LIST is a comma-seperated list of output names\n");
    return op;
  }

  // }}}

  // quite complex component for embedded property component 'prop'
  struct ConfigurableGUIWidget : public GUIWidget{

    std::vector<SmartPtr<VolatileUpdater> > timers;
    Configurable *conf;
    GUI gui;
    bool deactivateExec;

    struct StSt{
      std::string full,half;
      StSt(const std::string &full, const std::string &half):full(full),half(half){}
    };
    
    void update_all_components(){
      std::vector<std::string> props = conf->getPropertyListWithoutDeactivated();
      for(unsigned int i=0;i<props.size();++i){
        const std::string &p = props[i];
        std::string t = conf->getPropertyType(p);
        if(t == "range" || t == "range:slider"){
          gui.getValue<FSliderHandle>("#r#"+p).setValue( parse<icl32f>(conf->getPropertyValue(p)) );
        }else if( t == "range:spinbox"){
          gui.getValue<SpinnerHandle>("#R#"+p).setValue( parse<icl32s>(conf->getPropertyValue(p)) );
        }else if( t == "menu" || t == "value-list" || t == "valueList"){
          std::string handle = (t == "menu" ? "#m#" : "#v#")+p;
          gui.getValue<ComboHandle>(handle).setSelectedItem(conf->getPropertyValue(p));
        }else if( t == "info"){
          gui["#i#"+p] = conf->getPropertyValue(p);
        }else if( t == "flag"){
          gui["#f#"+p] = conf->getPropertyValue(p).as<bool>();
        }
      }
    }


    
    void add_component(GUI &gui,const StSt &p, std::ostringstream &ostr, GUI &timerGUI){
      std::string t = conf->getPropertyType(p.full);
      if(t == "range" || t == "range:slider"){
        // todo check stepping ...
        std::string handle="#r#"+p.full;
        SteppingRange<float> r = parse<SteppingRange<float> >(conf->getPropertyInfo(p.full));
        std::string c = conf->getPropertyValue(p.full);
        gui << "fslider("+str(r.minVal)+","+str(r.maxVal)+","+c+")[@handle="+handle+"@minsize=12x2@label="+p.half+"]";
        ostr << '\1' << handle;
      }else if( t == "range:spinbox"){
        std::string handle="#R#"+p.full;
        Range32s r = parse<Range32s>(conf->getPropertyInfo(p.full));
        std::string c = conf->getPropertyValue(p.full);
        gui << "spinner("+str(r.minVal)+","+str(r.maxVal)+","+c+")[@handle="+handle+"@minsize=12x2@label="+p.half+"]";
        ostr << '\1' << handle;
      }else if(t == "menu" || t == "value-list" || t == "valueList"){
        std::string handle = (t == "menu" ? "#m#" : "#v#")+p.full;
        gui << "combo("+conf->getPropertyInfo(p.full)+")[@handle="+handle+"@minsize=12x2@label="+p.half+"]";
        ostr << '\1' << handle;
      }else if(t == "command"){
        std::string handle = "#c#"+p.full;
        ostr << '\1' << handle;
        gui << "button("+p.half+")[@handle="+handle+"@minsize=12x2]";
      }else if(t == "info"){
        std::string handle = "#i#"+p.full;
        ostr << '\1' << handle;
        gui << "label("+conf->getPropertyValue(p.full)+")[@handle="+handle+"@minsize=12x2@label="+p.half+"]";
        int volatileness = conf->getPropertyVolatileness(p.full);
        if(volatileness){
          timers.push_back(new VolatileUpdater(volatileness,p.full,timerGUI,*conf));
        } 
      }else if(t == "flag"){
        std::string handle = "#f#"+p.full;
        ostr << '\1' << handle;
        gui << "checkbox("+p.half+","+(conf->getPropertyValue(p.full).as<bool>() ? "checked" : "unchecked") +")[@handle="+handle+"@minsize=12x1]";
      }else{
        ERROR_LOG("unable to create GUI-component for property \"" << p.full << "\" (unsupported property type: \"" + t+ "\")");
      }
     }

    ConfigurableGUIWidget(const GUIDefinition &def):GUIWidget(def,1,1),deactivateExec(false){
      conf = Configurable::get(def.param(0));
      if(!conf) throw GUISyntaxErrorException(def.defString(),"No Configurable with ID "+def.param(0)+" registered");
      
      std::vector<std::string> props = conf->getPropertyListWithoutDeactivated();
      std::map<std::string,std::vector<StSt> > sections;

      for(unsigned int i=0;i<props.size();++i){
        const std::string &p = props[i];
        unsigned int pos = p.find('.');
        if(pos == std::string::npos){
          sections["general"].push_back(StSt(p,p));
        }else{
          sections[p.substr(0,pos)].push_back(StSt(p,p.substr(pos+1))); //
        }
      }
      std::string tablist;
      
      int generalIdx = 0;
      int i=0;
      for(std::map<std::string,std::vector<StSt> >::iterator it=sections.begin();it != sections.end();++it){
        if(it->first == "general") {
          generalIdx = i;
        }
        tablist += (tablist.length()?",":"")+it->first;
        ++i;
      }
      
      gui = GUI("tab("+tablist+")[@handle=__the_tab__]",this);

      std::ostringstream ostr;
      for(std::map<std::string,std::vector<StSt> >::iterator it=sections.begin();it != sections.end();++it){
        GUI tab("vscroll");
        for(unsigned int i=0;i<it->second.size();++i){
          add_component(tab,it->second[i],ostr,gui);          
        }
        if(it->first == "general"){
          tab << ( GUI("hbox")
                    << "button(load)[@handle=#X#load]"
                    << "button(save)[@handle=#X#save]"
                 );
          ostr <<  '\1' << "#X#load";
          ostr <<  '\1' << "#X#save";
        }
        gui << tab;
      }
      
      gui.create();
      
      (**gui.getValue<TabHandle>("__the_tab__")).setCurrentIndex(generalIdx);
      
      std::string cblist = ostr.str();
      if(cblist.size() > 1){
        gui.registerCallback(function(this,&icl::ConfigurableGUIWidget::exec),cblist.substr(1),'\1');
      }
      for(unsigned int i=0;i<timers.size();++i){
        timers[i]->start();
      }
      
      conf->registerCallback(function(this,&icl::ConfigurableGUIWidget::propertyChanged));
    }

    /// Called if a property is changed from somewhere else
    void propertyChanged(const Configurable::Property &p){
      const std::string &name = p.name;
      const std::string &type = p.type;
      //const std::string &value = p.value;
      
      DEBUG_LOG("name: " << name << "  type:" << type);
      deactivateExec = true;
      if(type == "range" || type == "range:slider"){
        gui.getValue<FSliderHandle>("#r#"+name).setValue( parse<icl32f>(conf->getPropertyValue(name)) );
      }else if (type == "range:spinbox"){
        gui.getValue<SpinnerHandle>("#R#"+name).setValue( parse<icl32s>(conf->getPropertyValue(name)) );
      }else if( type == "menu" || type == "value-list" || type == "valueList"){
        std::string handle = (type == "menu" ? "#m#" : "#v#")+name;
        gui.getValue<ComboHandle>(handle).setSelectedItem(conf->getPropertyValue(name));
      }else if( type == "info"){
        gui["#i#"+name] = conf->getPropertyValue(name);
      }else if( type == "flag"){
        gui["#f#"+name] = conf->getPropertyValue(name).as<bool>();
      }
      
      deactivateExec = false;
    }
    
    void exec(const std::string &handle){
      if(deactivateExec) return;
      if(handle.length()<3 || handle[0] != '#') throw ICLException("invalid callback (this should not happen)");
      std::string prop = handle.substr(3);
      switch(handle[1]){
        case 'r': 
        case 'R': 
        case 'm': 
        case 'v': 
        case 'f':
          conf->setPropertyValue(prop,gui[handle].as<Any>());
        break;
        case 'c': 
          conf->setPropertyValue(prop,"");
          break;
        case 'X':
          if(prop == "load"){
            QString filename = QFileDialog::getOpenFileName(this,"load property file","","XML-Files (*.xml)");
            if(!filename.isNull() && filename != ""){
              conf->loadProperties(filename.toLatin1().data());
              //update_all_components();
            }
          }else if(prop == "save"){
            QString filename = QFileDialog::getSaveFileName(this,"save properties to file","","XML-Files (*.xml)");
            if(!filename.isNull() && filename != ""){
              conf->saveProperties(filename.toLatin1().data());
            }
          }
          break;
        default:
          ERROR_LOG("invalid callback ID " << handle);
      }
    }

    static string getSyntax(){
      return string("prop(ConfigurableID)[general params]\n")+gen_params();
    }

  };

  struct CamCfgGUIWidget : public GUIWidget{
    // {{{ open
    CamCfgGUIWidget(const GUIDefinition &def):GUIWidget(def,0,2){
      if(def.numParams() == 2){
        throw GUISyntaxErrorException(def.defString(),"camcfg can take 0 or 2 parameters");
      }
      m_button = new QPushButton("camcfg",this);
      connect(m_button,SIGNAL(clicked()),this,SLOT(ioSlot()));
      
      if(def.numParams()){
        devType = def.param(0);
        devID = def.param(1);
      }
      m_cfg = 0;

      addToGrid(m_button);
    }
    virtual void processIO(){
      if(!m_cfg){
        m_cfg = new CamCfgWidget(devType,devID);
      }
      m_cfg->show();
    }
    static string getSyntax(){
      return string("camcfg(devType="",devID="")[general params]\n")+gen_params();
    }
    CamCfgWidget *m_cfg;
    QPushButton *m_button;
    std::string devType,devID;
  };

  struct ConfigFileGUIWidget : public GUIWidget{
    // {{{ open
    ConfigFileGUIWidget(const GUIDefinition &def):GUIWidget(def,1){
      
      if(def.param(0) == "embedded"){
        m_button = 0;
        m_cfg = new ConfigFileGUI(ConfigFile::getConfig(),this);
        m_cfg->create();
        addToGrid(m_cfg->getWidget());
      }else if(def.param(0) == "popup"){
        m_button = new QPushButton("config",this);
        connect(m_button,SIGNAL(clicked()),this,SLOT(ioSlot()));
        m_cfg = new ConfigFileGUI;
        addToGrid(m_button);
      }else{
        throw GUISyntaxErrorException(def.defString(),"allowed parameters are \"embedded\" or \"popup\"");
      }
    }
    virtual void processIO(){
      m_cfg->show();
    }
    static string getSyntax(){
      return string("config(popup|embedded)[general params]\n")+gen_params();
    }
    ConfigFileGUI *m_cfg;
    QPushButton *m_button;
  };


  struct ScrollGUIWidgetBase : public GUIWidget, public ProxyLayout{
    // {{{ open

    ScrollGUIWidgetBase(const GUIDefinition &def, QBoxLayout::Direction d):
      GUIWidget(def,0,-1,GUIWidget::noLayout){
      setLayout(new QBoxLayout(d,this));
      m_poScroll = new QScrollArea(this);
      layout()->addWidget(m_poScroll);
      layout()->setContentsMargins(0,0,0,0);
      m_poScroll->setWidget(new QWidget(m_poScroll));
      m_poScroll->setWidgetResizable(true);

      m_poScroll->widget()->setLayout(new QBoxLayout(d,m_poScroll));
      m_poScroll->widget()->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));

      m_poScroll->widget()->layout()->setMargin(def.margin());
      m_poScroll->widget()->layout()->setSpacing(def.spacing());
      
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<BoxHandle>(def.handle(),BoxHandle(d != QBoxLayout::LeftToRight,m_poScroll->widget(),this,m_poScroll));
        getGUI()->unlockData();
      }
    }

    virtual ProxyLayout *getProxyLayout() { return this; }

    virtual void addWidget(GUIWidget *widget){
      widget->setParent(m_poScroll->widget());
      m_poScroll->widget()->layout()->addWidget(widget);
    }

    private:
    QScrollArea *m_poScroll;
  };

  // }}}

  struct HScrollGUIWidget : public ScrollGUIWidgetBase{
    // {{{ open
    HScrollGUIWidget(const GUIDefinition &def):ScrollGUIWidgetBase(def,QBoxLayout::LeftToRight){}
    static string getSyntax(){
      return string("hscroll()[general params]\n")+ gen_params();
    }
  };
  // }}}

  struct VScrollGUIWidget : public ScrollGUIWidgetBase{
    // {{{ open
    VScrollGUIWidget(const GUIDefinition &def):ScrollGUIWidgetBase(def,QBoxLayout::TopToBottom){}
    static string getSyntax(){
      return string("vscroll()[general params]\n")+ gen_params();
    }
  };
  // }}}
    
  struct HBoxGUIWidget : public GUIWidget{
    // {{{ open
    HBoxGUIWidget(const GUIDefinition &def):GUIWidget(def,0,0,GUIWidget::hboxLayout){
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

      if(def.handle() != ""){
        getGUI()->lockData();
        //        DEBUG_LOG("allocating h box handle");
        getGUI()->allocValue<BoxHandle>(def.handle(),BoxHandle(true,this,this));//def.parentWidget()));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return string("hbox()[general params]\n")+gen_params();
    }
  };
  
  // }}}


  struct VBoxGUIWidget : public GUIWidget{
    // {{{ open
    VBoxGUIWidget(const GUIDefinition &def):GUIWidget(def,0,0,GUIWidget::vboxLayout){
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      if(def.handle() != ""){
        getGUI()->lockData();
        //DEBUG_LOG("allocating v box handle");
        getGUI()->allocValue<BoxHandle>(def.handle(),BoxHandle(false,this,this));//def.parentWidget()));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return string("hbox()[general params]\n")+gen_params();
    }
  };

  // }}}

  struct SplitterGUIWidgetBase : public GUIWidget, public ProxyLayout{
    // {{{ open
    SplitterGUIWidgetBase(const GUIDefinition &def, bool horz):GUIWidget(def,0,0,GUIWidget::noLayout){
      m_layout = new QGridLayout(this);
      m_splitter = new QSplitter(horz ? Qt::Horizontal:Qt::Vertical , this);
      m_layout->addWidget(m_splitter,0,0);
      m_layout->setContentsMargins(2,2,2,2);

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<SplitterHandle>(def.handle(),SplitterHandle(m_splitter,this));
        getGUI()->unlockData();
      }
    }
    
    // implements the ProxyLayout interface, what should be done if components are added
    // using the GUI-stream operator <<
    virtual void addWidget(GUIWidget *widget){
      m_splitter->addWidget(widget);
    }
    
    // as this implements also to proxy layout class, this interface function
    // can directly return itself 
    virtual ProxyLayout *getProxyLayout() { return this; }
    
    //static string getSyntax(){
    //  return string("(COMMA_SEPERATED_TAB_LIST)[general params]\n")+gen_params();
    //}
    QSplitter *m_splitter;
    QGridLayout *m_layout;
  };
  // }}}
  
  struct HSplitterGUIWidget : public SplitterGUIWidgetBase{
    // {{{ open
    HSplitterGUIWidget(const GUIDefinition &def):SplitterGUIWidgetBase(def,true){}
    static string getSyntax(){
      return string("hsplit()[general params]\n")+gen_params();
    }
  };
  // }}}

  struct VSplitterGUIWidget : public SplitterGUIWidgetBase{
    // {{{ open
    VSplitterGUIWidget(const GUIDefinition &def):SplitterGUIWidgetBase(def,false){}
    static string getSyntax(){
      return string("vsplit()[general params]\n")+gen_params();
    }
  };
  // }}}

  struct TabGUIWidget : public GUIWidget, public ProxyLayout{
    // {{{ open
    TabGUIWidget(const GUIDefinition &def):GUIWidget(def,0,2<<20,GUIWidget::noLayout){
      m_layout = new QGridLayout(this);
      m_tabWidget = new QTabWidget(this);
      m_layout->addWidget(m_tabWidget,0,0);
      m_layout->setContentsMargins(2,2,2,2);
      m_nextTabIdx = 0;
      m_tabNames = def.allParams();

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<TabHandle>(def.handle(),TabHandle(m_tabWidget,this));
        getGUI()->unlockData();
      }
    }
    
    // implements the ProxyLayout interface, what should be done if components are added
    // using the GUI-stream operator <<
    virtual void addWidget(GUIWidget *widget){
      QString tabName;
      if(m_nextTabIdx < (int)m_tabNames.size()){
        tabName = m_tabNames[m_nextTabIdx].c_str();
      }else{
        ERROR_LOG("no tab name defined for " << (m_nextTabIdx) << "th tab");
        tabName = QString("Tab ")+QString::number(m_nextTabIdx);
      }
      m_tabWidget->addTab(widget,tabName);
      m_nextTabIdx++;
    }
    
    // as this implements also to proxy layout class, this interface function
    // can directly return itself 
    virtual ProxyLayout *getProxyLayout() { return this; }
    
    static string getSyntax(){
      return string("tab(COMMA_SEPERATED_TAB_LIST)[general params]\n")+gen_params();
    }
    
    std::vector<std::string> m_tabNames;
    QTabWidget *m_tabWidget;
    int m_nextTabIdx;
    QGridLayout *m_layout;
  };
  // }}}

  struct BorderGUIWidget : public GUIWidget{
    // {{{ open

    BorderGUIWidget(const GUIDefinition &def):GUIWidget(def,1){
      m_poGroupBox = new QGroupBox(def.param(0).c_str(),def.parentWidget());
      m_poGroupBox->setFlat(false);
      //m_poGroupBox->setStyleSheet("QGroupBox{ border: 1px solid gray; border-radius: 3px;}");
      m_poGroupBox->setStyle(new QCleanlooksStyle());
      m_poLayout = new QVBoxLayout;
      m_poLayout->setMargin(def.margin());
      m_poLayout->setSpacing(def.spacing());
      m_poGroupBox->setLayout(m_poLayout);
      addToGrid(m_poGroupBox);
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<BorderHandle>(def.handle(),BorderHandle(m_poGroupBox,this));
        getGUI()->unlockData();
      }
      
    }
    static string getSyntax(){
      return string("border(LABEL)[general params]\n")+
      string("\tLABEL is the border label that is shown\n")+
      gen_params();
    }
    virtual QLayout *getGUIWidgetLayout() { return m_poLayout; }
    private:
    QGroupBox *m_poGroupBox;
    QVBoxLayout *m_poLayout;
  };

  // }}}


  struct ColorGUIWidget : public GUIWidget{
    ColorGUIWidget(const GUIDefinition &def):GUIWidget(def,3,4,GUIWidget::gridLayout,Size(6,2)){
      QPushButton *b = new QPushButton("select",def.parentWidget());
      addToGrid(b);
      connect(b,SIGNAL(clicked()),this,SLOT(ioSlot()));
      
      m_haveAlpha = (def.numParams() == 4);

      
      if(m_haveAlpha) m_color = new Color4D(def.intParam(0),def.intParam(1),def.intParam(2),0);
      else m_color = new Color4D();

      Color4D col(def.intParam(0),def.intParam(1),def.intParam(2),m_haveAlpha ?def.intParam(3):0);
      
      getGUI()->lockData();
      m_color = &getGUI()->allocValue<Color4D>(def.output(0),col);
      getGUI()->unlockData();

      colorLabel = new ColorLabel(*m_color,m_haveAlpha,def.parentWidget());

      addToGrid(colorLabel,1,0,1,1);
      
      if(def.handle() != ""){
        getGUI()->lockData();
        handle = &getGUI()->allocValue<ColorHandle>(def.handle(),ColorHandle(colorLabel,this));
        getGUI()->unlockData();
      }else{
        handle = 0;
      }
    }

    virtual void processIO(){
      QColor c = !m_haveAlpha ? 
                 QColor((*m_color)[0],(*m_color)[1],(*m_color)[2]) : 
                 QColor((*m_color)[0],(*m_color)[1],(*m_color)[2],(*m_color)[3]);
      
      QColor color = QColorDialog::getColor(c,this,"choose color...", 
                                            m_haveAlpha ? QColorDialog::ShowAlphaChannel : 
                                            (QColorDialog::ColorDialogOption)0);
      
      colorLabel->setColor(Color4D(color.red(),color.green(),color.blue(),color.alpha()));
    }
    
    static string getSyntax(){
      return string("color(R,G,B[,A])[general params] \n")+
      string("\tgiven initial Red, Green and Blue values (Alpha is optional)\n")+
      gen_params();
    }
    ColorLabel *colorLabel;
    ColorHandle *handle;
    Color4D *m_color;
    bool m_haveAlpha;
  };

  struct ButtonGUIWidget : public GUIWidget{
    // {{{ open
    ButtonGUIWidget(const GUIDefinition &def):GUIWidget(def,1,1,GUIWidget::gridLayout,Size(4,1)){
      QPushButton *b = new QPushButton(def.param(0).c_str(),def.parentWidget());
      addToGrid(b);
      connect(b,SIGNAL(pressed()),this,SLOT(ioSlot()));
      
      if(def.handle() != ""){
        getGUI()->lockData();
        m_poClickedEvent = &getGUI()->allocValue<ButtonHandle>(def.handle(),ButtonHandle(b,this));
        getGUI()->unlockData();
      }else{
        m_poClickedEvent = 0;
      }
    }
    static string getSyntax(){
      return string("button(TEXT)[general params] \n")+
      string("\tTEXT is the button text\n")+
      gen_params();
    }
    virtual void processIO(){
      if(m_poClickedEvent){      
        m_poClickedEvent->trigger(false);
      }
    }
  private:
    ButtonHandle *m_poClickedEvent;
  };

  // }}}
  struct ButtonGroupGUIWidget : public GUIWidget{
    // {{{ open

    ButtonGroupGUIWidget(const GUIDefinition &def):
      GUIWidget(def,1,2<<20,GUIWidget::gridLayout,Size(4,def.numParams())), m_uiInitialIndex(0){
     
      for(unsigned int i=0;i<def.numParams();i++){
        string text = def.param(i);
        if(text.length() && text[0]=='!'){
          m_uiInitialIndex = i;
          text = text.substr(1);
        }
        QRadioButton * b = new QRadioButton(text.c_str(),def.parentWidget());
        m_vecButtons.push_back(b);
        addToGrid(b,0,i);
        connect(b,SIGNAL(clicked()),this,SLOT(ioSlot()));
      }
      
      getGUI()->lockData();
      m_uiIdx = &getGUI()->allocValue<unsigned int>(def.output(0),m_uiInitialIndex);
      getGUI()->unlockData();
      
      m_vecButtons[m_uiInitialIndex]->setChecked(true);

      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<ButtonGroupHandle>(def.handle(),ButtonGroupHandle(&m_vecButtons,this));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return string("buttongroup(LIST)[general params] \n")+
      string("\tLIST is a comma seperated list of radio button texts to create\n")+
      string("\tthe button with a '!'-prefix is selected (of index 0 by default)\n")+
      gen_params();
    }
    virtual void processIO(){
      *m_uiIdx = 0;
      for(unsigned int i=0;i<m_vecButtons.size();i++){
        if(m_vecButtons[i]->isChecked()){
          *m_uiIdx = i;
          break;
        }
      }
    }
    //    virtual Size getDefaultSize() { 
    //  return Size(4,m_vecButtons.size()); 
    //}
  private:
    unsigned int *m_uiIdx;
    vector<QRadioButton*> m_vecButtons;
    unsigned int m_uiInitialIndex ;
  };

  // }}}
  struct ToggleButtonGUIWidget : public GUIWidget{
    // {{{ open
  public:
    ToggleButtonGUIWidget(const GUIDefinition &def):
      GUIWidget(def,2){

      bool initToggled = false;
      if(def.param(1).length() && def.param(1)[0] == '!'){
        initToggled = true;
      }

      getGUI()->lockData();
      bool *stateRef = &getGUI()->allocValue<bool>(def.output(0),initToggled);
      getGUI()->unlockData();
      
      std::string t1 = def.param(0);
      std::string t2 = def.param(1);
      if(t1.length() && t1[0]=='!') t1 = t1.substr(1);
      if(t2.length() && t2[0]=='!') t2 = t2.substr(1);
      m_poButton = new ToggleButton(t1,t2,def.parentWidget(),stateRef);
      if(initToggled){
        m_poButton->setChecked(true);
      }
      
      addToGrid(m_poButton);
      
      // this must be connected to the toggled function too (not to the clicked() signal) because 
      // the clicked()-signal is emitted BEFORE the toggled-signale, which makes the button get
      // out of sync-with it's underlying value :-(
      connect(m_poButton,SIGNAL(toggled(bool)),this,SLOT(ioSlot()));

      if(def.handle() != ""){
        getGUI()->lockData();
        m_poHandle = &getGUI()->allocValue<ButtonHandle>(def.handle(),ButtonHandle(m_poButton,this));
        getGUI()->unlockData();
      }else{
        m_poHandle = 0;
      }
    }
    static string getSyntax(){
      return string("togglebutton(U,T)[general params] \n")+
      string("\tU is the buttons text in untoggled state\n")+
      string("\tT is the buttons text in toggled state\n")+
      string("\tif one of U or T has a '!'-prefix, the button is created with this state\n")+
      gen_params();
    }
    virtual void processIO(){
      if(m_poHandle){
        m_poHandle->trigger(false);
      }
    }
  private:
    ToggleButton *m_poButton;
    ButtonHandle *m_poHandle;
    bool *m_pbToggled;
  };

// }}}


  struct CheckBoxGUIWidget : public GUIWidget{
    // {{{ open
  public:
    CheckBoxGUIWidget(const GUIDefinition &def):
      GUIWidget(def,2){

      bool initChecked = false;
      if(def.param(1)=="on" || def.param(1) == "yes" || def.param(1) == "checked"){
        initChecked = true;
      }

      getGUI()->lockData();
      m_stateRef = &getGUI()->allocValue<bool>(def.output(0),initChecked);
      getGUI()->unlockData();
      
      std::string t = def.param(0);
      m_poCheckBox = new QCheckBox(def.param(0).c_str(),def.parentWidget());
      m_poCheckBox->setTristate(false);
      if(initChecked){
        m_poCheckBox->setCheckState(Qt::Checked);
      }else{
        m_poCheckBox->setCheckState(Qt::Unchecked);
      }
      
      addToGrid(m_poCheckBox);
      
      // this must be connected to the toggled function too (not to the clicked() signal) because 
      // the clicked()-signal is emitted BEFORE the toggled-signale, which makes the button get
      // out of sync-with it's underlying value :-(
      connect(m_poCheckBox,SIGNAL(stateChanged(int)),this,SLOT(ioSlot()));

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<CheckBoxHandle>(def.handle(),CheckBoxHandle(m_poCheckBox,this,m_stateRef));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return string("checkbox(TEXT,INIT>)[general params] \n")+
      string("\tTEXT is the check box text\n");
      string("\tINIT defines whether the checkbox is initially checked (checked|unchecked)\n")+
      gen_params();
    }
    virtual void processIO(){
      *m_stateRef = m_poCheckBox->checkState() == Qt::Checked;
    }
  private:
    bool *m_stateRef;
    QCheckBox *m_poCheckBox;
  };

// }}}




  struct LabelGUIWidget : public GUIWidget{
    // {{{ open
    LabelGUIWidget(const GUIDefinition &def):GUIWidget(def,0,1,GUIWidget::gridLayout,Size(4,1)){
    
      m_poLabel = new CompabilityLabel(def.numParams()==1?def.param(0).c_str():"",def.parentWidget());
      
      addToGrid(m_poLabel);
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<LabelHandle>(def.handle(),LabelHandle(m_poLabel,this));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return 
      string("label(TEXT="")[general params] \n")+
      string("\tTEXT is the initial text showed by the label")+
      gen_params();
    }
  private:
    CompabilityLabel *m_poLabel;
  };
  
  // }}}



  struct StateGUIWidget : public GUIWidget{
    // {{{ open
    StateGUIWidget(const GUIDefinition &def):GUIWidget(def,0,1,GUIWidget::gridLayout,Size(4,1)){
      m_text = new ThreadedUpdatableTextView(def.parentWidget());
      m_text->setReadOnly(true);

      addToGrid(m_text);
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<StateHandle>(def.handle(),StateHandle(m_text,this,def.numParams()?parse<int>(def.param(0)):1<<30));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return 
      string("state(MAX_LINES)[general params] \n")+
      string("\tMAX_LINES is the maximal line count of the state widget, odd lines are removed automatically");
      gen_params();
    }
  private:
    ThreadedUpdatableTextView *m_text;
  };
  
  // }}}


  struct SliderGUIWidget : public GUIWidget{
    // {{{ open

    static bool vertical(const GUIDefinition &def){
      return (def.numParams() == 4) ? (def.param(3)=="vertical") : false;
    }
    
    SliderGUIWidget(const GUIDefinition &def):GUIWidget(def,3,4,GUIWidget::gridLayout,vertical(def)?Size(1,4):Size(4,1)){
      /// param_order = min,max,curr,step=1,orientation=("horizontal")|"vertical"
      
      m_piValue = &getGUI()->allocValue<int>(def.output(0),def.intParam(2));
      
      int iVerticalFlag = vertical(def);
      int iMin = def.intParam(0);
      int iMax = def.intParam(1);
      int iCurr = def.intParam(2);
      
      if(iVerticalFlag){
        m_poSlider = new ThreadedUpdatableSlider(Qt::Vertical,def.parentWidget());
      }else{
        m_poSlider = new ThreadedUpdatableSlider(Qt::Horizontal,def.parentWidget());
      }
      addToGrid(m_poSlider);
      
      m_poSlider->setMinimum(iMin);
      m_poSlider->setMaximum(iMax);
      m_poSlider->setValue(iCurr);
     
      int nDigits = iclMax(QString::number(iMin).length(),QString::number(iMax).length());
      // what is this ???
      // int iAbsMax = iMax > -iMin ? iMax : -iMin;
      // int iAddOneForSign = iMax < -iMin;
      // m_poLCD = new QLCDNumber(QString::number(iAbsMax).length()+iAddOneForSign,def.parentWidget());
      m_poLCD = new QLCDNumber(nDigits,def.parentWidget());
      m_poLCD->display(iCurr);
      
      if(iVerticalFlag){
        addToGrid(m_poLCD,0,1,1,4);
      }else{
        addToGrid(m_poLCD,1,0,4,1);
      }

      connect(m_poSlider,SIGNAL(valueChanged(int)),this,SLOT(ioSlot()));
      connect(m_poSlider,SIGNAL(valueChanged(int)),m_poLCD,SLOT(display(int)));
      
      m_bVerticalFlag = iVerticalFlag ? true : false;
     
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<SliderHandle>(def.handle(),SliderHandle(m_poSlider,this));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return 
      string("slider(MIN,MAX,CURR,ORIENTATION=horizontal)[general params] \n")+
      string("\tMIN is the minimum value of the slider\n")+
      string("\tMAX is the maximum value of the slider\n")+
      string("\tCURR is the initializing value of the slider\n")+
      string("\tORIENTATION is horizontal or vertical\n");
      gen_params();
    }
    virtual void processIO(){
      //cb();
      //iStep is handled as a value that must '%' the slider to 0
      *m_piValue = m_poSlider->value();
    }
  private:
    ThreadedUpdatableSlider *m_poSlider;
    QLCDNumber *m_poLCD;
    int *m_piValue;
    bool m_bVerticalFlag;
  };

// }}}
  struct FloatSliderGUIWidget : public GUIWidget{
    // {{{ open

    static bool vertical(const GUIDefinition &def){
      return (def.numParams() == 4) ? (def.param(3)=="vertical") : false;
    }
    
    FloatSliderGUIWidget(const GUIDefinition &def):GUIWidget(def,3,4,GUIWidget::gridLayout,vertical(def)?Size(1,4):Size(4,1)){
      //
      // y = mx+b
      // m =dy/dx = max-min/1000
      // b = min 
      //float fMin, float fMax, float fCurr vertical|horizontal
      //
      
      /// param_order = min,max,curr,orientation=("horizontal")|"vertical"
  
      getGUI()->lockData();
      m_pfValue = &getGUI()->allocValue<float>(def.output(0),def.floatParam(2));
      getGUI()->unlockData();
      
      int iVerticalFlag = vertical(def);
      m_fMinVal = def.floatParam(0);
      m_fMaxVal = def.floatParam(1);
      float fCurr = def.floatParam(2);
      int nDigits = 6;
      
      m_fM = (m_fMaxVal-m_fMinVal)/10000.0;
      m_fB = m_fMinVal;
      
      if(iVerticalFlag){
        m_poSlider = new ThreadedUpdatableSlider(Qt::Vertical,def.parentWidget());
      }else{
        m_poSlider = new ThreadedUpdatableSlider(Qt::Horizontal,def.parentWidget());
      }
      addToGrid(m_poSlider);

      m_poSlider->setMinimum(0);
      m_poSlider->setMaximum(10000);
      m_poSlider->setValue(f2i(fCurr));
      m_poLCD = new QLCDNumber(nDigits,def.parentWidget());
      m_poLCD->display(fCurr);
      
      if(iVerticalFlag){
        addToGrid(m_poLCD,0,1,1,4);
      }else{
        addToGrid(m_poLCD,1,0,4,1);
      }    
      connect(m_poSlider,SIGNAL(valueChanged(int)),this,SLOT(ioSlot()));
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<FSliderHandle>(def.handle(),FSliderHandle(m_poSlider,&m_fMinVal,&m_fMaxVal,&m_fM,&m_fB,10000,this));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return 
      string("fslider(MIN,MAX,CURR,ORIENTATION=horizontal)[general params] \n")+
      string("\tMIN is the minimum value of the slider\n")+
      string("\tMAX is the maximum value of the slider\n")+
      string("\tCURR is the initializing value of the slider\n")+
      string("\tORIENTATION is horizontal or vertical\n");
      gen_params();
    }
    virtual void processIO(){
      float value = i2f(m_poSlider->value());
      m_poLCD->display(value);
      //      pritnf("displaying %f \n",value);
      *m_pfValue = value;
    }
  private:
    ThreadedUpdatableSlider *m_poSlider;
    QLCDNumber *m_poLCD;
    float *m_pfValue;
    float m_fM,m_fB;
    float m_fMinVal, m_fMaxVal;
    int f2i(float f){
      return (int)((f-m_fB)/m_fM);
    }
    float i2f(int i){
      return m_fM*i+m_fB;
    } 
  };

// }}}
  struct IntGUIWidget : public GUIWidget{
    // {{{ open
public:
    IntGUIWidget(const GUIDefinition &def):GUIWidget(def,3){
      m_poLineEdit = new QLineEdit(def.parentWidget());
      m_poLineEdit->setValidator(new QIntValidator(def.intParam(0),def.intParam(1),0));
      m_poLineEdit->setText(QString::number(def.intParam(2)));
      
      QObject::connect(m_poLineEdit,SIGNAL(returnPressed ()),this,SLOT(ioSlot()));
      
      addToGrid(m_poLineEdit);

      getGUI()->lockData();
      m_piOutput = &getGUI()->allocValue<int>(def.output(0),def.intParam(2));
      getGUI()->unlockData();
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<IntHandle>(def.handle(),IntHandle(m_poLineEdit,this));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return 
      string("int(MIN,MAX,CURR)[general params] \n")+
      string("\tMIN is the minimum allowed input value\n")+
      string("\tMAX is the maximum allowed input value\n")+
      string("\tCURR is the initial value of the textfield\n")+
      gen_params();
    }
    virtual void processIO(){
      bool iOk;
      int iVal = m_poLineEdit->text().toInt(&iOk);
      if(iOk){
        *m_piOutput = iVal;
      }
    }
  private:
    QLineEdit *m_poLineEdit;
    int *m_piOutput;
  };

// }}}
  struct FloatGUIWidget : public GUIWidget{
    // {{{ open
public:
    FloatGUIWidget(const GUIDefinition &def):GUIWidget(def,3){
      m_poLineEdit = new QLineEdit(def.parentWidget());
      m_poLineEdit->setValidator(new QDoubleValidator(def.floatParam(0),def.floatParam(1),20,0));
      m_poLineEdit->setText(QString::number(def.floatParam(2)));
      
      QObject::connect(m_poLineEdit,SIGNAL(returnPressed ()),this,SLOT(ioSlot()));
      
      addToGrid(m_poLineEdit);
      getGUI()->lockData();
      m_pfOutput = &getGUI()->allocValue<float>(def.output(0),def.intParam(2));
      getGUI()->unlockData();

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<FloatHandle>(def.handle(),FloatHandle(m_poLineEdit,this));
        getGUI()->unlockData();
      }

    }
    static string getSyntax(){
      return 
      string("float(MIN,MAX,CURR)[general params] \n")+
      string("\tMIN is the minimum allowed input value\n")+
      string("\tMAX is the maximum allowed input value\n")+
      string("\tCURR is the initial value of the textfield\n")+
      gen_params();
    }
    virtual void processIO(){
      bool iOk;
      float fVal = m_poLineEdit->text().toFloat(&iOk);
      if(iOk){
        *m_pfOutput = fVal;
      }
    }
  private:
    QLineEdit *m_poLineEdit;
    float *m_pfOutput;
  };

// }}} 
  struct StringGUIWidget : public GUIWidget{
    // {{{ open
    class StringLenValidator : public QValidator{
    public:
      StringLenValidator(int iMaxLen):QValidator(0){
        this->iMaxLen = iMaxLen;
      }
      virtual State validate(QString &sInput,int &iPos)const{
        (void)iPos;
        return sInput.length()>iMaxLen ?  Invalid : Acceptable;
      }
    private:
      int iMaxLen;
    };
    
    StringGUIWidget(const GUIDefinition &def):GUIWidget(def,1,2){
      m_poLineEdit = new QLineEdit(def.parentWidget());
      m_poLineEdit->setValidator(new StringLenValidator(def.numParams() == 2 ? def.intParam(1) : 100));
      m_poLineEdit->setText(def.numParams()>=1 ? def.param(0).c_str() : "");
      
      QObject::connect(m_poLineEdit,SIGNAL(returnPressed ()),this,SLOT(ioSlot()));
      
      addToGrid(m_poLineEdit);
      
      getGUI()->lockData();
      m_psOutput = &getGUI()->allocValue<string>(def.output(0),def.param(0));
      getGUI()->unlockData();

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<StringHandle>(def.handle(),StringHandle(m_poLineEdit,m_psOutput,this));
        getGUI()->unlockData();
      }

    }
    static string getSyntax(){
      return 
      string("string(TEXT,MAXLEN)[general params] \n")+
      string("\tTEXT is the initial value of the textfield\n")+
      string("\tMAXLEN is max. number of characters that might be written into the textfiled\n")+
      gen_params();
    }
    virtual void processIO(){
      getGUI()->lockData();
      *m_psOutput = m_poLineEdit->text().toLatin1().data();
      getGUI()->unlockData();
    }
  private:
    QLineEdit *m_poLineEdit;
    string *m_psOutput;
  };

// }}} 
  struct DispGUIWidget : public GUIWidget{
    // {{{ open

    static Size dim(const GUIDefinition &def, int facX=1, int facY=1){
      int nW = def.intParam(0);
      int nH = def.intParam(1);
      if(nW < 1) throw GUISyntaxErrorException(def.defString(),"NW must be > 0");
      if(nH < 1) throw GUISyntaxErrorException(def.defString(),"NW must be > 0");
      return Size(nW,nH);
    }
    
    DispGUIWidget(const GUIDefinition &def):GUIWidget(def,2,2,GUIWidget::gridLayout,dim(def,2,1)){
      
      Size size = dim(def);
      int nW = size.width;
      int nH = size.height;

      m_poLabelMatrix = new LabelMatrix(nW,nH);

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<DispHandle>(def.handle(),DispHandle(m_poLabelMatrix,this));
        getGUI()->unlockData();  
      }
        
      for(int x=0;x<nW;x++){
        for(int y=0;y<nH;y++){
          CompabilityLabel *l = new CompabilityLabel("",def.parentWidget());
          (*m_poLabelMatrix)(x,y) = LabelHandle(l,this);
          addToGrid(l,x,y);
        }
      }
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    }
    static string getSyntax(){
      return 
      string("disp(NW,NH)[general params] \n")+
      string("\tNW is width of the display label matrix (must be > 0)")+    
      string("\tNH is height of the display label matrix (must be > 0)")+
      gen_params();
    }
  
  private:
    LabelMatrix *m_poLabelMatrix;
  };
  
  // }}}
  struct ImageGUIWidget : public GUIWidget{
    // {{{ open
    ImageGUIWidget(const GUIDefinition &def):GUIWidget(def,0){

      m_poWidget = new ICLWidget(def.parentWidget());
      addToGrid(m_poWidget);
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<ImageHandle>(def.handle(),ImageHandle(m_poWidget,this));
        getGUI()->unlockData();  
      }
      
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    }
    static string getSyntax(){
      return string("image()[general params] \n")+
      gen_params();
    }
  private:
    ICLWidget *m_poWidget;
  };

  // }}}
  struct DrawGUIWidget : public GUIWidget{
    // {{{ open
    DrawGUIWidget(const GUIDefinition &def):GUIWidget(def,0,1){
      m_poWidget = new ICLDrawWidget(def.parentWidget());
      if(def.numParams() == 1) {
        m_poWidget->setViewPort(parse<Size>(def.param(0)));
      }
      addToGrid(m_poWidget);
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<DrawHandle>(def.handle(),DrawHandle(m_poWidget,this));
        getGUI()->unlockData();  
      }
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    }
    static string getSyntax(){
      return string("draw()[general params] \n")+
      gen_params();
    }
  private:
    ICLDrawWidget *m_poWidget;
  };

  // }}}


  struct MultiDrawGUIWidget : public GUIWidget{
    // {{{ open
    MultiDrawGUIWidget(const GUIDefinition &def):GUIWidget(def,1,2<<20,GUIWidget::vboxLayout){
      m_bAll = false;
      m_bDeep = true;
      
      bool x[2]= {false,false};
      bool err = false;
      for(unsigned int i=0;i<def.numParams();++i){
        if(def.param(i) == "!one") {
          m_bAll = false;
          if(x[0]) err = true;
          x[0] = true;
        }
        if(def.param(i) == "!all"){
          m_bAll = true;
          if(x[0]) err = true;
          x[0] = true;
        }
        if(def.param(i) == "!deepcopy"){
          m_bDeep = true;          
          if(x[1]) err = true;
          x[1] = true;
        } 
        if(def.param(i) == "!shallowcopy"){
          m_bDeep = false;
          if(x[1]) err = true;
          x[1] = true;
        } 
      }
      if(err){
        throw GUISyntaxErrorException(def.defString(),"any two parameters are doubled or contradictory");
      }
      
      m_poTabBar = new QTabBar(def.parentWidget());
      m_poDrawWidget = new ICLDrawWidget(def.parentWidget());
      
      m_poTabBar->setMaximumHeight(25);
      getGUIWidgetLayout()->addWidget(m_poTabBar);
      getGUIWidgetLayout()->addWidget(m_poDrawWidget);

      for(unsigned int i=0;i<def.numParams();++i){
        std::string s = def.param(i);
        if(s.length() && s[0] == '!') continue;
        m_poTabBar->addTab(s.c_str());
      }
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<MultiDrawHandle>(def.handle(),MultiDrawHandle(m_poDrawWidget,m_poTabBar,&m_vecImageBuffer,m_bAll, m_bDeep,this));
        getGUI()->unlockData();  
      }
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    }
    static string getSyntax(){
      return string("multidraw(TAB-LIST)[general params] \n")+
      gen_params();
    }
    private:
    
    QTabBar *m_poTabBar;
    ICLDrawWidget *m_poDrawWidget;
    std::vector<ImgBase *> m_vecImageBuffer;
    
    bool m_bAll;
    bool m_bDeep;
  };

  // }}}

#ifdef HAVE_OPENGL
  struct DrawGUIWidget3D : public GUIWidget{
    // {{{ open
    DrawGUIWidget3D(const GUIDefinition &def):GUIWidget(def,0,1){
      m_poWidget3D = new ICLDrawWidget3D(def.parentWidget());
      if(def.numParams() == 1) {
        m_poWidget3D->setViewPort(parse<Size>(def.param(0)));
      }


      addToGrid(m_poWidget3D);
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<DrawHandle3D>(def.handle(),DrawHandle3D(m_poWidget3D,this));
        getGUI()->unlockData();  
      }
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    }
    static string getSyntax(){
      return string("draw()[general params] \n")+
      gen_params();
    }
  private:
    ICLDrawWidget3D *m_poWidget3D;
  };
#endif

  // }}}
  struct ComboGUIWidget : public GUIWidget{
    // {{{ open
    ComboGUIWidget(const GUIDefinition &def):GUIWidget(def,1,2<<20,GUIWidget::gridLayout,Size(4,1)){
      if(def.numParams() < 1) throw GUISyntaxErrorException(def.defString(),"at least 1 param needed here!");
    
      m_poCombo = new QComboBox(def.parentWidget());
      addToGrid(m_poCombo);

      unsigned int selectedIndex = 0;
      string sFirst = def.param(0);
      for(unsigned int i=0;i<def.numParams();i++){
        const std::string &s = def.param(i);
        if(s.length() && s[0]=='!'){
          sFirst = s.substr(1);
          selectedIndex = i;
          m_poCombo->addItem(s.substr(1).c_str());
        }else{
          m_poCombo->addItem(s.c_str());
        }
      }

      getGUI()->lockData();
      m_psCurrentText = &getGUI()->allocValue<string>(def.output(0),sFirst);
      getGUI()->unlockData();
      
      m_poCombo->setCurrentIndex(selectedIndex);
      
      connect(m_poCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(ioSlot()));	
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<ComboHandle>(def.handle(),ComboHandle(m_poCombo,this));
        getGUI()->unlockData();  
      }
    }
    static string getSyntax(){
      return string("combo(entry1,entry2,entry3)[general params] \n")+
      string("\tentryX is the the X-th entry of the combo box\n")+
      string("\tif any entry has a '!'-prefix, this entry will be selected initially\n")+
      gen_params();
    }
    virtual void processIO(){
      getGUI()->lockData();
      *m_psCurrentText = m_poCombo->currentText().toLatin1().data();
      getGUI()->unlockData();
    }
   
  private:
    string *m_psCurrentText;
    QComboBox *m_poCombo;
  };

  // }}}
  struct SpinnerGUIWidget : public GUIWidget{
    // {{{ open
public:
    SpinnerGUIWidget(const GUIDefinition &def):GUIWidget(def,3,3,GUIWidget::gridLayout,Size(4,1)){
      m_poSpinBox = new QSpinBox(def.parentWidget());
      m_poSpinBox->setRange(def.intParam(0),def.intParam(1));
      m_poSpinBox->setValue(def.intParam(2));
      
      QObject::connect(m_poSpinBox,SIGNAL(valueChanged(int)),this,SLOT(ioSlot()));
      
      addToGrid(m_poSpinBox);

      getGUI()->lockData();
      m_piOutput = &getGUI()->allocValue<int>(def.output(0),def.intParam(2));
      getGUI()->unlockData();

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<SpinnerHandle>(def.handle(),SpinnerHandle(m_poSpinBox,this));
        getGUI()->unlockData();  
      }

    }
    static string getSyntax(){
      return 
      string("spinner(MIN,MAX,CURR)[general params] \n")+
      string("\tMIN is the minimum possible value\n")+
      string("\tMAX is the maximum possible value\n")+
      string("\tCURR is the initial value of the spinbox\n")+
      gen_params();
    }
    virtual void processIO(){
      *m_piOutput = m_poSpinBox->value();
    }
  private:
    QSpinBox *m_poSpinBox;
    int *m_piOutput;
  };

// }}}

  struct FPSGUIWidget : public GUIWidget{
    // {{{ open
    FPSGUIWidget(const GUIDefinition &def):GUIWidget(def,0,1,GUIWidget::gridLayout,Size(4,2)){
      int np = def.numParams();
      int fpsEstimatorTimeWindow = np ? def.intParam(0) : 10;
      
      m_poLabel = new CompabilityLabel("fps...",def.parentWidget());
      
      addToGrid(m_poLabel);
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<FPSHandle>(def.handle(),FPSHandle(fpsEstimatorTimeWindow,m_poLabel,this));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return string("fps(TIME_WINDOW=10)[general params] \n"
                    "  TIME_WINDOW is the number of timesteps, that are used as \n"
                    "  Low-Pass-Filter for estimated fps counts\n")
                    +gen_params();
    }
    virtual Size getDefaultSize() { 
      return Size(4,1); 
    }
  private:
    CompabilityLabel *m_poLabel;
  };
  
  // }}}
  /**
      struct HContainerGUIWidget : public GUIWidget{
      // {{{ open
    HContainerGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::hboxLayout,2,0,0){
      
      getGUI()->lockData();
      getGUI()->allocValue<QWidget*>(def.input(0),def.parentWidget());
      getGUI()->allocValue<QLayout*>(def.input(1),def.parentLayout());
      getGUI()->unlockData();
      
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    }
    static string getSyntax(){
      return string("hcontainer()[general params]\n")+gen_params();
    }
  };
  // }}}
      struct VContainerGUIWidget : public GUIWidget{
      // {{{ open
    VContainerGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::vboxLayout,2,0,0){
      
      getGUI()->lockData();
      getGUI()->allocValue<QWidget*>(def.input(0),def.parentWidget());
      getGUI()->allocValue<QLayout*>(def.input(1),def.parentLayout());
      getGUI()->unlockData();
      
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    }
    static string getSyntax(){
      return string("vcontainer()[general params]\n")+gen_params();
    }
  };
  // }}}
  **/
      
      /// template for creating arbitrary GUIWidget's
  template<class T>
  GUIWidget *create_widget_template(const GUIDefinition &def){
    // {{{ open
    T *t = 0;
    try{
      t = new  T(def);
    }catch(GUISyntaxErrorException &ex){
      printf("%s\nsyntax is: %s\n",ex.what(),T::getSyntax().c_str());
      return 0;
    }
    return t;
  }

  // }}}<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  
  /// Definition for an arbitrary GUIWidget creator function
  typedef GUIWidget* (*gui_widget_creator_function)(const GUIDefinition &def);
  
  // Type definition for a function map to accelerate the gui creation process
  typedef std::map<string,gui_widget_creator_function> CreatorFuncMap;
  
  /// NEW CREATOR MAP ENTRIES HERE !!!
  /*  
      This function is called by the GUI::create function,
      to create arbitrary widgets. To accelerate the widget creation process
      it build a CreatorFuncMap which uses the GUIDefinitinos type-string as
      identifier to estimate which creation function must be called.         */
  GUIWidget *create_widget(const GUIDefinition &def){
    // {{{ open

    /// use a static map to accelerate the widget creation process
    static bool first = true;
    static CreatorFuncMap MAP_CREATOR_FUNCS;
    if(first){
      first = false;
      /// Fill the map with creator function ( use the template if possible )
      MAP_CREATOR_FUNCS["hbox"] = create_widget_template<HBoxGUIWidget>;
      MAP_CREATOR_FUNCS["vbox"] = create_widget_template<VBoxGUIWidget>;
      MAP_CREATOR_FUNCS["hscroll"] = create_widget_template<HScrollGUIWidget>;
      MAP_CREATOR_FUNCS["vscroll"] = create_widget_template<VScrollGUIWidget>;
      MAP_CREATOR_FUNCS["button"] = create_widget_template<ButtonGUIWidget>;
      MAP_CREATOR_FUNCS["border"] = create_widget_template<BorderGUIWidget>;
      MAP_CREATOR_FUNCS["buttongroup"] = create_widget_template<ButtonGroupGUIWidget>;     
      MAP_CREATOR_FUNCS["togglebutton"] = create_widget_template<ToggleButtonGUIWidget>;
      MAP_CREATOR_FUNCS["checkbox"] = create_widget_template<CheckBoxGUIWidget>;
      MAP_CREATOR_FUNCS["label"] = create_widget_template<LabelGUIWidget>;
      MAP_CREATOR_FUNCS["slider"] = create_widget_template<SliderGUIWidget>;
      MAP_CREATOR_FUNCS["fslider"] = create_widget_template<FloatSliderGUIWidget>;
      MAP_CREATOR_FUNCS["int"] = create_widget_template<IntGUIWidget>;
      MAP_CREATOR_FUNCS["float"] = create_widget_template<FloatGUIWidget>;
      MAP_CREATOR_FUNCS["string"] = create_widget_template<StringGUIWidget>;
      MAP_CREATOR_FUNCS["disp"] = create_widget_template<DispGUIWidget>;
      MAP_CREATOR_FUNCS["image"] = create_widget_template<ImageGUIWidget>;
      MAP_CREATOR_FUNCS["state"] = create_widget_template<StateGUIWidget>;
      MAP_CREATOR_FUNCS["draw"] = create_widget_template<DrawGUIWidget>;
#ifdef HAVE_OPENGL
      MAP_CREATOR_FUNCS["draw3D"] = create_widget_template<DrawGUIWidget3D>;
#endif
      MAP_CREATOR_FUNCS["combo"] = create_widget_template<ComboGUIWidget>;
      MAP_CREATOR_FUNCS["spinner"] = create_widget_template<SpinnerGUIWidget>;
      MAP_CREATOR_FUNCS["fps"] = create_widget_template<FPSGUIWidget>;
      MAP_CREATOR_FUNCS["multidraw"] = create_widget_template<MultiDrawGUIWidget>;
      MAP_CREATOR_FUNCS["tab"] = create_widget_template<TabGUIWidget>;
      MAP_CREATOR_FUNCS["hsplit"] = create_widget_template<HSplitterGUIWidget>;
      MAP_CREATOR_FUNCS["vsplit"] = create_widget_template<VSplitterGUIWidget>;
      MAP_CREATOR_FUNCS["camcfg"] = create_widget_template<CamCfgGUIWidget>;
      MAP_CREATOR_FUNCS["config"] = create_widget_template<ConfigFileGUIWidget>;
      MAP_CREATOR_FUNCS["prop"] = create_widget_template<ConfigurableGUIWidget>;
      MAP_CREATOR_FUNCS["color"] = create_widget_template<ColorGUIWidget>;

      //      MAP_CREATOR_FUNCS["hcontainer"] = create_widget_template<HContainerGUIWidget>;
      //      MAP_CREATOR_FUNCS["vcontainer"] = create_widget_template<VContainerGUIWidget>;
    }
    
    /// find the creator function
    CreatorFuncMap::iterator it = MAP_CREATOR_FUNCS.find(def.type());
    if(it != MAP_CREATOR_FUNCS.end()){
      /// call the function if it could be found
      return it->second(def);
    }else{
      ERROR_LOG("unknown type \""<< def.type() << "\"");
      return 0;
    }
  }

  // }}}
  
  string extract_label(string s){
    // {{{ open

    string::size_type p = s.find('[');
    if(p == string::npos) return "";
    s = s.substr(p+1);
    if(!s.length()) return "";
    if(s[s.length()-1] != ']') return "";
    StrTok t(s.substr(0,s.length()-1),"@");
    while(t.hasMoreTokens()){
      const string &s2 = t.nextToken();
      if(!s2.find("label",0)){
        if(s2.length() < 7) return "";
        return s2.substr(6);
      } 
    }
    return "";
  }

  // }}}
  string extract_minsize(string s){
    // {{{ open
    
    string::size_type p = s.find('[');
    if(p == string::npos) return "";
    s = s.substr(p+1);
    if(!s.length()) return "";
    if(s[s.length()-1] != ']') return "";
    StrTok t(s.substr(0,s.length()-1),"@");
    while(t.hasMoreTokens()){
      const string &s2 = t.nextToken();
      if(!s2.find("minsize",0)){
        if(s2.length() < 9) return "";
        return s2.substr(8);
      } 
    }
    return "";
  }

  // }}}
  string extract_maxsize(string s){
    // {{{ open
    
    string::size_type p = s.find('[');
    if(p == string::npos) return "";
    s = s.substr(p+1);
    if(!s.length()) return "";
    if(s[s.length()-1] != ']') return "";
    StrTok t(s.substr(0,s.length()-1),"@");
    while(t.hasMoreTokens()){
      const string &s2 = t.nextToken();
      if(!s2.find("maxsize",0)){
        if(s2.length() < 9) return "";
        return s2.substr(8);
      } 
    }
    return "";
  }

  // }}}
  string extract_size(string s){
    // {{{ open
    
    string::size_type p = s.find('[');
    if(p == string::npos) return "";
    s = s.substr(p+1);
    if(!s.length()) return "";
    if(s[s.length()-1] != ']') return "";
    StrTok t(s.substr(0,s.length()-1),"@");
    while(t.hasMoreTokens()){
      const string &s2 = t.nextToken();
      if(!s2.find("size",0)){
        if(s2.length() < 6) return "";
        return s2.substr(5);
      } 
    }
    return "";
  }

  // }}}

  string remove_label(const string &s, const std::string &label){
    // {{{ open
    string toRemove = string("@label=")+label;
    unsigned int p = s.find(toRemove);
    return s.substr(0,p) + s.substr(p+toRemove.length());
  } 

  // }}}

  GUI::GUI(const std::string &definition,QWidget *parent):
    // {{{ open
    m_sDefinition(definition),m_poWidget(0),m_bCreated(false),m_poParent(parent){
  }

  // }}}
  GUI::GUI(const GUI &g,QWidget *parent):
    // {{{ open
    m_sDefinition(g.m_sDefinition),
    m_vecChilds(g.m_vecChilds),
    m_poWidget(NULL),m_bCreated(false),
    m_poParent(parent){
  }
  // }}}
  
  GUI::~GUI(){
    // this leads to seg-faults 
    //delete m_poWidget;
  }


  GUI &GUI::operator<<(const std::string &definition){
    // {{{ open
    if(m_poWidget) { ERROR_LOG("this GUI is already visible"); return *this; }
    if(definition.length() > 100000) {
      throw GUISyntaxErrorException("-- long text --","definition string was too large! (>100000 characters)");
    }

    if(definition.size() && definition[0] == '!'){
      if(definition == "!show"){
        show();
        return *this;
      }else if(definition == "!create"){
        create();
        return *this;
      }
      throw GUISyntaxErrorException(definition,"wrong !xxx command found in GUI::operator<< : allowed are \"!show\" and \"!create\")");
    }
    
    /**
        //#ifndef WIN32
        usleep(1000*100);
        #else
	Sleep(100);
        //#endif
    **/

    string label = extract_label(definition);
    string minsize = extract_minsize(definition);
    string maxsize = extract_maxsize(definition);
    string size = extract_size(definition);
    
    Size S11(1,1);
    if(minsize.length()) minsize = string("@minsize=")+str(parse<Size>(minsize)+S11);
    if(maxsize.length()) maxsize = string("@maxsize=")+str(parse<Size>(maxsize)+S11);
    if(size.length()) size = string("@size=")+str(parse<Size>(size)+S11);

    if(label.length()){
      string rest = remove_label(definition,label);
      return ( (*this) << ( GUI(string("border("+label+")["+minsize+maxsize+size+"]")) << rest ) );
    }else{
      m_vecChilds.push_back(new GUI(definition));
      return *this;
    }
  }

  // }}}
  GUI &GUI::operator<<(const GUI &g){
    // {{{ open

    if(m_poWidget) { ERROR_LOG("this GUI is already visible"); return *this; }
    string label = extract_label(g.m_sDefinition);
    string minsize = extract_minsize(g.m_sDefinition);
    string maxsize = extract_maxsize(g.m_sDefinition);
    string size = extract_size(g.m_sDefinition);
    
    Size S11(1,1);
    if(minsize.length()) minsize = string("@minsize=")+str(parse<Size>(minsize)+S11);
    if(maxsize.length()) maxsize = string("@maxsize=")+str(parse<Size>(maxsize)+S11);
    if(size.length()) size = string("@size=")+str(parse<Size>(size)+S11);


    if(label.length()){
      GUI gNew(g);
      if(gNew.m_sDefinition.length() > 100000) {
        throw GUISyntaxErrorException("-- long text --","definition string was too large! (>100000 characters)");
      }
      gNew.m_sDefinition = remove_label(g.m_sDefinition,label);
      return ( *this << (  GUI(string("border(")+label+")["+minsize+maxsize+size+"]") << gNew ) );
    }else{
      m_vecChilds.push_back(new GUI(g));
      return *this;
    }

    
    //    m_vecChilds.push_back(new GUI(g));
    return *this;
  }

  // }}}
  
  void GUI::create(QLayout *parentLayout,ProxyLayout *proxy,QWidget *parentWidget, DataStore *ds){
    // {{{ open
    if(ds) m_oDataStore = *ds;
    try{
      GUIDefinition def(m_sDefinition,this,parentLayout,proxy,parentWidget);
      
      m_poWidget = create_widget(def);

      if(!parentWidget){
        //        std::cout << "setting window title:" << QApplication::applicationName().toLatin1().data() << std::endl;
        //m_poWidget->setWindowTitle(File(QApplication::arguments().at(0).toLatin1().data()).getBaseName().c_str());
        m_poWidget->setWindowTitle(File(QApplication::applicationName().toLatin1().data()).getBaseName().c_str());
      }
      
      if(!m_poWidget){
        ERROR_LOG("Widget could not be created ( aborting to avoid errors ) \n");
        exit(0);
      }
      QLayout *layout = m_poWidget->getGUIWidgetLayout();
      ProxyLayout *proxy = m_poWidget->getProxyLayout();

      if(!layout && !proxy && m_vecChilds.size()){
        ERROR_LOG("GUI widget has no layout, "<< m_vecChilds.size() <<" child components can't be added!");
        return;
      }
      for(unsigned int i=0;i<m_vecChilds.size();i++){
        m_vecChilds[i]->create(layout,proxy,m_poWidget,&m_oDataStore);
      }
      m_bCreated = true;
    }catch(GUISyntaxErrorException &ex){
      ERROR_LOG(ex.what());
      exit(0);
    }
     
  }

  // }}}
  
  bool GUI::isVisible() const{
    if(!m_bCreated) return false;
    return m_poWidget->isVisible();
  }
  
  void GUI::switchVisibility(){
    if(isVisible()) hide();
    else show();
  }

  void GUI::hide(){
    if(m_bCreated){
      m_poWidget->setVisible(false);
    }else{
      ERROR_LOG("unable to hide GUI that has not been created yet, call create() or show() first!");
    } 
  }

  void GUI::create(){
    if(!m_bCreated){
      if(m_poParent){
        create(m_poParent->layout(),0,m_poParent,0);
      }else{
        create(0,0,0,0);
      }
    }
  }

  void GUI::show(){
    // {{{ open
    create();
    m_poWidget->setVisible(true);
  }

  // }}}

  void GUI::waitForCreation(){
    while(!m_bCreated){
#ifndef WIN32
		usleep(1000*100);
#else
		Sleep(100);
#endif
    }
  }
  
  void GUI::registerCallback(const Callback &cb, const std::string &handleNamesList, char delim){
    std::string delims; delims+=delim;

    StrTok tok(handleNamesList,delims);
    while(tok.hasMoreTokens()){
      getValue<GUIHandleBase>(tok.nextToken(),false).registerCallback(cb);
    }
  }

  void GUI::registerCallback(const ComplexCallback &cb, const std::string &handleNamesList, char delim){
    std::string delims; delims+=delim;

    StrTok tok(handleNamesList,delims);
    while(tok.hasMoreTokens()){
      getValue<GUIHandleBase>(tok.nextToken(),false).registerCallback(cb);
    }
  }

  void GUI::removeCallbacks(const std::string &handleNamesList, char delim){
    std::string delims; delims+=delim;

    StrTok tok(handleNamesList,delims);
    while(tok.hasMoreTokens()){
      getValue<GUIHandleBase>(tok.nextToken(),false).removeCallbacks();
    }
  }

}

//  LocalWords:  if
