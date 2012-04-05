/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/Widget.h                                 **
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

#ifndef ICL_WIDGET_H
#define ICL_WIDGET_H

#include <QtOpenGL/QGLWidget>
#include <ICLCore/ImgBase.h>
#include <ICLCore/Types.h>
#include <ICLQt/ImageStatistics.h>
#include <ICLQt/MouseHandler.h>
#include <ICLQt/WidgetCaptureMode.h>
#include <ICLQt/GUI.h>
#include <ICLUtils/Function.h>

namespace icl{

  /** \cond */
  class PaintEngine;
  class OSDGLButton;
  /** \endcond */
 
  /// Class for openGL-based image visualization components \ingroup COMMON
  /** The ICLWidget class provide basic abilities for displaying ICL images (ImgBase) on embedded 
      Qt GUI components. Its is fitted out with a responsive OpenGL-Overlay On-Screen-Display (OSD),
      which can be used to adapt some ICLWidget specific settings, and which is partitioned into several
      sub menus:
      - <b>adjust</b> here one can define image adjustment parameters via sliders, like brightness and
        contrast. In addition, there are three different modes: <em>off</em> (no adjustments are 
        preformed), <em>manual</em> (slider values are exploited) and <em>auto</em> (brightness 
        and intensity are adapted automatically to exploit the full image range). <b>Note:</b> This time,
        no intensity adaptions are implemented at all.
      - <b>fitmode</b> Determines how the image is fit into the widget geometry (see enum 
        ICLWidget::fitmode) 
      - <b>channel selection</b> A slider can be used to select a single image channel that should be 
        shown as gray image 
      - <b>capture</b> Yet, just a single button is available to save the currently shown image into the
        local directory 
      - <b>info</b> shows information about the current image
      - <b>menu</b> settings for the menu itself, like the alpha value or the color (very useful!)
      
      The following code (also available in <em>ICLQt/examples/camviewer_lite.cpp</em> demonstrates how
      simple an ICLWidget can be used to create a simple USB Webcam viewer:
      \code
#include <ICLQuick/Common.h>

ICLWidget *widget = 0;
GenericGrabber grabber;

// initialization method (called in the main thread)
void init(){
  grabber.init(FROM_PROGARG("-i"));
  widget = new ICLWidget;
  widget->setGeometry(200,200,640,480);   
  widget->show();         
}

// working method (running looped in the working thread)
void run(){
  widget->setImage(grabber.grab());
  // don't use update() which is not thread-safe!
  widget->updateFromOtherThread();
}

int main(int n, char **args){
  return ICLApp(n,args,"[m]-input|-i(2)",init,run).exec();
}
      \endcode

      When other things, like annotations should be drawn to the widget, you can either
      create a new class that inherits the ICLWidget class, and reimplement the function:
      \code
      virtual void customPaintEvent(PaintEngine *e);
      \endcode
      or, you can use the extended ICLDrawWidget class, which enables you to give 2D draw
      commands in image coordinates. (The last method is more recommended, as it should be
      exactly what you want!!!)
      @see ICLDrawWidget

      \section ICLWIDGET_OSM ICLWidget's On-Screen-Menu

    <TABLE border=0><TR><TD>
    TODO ...
    </TD><TD>
    \image html osm-0.png "On-screen-menu's sub-tabs"
    </TD></TR></TABLE>

    <TABLE border=0><TR><TD>
    TODO ...
    </TD><TD>
    \image html osm-2.png "On-screen-menu's 'bci'-tab. Here, brightness and contrast adaption mode can be set up here."
    </TD></TR></TABLE>


    <TABLE border=0><TR><TD>
    TODO ...
    </TD><TD>
    \image html osm-3.png "On-screen-menu's 'scale'-tab. This tab can be used to adapt how the image is set into the widget."
    </TD></TR></TABLE>

    <TABLE border=0><TR><TD>
    TODO ...
    </TD><TD>
    \image html osm-4.png "On-screen-menu's 'channel'-tab. Here the widget can be set up to visualize a single image channel only."
    </TD></TR></TABLE>

    <TABLE border=0><TR><TD>
    TODO ...
    </TD><TD>
    \image html osm-5.png "On-screen-menu's 'capture'-tab. This most complex tab is provided to capture the current image or the current frame-buffer. Furthermore, automatic capturing can be activated here."
    </TD></TR></TABLE>

    <TABLE border=0><TR><TD>
      TODO ...
    </TD><TD>
    \image html osm-6.png "On-screen-menu's 'histo'-tab. Here, an online image histogram is shown."
    </TD></TR></TABLE>

  */
  class ICLWidget : public QGLWidget{
    Q_OBJECT
    public:
    /// just used internally 
    friend class OSDGLButton;

