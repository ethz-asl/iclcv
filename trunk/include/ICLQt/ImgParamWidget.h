/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/ImgParamWidget.h                         **
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

#pragma once

#include <QtGui/QWidget>
#include <QtCore/QString>
#include <ICLCore/Types.h>

/** \cond **/
class QComboBox;
class QHBoxLayout;
/** \endcond **/

namespace icl{
  namespace qt{
    
    /// Internally used widget to define image params \ingroup UNCOMMON
    class ImgParamWidget : public QWidget{
      Q_OBJECT
      public:
      ImgParamWidget(QWidget *parent);
      void doEmitState();
      void getParams(int &width, int &height, int &d, int &fmt) const;
      
      private slots:
      void sizeChanged(const QString &val);
      void formatChanged(const QString &val);
      void depthChanged(const QString &val);
  
      void setup(int width, int height, int d, int core::format);
      
      signals:
      void somethingChanged(int width, int height, int d, int core::format);
  
      
      private:
      QComboBox *m_poSizeCombo;
      QComboBox *m_poDepthCombo;
      QComboBox *m_poFormatCombo;
      QHBoxLayout *m_poLayout;
      int m_iWidth,m_iHeight,m_iDepth, m_iFormat;
    };
  } // namespace qt
}

