#include "modelviewerqt.h"

#include <osg/Node> 
#include <osg/Group> 
#include <osg/Geode> 
#include <osg/Geometry> 
#include <osg/Texture2D> 
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>  
#include <osgDB/FileUtils>
#include <osgViewer/Viewer> 
#include <osg/PositionAttitudeTransform> 

#include <osgGA/TrackballManipulator>

#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/BasicAnimationManager>


ModelViewerQt::ModelViewerQt(QWidget *parent, Qt::WindowFlags flags)
: QMainWindow(parent, flags)
{
    ui.setupUi(this);

    m_focusDetectionEnabled = true;

    // enable drag'n'drop
    this->setAcceptDrops(true);

    osg::DisplaySettings::instance()->setNumMultiSamples(8);
    osg::DisplaySettings::instance()->setDoubleBuffer(true);

    m_viewer = new VIEWEROSG::ViewerQTCgs(this);
    this->setCentralWidget(m_viewer);

	this->initializeGeometry();
	
	
	// switch off lighting as we haven't assigned any normals. 
    //m_root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF); 
	
	m_viewer->setCameraManipulator(new osgGA::TrackballManipulator()); 
	//m_viewer->getCamera()->setAllowEventFocus(false);
	//m_viewer->getCamera()->setViewMatrixAsLookAt(osg::Vec3(0,-120,0), osg::Vec3(0,0,0), osg::Vec3(0,0,1)); 

	//The final step is to set up and enter a simulation loop. 
    m_viewer->setSceneData(m_root);
	
}

ModelViewerQt::~ModelViewerQt()
{
}

void ModelViewerQt::leaveEvent( QEvent* event )
{
    if(m_focusDetectionEnabled)
    {
        m_viewer->setInFocus(false);
    }
}

void ModelViewerQt::enterEvent( QEvent* event )
{
    if(m_focusDetectionEnabled)
    {
        m_viewer->setInFocus(true);
    }
}

void ModelViewerQt::initializeGeometry()
{
	this->loadObjectWithAnimation();
}

void ModelViewerQt::loadObjectWithAnimation(){

	osg::Group* root = new osg::Group(); 
	// convenience variables
	//osg::Node* bvh = osgDB::readNodeFile( osgDB::findDataFile( "untitled.dae" ) );
	osg::Node* bvh = osgDB::readNodeFile( "data/inputObjects/untitled.dae");
	//osg::Node* model = osgDB::readNodeFile( osgDB::findDataFile( "test.dae" ) );
	osgAnimation::BasicAnimationManager* manager = new osgAnimation::BasicAnimationManager;

	//get animation from bvh
	osgAnimation::AnimationManagerBase* animationManager = dynamic_cast<osgAnimation::AnimationManagerBase*>(bvh->getUpdateCallback());
	osgAnimation::Animation* anim = animationManager->getAnimationList()[0];
	anim->setPlayMode( osgAnimation::Animation::LOOP );
   
	osg::Group* group = new osg::Group;
	group->setUpdateCallback( manager );
	group->addChild( bvh ); //model

	manager->registerAnimation(anim);
	manager->buildTargetReference();
	manager->playAnimation( anim );

	root->addChild(group);
	m_root = root;
}