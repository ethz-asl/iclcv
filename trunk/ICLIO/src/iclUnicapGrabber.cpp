#include "iclUnicapGrabber.h"
#include <iclImg.h>
#include <string>
#include <map>
#include <unicap.h>
#include <iclConverter.h>
#include <iclCCFunctions.h>
#include "iclUnicapDevice.h"
#include <iclStringUtils.h>

#include "iclUnicapGrabEngine.h"
#include "iclUnicapConvertEngine.h"

#include "iclDefaultConvertEngine.h"
#include "iclDefaultGrabEngine.h"

#include <regex.h>

using namespace icl;
using namespace std;

namespace icl{

  UnicapGrabberImpl::UnicapGrabberImpl(const UnicapDevice &device):
    // {{{ open

    m_oDevice(device),m_poConversionBuffer(0),
    m_poGrabEngine(0),m_poConvertEngine(0), m_bUseDMA(false),
    m_bProgressiveGrabMode(true){

    init();
  }

  // }}}
  
  UnicapGrabberImpl::UnicapGrabberImpl(const std::string &deviceFilter, unsigned int useIndex):
    // {{{ open

    m_poConversionBuffer(0),m_poGrabEngine(0),
    m_poConvertEngine(0), m_bUseDMA(false),
    m_bProgressiveGrabMode(true){
    const std::vector<UnicapDevice> &ds = getDeviceList(deviceFilter);
    if(useIndex < ds.size()){
      m_oDevice = ds[useIndex];
    }else{
      ERROR_LOG("no device found for filter: \""<<deviceFilter<<"\"!");
    }    

    init();
  }

  // }}}

  UnicapGrabberImpl::~UnicapGrabberImpl(){
    // {{{ open
    m_oMutex.lock();
    ICL_DELETE(m_poGrabEngine);
    ICL_DELETE(m_poConvertEngine);
    ICL_DELETE(m_poImage);
    ICL_DELETE(m_poConversionBuffer);
    m_oMutex.unlock();
  }

  // }}}

  void UnicapGrabberImpl::init(){
    // {{{ open

    string modelname = m_oDevice.getModelName();
    if(modelname == "Philips 740 webcam"){
#define DEACTIVATE_PWC_CAM_IN_UNICAP
#ifdef DEACTIVATE_PWC_CAM_IN_UNICAP
      //      printf("Using PWCGrabEngine !\n");
      ERROR_LOG("Philips 740 webcam is not supported by the UnicapGrabberImpl !");
      m_poGrabEngine = 0 ; //new PWCGrabEngine(&m_oDevice);
      m_poConvertEngine = 0;
#else
      m_poGrabEngine = new DefaultGrabEngine(&m_oDevice, m_bUseDMA, m_bProgressiveGrabMode);
      m_poConvertEngine = new DefaultConvertEngine(&m_oDevice);
#endif
    }else if(modelname == "DFW-VL500 2.30"){ // sony cams !
      m_poGrabEngine = new DefaultGrabEngine(&m_oDevice, m_bUseDMA, m_bProgressiveGrabMode);
      m_poConvertEngine = new DefaultConvertEngine(&m_oDevice);
      
    }else if(modelname == "Hauppauge WinTV 34xxx models"){
      m_poGrabEngine = new DefaultGrabEngine(&m_oDevice, m_bUseDMA,m_bProgressiveGrabMode);
      m_poConvertEngine = new DefaultConvertEngine(&m_oDevice);

      static bool AVOID_RECURSIVE_CALL_FLAG = false;
      if(!AVOID_RECURSIVE_CALL_FLAG){
        AVOID_RECURSIVE_CALL_FLAG = true;
        setProperty("video source","S-Video");
        setProperty("video norm","PAL-BG");       //*****************************
        setProperty("size","640x480");            //                            *   
        setProperty("format","24 bpp RGB, le");   //   this properties work     *
        setProperty("Saturation","200");          //   finest, i think!         *
        setProperty("Contrast","66");             //                            *
        AVOID_RECURSIVE_CALL_FLAG = false;        //*****************************
      }                                           
    }else{
      m_poGrabEngine = new DefaultGrabEngine(&m_oDevice, m_bUseDMA, m_bProgressiveGrabMode);
      m_poConvertEngine = new DefaultConvertEngine(&m_oDevice);
    }
    m_fCurrentFps = 0;
    m_oLastTime = icl::Time::now();
  }

