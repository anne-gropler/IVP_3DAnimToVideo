/* -*-c++-*-
* Stealth Viewer - OSGGraphicsWindowQt (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2007-2008, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
*
* David Guthrie
*/

#include <osg/GL>
//#include <cgs/gui/GLWidgetFactory.h>
#include <iostream>
#include "osggraphicswindowqt.h"
#include "osgadapterwidget.h"

namespace VIEWEROSG{

        ////////////////////////////////////////////////////////////
        OSGGraphicsWindowQt::OSGGraphicsWindowQt(osg::GraphicsContext::Traits* traits,
            //CGS::GUI::GLWidgetFactory* factory, 
            OSGAdapterWidget* adapter)
            : BaseClass()
            , mValid(false)
            , mRealized(false)
            , mCloseRequested(false) 
            , mQWidget(NULL)
            , mCursorShape(Qt::ArrowCursor)
        {
            if (traits) 
            {
                _traits = traits;
            }
            else
            {
                _traits = new GraphicsContext::Traits(osg::DisplaySettings::instance());
            }

            QGLWidget* sharedContextWidget = NULL;
            if (_traits->sharedContext != NULL)
            {
                OSGGraphicsWindowQt* sharedWin = dynamic_cast<OSGGraphicsWindowQt*>(_traits->sharedContext.get());
                if (sharedWin != NULL)
                {
                    sharedContextWidget = sharedWin->getQGLWidget();
                }
                else
                {
                    std::cout << "A shared context was specified, but it is not a QGLWidget based context, so it can't be shared." << std::endl;            
                }
            }

            if (adapter == NULL)
            {
                Qt::WindowFlags flags = NULL;
                if (!traits->windowDecoration)
                {
                    flags |= Qt::FramelessWindowHint;
                }

                QGLFormat format = OSGGraphicsWindowQt::createQGLFormat(_traits);

                //if (factory != NULL)
                //{
                //  adapter = factory->CreateWidget(format, false, NULL, sharedContextWidget, flags);
                //}
                //else
                //{
                adapter = new OSGAdapterWidget(format, /*false,*/ NULL, sharedContextWidget, flags);
                //}
            }

            adapter->setGraphicsWindow(this);
            adapter->setFocusPolicy(Qt::StrongFocus);
            setQGLWidget(adapter);
        }

        ////////////////////////////////////////////////////////////
        OSGGraphicsWindowQt::~OSGGraphicsWindowQt()
        {
            setQGLWidget(NULL);
        }

        ////////////////////////////////////////////////////////////
        QGLFormat OSGGraphicsWindowQt::createQGLFormat(osg::ref_ptr<osg::GraphicsContext::Traits> traits) {
            if (!traits.valid()) {
                traits = new osg::GraphicsContext::Traits(osg::DisplaySettings::instance());

                // TODO: shouldn't this be set by the Traits constructor?
                traits->doubleBuffer = osg::DisplaySettings::instance()->getDoubleBuffer(); 

                traits->vsync = false; // turn off by default because Qt is lagging otherwise
            }

            QGLFormat format = QGLFormat::defaultFormat();
            format.setAlpha(traits->alpha > 0);
            format.setAlphaBufferSize(traits->alpha);
            format.setDepth(traits->depth > 0);
            format.setDepthBufferSize(traits->depth);
            format.setDoubleBuffer(traits->doubleBuffer);
            format.setStencil(traits->stencil > 0);
            format.setStencilBufferSize(traits->stencil);
            format.setSamples(traits->samples);
            format.setSampleBuffers(traits->sampleBuffers);
            format.setSwapInterval(traits->vsync ? 1 : 0);

            format.setRedBufferSize(traits->red);
            format.setGreenBufferSize(traits->green);
            format.setBlueBufferSize(traits->blue);
            format.setStereo(traits->quadBufferStereo); 

            return format;

            // unused:
            //traits->x;
            //traits->y;
            //traits->width;
            //traits->height;
            //traits->windowName;
            //traits->supportsResize;
            //traits->pbuffer;
            //traits->target;
            //traits->level;
            //traits->face;
            //traits->mipMapGeneration;
            //traits->useMultiThreadedOpenGLEngine;
            //traits->useCursor;
            //traits->glContextVersion;
            //traits->glContextFlags; // -> setOptions?
            //traits->glContextProfileMask;
            //traits->sharedContext;
            //traits->inheritedWindowData;
            //traits->overrideRedirect;
            //traits->swapMethod;
            //traits->windowDecoration;
        }

