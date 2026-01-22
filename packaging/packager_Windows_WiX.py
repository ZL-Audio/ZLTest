import os
import sys
import uuid
import subprocess
from pathlib import Path

# Helper to generate stable GUIDs based on file paths
NAMESPACE_GUID = uuid.UUID('12345678-1234-5678-1234-567812345678')

def get_guid(string_input):
    return str(uuid.uuid5(NAMESPACE_GUID, string_input)).upper()

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
    
    # Generate Product GUID
    product_id = "*" 
    upgrade_code = get_guid(f"{project_name}_UpgradeCode") 

    f.write(f'    <Product Id="{product_id}" Name="{escape_xml(product_name)}" Language="1033" Version="{version}" Manufacturer="{escape_xml(publisher)}" UpgradeCode="{upgrade_code}">\n')
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
    f.write('                    <Directory Id="CLAPDIR" Name="CLAP" />\n') # <--- ADDED CLAP
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
    # We set CLAP to True (Bundle) by default, but the logic below handles single-file overrides.
    formats = [
        ("VST3", "vst3", "VST3DIR", True),
        ("CLAP", "clap", "CLAPDIR", True),  # <--- ADDED CLAP
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

        # SMART CHECK:
        # If config says "Bundle" (Folder), but the source is actually a single File,
        # treat it as a file to prevent errors.
        is_actually_file = os.path.isfile(source_path)
        use_bundle_logic = is_bundle_config and not is_actually_file

        if use_bundle_logic:
            # --- BUNDLE LOGIC (Folders like .vst3, .aaxplugin, or .clap bundles) ---
            bundle_dir_name = f"{product_name}.{ext}"
            bundle_dir_id = f"DIR_{fmt_name}_BUNDLE"
            
            # Create the container folder
            f.write(f'            <Directory Id="{bundle_dir_id}" Name="{bundle_dir_name}">\n')
            
            # Recursively walk the content
            write_dir_recursive(f, source_path, bundle_dir_id, features[feature_id]["components"], fmt_name)

            f.write('            </Directory>\n')
        
        else:
            # --- SINGLE FILE / FLAT LOGIC ---
            if is_actually_file:
                # It's a single file (e.g. MyPlugin.clap or Standalone.exe)
                comp_id = f"COMP_{fmt_name}_FILE"
                file_id = f"FILE_{fmt_name}_FILE"
                features[feature_id]["components"].append(comp_id)
                
                # Check if the source filename matches the product name, if not, do we rename?
                # Usually WiX installs with the Name attribute.
                # Here we ensure the installed file is named "Product.ext"
                target_name = f"{product_name}.{ext}"
                
                f.write(f'            <Component Id="{comp_id}" Guid="{get_guid(comp_id)}" Win64="yes">\n')
                f.write(f'                <File Id="{file_id}" Source="{source_path}" Name="{target_name}" KeyPath="yes" />\n')
                f.write('            </Component>\n')
            else:
                # It's a folder, but is_bundle=False (Standalone app folder usually)
                # Just dump contents directly into Parent Dir
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
        # Unique IDs based on path
        file_id = get_guid(f"FILE_{prefix}_{full_path}")
        comp_id = get_guid(f"COMP_{prefix}_{full_path}")
        
        clean_filename = escape_xml(filename)
        component_list.append(comp_id)
        
        file_handle.write(f'                <Component Id="{comp_id}" Guid="{get_guid(comp_id)}" Win64="yes">\n')
        file_handle.write(f'                    <File Id="{file_id}" Source="{full_path}" KeyPath="yes" />\n')
        file_handle.write(f'                </Component>\n')

    # Recurse Dirs
    for dirname in dirs:
        full_path = os.path.join(current_os_path, dirname)
        dir_id = get_guid(f"DIR_{prefix}_{full_path}")
        clean_dirname = escape_xml(dirname)
        
        file_handle.write(f'                <Directory Id="{dir_id}" Name="{clean_dirname}">\n')
        write_dir_recursive(file_handle, full_path, dir_id, component_list, prefix)
        file_handle.write(f'                </Directory>\n')

if __name__ == '__main__':
    main()