  // }}}

  void UnicapGrabberImpl::updateFps(){
    // {{{ open

    Time now = icl::Time::now();
    Time dt = now-m_oLastTime;
    long micro = dt.toMicroSeconds();
    m_fCurrentFps = micro ? 1000000.0/micro : 99999999;
    m_oLastTime = now;
  }

  // }}}

  float UnicapGrabberImpl::getCurrentFps() const{
    // {{{ open
    
    return m_fCurrentFps;
  }
    // }}}


  namespace{
    string force_lower_case(const string &s){
      // {{{ open

      static const Range<char> uppers('A','Z');
      static const int offs = 'A' - 'a';

      string r=s;
      for(unsigned int i=0;i<r.length();i++){
        if(uppers.contains(r[i])) r[i]-=offs;
      }
      return r;
    }
    
    // }}}
    string sizeVecToStr(const vector<Size> &v){
      // {{{ open
      if(!v.size()) return "{}";
      std::ostringstream os;

      os << '{';
      for(unsigned int i=0;i<v.size()-1;i++){
        os << '"' << v[i] << '"' << ",";
      }      
      os << '"' << v.back() << '"' << '}';
      return os.str();
    }

    // }}}
  }
  
  void UnicapGrabberImpl::setProperty(const std::string &property, const std::string &value){
    // {{{ open
    
    //    bool verbose = true;
    string p = force_lower_case(property);

    // search for former "params"
    m_oMutex.lock();
    if(p == "size"){
      // {{{ open
      vector<string> ts = tok(value,"x");
      if(ts.size() != 2){
        ERROR_LOG("unable to set parameter size to \""<<value<<"\" (expected syntax WxH)");
        m_oMutex.unlock();
        return;
      }else{
        Size s(atoi(ts[0].c_str()),atoi(ts[1].c_str()));
        delete m_poGrabEngine;
        m_poGrabEngine = 0;
        delete m_poConvertEngine;
        m_poConvertEngine = 0;
        m_oDevice.setFormatSize(s);
        init(); // creates a new grab engine
      }

      // }}}
    }else if(p == "format"){
      // {{{ open

      delete m_poGrabEngine;
      m_poGrabEngine = 0;
      delete m_poConvertEngine;
      m_poConvertEngine = 0;
      m_oDevice.setFormatID(value);
      init(); // creates new grab engine

      // }}}
    }else if(p == "size&format"){
      // {{{ open

      vector<string> tmp = tok(value,"&");
      if(tmp.size() != 2){
        ERROR_LOG("unable to set parameter format&size to \""<<value<<"\" (expected syntax WxH&FORMAT_ID)");
        m_oMutex.unlock();
        return;
      }

      string size = tmp[0];
      string fmt = tmp[1];
      vector<string> ts = tok(size,"x");
      if(ts.size() != 2){
        ERROR_LOG("unable to set size of parameter format&size to \""<<size<<"\" (expected syntax WxH)");
        m_oMutex.unlock();
        return;
      }else{
        Size s(atoi(ts[0].c_str()),atoi(ts[1].c_str()));
        delete m_poGrabEngine;
        m_poGrabEngine = 0;
        delete m_poConvertEngine;
        m_poConvertEngine = 0;
        m_oDevice.setFormat(fmt,s);
        init(); // creates a new grab engine
      }

      // }}}
    }else if(p == "dma"){
      // {{{ open
      if(value == "on" || value == "off"){
        if((value == "on" && !m_bUseDMA) || (value == "off" && m_bUseDMA)){
          delete m_poGrabEngine;
          m_poGrabEngine = 0;
          delete m_poConvertEngine;
          m_poConvertEngine = 0;
          m_bUseDMA = value == "on" ? true : false;
          init(); // creates a new grab engine
        }
      }else{
        ERROR_LOG("unable to set param dma to \"" << value << "\" (allowed values are \"on\" and \"off\")");
      }
      // }}}
    }else if(p == "grab mode"){
      // {{{ open
      if(value == "progressive" || value == "frame by frame"){
        if((value == "progressive" && !m_bProgressiveGrabMode) || (value == "frame by frame" && m_bProgressiveGrabMode)){
          ICL_DELETE(m_poGrabEngine);
          ICL_DELETE(m_poConvertEngine);
          m_bProgressiveGrabMode = value == "progressive" ? true : false;
          init(); // creates a new grab engine
        }
      }else{
        ERROR_LOG("unable to set param dma to \"" << value << "\" (allowed values are \"on\" and \"off\")");
      }
      // }}}
    }
    m_oMutex.unlock();
    
    // else search for properties
    vector<UnicapProperty> ps = m_oDevice.getProperties();
    for(unsigned int i=0;i<ps.size();i++){
      UnicapProperty &prop = ps[i];
      if(force_lower_case(prop.getID()) != p) continue;
      switch(prop.getType()){
        case UnicapProperty::range:{
          // {{{ open

          Range<double> range = prop.getRange();
          double val = atof(value.c_str());
          if(range.contains(val)){
            prop.setValue(val);
            m_oDevice.setProperty(prop);
            //      if(verbose) printf("UnicapGrabberImpl::setParam(%s=%s) [done]\n",param.c_str(),value.c_str());
          }else{
            printf("UnicapGrabberImpl::setParam() value %f is out of range [%f..%f]\n",val,range.minVal, range.maxVal);
          }
          break;
        }

          // }}}
        case UnicapProperty::valueList:{
          // {{{ open

          vector<double> valueList = prop.getValueList();
          double val = atof(value.c_str());
          bool foundValue = false;
          for(unsigned int j=0;j<valueList.size();j++){
            if(abs(valueList[j]-val)<0.00001){
              //              if(verbose) printf("UnicapGrabberImpl::setParam(%s=%s) [done]\n",param.c_str(),value.c_str());
              prop.setValue(valueList[j]);
              m_oDevice.setProperty(prop);
              foundValue = true;
              break;
            }
          }
          if(!foundValue){
            string valueListStr="{";
            char buf[50];
            for(unsigned int j=0;j<valueList.size();j++){
              sprintf(buf,"%f",valueList[j]);
              valueListStr += buf;
              if(j<valueList.size()-1){
                valueListStr+=",";
              }else{
                valueListStr+="}";
              }
            }
            // printf("UnicapGrabberImpl::setParam() value %f is not in value list %s \n",val,valueListStr.c_str());
          }
          break;
        }

          // }}}
        case UnicapProperty::menu:{
          // {{{ open
          vector<string> men = prop.getMenu();
          string val = force_lower_case(value);
          bool foundEntry = false;
          for(unsigned int j=0;j<men.size();j++){
            if(force_lower_case(men[j])==val){
              prop.setMenuItem(men[j]);
              m_oDevice.setProperty(prop);
              //  if(verbose) printf("UnicapGrabberImpl::setParam(%s=%s) [done]\n",param.c_str(),value.c_str());
              foundEntry = true;
            }
          }
          if(!foundEntry){
            printf("UnicapGrabberImpl::setParam() value is entry %s is not an valide menu entry \n",value.c_str());
            printf("menu={");
            for(unsigned int j=0;j<men.size();j++){
              printf("%s%s",men[j].c_str(),j<men.size()-1 ? "," : "}");
            }
            printf("\n");
          }
          break;
        }
        // }}}
        case UnicapProperty::data:{
          // {{{ open

          WARNING_LOG("setting up \"data\"-properties is not yet supported!");
          return;
          break;
        }

          // }}}
        case UnicapProperty::flags:{
          // {{{ open

          char *end = 0;
          prop.setFlags(strtoul(value.c_str(),&end,10));
          m_oDevice.setProperty(prop);
          break;
        }

          // }}}
        default: 
          ERROR_LOG("Unknown unicap property type ![code 8] ");
          break;
      }        
      break;
    }      
  }

