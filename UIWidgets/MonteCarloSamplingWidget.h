#ifndef MONTECARLOSAMPLINGWIDGET_H
#define MONTECARLOSAMPLINGWIDGET_H

#include "SimCenterAppWidget.h"

class QLineEdit;

class MonteCarloSamplingWidget : public SimCenterAppWidget
{
public:
    MonteCarloSamplingWidget(QWidget* parent = nullptr);
    ~MonteCarloSamplingWidget();

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

    void clear();

private:

    QLineEdit *randomSeed;
    QLineEdit *numSamples;
};

#endif // MONTECARLOSAMPLINGWIDGET_H
