/*
 * WebServiceMap.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 19-09-2013                                                 #
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

#include <Wt/WPointF>
#include <Wt/WRectArea>
#include "WebMap.hpp"
#include "MonitorBroker.hpp"
#include "utilsClient.hpp"

namespace {
  const double MAP_PADDING = 40;
}

WebMap::WebMap(CoreDataT* _cdata)
  : WPaintedWidget(0),
    m_cdata(_cdata),
    m_scaleX (1),
    m_scaleY(1),
    m_layoutWidth(0),
    m_layoutHeight(0),
    m_icons(utils::nodeIcons()),
    m_scrollArea(new Wt::WScrollArea())
{
  setPreferredMethod(InlineSvgVml); //FIXME: do this according to the user agent
  setInline(false);

  setLayoutSizeAware(true);
  setJavaScriptMember(WWidget::WT_RESIZE_JS,
                      "function(self, w, h) {"
                      """var u = $(self).find('canvas, img');"
                      """if (w >= 0) "
                      ""  "u.width(w);"
                      """if (h >= 0) "
                      """u.height = h;"
                      "}");
  m_scrollArea->setWidget(this);
}


WebMap::~WebMap()
{
  m_icons.clear();
  delete m_scrollArea;
}

void WebMap::drawMap(const bool& _init)
{
  if(_init) {
    m_scaleX = m_layoutWidth/m_cdata->map_width;
    m_scaleY = static_cast<double>(YSCAL_FACTOR)/XSCAL_FACTOR * m_scaleX;
  }

  Wt::WPaintedWidget::update(); //this call paintEvent
  Wt::WPaintedWidget::resize(m_cdata->map_width * m_scaleX + MAP_PADDING,
                             m_cdata->map_height * m_scaleY + MAP_PADDING);
}

void WebMap::paintEvent(Wt::WPaintDevice* _pdevice)
{
  m_painter = new Wt::WPainter(_pdevice);
  m_painter->scale(m_scaleX, m_scaleY);  //TODO Make it dynamic
  m_painter->setRenderHint(Wt::WPainter::Antialiasing);

  // Draw edges
  // Must to be drawn before the icon for hiding some technical details
  for (StringListT::Iterator edge = m_cdata->edges.begin(), end = m_cdata->edges.end();
       edge != end; ++edge) {
    drawEdge(edge.key(), edge.value());
  }

  /* Draw node related to business services */
  for(NodeListT::ConstIterator node=m_cdata->bpnodes.begin(), end=m_cdata->bpnodes.end();
      node != end; ++node) {
    drawNode(*node);
  }

  /* Draw node related to alarm services */
  for(NodeListT::ConstIterator node=m_cdata->cnodes.begin(),end=m_cdata->cnodes.end();
      node != end; ++node) {
    drawNode(*node);
  }
}

/**
 * Draw edge before node to hide some details of drawing
 */
void WebMap::drawNode(const NodeT& _node)
{
  Wt::WPointF posIcon(_node.pos_x - 20,  _node.pos_y - 24);
  Wt::WPointF posLabel(_node.pos_x, _node.pos_y + 9);
  Wt::WPointF posExpIcon(_node.pos_x - 10, _node.pos_y + 15);

  // Draw icon
  m_painter->drawImage(posIcon,
                       Wt::WPainter::Image(utils::getResourcePath(m_icons[_node.icon]),40,40)
                       );
  // Draw anchor icon
  if( _node.type == NodeType::SERVICE_NODE) { //FIXME:  map_enable_nav_icon
    m_painter->drawImage(posExpIcon, Wt::WPainter::Image(utils::getResourcePath(m_icons[utils::MINUS]), 19, 18));
  }
  // Draw text
  m_painter->drawText(posLabel.x(), posLabel.y(),
                      Wt::WLength::Auto.toPixels(),
                      Wt::WLength::Auto.toPixels(),
                      Wt::AlignCenter,
                      Wt::WString(_node.name.toStdString()));
  createLink(_node);
}

void WebMap::drawEdge(const QString& _parentId, const QString& _childId)
{

  NodeListT::Iterator parent;
  NodeListT::Iterator child;

  if (utils::findNode(m_cdata->bpnodes, m_cdata->cnodes, _parentId, parent)
      && utils::findNode(m_cdata->bpnodes, m_cdata->cnodes, _childId, child)) {

    Wt::WPen pen;
    if(child->prop_sev == MonitorBroker::Critical){ //FIXME: child->prop_sev == MonitorBroker::Critical
      pen.setColor(Wt::red);
    } else if(child->prop_sev == MonitorBroker::Major){
      pen.setColor(Wt::darkYellow);
    } else if(child->prop_sev == MonitorBroker::Normal){
      pen.setColor(Wt::darkGreen);
    } else {
      pen.setColor(Wt::gray);
    }
    m_painter->setPen(pen);

    Wt::WPointF edgeP1(parent->pos_x, parent->pos_y + 24);
    Wt::WPointF edgeP2(child->pos_x, child->pos_y - 24);
    m_painter->drawLine(edgeP1, edgeP2);
  }

}

void WebMap::createLink(const NodeT& _node)
{
  double x = _node.pos_x * m_scaleX;
  double y = _node.pos_y * m_scaleY;
  double width = 40.0 * m_scaleX;
  double height = 40.0 * m_scaleY;
  Wt::WRectArea *area = new Wt::WRectArea(x, y, width, height);
  area->setToolTip(utils::getNodeToolTip(_node).toStdString());
  area->setLink("#");
  addArea(area);
}

//FIXME: void WebMap::msgPanelSizedChanged(int width)
//void WebMap::msgPanelSizedChanged(int width)
//{
//  layoutWidth = width;
//  drawMap(layoutWidth, layoutHeight, true);
//}