  // }}}
  
  
  vector<string> UnicapGrabberImpl::getPropertyList(){
    // {{{ open

    vector<string> v;
    v.push_back("size");
    v.push_back("format");
    // [deprecated !] v.push_back("size&format");
    v.push_back("dma");
    v.push_back("grab mode");
    vector<UnicapProperty> ps = m_oDevice.getProperties();
    for(unsigned int i=0;i<ps.size();i++){
      v.push_back(ps[i].getID());
    }
    return v;
  }

  // }}}

  string UnicapGrabberImpl::getInfo(const string &name){
    // {{{ open

    if(name == "size"){
      // {{{ open

      UnicapFormat fmt= m_oDevice.getCurrentUnicapFormat();
      vector<Size> sizes = fmt.getPossibleSizes();
      if(sizes.size()){
        return sizeVecToStr(sizes);
        
      }else{
        Size others[] = {fmt.getSize(),fmt.getMinSize(),fmt.getMaxSize()};
        for(int i=0;i<3;i++){
          if(others[i] != Size::null && others[i] != Size(-1,-1)){
            return string("{\"")+str(others[i])+"\"}";
          }
        }
        return "{}";
      }      

      // }}}
    }else if(name == "format"){
      // {{{ open

      vector<UnicapFormat> fmts = m_oDevice.getFormats();
      if(!fmts.size()) return "{}";
      string s = "{";
      for(unsigned i = 0;i<fmts.size()-1;i++){
        s+=string("\"")+fmts[i].getID()+"\",";
      }
      return s+string("\"")+fmts[fmts.size()-1].getID()+"\"}";

      // }}}
    }else if(name == "size&format"){
      // {{{ open

      string s = "{";
      vector<UnicapFormat> fmts = m_oDevice.getFormats();
      for(unsigned i = 0;i<fmts.size();i++){
        vector<Size> sizes = fmts[i].getPossibleSizes();
        if(sizes.size()){
          for(unsigned int j=0;j<sizes.size();j++){
            s+=string("\"")+fmts[i].getID()+"&"+str(sizes[j])+"\"";
            if(i==fmts.size()-1 && j == sizes.size()-1){
              s+="}";
            }else{
              s+=",";
            }
          }
        }else{
          Size others[] = {fmts[i].getSize(),fmts[i].getMinSize(),fmts[i].getMaxSize()};
          for(int j=0;j<3;j++){
            if(others[j] != Size::null && others[j] != Size(-1,-1)){
              s+=string("\"")+fmts[i].getID()+"&"+str(others[j])+"\"";
              break;
            }
          }
          if(i==fmts.size()-1){
            s+="}";
          }else{
            s+=",";
          }
        }
      }
      return s;

      // }}}
    }else if(name == "dma"){
      // {{{ open
      return "{\"on\",\"off\"}";
      // }}}
    }else if(name == "grab mode"){
      return "{\"progressive\",\"frame by frame\"}";
    }else{ // checking all properties
      // {{{ open
      
      string t = getType(name);
      if(t == "undefined") return t;
      UnicapProperty p;
      vector<UnicapProperty> ps = m_oDevice.getProperties();
      bool found = false;
      for(unsigned int i=0;i<ps.size();i++){
        if(ps[i].getID() == name){
          p = ps[i];
          found = true;
          break;
        }
      }
      if(!found)return "undefined";
      if(t == "menu"){
        return Grabber::translateStringVec(p.getMenu());
      }else if(t == "range"){
        Range<double> r = p.getRange();
        return Grabber::translateSteppingRange(SteppingRange<double>(r.minVal,r.maxVal,p.getStepping()));
      }else if(t == "valueList"){
        return Grabber::translateDoubleVec(p.getValueList());
      }else{
        return "undefined";
      }
    }
    // }}}
  }

