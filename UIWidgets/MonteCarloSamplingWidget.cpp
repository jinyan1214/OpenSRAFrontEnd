#include "MonteCarloSamplingWidget.h"

#include <QGridLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QValidator>

MonteCarloSamplingWidget::MonteCarloSamplingWidget(QWidget* parent) : SimCenterAppWidget(parent)
{
    auto layout = new QGridLayout();

    // create layout label and entry for # samples
    numSamples = new QLineEdit();
    numSamples->setText(tr("20"));
    numSamples->setValidator(new QIntValidator);
    numSamples->setToolTip("Specify the number of samples");

    layout->addWidget(new QLabel("# Samples"), 0, 0);
    layout->addWidget(numSamples, 0, 1);

    // create label and entry for seed to layout
    srand(time(NULL));
    int randomNumber = rand() % 1000 + 1;
    randomSeed = new QLineEdit();
    randomSeed->setText(QString::number(randomNumber));
    randomSeed->setValidator(new QIntValidator);
    randomSeed->setToolTip("Set the seed");

    layout->addWidget(new QLabel("Seed"), 1, 0);
    layout->addWidget(randomSeed, 1, 1);

    layout->setRowStretch(2, 1);
    layout->setColumnStretch(2, 1);
    this->setLayout(layout);
}

MonteCarloSamplingWidget::~MonteCarloSamplingWidget()
{

}


bool MonteCarloSamplingWidget::outputToJSON(QJsonObject &jsonObj)
{
    jsonObj.insert("NumberOfSamples",numSamples->text().toInt());
    jsonObj.insert("Seed",randomSeed->text().toDouble());

    return true;
}


void MonteCarloSamplingWidget::clear()
{
    numSamples->clear();

    int randomNumber = rand() % 1000 + 1;
    randomSeed->setText(QString::number(randomNumber));
}


bool MonteCarloSamplingWidget::inputFromJSON(QJsonObject &jsonObject){

    int samples=jsonObject["NumberOfSamples"].toInt();

    double seed=jsonObject["Seed"].toDouble();

    numSamples->setText(QString::number(samples));
    randomSeed->setText(QString::number(seed));

    return true;
}
