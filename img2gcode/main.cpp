#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>
#include <QDebug>
#include "img2gcode.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("img2gcode");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("PCB image to G-Code convertor");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("img", QCoreApplication::translate("main", "PCB image."));
    parser.addPositionalArgument("out", QCoreApplication::translate("main", "G-code file."));
    parser.addPositionalArgument("pin", QCoreApplication::translate("main", "Laser control pin."));
    parser.process(a);
    if(parser.positionalArguments().count() != 3)
    {
        parser.showHelp(-1);
    }
    Img2Gcode *task = new Img2Gcode(parser.positionalArguments(), &a);
    QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));
    QTimer::singleShot(0, task, SLOT(run()));
    return a.exec();
}
