#include "modelviewerqt.h"

#include <osg/Node> 
#include <osg/Group> 
#include <osg/Geode> 
#include <osg/Geometry> 
#include <osg/Texture2D> 
#include <osg/PolygonMode>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>  
#include <osgDB/FileUtils>
#include <osgViewer/Viewer> 
#include <osg/PositionAttitudeTransform> 

#include <osgGA/TrackballManipulator>

#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/BasicAnimationManager>

#include <osg/ComputeBoundsVisitor>
#include <osgwTools/Shapes.h>


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
	
	
	//switch off lighting as we haven't assigned any normals. 
    //m_root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF); 
	
	//m_viewer->setCameraManipulator(new osgGA::TrackballManipulator()); 
	m_viewer->getCamera()->setAllowEventFocus(false);
	m_viewer->getCamera()->setViewMatrixAsLookAt(osg::Vec3(0,120,0), osg::Vec3(0,0,0), osg::Vec3(0,0,1)); 

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
	m_root = new osg::Group(); 
	this->loadObjectWithAnimation();
}

void ModelViewerQt::loadObjectWithAnimation()
{
	osg::Node* object = osgDB::readNodeFile("data/inputObjects/bigMonkey.dae");
	
	osg::Group* group = new osg::Group;
	group->addChild(object);

	//get animation from object
	osgAnimation::BasicAnimationManager* manager = new osgAnimation::BasicAnimationManager;
	osgAnimation::AnimationManagerBase* animationManager = dynamic_cast<osgAnimation::AnimationManagerBase*>(object->getUpdateCallback());
	
	if (animationManager == nullptr)
	{
		osg::BoundingSphere sphere = object->computeBound();
		m_viewer->getCamera()->setViewMatrixAsLookAt(osg::Vec3(0,4*sphere._radius,0), sphere._center, osg::Vec3(0,0,1)); 

		osg::MatrixTransform* trans = new osg::MatrixTransform();

		osg::AnimationPathCallback* apc = new osg::AnimationPathCallback(createAnimationPathSphere(sphere._center, sphere._radius, 6, osg::Vec3f(0,0,1)), 0, 1);
		trans->setUpdateCallback(apc);
		trans->addChild(group);
		m_root->addChild(trans);
	}
	else
	{	
		osgAnimation::Animation* anim = animationManager->getAnimationList()[0];
		anim->setPlayMode(osgAnimation::Animation::LOOP);
   
	
		group->setUpdateCallback(manager);
		group->addChild(object);

		manager->registerAnimation(anim);
		manager->buildTargetReference();
		manager->playAnimation(anim);

		m_root->addChild(group);
	}
}

void ModelViewerQt::loadObjectWithoutAnimation(bool drawBoundingSphereAtSide)
{
	osg::Node* object = osgDB::readNodeFile( "data/inputObjects/bigMonkey.dae");
	
   	osg::Group* group = new osg::Group;
	group->addChild(object);

	osg::BoundingSphere sphere = object->computeBound();

	if (drawBoundingSphereAtSide)
	{
		osg::Sphere* drawSphere = new osg::Sphere(sphere._center, sphere._radius);
		osg::ShapeDrawable* sphereDrawable = new osg::ShapeDrawable(drawSphere);
		osg::PositionAttitudeTransform* sphereXForm = new osg::PositionAttitudeTransform();
		sphereXForm->setPosition(osg::Vec3(sphere._radius,0,0));
		osg::Geode* unitSphereGeode = new osg::Geode();
		m_root->addChild(sphereXForm);
		sphereXForm->addChild(unitSphereGeode);
		unitSphereGeode->addDrawable(sphereDrawable);
	}

	m_viewer->getCamera()->setViewMatrixAsLookAt(osg::Vec3(0,4*sphere._radius,0), sphere._center, osg::Vec3(0,0,1)); 

	osg::MatrixTransform* trans = new osg::MatrixTransform();

	osg::AnimationPathCallback* apc = new osg::AnimationPathCallback(createAnimationPathSphere(sphere._center, sphere._radius, 6, osg::Vec3f(0,0,1)), 0, 1);
    trans->setUpdateCallback(apc);
	trans->addChild(group);
	m_root->addChild(trans);
}


