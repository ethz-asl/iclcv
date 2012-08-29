/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/examples/filter-array.cpp                    **
** Module : ICLFilter                                              **
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

#include <ICLQt/Common.h>

#include <ICLFilter/UnaryOp.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLFilter/MorphologicalOp.h>
#include <map>

HSplit gui;
int N = 0;



std::string get_filters(){
  return cat(UnaryOp::listFromStringOps(),",");
}

GUI gui_col(int i){
  std::string si = str(i);
  return ( VBox()
           << Image().handle("im"+si).minSize(8,6)
           << ( HBox().maxSize(100,3)
                << Combo(get_filters()).handle("cb"+si).maxSize(100,2).minSize(3,2).label("filter")
                << String("").label("params").handle("ps"+si).maxSize(100,2).minSize(4,2)
                << CheckBox("vis",true).maxSize(3,2).out("vis"+si).minSize(3,2)
                )
           << ( HBox().maxSize(100,3)
                << Label("ok").handle("err"+si).maxSize(100,2).label("error").minSize(1,2)
                << Label("9999ms").label("dt").handle("dt"+si).size(2,3)
                << Label("unknown").handle("syn"+si).maxSize(100,2).label("syntax").minSize(1,2)
                ) 
           );
}

void init(){
  N = pa("-n");
#ifdef OLD_GUI
         gui << ( GUI("vbox") 
                  << "image[@handle=input@minsize=8x6]"
                  << (GUI("hbox[@label=desired params@maxsize=100x4]") 
                      << "combo(QVGA,!VGA,SVGA,XGA,WXGA,UXGA)[@out=_dsize@handle=dsize@label=size]"
                      << "combo(!depth8u,depth16s,depth32s,depth32f,depth64f)[@out=_ddepeth@handle=ddepth@label=size]"
                      << "combo(gray,!rgb,hls,lab,yuv)[@out=_dformat@handle=dformat@label=format]"));
#endif
  
  gui << ( VBox() 
           << Image().handle("input").minSize(8,6)
           << (HBox().label("desired params").maxSize(100,4) 
               << Combo("QVGA,!VGA,SVGA,XGA,WXGA,UXGA").handle("dsize").label("size")
               << Combo("!depth8u,depth16s,depth32s,depth32f,depth64f").handle("ddepth").label("size")
               << Combo("gray,!rgb,hls,lab,yuv").handle("dformat").label("format")
               )
           );
         
  for(int i=0;i<N;++i){
    gui << gui_col(i);
  }

  gui << Show();
}

void run(){
  static GenericGrabber g(pa("-i"));
  g.useDesired(parse<depth>(gui["ddepth"]));
  g.useDesired(parse<Size>(gui["dsize"]));
  g.useDesired(parse<format>(gui["dformat"]));
  
  const ImgBase *image = g.grab();
  gui["input"] = image;
  std::vector<UnaryOp*> ops;
  for(int i=0;i<N;++i){
    Time t = Time::now();
    std::string si = str(i);
    std::string opName = gui["cb"+si].as<std::string>();
    std::string params = gui["ps"+si].as<std::string>();
    
    UnaryOp *op = 0;
    gui["syn"+si] = UnaryOp::getFromStringSyntax(opName);
    try{
      op = UnaryOp::fromString(params.size() ? (opName+"("+params+")") : opName);
      op->setClipToROI(false);
      ops.push_back(op);
      gui["err"+si] = str("ok"); 
    }catch(const ICLException &ex){
      gui["err"+si] = str(ex.what());
    }
    if(op && image && gui["vis"+si].as<bool>()){
      try{
        image = op->apply(image);
      }catch(const ICLException &ex){
        gui["err"+si] = str(ex.what());
      }
      gui["dt"+si] = str((Time::now()-t).toMilliSeconds())+"ms";
      gui["im"+si] = image;
    }
  }
  for(unsigned int i=0;i<ops.size();++i){
    delete ops[i];
  }
}

int main(int n, char **ppc){
  paex("-input","image source definition like -input dc 0")
      ("-n","number of filter instances in a row (3 by default)");
  
  return ICLApplication(n,ppc,"[m]-input|-i(2) -num-filters|-n(int=3)",init,run).exec();
}

