/*
 * utilsCore.cpp
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

#include "utilsCore.hpp"
#include <QFileInfo>


namespace {
  const QString DEFAULT_TIP_PATTERN(QObject::tr("Service: %1\nDescription: %2\nSeverity: %3\n   Calc. Rule: %4\n   Prop. Rule: %5"));
  const QString ALARM_SPECIFIC_TIP_PATTERN(QObject::tr("\nTarget Host: %6\nData Point: %7\nRaw Output: %8\nOther Details: %9"));
}

QString ngrt4n::severityText(const qint32& _status)
{
  switch(static_cast<ngrt4n::SeverityT>(_status))
  {
  case ngrt4n::Normal:
    return QObject::tr("Normal");
    break;
  case ngrt4n::Minor:
    return  QObject::tr("Minor");
    break;
  case ngrt4n::Major:
    return  QObject::tr("Major");
    break;
  case ngrt4n::Critical:
    return  QObject::tr("Critical");
    break;
  default:
    break;
  }
  return QObject::tr("Unknown");
}

void ngrt4n::clear(CoreDataT& _cdata)
{
  _cdata.cnodes.clear();
  _cdata.bpnodes.clear();
  _cdata.edges.clear();
}

QString ngrt4n::getAbsolutePath(const QString& _path)
{
  QFileInfo fileInfo(_path);
  return fileInfo.absoluteFilePath();
}

ngrt4n::SeverityT ngrt4n::computeSeverity(const int& _monitor, const int& _statusOrSeverity)
{
  int criticity = ngrt4n::Unknown;
  if(_monitor == ngrt4n::Nagios) {
    switch(_statusOrSeverity) {
    case ngrt4n::NagiosOk:
      criticity = ngrt4n::Normal;
      break;
    case ngrt4n::NagiosWarning:
      criticity = ngrt4n::Major;
      break;
    case ngrt4n::NagiosCritical:
      criticity = ngrt4n::Critical;
      break;
    default:
      break;
    }
  } else if (_monitor == ngrt4n::Zabbix) {
    switch(_statusOrSeverity) {
    case ngrt4n::ZabbixClear:
      criticity = ngrt4n::Normal;
      break;
    case ngrt4n::ZabbixInfo:
    case ngrt4n::ZabbixWarn:
      criticity = ngrt4n::Minor;
      break;
    case ngrt4n::ZabbixAverage:
      criticity = ngrt4n::Major;
      break;
    case ngrt4n::ZabbixHigh:
    case ngrt4n::ZabbixDisaster:
      criticity = ngrt4n::Critical;
      break;
    default:
      break;
    }
  } else if (_monitor == ngrt4n::Zenoss){
    switch(_statusOrSeverity) {
    case ngrt4n::ZenossClear:
      criticity = ngrt4n::Normal;
      break;
    case ngrt4n::ZenossDebug:
      criticity = ngrt4n::Minor;
      break;
    case ngrt4n::ZenossWarning:
      criticity = ngrt4n::Major;
      break;
    case ngrt4n::ZenossError:
    case ngrt4n::ZenossCritical:
      criticity = ngrt4n::Critical;
      break;
    default:
      break;
    }
  }
  return static_cast<ngrt4n::SeverityT>(criticity);
}

int ngrt4n::computeSeverity2Propagate(const qint8& _critValue, const qint8& propRule)
{
  ngrt4n::SeverityT propCriticity = static_cast<ngrt4n::SeverityT>(_critValue);
  Criticity criticity(static_cast<ngrt4n::SeverityT>(_critValue));
  switch(propRule) {
  case PropRules::Increased: propCriticity = (++criticity).getValue();
    break;
  case PropRules::Decreased: propCriticity = (--criticity).getValue();
    break;
  default:
    break;
  }
  return propCriticity;
}

QString ngrt4n::getIconPath(int _severity)
{
  QString ipath("images/built-in/unknown.png");
  switch (static_cast<ngrt4n::SeverityT>(_severity)) {
  case ngrt4n::Normal:
    ipath = "images/built-in/normal.png";
    break;
  case ngrt4n::Minor:
    ipath = "images/built-in/minor.png";
    break;
  case ngrt4n::Major:
    ipath = "images/built-in/major.png";
    break;
  case ngrt4n::Critical:
    ipath = "images/built-in/critical.png";
    break;
  default:
    break;
  }
  return ipath;
}

bool ngrt4n::findNode(CoreDataT* coreData, const QString& nodeId, NodeListT::iterator& node)
{
  return findNode(coreData->bpnodes, coreData->cnodes, nodeId, node);
}

bool ngrt4n::findNode(NodeListT& bpnodes,
                      NodeListT& cnodes,
                      const QString& nodeId,
                      NodeListT::iterator& node)
{
  bool found = false;
  node = bpnodes.find(nodeId);
  if(node != bpnodes.end()) {
    found = true;
  } else {
    node = cnodes.find(nodeId);
    if(node != cnodes.end()) {
      found = true;
    }
  }
  return found;
}

bool ngrt4n::findNode(const NodeListT& bpnodes,
                      const NodeListT& cnodes,
                      const QString& nodeId,
                      NodeListT::const_iterator& node)
{
  bool found = false;
  node = bpnodes.find(nodeId);
  if(node != bpnodes.end()) {
    found = true;
  } else {
    node = cnodes.find(nodeId);
    if(node != cnodes.end()) {
      found = true;
    }
  }
  return found;
}

QString ngrt4n::sourceData2Json(const SourceT& src)
{
  return QString("{\"sid\":\"%1\","
                 "\"mon_type\":\"%2\","
                 "\"use_ngrt4nd\":\"%3\","
                 "\"mon_url\":\"%4\","
                 "\"ls_addr\":\"%5\","
                 "\"ls_port\":\"%6\","
                 "\"auth\":\"%7\","
                 "\"verify_ssl_peer\":\"%8\""
                 "}").arg(src.id,
                          QString::number(src.mon_type),
                          QString::number(src.use_ngrt4nd),
                          src.mon_url,
                          src.ls_addr,
                          QString::number(src.ls_port),
                          src.auth,
                          QString::number(src.verify_ssl_peer));
}

qint32 ngrt4n::convert2ApiType(const QString& str)
{
  QStringList types = sourceTypes();
  int type;
  if (str == types[ngrt4n::Nagios])
    type = ngrt4n::Nagios;
  else if (str == types[ngrt4n::Zabbix])
    type = ngrt4n::Zabbix;
  else if (str == types[ngrt4n::Zenoss])
    type = ngrt4n::Zenoss;
  else
    type = ngrt4n::Auto;

  return type;
}


void ngrt4n::setCheckOnError(int status, const QString& msg, CheckT& invalidCheck)
{
  invalidCheck.status = status;
  invalidCheck.last_state_change = humanTimeText("0");
  invalidCheck.host = "-";
  invalidCheck.check_command = "-";
  invalidCheck.alarm_msg = msg.toStdString();
}

QStringList ngrt4n::sourceTypes(void)
{
  return QStringList() << "Nagios-like"
                       << "Zabbix"
                       << "Zenoss";
}

QStringList ngrt4n::sourceIndexes(void)
{
  return QStringList() << "0"
                       << "1"
                       << "2"
                       << "3"
                       << "4"
                       << "5"
                       << "6"
                       << "7"
                       << "8"
                       << "9";
}


/* return <[sourcei:]hostaddr, checkid> */
StringPairT ngrt4n::splitHostCheckInfo(const QString& info)
{
  int pos = info.indexOf("/");
  QString second = ((pos == -1)? "ping" : info.mid(pos+1));

  return QPair<QString, QString>(info.left(pos), second);
}


