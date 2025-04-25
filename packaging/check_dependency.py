import os
import sys
import platform
import subprocess


def main():
    # project_name = os.getenv("PROJECT_NAME", "Pamplejuce")
    product_name = os.getenv("PRODUCT_NAME", "")
    standalone_path = os.getenv("Standalone_PATH", "")

    if platform.system() == 'Linux':
        for dp, dn, filenames in os.walk(standalone_path):
            for f in filenames:
                if os.path.splitext(f)[1] == '.so':
                    try:
                        result = subprocess.run(['ldd', os.path.join(dp, f)], capture_output=True, text=True)
                        print(result)
                        print(result.stdout)
                        print(result.stderr)
                    except Exception as e:
                        print("Error:", e)
    elif platform.system() == 'Windows':
        path2022 = 'C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Auxiliary/Build/vcvars64.bat'
        path2025 = 'C:/Program Files/Microsoft Visual Studio/2025/Enterprise/VC/Auxiliary/Build/vcvars64.bat'
        for msvc_path in [path2022, path2025]:
            if (os.path.exists(msvc_path)):
                subprocess.run(['call', msvc_path])
                print(msvc_path)
                break
        result = subprocess.run(['DUMPBIN', '/headers', standalone_path], capture_output=True, text=True)
        print(result)
        print(result.stdout)
        print(result.stderr)
    elif platform.system() == 'Darwin':
        macos_path = os.path.join(standalone_path, 'Contents', 'MacOS', product_name)
        result = subprocess.run(['otool', '-L', macos_path], capture_output=True, text=True)
        print(result)
        print(result.stdout)
        print(result.stderr)

if __name__ == '__main__':
    sys.exit(main())
