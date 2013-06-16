#include "OpenAggregator.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OpenAggregator w;
    w.show();
    
    return a.exec();
}