/* return <source, hostaddr> */
StringPairT ngrt4n::splitSourceHostInfo(const QString& info)
{
  int pos = info.indexOf(":");
  QString first;
  if (pos == -1) {
    first = SRC_BASENAME%"0";
    return QPair<QString, QString>(first, info);
  }

  return QPair<QString, QString>(info.left(pos), info.mid(pos+1));
}


QString ngrt4n::getSourceIdFromStr(const QString& str)
{
  QString srcid = "";
  int pos = str.indexOf(":");
  if (pos != -1) {
    srcid = str.mid(0, pos);
  }
  return srcid;
}

QPair<bool, int> ngrt4n::checkSourceId(const QString &id)
{
  int index = -1;
  bool valid = false;
  if (! id.isEmpty()) {
    QString idStr = id.at(id.size()-1);
    if (id == SRC_BASENAME%idStr) {
      valid = true;
      index = idStr.toInt();
    }
  }
  return QPair<bool, int>(valid, index);
}


QString ngrt4n::getNodeToolTip(const NodeT& _node)
{
  QString toolTip = DEFAULT_TIP_PATTERN.arg(_node.name,
                                            const_cast<QString&>(_node.description).replace("\n", " "),
                                            ngrt4n::severityText(_node.severity),
                                            CalcRules::label(_node.sev_crule),
                                            PropRules::label(_node.sev_prule));
  if (_node.type == NodeType::AlarmNode) {
    toolTip += ALARM_SPECIFIC_TIP_PATTERN.arg(QString::fromStdString(_node.check.host).replace("\n", " "),
                                              _node.child_nodes,
                                              QString::fromStdString(_node.check.alarm_msg),
                                              _node.actual_msg);
  }
  return toolTip;
}

