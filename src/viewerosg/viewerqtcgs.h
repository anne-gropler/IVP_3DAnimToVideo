#pragma once

#include <osg/GL>
#include "osgadapterwidget.h"

#include <osgViewer/Viewer>
#include <QtCore/QTimer>

namespace VIEWEROSG{

//! @short A specialised CGS::GUI::QtViewer.
//! A sublass of CGS::GUI::QtViewer, adding the possibility to pause the scene's updating process.
class ViewerQTCgs : public OSGAdapterWidget, public osgViewer::Viewer
{
    public:
        //! Constructor.
        //! If no traits parameter is passed, osg::DisplaySettings will be used for the configuration of the OpenGL context
        ViewerQTCgs(QWidget * parent = 0, osg::ref_ptr<osg::GraphicsContext::Traits> traits = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0);

        //! Constructor.
        //! Uses specified QGLFormat to configure OpenGL context of QGLWidget, ignoring osg::DisplaySettings
        ViewerQTCgs(const QGLFormat& format, QWidget* parent = 0, const QGLWidget* shareWidget = 0, Qt::WindowFlags f = 0);

        virtual ~ViewerQTCgs();

        virtual void updateGL();

        virtual void setThreadingModel(ThreadingModel threadingModel);

        QWidget* asWidget() { return this; }

        osg::Image* largeScaleSnapshot(int width, int height, bool orthogonalProjection = false, bool withAlphaChannel = true);
        bool largeScaleSnapshot(osg::Image* image, bool orthogonalProjection = false);

        //! This method pauses or restarts the scene's updating process.
        //! @param on. A boolean value indicating whether to pause (true) or restart (false) the updating process.
        void pause(bool on);

        //! This method defines whether the viewer is focused.
        //! @param inFocus A boolean value indicating whether the viewer lies in focus
        void setInFocus(bool inFocus);

    protected:
        //! Indicates whether the viewer is focused and therefore paused
        bool m_inFocus;
        //! Indicates whether the viewer is busy, e.g. loading data and therefore paused
        bool m_busy;

        QTimer _timer;

        void init();
};

}//end namespace VIEWEROSG


// forbid osg::ref_ptr usage of the QtViewer, because Qt will hardcore delete this object anyway
namespace osg {
    template<> class ref_ptr<VIEWEROSG::ViewerQTCgs>;
}
