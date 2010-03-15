/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQuick/Common.h                              **
** Module : ICLQuick                                               **
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
*********************************************************************/

#ifdef HAVE_QT
#include <ICLQt/GUI.h>
#include <ICLQt/QtMacros.h>
#include <ICLQt/Application.h>
#endif

#include <ICLUtils/Thread.h>
#include <ICLUtils/StringUtils.h>
#include <ICLQuick/Quick.h>
#include <ICLIO/GenericGrabber.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/Mutex.h>
#include <ICLIO/FileWriter.h>



