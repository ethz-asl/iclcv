#include <iclFSliderHandle.h>
#include <QSlider>

namespace icl{
  FSliderHandle::FSliderHandle():GUIHandle<QSlider>(0),
    m_fMin(0),m_fMax(0),m_iSliderRange(0){
  }
  FSliderHandle::FSliderHandle(QSlider *sl,float minV, float maxV, int range):
    GUIHandle<QSlider>(sl),m_fMin(minV),m_fMax(maxV),m_iSliderRange(range){
    updateMB();
  }
  void FSliderHandle::setMin(float min){
    m_fMin = min;
    updateMB();
  }
  void FSliderHandle::setMax(float max){
    m_fMax = max;
    updateMB();
  }

  void FSliderHandle::setValue(float val){
    (**this)->setValue(f2i(val));
  }
  
  float FSliderHandle::getMin() const{
    return m_fMin;
  }
  float FSliderHandle::getMax() const{
    return m_fMax;
  }
  float FSliderHandle::getValue() const{
    return i2f((**this)->value());
  }

  
}
