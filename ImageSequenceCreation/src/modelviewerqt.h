#pragma once

#include <QMainWindow>
#include "ui_modelviewerqt.h"
#include <osg/ref_ptr>
#include <osgViewer/Viewer>
#include "viewerosg/viewerqtcgs.h"
#include <Qt>
#include <osg/AnimationPath>

class ModelViewerQt : public QMainWindow
{
    Q_OBJECT

public:
    ModelViewerQt(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~ModelViewerQt();

protected slots:
    virtual void leaveEvent(QEvent* event);
    virtual void enterEvent(QEvent* event);

protected:
    Ui::QtModelViewerClass ui;
    //VIEWEROSG::ViewerQTCgs* m_viewer;
    osgViewer::Viewer* m_viewer;
	osg::ref_ptr<osg::Group> m_root;

    bool m_focusDetectionEnabled;
	void initializeGeometry();
	void loadObjectWithAnimation();
	void loadObjectWithoutAnimation(bool drawBoundingSphereAtSide);
	void drawBoundingSphere(osg::NodePath &nodepath);
	void drawBoundingBox(osg::Node* node);

	osg::AnimationPath* createAnimationPathSphere(osg::Vec3f &center, float radius, double looptime, osg::Vec3 &up);
};
