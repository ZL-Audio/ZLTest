import os
import uuid
from pathlib import Path
import xml.etree.ElementTree as ET
import xml.dom.minidom as minidom

def check_file_exists(file_path, file_description):
    """Check if a file exists and return its absolute path."""
    if not os.path.isfile(file_path):
        print(f"Warning: {file_description} not found at {file_path}")
        return None
    return os.path.abspath(file_path)

def check_path_exists(path, path_description):
    """Check if a file or directory exists and return its absolute path."""
    if not os.path.exists(path):
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
        'InstallScope': 'perMachine'
    })

    # Media
    ET.SubElement(product, f"{{{wix_ns}}}MediaTemplate", {
        'EmbedCab': 'yes'
    })

    # Major Upgrade
    major_upgrade = ET.SubElement(product, f"{{{wix_ns}}}MajorUpgrade", {
        'DowngradeErrorMessage': 'A newer version of [ProductName] is already installed.',
        'AllowSameVersionUpgrades': 'yes'
    })

    # Directory structure
    directory = ET.SubElement(product, f"{{{wix_ns}}}Directory", {
        'Id': 'TARGETDIR',
        'Name': 'SourceDir'
    })
    program_files = ET.SubElement(directory, f"{{{wix_ns}}}Directory", {
        'Id': 'ProgramFiles64Folder'
    })
    company_dir = ET.SubElement(program_files, f"{{{wix_ns}}}Directory", {
        'Id': 'COMPANYDIR',
        'Name': config['company_name']
    })
    install_dir = ET.SubElement(company_dir, f"{{{wix_ns}}}Directory", {
        'Id': 'INSTALLFOLDER',
        'Name': config['plugin_name']
    })

    # Feature
    feature = ET.SubElement(product, f"{{{wix_ns}}}Feature", {
        'Id': 'MainFeature',
        'Title': config['plugin_name'],
        'Level': '1'
    })

    # Components
    component_group = ET.SubElement(product, f"{{{wix_ns}}}ComponentGroup", {
        'Id': 'PluginComponents',
        'Directory': 'INSTALLFOLDER'
    })

    # Plugin files
    plugin_paths = [
        ('vst3_plugin_path', 'VST3 Plugin', 'VST3'),
        ('clap_plugin_path', 'CLAP Plugin', 'CLAP'),
        ('standalone_plugin_path', 'Standalone Plugin', 'Standalone')
    ]

    for path_key, description, plugin_type in plugin_paths:
        path = config.get(path_key)
        if path:
            is_directory = os.path.isdir(path)
            component = ET.SubElement(component_group, f"{{{wix_ns}}}Component", {
                'Id': f'{plugin_type}Component',
                'Guid': str(uuid.uuid4())
            })
            if is_directory:
                # Handle directories (e.g., VST3)
                dir_ref = ET.SubElement(component, f"{{{wix_ns}}}CreateFolder")
                files = [os.path.join(path, f) for f in os.listdir(path) if os.path.isfile(os.path.join(path, f))]
                for file_path in files:
                    file_id = f'{plugin_type}File_{os.path.basename(file_path).replace(".", "_")}'
                    file_elem = ET.SubElement(component, f"{{{wix_ns}}}File", {
                        'Id': file_id,
                        'Source': file_path,
                        'KeyPath': 'yes' if file_path == files[0] else 'no'
                    })
            else:
                # Handle single files
                file_elem = ET.SubElement(component, f"{{{wix_ns}}}File", {
                    'Id': f'{plugin_type}File',
                    'Source': path,
                    'KeyPath': 'yes'
                })
            ET.SubElement(feature, f"{{{wix_ns}}}ComponentRef", {
                'Id': f'{plugin_type}Component'
            })

    # Additional files (EULA, Readme, Icon)
    additional_files = [
        ('eula_path', 'EULA', 'EULAFile'),
        ('readme_path', 'Readme', 'ReadmeFile'),
        ('icon_path', 'Icon', 'IconFile')
    ]

    for path_key, description, file_id in additional_files:
        path = config.get(path_key)
        if path:
            component = ET.SubElement(component_group, f"{{{wix_ns}}}Component", {
                'Id': f'{file_id}Component',
                'Guid': str(uuid.uuid4())
            })
            file_elem = ET.SubElement(component, f"{{{wix_ns}}}File", {
                'Id': file_id,
                'Source': path,
                'KeyPath': 'yes'
            })
            ET.SubElement(feature, f"{{{wix_ns}}}ComponentRef", {
                'Id': f'{file_id}Component'
            })

    # Property for EULA
    if config.get('eula_path'):
        ET.SubElement(product, f"{{{wix_ns}}}Property", {
            'Id': 'WIXUI_INSTALLDIR',
            'Value': 'INSTALLFOLDER'
        })
        ET.SubElement(product, f"{{{wix_ns}}}WixVariable", {
            'Id': 'WixUILicenseRtf',
            'Value': config['eula_path']
        })

    # UI
    ET.SubElement(product, f"{{{wix_ns}}}UIRef", {
        'Id': 'WixUI_InstallDir'
    })

    # Pretty print XML
    rough_string = ET.tostring(root, 'utf-8')
    reparsed = minidom.parseString(rough_string)
    pretty_xml = reparsed.toprettyxml(indent="  ")

    return pretty_xml

def main():
    # Configuration from environment variables
    config = {
        'plugin_name': os.getenv('PLUGIN_NAME', 'ZLPlugin'),
        'version': os.getenv('PLUGIN_VERSION', '1.0.0'),
        'company_name': os.getenv('COMPANY_NAME', 'ZL Audio'),
        'output_dir': os.getenv('OUTPUT_DIR', './dist'),
        'vst3_plugin_path': os.getenv('VST3_PATH'),
        'clap_plugin_path': os.getenv('CLAP_PATH'),
        'standalone_plugin_path': os.getenv('STANDALONE_PATH'),
        'icon_path': os.getenv('ICON_PATH'),
        'eula_path': os.getenv('EULA_PATH'),
        'readme_path': os.getenv('README_PATH')
    }

    # Validate paths
    config['vst3_plugin_path'] = check_path_exists(config['vst3_plugin_path'], 'VST3 Plugin') if config['vst3_plugin_path'] else None
    config['clap_plugin_path'] = check_file_exists(config['clap_plugin_path'], 'CLAP Plugin') if config['clap_plugin_path'] else None
    config['standalone_plugin_path'] = check_file_exists(config['standalone_plugin_path'], 'Standalone Plugin') if config['standalone_plugin_path'] else None
    config['icon_path'] = check_file_exists(config['icon_path'], 'Icon') if config['icon_path'] else None
    config['eula_path'] = check_file_exists(config['eula_path'], 'EULA') if config['eula_path'] else None
    config['readme_path'] = check_file_exists(config['readme_path'], 'Readme') if config['readme_path'] else None

    # Create output directory
    os.makedirs(config['output_dir'], exist_ok=True)

    # Generate WiX XML
    wix_xml = create_wix_xml(config)
    output_path = os.path.join(config['output_dir'], f"{config['plugin_name']}.wxs")
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(wix_xml)

    print(f"WiX configuration file generated at {output_path}")

if __name__ == "__main__":
    main()