  // }}}
  
  string UnicapGrabberImpl::getType(const string &name){
    // {{{ open

    static map<UnicapProperty::type,string> *typeMap = 0;
    if(!typeMap){
      typeMap = new map<UnicapProperty::type,string>;
      (*typeMap)[UnicapProperty::valueList] = "valueList";
      (*typeMap)[UnicapProperty::menu]      = "menu";
      (*typeMap)[UnicapProperty::range]     = "range";
      (*typeMap)[UnicapProperty::flags]     = "undefined";
      (*typeMap)[UnicapProperty::data]      = "undefined";
      (*typeMap)[UnicapProperty::anytype]   = "undefined";
    }
    
    vector<UnicapProperty> ps = m_oDevice.getProperties();
    for(unsigned int i=0;i<ps.size();i++){
      if(ps[i].getID() == name){
        return (*typeMap)[ps[i].getType()];
      }
    }

    if(name == "size" || name == "format" || name == "format&size" || name == "dma" || name == "grab mode"){
      return "menu";
    }
    return "undefined";
  }

  // }}}
  
  string UnicapGrabberImpl::getValue(const std::string &name){
    // {{{ open

    // look for a specific property:
    vector<UnicapProperty> ps = m_oDevice.getProperties();
    for(unsigned int i=0;i<ps.size();i++){
      if(ps[i].getID() == name){
        char buf[30];
        switch(ps[i].getType()){
          case UnicapProperty::range:
          case UnicapProperty::valueList:
            sprintf(buf,"%f",ps[i].getValue());
            return buf;
          case UnicapProperty::menu:
            return ps[i].getMenuItem();
          default:
            return "undefined";
        }
      }
    }
    if(name == "size"){
      return str(m_oDevice.getCurrentSize());
    }else if(name == "format"){
      return m_oDevice.getFormatID();
    }else if(name == "size&format"){
      return str(m_oDevice.getCurrentSize())+"&"+m_oDevice.getFormatID();
    }else if(name == "dma"){
      if( m_bUseDMA ){
        return "on";
      }else{
        return "off";
      }
    }else if(name == "grab mode"){
      if(m_bProgressiveGrabMode){
        return "progressive";
      }else{
        return "frame by frame";
      }
    }else{
      return "undefined";
    }
  }

