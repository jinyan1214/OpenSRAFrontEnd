#include "FixedResidualsSamplingWidget.h"

#include <QGridLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QRegExpValidator>

#include <sstream>

FixedResidualsSamplingWidget::FixedResidualsSamplingWidget(QWidget* parent) : SimCenterAppWidget(parent)
{
    auto layout = new QGridLayout();

//    QRegExp LERegExp ("((([1-9][0-9]*))[ ]*,[ ]*)*([1-9][0-9]*|[0-9]*)");
//    QRegExpValidator* LEValidator = new QRegExpValidator(LERegExp);

    // create layout label and entry for # samples
    residualsLineEdit = new QLineEdit();
    residualsLineEdit->setText("-1.65, 0, 1.65");
//    residualsLineEdit->setValidator(LEValidator);
    residualsLineEdit->setToolTip("Specify the fixed residuals");

    layout->addWidget(new QLabel("ListOfResiduals"), 0, 0);
    layout->addWidget(residualsLineEdit, 0, 1);

    // create label and entry for seed to layout
    weightsLineEdit = new QLineEdit();
    weightsLineEdit->setText("0.2, 0.6, 0.2");
//    weightsLineEdit->setValidator(LEValidator);
    weightsLineEdit->setToolTip("Set the Weights");

    layout->addWidget(new QLabel("Weights"), 1, 0);
    layout->addWidget(weightsLineEdit, 1, 1);

    layout->setRowStretch(2, 1);
    layout->setColumnStretch(2, 1);
    this->setLayout(layout);

    QJsonObject test;
    this->outputToJSON(test);

    this->inputFromJSON(test);
}


FixedResidualsSamplingWidget::~FixedResidualsSamplingWidget()
{

}


void FixedResidualsSamplingWidget::clear()
{
    weightsLineEdit->clear();
    residualsLineEdit->clear();
}


bool FixedResidualsSamplingWidget::outputToJSON(QJsonObject &jsonObj)
{
    auto residualsText = residualsLineEdit->text();
    QJsonArray residualsArray = this->getArrayFromText(residualsText);

    auto weightsText = weightsLineEdit->text();
    QJsonArray weightsArray = this->getArrayFromText(weightsText);

    jsonObj["ListOfResiduals"]=residualsArray;
    jsonObj["Weights"]=weightsArray;

    return true;
}


bool FixedResidualsSamplingWidget::inputFromJSON(QJsonObject &jsonObject)
{
    auto resArray = jsonObject["ListOfResiduals"].toArray();
    auto weightsArray = jsonObject["Weights"].toArray();

    auto resText = this->getTextFromArray(resArray);
    auto weightsText = this->getTextFromArray(weightsArray);

    residualsLineEdit->setText(resText);
    weightsLineEdit->setText(weightsText);

    return true;
}


QJsonArray FixedResidualsSamplingWidget::getArrayFromText(const QString& text)
{
    QString inputText = text;

    QJsonArray array;

    // Quick return if the input text is empty
    if(inputText.isEmpty())
        return array;

    // Remove any white space from the string
    inputText.remove(" ");

    // Split the incoming text into the parts delimited by commas
    std::vector<std::string> subStringVec;

    // Create string stream from the string
    std::stringstream s_stream(inputText.toStdString());

    // Split the input string to substrings at the comma
    while(s_stream.good()) {
        std:: string subString;
        getline(s_stream, subString, ',');

        if(!subString.empty())
            subStringVec.push_back(subString);
    }

    try
    {
        for(auto&& subStr : subStringVec)
        {
            auto val = std::stod(subStr);
            array.append(val);
        }
    }
    catch (std::exception& e)
    {
        qDebug()<<"Error getting array form string with message "<<e.what();
    }

    return array;
}


QString FixedResidualsSamplingWidget::getTextFromArray(const QJsonArray& array)
{
    QString text;

    auto valList = array.toVariantList();

    auto numVals = valList.size();

    for(int i = 0; i<numVals; ++i)
    {
        auto val = valList.at(i);

        text.append(val.toString());

        if(i != numVals-1)
            text.append(", ");
    }

    return text;
}
