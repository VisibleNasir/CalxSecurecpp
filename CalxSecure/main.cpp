#include "CalxSecure.h"
#include <QtWidgets/QApplication>
#include "AppController.h"
#include "LoginDialog.h"
#include "Home.h"
#include "DashboardPage.h"
#include "P2PPage.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Do not exit the application when simple message boxes or the login dialog are closed
    app.setQuitOnLastWindowClosed(false);
    
    // Launch the AppController which acts as our main window and router
    AppController window;
    window.show();
    
    return app.exec();
}
