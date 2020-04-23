#include <QApplication>
#include <QTimer>
#include <QCommandLineParser>
#include <QDebug>
#include "img2gcode.h"
#include "img2vectors.h"
#include "grb2vectors.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("img2gcode");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("PCB image to G-Code convertor");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("img", QCoreApplication::translate("main", "PCB image."));
    parser.addPositionalArgument("out", QCoreApplication::translate("main", "G-code file."));
    parser.addPositionalArgument("pin", QCoreApplication::translate("main", "Laser control pin."));
    QCommandLineOption tVectorizeOption("vectorize", QCoreApplication::translate("main", "Vectorize image"));
    QCommandLineOption tGrbOption("grb", QCoreApplication::translate("main", "Convert from Gerber"));
    QCommandLineOption tHorizontalScanOption("xscan", QCoreApplication::translate("main", "Horizontal scan"));
    parser.addOption(tVectorizeOption);
    parser.addOption(tHorizontalScanOption);
    parser.addOption(tGrbOption);
    parser.process(a);
    if(parser.positionalArguments().count() < 3)
    {
        parser.showHelp(-1);
    }
    BaseTask *task = nullptr;
    if(parser.isSet(tVectorizeOption))
        task = new img2vectors(parser.positionalArguments(), &a);
    else if(parser.isSet(tGrbOption))
        task = new grb2vectors(parser.positionalArguments(), &a);
    else
        task = new Img2Gcode(parser.positionalArguments(), parser.isSet(tHorizontalScanOption), &a);
    QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));
    QTimer::singleShot(0, task, SLOT(run()));
    return a.exec();
}
