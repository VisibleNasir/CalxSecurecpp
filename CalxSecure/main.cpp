#include "CalxSecure.h"
#include <QtWidgets/QApplication>
#include "AppController.h"
#include "LoginDialog.h"
#include "Home.h"
#include "DashboardPage.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // FIX: Do not kill the application when simple message boxes or the login dialog are manipulated
    app.setQuitOnLastWindowClosed(false);
    
    // Launch the AppController which acts as our main window and router
    AppController window;
    
    // Since AppController hides itself instantly to show LoginDialog, we call show here.
    window.show();
    
    return app.exec();
}
