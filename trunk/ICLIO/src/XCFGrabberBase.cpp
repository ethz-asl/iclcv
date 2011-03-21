/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/XCFGrabberBase.cpp                           **
** Module : ICLIO                                                  **
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

#ifdef HAVE_XCF
#include <ICLIO/XCFGrabberBase.h>
#include <xmltio/xmltio.hpp>

#include <ICLIO/XCFUtils.h>

using namespace std;
using namespace xmltio;

namespace icl {


  XCFGrabberBase::XCFGrabberBase(): m_result(0), m_poSource(0)
#if 0
                                  ,m_poBayerBuffer(0),m_poDesiredParamsBuffer(0)
#endif                               
  {
    
#if 0
    m_poBayerConverter = new BayerConverter(BayerConverter::simple,
                                            BayerConverter::bayerPattern_RGGB);
#endif
  }
  
  XCFGrabberBase::~XCFGrabberBase(){
    ICL_DELETE(m_poSource);

#if 0
    ICL_DELETE(m_poBayerConverter);
    ICL_DELETE(m_poBayerBuffer);
    ICL_DELETE(m_poDesiredParamsBuffer);
#endif

  }
  
#if 0
  void XCFGrabberBase::makeOutput (const xmltio::Location& l, ImgBase *poOutput) {
    xmltio::LocationPtr locBayer = xmltio::find (l, "PROPERTIES/@bayerPattern");

    if (locBayer) {
      const std::string& bayerPattern =  xmltio::extract<std::string>(*locBayer);
      ImgParams p = m_poSource->getParams();
      p.setFormat (formatRGB);
      m_poBayerBuffer = icl::ensureCompatible (&m_poBayerBuffer, m_poSource->getDepth(), p);
      
      m_poBayerConverter->setBayerPattern(BayerConverter::translateBayerPattern(bayerPattern));
      
      m_poBayerConverter->apply(m_poSource->asImg<icl8u>(), &m_poBayerBuffer);
      m_oConverter.apply (m_poBayerBuffer, poOutput);
    } else {
      m_oConverter.apply (m_poSource, poOutput);
    }
  }
#endif

  const ImgBase* XCFGrabberBase::acquireImage(){
     receive (m_result);
     
     LocationPtr loc = xmltio::find(xmltio::Location(m_result->getXML()), 
                                    "//IMAGE[@uri]");
     
     if(loc){
       if(!xmltio::find (*loc, "PROPERTIES/@bayerPattern")){
         WARNING_LOG("bayer support is not yet implemented for this class");
       }
       XCFUtils::CTUtoImage(m_result, *loc, &m_poSource);
       return m_poSource;
     }else{
       ERROR_LOG("unable to find XPath: \"//IMAGE[@uri]\"");
       return 0;
     }
   }

#if 0
   void XCFGrabberBase::grab (std::vector<ImgBase*>& vGrabbedImages) {
      receive (m_result);
      static bool first = true;
      if(first){
        first = false;
        ERROR_LOG("please note that mulitiple image grabbing does not support to ignore grabbers desired parameters!");
      }
      xmltio::Location doc (m_result->getXML());
      vector<ImgBase*>::iterator imgIt  = vGrabbedImages.begin();
      vector<ImgBase*>::iterator imgEnd = vGrabbedImages.end();
      unsigned int nCount = 0;
      for (XPathIterator imLoc = XPath("//IMAGE[@uri]").evaluate(doc);
           imLoc; ++imLoc) {
         ImgBase *poOutput=0;
         if (imgIt != imgEnd) {
            // use existing images
            poOutput = prepareOutput (&(*imgIt));
            ++imgIt;
         } else {
            // create new image
            poOutput = 0;
            poOutput = prepareOutput (&poOutput);
            vGrabbedImages.push_back (poOutput);
         }
         XCFUtils::CTUtoImage(m_result, *imLoc, &m_poSource);
         makeOutput (*imLoc, poOutput);
         nCount++;
      }
      // shrink vGrabbedImages to number of actually grabbed images
      vGrabbedImages.resize (nCount);
   }
#endif
   
}
#endif
