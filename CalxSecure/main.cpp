#include "CalxSecure.h"
#include <QtWidgets/QApplication>
#include "AppController.h"
#include "LoginDialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    //CalxSecure window;
    //AppController window;
    LoginDialog window;
    window.show();
    return app.exec();
}