  // }}}

  const ImgBase* UnicapGrabberImpl::grabUD(ImgBase **ppoDst){
    // {{{ open

    ICLASSERT_RETURN_VAL(m_poGrabEngine , 0);

    if(!ppoDst) ppoDst = &m_poImage;  


    if(getIgnoreDesiredParams()){
      m_oMutex.lock();
      m_poGrabEngine->lockGrabber();

      // ---
      if(m_poGrabEngine->needsConversion()){
        const icl8u *rawData = m_poGrabEngine->getCurrentFrameUnconverted();
        m_poConvertEngine->cvtNative(rawData,ppoDst);
      }else{
        ERROR_LOG("unable to estimate hardware image parameters!\n"
                  "returning empty image with desired params");
        ensureCompatible(ppoDst,getDesiredDepth(),getDesiredParams());
        return *ppoDst;
      }

    
      // ---
      
      m_poGrabEngine->unlockGrabber();
      m_oMutex.unlock();
      updateFps();
      return *ppoDst;
    }else{
      ensureCompatible(ppoDst,getDesiredDepth(),getDesiredParams());
      
      // indicates whether a conversion to the desired parameters will be needed
      bool needFinalConversion = false; // assume, that no conversion is needed
      
      // get the image from the grabber
      m_oMutex.lock();
      m_poGrabEngine->lockGrabber();
      if(m_poGrabEngine->needsConversion()){
        const icl8u *rawData = m_poGrabEngine->getCurrentFrameUnconverted();
        if(m_poConvertEngine->isAbleToProvideParams(m_oDesiredParams,m_eDesiredDepth)){
          m_poConvertEngine->cvt(rawData,m_oDesiredParams,m_eDesiredDepth,ppoDst);
        }else{
          m_poConvertEngine->cvt(rawData,m_oDesiredParams,m_eDesiredDepth,&m_poConversionBuffer);
          needFinalConversion = true;
        }
      }else{
        if(m_poGrabEngine->isAbleToProvideParams(m_oDesiredParams,m_eDesiredDepth)){
          m_poGrabEngine->getCurrentFrameConverted(m_oDesiredParams,m_eDesiredDepth,ppoDst);
        }else{
          m_poGrabEngine->getCurrentFrameConverted(m_oDesiredParams,m_eDesiredDepth,&m_poConversionBuffer);
          needFinalConversion = true;
        }
      }
      m_poGrabEngine->unlockGrabber();
      
      
      /// 3rd step: the image, got by the Grab/Convert-Engine must not have the desired
      /// parameters, so: check and convert on demand
      if(needFinalConversion){
        m_oConverter.apply(m_poConversionBuffer,*ppoDst);
      }
      m_oMutex.unlock();
      updateFps();
      return *ppoDst;
    }
  }

