#define Version Trim(FileRead(FileOpen("..\VERSION")))
#define ProjectName GetEnv('PROJECT_NAME')
#define ProductName GetEnv('PRODUCT_NAME')
#define Publisher GetEnv('COMPANY_NAME')
#define Year GetDateTimeString("yyyy","","")

[Setup]
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
AppName={#ProductName}
OutputBaseFilename={#ProductName}-{#Version}
AppCopyright=Copyright (C) {#Year} {#Publisher}
AppPublisher={#Publisher}
AppVersion={#Version}
DefaultDirName="{commoncf64}\VST3\{#ProductName}.vst3"
DisableDirPage=yes

; MAKE SURE YOU READ THE FOLLOWING!
; LicenseFile="EULA.rtf"
InfoBeforeFile="EULA.rtf"
UninstallFilesDir="{commonappdata}\{#ProductName}\uninstall"

[Types]
Name: "full";     Description: "Full installation";
Name: "lv2";      Description: "LV2 only";
Name: "vst3"; Description: "VST3 only";

[Components]
Name: "full";     Description: "Full installation"; Types: lv2 vst3
Name: "lv2";      Description: "LV2 only";          Types: lv2
Name: "vst3";     Description: "VST3 only";         Types: vst3

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#ProductName}Data"

; MSVC adds a .ilk when building the plugin. Let's not include that.
[Files]
Source: "..\Builds\{#ProjectName}_artefacts\Release\LV2\{#ProductName}.lv2\*"; DestDir: "{commoncf64}\LV2\{#ProductName}.lv2\"; Excludes: *.ilk; Flags: ignoreversion recursesubdirs; Components: full lv2

Source: "..\Builds\{#ProjectName}_artefacts\Release\VST3\{#ProductName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#ProductName}.vst3\"; Excludes: *.ilk; Flags: ignoreversion recursesubdirs; Components: full vst3

[Run]
Filename: "{cmd}"; \
    WorkingDir: "{commoncf64}\LV2"; \
    Parameters: "/C mklink /D ""{commoncf64}\LV2\{#ProductName}Data"" ""{commonappdata}\{#ProductName}"""; \
    Flags: runascurrentuser; Components: full lv2
Filename: "{cmd}"; \
    WorkingDir: "{commoncf64}\VST3"; \
    Parameters: "/C mklink /D ""{commoncf64}\VST3\{#ProductName}Data"" ""{commonappdata}\{#ProductName}"""; \
    Flags: runascurrentuser; Components: full vst3