import os
import sys
import uuid
import hashlib
import subprocess
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
    We use MD5 to ensure uniqueness and stability, prefixed with 'ID_'.
    """
    hash_object = hashlib.md5(string_input.encode('utf-8'))
    # Hexdigest is alphanumeric (0-9, a-f). We prefix with 'ID_' to ensure it starts with a letter.
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
    
    # Generate Product GUID and UpgradeCode
    upgrade_code = get_guid(f"{project_name}_UpgradeCode") 

    f.write(f'    <Product Id="*" Name="{escape_xml(product_name)}" Language="1033" Version="{version}" Manufacturer="{escape_xml(publisher)}" UpgradeCode="{upgrade_code}">\n')
    f.write(f'        <Package InstallerVersion="500" Compressed="yes" InstallScope="perMachine" Platform="{platform_val}" />\n')
    f.write('        <MajorUpgrade DowngradeErrorMessage="A newer version of [ProductName] is already installed." />\n')
    f.write('        <MediaTemplate EmbedCab="yes" />\n')

    f.write('        <UIRef Id="WixUI_FeatureTree" />\n')
    if os.path.exists("packaging/EULA.rtf"):
        f.write('        <WixVariable Id="WixUILicenseRtf" Value="packaging/EULA.rtf" />\n')

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
    # -------------------------------------------

    f.write(f'                <Directory Id="CompanyDIR" Name="ZLAudio" />\n') 
    f.write('            </Directory>\n') 
    f.write('        </Directory>\n') 

    # 4. Harvest Files and Generate Components
    features = {} 
    
    # Configuration: (EnvVar, Extension, ParentDirID, IsBundleDefaultsToTrue)
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

        # SMART CHECK: Single file vs Directory
        is_actually_file = os.path.isfile(source_path)
        use_bundle_logic = is_bundle_config and not is_actually_file

        if use_bundle_logic:
            # --- BUNDLE LOGIC ---
            bundle_dir_name = f"{product_name}.{ext}"
            # Use get_wix_id for Directory ID
            bundle_dir_id = get_wix_id(f"DIR_{fmt_name}_BUNDLE")
            
            f.write(f'            <Directory Id="{bundle_dir_id}" Name="{bundle_dir_name}">\n')
            
            # Pass 'f' (the file object) to the function
            write_dir_recursive(f, source_path, bundle_dir_id, features[feature_id]["components"], fmt_name)

            f.write('            </Directory>\n')
        
        else:
            # --- SINGLE FILE LOGIC ---
            if is_actually_file:
                # Generate valid IDs
                comp_id = get_wix_id(f"COMP_{fmt_name}_FILE")
                file_id = get_wix_id(f"FILE_{fmt_name}_FILE")
                
                features[feature_id]["components"].append(comp_id)
                target_name = f"{product_name}.{ext}"
                
                f.write(f'            <Component Id="{comp_id}" Guid="{get_guid(comp_id)}" Win64="yes">\n')
                f.write(f'                <File Id="{file_id}" Source="{source_path}" Name="{target_name}" KeyPath="yes" />\n')
                f.write('            </Component>\n')
            else:
                # Folder but not a bundle (Standalone folder)
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

def write_dir_recursive(file_handle, current_os_path, parent_wix_id, component_list, prefix):
    try:
        items = os.listdir(current_os_path)
    except OSError:
        return

    files = [i for i in items if os.path.isfile(os.path.join(current_os_path, i))]
    dirs = [i for i in items if os.path.isdir(os.path.join(current_os_path, i))]
    files = [f for f in files if not f.endswith(".ilk")]

    # Write Files
    for filename in files:
        full_path = os.path.join(current_os_path, filename)
        
        # Unique IDs
        file_id = get_wix_id(f"FILE_{prefix}_{full_path}")
        comp_id = get_wix_id(f"COMP_{prefix}_{full_path}")
        
        component_list.append(comp_id)
        
        # Use file_handle instead of f
        file_handle.write(f'                <Component Id="{comp_id}" Guid="{get_guid(comp_id)}" Win64="yes">\n')
        file_handle.write(f'                    <File Id="{file_id}" Source="{full_path}" KeyPath="yes" />\n')
        file_handle.write(f'                </Component>\n')

    # Recurse Dirs
    for dirname in dirs:
        full_path = os.path.join(current_os_path, dirname)
        
        dir_id = get_wix_id(f"DIR_{prefix}_{full_path}")
        clean_dirname = escape_xml(dirname)
        
        # Use file_handle instead of f
        file_handle.write(f'                <Directory Id="{dir_id}" Name="{clean_dirname}">\n')
        write_dir_recursive(file_handle, full_path, dir_id, component_list, prefix)
        file_handle.write(f'                </Directory>\n')

if __name__ == '__main__':
    main()