  // }}}
  
  namespace{
    
    enum matchmode{
      eq, // == equal 
      in, // ~= contains
      rx  // *= regex-match
    };

    bool match_regex(const string &text,const string &patternIn){
      // {{{ open

      string patternSave = patternIn;
      char *pattern = const_cast<char*>(patternSave.c_str());
      int    status;
      regex_t    re;
      
      if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
        ERROR_LOG("error in regular expression " << patternIn);
        return false; 
      }
      status = regexec(&re, text.c_str(), (size_t) 0, NULL, 0);
      regfree(&re);
      return !status;
    }

    // }}}

    bool match_string(const string &text, const string &pattern, matchmode mode){
      switch(mode){
        case eq: return text == pattern;
        case in: return text.find(pattern,0) != string::npos;
        default: return match_regex(text,pattern);
      }
    } 
    
    struct ParamFilter{
      // {{{ open
      ParamFilter(const string &str,matchmode mode, unsigned int ui=0, unsigned long long ull=0):
        str(str),ui(ui),ull(ull),mode(mode){ }
      virtual ~ParamFilter(){}
      virtual bool ok(const UnicapDevice &d)=0;
      string str;
      unsigned int ui;
      unsigned long long ull;
      matchmode mode;
    };

    // }}}
    
    struct ParamFilterID : public ParamFilter{
      // {{{ open

      ParamFilterID(const string &id, matchmode mode):ParamFilter(str,mode){}
      virtual bool ok(const UnicapDevice &d){
        return match_string(d.getID(),str,mode);
      }
    };

    // }}}
    struct ParamFilterModelName : public ParamFilter{
      // {{{ open

      ParamFilterModelName(const string &mn, matchmode mode):ParamFilter(mn,mode){}
      virtual bool ok(const UnicapDevice &d){
        return match_string(d.getModelName(),str,mode);
      }
    };

    // }}}
    struct ParamFilterVendorName : public ParamFilter{
      // {{{ open

      ParamFilterVendorName(const string &vn, matchmode mode):ParamFilter(vn,mode){}
      virtual bool ok(const UnicapDevice &d){
        return match_string(d.getVendorName(),str,mode);
      }
    };

    // }}}
    struct ParamFilterModelID : public ParamFilter{
      // {{{ open

      ParamFilterModelID(unsigned long long mid, matchmode mode):ParamFilter("",mode,0,mid){}
      virtual bool ok(const UnicapDevice &d){
        return d.getModelID()==ull;
      }
    };

    // }}}
    struct ParamFilterVendorID : public ParamFilter{
      // {{{ open

      ParamFilterVendorID(unsigned int vid, matchmode mode):ParamFilter("",mode,vid){}
      virtual bool ok(const UnicapDevice &d){
        return d.getVendorID()==ui;
      }
    };

    // }}}
    struct ParamFilterCPILayer : public ParamFilter{
      // {{{ open

      ParamFilterCPILayer(const string &cpil, matchmode mode):ParamFilter(cpil,mode){}
      virtual bool ok(const UnicapDevice &d){
        return match_string(d.getCPILayer(),str,mode);
      }
    };

