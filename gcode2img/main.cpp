#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>
#include <QDebug>
#include "gcode2img.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("gcode2img");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("G-Code  to convertorPCB image");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("gcode", QCoreApplication::translate("main", "G-code file."));
    parser.addPositionalArgument("img", QCoreApplication::translate("main", "PCB image."));
    parser.addPositionalArgument("width", QCoreApplication::translate("main", "Image width."));
    parser.addPositionalArgument("height", QCoreApplication::translate("main", "Image height."));
    parser.addPositionalArgument("dpi", QCoreApplication::translate("main", "Image dpi."));
    parser.process(a);
    if(parser.positionalArguments().count() != 5)
    {
        parser.showHelp(-1);
    }
    GCode2Img *task = new GCode2Img(parser.positionalArguments(), &a);
    QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));
    QTimer::singleShot(0, task, SLOT(run()));
    return a.exec();
}
