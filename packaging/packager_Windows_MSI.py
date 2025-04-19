import os
import uuid
from pathlib import Path
import xml.etree.ElementTree as ET
import xml.dom.minidom as minidom

def check_file_exists(file_path, file_description):
    """Check if a file exists and return its absolute path."""
    if not file_path or not os.path.isfile(file_path):
        print(f"Warning: {file_description} not found at {file_path}")
        return None
    return os.path.abspath(file_path)

def check_path_exists(path, path_description):
    """Check if a file or directory exists and return its absolute path."""
    if not path or not os.path.exists(path):
        print(f"Warning: {path_description} not found at {path}")
        return None
    return os.path.abspath(path)

def create_wix_xml(config):
    """Create a WiX XML configuration file for MSI packaging."""
    # Define namespaces
    wix_ns = "http://schemas.microsoft.com/wix/2006/wi"
    ET.register_namespace('', wix_ns)

    # Root element
    root = ET.Element(f"{{{wix_ns}}}Wix")
    product = ET.SubElement(root, f"{{{wix_ns}}}Product", {
        'Id': str(uuid.uuid4()),
        'Name': config['plugin_name'],
        'Language': '1033',
        'Version': config['version'],
        'Manufacturer': config['company_name'],
        'UpgradeCode': str(uuid.uuid4())
    })
    package = ET.SubElement(product, f"{{{wix_ns}}}Package", {
        'InstallerVersion': '200',
        'Compressed': 'yes',
        'InstallScope': 'perMachine',
        'Platform': 'x64'
    })

    # Media
    ET.SubElement(product, f"{{{wix_ns}}}MediaTemplate", {
        'EmbedCab': 'yes'
    })

    # Major Upgrade
    ET.SubElement(product, f"{{{wix_ns}}}MajorUpgrade", {
        'DowngradeErrorMessage': 'A newer version of [ProductName] is already installed.',
        'Schedule': 'afterInstallInitialize',
        'AllowDowngrades': 'no'
    })

    # Directory structure
    directory = ET.SubElement(product, f"{{{wix_ns}}}Directory", {
        'Id': 'TARGETDIR',
        'Name': 'SourceDir'
    })

    # CommonFiles64Folder for plugins
    common_files = ET.SubElement(directory, f"{{{wix_ns}}}Directory", {
        'Id': 'CommonFiles64Folder'
    })

    # VST3 directory
    vst3_dir = ET.SubElement(common_files, f"{{{wix_ns}}}Directory", {
        'Id': 'VST3Folder',
        'Name': 'VST3'
    })
    vst3_bundle_dir = ET.SubElement(vst3_dir, f"{{{wix_ns}}}Directory", {
        'Id': 'VST3BundleFolder',
        'Name': f"{config['plugin_name']}.vst3"
    })

    # LV2 directory
    lv2_dir = ET.SubElement(common_files, f"{{{wix_ns}}}Directory", {
        'Id': 'LV2Folder',
        'Name': 'LV2'
    })
    lv2_bundle_dir = ET.SubElement(lv2_dir, f"{{{wix_ns}}}Directory", {
        'Id': 'LV2BundleFolder',
        'Name': f"{config['plugin_name']}.lv2"
    })

    # CLAP directory
    clap_dir = ET.SubElement(common_files, f"{{{wix_ns}}}Directory", {
        'Id': 'CLAPFolder',
        'Name': 'CLAP'
    })

    # AAX directory
    avid_dir = ET.SubElement(common_files, f"{{{wix_ns}}}Directory", {
        'Id': 'AvidFolder',
        'Name': 'Avid'
    })
    audio_dir = ET.SubElement(avid_dir, f"{{{wix_ns}}}Directory", {
        'Id': 'AudioFolder',
        'Name': 'Audio'
    })
    aax_dir = ET.SubElement(audio_dir, f"{{{wix_ns}}}Directory", {
        'Id': 'AAXFolder',
        'Name': 'Plug-Ins'
    })
    aax_bundle_dir = ET.SubElement(aax_dir, f"{{{wix_ns}}}Directory", {
        'Id': 'AAXBundleFolder',
        'Name': f"{config['plugin_name']}.aaxplugin"
    })

    # ProgramFiles64Folder for standalone
    program_files = ET.SubElement(directory, f"{{{wix_ns}}}Directory", {
        'Id': 'ProgramFiles64Folder'
    })
    company_dir = ET.SubElement(program_files, f"{{{wix_ns}}}Directory", {
        'Id': 'COMPANYDIR',
        'Name': 'ZLAudio'
    })

    # Feature
    feature = ET.SubElement(product, f"{{{wix_ns}}}Feature", {
        'Id': 'MainFeature',
        'Title': config['plugin_name'],
        'Level': '1'
    })

    # Component groups
    vst3_components = ET.SubElement(product, f"{{{wix_ns}}}ComponentGroup", {
        'Id': 'VST3Components',
        'Directory': 'VST3BundleFolder'
    })
    lv2_components = ET.SubElement(product, f"{{{wix_ns}}}ComponentGroup", {
        'Id': 'LV2Components',
        'Directory': 'LV2BundleFolder'
    })
    clap_components = ET.SubElement(product, f"{{{wix_ns}}}ComponentGroup", {
        'Id': 'CLAPComponents',
        'Directory': 'CLAPFolder'
    })
    aax_components = ET.SubElement(product, f"{{{wix_ns}}}ComponentGroup", {
        'Id': 'AAXComponents',
        'Directory': 'AAXBundleFolder'
    })
    standalone_components = ET.SubElement(product, f"{{{wix_ns}}}ComponentGroup", {
        'Id': 'StandaloneComponents',
        'Directory': 'COMPANYDIR'
    })

    # Plugin files
    plugin_paths = [
        ('vst3_plugin_path', 'VST3 Plugin', 'VST3', vst3_components, True),
        ('lv2_plugin_path', 'LV2 Plugin', 'LV2', lv2_components, True),
        ('aax_plugin_path', 'AAX Plugin', 'AAX', aax_components, True),
        ('clap_plugin_path', 'CLAP Plugin', 'CLAP', clap_components, False),
        ('standalone_plugin_path', 'Standalone Plugin', 'Standalone', standalone_components, False)
    ]

    included_files = []

    for path_key, description, plugin_type, component_group, is_directory in plugin_paths:
        path = config.get(path_key)
        if path:
            component = ET.SubElement(component_group, f"{{{wix_ns}}}Component", {
                'Id': f'{plugin_type}Component',
                'Guid': str(uuid.uuid4()),
                'Win64': 'yes'
            })
            if is_directory:
                # Handle directories (e.g., VST3, LV2, AAX)
                dir_ref = ET.SubElement(component, f"{{{wix_ns}}}CreateFolder")
                files = [os.path.join(path, f) for f in os.listdir(path) if os.path.isfile(os.path.join(path, f))]
                if not files:
                    print(f"Warning: No files found in {description} directory at {path}")
                    continue
                for file_path in files:
                    file_id = f'{plugin_type}File_{os.path.basename(file_path).replace(".", "_").replace(" ", "_")}'
                    file_elem = ET.SubElement(component, f"{{{wix_ns}}}File", {
                        'Id': file_id,
                        'Source': file_path,
                        'KeyPath': 'yes' if file_path == files[0] else 'no'
                    })
                    included_files.append(file_path)
            else:
                # Handle single files (e.g., CLAP, Standalone)
                file_id = f'{plugin_type}File'
                file_name = f"{config['plugin_name']}.clap" if plugin_type == 'CLAP' else f"{config['plugin_name']}.exe"
                file_elem = ET.SubElement(component, f"{{{wix_ns}}}File", {
                    'Id': file_id,
                    'Source': path,
                    'Name': file_name,
                    'KeyPath': 'yes'
                })
                included_files.append(path)
            ET.SubElement(feature, f"{{{wix_ns}}}ComponentRef", {
                'Id': f'{plugin_type}Component'
            })

    # EULA for license agreement
    if config.get('eula_path'):
        ET.SubElement(product, f"{{{wix_ns}}}WixVariable", {
            'Id': 'WixUILicenseRtf',
            'Value': config['eula_path']
        })

    # Icon for MSI and Add/Remove Programs
    if config.get('icon_path'):
        ET.SubElement(product, f"{{{wix_ns}}}Icon", {
            'Id': 'ProductIcon',
            'SourceFile': config['icon_path']
        })
        ET.SubElement(product, f"{{{wix_ns}}}Property", {
            'Id': 'ARPPRODUCTICON',
            'Value': 'ProductIcon'
        })

    # UI
    ET.SubElement(product, f"{{{wix_ns}}}UIRef", {
        'Id': 'WixUI_Minimal'  # Use minimal UI to show EULA
    })

    # Check if any files were included
    if not included_files:
        raise RuntimeError("No files were included in the MSI package. Please check the environment variables and ensure at least one file or directory exists.")

    # Pretty print XML
    rough_string = ET.tostring(root, 'utf-8')
    reparsed = minidom.parseString(rough_string)
    pretty_xml = reparsed.toprettyxml(indent="  ")

    return pretty_xml

