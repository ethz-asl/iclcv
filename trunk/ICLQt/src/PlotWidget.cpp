/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/PlotWidget.cpp                               **
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

#include <ICLQt/PlotWidget.h>
#include <ICLUtils/SmartArray.h>

namespace icl{
  
  
  struct PlotWidget::Data{
    struct Buffer : SmartArray<float>{
      Buffer(){}
      Buffer(int len):SmartArray<float>(new float[len]), len(len), used(false){}
      int len;
      bool used;
    };
    std::vector<Buffer> buffers;
    
    void freeBuffers(){
      for(unsigned int i=0;i<buffers.size();++i){
        buffers[i].used = false;
      }
      if(buffers.size() > 20) buffers.resize(20);
    }
    
    float *getBuffer(int len){
      for(unsigned int i=0;i<buffers.size();++i){
        if(!buffers[i].used && buffers[i].len >= len){
          buffers[i].used = true;
          return buffers[i];
        }
      }
      buffers.push_back(Buffer(len));
      buffers.back().used = true;
      return buffers.back().get();
    }
    
    
    AbstractPlotWidget::Pen state;
    std::string name;
  };
  
  PlotWidget::PlotWidget(QWidget *parent):
    LowLevelPlotWidget(parent),m_data(new Data){
  }
  
  PlotWidget::~PlotWidget(){
    delete m_data;
  }
  
  void PlotWidget::clear::clear() {
    PlotWidget::clear();
    m_data->freeBuffers();
  }
  void PlotWidget::name(const std::string &nextName){
    m_data->name = nextName;
  }
  
  void PlotWidget::color(int r, int g, int b, int a=255){
    m_data->state.linePen.setColor(a==255 ? QColor(r,g,b) : QColor(r,g,b,a));
    m_data->state.symbolPen.setColor(a==255 ? QColor(r,g,b) : QColor(r,g,b,a));
  }
  void PlotWidget::pen(const QPen &pen){
    m_data->linePen = m_data->symbolPen = pen;
  }  

  void PlotWidget::fill(int r, int g, int b, int a=255){
    m_data->state.fillBrush = QBrush(a==255 ? QColor(r,g,b) : QColor(r,g,b,a));
  }
  void PlotWidget::brush(const QBrush &brush){
    m_data->state.fillBrush = brush;
  }

  void PlotWidget::sym(char s){
    m_data->state.symbol = s;
  }
  
  void PlotWidget::linewidth(float width){
    m_data->state.linePen.setWidthF(width);
    m_data->state.sybolPen.setWidthF(width);
  }
  
  void PlotWidget::symsize(float size){
    m_data->state.symbolSize = size;
  }
  
  template<class T>
  void PlotWidget::scatter(const T *xs, const T *ys, int num, int xStride, int yStride, bool connect){
    float *b = m_data->getBuffer(2*num);
    float *startB = b;
    float *endB = b + 2*num;
    while(b<endB){
      *b++ = *xs;
      *b++ = *ys;
      xs += xStride;
      ys += yStride;
    }
    const QColor c = m_data->state.symbolPen.color();
    
    addScatterData(m_data->state.symbol,
                   startB, startB+1, num, m_data->name,
                   c.red(), c.green(), c.blue(),
                   m_data->state.symbolSize,
                   connect, 2, 2, false, false, false);
  }


  template<class T>
  void PlotWidget::series(const T *data, int num, int stride){
    float *b = m_data->getBuffer(num);
    if(stride ==1){
      std::copy(data,data+num, b);
    }else{
      for(int i=0;i<num;++i){
        b[i] =  data[i*stride];
      }
    }
    addSeriesData(b,num,new AbstractPlotWidget::Pen(m_data->state), m_data->name, 1, false, false);
  }

  template<class T>
  void PlotWidget::bars(const T *data, int num, int stride){
    float *b = m_data->getBuffer(num);
    if(stride ==1){
      std::copy(data,data+num, b);
    }else{
      for(int i=0;i<num;++i){
        b[i] =  data[i*stride];
      }
    }
    addBarPlotData(b,num,new AbstractPlotWidget::Pen(m_data->state), m_data->name, 1, false, false);
  }

  /*

const char type,const float *data, int num=1, 
                        const QPen &linePen = QColor(255,0,0),
                        const QBrush &brush = Qt::NoBrush,
                        const std::string &text="", const std::string &textDelim=","

*/

  void PlotWidget::line(const Point32f &a, const Point32f &b){
    const float data[] = { a.x, a.y, b.x, b.y };
    addAnnotations('l',data, 1, m_data->state.linePen, Qt::NoBrush, m_data->text, '\0'); 
  }

  void PlotWidget::point(const Point32f &p){
    const float data[] = { a.x, a.y, b.x, b.y };
    addAnnotations('l',data, 1, m_data->state.linePen, Qt::NoBrush, m_data->text, '\0'); 
    
  }
  
  void PlotWidget::linestrip(const std::vector<Point32f> &ps, bool closedLoop=true){
    linestrip(&ps[0].x, &ps[0].y, ps.size(), closedLoop);
  }
  void PlotWidget::linestrip(const std::vector<Point> &ps, bool closedLoop=true){}
  void PlotWidget::linestrip(const Point32f *ps, int num, bool closedLoop=true){}
  linestrip(&ps[0].x, &ps[0].y, num, closedLoop);
  void PlotWidget::linestrip(const Point *ps, int num, bool closedLoop=true){
    std::vector<Point32f> data(2*(num-1) + closedLoop*4);
    int idx;
    for(int i=1;i<num;++i){
      data[idx++] = ps[i-1];
      data[idx++] = ps[i];
    }
    if(closedLoop){
      data[idx++] = ps[num-1];
      data[idx++] = ps[0];
    }
    addAnnotations('l',data, 1, m_data->state.linePen, Qt::NoBrush, m_data->text, '\0');
  }
  void PlotWidget::linestrip(const float *xs, const float *ys, int num, bool closedLoop=true){
    
  }
  
  
  void PlotWidget::rect(const Point32f &ul, const Point32f &lr){}
  void PlotWidget::rect(const Rect &r){}
  void PlotWidget::rect(const Rect32f &r){}
  void PlotWidget::rect(float x, float y, float w, float h){}
  
  void PlotWidget::circle(const Point32f &c, float r){
  
  }
  void PlotWidget::circle(float cx, float cy, float r){
  
  }
  
  void PlotWidget::text(float x, float y, const std::string &text){}
  void PlotWidget::text(const Point32f &p, const std::string &text){}
  
}
