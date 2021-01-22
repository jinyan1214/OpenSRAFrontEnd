#ifndef FixedResidualsSamplingWidget_H
#define FixedResidualsSamplingWidget_H

#include "SimCenterAppWidget.h"

class QLineEdit;

class FixedResidualsSamplingWidget : public SimCenterAppWidget
{
public:
    FixedResidualsSamplingWidget(QWidget* parent = nullptr);
    ~FixedResidualsSamplingWidget();

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

    void clear();

private:

    QJsonArray getArrayFromText(const QString& text);
    QString getTextFromArray(const QJsonArray& array);

    QLineEdit *weightsLineEdit;
    QLineEdit *residualsLineEdit;
};

#endif // FixedResidualsSamplingWidget_H
