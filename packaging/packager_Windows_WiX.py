import os
import sys
import uuid
import hashlib
from pathlib import Path

# Helper to generate stable GUIDs
NAMESPACE_GUID = uuid.UUID('12345678-1234-5678-1234-567812345678')

def get_guid(string_input):
    return str(uuid.uuid5(NAMESPACE_GUID, string_input)).upper()

def get_wix_id(string_input):
    hash_object = hashlib.md5(string_input.encode('utf-8'))
    return "ID_" + hash_object.hexdigest().upper()

def escape_xml(s):
    return str(s).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace('"', "&quot;")

def main():
    # 1. Setup
    temp_dir = "./windowstmp"
    os.makedirs(temp_dir, exist_ok=True)
    
    project_name = os.getenv("PROJECT_NAME", "Pamplejuce")
    product_name = os.getenv("PRODUCT_NAME", "Pamplejuce Demo")
    version = os.getenv("VERSION", "0.0.0")
    publisher = os.getenv("COMPANY_NAME", "MyCompany")
    artifact_name = os.getenv("ARTIFACT_NAME", "Installer")
    
    is_arm = "arm" in artifact_name.lower()
    platform_val = "arm64" if is_arm else "x64"
    program_files_folder = "ProgramFiles64Folder" 

    outfile_path = "packaging/installer.wxs"
    os.makedirs(os.path.dirname(outfile_path), exist_ok=True)
    f = open(outfile_path, "w", encoding="utf-8")

    # 2. Header
    f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
    f.write(f'<Wix xmlns="http://schemas.microsoft.com/wix/2006/wi">\n')
    
    upgrade_code = get_guid(f"{project_name}_UpgradeCode") 

    f.write(f'    <Product Id="*" Name="{escape_xml(product_name)}" Language="1033" Version="{version}" Manufacturer="{escape_xml(publisher)}" UpgradeCode="{upgrade_code}">\n')
    f.write(f'        <Package InstallerVersion="500" Compressed="yes" InstallScope="perMachine" Platform="{platform_val}" />\n')
    f.write(f'        <MajorUpgrade AllowDowngrades="yes" Schedule="afterInstallInitialize" />\n')
    f.write(f'        <MediaTemplate EmbedCab="yes" />\n')

    # 3. Upgrade Detection (Fixing ICE61)
    # We only use this to detect if a newer version is present for the popup.
    # We add 'IncludeMaximum' to avoid overlap confusion, though MajorUpgrade handles the actual logic.
    f.write(f'        <Upgrade Id="{upgrade_code}">\n')
    f.write(f'            <UpgradeVersion Minimum="{version}" IncludeMinimum="no" OnlyDetect="yes" Property="NEWER_VERSION_DETECTED" />\n')
    f.write(f'        </Upgrade>\n')

    # 4. Custom Warning Action
    f.write('        <Binary Id="DowngradeWarningScript" SourceFile="packaging/downgrade_warn.vbs" />\n')
    f.write('        <CustomAction Id="CA_WarnDowngrade" BinaryKey="DowngradeWarningScript" VBScriptCall="Main" />\n')
    f.write('        <InstallUISequence>\n')
    f.write('            <Custom Action="CA_WarnDowngrade" After="AppSearch">NEWER_VERSION_DETECTED</Custom>\n')
    f.write('        </InstallUISequence>\n')

    # --- Directory Structure ---
    company_dir_id = "CompanyDIR"
    
    f.write('        <Directory Id="TARGETDIR" Name="SourceDir">\n')
    f.write(f'            <Directory Id="{program_files_folder}">\n')
    f.write('                <Directory Id="CommonFiles64Folder">\n')
    f.write('                    <Directory Id="VST3DIR" Name="VST3" />\n')
    f.write('                    <Directory Id="CLAPDIR" Name="CLAP" />\n')
    f.write('                    <Directory Id="LV2DIR" Name="LV2" />\n')
    f.write('                    <Directory Id="AvidDir" Name="Avid">\n')
    f.write('                        <Directory Id="AudioDir" Name="Audio">\n')
    f.write('                            <Directory Id="AAXDIR" Name="Plug-Ins" />\n')
    f.write('                        </Directory>\n')
    f.write('                    </Directory>\n')
    f.write('                </Directory>\n') 
    f.write(f'                <Directory Id="{company_dir_id}" Name="ZLAudio" />\n') 
    f.write('            </Directory>\n') 
    f.write('        </Directory>\n') 

    # --- Harvest & Features ---
    features = {} 
    found_formats = [] 
    
    formats = [
        ("VST3", "vst3", "VST3DIR", True),
        ("CLAP", "clap", "CLAPDIR", True),
        ("AAX", "aaxplugin", "AAXDIR", True),
        ("LV2", "lv2", "LV2DIR", True),
        ("Standalone", "exe", company_dir_id, False) 
    ]

    for fmt_name, ext, parent_dir_id, is_bundle_config in formats:
        env_var = f"{fmt_name}_PATH"
        if env_var not in os.environ:
            continue
            
        source_path = os.environ[env_var]
        if not os.path.exists(source_path):
            print(f"Warning: {source_path} not found. Skipping {fmt_name}.")
            continue

        print(f"Harvesting {fmt_name} from {source_path}...")
        
        # 1. Define Property (Checkbox)
        checkbox_prop = f"INSTALL_{fmt_name.upper()}"
        f.write(f'        <Property Id="{checkbox_prop}" Value="1" />\n')
        found_formats.append((fmt_name, checkbox_prop))

        feature_id = f"Feature_{fmt_name}"
        features[feature_id] = {"title": f"{fmt_name}", "components": [], "property": checkbox_prop}

        f.write(f'        <DirectoryRef Id="{parent_dir_id}">\n')

        is_actually_file = os.path.isfile(source_path)
        use_bundle_logic = is_bundle_config and not is_actually_file

        if use_bundle_logic:
            bundle_dir_name = f"{product_name}.{ext}"
            bundle_dir_id = get_wix_id(f"DIR_{fmt_name}_BUNDLE")
            f.write(f'            <Directory Id="{bundle_dir_id}" Name="{bundle_dir_name}">\n')
            write_dir_recursive(f, source_path, bundle_dir_id, features[feature_id]["components"], fmt_name)
            f.write('            </Directory>\n')
        else:
            if is_actually_file:
                comp_id = get_wix_id(f"COMP_{fmt_name}_FILE")
                file_id = get_wix_id(f"FILE_{fmt_name}_FILE")
                features[feature_id]["components"].append(comp_id)
                target_name = f"{product_name}.{ext}"
                f.write(f'            <Component Id="{comp_id}" Guid="{get_guid(comp_id)}" Win64="yes">\n')
                f.write(f'                <File Id="{file_id}" Source="{source_path}" Name="{target_name}" KeyPath="yes" />\n')
                f.write('            </Component>\n')
            else:
                write_dir_recursive(f, source_path, parent_dir_id, features[feature_id]["components"], fmt_name)

        f.write('        </DirectoryRef>\n')

    # --- Write Features ---
    f.write('        <Feature Id="Complete" Title="Complete Installation" Display="expand" Level="1" ConfigurableDirectory="TARGETDIR">\n')
    for feat_id, data in features.items():
        if not data["components"]: continue 
        # 2. Condition Feature based on Checkbox (Level 0 = Disabled)
        f.write(f'            <Feature Id="{feat_id}" Title="{data["title"]}" Level="1">\n')
        f.write(f'                <Condition Level="0"><![CDATA[{data["property"]} <> "1"]]></Condition>\n')
        for comp_id in data["components"]:
            f.write(f'                <ComponentRef Id="{comp_id}" />\n')
        f.write(f'            </Feature>\n')
    f.write('        </Feature>\n')

    # --- UI Logic ---
    eula_path = "packaging/EULA.rtf"
    readme_path = "packaging/Readme.rtf"
    license_file = None

    if os.path.exists(eula_path): license_file = eula_path
    elif os.path.exists(readme_path): license_file = readme_path
    else:
        license_file = os.path.join(temp_dir, "GenericLicense.rtf")
        with open(license_file, "w") as lf:
            lf.write(r"{\rtf1\ansi No EULA provided.\par}")

    f.write(f'        <WixVariable Id="WixUILicenseRtf" Value="{license_file}" />\n')
    f.write(f'        <Property Id="WIXUI_INSTALLDIR" Value="{company_dir_id}" />\n')

    # --- UI Injection (Fixing ICE20) ---
    f.write('        <UI>\n')
    # Use standard InstallDir logic (Provides ErrorDialog, FatalError, etc automatically)
    f.write('            <UIRef Id="WixUI_InstallDir" />\n')
    f.write('            <UIRef Id="WixUI_ErrorProgressText" />\n')

    # Define Custom Dialog
    f.write('            <Dialog Id="PluginSelectDlg" Width="370" Height="270" Title="Select Components">\n')
    f.write('                <Control Id="Next" Type="PushButton" X="236" Y="243" Width="56" Height="17" Default="yes" Text="Next">\n')
    f.write('                    <Publish Event="NewDialog" Value="VerifyReadyDlg">1</Publish>\n')
    f.write('                </Control>\n')
    f.write('                <Control Id="Cancel" Type="PushButton" X="304" Y="243" Width="56" Height="17" Cancel="yes" Text="Cancel">\n')
    f.write('                    <Publish Event="SpawnDialog" Value="CancelDlg">1</Publish>\n')
    f.write('                </Control>\n')
    f.write('                <Control Id="Back" Type="PushButton" X="180" Y="243" Width="56" Height="17" Text="Back">\n')
    f.write('                    <Publish Event="NewDialog" Value="InstallDirDlg">1</Publish>\n')
    f.write('                </Control>\n')
    
    f.write('                <Control Id="Description" Type="Text" X="25" Y="23" Width="280" Height="15" Transparent="yes" NoPrefix="yes" Text="Select the plugin formats you wish to install:" />\n')
    f.write('                <Control Id="Title" Type="Text" X="15" Y="6" Width="200" Height="15" Transparent="yes" NoPrefix="yes" Text="{\\WixUI_Font_Title}Select Components" />\n')
    f.write('                <Control Id="BannerBitmap" Type="Bitmap" X="0" Y="0" Width="370" Height="44" TabSkip="no" Text="!(loc.InstallDirDlgBannerBitmap)" />\n')
    f.write('                <Control Id="BannerLine" Type="Line" X="0" Y="44" Width="370" Height="0" />\n')
    f.write('                <Control Id="BottomLine" Type="Line" X="0" Y="234" Width="370" Height="0" />\n')

    current_y = 60
    for fmt_name, prop_name in found_formats:
        f.write(f'                <Control Id="Chk_{fmt_name}" Type="CheckBox" X="25" Y="{current_y}" Width="200" Height="17" Property="{prop_name}" CheckBoxValue="1" Text="Install {fmt_name}" />\n')
        current_y += 20
        
    f.write('            </Dialog>\n')

    # --- THE OVERRIDES ---
    # We use Order="5" to force these actions to take precedence over the default WixUI_InstallDir actions.
    
    # 1. On InstallDirDlg "Next", go to PluginSelectDlg (Instead of VerifyReady)
    f.write('            <Publish Dialog="InstallDirDlg" Control="Next" Event="NewDialog" Value="PluginSelectDlg" Order="5">1</Publish>\n')
    
    # 2. On VerifyReadyDlg "Back", go to PluginSelectDlg (Instead of InstallDir)
    f.write('            <Publish Dialog="VerifyReadyDlg" Control="Back" Event="NewDialog" Value="PluginSelectDlg" Order="5">1</Publish>\n')
    
    f.write('        </UI>\n')

    f.write('    </Product>\n')
    f.write('</Wix>\n')
    f.close()
    print(f"Generated {outfile_path}")

    generate_vbs("packaging/downgrade_warn.vbs")

