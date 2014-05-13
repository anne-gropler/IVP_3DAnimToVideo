#include "viewerqtcgs.h"

#include <QProgressDialog>
#include <osg/GL>
#include "osggraphicswindowqt.h"

#include <iostream>

namespace VIEWEROSG{

ViewerQTCgs::ViewerQTCgs( QWidget * parent /*= 0*/, osg::ref_ptr<osg::GraphicsContext::Traits> traits /*= 0*/, const QGLWidget * shareWidget /*= 0*/, Qt::WindowFlags f /*= 0*/ ) : 
    osgViewer::Viewer(), OSGAdapterWidget(OSGGraphicsWindowQt::createQGLFormat(traits), parent, shareWidget,f)
{
    init();
}

ViewerQTCgs::ViewerQTCgs( const QGLFormat& format, QWidget* parent /*= 0*/, const QGLWidget* shareWidget /*= 0*/, Qt::WindowFlags f /*= 0*/ ) :
    osgViewer::Viewer(), OSGAdapterWidget(format, parent, shareWidget,f)
{
    init();
}

void ViewerQTCgs::init() {
    setGraphicsWindow(new OSGGraphicsWindowQt(0, this));

    connect(&_timer, SIGNAL(timeout()), this , SLOT(updateGL()));

    getCamera()->setViewport(new osg::Viewport(0,0,width(),height()));
    getCamera()->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(width())/static_cast<double>(height()), 1.0f, 10000.0f);
    getCamera()->setGraphicsContext(getGraphicsWindow());


    osgViewer::Viewer::ThreadingModel threadingModel = osgViewer::Viewer::suggestBestThreadingModel(); // needs to be done after GraphicsContext is set (otherwise it will be SingleThreaded even if several CPU Cores exist)
    // [TODO] Application still crashes with multi-threaded under linux :(
    threadingModel = osgViewer::Viewer::SingleThreaded;

    if (threadingModel != osgViewer::Viewer::SingleThreaded)
        threadedInitializeGL();

    setThreadingModel(threadingModel);

    _timer.start(10);

    m_inFocus = true;
    m_busy = false;
}

void ViewerQTCgs::pause( bool on )
{
    m_busy = on;
    if(m_inFocus)
    {
        if (on)
        {
            _timer.stop();
        } 
        else
        {
            _timer.start(10);
        }
    }
}

void ViewerQTCgs::updateGL()
{
    if (!isHidden()) {
        frame();
    }
}

void ViewerQTCgs::setThreadingModel(ThreadingModel threadingModel) {
    if(osgViewer::Viewer::SingleThreaded != threadingModel) {
        threadedInitializeGL();
    }
    osgViewer::Viewer::setThreadingModel(threadingModel);
}

ViewerQTCgs::~ViewerQTCgs() {
}

osg::Image* ViewerQTCgs::largeScaleSnapshot(int width, int height, bool orthogonalProjection, bool withAlphaChannel) {
    osg::Image* image = new osg::Image;
    image->allocateImage(
        width,
        height,
        1,
        withAlphaChannel ? GL_RGBA : GL_RGB,
        GL_UNSIGNED_BYTE);
    if (!largeScaleSnapshot(image, orthogonalProjection)) {
        image->ref();
        image->unref();
        image = 0;
    }
    return image;
}

bool ViewerQTCgs::largeScaleSnapshot(osg::Image *image, bool orthogonalProjection) {
    unsigned int imageWidth = image->s();
    unsigned int imageHeight = image->t();
    osg::ref_ptr<osg::Camera> oldCamera = getCamera();
    unsigned int viewportWidth = oldCamera->getViewport()->width();
    unsigned int viewportHeight = oldCamera->getViewport()->height();

    // For now, fail, if image is smaller than current viewport (that's why we're called "large" snapshot...)
    if (imageWidth < viewportWidth || imageHeight < viewportHeight) {
        return false;
    }

    unsigned int tileWidth = qMin(viewportWidth, imageWidth);
    unsigned int tileHeight = qMin(viewportHeight, imageHeight);

    QProgressDialog progress("Taking large screenshot", "Abort", 0, imageWidth * imageHeight, this);
    progress.setValue(0);

    osg::ref_ptr<osg::Image> tile = new osg::Image;
    tile->allocateImage(tileWidth, tileHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE);

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setClearColor(getCamera()->getClearColor());
    camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera->setCullMask(getCamera()->getCullMask());
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setRenderOrder(osg::Camera::PRE_RENDER);
    camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    camera->setViewport(0, 0, tileWidth, tileHeight);
    camera->attach(osg::Camera::COLOR_BUFFER, tile, 8, 8);
    osg::Matrixd projectionMatrix = oldCamera->getProjectionMatrix();
    if (orthogonalProjection) {
        double left, right, top, bottom, zNear, zFar;
        projectionMatrix.getOrtho(left, right, bottom, top, zNear, zFar);
        projectionMatrix.makeOrtho(bottom * imageWidth/imageHeight, top * imageWidth/imageHeight, bottom, top, zNear, zFar);
    }
    camera->setViewMatrix(oldCamera->getViewMatrix());
    camera->addChild(getSceneData());
    getCamera()->addChild(camera);

    // start the tiling process
    const double viewPortPartX = static_cast<double>(tileWidth) / static_cast<double>(imageWidth);
    const double viewPortPartY = static_cast<double>(tileHeight) / static_cast<double>(imageHeight);

    for (unsigned int y = 0; y < imageHeight; y += tileHeight) {
        for (unsigned int x = 0; x < imageWidth; x += tileWidth) {
            progress.setValue(x + y * imageWidth);
            if (progress.wasCanceled()) {
                break;
            }
            osg::Matrixd scaling = osg::Matrixd::scale(1.0 / viewPortPartX, 1.0 / viewPortPartY, 1.0);
            const osg::Matrixd translation = osg::Matrixd::translate(
                -((double)x/(double)imageWidth - 0.5) * 2.0/viewPortPartX - 1.0,
                -((double)y/(double)imageHeight - 0.5) * 2.0/viewPortPartY - 1.0,
                0.0);
            camera->setProjectionMatrix(projectionMatrix * (scaling * translation));

            renderingTraversals();

            // Copy tile image data to our real image
            const unsigned int w = qMin(x + tileWidth, imageWidth) - x;
            const unsigned int h = qMin(y + tileHeight, imageHeight) - y;
            for (unsigned int tx = 0; tx < w; tx++)
            {
                for (unsigned int ty = 0; ty < h; ty++)
                {
                    unsigned char* src = tile->data(tx, ty);
                    unsigned char* target = image->data(x + tx, y + ty);
                    for ( int u=0; u<4; ++u )
                        *(target + u) = *(src++);
                }
                progress.setValue(x + y * imageWidth + tx * h);
            }
        }
    }
    progress.setValue(imageWidth * imageHeight);

    getCamera()->removeChild(camera);
    return !progress.wasCanceled();
}

void ViewerQTCgs::setInFocus( bool inFocus )
{
    m_inFocus = inFocus;

    if(!m_busy)
    {
        if (inFocus)
        {
            _timer.start(10);
        } 
        else
        {
            _timer.stop();
        }
    }
}

}//end namespace VIEWEROSG
