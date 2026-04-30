; ===============================
; CalxSecure Installer Script
; ===============================

[Setup]
AppName=CalxSecure
AppVersion=1.0
AppPublisher=Nasir Nadaf
DefaultDirName={autopf}\CalxSecure
DefaultGroupName=CalxSecure
OutputDir=installer_output
OutputBaseFilename=CalxSecureSetup
Compression=lzma
SolidCompression=yes
WizardStyle=modern

; Require admin for Program Files install
PrivilegesRequired=admin

; Icon (optional)
; SetupIconFile=icon.ico

; ===============================
; FILES
; ===============================

[Files]
; Main EXE
#ifexist "C:\Users\aniga\source\repos\CalxSecure\out\build\release\CalxSecure\CalxSecure.exe"
Source: "C:\Users\aniga\source\repos\CalxSecure\out\build\release\CalxSecure\CalxSecure.exe"; DestDir: "{app}"; Flags: ignoreversion
#endif

; Qt DLLs
#ifexist "release\*.dll"
Source: "release\*.dll"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
#endif

; Platforms (IMPORTANT)
#ifexist "release\platforms\*"
Source: "release\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
#endif

; Styles (if exists)
#ifexist "release\styles\*"
Source: "release\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs
#endif

; SQL drivers (IMPORTANT if using QSql)
#ifexist "release\sqldrivers\*"
Source: "release\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs createallsubdirs
#endif

; ===============================
; SHORTCUTS
; ===============================

[Icons]
#ifexist "C:\Users\aniga\source\repos\CalxSecure\out\build\release\CalxSecure\CalxSecure.exe"
Name: "{group}\CalxSecure"; Filename: "{app}\CalxSecure.exe"
Name: "{commondesktop}\CalxSecure"; Filename: "{app}\CalxSecure.exe"; Tasks: desktopicon
#endif

; ===============================
; OPTIONAL TASKS
; ===============================

[Tasks]
Name: "desktopicon"; Description: "Create Desktop Shortcut"; GroupDescription: "Additional Icons:"; Flags: unchecked

; ===============================
; RUN AFTER INSTALL
; ===============================

[Run]
#ifexist "C:\Users\aniga\source\repos\CalxSecure\out\build\release\CalxSecure\CalxSecure.exe"
Filename: "{app}\CalxSecure.exe"; Description: "Launch CalxSecure"; Flags: nowait postinstall skipifsilent
#endif