        ////////////////////////////////////////////////////////////
        void OSGGraphicsWindowQt::setQGLWidget(QGLWidget* qwidget)
        {
            if (mQWidget != NULL && getState() != NULL)
            {
                mQWidget = NULL;
                decrementContextIDUsageCount(getState()->getContextID());
                setState(NULL);
            }

            mQWidget = qwidget;
            mValid = mQWidget != NULL;

            if (valid())
            {
                setState( new osg::State );
                getState()->setGraphicsContext(this);

                if (_traits.valid() && _traits->sharedContext.get())
                {
                    getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
                    incrementContextIDUsageCount( getState()->getContextID() );
                }
                else
                {
                    getState()->setContextID( osg::GraphicsContext::createNewContextID() );
                }
            }
        }

        ////////////////////////////////////////////////////////////
        QGLWidget* OSGGraphicsWindowQt::getQGLWidget()
        {
            return mQWidget;
        }

        ////////////////////////////////////////////////////////////
        const QGLWidget* OSGGraphicsWindowQt::getQGLWidget() const
        {
            return mQWidget;
        }

        ////////////////////////////////////////////////////////////
        bool OSGGraphicsWindowQt::isSameKindAs(const Object* object) const
        {
            return dynamic_cast<const OSGGraphicsWindowQt*>(object)!=0;
        }

        ////////////////////////////////////////////////////////////
        const char* OSGGraphicsWindowQt::libraryName() const
        {
            return "SteathQt";
        }

        ////////////////////////////////////////////////////////////
        const char* OSGGraphicsWindowQt::className() const
        {
            return "OSGGraphicsWindowQt";
        }

        ////////////////////////////////////////////////////////////
        bool OSGGraphicsWindowQt::valid() const
        {
            return mValid;
        }

        ////////////////////////////////////////////////////////////
        bool OSGGraphicsWindowQt::realizeImplementation() //
        {
            if (mQWidget != NULL)
            {
                //mQWidget->show();

                // without doneCurrent() multi-threaded rendering didn't work with the old context sharing implementation
                //mQWidget->makeCurrent();
                //mQWidget->doneCurrent();

                mRealized = true;
            }
            else
            {
                mRealized = false;
            }
            return mRealized;
        }

        ////////////////////////////////////////////////////////////
        bool OSGGraphicsWindowQt::isRealizedImplementation() const
        {
            return mRealized;
        }

        ////////////////////////////////////////////////////////////
        void OSGGraphicsWindowQt::closeImplementation()
        {
            // the QWidget might be already deleted and we do not own it, therefore we do nothing here
            mQWidget = NULL;
        }

        ////////////////////////////////////////////////////////////
        bool OSGGraphicsWindowQt::makeCurrentImplementation()
        {
            if (mQWidget != NULL)
            {
                if (!mQWidget->isVisible())
                {
                    return false;
                }

                // TODO: change mQWidget into a OSGAdapterWidget?
                OSGAdapterWidget* adapter = dynamic_cast<OSGAdapterWidget*>(mQWidget);
                if (adapter)
                    adapter->threadedMakeCurrent();
                else {
                    mQWidget->makeCurrent();
                    osg::notify(osg::WARN) << "Warning - the QGLWidget used in OSGGraphicsWindowQt is not a OSGAdapterWidget - multithreaded rendering may not work because OpenGL context cannot be shared correctly between the threads" << std::endl;
                }


                return true;
            }
            return false;
        }

        ////////////////////////////////////////////////////////////
        bool OSGGraphicsWindowQt::releaseContextImplementation()
        {
            if (mQWidget != NULL)
            {
                //mQWidget->doneCurrent(); 

                OSGAdapterWidget* adapter = dynamic_cast<OSGAdapterWidget*>(mQWidget);
                if (adapter)
                    adapter->threadedMakeCurrent();
                return true;
            }
            return false;
        }

        ////////////////////////////////////////////////////////////
        void OSGGraphicsWindowQt::swapBuffersImplementation()
        {
            if (mQWidget != NULL)
            {
                mQWidget->swapBuffers();
            }
        }

        ////////////////////////////////////////////////////////////
        bool OSGGraphicsWindowQt::checkEvents()
        {
            if (mCloseRequested)
                getEventQueue()->closeWindow();
            return !(getEventQueue()->empty());
        }

