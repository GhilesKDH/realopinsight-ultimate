/*
 * ZnsHelper.hpp
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


#ifndef ZENOSSHELPER_HPP_
#define ZENOSSHELPER_HPP_
#include "Base.hpp"
#include "JsonHelper.hpp"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QSslConfiguration>

namespace {
  const QString ZNS_API_CONTEXT = "/zport/dmd";
  const QString ZNS_LOGIN_API_CONTEXT = "/zport/acl_users/cookieAuthHelper/login";
  }

class ZnsHelper : public QNetworkAccessManager {
  Q_OBJECT

public:
  enum RequesT{
    Login=1,
    Component=2,
    Device=3,
    DeviceInfo=4
  };
  const static RequestListT ReqPatterns;
  const static RequestListT ContentTypes;
  const static RequestListT Routers;

  ZnsHelper(const QString & baseUrl="http://localhost:8080");
  virtual ~ZnsHelper();

  static RequestListT
  contentTypes();
  static RequestListT
  requestsPatterns();
  static RequestListT
  routers();
  void
  setBaseUrl(const QString & url);
  QNetworkReply*
  postRequest(int reqId,  const QByteArray & data);
  void
  setRouterEndpoint(const int & reqType);
  QString
  getRequestEndpoint(void) const {return m_reqHandler.url().toString();}
  void
  setRequestEndpoint(const QString & url) {m_reqHandler.setUrl(QUrl(url));}
  void
  setRequestEndpoint(const QUrl & url) {m_reqHandler.setUrl(url);}
  QString
  getApiContextEndpoint(void) const {return m_apiBaseUrl+ZNS_API_CONTEXT;}
  QString
  getApiBaseEndpoint(void) const {return m_apiBaseUrl;}
  static QString
  getDeviceName(const QString& uid) {return uid.mid(uid.lastIndexOf("/")+1, -1);}
  void
  setIsLogged(bool state) {m_isLogged = state;}
  QString
  lastError(void) const {return m_lastError;}
  bool
  getIsLogged(void) const {return m_isLogged;}
  void
  setSslPeerVerification(bool verifyPeer);
  int
  parseReply(QNetworkReply* reply);
  bool
  checkRPCResultStatus(void);
  int
  openSession(const SourceT& srcInfo);
  int
  processLoginReply(QNetworkReply* reply);
  int
  processDeviceReply(QNetworkReply* reply, ChecksT& checks);
  int
  processDeviceInfoReply(QNetworkReply* reply, ChecksT& checks);
  int
  processComponentReply(QNetworkReply* reply, ChecksT& checks);
  int
  loadChecks(const SourceT& srcInfo, ChecksT& checks,  const QString& filterValue,
             ngrt4n::RequestFilterT filterType = ngrt4n::HostFilter);


public Q_SLOTS:
  void processError(const QNetworkReply::NetworkError& code) { m_evlHandler.exit(code); }

Q_SIGNALS:
  void propagateError(QNetworkReply::NetworkError);

private :
  QString m_apiBaseUrl;
  QNetworkRequest m_reqHandler;
  QEventLoop m_evlHandler;
  bool m_isLogged;
  QSslConfiguration m_sslConfig;
  QString m_lastError;
  QString m_replyData;
  JsonHelper m_replyJsonData;

  void setSslReplyErrorHandlingOptions(QNetworkReply* reply);
  std::string parseHostGroups(const QScriptValue& json);
};

#endif /* ZENOSSHELPER_HPP_ */
