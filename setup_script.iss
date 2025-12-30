[Setup]
AppId={{0080DD2B-33CB-4B0A-8AC3-9CA638B3C418}}
AppName=CyEcho
AppVersion=1.0.0
DefaultDirName={autopf}\CyEcho
DefaultGroupName=CyEcho
UninstallDisplayIcon={app}\CyEcho.exe
Compression=lzma
SolidCompression=yes
WizardStyle=modern
; Tells Inno Setup to create a 64-bit installer
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "out\build\x64-release\CyEcho.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "resources\*"; DestDir: "{app}\resources"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "out\build\x64-release\settings.json"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
Name: "{group}\CyEcho"; Filename: "{app}\CyEcho.exe"
Name: "{commondesktop}\CyEcho"; Filename: "{app}\CyEcho.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\CyEcho.exe"; Description: "{cm:LaunchProgram,CyEcho}"; Flags: nowait postinstall skipifsilent