IconMapT ngrt4n::nodeIcons() {
  IconMapT icons;
  icons[DEFAULT_ICON]= ":/images/business-process.png";
  icons["Other Check"] = ":/images/check.png";
  icons["Server"] = ":/images/server.png";
  icons["Firewall"] = ":/images/firewall.png";
  icons["Router"] = ":/images/router.png";
  icons["Network"] = ":/images/network.png";
  icons["Switch"] = ":/images/switch.png";
  icons["Filer"] = ":/images/filer.png";
  icons["Hard disk"] = ":/images/harddisk.png";
  icons["Storage Area"] = ":/images/storage.png";
  icons["Linux"] = ":/images/linux.png";
  icons["Windows OS"] = ":/images/windows.png";
  icons["Solaris"] = ":/images/solaris.png";
  icons["Cloud"] = ":/images/cloud.png";
  icons["Hypervisor"] = ":/images/hypervisor.png";
  icons["Application"] = ":/images/application.png";
  icons["Web Accessibility"] = ":/images/web.png";
  icons["Web server"] = ":/images/web-server.png";
  icons["Database Engine"] = ":/images/db.png";
  icons["Database Server"] = ":/images/db-server.png";
  icons["Process"] = ":/images/process.png";
  icons["Logfile"] = ":/images/log.png";
  icons["Network Bandwith"] = ":/images/network-usage.png";
  icons["CPU"] = ":/images/cpu.png";
  icons["CPU Load"] = ":/images/performance-level.png";
  icons["Memory"] = ":/images/memory.png";
  icons["Memory Usage"] = ":/images/memory-usage.png";
  icons["Resource Utilization"] = ":/images/resource-usage.png";
  icons["Performance"] = ":/images/performance.png";
  icons[PLUS] = ":/images/plus.png";
  icons[MINUS] = ":/images/minus.png";
  icons["Nagios Basic Logo"] = ":/images/nagios-logo-n.png";
  icons["Zabbix Basic Logo"] = ":/images/zabbix-logo-z.png";
  icons["Zenoss Basic Logo"] = ":/images/zenoss-logo-o.png";
  icons["Hierarchy"] = ":/images/hierarchy.png";
  icons[MINUS] = ":/images/minus.png";
  return icons;
}


QStringList ngrt4n::getAuthInfo(const QString& authString)
{
  QStringList authInfo = QStringList();
  int pos = authString.indexOf(":");
  if (pos != -1) {
    authInfo.push_back(authString.left(pos));
    authInfo.push_back(authString.mid(pos+1, -1));
  }
  return authInfo;
}
