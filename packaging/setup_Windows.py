import os
import subprocess
import sys
import platform
from pathlib import Path

def setup_windows_sdk_environment():
    # Detect architecture
    is_arm64 = platform.machine().lower() in ('arm64', 'aarch64')
    arch = "arm64" if is_arm64 else "x64"
    print(f"Detected architecture: {arch}")
    
    # Find Windows SDK installation path
    program_files = os.environ.get('ProgramFiles(x86)', 'C:\\Program Files (x86)')
    sdk_root = Path(program_files) / 'Windows Kits' / '10'
    
    # On ARM64, some paths might be different
    if is_arm64 and not sdk_root.exists():
        program_files = os.environ.get('ProgramFiles', 'C:\\Program Files')
        sdk_root = Path(program_files) / 'Windows Kits' / '10'
    
    if not sdk_root.exists():
        print(f"Windows SDK not found at {sdk_root}")
        return False
    
    # Find the latest SDK version
    include_dir = sdk_root / 'Include'
    if not include_dir.exists():
        print(f"Windows SDK Include directory not found at {include_dir}")
        return False
    
    # Get latest SDK version by checking directories
    sdk_versions = [d for d in include_dir.iterdir() if d.is_dir()]
    if not sdk_versions:
        print("No SDK versions found")
        return False
    
    latest_version = sorted(sdk_versions, key=lambda x: x.name)[-1].name
    print(f"Using Windows SDK version: {latest_version}")
    
    # Set up include paths
    include_paths = [
        str(sdk_root / 'Include' / latest_version / 'um'),
        str(sdk_root / 'Include' / latest_version / 'shared'),
        str(sdk_root / 'Include' / latest_version / 'ucrt'),
        str(sdk_root / 'Include' / latest_version / 'cppwinrt')
    ]
    
    # Find Visual Studio installation
    vs_paths = [
        Path(os.environ.get('ProgramFiles', 'C:\\Program Files')) / 'Microsoft Visual Studio',
        Path(os.environ.get('ProgramFiles(x86)', 'C:\\Program Files (x86)')) / 'Microsoft Visual Studio'
    ]
    
    vs_years = ['2022', '2019']
    editions = ['Enterprise', 'Professional', 'Community', 'BuildTools']
    
    # Find MSVC include paths
    msvc_include_path = None
    for vs_path in vs_paths:
        if not vs_path.exists():
            continue
            
        for year in vs_years:
            vs_year_path = vs_path / year
            if not vs_year_path.exists():
                continue
                
            for edition in editions:
                vs_edition_path = vs_year_path / edition
                if not vs_edition_path.exists():
                    continue
                    
                msvc_path = vs_edition_path / 'VC' / 'Tools' / 'MSVC'
                if msvc_path.exists():
                    msvc_versions = [d for d in msvc_path.iterdir() if d.is_dir()]
                    if msvc_versions:
                        latest_msvc = sorted(msvc_versions, key=lambda x: x.name)[-1]
                        msvc_include_path = latest_msvc / 'include'
                        print(f"Found MSVC at: {msvc_include_path}")
                        include_paths.append(str(msvc_include_path))
                        break
            
            if msvc_include_path:
                break
        
        if msvc_include_path:
            break
    
    # Set environment variables
    if 'INCLUDE' in os.environ:
        os.environ['INCLUDE'] = ';'.join(include_paths) + ';' + os.environ['INCLUDE']
    else:
        os.environ['INCLUDE'] = ';'.join(include_paths)
    
    # Set LIB environment variable - architecture specific
    lib_paths = [
        str(sdk_root / 'Lib' / latest_version / 'um' / arch),
        str(sdk_root / 'Lib' / latest_version / 'ucrt' / arch)
    ]
    
    # Add MSVC libraries for the specific architecture
    if msvc_include_path:
        msvc_lib_path = msvc_include_path.parent / 'lib' / arch
        if msvc_lib_path.exists():
            lib_paths.append(str(msvc_lib_path))
    
    if 'LIB' in os.environ:
        os.environ['LIB'] = ';'.join(lib_paths) + ';' + os.environ['LIB']
    else:
        os.environ['LIB'] = ';'.join(lib_paths)
    
    # Set PATH to include the right architecture tools
    bin_paths = []
    
    # Add Windows SDK tools path
    sdk_bin_path = sdk_root / 'bin' / latest_version / arch
    if sdk_bin_path.exists():
        bin_paths.append(str(sdk_bin_path))
    
    # Add MSVC bin path
    if msvc_include_path:
        msvc_bin_path = msvc_include_path.parent / 'bin' / 'Host' / arch / arch
        if msvc_bin_path.exists():
            bin_paths.append(str(msvc_bin_path))
    
    if bin_paths:
        if 'PATH' in os.environ:
            os.environ['PATH'] = ';'.join(bin_paths) + ';' + os.environ['PATH']
        else:
            os.environ['PATH'] = ';'.join(bin_paths)
    
    print(f"INCLUDE paths: {os.environ['INCLUDE']}")
    print(f"LIB paths: {os.environ['LIB']}")
    return True

if __name__ == "__main__":
    if setup_windows_sdk_environment():
        # Run the original command with the fixed environment
        if len(sys.argv) > 1:
            cmd = sys.argv[1:]
            print(f"Running command: {' '.join(cmd)}")
            result = subprocess.run(cmd)
            sys.exit(result.returncode)
        else:
            print("Environment configured successfully. No command to run.")
    else:
        print("Failed to set up Windows SDK environment")
        sys.exit(1)