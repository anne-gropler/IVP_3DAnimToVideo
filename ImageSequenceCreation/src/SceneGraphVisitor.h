#ifndef SCENEGRAPHVISITOR_h__
#define SCENEGRAPHVISITOR_h__

#include <osg/NodeVisitor>
#include <osg/Transform>
#include <osg/Camera>
#include <iostream>



class SceneGraphVisitor: public osg::NodeVisitor
{
public:
    SceneGraphVisitor(bool spam = false);
    ~SceneGraphVisitor();

    virtual void apply(osg::Geode& geode);
    virtual void apply(osg::Node& node);
    virtual void apply(osg::Group& group);
    virtual void apply(osg::Transform& trans);
    virtual void apply(osg::Camera& camera);

    osg::BoundingBox getOverallBoundingBox(){return m_bb;}; 

    std::string spaces();

private:
    unsigned int _level;
    osg::BoundingBox m_bb;
    bool m_spam;
};

#endif // SCENEGRAPHVISITOR_h__
