cmake_minimum_required(VERSION 3.16)
project(CalxSecure LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
    Network
)

qt_standard_project_setup()

set(PROJECT_SOURCES
    main.cpp
    CalxSecure.ui
    CalxSecure.h
    CalxSecure.cpp
    AppController.h
    AppController.cpp
    AuthManager.h
    AuthManager.cpp
    DatabaseManager.h
    DatabaseManager.cpp
    GlobalStyle.h
    GlobalStyle.cpp
    components/AnimatedButton.h
    components/AnimatedButton.cpp
    components/Card.h
    components/Card.cpp
    components/DockWidget.h
    components/DockWidget.cpp
    components/GradientLabel.h
    components/GradientLabel.cpp
    components/NavBar.h
    components/NavBar.cpp
    Home.ui
    Home.h
    Home.cpp

    LoginDialog.ui
    LoginDialog.h
    LoginDialog.cpp
    DashboardPage.ui
    DashboardPage.h
    DashboardPage.cpp
     
    core/AppState.h
    core/AppState.cpp
    services/ApiClient.h
    services/ApiClient.cpp
    services/PaymentService.h
    services/PaymentService.cpp
     
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

set_target_properties(${PROJECT_NAME}
    PROPERTIES WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Network
)