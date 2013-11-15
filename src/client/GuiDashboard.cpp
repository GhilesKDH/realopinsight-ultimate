/*
 * GuiDashboard.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2013 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 19-09-2013                                                  #
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
#include "GuiDashboard.hpp"
#include "core/MonitorBroker.hpp"
#include "core/ns.hpp"
#include "client/utilsClient.hpp"
#include "client/JsHelper.hpp"
#include "client/LsHelper.hpp"
#include <QScriptValueIterator>
#include <QNetworkCookieJar>
#include <QSystemTrayIcon>
#include <sstream>
#include <QStatusBar>
#include <QObject>
#include <zmq.h>
#include <iostream>
#include <algorithm>


namespace {
  const QString DEFAULT_TIP_PATTERN(QObject::tr("Service: %1\nDescription: %2\nSeverity: %3\n   Calc. Rule: %4\n   Prop. Rule: %5"));
  const QString ALARM_SPECIFIC_TIP_PATTERN(QObject::tr("\nTarget Host: %6\nData Point: %7\nRaw Output: %8\nOther Details: %9"));
  const QString SERVICE_OFFLINE_MSG(QObject::tr("Failed to connect to %1 (%2)"));
  const QString JSON_ERROR_MSG("{\"return_code\": \"-1\", \"message\": \""%SERVICE_OFFLINE_MSG%"\"}");
}

StringMapT GuiDashboard::propRules() {
  StringMapT map;
  map.insert(PropRules::label(PropRules::Unchanged), PropRules::toString(PropRules::Unchanged));
  map.insert(PropRules::label(PropRules::Decreased), PropRules::toString(PropRules::Decreased));
  map.insert(PropRules::label(PropRules::Increased), PropRules::toString(PropRules::Increased));
  return map;
}

StringMapT GuiDashboard::calcRules() {
  StringMapT map;
  map.insert(CalcRules::label(CalcRules::HighCriticity),
             CalcRules::toString(CalcRules::HighCriticity));
  map.insert(CalcRules::label(CalcRules::WeightedCriticity),
             CalcRules::toString(CalcRules::WeightedCriticity));
  return map;
}

GuiDashboard::GuiDashboard(const qint32& _userRole, const QString& _config)
  : DashboardBase(_userRole, _config),
    m_chart (std::make_shared<Chart>()),
    m_filteredMsgConsole (NULL),
    m_widget (new QSplitter(this)),
    m_rightSplitter (new QSplitter()),
    m_viewPanel (new QTabWidget()),
    m_browser (new WebKit()),
    m_map (new GraphView(m_cdata, this)),
    m_tree (new SvNavigatorTree()),
    m_msgConsole(new MsgConsole(this)),
    m_trayIcon(new QSystemTrayIcon(QIcon(":images/built-in/icon.png"))),
    m_bxSourceSelection(new QComboBox()),
    m_msgPane(new QTabWidget())
{
  m_viewPanel->addTab(m_map, tr("Map")),
      m_viewPanel->setTabIcon(ConsoleTab, QIcon(":images/hierarchy.png"));
  m_viewPanel->addTab(m_browser, tr("Web Browser")),
      m_viewPanel->setTabIcon(BrowserTab, QIcon(":images/web.png"));
  m_widget->addWidget(m_tree);
  m_widget->addWidget(m_rightSplitter);
  m_rightSplitter->addWidget(m_viewPanel);
  m_rightSplitter->addWidget(builtMsgPane());
  m_rightSplitter->setOrientation(Qt::Vertical);
  addEvents();
  if (! m_config.isEmpty()) {
    load(m_config);
  }
}

GuiDashboard::~GuiDashboard()
{
  if (m_filteredMsgConsole) delete m_filteredMsgConsole;
  delete m_msgConsole;
  m_chart.reset();
  delete m_tree;
  delete m_browser;
  delete m_map;
  delete m_cdata;
  delete m_viewPanel;
  delete m_rightSplitter;
  delete m_widget;
  delete m_preferences;
  delete m_changePasswdWindow;
  delete m_trayIcon;
  delete m_msgPane;
}

void GuiDashboard::load(const QString& _file)
{
  if (!_file.isEmpty()) {
    m_config = utils::getAbsolutePath(_file);
  }

  Parser parser(m_config, m_cdata);
  parser.process(true);
  parser.computeNodeCoordinates(0);

  m_tree->clear();
  m_tree->addTopLevelItem(m_cdata->tree_items[utils::ROOT_ID]);
  //FIXME: m_map->load(parser.getDotGraphFile(), m_cdata->bpnodes, m_cdata->cnodes);
  m_map->drawMap();

  m_root = m_cdata->bpnodes.find(utils::ROOT_ID);
  if (m_root == m_cdata->bpnodes.end()) {
    utils::alert(tr("The configuration seems to be invalid, there is not a root service!"));
    exit(1);
  }
}

void GuiDashboard::handleChangePasswordAction(void)
{
  m_changePasswdWindow->exec();
}

void GuiDashboard::handleChangeMonitoringSettingsAction(void)
{
  m_preferences->clearUpdatedSources();
  m_preferences->exec();
}

void GuiDashboard::handleShowOnlineResources(void)
{
  QDesktopServices appLauncher;
  appLauncher.openUrl(QUrl("http://RealOpInsight.com/"));
}

void GuiDashboard::handleShowAbout(void)
{
  Preferences about(m_userRole, Preferences::ShowAbout);
  about.exec();
}


void GuiDashboard::toggleTroubleView(bool _toggled)
{
  m_msgConsole->setEnabled(false);
  m_showOnlyTroubles = _toggled;
  if (m_showOnlyTroubles) {
    m_msgConsole->clearNormalMsg();
  } else {
    for (auto it = m_cdata->cnodes.begin(), end = m_cdata->cnodes.end();
         it != end; ++it) m_msgConsole->updateNodeMsg(it);
    m_msgConsole->sortByColumn(1);
  }
  m_msgConsole->setEnabled(true);
}

void GuiDashboard::toggleIncreaseMsgFont(bool _toggled)
{
  m_msgConsole->useLargeFont(_toggled);
}

void GuiDashboard::updateMap(const NodeListT::iterator& _node, const QString& _tip)
{
  m_map->updateNode(_node, _tip);
}

void GuiDashboard::updateDashboard(NodeListT::iterator& _node)
{
  updateDashboard(*_node);
}

void GuiDashboard::updateDashboard(const NodeT& _node)
{
  QString toolTip = utils::getNodeToolTip(_node);
  updateNavTreeItemStatus(_node, toolTip);
  m_map->updateNode(_node, toolTip);
  if (!m_showOnlyTroubles || (m_showOnlyTroubles && _node.severity != MonitorBroker::Normal)) {
    m_msgConsole->updateNodeMsg(_node);
    if (m_msgConsole->getRowCount() == 1) {
      m_msgConsole->updateEntriesSize(false);
    }
  }
  emit hasToBeUpdate(_node.parent);
}


void GuiDashboard::finalizeUpdate(const SourceT& src)
{
  if (m_cdata->cnodes.isEmpty()) {
    return;
  }

  Chart *chart = new Chart;
  chart->update(m_cdata->check_status_count, m_cdata->cnodes.size());
  m_map->updateStatsPanel(chart);
  m_chart.reset(chart);
  m_msgConsole->sortByColumn(1, Qt::AscendingOrder);

  for (NodeListIteratorT cnode = m_cdata->cnodes.begin(),
       end = m_cdata->cnodes.end(); cnode != end; ++cnode)
  {
    if (! cnode->monitored &&
        cnode->child_nodes.toLower()==utils::realCheckId(src.id,
                                                         QString::fromStdString(cnode->check.id)).toLower())
    {
      utils::setCheckOnError(MonitorBroker::Unknown,
                             tr("Undefined service (%1)").arg(cnode->child_nodes),
                             cnode->check);
      computeStatusInfo(cnode, src);
      m_cdata->check_status_count[cnode->severity]+=1;
      updateDashboard(cnode);
    }
    cnode->monitored = false;
  }
}

void GuiDashboard::updateNavTreeItemStatus(const NodeT& _node, const QString& _tip)
{
  auto tnode_it = m_cdata->tree_items.find(_node.id);
  if (tnode_it != m_cdata->tree_items.end()) {
    (*tnode_it)->setIcon(0, utils::computeCriticityIcon(_node.severity));
    (*tnode_it)->setToolTip(0, _tip);
  }
}

void GuiDashboard::expandNode(const QString& _nodeId, const bool& _expand, const qint32& _level)
{
  auto node = m_cdata->bpnodes.find(_nodeId);
  if (node == m_cdata->bpnodes.end()) {
    return;
  }
  if (!node->child_nodes.isEmpty()) {
    QStringList  childNodes = node->child_nodes.split(Parser::CHILD_SEP);
    foreach (const auto& cid, childNodes) {
      m_map->setNodeVisible(cid, _nodeId, _expand, _level);
    }
  }
}

void GuiDashboard::centerGraphOnNode(const QString& _nodeId)
{
  if (!_nodeId.isEmpty()) DashboardBase::setSelectedNode(_nodeId);
  m_map->centerOnNode(DashboardBase::getSelectedNode());
}

void GuiDashboard::filterNodeRelatedMsg(void)
{
  if (m_filteredMsgConsole) delete m_filteredMsgConsole;
  m_filteredMsgConsole = new MsgConsole();
  NodeListT::iterator node;
  if (utils::findNode(m_cdata, m_selectedNode, node)) {
    filterNodeRelatedMsg(m_selectedNode);
    QString title = tr("Messages related to '%2' - %1").arg(APP_NAME, node->name);
    m_filteredMsgConsole->updateEntriesSize(true);
    m_filteredMsgConsole->setWindowTitle(title);
  }
  qint32 rh = qMax(m_filteredMsgConsole->getRowCount() * m_filteredMsgConsole->rowHeight(0) + 50, 100);
  if (m_filteredMsgConsole->height() > rh) {
    m_filteredMsgConsole->resize(m_msgConsole->getConsoleSize().width(), rh);
  }
  m_filteredMsgConsole->sortByColumn(1, Qt::AscendingOrder);
  m_filteredMsgConsole->show();
}

void GuiDashboard::filterNodeRelatedMsg(const QString& _nodeId)
{
  NodeListT::iterator node;
  if (utils::findNode(m_cdata, _nodeId, node) &&
      !node->child_nodes.isEmpty()) {
    if (node->type == NodeType::ALARM_NODE) {
      m_filteredMsgConsole->updateNodeMsg(node);
    } else {
      QStringList childIds = node->child_nodes.split(Parser::CHILD_SEP);
      foreach (const QString& chkid, childIds) {
        filterNodeRelatedMsg(chkid);
      }
    }
  }
}

void GuiDashboard::centerGraphOnNode(QTreeWidgetItem * _item)
{
  centerGraphOnNode(_item->data(0, QTreeWidgetItem::UserType).toString());
}

void GuiDashboard::resizeDashboard(qint32 width, qint32 height)
{
  const qreal GRAPH_HEIGHT_RATE = 0.50;
  QSize mcSize = QSize(width * 0.80, height * (1.0 - GRAPH_HEIGHT_RATE));;

  QList<qint32> framesSize;
  framesSize.push_back(width * 0.20);
  framesSize.push_back(mcSize.width());
  m_widget->setSizes(framesSize);

  framesSize[0] = (height * GRAPH_HEIGHT_RATE);
  framesSize[1] = (mcSize.height());
  m_rightSplitter->setSizes(framesSize);

  m_widget->resize(width, height * 0.85);
  m_msgConsole->setConsoleSize(mcSize);
}


void GuiDashboard::scalPaneContentsToViewPort(void) const
{
  m_map->scaleToFitViewPort();
  m_msgConsole->updateEntriesSize(false);
}

void GuiDashboard::updateTrayInfo(const NodeT& _node)
{
  QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;
  if (_node.severity == MonitorBroker::Critical ||
      _node.severity == MonitorBroker::Unknown) {
    icon = QSystemTrayIcon::Critical;
  } else if (_node.severity == MonitorBroker::Minor ||
             _node.severity == MonitorBroker::Major) {
    icon = QSystemTrayIcon::Warning;
  }
  qint32 pbCount = m_cdata->cnodes.size() - m_cdata->check_status_count[MonitorBroker::Normal];
  QString title = APP_NAME%" - "%_node.name;
  QString msg = tr(" - %1 Problem%2\n"
                   " - Level of Impact: %3").arg(QString::number(pbCount), pbCount>1?tr("s"):"",
                                                 utils::criticityToText(_node.severity).toUpper());

  m_trayIcon->showMessage(title, msg, icon);
  m_trayIcon->setToolTip(title%"\n"%msg);
}

QTabWidget* GuiDashboard::builtMsgPane(void)
{
  QHBoxLayout* lyt(new QHBoxLayout());
  QGroupBox* wdgsGrp(new QGroupBox());
  lyt->addWidget(m_msgConsole, Qt::AlignLeft);
  lyt->setMargin(0);
  lyt->setContentsMargins(QMargins(0, 0, 0, 0));
  wdgsGrp->setLayout(lyt);
  m_msgPane->addTab(wdgsGrp, tr("Message Console"));
  return m_msgPane;
}

void GuiDashboard::handleSettingsLoaded(void)
{
  m_bxSourceSelection->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  for (SourceListT::iterator it=m_sources.begin(),
       end = m_sources.end(); it != end; ++it)
  {
    if (m_cdata->sources.contains(it->id))
    {
      switch(it->mon_type) {
        case MonitorBroker::Nagios:
          it->icon = ":images/nagios-logo-n.png";
          break;
        case MonitorBroker::Zabbix:
          it->icon = ":images/zabbix-logo-z.png";
          break;
        case MonitorBroker::Zenoss:
          it->icon = ":images/zenoss-logo-o.png";
          break;
        default:
          break;
      }
      m_bxSourceSelection->addItem(QIcon(it->icon), it->id, QVariant(it->id));
    }
  }
  handleUpdateSourceUrl();
}


void GuiDashboard::handleSourceBxItemChanged(int index)
{
  int idx = extractSourceIndex(m_bxSourceSelection->itemData(index).toString());
  SourceListT::Iterator src = m_sources.find(idx);
  if (src != m_sources.end()) {
    changeBrowserUrl(src->id, src->mon_url, src->icon);
  }
}

void GuiDashboard::handleUpdateSourceUrl(void)
{
  if (m_firstSrcIndex >=0 ) {
    SourceListT::Iterator first = m_sources.find(m_firstSrcIndex);
    if (first != m_sources.end()) {
      changeBrowserUrl(first->id, first->mon_url, first->icon);
    }
  }
}

void GuiDashboard::changeBrowserUrl(const QString& sid, const QString& url, const QString& icon)
{
  m_browser->setUrl(url);
  m_viewPanel->setTabText(BrowserTab, tr("Web Browser (%1)").arg(sid));
  m_viewPanel->setTabIcon(BrowserTab, QIcon(icon));
}

void GuiDashboard::setMsgPaneToolBar(const QList<QAction*>& menuAtions)
{
  QToolBar* tlbar (new QToolBar());
  tlbar->setOrientation(Qt::Vertical);
  foreach (QAction* action, menuAtions) {
    tlbar->addAction(action);
  }
  m_msgPane->widget(0)->layout()->addWidget(tlbar);
}

void GuiDashboard::addEvents(void)
{
  connect(m_viewPanel, SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged(int)));
  connect(m_preferences, SIGNAL(sourcesChanged(QList<qint8>)), this, SLOT(handleSourceSettingsChanged(QList<qint8>)));
  connect(this, SIGNAL(hasToBeUpdate(QString)), this, SLOT(updateBpNode(QString)));
  connect(m_map, SIGNAL(expandNode(QString, bool, qint32)), this, SLOT(expandNode(const QString &, const bool &, const qint32 &)));
  connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(centerGraphOnNode(QTreeWidgetItem *)));
  connect(m_bxSourceSelection, SIGNAL(activated(int)), this, SLOT(handleSourceBxItemChanged(int)));
  connect(this, SIGNAL(settingsLoaded(void)), this, SLOT(handleSettingsLoaded(void)));
  connect(this, SIGNAL(updateSourceUrl(void)), this, SLOT(handleUpdateSourceUrl(void)));
}