def generate_vbs(vbs_path):
    with open(vbs_path, "w") as vbs:
        vbs.write("""
Function Main()
    Dim result
    result = MsgBox("You are installing an older version of this product. Do you want to continue?", 49, "Downgrade Warning")
    If result = 2 Then 
        Main = 1602 
    Else
        Main = 1
    End If
End Function
""")

def write_dir_recursive(file_handle, current_os_path, parent_wix_id, component_list, prefix):
    try:
        items = os.listdir(current_os_path)
    except OSError:
        return

    files = [i for i in items if os.path.isfile(os.path.join(current_os_path, i))]
    dirs = [i for i in items if os.path.isdir(os.path.join(current_os_path, i))]
    files = [f for f in files if not f.endswith(".ilk")]

    for filename in files:
        full_path = os.path.join(current_os_path, filename)
        file_id = get_wix_id(f"FILE_{prefix}_{full_path}")
        comp_id = get_wix_id(f"COMP_{prefix}_{full_path}")
        component_list.append(comp_id)
        
        file_handle.write(f'                <Component Id="{comp_id}" Guid="{get_guid(comp_id)}" Win64="yes">\n')
        file_handle.write(f'                    <File Id="{file_id}" Source="{full_path}" KeyPath="yes" />\n')
        file_handle.write(f'                </Component>\n')

    for dirname in dirs:
        full_path = os.path.join(current_os_path, dirname)
        dir_id = get_wix_id(f"DIR_{prefix}_{full_path}")
        clean_dirname = escape_xml(dirname)
        
        file_handle.write(f'                <Directory Id="{dir_id}" Name="{clean_dirname}">\n')
        write_dir_recursive(file_handle, full_path, dir_id, component_list, prefix)
        file_handle.write(f'                </Directory>\n')

if __name__ == '__main__':
    main()