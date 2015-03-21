/*
 * WebPieChart.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 23-03-2014                                                  #
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
#include "WebPieChart.hpp"
#include "utilsCore.hpp"
#include <Wt/WStandardItem>
#include <QString>
#include <QDebug>
#include <Wt/WPainter>
#include <Wt/WPen>
#include <Wt/WScrollArea>
#include <Wt/Chart/WChartPalette>
#include <Wt/WTemplate>
#include "WebUtils.hpp"

class WebCharPalette : public Wt::Chart::WChartPalette
{
public:
  WebCharPalette(Wt::WStandardItemModel* model):
    m_model(model)
  {

  }

  virtual Wt::WBrush brush (int index) const
  {
    return Wt::WBrush(ngrt4n::severityWColor(index));
  }

  virtual Wt::WPen borderPen (int index) const
  {
    return Wt::WPen(Wt::WColor(255, 255, 255, 0));
  }

  Wt::WPen strokePen (int) const
  {
    // TODO: check value first
    return Wt::WPen(Wt::WColor(255, 255, 255, 1));
  }

  Wt::WColor fontColor (int index) const
  {
    // TOTO: check value first
    return Wt::WColor(255, 255, 255, 0);
  }


  virtual Wt::WColor color (int index) const
  {
    return Wt::WColor(255, 255, 255, 0);
  }

private:
  Wt::WStandardItemModel* m_model;
};

WebPieChart::WebPieChart(void)
  : Wt::Chart::WPieChart(),
    m_model(new Wt::WStandardItemModel(this)),
    m_scrollArea(new Wt::WScrollArea())
{
  setModel(m_model);

  resize(ngrt4n::CHART_WIDTH, ngrt4n::CHART_HEIGHT);  // WPaintedWidget must be given an explicit size.
  setMargin(0, Wt::Top);
  setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);

  m_scrollArea->setWidget(createChartTemplate());
  m_scrollArea->setMargin(0, Wt::Top| Wt::Bottom);

  // Configure the header.
  m_model->insertColumns(m_model->columnCount(), 2);
  m_model->setHeaderData(0, Wt::WString("Severity"));
  m_model->setHeaderData(1, Wt::WString("Count"));
  m_model->insertRows(m_model->rowCount(), 5);

  // Draw the chart
  setLabelsColumn(0);    // Set the column that holds the labels.
  setDataColumn(1);      // Set the column that holds the data.
  setDisplayLabels(Wt::Chart::NoLabels);
  setPerspectiveEnabled(true, 0.2); // Enable a 3D and shadow effect.
  setShadowEnabled(true);

  setPlotAreaPadding(0, Wt::All);
  setPalette(new WebCharPalette(m_model));
}


WebPieChart::~WebPieChart()
{
  delete m_model;
}


Wt::WTemplate* WebPieChart::createChartTemplate(void)
{
  Wt::WTemplate* m_chartTpl = new Wt::WTemplate(Wt::WString::tr("chart.tpl"));
  m_chartTpl->bindWidget("chart-legend-bar", m_chartLegendBar = createChartLegendBar());
  m_chartTpl->bindWidget("chart-content", this);
  m_chartLegendBar->setHidden(true);
  return m_chartTpl;
}

Wt::WTemplate* WebPieChart::createChartLegendBar(void)
{
  Wt::WTemplate* chartLegendBar = new Wt::WTemplate(Wt::WString::tr("chart-legend-bar.tpl"));
  chartLegendBar->bindWidget("unknown-count", m_legendBadges[ngrt4n::Unknown] = new Wt::WText());
  chartLegendBar->bindWidget("critical-count", m_legendBadges[ngrt4n::Critical] = new Wt::WText());
  chartLegendBar->bindWidget("major-count", m_legendBadges[ngrt4n::Major] = new Wt::WText());
  chartLegendBar->bindWidget("minor-count", m_legendBadges[ngrt4n::Minor] = new Wt::WText());
  chartLegendBar->bindWidget("normal-count", m_legendBadges[ngrt4n::Normal] = new Wt::WText());
  return chartLegendBar;
}


void WebPieChart::repaint()
{
  for(auto it = std::begin(m_statsData); it != std::end(m_statsData); ++it) {
    m_model->setData(it.key(), 0, Severity(it.key()).toString().toStdString());
    m_model->setData(it.key(), 1, it.value());
    m_legendBadges[it.key()]->setText(QString::number(it.value()).toStdString());
  }
  if (m_statsData[ngrt4n::Normal] > 0) {
    setExplode(ngrt4n::Normal, 0.3);
  }
  if (m_dataType == SLAData) {
    m_chartLegendBar->setHidden(true);
    setTitle(QObject::tr("SLA: %1%").arg(QString::number(m_severityRatio[ngrt4n::Normal])).toStdString());
  } else {
    m_chartLegendBar->setHidden(false);
  }
  setToolTip(ChartBase::tooltipText());
}
