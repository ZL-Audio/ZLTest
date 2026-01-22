import os
import sys
import uuid
import hashlib
from pathlib import Path

# Helper to generate stable GUIDs based on file paths
NAMESPACE_GUID = uuid.UUID('12345678-1234-5678-1234-567812345678')

def get_guid(string_input):
    """Generates a standard UUID (e.g. DDC3F9...) for the Guid attribute."""
    return str(uuid.uuid5(NAMESPACE_GUID, string_input)).upper()

def get_wix_id(string_input):
    """
    Generates a valid WiX Identifier for the Id attribute.
    Rules: Must start with letter/underscore, no hyphens.
    """
    hash_object = hashlib.md5(string_input.encode('utf-8'))
    return "ID_" + hash_object.hexdigest().upper()

def escape_xml(s):
    return str(s).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace('"', "&quot;")

def main():
    # 1. Setup & Environment Variables
    temp_dir = "./windowstmp"
    os.makedirs(temp_dir, exist_ok=True)
    
    project_name = os.getenv("PROJECT_NAME", "Pamplejuce")
    product_name = os.getenv("PRODUCT_NAME", "Pamplejuce Demo")
    version = os.getenv("VERSION", "0.0.0")
    publisher = os.getenv("COMPANY_NAME", "MyCompany")
    artifact_name = os.getenv("ARTIFACT_NAME", "Installer")
    
    # Determine Architecture
    is_arm = "arm" in artifact_name.lower()
    platform_val = "arm64" if is_arm else "x64"
    program_files_folder = "ProgramFiles64Folder" 

    outfile_path = "packaging/installer.wxs"
    os.makedirs(os.path.dirname(outfile_path), exist_ok=True)
    f = open(outfile_path, "w", encoding="utf-8")

    # 2. Write WiX Header
    f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
    f.write(f'<Wix xmlns="http://schemas.microsoft.com/wix/2006/wi">\n')
    
    upgrade_code = get_guid(f"{project_name}_UpgradeCode") 

    f.write(f'    <Product Id="*" Name="{escape_xml(product_name)}" Language="1033" Version="{version}" Manufacturer="{escape_xml(publisher)}" UpgradeCode="{upgrade_code}">\n')
    f.write(f'        <Package InstallerVersion="500" Compressed="yes" InstallScope="perMachine" Platform="{platform_val}" />\n')
    f.write(f'        <MajorUpgrade AllowDowngrades="yes" Schedule="afterInstallInitialize" />\n')

    f.write(f'        <MediaTemplate EmbedCab="yes" />\n')

    # -------------------------------------------------------------------------
    # 3. DETECT if a newer version exists (for our custom warning)
    #    We need to know if we are downgrading so we can trigger the popup.
    # -------------------------------------------------------------------------
    f.write(f'        <Upgrade Id="{upgrade_code}">\n')
    f.write(f'            <UpgradeVersion Minimum="{version}" IncludeMinimum="no" OnlyDetect="yes" Property="NEWER_VERSION_DETECTED" />\n')
    f.write(f'        </Upgrade>\n')

    # 4. DEFINE THE WARNING POPUP (VBScript)
    #    This runs a simple message box. If the user clicks "Cancel" (2), installation stops.
    f.write('        <Binary Id="DowngradeWarningScript" SourceFile="packaging/downgrade_warn.vbs" />\n')
    f.write('        <CustomAction Id="CA_WarnDowngrade" BinaryKey="DowngradeWarningScript" VBScriptCall="Main" />\n')
    
    # 5. TRIGGER THE POPUP
    #    Only run this in the UI Sequence, and ONLY if we detected a newer version.
    f.write('        <InstallUISequence>\n')
    f.write('            <Custom Action="CA_WarnDowngrade" After="AppSearch">NEWER_VERSION_DETECTED</Custom>\n')
    f.write('        </InstallUISequence>\n')

    # --- UI Configuration & EULA Logic ---
    f.write('        <UIRef Id="WixUI_FeatureTree" />\n')
    
    eula_path = "packaging/EULA.rtf"
    readme_path = "packaging/Readme.rtf"
    license_file = None

    # Logic: Prefer EULA, fall back to Readme, fall back to generic generated RTF
    if os.path.exists(eula_path):
        license_file = eula_path
    elif os.path.exists(readme_path):
        print("No EULA found. Using Readme.rtf as License text.")
        license_file = readme_path
    else:
        # Generate a dummy RTF to prevent the "Strange" default WiX text
        print("No EULA or Readme found. Generating generic license file.")
        license_file = os.path.join(temp_dir, "GenericLicense.rtf")
        with open(license_file, "w") as lf:
            lf.write(r"{\rtf1\ansi\ansicpg1252\deff0\nouicompat\deflang1033{\fonttbl{\f0\fnil\fcharset0 Calibri;}}\viewkind4\uc1\pard\sa200\sl276\slmult1\f0\fs22\lang9 No EULA provided for this software.\par}")

    # Set the variable. This forces the UI to use OUR file, not the default.
    f.write(f'        <WixVariable Id="WixUILicenseRtf" Value="{license_file}" />\n')
    
    # Optional: Set the banner images if you have them (mimicking Inno Setup icons)
    # if os.path.exists("packaging/installer_top.bmp"):
    #     f.write(r'        <WixVariable Id="WixUIBannerBmp" Value="packaging/installer_top.bmp" />')
    # if os.path.exists("packaging/installer_side.bmp"):
    #     f.write(r'        <WixVariable Id="WixUIDialogBmp" Value="packaging/installer_side.bmp" />')

    # 3. Define Directory Structure
    f.write('        <Directory Id="TARGETDIR" Name="SourceDir">\n')
    f.write(f'            <Directory Id="{program_files_folder}">\n')
    
    # --- Common Files (VST3, AAX, LV2, CLAP) ---
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

    f.write(f'                <Directory Id="CompanyDIR" Name="ZLAudio" />\n') 
    f.write('            </Directory>\n') 
    f.write('        </Directory>\n') 

    # 4. Harvest Files and Generate Components
    features = {} 
    
    formats = [
        ("VST3", "vst3", "VST3DIR", True),
        ("CLAP", "clap", "CLAPDIR", True),
        ("AAX", "aaxplugin", "AAXDIR", True),
        ("LV2", "lv2", "LV2DIR", True),
        ("Standalone", "exe", "CompanyDIR", False) 
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
        
        feature_id = f"Feature_{fmt_name}"
        features[feature_id] = {"title": f"{fmt_name} Plugin", "components": []}

        f.write(f'        \n')
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

    # 5. Define Features
    f.write('        <Feature Id="Complete" Title="Complete Installation" Display="expand" Level="1" ConfigurableDirectory="TARGETDIR">\n')
    for feat_id, data in features.items():
        if not data["components"]: continue 
        f.write(f'            <Feature Id="{feat_id}" Title="{data["title"]}" Level="1">\n')
        for comp_id in data["components"]:
            f.write(f'                <ComponentRef Id="{comp_id}" />\n')
        f.write(f'            </Feature>\n')
    f.write('        </Feature>\n')

    f.write('    </Product>\n')
    f.write('</Wix>\n')
    f.close()
    print(f"Generated {outfile_path}")

    vbs_path = "packaging/downgrade_warn.vbs"
    with open(vbs_path, "w") as vbs:
        vbs.write("""
Function Main()
    Dim result
    ' MsgBox(Prompt, Buttons+Icon, Title)
    ' 1 = OK/Cancel, 48 = Warning Icon
    result = MsgBox("You are installing an older version of this product. This effectively downgrades the installation. Do you want to continue?", 49, "Downgrade Warning")
    
    If result = 2 Then ' 2 is Cancel
        Main = 1602 ' User Cancelled Error Code
    Else
        Main = 1 ' Success
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