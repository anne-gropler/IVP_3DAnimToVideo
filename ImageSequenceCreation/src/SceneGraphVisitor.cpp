#include "SceneGraphVisitor.h"


#include <osg/Geode>

SceneGraphVisitor::SceneGraphVisitor(bool spam):NodeVisitor(NodeVisitor::NODE_VISITOR, NodeVisitor::TRAVERSE_ALL_CHILDREN),
_level(0),
m_spam(spam)
{
    this->setTraversalMode(TRAVERSE_ALL_CHILDREN);
    m_bb = osg::BoundingBox();
    m_bb._min = osg::Vec3(10000.0,10000.0, 100000.0);
    m_bb._max = osg::Vec3(-10000.0,-10000.0, -100000.0);
}

SceneGraphVisitor::~SceneGraphVisitor()
{
}

void SceneGraphVisitor::apply(osg::Geode& geode)
{
    if(m_spam)
        std::cout << spaces() << geode.className() << " " << geode.getName() << std::endl;
    _level++;
    m_bb.expandBy(geode.getBoundingBox());
    traverse(geode);
    _level--;
}

void SceneGraphVisitor::apply(osg::Node& node)
{
    if(m_spam)
        std::cout << spaces() << node.className() << " " << node.getName() << std::endl;
    _level++;
    traverse(node);    
    _level--;
}

void SceneGraphVisitor::apply(osg::Group& group)
{
    if(m_spam)
        std::cout << spaces() << group.className() << " " << group.getName() << std::endl;
    _level++;
    traverse(group);
    _level--;
}

void SceneGraphVisitor::apply(osg::Transform& trans )
{
    if(m_spam)
        std::cout << spaces() << trans.className() << " " << trans.getName() << std::endl;
    _level++;
    traverse(trans);
    _level--;
}

void SceneGraphVisitor::apply(osg::Camera& camera )
{
    if(m_spam)
        std::cout << spaces() << camera.className() << " " << camera.getName() << std::endl;
    _level++;
    traverse(camera);
    _level--;
}

std::string SceneGraphVisitor::spaces()
{
    return std::string(_level*2, ' ');
}