    // }}}
    struct ParamFilterDevice : public ParamFilter{
      // {{{ open

      ParamFilterDevice(const string &dev, matchmode mode):ParamFilter(dev,mode){}
      virtual bool ok(const UnicapDevice &d){
        return match_string(d.getDevice(),str,mode);
      }
    };
    
    // }}}
    struct ParamFilterFlags : public ParamFilter{
      // {{{ open
      
      ParamFilterFlags(unsigned int flags, matchmode mode):ParamFilter("",mode,flags){}
      virtual bool ok(const UnicapDevice &d){
        return d.getFlags()==ui;
      }
    };

    // }}}
   

    void filter_devices(const vector<UnicapDevice> &src, 
                        vector<UnicapDevice> &dst, const string &filter){
      // {{{ open

      dst.clear();
      const std::vector<string> toks = tok(filter,"\n");
      vector<ParamFilter*> filters;
      for(unsigned int i=0;i<toks.size();++i){
        static const char* apcOps[] = {"==","~=","*="};
        static const matchmode aeModes[] = {eq,in,rx};
        
        size_t pos = string::npos;
        matchmode mode;
        string id,value;
        for(int j=0;j<3;j++){
          if((pos=toks[i].find(apcOps[j],0)) != string::npos){
            id = toks[i].substr(0,pos);
            value = toks[i].substr(pos+2,toks[i].size()-pos-1);
            mode = aeModes[j];
            printf("filter: id=[%s] mode=[%s] value=[%s]\n ",id.c_str(),apcOps[j],value.c_str());
            break;
          }
        }
        if(pos ==  string::npos){
          WARNING_LOG("unknown filter operator token in \""<< toks[i] <<"\"");
          continue;
        }
        if(id == "id"){
          filters.push_back(new ParamFilterID(value,mode));
        }else if (id == "modelname"){
          filters.push_back(new ParamFilterModelName(value,mode));
        }else if (id == "vendorname"){
          filters.push_back(new ParamFilterVendorName(value,mode));
        }else if (id == "modelid"){
          filters.push_back(new ParamFilterModelID(atol(value.c_str()),mode));
        }else if (id == "vendorid"){
          filters.push_back(new ParamFilterVendorID((unsigned int)atol(value.c_str()),mode));
        }else if (id == "cpilayer"){
          filters.push_back(new ParamFilterCPILayer(value,mode));
        }else if (id == "device"){
          filters.push_back(new ParamFilterDevice(value,mode));
        }else if (id == "flags"){
          filters.push_back(new ParamFilterFlags((unsigned int )atol(value.c_str()),mode));
        }else{
          WARNING_LOG("unknown filter id \"" << id << "\"");
        }
      }
      for(unsigned int i=0;i<src.size();++i){
        bool ok = true;
        for(unsigned int j=0;j<filters.size();++j){
          if(!filters[j]->ok(src[i])){
            ok = false;
            break;
          }
        }
        if(ok) dst.push_back(src[i]);
      }
      for(unsigned int j=0;j<filters.size();++j){
        delete filters[j];
      }
    }    
    // }}}
  }  


  
  vector<UnicapDevice>
  UnicapGrabberImpl::getDeviceList(const string &filter){
    // {{{ open

    static std::vector<UnicapDevice> vCurrentDevices;
    // rebuild current list of available devices
    vCurrentDevices.clear();

    for(int i=0;true;++i){
       UnicapDevice d(i);
       if(!d.isValid()) break;

       vCurrentDevices.push_back(d);
    }
    return filterDevices (vCurrentDevices, filter);
  }

  // }}}
  
  /* Returning a real vector instead of a const reference to some static variable
     here allows to apply several filterDevices-calls in series */
  vector<UnicapDevice>
  UnicapGrabberImpl::filterDevices(const std::vector<UnicapDevice> &devices, 
                               const string &filter){
    // {{{ open

    std::vector<UnicapDevice> vResult;
    filter_devices(devices,vResult,filter);
    return vResult;
  }

  // }}}

}

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