    class Data;   
    class OutputBufferCapturer;
    
    /// determines how the image is fit into the widget geometry
    enum fitmode{ 
      fmNoScale=0,  /**< the image is not scaled it is centered to the image rect */
      fmHoldAR=1,   /**< the image is scaled to fit into the image rect, but its aspect ratio is hold */
      fmFit=2,      /**< the image is fit into the frame ( this may change the images aspect ratio)*/
      fmZoom=3      /**< new mode where an image rect can be specified in the gui ... */
    };
    
    /// determines intensity adaption mode 
    enum rangemode { 
      rmOn = 1 ,  /**< range settings of the sliders are used */ 
      rmOff = 2 , /**< no range adjustment is used */
      rmAuto      /**< automatic range adjustment */ 
    };   
    
    /// creates a new ICLWidget within the parent widget
    ICLWidget(QWidget *parent=0);
    
    /// destructor
    virtual ~ICLWidget();

    /// GLContext initialization
    virtual void initializeGL();
    
    /// called by resizeEvent to adapt the current GL-Viewport
    virtual void resizeGL(int w, int h);
    
    /// draw function
    virtual void paintGL();

    /// drawing function for NO-GL fallback
    virtual void paintEvent(QPaintEvent *e);
    
    /// this function can be overwritten do draw additional misc using the given PaintEngine
    virtual void customPaintEvent(PaintEngine *e);

    virtual void setVisible(bool visible);

    /// sets the current fitmode
    void setFitMode(fitmode fm);
    
    /// sets the current rangemode
    void setRangeMode(rangemode rm);
    
    /// set up current brightness, contrast and intensity adaption values
    void setBCI(int brightness, int contrast, int intensity);

    
    /// returns the widgets size as icl::Size
    Size getSize() { return Size(width(),height()); }
    
    /// returns the current images size
    Size getImageSize(bool fromGUIThread=false);

    /// returns the rect, that is currently used to draw the image into
    Rect getImageRect(bool fromGUIThread=false);

    /// returns current fit-mode
    fitmode getFitMode();
        
    /// returns current range mode
    rangemode getRangeMode();

    /// returns a list of image specification string (used by the OSD)
    std::vector<std::string> getImageInfo();

    
    /// adds a new mouse handler via signal-slot connection
    /** Ownership is not passed ! */
    void install(MouseHandler *h);

    /// deletes mouse handler connection 
    /** Ownership was not passed -> h is not deleted  */
    void uninstall(MouseHandler *h);

    /// registers a simple callback 
    /** @param cb callback functor to use
        @param eventList comma-separated list of events. Supported types are:
               - all (for all events)
               - move (mouse if moved, no button pressed)
               - drag  (mouse is moved, at least one button is pressed)
               - press, (guess what)
               - release (button released)
               - enter  (mouse cursor enters the widget)
               - leave  (mouse cursor leaved the widget)
    */
    void registerCallback(const GUI::Callback &cb, const std::string &eventList="drag,press");

    /// removes all callbacks registered using registerCallback
    void removeCallbacks();

    /// this function should be called to update the widget asyncronously from a working thread
    void updateFromOtherThread();
    
    /// overloaded event function processing special thread save update events
    virtual bool event(QEvent *event);
    
    /// returns current ImageStatistics struct (used by OSD)
    const ImageStatistics &getImageStatistics();

    /// if the menu is disabled, there will be no menu button at the top left of the widget
    void setMenuEnabled(bool enabled);
    
    /// This can be used in order to hide to label at the lower right image rect
    void setImageInfoIndicatorEnabled(bool enabled);
    