        ////////////////////////////////////////////////////////////
        void OSGGraphicsWindowQt::getWindowRectangle(int& x, int& y, int& width, int& height)
        {
            if (mQWidget != NULL)
            {
                QRect r = mQWidget->geometry();
                x = r.left();
                y = r.top();
                width = r.width();
                height = r.height();
            }
        }


        ////////////////////////////////////////////////////////////
        bool OSGGraphicsWindowQt::setWindowRectangleImplementation(int x, int y, int width, int height)
        {
            if (mQWidget != NULL)
            {
                mQWidget->setGeometry(x, y, width, height);
                return true;
            }
            return false;
        }

        ////////////////////////////////////////////////////////////
        bool OSGGraphicsWindowQt::setWindowDecorationImplementation(bool flag)
        {
            if (mQWidget != NULL)
            {
                // this cas be done but there are some quirks with it.
                //mQWidget->setWindowFlags();
                //mQWidget->show();
                return true;
            }
            return false;
        }

        ////////////////////////////////////////////////////////////
        void OSGGraphicsWindowQt::grabFocus()
        {
            if (mQWidget != NULL)
            {
                mQWidget->setFocus();
            }
        }

        ////////////////////////////////////////////////////////////
        void OSGGraphicsWindowQt::grabFocusIfPointerInWindow()
        {
            //TODO fix this so it checks for the pointer.
            if (mQWidget != NULL)
            {
                mQWidget->setFocus();
            }
        }

        ////////////////////////////////////////////////////////////
        void OSGGraphicsWindowQt::requestClose()
        {
            mCloseRequested = true;
        }

        ////////////////////////////////////////////////////////////
        void OSGGraphicsWindowQt::resizedImplementation(int x, int y, int width, int height)
        {
            BaseClass::resizedImplementation(x, y, width, height);
            if (mQWidget != NULL)
            {
                mQWidget->resize(width, height);
            }
        }

        ////////////////////////////////////////////////////////////
        void OSGGraphicsWindowQt::setWindowName (const std::string& name)
        {
            if (mQWidget != NULL)
            {
                mQWidget->setWindowTitle(name.c_str());
            }
        }

        ////////////////////////////////////////////////////////////
        void OSGGraphicsWindowQt::useCursor(bool cursorOn)
        {
            if (mQWidget != NULL)
            {
                if (cursorOn)
                {
                    mQWidget->setCursor(QCursor(mCursorShape));
                }
                else
                {
                    mQWidget->setCursor(QCursor(Qt::BlankCursor));
                }
            }
        }


        ////////////////////////////////////////////////////////////
        using osgViewer::GraphicsWindow;
        void OSGGraphicsWindowQt::setCursor(osgViewer::GraphicsWindow::MouseCursor mouseCursor)
        {

            if (mQWidget != NULL)
            {
                switch (mouseCursor)
                {
                case InheritCursor:
                    mCursorShape = Qt::LastCursor;
                    break;
                case NoCursor:
                    mCursorShape = Qt::BlankCursor;
                    break;
                case RightArrowCursor:
                    mCursorShape = Qt::ArrowCursor;
                    break;
                case LeftArrowCursor:
                    break;
                case InfoCursor:
                    break;
                case DestroyCursor:
                    break;
                case HelpCursor:
                    mCursorShape = Qt::WhatsThisCursor;
                    break;
                case CycleCursor:
                    break;
                case SprayCursor:
                    break;
                case WaitCursor:
                    mCursorShape = Qt::WaitCursor;
                    break;
                case TextCursor:
                    break;
                case CrosshairCursor:
                    mCursorShape = Qt::CrossCursor;
                    break;
                case UpDownCursor:
                    mCursorShape = Qt::UpArrowCursor;
                    break;
                case LeftRightCursor:
                    break;
                case TopSideCursor:
                    break;
                case BottomSideCursor:
                    break;
                case LeftSideCursor:
                    break;
                case RightSideCursor:
                    break;
                case TopLeftCorner:
                    break;
                case TopRightCorner:
                    break;
                case BottomRightCorner:
                    break;
                case BottomLeftCorner:
                    break;
                default:
                    ;
                }
                mQWidget->setCursor(mCursorShape);
            }
        }

        ////////////////////////////////////////////////////////////
        void OSGGraphicsWindowQt::requestWarpPointer(float x, float y)
        {
            mQWidget->cursor().setPos(mQWidget->mapToGlobal(QPoint(x,y)));
            getEventQueue()->mouseWarped(x,y);
        }
}