void ModelViewerQt::drawBoundingBox(osg::Node* node)
{
  //initialize the visitor, matrixtransform and geode
  osg::ref_ptr<osg::ComputeBoundsVisitor> cbv = new osg::ComputeBoundsVisitor();
  osg::ref_ptr<osg::MatrixTransform> boundingBoxMt = new osg::MatrixTransform();
  osg::ref_ptr<osg::Geode> boundingBoxGeode = new osg::Geode();
  osg::BoundingBox geodeBoundingBox;
  //attach the compute bounds visitor to the node
  node->accept( *cbv );

  osg::BoundingBox bb( cbv->getBoundingBox() );
  //set the matrix for the transform
  boundingBoxMt->setMatrix( osg::Matrix::translate( geodeBoundingBox.center() ) );
  //get the extents of the bounding box, call osgWorks function to draw
  osg::Vec3 ext( bb._max - bb._min );
  boundingBoxGeode->addDrawable( osgwTools::makeWireBox( ext * 0.5 ) );
  //add geode under Mt
  boundingBoxMt->addChild(boundingBoxGeode);
  //add Mt to subgraph
  m_root->addChild(boundingBoxMt);

  boundingBoxMt->removeChildren(0, boundingBoxMt->getNumChildren());

  return;
} 

void ModelViewerQt::drawBoundingSphere(osg::NodePath &nodePath)
{
  //use node path to get world matrix
  osg::Matrix worldMatrix = osg::computeLocalToWorld(nodePath);
  //init matrix transform

  osg::ref_ptr<osg::MatrixTransform> boundingBoxMt = new osg::MatrixTransform();
  boundingBoxMt->postMult(worldMatrix);
  //init geode

  osg::ref_ptr<osg::Geode> boundingBoxGeode = new osg::Geode();

  //apply state set for bb geode
  osg::StateSet* ss = boundingBoxGeode->getOrCreateStateSet();
  ss->setAttributeAndModes( new osg::PolygonMode(  osg::PolygonMode::FRONT_AND_BACK,  osg::PolygonMode::LINE ) );
  ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF ); 

  //last node in the node path corresponds to the current node the visitor is at
  const osg::BoundingSphere bs( nodePath.back()->getBound() );
  //boundingBoxGeode->addDrawable( osgwTools::makeGeodesicSphere( bs._radius, 1 ) );    
  //add geode under Mt
  boundingBoxMt->addChild(boundingBoxGeode);

  //add Mt to subgraph
  //add to the root (if world matrix is right, this should place it correctly?)
  m_root->addChild(boundingBoxMt); 
  //place the bounding box under the Matrixtransform for the 'current' node
  nodePath.back()->getParent(0)->asTransform()->addChild(boundingBoxMt);
 
  return;
}


osg::AnimationPath* ModelViewerQt::createAnimationPathSphere(osg::Vec3 &center, float radius, double looptime, osg::Vec3 &up)
{
    osg::AnimationPath* animationPath = new osg::AnimationPath();

    animationPath->setLoopMode(osg::AnimationPath::LOOP);

	radius = 0;

    int numSamples = 40;
    float yaw = 0.0f;
    float yaw_delta = 2.0f * osg::PI / ((float)numSamples - 1.0f);
    float roll = osg::inDegrees(0.0f);

    double time = 0.0f;
    double time_delta = looptime / ( double )numSamples;
    for(int i = 0; i < numSamples; ++i)
    {
        osg::Vec3 position(center + osg::Vec3(sinf(yaw) * radius, cosf(yaw) * radius, 0.0f));
        osg::Quat rotation(osg::Quat(roll, osg::Vec3(0.0, 1.0, 0.0)) * osg::Quat(-(yaw + osg::inDegrees(90.0f)), osg::Vec3( 0.0, 0.0, 1.0)));

        animationPath->insert(time, osg::AnimationPath::ControlPoint(position, rotation));

        yaw += yaw_delta;
        time += time_delta;
    }
    return animationPath;
}