    /// sets wheather to notify, that no image was set
    void setShowNoImageWarnings(bool showWarnings);
    
    /// Sets a viewport size that is used if no image was set
    /** if no image was set, then the OpenGL viewport is adapted as if there was an image with this size.
        If the given size is Size::null  the viewport is not adated */
    void setViewPort(const Size &size);

    /// Adds a new toggle-button to the OSD-button bar on the upper widget edge
    /** Special buttons can directly be attached to specific ICLWidget slots, 
        furthermore special button- clicks and toggle events are notified using
        the ICLWidget-signals specialButtonClicked and specialButtonToggled.
        
        @param id handle to reference the button lateron 
        
        @param untoggledIcon optional button icon(recommeded: use buttons from the
               ICLQt::IconFactory class)

        @param toggledIcon optional button icon (recommeded: use buttons from the
               ICLQt::IconFactory class)

        @param ICLWidgetSlot here you can direcly define a slot from ICLWidget
               class, the button is attached to. Note, if toggable is true,
               it has to be a foo(bool)-slot, otherwise you'll need a foo(void)
               slot.
    */
    void addSpecialToggleButton(const std::string &id, 
                                const ImgBase* untoggledIcon = 0, 
                                const ImgBase *toggledIcon = 0, 
                                bool initiallyToggled = 0, 
                                const Function<void,bool> &cb=Function<void,bool>(),
                                const std::string &toolTipText="");

    /// Adds a new toggle-button to the OSD-button bar on the upper widget edge
    /** @see addSpecialToggleButton */
    void addSpecialButton(const std::string &id, 
                          const ImgBase* icon = 0, 
                          const Function<void> &cb=Function<void>(),
                          const std::string &toolTipText="");

    
    /// removes special button with given ID
    void removeSpecialButton(const std::string &id);
    
    public slots:
    /// sets up the current image
    void setImage(const ImgBase *image);

    signals:
    /// invoked when any mouse interaction was performed
    void mouseEvent(const MouseEvent &event);

    
    void specialButtonClicked(const std::string &id);
    void specialButtonToggled(const std::string &id, bool down);


    public:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void enterEvent(QEvent *e);
    virtual void leaveEvent(QEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void wheelEvent(QWheelEvent *e);
    virtual void hideEvent(QHideEvent *e);
    /// listens for F11 which enables the fullscreen mode
    virtual void keyPressEvent(QKeyEvent *event);
    
    public slots:
    void showHideMenu();
    void setMenuEmbedded(bool embedded);

    void bciModeChanged(int modeIdx);
    void brightnessChanged(int val);
    void contrastChanged(int val);
    void intensityChanged(int val);
    
    void scaleModeChanged(int modeIdx);
    void currentChannelChanged(int modeIdx);

    void captureCurrentImage();
    void captureCurrentFrameBuffer();
    
    void recordButtonToggled(bool checked);
    void pauseButtonToggled(bool checked);
    void stopButtonClicked();
    void skipFramesChanged(int frameSkip);
    void menuTabChanged(int index);
    void histoPanelParamChanged();
    
    void setEmbeddedZoomModeEnabled(bool enabled);
    
    void setLinInterpolationEnabled(bool enabled);

    void setShowPixelGridEnabled(bool enabled);

    void setRangeModeNormalOrScaled(bool enabled);
    
    void showBackgroundColorDialog();
    void showGridColorDialog();
    void setGridAlpha(int alpha);
    void setBackgroundBlack();
    void setBackgroundWhite();
    void setBackgroundGray();
    void setGridBlack();
    void setGridWhite();
    void setGridGray();
    
    private:
    /// internally used, grabs the current framebuffer as Img8u 
    const Img8u &grabFrameBufferICL();
    
    /// internal utility function
    std::string getImageCaptureFileName();
    
    /// internal utility function
    void updateInfoTab();

    /// just internally used
    void rebufferImageInternal();

    /// Internal data class (large, so it's hidden)
    Data *m_data;

    /// creates internal event instance
    const MouseEvent &createMouseEvent(MouseEventType type);
  };
  
}
#endif
