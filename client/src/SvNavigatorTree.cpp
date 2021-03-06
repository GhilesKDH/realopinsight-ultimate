/*
 * SvNavigatorTree.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
#                                                                          #
# This file is part of RealOpInsight (http://RealOpInsight.com) authored   #
# by Rodrigue Chakode <rodrigue.chakode@gmail.com>                         #
#                                                                          #
# RealOpInsight is free software: you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# The Software is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with RealOpInsight.  If not, see <http://www.gnu.org/licenses/>.   #
#--------------------------------------------------------------------------#
 */

#include "SvNavigatorTree.hpp"
#include "Parser.hpp"
#include "utilsCore.hpp"
#include <QtGui>

SvNavigatorTree::SvNavigatorTree(CoreDataT* _cdata, const bool& _enableDrag, QWidget* _parent)
  : QTreeWidget(_parent),
    m_cdata(_cdata)
{
  setHeaderLabel(tr("Explorer"));
  setColumnCount(1);
  setDragDropMode(QAbstractItemView::DragDrop);
  setDragEnabled(_enableDrag);
}

void SvNavigatorTree::dropEvent(QDropEvent * _event)
{
  QTreeWidgetItem* targetTreeNode = itemAt(_event->pos());
  if (targetTreeNode && m_cdata) {
    NodeListT::iterator  targetNode = m_cdata->bpnodes.find(targetTreeNode->data(0, QTreeWidgetItem::UserType).toString());
    if (targetNode != m_cdata->bpnodes.end()) {
      if (targetNode->type != NodeType::BusinessService) {
        ngrt4n::alert(tr("Action not allowed on %1").arg(NodeType::toString(targetNode->type)));
      } else {
        _event->setDropAction(Qt::MoveAction);
        QTreeWidget::dropEvent(_event);
        Q_EMIT treeNodeMoved(m_selectedNode);
      }
    }
  }
}

void SvNavigatorTree::startDrag(Qt::DropActions _action)
{
  QList<QTreeWidgetItem*> items;
  items = selectedItems();
  if(items.length())
    m_selectedNode = items[0]->data(0, QTreeWidgetItem::UserType).toString();
  QTreeWidget::startDrag(_action);
}

QTreeWidgetItem* SvNavigatorTree::addNode(const NodeT& _node,
                                          const bool& _isFirstInsertion)
{
  QTreeWidgetItem* nitem = findNodeItem(_node.id);
  if(! nitem) {
    QTreeWidgetItem* item = SvNavigatorTree::createItem(_node);
    if(_isFirstInsertion && !_node.parent.isEmpty()) {
      GuiTreeItemListT::iterator pitem = m_items.find(_node.parent);
      if(pitem != m_items.end()) m_items[_node.parent]->addChild(item);
    }
    m_items.insert(_node.id, item);
  } else {
    nitem->setIcon(0, QIcon(":/images/built-in/unknown.png"));
    nitem->setText(0, _node.name);
    nitem->setData(0, QTreeWidgetItem::UserType, _node.id);
  }

  nitem = findNodeItem(_node.id); //FIXME : avoid research
  if (_node.type == NodeType::BusinessService && ! _node.child_nodes.isEmpty()) {
    QStringList cids = _node.child_nodes.split(ngrt4n::CHILD_SEP.c_str());
    Q_FOREACH (const QString& cid, cids) {
      GuiTreeItemListT::iterator chkit = m_items.find(cid);
      if (chkit == m_items.end()) {
        m_items[cid] = new QTreeWidgetItem(QTreeWidgetItem::UserType);
        nitem->addChild(m_items[cid]);
      } else {
        nitem->addChild(*chkit);
      }
    }
  }

  return nitem;
}

void SvNavigatorTree::update(void)
{
  QTreeWidget::clear();
  QTreeWidget::addTopLevelItem(m_items[ngrt4n::ROOT_ID]);
  QTreeWidget::setCurrentItem(m_items[ngrt4n::ROOT_ID]);
  QTreeWidget::expandAll();
}


void SvNavigatorTree::build(void)
{
  // Create a tree item for each bpnode
  for(NodeListT::ConstIterator node = m_cdata->bpnodes.begin(),
      end = m_cdata->bpnodes.end();
      node != end; ++node) {
    m_items.insertMulti(node->id, SvNavigatorTree::createItem(*node));
  }

  // Create a tree item for each bpnode
  for(NodeListT::ConstIterator node=m_cdata->cnodes.begin(),
      end=m_cdata->cnodes.end();
      node != end; ++node) {
    m_items.insertMulti(node->id, SvNavigatorTree::createItem(*node));
  }

  for(NodeListT::ConstIterator node = m_cdata->bpnodes.begin(),
      end = m_cdata->bpnodes.end();
      node != end; ++node) {
    QTreeWidgetItem* parent = findNodeItem(node->id);
    if (parent) {
      QStringList childs = node->child_nodes.split(ngrt4n::CHILD_SEP.c_str());
      Q_FOREACH(const QString& id, childs) {
        QTreeWidgetItem* child = findNodeItem(id);
        if (child) {
          parent->addChild(child);
        }
      }
    }
  }
  clear();
  addTopLevelItem(m_items[ngrt4n::ROOT_ID]);
}

QTreeWidgetItem* SvNavigatorTree::findNodeItem(const QString& _nodeId)
{
  GuiTreeItemListT::Iterator tnode = m_items.find(_nodeId);
  return (tnode != m_items.end())? *tnode : NULL;
}

QTreeWidgetItem* SvNavigatorTree::createItem(const NodeT& _node)
{
  QTreeWidgetItem* item = new QTreeWidgetItem(QTreeWidgetItem::UserType);
  item->setIcon(0, QIcon(":/images/built-in/unknown.png"));
  item->setText(0, _node.name);
  item->setData(0, QTreeWidgetItem::UserType, _node.id);
  return item;
}

void SvNavigatorTree::updateNodeItem(const NodeT& _node, const QString& _tip)
{
  QTreeWidgetItem* item = findNodeItem(_node.id);
  if (item) {
    item->setIcon(0, ngrt4n::severityIcon(_node.sev));
    item->setToolTip(0, _tip);
  }
}

