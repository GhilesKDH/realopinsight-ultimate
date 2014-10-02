#ifndef SEVERITYAGGREGATOR_HPP
#define SEVERITYAGGREGATOR_HPP
#include "Base.hpp"

class StatusAggregator
{
public:
  explicit StatusAggregator(void);
  void resetData(void);
  void addSeverity(int value, double weight);
  void addThresholdLimit(QVector<ThresholdT>& thresholdsLimits, const ThresholdT& th);
  QString toDetailsString(void);
  void updateThresholds(void);
  int aggregate(int crule, const QVector<ThresholdT>& thresholdsLimits);
  static int propagate(int sev, int prule);
  int weightedAverage(void);
  int weightedAverageWithThresholds(const QVector<ThresholdT>& thresholdsLimits);
  int minSev(void) const {return m_minSeverity;}
  int maxSev(void) const {return m_maxSeverity;}
  int count(void) const {return m_count;}
  double totalWeight(void) const {return m_nonEssentialTotalWeight;}
  void displayWeight(void) { Q_FOREACH(int sev, m_severityWeightsMap.keys()) qDebug()<<Severity(sev).toString() <<  m_statusRatios[sev]; }
  QString thresholdExceededMsg(void) const {return m_thresholdExceededMsg;}

private:
  int m_count;
  int m_essentialCount;
  double m_nonEssentialTotalWeight;
  int m_minSeverity;
  int m_maxSeverity;
  int m_maxEssential;
  QString m_thresholdExceededMsg;
  QMap<int, double> m_severityWeightsMap;
  QMap<int, double> m_statusRatios;
};


#endif // SEVERITYAGGREGATOR_HPP
