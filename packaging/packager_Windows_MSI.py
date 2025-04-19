import os
import sys
import json
import shutil
from pathlib import Path
import xml.etree.ElementTree as ET
from xml.dom import minidom
import uuid

class Packager:
    def __init__(self):
        self.project_name = os.environ.get('PROJECT_NAME')
        self.version = os.environ.get('VERSION')
        self.company_name = os.environ.get('COMPANY_NAME', 'Company')  # Default value
        self.company_website = os.environ.get('COMPANY_WEBSITE', '')  # Default value
        self.copyright = os.environ.get('COPYRIGHT', '')  # Default value
        self.description = os.environ.get('DESCRIPTION', '')  # Default value
        self.vst3_path = os.environ.get('VST3_PATH', '')
        self.clap_path = os.environ.get('CLAP_PATH', '')
        self.aax_path = os.environ.get('AAX_PATH', '')
        self.au_path = os.environ.get('AU_PATH', '')
        self.lv2_path = os.environ.get('LV2_PATH', '')
        self.standalone_path = os.environ.get('STANDALONE_PATH', '')

        if not self.project_name or not self.version:
            print("Error: PROJECT_NAME and VERSION environment variables must be set.")
            sys.exit(1)

        self.script_dir = Path(__file__).parent.resolve()
        self.assets_dir = self.script_dir / "Assets"
        self.output_dir = self.script_dir / "output"
        self.output_dir.mkdir(exist_ok=True)
        
        # Set up paths using the same structure as the original script
        self.icon_path = self.assets_dir / "installer_icon.ico"
        self.eula_path = self.assets_dir / "eula.rtf"
        self.readme_path = self.assets_dir / "readme.rtf"
        
        # Output file path
        self.wix_config_path = self.output_dir / f"{self.project_name}.wxs"

        # Check environment variables
        self.check_environment_variables()

    def check_environment_variables(self):
        """Verify that required environment variables are set."""
        required_vars = ['PROJECT_NAME', 'VERSION']
        for var in required_vars:
            if not os.environ.get(var):
                print(f"Error: {var} environment variable is not set.")
                sys.exit(1)

    def check_file_exists(self, path, file_type="File"):
        """Check if a file exists and print warning if not."""
        if not path:
            return False
        
        file_path = Path(path)
        if not file_path.exists():
            print(f"Warning: {file_type} not found at {file_path}")
            return False
        return True

    def create_wix_config(self):
        """Create the WiX configuration file."""
        # Root element
        root = ET.Element("Wix", xmlns="http://schemas.microsoft.com/wix/2006/wi")
        
        # Product information
        product = ET.SubElement(root, "Product", 
                               Id="*", 
                               Name=self.project_name,
                               Language="1033",
                               Version=self.version,
                               Manufacturer=self.company_name,
                               UpgradeCode=str(uuid.uuid5(uuid.NAMESPACE_DNS, f"{self.company_name}.{self.project_name}")))
        
        # Package information
        package = ET.SubElement(product, "Package", 
                               InstallerVersion="200",
                               Compressed="yes",
                               InstallScope="perMachine",
                               Description=self.description if self.description else self.project_name,
                               Manufacturer=self.company_name)
        
        # Media
        ET.SubElement(product, "Media", Id="1", Cabinet=f"{self.project_name}.cab", EmbedCab="yes")
        
        # Add icon
        if self.check_file_exists(self.icon_path, "Icon"):
            ET.SubElement(product, "Icon", Id="ProductIcon", SourceFile=str(self.icon_path))
            ET.SubElement(product, "Property", Id="ARPPRODUCTICON", Value="ProductIcon")
        
        # Add company website
        if self.company_website:
            ET.SubElement(product, "Property", Id="ARPURLINFOABOUT", Value=self.company_website)
        
        # Add EULA
        if self.check_file_exists(self.eula_path, "EULA"):
            ET.SubElement(product, "WixVariable", Id="WixUILicenseRtf", Value=str(self.eula_path))
        
        # Directory structure
        directory = ET.SubElement(product, "Directory", Id="TARGETDIR", Name="SourceDir")
        program_files_dir = ET.SubElement(directory, "Directory", Id="ProgramFiles64Folder")
        
        # Create menu folder for shortcuts
        menu_dir = ET.SubElement(directory, "Directory", Id="ProgramMenuFolder")
        app_menu_dir = ET.SubElement(menu_dir, "Directory", Id="ApplicationProgramsFolder", Name=self.project_name)
        
        # Create feature
        main_feature = ET.SubElement(product, "Feature", 
                                   Id="ProductFeature", 
                                   Title=self.project_name,
                                   Level="1")
        
        # Add plugins
        self.add_vst3_plugin(product, program_files_dir, main_feature)
        self.add_clap_plugin(product, program_files_dir, main_feature)
        self.add_aax_plugin(product, program_files_dir, main_feature)
        self.add_lv2_plugin(product, program_files_dir, main_feature)
        self.add_standalone_app(product, program_files_dir, main_feature, app_menu_dir)
        
        # Add readme
        self.add_readme(product, app_menu_dir, main_feature)
        
        # Add UI references
        ET.SubElement(product, "UIRef", Id="WixUI_InstallDir")
        ET.SubElement(product, "UIRef", Id="WixUI_ErrorProgressText")
        ET.SubElement(product, "Property", Id="WIXUI_INSTALLDIR", Value="INSTALLDIR")
        
        # Format the XML nicely
        rough_string = ET.tostring(root, encoding='utf-8')
        reparsed = minidom.parseString(rough_string)
        pretty_xml = reparsed.toprettyxml(indent="  ")
        
        # Write to file
        with open(self.wix_config_path, 'w') as f:
            f.write(pretty_xml)
        
        print(f"WiX configuration file created at {self.wix_config_path}")

    def add_vst3_plugin(self, product, program_files_dir, feature):
        """Add VST3 plugin to the installer."""
        if not self.vst3_path:
            return
            
        if self.check_file_exists(self.vst3_path, "VST3 Plugin"):
            # VST3 directory structure
            vst3_parent_dir = ET.SubElement(program_files_dir, "Directory", Id="VST3Dir", Name="Common Files")
            vst3_dir = ET.SubElement(vst3_parent_dir, "Directory", Id="VST3SubDir", Name="VST3")
            
            # VST3 component
            vst3_comp = ET.SubElement(product, "DirectoryRef", Id="VST3SubDir")
            vst3_component = ET.SubElement(vst3_comp, "Component", Id="VST3Component", Guid="*")
            
            # VST3 file
            ET.SubElement(vst3_component, "File", 
                         Id="VST3File", 
                         Source=self.vst3_path, 
                         KeyPath="yes")
            
            # Reference component
            ET.SubElement(feature, "ComponentRef", Id="VST3Component")

    def add_clap_plugin(self, product, program_files_dir, feature):
        """Add CLAP plugin to the installer."""
        if not self.clap_path:
            return
            
        if self.check_file_exists(self.clap_path, "CLAP Plugin"):
            # CLAP directory structure
            clap_parent_dir = ET.SubElement(program_files_dir, "Directory", Id="CLAPDir", Name="Common Files")
            clap_dir = ET.SubElement(clap_parent_dir, "Directory", Id="CLAPSubDir", Name="CLAP")
            
            # CLAP component
            clap_comp = ET.SubElement(product, "DirectoryRef", Id="CLAPSubDir")
            clap_component = ET.SubElement(clap_comp, "Component", Id="CLAPComponent", Guid="*")
            
            # CLAP file
            ET.SubElement(clap_component, "File", 
                         Id="CLAPFile", 
                         Source=self.clap_path, 
                         KeyPath="yes")
            
            # Reference component
            ET.SubElement(feature, "ComponentRef", Id="CLAPComponent")

    def add_aax_plugin(self, product, program_files_dir, feature):
        """Add AAX plugin to the installer."""
        if not self.aax_path:
            return
            
        if self.check_file_exists(self.aax_path, "AAX Plugin"):
            # AAX directory structure
            aax_parent_dir = ET.SubElement(program_files_dir, "Directory", Id="AAXDir", Name="Common Files")
            aax_avid_dir = ET.SubElement(aax_parent_dir, "Directory", Id="AvidDir", Name="Avid")
            aax_plugin_dir = ET.SubElement(aax_avid_dir, "Directory", Id="PlugInsDir", Name="Audio")
            aax_dir = ET.SubElement(aax_plugin_dir, "Directory", Id="AAXPlugInsDir", Name="Plug-Ins AAX")
            
            # AAX component
            aax_comp = ET.SubElement(product, "DirectoryRef", Id="AAXPlugInsDir")
            aax_component = ET.SubElement(aax_comp, "Component", Id="AAXComponent", Guid="*")
            
            # AAX file
            ET.SubElement(aax_component, "File", 
                         Id="AAXFile", 
                         Source=self.aax_path, 
                         KeyPath="yes")
            
            # Reference component
            ET.SubElement(feature, "ComponentRef", Id="AAXComponent")

    def add_lv2_plugin(self, product, program_files_dir, feature):
        """Add LV2 plugin to the installer."""
        if not self.lv2_path:
            return
            
        if self.check_file_exists(self.lv2_path, "LV2 Plugin"):
            # LV2 directory structure
            lv2_parent_dir = ET.SubElement(program_files_dir, "Directory", Id="LV2Dir", Name="Common Files")
            lv2_dir = ET.SubElement(lv2_parent_dir, "Directory", Id="LV2SubDir", Name="LV2")
            
            # LV2 component
            lv2_comp = ET.SubElement(product, "DirectoryRef", Id="LV2SubDir")
            lv2_component = ET.SubElement(lv2_comp, "Component", Id="LV2Component", Guid="*")
            
            # LV2 file
            ET.SubElement(lv2_component, "File", 
                         Id="LV2File", 
                         Source=self.lv2_path, 
                         KeyPath="yes")
            
            # Reference component
            ET.SubElement(feature, "ComponentRef", Id="LV2Component")

    def add_standalone_app(self, product, program_files_dir, feature, menu_dir):
        """Add standalone application to the installer."""
        if not self.standalone_path:
            return
            
        if self.check_file_exists(self.standalone_path, "Standalone Application"):
            # Standalone directory structure
            app_dir = ET.SubElement(program_files_dir, "Directory", 
                                   Id="CompanyDir", 
                                   Name=self.company_name)
            product_dir = ET.SubElement(app_dir, "Directory", 
                                       Id="INSTALLDIR", 
                                       Name=self.project_name)
            
            # Standalone component
            standalone_comp = ET.SubElement(product, "DirectoryRef", Id="INSTALLDIR")
            standalone_component = ET.SubElement(standalone_comp, "Component", Id="StandaloneComponent", Guid="*")
            
            # Standalone file
            ET.SubElement(standalone_component, "File", 
                         Id="StandaloneFile", 
                         Source=self.standalone_path, 
                         KeyPath="yes")
            
            # Shortcut component
            shortcut_comp = ET.SubElement(product, "DirectoryRef", Id="ApplicationProgramsFolder")
            shortcut_component = ET.SubElement(shortcut_comp, "Component", Id="ApplicationShortcut", Guid="*")
            
            # Application shortcut
            ET.SubElement(shortcut_component, "Shortcut",
                         Id="ApplicationStartMenuShortcut",
                         Name=self.project_name,
                         Description=self.description if self.description else self.project_name,
                         Target="[#StandaloneFile]",
                         WorkingDirectory="INSTALLDIR")
            
            # Add RemoveFolder element for proper uninstall
            ET.SubElement(shortcut_component, "RemoveFolder", 
                         Id="RemoveApplicationProgramsFolder", 
                         On="uninstall")
            
            # Add registry entry to ensure the component has a keypath
            ET.SubElement(shortcut_component, "RegistryValue",
                         Root="HKCU",
                         Key=f"Software\\{self.company_name}\\{self.project_name}",
                         Name="installed",
                         Type="integer",
                         Value="1",
                         KeyPath="yes")
            
            # Add component references
            ET.SubElement(feature, "ComponentRef", Id="StandaloneComponent")
            ET.SubElement(feature, "ComponentRef", Id="ApplicationShortcut")

    def add_readme(self, product, menu_dir, feature):
        """Add README file to the installer."""
        if not self.check_file_exists(self.readme_path, "README"):
            return
            
        # README component
        readme_comp = ET.SubElement(product, "DirectoryRef", Id="ApplicationProgramsFolder")
        readme_component = ET.SubElement(readme_comp, "Component", Id="ReadmeComponent", Guid="*")
        
        # README file
        readme_file = ET.SubElement(readme_component, "File", 
                                   Id="ReadmeFile", 
                                   Source=str(self.readme_path), 
                                   KeyPath="yes")
        
        # Add shortcut to readme
        ET.SubElement(readme_component, "Shortcut",
                     Id="ReadmeShortcut",
                     Name=f"{self.project_name} Readme",
                     Target="[#ReadmeFile]")
        
        # Reference component
        ET.SubElement(feature, "ComponentRef", Id="ReadmeComponent")

    def generate_msi_instructions(self):
        """Generate instructions for building the MSI package."""
        print("\nTo build the MSI package, you will need to install WiX Toolset and run:")
        print(f"candle.exe {self.wix_config_path}")
        print(f"light.exe -ext WixUIExtension {self.project_name}.wixobj -o {self.output_dir / (self.project_name + '-' + self.version + '.msi')}")

def main():
    """Main function to create the WiX configuration."""
    if sys.platform != "win32":
        print("This script is intended to be run on Windows only.")
        return
    
    packager = Packager()
    packager.create_wix_config()
    packager.generate_msi_instructions()

if __name__ == "__main__":
    main()