def main():
    # Configuration from environment variables
    config = {
        'plugin_name': os.getenv('PRODUCT_NAME', 'ZL Plugin'),
        'version': os.getenv('VERSION', '1.0.0'),
        'company_name': os.getenv('COMPANY_NAME', 'ZL Audio'),
        'output_dir': os.getenv('OUTPUT_DIR', ''),
        'vst3_plugin_path': os.getenv('VST3_PATH'),
        'lv2_plugin_path': os.getenv('LV2_PATH'),
        'aax_plugin_path': os.getenv('AAX_PATH'),
        'clap_plugin_path': os.getenv('CLAP_PATH'),
        'standalone_plugin_path': os.getenv('Standalone_PATH'),
        'icon_path': os.getenv('ICON_PATH', '/packaging/icon.ico'),
        'eula_path': os.getenv('EULA_PATH', '/packaging/EULA'),
        'readme_path': os.getenv('README_PATH', '/packaging/Readme.rtf')
    }

    # Log environment variables for debugging
    print("Environment variables:")
    for key, value in config.items():
        print(f"  {key}: {value}")

    # Validate paths
    config['vst3_plugin_path'] = check_path_exists(config['vst3_plugin_path'], 'VST3 Plugin') if config['vst3_plugin_path'] else None
    config['lv2_plugin_path'] = check_path_exists(config['lv2_plugin_path'], 'LV2 Plugin') if config['lv2_plugin_path'] else None
    config['aax_plugin_path'] = check_path_exists(config['aax_plugin_path'], 'AAX Plugin') if config['aax_plugin_path'] else None
    config['clap_plugin_path'] = check_file_exists(config['clap_plugin_path'], 'CLAP Plugin') if config['clap_plugin_path'] else None
    config['standalone_plugin_path'] = check_file_exists(config['standalone_plugin_path'], 'Standalone') if config['standalone_plugin_path'] else None
    config['icon_path'] = check_file_exists(config['icon_path'], 'Icon') if config['icon_path'] else None
    config['eula_path'] = check_file_exists(config['eula_path'], 'EULA') if config['eula_path'] else None
    config['readme_path'] = check_file_exists(config['readme_path'], 'Readme') if config['readme_path'] else None

    # Create output directory
    if len(config['output_dir']) > 0:
        os.makedirs(config['output_dir'], exist_ok=True)

    # Generate WiX XML
    wix_xml = create_wix_xml(config)
    output_path = os.path.join(config['output_dir'], f"{config['plugin_name']}.wxs")
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(wix_xml)

    print(f"WiX configuration file generated at {output_path}")
    print("Included files:")
    for file_path in included_files:
        print(f"  {file_path}")

if __name__ == "__main__":
    main()